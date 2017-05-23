#include <stdbool.h>
#include <errno.h>
#include "buffer.h"

#define DEFAULT_BUFFER_SHRINKSIZE 65536

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

KHASH_MAP_INIT_INT64(pointer_hashmap, void*)

typedef struct {
    size_t max_size;
    size_t size;
    size_t mark;
    size_t used;
    size_t used_before;
    void* current;
    khash_t(pointer_hashmap)* offsets;
    bool offsets_active;
} buffer;

void buffer_consume(oct_buffer* buf, size_t consumed) {
    buffer* b = (buffer*) buf;
    b->used_before = b->used;
    b->used += consumed;
}

void buffer_mark(oct_buffer* buf) {
    buffer* b = (buffer*) buf;
    /* unfortunately, the parser doesn't tell us where the request ends exactly,
     * so the only thing we can be sure is that it ends in the current buffer chunk, so anything before it can
     * effectively be swept, so we're placing the mark at that point now. */
    b->mark = b->used_before;
}

void buffer_sweep(oct_buffer* buf) {
    buffer* b = (buffer*) buf;
    void* pointer;
    int offset;
    int used = b->used - b->mark;

    if (b->mark > 0) {
        bool offsets_active = false;

        if (b->used > 0) {
            /* Move data beyond the mark to the beginning of the buffer.
             * While we should avoid memory copies, this is relatively infrequent and will only really copy a
             * significant amount of data if requests are pipelined. Otherwise, we'll just be copying the last chunk
             * that was read back to the beginning of the buffer. */
            memcpy(b->current, b->current + b->mark, used);
        }

        if (b->size > DEFAULT_BUFFER_SHRINKSIZE && b->used < DEFAULT_BUFFER_SHRINKSIZE) {
            /* Shrink buffer */
            b->size = DEFAULT_BUFFER_SHRINKSIZE;
            b->current = realloc(b->current, b->size);
            if (!b->current) {
                errno = ENOMEM;
                b->size = 0;
            }
        }

        /* Update offsets */
        if (b->used) {
            kh_foreach(b->offsets, pointer, offset, {
                    khiter_t offset_key = kh_get(pointer_hashmap, b->offsets, pointer);

                    if (offset <= b->mark) {
                        /* Delete offsets that pointed to bytes before the mark */
                        kh_del(pointer_hashmap, b->offsets, offset_key);
                    } else {
                        /* There's at least one offset beyond the mark, so the offsets are active and should be considered
                         * when locating pointers. We need to shift the offset back by the width of the mark */
                        offsets_active = true;
                        kh_value(b->offsets, offset_key) = offset - b->mark;
                    }
            });
        } else {
            /* the buffer is now empty, so we don't need to keep offsets */
            kh_clear(pointer_hashmap, b->offsets);
        }

        b->mark = 0;
        b->used = used;
        b->offsets_active = offsets_active;
    }
}

oct_buffer* buffer_init(size_t max_size) {
    buffer* b = malloc(sizeof(buffer));
    b->max_size = max_size;
    b->size = 0;
    b->used = 0;
    b->mark = 0;
    b->used_before = 0;
    b->current = NULL;
    b->offsets = kh_init(pointer_hashmap);
    b->offsets_active = false;
    return b;
}

void buffer_chunk_init(oct_buffer* buf, buffer_chunk* chunk) {
    buffer *b = (buffer *) buf;
    chunk->size = b->size ? b->size - b->used : 0;
    chunk->buffer = b->current + b->used;
}

bool buffer_alloc(oct_buffer* buf, size_t requested_size) {
    buffer* b = (buffer*) buf;
    bool ret = true;
    void* previous = NULL;

    size_t requested_size_capped = MIN(b->max_size, requested_size);

    if (!b->current) {
        b->current = malloc(requested_size_capped);
        if (!b->current) {
            b->size = 0;
            errno = ENOMEM;
            ret = false;
        } else {
            b->size = requested_size_capped;
        }
    } else if (b->used * 2 < b->size) {
        /* ignoring allocation size unless we're above 50% usage */
    } else if (b->size + requested_size_capped <= b->max_size) {
        /* time to reallocate memory and re-point anything using the buffer */
        previous = b->current;

        b->current = realloc(b->current, b->size + requested_size_capped);
        b->size += requested_size_capped;

        if (!b->current) {
            b->size = 0;
            errno = ENOMEM;
            ret = false;
        } else if (b->current != previous) {
            b->offsets_active = true;
        }
    } else {
        /* maximum request size exceeded */
        errno = ERANGE;
        b->size = 0;
        ret = false;
    }

    return ret;
}

void buffer_print(oct_buffer* buf) {
    buffer* b = (buffer*) buf;

    printf("Buffer: current=%u; size=%u; used=%u\n", b->current, b->size, b->used);
    printf("    0\t");
    for (int i = 0; i < b->used; i++) {
        if (((char*) b->current)[i] == '\n') {
            printf("\\n");
        } else if (((char*) b->current)[i] == '\r') {
            printf("\\r");
        } else {
            printf("%c", ((char*) b->current)[i]);
        }

        if ((i + 1) % 10 == 0) {
            printf("\n%5d\t", (i + 1) / 10);
        } else {
            printf("\t");
        }
    }
    printf("\n");

    void* pointer;
    int offset;
    kh_foreach(b->offsets, pointer, offset, {
            printf("\tPointer %u -> offset=%u\n", pointer, offset);
    });
    printf("----\n");
}

void buffer_pin(oct_buffer* buf, void* key, void* pointer) {
    buffer* b = (buffer*) buf;

    khiter_t offset_key = kh_get(pointer_hashmap, b->offsets, key);

    int offset = pointer - b->current;
    int ret;

    int is_missing = (offset_key == kh_end(b->offsets));
    if (is_missing) {
        offset_key = kh_put(pointer_hashmap, b->offsets, key, &ret);
    }

    kh_value(b->offsets, offset_key) = offset;
}

void buffer_reassign_pin(oct_buffer* buf, void* old_key, void* new_key) {
    buffer* b = (buffer*) buf;

    khiter_t old_offset_key = kh_get(pointer_hashmap, b->offsets, old_key);

    int offset;
    int ret;

    int is_missing = (old_offset_key == kh_end(b->offsets));
    if (!is_missing) {
        offset = kh_val(b->offsets, old_offset_key);

        khiter_t new_offset_key = kh_put(pointer_hashmap, b->offsets, new_key, &ret);
        kh_value(b->offsets, new_offset_key) = offset;
        old_offset_key = kh_get(pointer_hashmap, b->offsets, old_key);
        kh_del(pointer_hashmap, b->offsets, old_offset_key);
    }
}

void* buffer_locate(oct_buffer* buf, void* key, void* default_pointer) {
    buffer* b = (buffer*) buf;
    void* location = default_pointer;
    khiter_t offset_key = kh_get(pointer_hashmap, b->offsets, key);

    int offset, is_missing;

    if (b->offsets_active) {
        is_missing = (offset_key == kh_end(b->offsets));
        if (!is_missing) {
            offset = kh_value(b->offsets, offset_key);
            location = b->current + offset;
        }
    }

    return location;
}

void buffer_destroy(oct_buffer* buf) {
    buffer* b = (buffer*) buf;
    kh_destroy(pointer_hashmap, b->offsets);
    free(b->current);
    free(b);
}
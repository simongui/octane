#pragma once
#include <stdbool.h>
#include "khash.h"

typedef struct {
    void* buffer;
    size_t size;
} buffer_chunk;

typedef void* oct_buffer;

/**
 * Initializes a new buffer. The caller is responsible for calling http_request_buffer_destroy to free up memory.
 */
oct_buffer* buffer_init(size_t max_size);

/**
 * Signals that "size" bytes from the buffer are now in use.
 */
void buffer_consume(oct_buffer* buffer, size_t size);

/**
 * Marks all buffer chunks up to the last one allocated (exclusive) for removal when
 * http_request_buffer_sweep gets called.
 */
void buffer_mark(oct_buffer* buffer);

/**
 * Sweeps all buffer chunks up to the mark. Data for the last chunk is copied to the beginning of the buffer
 * and the offsets are updates accordingly.
 */
void buffer_sweep(oct_buffer* buffer);

/**
 * Prints the buffer.
 */
void buffer_print(oct_buffer* buffer);

/**
 * Ensures that there's will be a sizable chunk available when the it's requested via buffer_chunk.
 * This will ensure that the requested size doesn't exceed the maximum buffer size and will try to
 * re-use the existing underlying buffers if there's sufficient space still to be used.
 * Therefore, "requested_size" is just a hint, and not necessarily the size that will be allocated.
 *
 * Returns true if the allocation succeeds, false otherwise (errno is set accordingly).
 */
bool buffer_alloc(oct_buffer* buf, size_t requested_size);

/**
 * Returns a new buffer chunk to be used by request reader.
 */
void buffer_chunk_init(oct_buffer* buffer, buffer_chunk* chunk);

/**
 * Informs the buffer manager that *pointer is a memory region of interest and that will have to be made available to
 * the caller eventually, when http_request_buffer_locate is called. The pin is given a key by the caller so it can be
 * retrieved later. Pins will be overwritten if the same key is used multiple times.
 */
void buffer_pin(oct_buffer* buffer, void* key, void* pointer);

/**
 * Allows the caller to assign a new key to a pin.
 */
void buffer_reassign_pin(oct_buffer* buffer, void* old_key, void* new_key);

/**
 * Returns the pointer to the memory region associated with a pin. If there isn't a pin with that key, then
 * the value of "default_pointer" is returned.
 *
 * This call guarantees that the memory region returned is contiguous, i.e. even if multiple chunks non contiguous
 * chunks were used before, the pointer returned by this function points to a memory region that can be read
 * sequentially. It's the responsibility of the caller to know how long that region is, by keeping track of its
 * length as it's being read in.
 */
void* buffer_locate(oct_buffer* buffer, void* key, void* default_pointer);

/**
 * Destroys the buffer and any underlying buffer chunks.
 */
void buffer_destroy(oct_buffer* buffer);
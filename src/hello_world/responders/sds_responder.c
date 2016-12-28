#include <uv.h>
#include <sds.h>
#include "sds_responder.h"
#include "../connection.h"
#include "../write_batch.h"

void create_plaintext_response_sds(write_batch* batch) {
    //sds response_buffer = sdsnew("HTTP/1.1 200 OK\r\nServer: octane/master\r\nContent-Type: text/plain\r\nContent-Length: 15\r\n");
    //response_buffer = sdscat(response_buffer, "Date: Mon Dec 12 00:00:00 2016\r\n");
    //response_buffer = sdscat(response_buffer, "\r\nHello, World!\n\n");

    sds response_buffer = sdsnew("HTTP/1.1 200 OK\r\n");
    response_buffer = sdscat(response_buffer, "Server: octane\r\n");
    response_buffer = sdscat(response_buffer, "Content-Type: text/plain\r\n");
    response_buffer = sdscat(response_buffer, "Content-Length: 14\r\n");
    response_buffer = sdscatprintf(response_buffer, "Date: %s", current_time);
    response_buffer = sdscat(response_buffer, "\r\nHello, World!\n");

    batch->buffers[batch->number_of_used_buffers].base = response_buffer;
    batch->buffers[batch->number_of_used_buffers].len = sdslen(response_buffer);
    batch->number_of_used_buffers++;
}

void create_json_response_sds(write_batch* batch) {
    sds response_buffer = sdsnew("HTTP/1.1 200 OK\r\n");
    response_buffer = sdscat(response_buffer, "Server: octane\r\n");
    response_buffer = sdscat(response_buffer, "Content-Type: application/json\r\n");
    response_buffer = sdscat(response_buffer, "Content-Length: 28\r\n");
    response_buffer = sdscatprintf(response_buffer, "Date: %s", current_time);
    response_buffer = sdscat(response_buffer, "\r\n{\"message\":\"Hello, World!\"}\n");

    batch->buffers[batch->number_of_used_buffers].base = response_buffer;
    batch->buffers[batch->number_of_used_buffers].len = sdslen(response_buffer);
    batch->number_of_used_buffers++;
}

void stream_on_read_sds(connection* conn, size_t requests, uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    uv_write_t *write_req = create_write_with_batch(requests);
    write_batch* batch = write_req->data;

    if (buf->base[5] == 'p') {
        // Plaintext route.
        for (int i=0; i<requests; i++) {
            create_plaintext_response_sds(batch);
        }
    } else if (buf->base[5] == 'j') {
        // JSON route.
        for (int i=0; i<requests; i++) {
            create_json_response_sds(batch);
        }
    }

    if (uv_is_writable(stream)) {
        // TODO: Use the return values from uv_write()
        int rc = uv_write(write_req, stream, batch->buffers, batch->number_of_used_buffers, after_write_sds);
    } else {
        // TODO: Handle closing the stream.
    }
}

void after_write_sds(uv_write_t* req, int status) {
    write_batch* batch = get_write_batch(req);
    for (int i=0; i<batch->number_of_used_buffers; i++) {
        sds buffer = batch->buffers[i].base;
        sdsfree(buffer);
    }
    free(req);
}
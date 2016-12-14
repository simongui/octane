#include <stdlib.h>
#include <uv.h>
#include "nobuffer_responder.h"
#include "connection.h"

uv_buf_t* create_response_nobuffer(uv_write_t* write_req) {
    uv_buf_t* buffers = malloc(sizeof(uv_buf_t) * 7);
    buffers[0].base = "HTTP/1.1 200 OK\r\n";
    buffers[0].len = 17;
    buffers[1].base = "Server: libchannels/master\r\n";
    buffers[1].len = 28;
    buffers[2].base = "Content-Type: text/plain\r\n";
    buffers[2].len = 26;
    buffers[3].base = "Content-Length: 15\r\n";
    buffers[3].len = 20;
    buffers[4].base = "Date: Mon Dec 12 00:00:00 2016\r\n";
    buffers[4].len = 32;
    buffers[5].base = "\r\n";
    buffers[5].len = 2;
    buffers[6].base = "Hello, World!\n\n";
    buffers[6].len = 15;

    return buffers;
}

void stream_on_read_nobuffer(connection* conn, uv_write_t *write_req, size_t requests, uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf, uv_write_cb cb) {
    uv_buf_t bufs[requests * 7];

    int buffer_index = 0;
    for (int i=0; i<requests; i++) {
        uv_buf_t* request_buffers = create_response_nobuffer(write_req);
        for (int x=0; x<7; x++) {
            uv_buf_t request_buffer = *(request_buffers + x);
            bufs[buffer_index] = request_buffer;
            buffer_index++;
        }
    }

    if (uv_is_writable(stream)) {
        // TODO: Use the return values from uv_write()
        int rc = uv_write(write_req, stream, bufs, buffer_index, cb);
    } else {
        // TODO: Handle closing the stream.
    }
}

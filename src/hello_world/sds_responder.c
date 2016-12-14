#include <uv.h>
#include <sds.h>
#include "sds_responder.h"
#include "connection.h"

uv_buf_t* create_response_sds(uv_write_t* write_req) {
    uv_buf_t* resbuf = (uv_buf_t *) (write_req + 1);

    sds response_buffer = sdsempty();
    //response_buffer = sdscat(response_buffer, "HTTP/1.1 200 OK\r\n");
    //response_buffer = sdscat(response_buffer, "Server: libchannels/master\r\n");
    //response_buffer = sdscat(response_buffer, "Date: Mon Dec 12 00:00:00 2016\r\n");
    //response_buffer = sdscat(response_buffer, "Content-Type: text/plain\r\n");
    //response_buffer = sdscat(response_buffer, "Content-Length: 15\r\n");
    //response_buffer = sdscat(response_buffer, "\r\n");
    //response_buffer = sdscat(response_buffer, "Hello, World!\n\n");

    response_buffer = sdscat(response_buffer, "HTTP/1.1 200 OK\r\nServer: libchannels/master\r\nContent-Type: text/plain\r\nContent-Length: 15\r\n");
    response_buffer = sdscat(response_buffer, "Date: Mon Dec 12 00:00:00 2016\r\n");
    response_buffer = sdscat(response_buffer, "\r\nHello, World!\n\n");

    resbuf->base = response_buffer;
    resbuf->len = sdslen(response_buffer);
    return resbuf;}

void stream_on_read_sds(connection* conn, uv_write_t *write_req, size_t requests, uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf, uv_write_cb cb) {
    uv_buf_t bufs[requests];
    for (int i=0; i<requests; i++) {
        bufs[i] = *create_response_sds(write_req);
    }

    if (uv_is_writable(stream)) {
        // TODO: Use the return values from uv_write()
        int rc = uv_write(write_req, stream, bufs, requests, cb);
    } else {
        // TODO: Handle closing the stream.
    }
}

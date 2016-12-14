#include <uv.h>
#include "bstrlib_responder.h"
#include "bstrlib.h"
#include "connection.h"

uv_buf_t* create_response_bstrlib(uv_write_t* write_req) {
    uv_buf_t* resbuf = (uv_buf_t *) (write_req + 1);

    bstring response_buffer;
    response_buffer = bfromcstr("HTTP/1.1 200 OK\r\n");
    bconcat(response_buffer, bfromcstr("Server: libchannels/master\r\n"));
    bconcat(response_buffer, bfromcstr("Date: Mon Dec 12 00:00:00 2016\r\n"));
    bconcat(response_buffer, bfromcstr("Content-Type: text/plain\r\n"));
    bconcat(response_buffer, bfromcstr("Content-Length: 15\r\n"));
    bconcat(response_buffer, bfromcstr("\r\n"));
    bconcat(response_buffer, bfromcstr("Hello, World!\n\n"));

    resbuf->base = response_buffer->data;
    resbuf->len = response_buffer->slen;
    return resbuf;
}

void stream_on_read_bstrlib(connection* conn, uv_write_t *write_req, size_t requests, uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf, uv_write_cb cb) {
    uv_buf_t bufs[requests];
    for (int i=0; i<requests; i++) {
        bufs[i] = *create_response_bstrlib(write_req);
    }

    if (uv_is_writable(stream)) {
        // TODO: Use the return values from uv_write()
        int rc = uv_write(write_req, stream, bufs, requests, cb);
    } else {
        // TODO: Handle closing the stream.
    }
}

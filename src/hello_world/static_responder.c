#include <uv.h>
#include "static_responder.h"
#include "connection.h"

uv_buf_t* create_response_static(uv_write_t* write_req) {
    uv_buf_t* resbuf = (uv_buf_t *) (write_req + 1);
    resbuf->base = RESPONSE;
    resbuf->len = sizeof(RESPONSE);
    return resbuf;
}

void stream_on_read_static(connection* conn, uv_write_t *write_req, size_t requests, uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf, uv_write_cb cb) {
    uv_buf_t bufs[requests];
    for (int i=0; i<requests; i++) {
        bufs[i] = *create_response_static(write_req);
        //bufs[i] = *create_response_bstrlib(write_req);
        //bufs[i] = *create_response_sds(write_req);
    }

    if (uv_is_writable(stream)) {
        // TODO: Use the return values from uv_write()
        int rc = uv_write(write_req, stream, bufs, requests, cb);
    } else {
        // TODO: Handle closing the stream.
    }
}

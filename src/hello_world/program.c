#include <stdio.h>
#include <uv.h>
#include <stdlib.h>
#include <channels.h>
#include "common.h"
#include "connection.h"
#include "static_responder.h"
#include "sds_responder.h"
#include "nobuffer_responder.h"

typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

void stream_on_connect(uv_stream_t* stream, int status);
void stream_on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf);
void stream_on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);

void (*stream_on_read_func)(connection* conn, size_t requests, uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

int main(int args, char **argsv) {
    stream_on_read_func = &stream_on_read_static;
    //stream_on_read_func = &stream_on_read_sds;
    //stream_on_read_func = &stream_on_read_nobuffer;

    uv_async_t* service_handle = 0;
    uv_loop_t* loop = uv_default_loop();

    uv_multi_listen("0.0.0.0", 8000, false, 20, DISPATCH_TYPE_REUSEPORT, loop, 128, stream_on_connect, stream_on_alloc);
}

void stream_on_connect(uv_stream_t* server_stream, int status) {
    /* TODO: Use the return values from uv_accept() and uv_read_start() */
    int rc = 0;

    connection* conn = create_connection();
    rc = uv_tcp_init(server_stream->loop, (uv_stream_t*)&conn->stream);
    conn->bytes_remaining = 0;
    conn->request_length = 0;
    conn->stream.data = conn;

    rc = uv_accept(server_stream, (uv_stream_t*)&conn->stream);
    uv_read_start((uv_stream_t*)&conn->stream, stream_on_alloc, stream_on_read);
}

void stream_on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf) {
    char* buffer;
    if(!(buffer = malloc(suggested_size))){
        memory_error("Unable to allocate buffer of size %d", suggested_size);
    }
    *buf = uv_buf_init(buffer, suggested_size);
}

void stream_on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    connection* conn = stream->data;

    if (nread < 0) {
        return;
    } else if (nread == 0) {
        return;
    } else if (nread > 0) {
        if (conn->request_length == 0) {
            // We need to seek the first request to find out how many characters each request is.
            for (int i = 1; i < nread; i++) {
                if (buf->base[i] == '\r' && buf->base[i - 1] == '\n') {
                    conn->request_length = i + 2;
                    break;
                }
            }
        }

        ssize_t requests = (nread + conn->bytes_remaining) / conn->request_length;
        conn->bytes_remaining = conn->bytes_remaining + (nread % conn->request_length);

        stream_on_read_func(conn, requests, stream, nread, buf);
        free(buf->base);
    }
}
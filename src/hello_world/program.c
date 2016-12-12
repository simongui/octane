#include <stdio.h>
#include <uv.h>
#include <stdlib.h>
#include "channels.h"

#define RESPONSE \
    "HTTP/1.1 200 OK\r\n" \
    "Server: libchannels/master\r\n" \
    "Date: Mon Dec 12 00:00:00 2016\r\n" \
    "Content-Type: text/plain\r\n" \
    "Content-Length: 15\r\n" \
    "\r\n" \
    "Hello, World!\n"

typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

void stream_on_connect(uv_stream_t* stream, int status);
void stream_on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf);
void stream_on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);
void after_write(uv_write_t* req, int status);

static int request_length = 0;

static void printchar(unsigned char theChar) {
    switch (theChar) {
        case '\n':
            printf("\\n\n");
            break;
        case '\r':
            printf("\\r");
            break;
        case '\t':
            printf("\\t");
            break;
        default:
            if ((theChar < 0x20) || (theChar > 0x7f)) {
                printf("\\%03o", (unsigned char)theChar);
            } else {
                printf("%c", theChar);
            }
            break;
    }
}

int main(int args, char **argsv) {
    uv_async_t* service_handle = 0;
    uv_loop_t* loop = uv_default_loop();

    uv_multi_listen("0.0.0.0", 8000, false, 4, DISPATCH_TYPE_REUSEPORT, loop, 128, stream_on_connect, stream_on_alloc);
}

void stream_on_connect(uv_stream_t* server_stream, int status) {
    /* TODO: Use the return values from uv_accept() and uv_read_start() */
    int rc = 0;
    connection* conn = malloc(sizeof(connection));

    rc = uv_tcp_init(server_stream->loop, (uv_stream_t*)&conn->stream);
    conn->bytes_remaining = 0;
    conn->stream.data = conn;

    rc = uv_accept(server_stream, (uv_stream_t*)&conn->stream);
    uv_read_start((uv_stream_t*)&conn->stream, stream_on_alloc, stream_on_read);
}

void stream_on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf) {
    char* buffer = malloc(suggested_size);
    *buf = uv_buf_init(buffer, suggested_size);
}

void stream_on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    connection* conn = stream->data;

    if (nread < 0) {
        return;
    } else if (nread == 0) {
        return;
    } else if (nread > 0) {
        if (request_length == 0) {
            // We need to seek the first request to find out how many characters each request is.
            for (int i=1; i<nread; i++) {
                if (buf->base[i] == '\r' && buf->base[i-1] == '\n') {
                    request_length = i;
                    //printf("LENGTH: %d\n", request_length);
                    break;
                }
            }
        }

        ssize_t requests = (nread + conn->bytes_remaining) / request_length;
        conn->bytes_remaining = conn->bytes_remaining + (nread % request_length);
        uv_buf_t bufs[requests];
        uv_write_t *write_req = (uv_write_t *) malloc(sizeof(*write_req) + sizeof(uv_buf_t));

        for (int i=0; i<requests; i++) {
            uv_buf_t* resbuf = (uv_buf_t *) (write_req + 1);

            resbuf->base = RESPONSE;
            resbuf->len = sizeof(RESPONSE);
            bufs[i] = *resbuf;
        }

        if (uv_is_writable(stream)) {
            //printf("%d\t%d\n", nread, requests);
            // TODO: Use the return values from uv_write()
            int rc = uv_write(write_req, stream, bufs, requests, after_write);
            //int rc = uv_try_write(stream, bufs, requests);
        } else {
            // TODO: Handle closing the stream.
        }

        //for (int i=0; i<requests; i++) {
        //    if (uv_is_writable(stream)) {
        //        // TODO: Use the return values from uv_write()
        //        uv_buf_t *resbuf = (uv_buf_t *) (write_req + 1);
        //        resbuf->base = RESPONSE;
        //        resbuf->len = sizeof(RESPONSE);
        //        int rc = uv_write(write_req, stream, resbuf, 1, after_write);
        //    } else {
        //        // TODO: Handle closing the stream.
        //    }
        //    //free(buf->base);
        //}
    }
}

void after_write(uv_write_t* req, int status) {
    //uv_buf_t *resbuf = (uv_buf_t *)(req+1);
    //printf("%d\n", resbuf->len);

    //free(resbuf->base);
    //free(req);

//   write_req_t* wr;

//   /* Free the read/write buffer and the request */
//   wr = (write_req_t*) req;
//   free(wr->buf.base);
//   free(wr);

//   if (status == 0)
//     return;

//   fprintf(stderr,
//           "uv_write error: %s - %s\n",
//           uv_err_name(status),
//           uv_strerror(status));
}

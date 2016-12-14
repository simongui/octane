#pragma once

#include <uv.h>
#include "connection.h"

#define RESPONSE \
    "HTTP/1.1 200 OK\r\n" \
    "Server: libchannels/master\r\n" \
    "Date: Mon Dec 12 00:00:00 2016\r\n" \
    "Content-Type: text/plain\r\n" \
    "Content-Length: 15\r\n" \
    "\r\n" \
    "Hello, World!\n"

uv_buf_t* create_response_static(uv_write_t* write_req);
void stream_on_read_static(connection* conn, uv_write_t *write_req, size_t requests, uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf, uv_write_cb cb);
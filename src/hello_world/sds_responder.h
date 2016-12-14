#pragma once

#include <uv.h>
#include "connection.h"

uv_buf_t* create_response_sds(uv_write_t* write_req);
void stream_on_read_sds(connection* conn, uv_write_t *write_req, size_t requests, uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf, uv_write_cb cb);
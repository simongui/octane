#pragma once
#include <stdbool.h>
#include <uv.h>
#include "octane.h"

void uv_stream_on_connect(uv_stream_t* server_stream, int status);
void uv_stream_on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf);
void uv_stream_on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
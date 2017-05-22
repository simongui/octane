#pragma once
#include <uv.h>
#include "octane.h"
#include "buffer.h"

/*
 * http_connection
 */
typedef struct http_connection {
    uv_tcp_t stream;
    void* listener;
    enum connection_state state;
    bool keep_alive;
    size_t current_parsed_position;
    size_t current_buffer_position;
    oct_buffer* buffer;
} http_connection;

http_connection* new_http_connection();
void free_http_connection(http_connection* connection);

#pragma once
#include <uv.h>
#include "octane.h"

/*
 * http_connection
 */
typedef struct http_connection {
    uv_tcp_t stream;
    void* listener;
    enum connection_state state;
    bool keep_alive;
    ssize_t partial_request_location;
} http_connection;

http_connection* new_http_connection();
void free_http_connection(http_connection* connection);

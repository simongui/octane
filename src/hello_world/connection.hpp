#pragma once

#include <stdbool.h>
#include "octane.h"

typedef struct
{
    uv_tcp_t stream;
    enum connection_state state;
    void* data;
    bool keep_alive;
    size_t bytes_remaining;
    size_t request_length;
} connection;

typedef enum {
    OK,
    SIZE_EXCEEDED,
    BAD_REQUEST,
    INTERNAL_ERROR
} request_state;

connection* create_connection();
void free_connection(connection* conn);
#pragma once

typedef struct
{
    uv_tcp_t stream;
    enum {OPEN, CLOSING, CLOSED} state;
    void* data;
    int bytes_remaining;
    int request_length;
} connection;

connection* create_connection();

#include <stdlib.h>
#include "octane.h"
#include "http_listener.h"
#include "common.h"
#include "http_connection.h"

http_listener* new_http_listener() {
    http_listener* listener;
    size_t size = sizeof(http_listener);
    if(!(listener = (http_listener*)malloc(size))){
        memory_error("Unable to allocate buffer of size %d", size);
    }
    return listener;
}

http_listener* get_listener_from_connection(http_connection* connection) {
    http_listener* listener = connection->listener;
    return listener;
}

void begin_listening(http_listener* listener, const char* address, int port, bool tcp_no_delay,
                     unsigned int threads, unsigned int backlog, oct_connection_cb connection_cb) {
    listener->connection_cb = connection_cb;
    uv_loop_t* loop = uv_default_loop();

    uv_multi_listen("0.0.0.0", 8000, false, 40, DISPATCH_TYPE_REUSEPORT, loop, backlog, listener,
                    uv_stream_on_connect);
}

void uv_stream_on_connect(uv_stream_t* server_stream, int status) {
    /* TODO: Use the return values from uv_accept() and uv_read_start() */
    int rc = 0;

    http_listener* listener = (http_listener*)server_stream->data;
    printf("%p\n", listener);

    http_connection* connection = new_http_connection();
    connection->listener = listener;
    rc = uv_tcp_init(server_stream->loop, (uv_tcp_t*)&connection->stream);
    connection->stream.data = connection;

    rc = uv_accept(server_stream, (uv_stream_t*)&connection->stream);

    //listener->connection_cb(connection, server_stream, status);

    uv_read_start((uv_stream_t*)&connection->stream, uv_stream_on_alloc, uv_stream_on_read);
}

void uv_stream_on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf) {
    char* buffer;
    if(!(buffer = (char*)malloc(suggested_size))){
        memory_error("Unable to allocate buffer of size %d", suggested_size);
    }
    *buf = uv_buf_init(buffer, suggested_size);
}

void uv_stream_on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    http_connection* connection = stream->data;
    http_listener* listener = get_listener_from_connection(connection);
    listener->read_cb(connection, stream, nread, buf);
}
#include <stdlib.h>
#include <uv.h>
#include "http_connection.h"
#include "common.h"

http_connection* new_http_connection()
{
    http_connection* connection;
    size_t size = sizeof(http_connection);
    if(!(connection = (http_connection*)malloc(size))){
        memory_error("Unable to allocate buffer of size %d", size);
    }
    //connection->buffer = malloc(sizeof(uv_buf_t));
    connection->current_buffer_position = 0;
    connection->current_parsed_position = 0;
    //connection->current_parser_status = 0;
    return connection;
}

void free_http_connection(http_connection* connection) {
    free(connection);
}

int http_connection_is_writable(http_connection* connection) {
    return uv_is_writable((uv_stream_t*)&connection->stream);
}

void http_connection_after_write(uv_write_t* req, int status) {
    write_batch* batch = get_write_batch_from_write_req(req);
    for (int i=0; i<batch->number_of_used_buffers; i++) {
        sds buffer = batch->buffers[i].base;
        sdsfree(buffer);
    }
    free(req);
}

int http_connection_write(http_connection* connection, write_batch* batch) {
    int rc = uv_write(batch->write_req, (uv_stream_t*)&connection->stream, batch->buffers, batch->number_of_used_buffers, http_connection_after_write);
}
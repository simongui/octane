#include <stdlib.h>
#include "http_connection.h"
#include "common.h"

http_connection* new_http_connection()
{
    http_connection* connection;
    size_t size = sizeof(http_connection);
    if(!(connection = (http_connection*)malloc(size))){
        memory_error("Unable to allocate buffer of size %d", size);
    }
    return connection;
}

void free_http_connection(http_connection* connection) {
    free(connection);
}

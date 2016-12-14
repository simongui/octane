#include <stdlib.h>
#include <channels.h>
#include "connection.h"

connection* create_connection()
{
    //connection* conn = malloc(sizeof(connection));
    connection* conn = calloc(1, sizeof(connection));
    conn->bytes_remaining = 0;
    conn->request_length = 0;
    return conn;
}

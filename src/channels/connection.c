#include "channels.h"
#include "connection.h"

connection* create_connection()
{
    connection* conn = calloc(1, sizeof(connection));
    return conn;
}

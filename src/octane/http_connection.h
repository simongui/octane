#pragma once
#include <uv.h>
#include "octane.h"

http_connection* new_http_connection();
void free_http_connection(http_connection* connection);

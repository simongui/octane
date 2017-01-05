#include <stdlib.h>
#include "octane.h"
#include "common.h"

http_request* new_http_request() {
    http_request* request;
    size_t size = sizeof(http_request);
    if(!(request = (http_request*)malloc(size))){
        memory_error("Unable to allocate buffer of size %d", size);
    }

    request->method = sdsempty();
    request->path = sdsempty();
    request->version = 0;
    return request;

}
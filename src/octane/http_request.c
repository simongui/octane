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

void free_http_request(http_request* request) {
    sdsfree(request->method);
    sdsfree(request->path);
    free(request);
}

void free_http_requests(http_request** requests, int number_of_requests) {
    for (int i=0; i<number_of_requests; i++) {
        free_http_request(requests[i]);
    }
    free(requests);
}
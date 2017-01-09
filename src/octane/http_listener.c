#include <stdlib.h>
#include "octane.h"
#include "http_listener.h"
#include "common.h"
#include "http_connection.h"
#include "picohttpparser.h"

void parse_http_stream(http_connection* connection, uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

http_listener* new_http_listener() {

    //uv_buf_t buffer;
    //buffer.base = "GET /plaintext HTTP/1.1\r\n"
    //        "Host: localhost:8000\r\n"
    //        "User-Agent: curl/7.49.1\r\n"
    //        "Accept: */*\r\n" \
    //        "\r\n";

    //buffer.base = "GET /plaintext HTTP/1.1\r\n"
    //        "Host: localhost:8000\r\n"
    //        "User-Agent: curl/7.49.1\r\n"
    //        "Accept: */*\r\n" \
    //        "\r\n" \
    //        "GET /json HTTP/1.1\r\n"
    //        "Host: localhost:8000\r\n"
    //        "User-Agent: curl/7.49.1\r\n"
    //        "Accept: */*\r\n" \
    //        "\r\n";
    //
    //buffer.len = strlen(buffer.base);
    //parse_http_stream(NULL, NULL, buffer.len, &buffer);

    http_listener* listener;
    size_t size = sizeof(http_listener);
    if(!(listener = (http_listener*)malloc(size))){
        memory_error("Unable to allocate buffer of size %d", size);
    }
    listener->loop = uv_default_loop();
    return listener;
}

http_listener* get_listener_from_connection(http_connection* connection) {
    http_listener* listener = connection->listener;
    return listener;
}

void begin_listening(http_listener* listener, const char* address, int port, bool tcp_no_delay,
                     unsigned int threads, unsigned int backlog, oct_connection_cb connection_cb,
                     oct_alloc_cb alloc_cb, oct_read_cb read_cb, oct_request_cb request_cb) {
    listener->connection_cb = connection_cb;
    listener->alloc_cb = alloc_cb;
    listener->read_cb = read_cb;
    listener->request_cb = request_cb;

    uv_multi_listen("0.0.0.0", 8000, false, 40, DISPATCH_TYPE_REUSEPORT, listener->loop, backlog, listener,
                    uv_stream_on_connect);
}

void uv_stream_on_connect(uv_stream_t* stream, int status) {
    // TODO: Use the return values from uv_accept() and uv_read_start()
    int rc = 0;

    http_listener* listener = (http_listener*)stream->data;
    http_connection* connection = new_http_connection();
    connection->listener = listener;

    rc = uv_tcp_init(stream->loop, (uv_tcp_t*)&connection->stream);
    connection->stream.data = connection;

    rc = uv_accept(stream, (uv_stream_t*)&connection->stream);

    if (listener->connection_cb != NULL) {
        listener->connection_cb(connection, stream, status);
    }

    // Start reading from the socket.
    uv_read_start((uv_stream_t*)&connection->stream, uv_stream_on_alloc, uv_stream_on_read);
}

void uv_stream_on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf) {
    http_connection* connection = (http_connection*)client->data;
    http_listener* listener = get_listener_from_connection(connection);
    if (listener->alloc_cb != NULL) {
        listener->alloc_cb(listener, client, suggested_size, buf);
    }
    if (listener->alloc_cb == NULL || buf->len == 0) {
        // No allocation function defined or no allocation happened so we default to allocating the size requested.
        char* buffer;
        if(!(buffer = (char*)malloc(suggested_size))){
            memory_error("Unable to allocate buffer of size %d", suggested_size);
        }
        *buf = uv_buf_init(buffer, suggested_size);
    }
}

void uv_stream_on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    http_connection* connection = stream->data;
    http_listener* listener = get_listener_from_connection(connection);

    if (listener->read_cb != NULL) {
        listener->read_cb(connection, stream, nread, buf);
    } else {
        parse_http_stream(connection, stream, nread, buf);
    }
}

void parse_http_stream(http_connection* connection, uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    http_listener* listener = get_listener_from_connection(connection);

    if (nread > 0) {
        http_request* requests = malloc(sizeof(http_request*) * 256);
        int number_of_requests = 0;

        size_t prevbuflen = 0;

        while (prevbuflen < nread) {
            char* method;
            char* path;
            int pret;
            int minor_version;
            struct phr_header headers[100];
            size_t method_len;
            size_t path_len;
            size_t num_headers;

            // Parse the request.
            num_headers = sizeof(headers) / sizeof(headers[0]);
            pret = phr_parse_request(&buf->base[prevbuflen], nread - prevbuflen, &method,
                                     &method_len, &path, &path_len, &minor_version, headers,
                                     &num_headers, 0);

            //printf("request is %d bytes long\n", pret);
            //printf("method is %.*s\n", (int) method_len, method);
            //printf("path is %.*s\n", (int) path_len, path);
            //printf("HTTP version is 1.%d\n", minor_version);
            //printf("headers:\n");
            //for (int i = 0; i != num_headers; ++i) {
            //    printf("%.*s: %.*s\n", (int) headers[i].name_len, headers[i].name,
            //           (int) headers[i].value_len, headers[i].value);
            //}

            if (pret > 0) {
                // Successfully parsed the HTTP request.
                prevbuflen += pret;

                http_request* request = new_http_request();
                request->method = sdscatlen(request->method, method, method_len);
                request->path = sdscatlen(request->path, path, path_len);
                request->version = minor_version;
                requests[number_of_requests] = *request;
                number_of_requests++;
            }

            //if (pret > 0)
            //    break; /* successfully parsed the request */
            //else if (pret == -1)
            //    return ParseError;
            ///* request is incomplete, continue the loop */
            //assert(pret == -2);
            //if (buflen == sizeof(buf))
            //    return RequestIsTooLongError;
        }

        if (number_of_requests > 0) {
            if (listener != NULL && listener->request_cb != NULL) {
                listener->request_cb(connection, requests, number_of_requests);
            }
        }
    }
}
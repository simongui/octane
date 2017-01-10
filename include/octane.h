/*-------------------------------------------------------------------------
 *
 * Overview
 *      Primary include file for octane. Include this file to gain
 *      access to the libuv extended functionality.
 *
 *-------------------------------------------------------------------------
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <uv.h>
#include "sds.h"

#ifdef _WIN32
    /* Windows - set up dll import/export decorators. */
#ifdef BUILDING_OCTANE_SHARED
    /* Building shared library. */
#define OCTANE_EXTERN __declspec(dllexport)
#else
#ifdef USING_OCTANE_SHARED
    /* Using shared library. */
#define OCTANE_EXTERN __declspec(dllimport)
#else
    /* Building static library. */
#define OCTANE_EXTERN /* nothing */
#endif
#endif
    
#define OCTANE_CALLING_CONVENTION __cdecl
#else
    /* Building static library. */
#define OCTANE_EXTERN /* nothing */
#define OCTANE_CALLING_CONVENTION /* nothing */
#endif

/*
 * dispatch_type is an enum that represents which type of TCP connection
 * load balancing strategy will be used to distribute connection
 * processing across threads and CPU cores. A libuv event loop is created
 * per thread.
 *
 * It is recommended to run a thread per physical (not hyper-threaded) CPU core
 * to reduce context switches. libchain can perform very fast I/O on each
 * CPU core and CPU context switches impact performance of the library
 * significantly.
 *
 * DISPATCH_TYPE_IPC will distribute TCP connections to each event loop
 * over IPC named pipes in userland.
 *
 * DISPATCH_TYPE_REUSEPORT will distribute TCP connections using the
 * SP_REUSEPORT functionality in the Linux 3.9 kernel that load balanes
 * TCP connections between sockets. octane creates a socket per event
 * loop (per thread).
 *
 */
typedef enum dispatch_type
{
    DISPATCH_TYPE_IPC,
    DISPATCH_TYPE_REUSEPORT,
};

typedef enum connection_state
{
    CONNECTION_OPEN,
    CONNECTION_CLOSING,
    CONNECTION_CLOSED
};

/*
 * http_connection
 */
typedef struct http_connection {
    void* listener;
    enum connection_state state;
    bool keep_alive;
    uv_tcp_t stream;
    void* data;
} http_connection;

OCTANE_EXTERN typedef void (*oct_connection_cb)(http_connection* connection, uv_stream_t* server, int status);
OCTANE_EXTERN typedef void (*oct_alloc_cb)(http_connection* connection, uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
OCTANE_EXTERN typedef void (*oct_read_cb)(http_connection* connection, uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

/*
 * http_request
 */
typedef enum http_request_state {
    OK,
    SIZE_EXCEEDED,
    BAD_REQUEST,
    INTERNAL_ERROR
} http_request_state;

typedef struct http_header {
    sds key;
    sds value;
} http_header;

typedef struct http_request {
    sds method;
    sds path;
    int version;
    http_header headers[100];
}http_request;

OCTANE_EXTERN http_request* new_http_request();
OCTANE_EXTERN void free_http_request(http_request* request);
OCTANE_EXTERN void free_http_requests(http_request** requests, int number_of_requests);
OCTANE_EXTERN typedef void (*oct_request_cb)(http_connection* connection, http_request** requests, int number_of_requests);

/*
 * http_listener
 */
typedef struct http_listener {
    uv_loop_t* loop;
    oct_connection_cb connection_cb;
    oct_alloc_cb alloc_cb;
    oct_read_cb read_cb;
    oct_request_cb request_cb;
} http_listener;

OCTANE_EXTERN http_listener* new_http_listener();
OCTANE_EXTERN http_listener* get_listener_from_connection(http_connection* connection);
OCTANE_EXTERN void begin_listening(http_listener* listener, const char* address, int port,
                                   bool tcp_no_delay, unsigned int threads, unsigned int backlog,
                                   oct_connection_cb connection_cb, oct_alloc_cb alloc_cb, oct_read_cb read_cb,
                                   oct_request_cb request_cb);

/*
 * uv_multi_listen is similar to uv_listen except it allows creating event
 * loops and listeners for each thread defined in threads.
 *
 * Returns 0 for success and non-zero for errors.
 *
 * address
 *      The address to listen for TCP connections on.
 * port
 *      The port to listen for TCP connections on.
 * tcp_nodelay
 *      Whether TCP nodelay should be enabled.
 * dispatch_type
 *      Which strategy to use to load balance TCP connections.
 * loop
 *      Main application event loop.
 * connection_backlog
 *      How many connections can be pending in the backlog.
 * uv_connection_cb
 *      Callback that gets called when a new TCP connection connects.
 * uv_alloc_cb
 *      Callback that gets called when libuv requires memory to be allocated.
 */
OCTANE_EXTERN int uv_multi_listen(const char* address, int port, bool tcp_nodelay,
                                  unsigned int threads, enum dispatch_type dispatcher,
                                  uv_loop_t* loop, int connection_backlog, void* data,
                                  uv_connection_cb cb);

#ifdef __cplusplus
}
#endif
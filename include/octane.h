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
 * connection is a struct representing a TCP client connection to the server.
 */
//typedef struct
//{
//    uv_tcp_t stream;
//    enum {OPEN, CLOSING, CLOSED} state;
//    void* data;
//    int bytes_remaining;
//    int request_length;
//} connection;

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
OCTANE_EXTERN int uv_multi_listen(const char* address, int port, bool tcp_nodelay, unsigned int threads, enum dispatch_type dispatcher, uv_loop_t* loop, int connection_backlog, uv_connection_cb cb);

#ifdef __cplusplus
}
#endif
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <uv.h>

#ifdef _WIN32
    /* Windows - set up dll import/export decorators. */
#ifdef BUILDING_LIBCHANNELS_SHARED
    /* Building shared library. */
#define LIBCHANNELS_EXTERN __declspec(dllexport)
#else
#ifdef USING_LIBCHANNELS_SHARED
    /* Using shared library. */
#define LIBCHANNELS_EXTERN __declspec(dllimport)
#else
    /* Building static library. */
#define LIBCHANNELS_EXTERN /* nothing */
#endif
#endif
    
#define LIBCHANNELS_CALLING_CONVENTION __cdecl
#else
    /* Building static library. */
#define LIBCHANNELS_EXTERN /* nothing */
#define LIBCHANNELS_CALLING_CONVENTION /* nothing */
#endif

typedef enum dispatch_type
{
    DISPATCH_TYPE_IPC,
    DISPATCH_TYPE_REUSEPORT,
};

typedef struct
{
    uv_tcp_t stream;
    enum {OPEN, CLOSING, CLOSED} state;
    void* data;
    int bytes_remaining;
} connection;

LIBCHANNELS_EXTERN int uv_multi_listen(const char* address, int port, bool tcp_nodelay, unsigned int threads, enum dispatch_type dispatcher, uv_loop_t* loop, int backlog, uv_connection_cb cb, uv_alloc_cb alloc_cb);

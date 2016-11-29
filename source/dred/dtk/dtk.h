// Copyright (C) 2016 David Reid. See included LICENSE file.

#ifndef DTK_H
#define DTK_H

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4201)   // nonstandard extension used: nameless struct/union
#endif

// Platform/backend detection.
#ifdef _WIN32
    #define DTK_WIN32
    #if (!defined(WINAPI_FAMILY) || WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
        #define DTK_WIN32_DESKTOP
    #endif
#else
    #define DTK_POSIX
    #ifdef __linux__
        #define DTK_LINUX
    #endif
#endif

#ifdef DTK_LINUX
    #define DTK_GTK
    //#define DTK_X11
#endif

typedef enum
{
    dtk_platform_win32,
    dtk_platform_gtk,
    //dtk_platform_x11  // Not yet.
} dtk_platform;

// Sized types.
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef   signed char    dtk_int8;
typedef unsigned char    dtk_uint8;
typedef   signed short   dtk_int16;
typedef unsigned short   dtk_uint16;
typedef   signed int     dtk_int32;
typedef unsigned int     dtk_uint32;
typedef   signed __int64 dtk_int64;
typedef unsigned __int64 dtk_uint64;
#else
#include <stdint.h>
typedef int8_t           dtk_int8;
typedef uint8_t          dtk_uint8;
typedef int16_t          dtk_int16;
typedef uint16_t         dtk_uint16;
typedef int32_t          dtk_int32;
typedef uint32_t         dtk_uint32;
typedef int64_t          dtk_int64;
typedef uint64_t         dtk_uint64;
#endif
typedef dtk_int8         dtk_bool8;
typedef dtk_int32        dtk_bool32;
#define DTK_TRUE         1
#define DTK_FALSE        0

typedef void* dtk_handle;
typedef void* dtk_ptr;
typedef void (* dtk_proc)();

// Result codes.
typedef int dtk_result;
#define DTK_SUCCESS                  0
#define DTK_NO_EVENT                 1      // Not an error. Returned by dtk_next_event() to indicate there are no events currently in the queue.
#define DTK_ERROR                   -1
#define DTK_INVALID_ARGS            -2
#define DTK_OUT_OF_MEMORY           -3
#define DTK_NO_BACKEND              -4
#define DTK_FAILED_TO_INIT_BACKEND  -5
#define DTK_QUIT                    -1024   // Returned by dtk_next_event() when a quit message is received.

// Standard library stuff.
#include <string.h>
#include <stdlib.h>

#ifndef dtk_assert
#define dtk_assert(condition)   assert(condition)
#endif
#ifndef dtk_malloc
#define dtk_malloc(sz)          malloc(sz);
#endif
#ifndef dtk_calloc
#define dtk_calloc(c, sz)       calloc(c, sz)
#endif
#ifndef dtk_free
#define dtk_free(p)             free(p)
#endif
#ifndef dtk_zero_memory
#define dtk_zero_memory(p, sz)  memset((p), 0, (sz))
#endif
#define dtk_zero_object(p)      dtk_zero_memory((p), sizeof(*(p)))

#if defined(_MSC_VER)
#define DTK_INLINE static __inline
#else
#define DTK_INLINE static inline
#endif

typedef struct dtk_context dtk_context;
typedef struct dtk_control dtk_control;
typedef struct dtk_window dtk_window;

typedef struct
{
    dtk_int32 left;
    dtk_int32 top;
    dtk_int32 right;
    dtk_int32 bottom;
} dtk_rect;

// Event types.
typedef int dtk_event_type;
#define DTK_EVENT_NONE          0
#define DTK_EVENT_QUIT          1
#define DTK_EVENT_CLOSE         2
#define DTK_EVENT_PAINT         3
#define DTK_EVENT_SIZE          4
#define DTK_EVENT_MOVE          5
#define DTK_EVENT_MOUSE_MOVE    6

typedef struct
{
    dtk_event_type type;
    dtk_context* pTK;
    dtk_control* pControl;

    union
    {
        struct
        {
            int result;
        } quit;
        
        struct
        {
            int unused;
        } close;

        struct
        {
            dtk_rect rect;
        } paint;

        struct
        {
            dtk_uint32 width;
            dtk_uint32 height;
        } size;

        struct
        {
            dtk_int32 x;
            dtk_int32 y;
        } move;

        struct
        {
            dtk_int32 x;
            dtk_int32 y;
        } mouseMove;
    };
} dtk_event;

// The callback function to call when an event is received and needs handling. The return value controls whether or
// not the event should be propagated. Returning true will propagate the event, false will cancel the event. You will
// almost always want to return true.
typedef dtk_bool32 (* dtk_event_proc)(dtk_event* pEvent);

#include "dtk_string.h"
#include "dtk_graphics.h"
#include "dtk_controls.h"
#include "dtk_window.h"

// The main toolkit context.
struct dtk_context
{
    dtk_platform platform;
    dtk_event_proc onEvent;
    int exitCode;

    union
    {
#ifdef DTK_WIN32
        struct
        {
            /*HMODULE*/ dtk_handle hComctl32DLL;
            dtk_proc InitCommonControlsEx;
        } win32;
#endif
#ifdef DTK_GTK
        struct
        {
            int unused;
        } gtk;

        struct
        {
            int unused;
        } cairo;
#endif
    };
};

// Initializes the toolkit.
//
// This can be called multiple times, but each call must be paired with a matching call to dtk_uninit().
//
// You can set onEvent to NULL, but you'll want to call dtk_set_event_callback() later on to hook up an
// event handler.
//
// Once a context has been initialized it's location in memory cannot be changed. The reason for this is
// that controls and whatnot need to reference the context that owns it, which is done via a pointer.
// Consider using malloc() to allocate memory for the context if this becomes an issue for you.
//
// Thread Safety: UNSAFE
dtk_result dtk_init(dtk_context* pTK, dtk_event_proc onEvent);

// Uninitializes the toolkit.
//
// Thread Safety: UNSAFE
dtk_result dtk_uninit(dtk_context* pTK);

// Sets the event handler callback.
//
// Thread Safety: SAFE
//   This is implemented as an atomic assignment.
dtk_result dtk_set_event_callback(dtk_context* pTK, dtk_event_proc proc);

// Waits for an event and processes it.
//
// Set <blocking> to DTK_TRUE to block until a message is received. Set to DTK_FALSE to return immediately
// if no event is in the queue, in which case DTK_NO_EVENT will be returned.
//
// Thread Safety: UNSAFE
//   Do not call this from multiple threads. Have a single thread that does all event handling.
dtk_result dtk_next_event(dtk_context* pTK, dtk_bool32 blocking);

// Posts a quit event to the event queue. This will cause the main loop to terminate and dtk_next_event() to
// return DTK_QUIT.
//
// Thread Safety: SAFE
dtk_result dtk_post_quit_event(dtk_context* pTK, int exitCode);


#endif  // DTK_H

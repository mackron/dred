// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef void (* dtk_timer_proc)(dtk_timer* pTimer, void* pUserData);

struct dtk_timer
{
    dtk_context* pTK;
    void* pUserData;
    dtk_timer_proc callback;
    unsigned int timeoutInMilliseconds;

    union
    {
    #ifdef DTK_WIN32
        struct
        {
            uintptr_t tag;  // The value returned by SetTimer().
        } win32;
    #endif
    #ifdef DTK_GTK
        struct
        {
            dtk_uint32 timerID;
        } gtk;
    #endif
    #ifdef DTK_X11
        struct
        {
        } x11;
    #endif
    };
};

// Initializes a callback based timer.
dtk_result dtk_timer_init(dtk_context* pTK, dtk_uint32 timeoutInMilliseconds, dtk_timer_proc callback, void* pUserData, dtk_timer* pTimer);

// Uninitializes a timer.
dtk_result dtk_timer_uninit(dtk_timer* pTimer);
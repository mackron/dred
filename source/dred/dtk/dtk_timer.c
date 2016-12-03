// Copyright (C) 2016 David Reid. See included LICENSE file.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// WIN32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
static VOID CALLBACK dtk_timer_proc_win32(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    (void)hWnd;
    (void)uMsg;
    (void)dwTime;

    dtk_timer* pTimer = (dtk_timer*)idEvent;
    dtk_assert(pTimer != NULL);

    if (pTimer->callback != NULL) {
        pTimer->callback(pTimer, pTimer->pUserData);
    }
}

dtk_result dtk_timer_init__win32(dtk_context* pTK, dtk_uint32 timeoutInMilliseconds, dtk_timer_proc callback, void* pUserData, dtk_timer* pTimer)
{
    (void)callback;
    (void)pUserData;

    pTimer->win32.tag = SetTimer((HWND)pTK->win32.hMessagingWindow, (UINT_PTR)pTimer, timeoutInMilliseconds, dtk_timer_proc_win32);
    if (pTimer->win32.tag == 0) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_timer_uninit__win32(dtk_timer* pTimer)
{
    KillTimer((HWND)pTimer->pTK->win32.hMessagingWindow, pTimer->win32.tag);
    return DTK_SUCCESS;
}
#endif  // DTK_WIN32


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// GTK
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_GTK
static gboolean dtk_timer_proc_gtk(gpointer data)
{
    dtk_timer* pTimer = (dtk_timer*)data;
    dtk_assert(pTimer != NULL);

    if (pTimer->callback != NULL) {
        pTimer->callback(pTimer, pTimer->pUserData);
    }

    return DTK_TRUE;
}

dtk_result dtk_timer_init__gtk(dtk_context* pTK, dtk_uint32 timeoutInMilliseconds, dtk_timer_proc callback, void* pUserData, dtk_timer* pTimer)
{
    (void)pTK;
    (void)callback;
    (void)pUserData;

    pTimer->gtk.timerID = g_timeout_add(timeoutInMilliseconds, dtk_timer_proc_gtk, pTimer);
    return DTK_SUCCESS;
}

dtk_result dtk_timer_uninit__gtk(dtk_timer* pTimer)
{
    g_source_remove(pTimer->gtk.timerID);
    return DTK_SUCCESS;
}
#endif  // DTK_GTK

dtk_result dtk_timer_init(dtk_context* pTK, dtk_uint32 timeoutInMilliseconds, dtk_timer_proc callback, void* pUserData, dtk_timer* pTimer)
{
    if (pTimer == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pTimer);
    pTimer->pTK = pTK;
    pTimer->callback = callback;
    pTimer->pUserData = pUserData;
    pTimer->timeoutInMilliseconds = timeoutInMilliseconds;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_timer_init__win32(pTK, timeoutInMilliseconds, callback, pUserData, pTimer);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_timer_init__gtk(pTK, timeoutInMilliseconds, callback, pUserData, pTimer);
    }
#endif

    return result;
}

dtk_result dtk_timer_uninit(dtk_timer* pTimer)
{
    if (pTimer == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTimer->pTK->platform == dtk_platform_win32) {
        result = dtk_timer_uninit__win32(pTimer);
    }
#endif
#ifdef DTK_GTK
    if (pTimer->pTK->platform == dtk_platform_gtk) {
        result = dtk_timer_uninit__gtk(pTimer);
    }
#endif

    return result;
}
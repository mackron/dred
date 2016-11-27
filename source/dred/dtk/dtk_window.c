// Copyright (C) 2016 David Reid. See included LICENSE file.

dtk_result dtk_window__handle_event(dtk_window* pWindow, dtk_event* pEvent)
{
    dtk_assert(pWindow != NULL);
    dtk_assert(pEvent != NULL);

    // Some event need special handling before posting to the event handler.
    switch (pEvent->type)
    {
        case DTK_EVENT_SIZE:
        {
            // When a window is resized the drawing surface also needs to be resized.
            DTK_CONTROL(pWindow)->width = pEvent->size.width;
            DTK_CONTROL(pWindow)->height = pEvent->size.height;
        } break;

        default: break;
    }

    dtk_bool32 propagate = DTK_CONTROL(pWindow)->onEvent == NULL || DTK_CONTROL(pWindow)->onEvent(pEvent);
    if (!propagate) {
        return DTK_SUCCESS;
    }

    switch (pEvent->type)
    {
        case DTK_EVENT_MOUSE_MOVE:
        {
            
        } break;

        default: break;
    }

    return DTK_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// WIN32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
#define DTK_WIN32_WINDOW_CLASS "dtk.window"

#define DTK_GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define DTK_GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))

LRESULT CALLBACK CALLBACK dtk_GenericWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    dtk_window* pWindow = (dtk_window*)GetWindowLongPtrA(hWnd, 0);
    if (pWindow == NULL) {
        return DefWindowProcA(hWnd, msg, wParam, lParam);
    }

    dtk_event e;
    e.type = DTK_EVENT_NONE;
    e.pTK = DTK_CONTROL(pWindow)->pTK;
    e.pControl = DTK_CONTROL(pWindow);

    switch (msg)
    {
        case WM_CLOSE:
        {
            e.type = DTK_EVENT_CLOSE;
            if (e.pControl->onEvent) {
                e.pControl->onEvent(&e);
            }
        } return 0;

        case WM_MOUSEMOVE:
        {
            e.type = DTK_EVENT_MOUSE_MOVE;
            e.mouseMove.x = DTK_GET_X_LPARAM(lParam);
            e.mouseMove.y = DTK_GET_Y_LPARAM(lParam);
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            {
                dtk_surface_draw_quad(e.pControl->pSurface, 32, 32, 64, 64);
            }
            EndPaint(hWnd, &ps);
        } break;

        default: break;
    }

    if (e.type != DTK_EVENT_NONE) {
        dtk_bool32 propagate = e.pTK->onEvent == NULL || e.pTK->onEvent(&e);
        if (propagate) {
            dtk_window__handle_event(pWindow, &e);
        }
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

dtk_result dtk_window_init__win32(dtk_context* pTK, dtk_control* pParent, const char* title, dtk_uint32 width, dtk_uint32 height, dtk_window* pWindow)
{
    (void)pTK;

    // Do an upwards traversal until we find the overarching window control.
    while (pParent != NULL && pParent->type != DTK_CONTROL_TYPE_WINDOW) {
        pParent = pParent->pParent;
    }

    DWORD dwStyleEx = 0;
    DWORD dwStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW;
    HWND hWnd = CreateWindowExA(dwStyleEx, DTK_WIN32_WINDOW_CLASS, title, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, width, height, (pParent != NULL) ? (HWND)DTK_WINDOW(pParent)->win32.hWnd : NULL, NULL, NULL, NULL);
    if (hWnd == NULL) {
        return DTK_ERROR;
    }

    pWindow->win32.hWnd = (dtk_handle)hWnd;

    // The dtk window needs to be linked to the Win32 window handle so it can be accessed from the event handler.
    SetWindowLongPtrA(hWnd, 0, (LONG_PTR)pWindow);

    return DTK_SUCCESS;
}

dtk_result dtk_window_uninit__win32(dtk_window* pWindow)
{
    if (!DestroyWindow((HWND)pWindow->win32.hWnd)) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_window_show__win32(dtk_window* pWindow, int mode)
{
    int nCmdShow[] = {
        SW_HIDE,            // DTK_HIDE
        SW_SHOWNORMAL,      // DTK_SHOW_NORMAL
        SW_SHOWMAXIMIZED,   // DTK_SHOW_MAXIMIZED
    };

    if (!ShowWindow((HWND)pWindow->win32.hWnd, nCmdShow[mode])) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}
#endif




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// GTK
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_GTK
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

dtk_result dtk_window_init(dtk_context* pTK, dtk_control* pParent, const char* title, dtk_uint32 width, dtk_uint32 height, dtk_event_proc onEvent, dtk_window* pWindow)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pWindow);

    if (title  == NULL) title = "";
    if (width  == 0) width  = 1;
    if (height == 0) height = 1;

    dtk_result result = dtk_control_init(pTK, DTK_CONTROL_TYPE_WINDOW, onEvent, DTK_CONTROL(pWindow));
    if (result != DTK_SUCCESS) {
        return result;
    }
    
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_window_init__win32(pTK, pParent, title, width, height, pWindow);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_window_init__win32(pTK, pParent, title, width, height, pWindow);
    }
#endif
    if (result != DTK_SUCCESS) {
        return result;
    }

    result = dtk_surface_init_window(pTK, pWindow, &pWindow->surface);
    if (result != DTK_SUCCESS) {
        dtk_window_uninit(pWindow);
    }
    DTK_CONTROL(pWindow)->pSurface = &pWindow->surface;


    return DTK_SUCCESS;
}

dtk_result dtk_window_uninit(dtk_window* pWindow)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    // Window controls always own their surfaces.
    if (DTK_CONTROL(pWindow)->pSurface) {
        dtk_surface_uninit(DTK_CONTROL(pWindow)->pSurface);
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_uninit__win32(pWindow);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_uint32__gtk(pWindow);
    }
#endif
    if (result != DTK_SUCCESS) {
        return result;
    }

    return dtk_control_uninit(DTK_CONTROL(pWindow));
}

dtk_result dtk_window_show(dtk_window* pWindow, int mode)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;
    if (mode != DTK_HIDE &&
        mode != DTK_SHOW_NORMAL &&
        mode != DTK_SHOW_MAXIMIZED) {
        return DTK_INVALID_ARGS;
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_show__win32(pWindow, mode);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_show__gtk(pWindow, mode);
    }
#endif

    return result;
}
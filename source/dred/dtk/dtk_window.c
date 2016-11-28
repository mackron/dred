// Copyright (C) 2016 David Reid. See included LICENSE file.

dtk_result dtk_window__handle_event(dtk_window* pWindow, dtk_event* pEvent)
{
    dtk_assert(pWindow != NULL);
    dtk_assert(pEvent != NULL);
    
    // Post to the global event handler first.
    dtk_event_proc onEventGlobal = DTK_CONTROL(pWindow)->pTK->onEvent;
    dtk_bool32 propagate = onEventGlobal == NULL || onEventGlobal(pEvent);
    if (!propagate) {
        return DTK_SUCCESS;
    }
    

    // Some event need special handling before posting to the window's event handler.
    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dtk_surface_clear(pEvent->pControl->pSurface, dtk_color_rgb(128, 255, 128));
            dtk_surface_draw_quad(pEvent->pControl->pSurface, 32, 32, 64, 64);
        } break;

        case DTK_EVENT_SIZE:
        {
            // When a window is resized the drawing surface also needs to be resized.
            DTK_CONTROL(pWindow)->width  = pEvent->size.width;
            DTK_CONTROL(pWindow)->height = pEvent->size.height;
            
            if (DTK_CONTROL(pWindow)->pSurface != NULL) {
                dtk_assert(DTK_CONTROL(pWindow)->pSurface == &pWindow->surface);
                dtk_surface_uninit(&pWindow->surface);
            }
            
            if (dtk_surface_init_window(DTK_CONTROL(pWindow)->pTK, pWindow, &pWindow->surface) != DTK_SUCCESS) {
                DTK_CONTROL(pWindow)->pSurface = NULL;
            }
        } break;
        
        case DTK_EVENT_MOVE:
        {
            DTK_CONTROL(pWindow)->absolutePosX = pEvent->move.x;
            DTK_CONTROL(pWindow)->absolutePosY = pEvent->move.y;
        } break;

        default: break;
    }

    dtk_event_proc onEventLocal = DTK_CONTROL(pWindow)->onEvent;
    propagate = onEventLocal == NULL || onEventLocal(pEvent);
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

        case WM_PAINT:
        {
            RECT rect;
            if (GetUpdateRect(hWnd, &rect, FALSE)) {
                e.type = DTK_EVENT_PAINT;
                e.paint.rect.left = rect.left;
                e.paint.rect.top = rect.top;
                e.paint.rect.right = rect.right;
                e.paint.rect.bottom = rect.bottom;
            }
        } break;

        case WM_SIZE:
        {
            e.type = DTK_EVENT_SIZE;
            e.size.width  = LOWORD(lParam);
            e.size.height = HIWORD(lParam);
        } break;

        case WM_MOVE:
        {
            e.type = DTK_EVENT_MOVE;

        #if 1
            // This technique will use the position of the entire window, including the frame.
            RECT rect;
            GetWindowRect(hWnd, &rect);
            e.move.x = rect.left;
            e.move.y = rect.top;
        #else
            // This technique will use the position of the client area.
            e.move.x = DTK_GET_X_LPARAM(lParam);
            e.move.y = DTK_GET_Y_LPARAM(lParam);
        #endif
        } break;

        case WM_MOUSEMOVE:
        {
            e.type = DTK_EVENT_MOUSE_MOVE;
            e.mouseMove.x = DTK_GET_X_LPARAM(lParam);
            e.mouseMove.y = DTK_GET_Y_LPARAM(lParam);
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
static gboolean dtk_window__on_close__gtk(GtkWidget* pWidget, GdkEvent* pEvent, gpointer pUserData)
{
    (void)pWidget;
    (void)pEvent;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) return DTK_TRUE;
    
    dtk_event e = dtk_event_init(DTK_EVENT_CLOSE, DTK_CONTROL(pWindow));
    dtk_window__handle_event(pWindow, &e);
    
    return DTK_TRUE;
}

static gboolean dtk_window__on_configure__gtk(GtkWidget* pWidget, GdkEventConfigure* pEvent, gpointer pUserData)
{
    dtk_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return DTK_FALSE;
    }

    if (pEvent->x != DTK_CONTROL(pWindow)->absolutePosX || pEvent->y != DTK_CONTROL(pWindow)->absolutePosY) {    
        // Position has changed.
        dtk_event e = dtk_event_init(DTK_EVENT_MOVE, DTK_CONTROL(pWindow));
        e.move.x = pEvent->x;
        e.move.y = pEvent->y;
        dtk_window__handle_event(pWindow, &e);
    }

    return DTK_FALSE;
}

static gboolean dtk_window__on_draw_clientarea__gtk(GtkWidget* pWidget, cairo_t* cr, gpointer pUserData)
{
    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL || DTK_CONTROL(pWindow)->pSurface == NULL) return DTK_FALSE;

    double clipLeft;
    double clipTop;
    double clipRight;
    double clipBottom;
    cairo_clip_extents(DTK_CONTROL(pWindow)->pSurface->cairo.pContext, &clipLeft, &clipTop, &clipRight, &clipBottom);

    dtk_event e = dtk_event_init(DTK_EVENT_PAINT, DTK_CONTROL(pWindow));
    e.paint.rect.left = clipLeft;
    e.paint.rect.top = clipTop;
    e.paint.rect.right = clipRight;
    e.paint.rect.bottom = clipBottom;
    dtk_window__handle_event(pWindow, &e);
    
    cairo_set_source_surface(cr, DTK_CONTROL(pWindow)->pSurface->cairo.pSurface, 0, 0);
    cairo_paint(cr);

    return DTK_FALSE;
}

static gboolean dtk_window__on_configure_clientarea__gtk(GtkWidget* pWidget, GdkEventConfigure* pEvent, gpointer pUserData)
{
    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_FALSE;
    }

    // If the window's size has changed, it's panel and surface need to be resized, and then redrawn.
    if (pEvent->width != DTK_CONTROL(pWindow)->width || pEvent->height != DTK_CONTROL(pWindow)->height) {
        // Size has changed.
        dtk_event e = dtk_event_init(DTK_EVENT_SIZE, DTK_CONTROL(pWindow));
        e.size.width = pEvent->width;
        e.size.height = pEvent->height;
        dtk_window__handle_event(pWindow, &e);

        // Invalidate the window to force a redraw.
        gtk_widget_queue_draw(pWidget);
    }

    return DTK_FALSE;
}


dtk_result dtk_window_init__gtk(dtk_context* pTK, dtk_control* pParent, const char* title, dtk_uint32 width, dtk_uint32 height, dtk_window* pWindow)
{
    // Client area. This is where everything is drawn.
    GtkWidget* pClientArea = gtk_drawing_area_new();
    if (pClientArea == NULL) {
        return DTK_ERROR;
    }
    
    gtk_widget_add_events(pClientArea,
        GDK_ENTER_NOTIFY_MASK   |
        GDK_LEAVE_NOTIFY_MASK   |
        GDK_POINTER_MOTION_MASK |
        GDK_BUTTON_PRESS_MASK   |
        GDK_BUTTON_RELEASE_MASK |
        GDK_SCROLL_MASK);
        
    g_signal_connect(pClientArea, "draw",            G_CALLBACK(dtk_window__on_draw_clientarea__gtk),      pWindow);
    g_signal_connect(pClientArea, "configure-event", G_CALLBACK(dtk_window__on_configure_clientarea__gtk), pWindow);
    
    
    // Box container. This is used to laying out the menu bar and client area.
    GtkWidget* pBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    if (pBox == NULL) {
        gtk_widget_destroy(pClientArea);
        return DTK_ERROR;
    }
    
    gtk_box_pack_start(GTK_BOX(pBox), pClientArea, TRUE, TRUE, 0);
    
    
    // Main window.
    GtkWidget* pWidget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (pWidget == NULL) {
        gtk_widget_destroy(pBox);
        gtk_widget_destroy(pClientArea);
        return DTK_ERROR;
    }
    
    gtk_widget_add_events(pWidget,
        GDK_KEY_PRESS_MASK      |
        GDK_KEY_RELEASE_MASK    |
        GDK_FOCUS_CHANGE_MASK);
        
    gtk_window_set_resizable(GTK_WINDOW(pWidget), TRUE);
    gtk_window_resize(GTK_WINDOW(pWidget), (int)width, (int)height);
    g_signal_connect(pWidget, "delete-event",    G_CALLBACK(dtk_window__on_close__gtk),     pWindow);
    g_signal_connect(pWidget, "configure-event", G_CALLBACK(dtk_window__on_configure__gtk), pWindow);
    
    gtk_container_add(GTK_CONTAINER(pWidget), pBox);



    pWindow->gtk.pWidget     = pWidget;
    pWindow->gtk.pBox        = pBox;
    pWindow->gtk.pClientArea = pClientArea;
    
    gtk_widget_show_all(GTK_WIDGET(pWidget));
    gtk_widget_realize(pWidget);
    
    return DTK_SUCCESS;
}

dtk_result dtk_window_uninit__gtk(dtk_window* pWindow)
{
    gtk_widget_destroy(GTK_WIDGET(pWindow->gtk.pClientArea));
    gtk_widget_destroy(GTK_WIDGET(pWindow->gtk.pBox));
    gtk_widget_destroy(GTK_WIDGET(pWindow->gtk.pWidget));
    return DTK_SUCCESS;
}

dtk_result dtk_window_show__gtk(dtk_window* pWindow, int mode)
{
    if (mode == DTK_HIDE) {
        gtk_widget_hide(GTK_WIDGET(pWindow->gtk.pWidget));
    } else if (mode == DTK_SHOW_NORMAL) {
        gtk_window_present(GTK_WINDOW(pWindow->gtk.pWidget));
    } else if (mode == DTK_SHOW_MAXIMIZED) {
        gtk_window_present(GTK_WINDOW(pWindow->gtk.pWidget));
        gtk_window_maximize(GTK_WINDOW(pWindow->gtk.pWidget));
    } else {
        return DTK_INVALID_ARGS;
    }
    
    return DTK_SUCCESS;
}
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
    
    // Do an upwards traversal until we find the overarching window control.
    while (pParent != NULL && pParent->type != DTK_CONTROL_TYPE_WINDOW) {
        pParent = pParent->pParent;
    }
    
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_window_init__win32(pTK, pParent, title, width, height, pWindow);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_window_init__gtk(pTK, pParent, title, width, height, pWindow);
    }
#endif
    if (result != DTK_SUCCESS) {
        return result;
    }

    result = dtk_surface_init_window(pTK, pWindow, &pWindow->surface);
    if (result != DTK_SUCCESS) {
        dtk_window_uninit(pWindow);
        return result;
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
        result = dtk_window_uninit__gtk(pWindow);
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
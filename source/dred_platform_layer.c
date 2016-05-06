
//////////////////////////////////////////////////////////////////
//
// Win32
//
//////////////////////////////////////////////////////////////////

#ifdef DRED_WIN32
static const char* g_WindowClass = "dred_WindowClass";

#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))

static void dred_win32_track_mouse_leave_event(HWND hWnd)
{
    TRACKMOUSEEVENT tme;
    ZeroMemory(&tme, sizeof(tme));
    tme.cbSize    = sizeof(tme);
    tme.dwFlags   = TME_LEAVE;
    tme.hwndTrack = hWnd;
    TrackMouseEvent(&tme);
}

bool dred_is_win32_mouse_button_key_code(WPARAM wParam)
{
    return wParam == VK_LBUTTON || wParam == VK_RBUTTON || wParam == VK_MBUTTON || wParam == VK_XBUTTON1 || wParam == VK_XBUTTON2;
}

drgui_key dred_win32_to_drgui_key(WPARAM wParam)
{
    switch (wParam)
    {
    case VK_BACK:   return DRGUI_BACKSPACE;
    case VK_SHIFT:  return DRGUI_SHIFT;
    case VK_ESCAPE: return DRGUI_ESCAPE;
    case VK_PRIOR:  return DRGUI_PAGE_UP;
    case VK_NEXT:   return DRGUI_PAGE_DOWN;
    case VK_END:    return DRGUI_END;
    case VK_HOME:   return DRGUI_HOME;
    case VK_LEFT:   return DRGUI_ARROW_LEFT;
    case VK_UP:     return DRGUI_ARROW_UP;
    case VK_RIGHT:  return DRGUI_ARROW_RIGHT;
    case VK_DOWN:   return DRGUI_ARROW_DOWN;
    case VK_DELETE: return DRGUI_DELETE;

    default: break;
    }

    return (drgui_key)wParam;
}

int dred_win32_get_modifier_key_state_flags()
{
    int stateFlags = 0;

    SHORT keyState = GetAsyncKeyState(VK_SHIFT);
    if (keyState & 0x8000) {
        stateFlags |= DRED_KEY_STATE_SHIFT_DOWN;
    }

    keyState = GetAsyncKeyState(VK_CONTROL);
    if (keyState & 0x8000) {
        stateFlags |= DRED_KEY_STATE_CTRL_DOWN;
    }

    keyState = GetAsyncKeyState(VK_MENU);
    if (keyState & 0x8000) {
        stateFlags |= DRED_KEY_STATE_ALT_DOWN;
    }

    return stateFlags;
}

int dred_win32_get_mouse_event_state_flags(WPARAM wParam)
{
    int stateFlags = 0;

    if ((wParam & MK_LBUTTON) != 0) {
        stateFlags |= DRED_MOUSE_BUTTON_LEFT_DOWN;
    }

    if ((wParam & MK_RBUTTON) != 0) {
        stateFlags |= DRED_MOUSE_BUTTON_RIGHT_DOWN;
    }

    if ((wParam & MK_MBUTTON) != 0) {
        stateFlags |= DRED_MOUSE_BUTTON_MIDDLE_DOWN;
    }

    if ((wParam & MK_XBUTTON1) != 0) {
        stateFlags |= DRED_MOUSE_BUTTON_4_DOWN;
    }

    if ((wParam & MK_XBUTTON2) != 0) {
        stateFlags |= DRED_MOUSE_BUTTON_5_DOWN;
    }


    if ((wParam & MK_CONTROL) != 0) {
        stateFlags |= DRED_KEY_STATE_CTRL_DOWN;
    }

    if ((wParam & MK_SHIFT) != 0) {
        stateFlags |= DRED_KEY_STATE_SHIFT_DOWN;
    }


    SHORT keyState = GetAsyncKeyState(VK_MENU);
    if (keyState & 0x8000) {
        stateFlags |= DRED_KEY_STATE_ALT_DOWN;
    }


    return stateFlags;
}

LRESULT CALLBACK GenericWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    dred_window* pWindow = (dred_window*)GetWindowLongPtrA(hWnd, 0);
    if (pWindow == NULL) {
        return DefWindowProcA(hWnd, msg, wParam, lParam);
    }

    switch (msg)
    {
        case WM_CREATE:
        {
            // This allows us to track mouse enter and leave events for the window.
            dred_win32_track_mouse_leave_event(hWnd);
        } return 0;

        case WM_DESTROY:
        {
        } break;

        
        case WM_CLOSE:
        {
            if (pWindow->onClose) {
                pWindow->onClose(pWindow);
            }
        } return 0;

        // show/hide
        case WM_WINDOWPOSCHANGING:
        {
            // Showing and hiding windows can be cancelled if false is returned by any of the event handlers.
            WINDOWPOS* pWindowPos = (WINDOWPOS*)lParam;
            assert(pWindowPos != NULL);

            if ((pWindowPos->flags & SWP_HIDEWINDOW) != 0 && pWindow->onHide) {
                if (!pWindow->onHide(pWindow, pWindow->onHideFlags)) {
                    pWindowPos->flags &= ~SWP_HIDEWINDOW;
                }

                pWindow->onHideFlags = 0;
            }

            if ((pWindowPos->flags & SWP_SHOWWINDOW) != 0 && pWindow->onShow) {
                if (!pWindow->onShow(pWindow)) {
                    pWindowPos->flags &= ~SWP_SHOWWINDOW;
                }
            }
        } break;

        case WM_ERASEBKGND:  return 1;       // Never draw the background of the window - always leave that to dr_gui.
        default: break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool dred_platform_init__win32()
{
    // We'll be handling DPI ourselves. This should be done at the top.
    dr_win32_make_dpi_aware();

    // Window classes.
    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.cbWndExtra    = sizeof(dred_window*);
    wc.lpfnWndProc   = (WNDPROC)GenericWindowProc;
    wc.lpszClassName = g_WindowClass;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(101));
    wc.style         = CS_DBLCLKS;
    if (!RegisterClassExA(&wc)) {
        return false;
    }

    return true;
}

void dred_platform_uninit__win32()
{
    UnregisterClassA(g_WindowClass, NULL);
}

int dred_platform_run__win32()
{
    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            return -42; // Unknown error.
        }

        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}

void dred_platform_post_quit_message__win32(int resultCode)
{
    PostQuitMessage(resultCode);
}





dred_window* dred_window_create__win32()
{
    DWORD dwStyleEx = 0;
    DWORD dwStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW;
    HWND hWnd = CreateWindowExA(dwStyleEx, g_WindowClass, "dred", dwStyle, 0, 0, 1280, 720, NULL, NULL, NULL, NULL);
    if (hWnd == NULL) {
        return NULL;
    }

    dred_window* pWindow = calloc(1, sizeof(*pWindow));
    if (pWindow == NULL) {
        goto on_error;
    }

    pWindow->hWnd = hWnd;

    // The dred window needs to be linked to the Win32 window handle so it can be accessed from the event handler.
    SetWindowLongPtrA(hWnd, 0, (LONG_PTR)pWindow);

    return pWindow;


on_error:
    if (pWindow) {
        free(pWindow);
    }

    DestroyWindow(hWnd);
    return NULL;
}

void dred_window_delete__win32(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    DestroyWindow(pWindow->hWnd);
    free(pWindow);
}


void dred_window_set_size__win32(dred_window* pWindow, unsigned int newWidth, unsigned int newHeight)
{
    RECT windowRect;
    RECT clientRect;
    GetWindowRect(pWindow->hWnd, &windowRect);
    GetClientRect(pWindow->hWnd, &clientRect);

    int windowFrameX = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
    int windowFrameY = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);

    assert(windowFrameX >= 0);
    assert(windowFrameY >= 0);

    int scaledWidth  = newWidth  + windowFrameX;
    int scaledHeight = newHeight + windowFrameY;
    SetWindowPos(pWindow->hWnd, NULL, 0, 0, scaledWidth, scaledHeight, SWP_NOZORDER | SWP_NOMOVE);
}

void dred_window_get_size__win32(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
    RECT rect;
    GetClientRect(pWindow->hWnd, &rect);

    if (pWidthOut != NULL) {
        *pWidthOut = rect.right - rect.left;
    }
    if (pHeightOut != NULL) {
        *pHeightOut = rect.bottom - rect.top;
    }
}


void dred_window_show__win32(dred_window* pWindow)
{
    ShowWindow(pWindow->hWnd, SW_SHOWNORMAL);
}

void dred_window_show_maximized__win32(dred_window* pWindow)
{
    ShowWindow(pWindow->hWnd, SW_SHOWMAXIMIZED);
}

void dred_window_hide__win32(dred_window* pWindow)
{
    ShowWindow(pWindow->hWnd, SW_HIDE);
}
#endif




//////////////////////////////////////////////////////////////////
//
// GTK+ 3
//
//////////////////////////////////////////////////////////////////

#ifdef DRED_GTK
int g_GTKMainLoopResultCode = 0;

bool dred_platform_init__gtk()
{
    gtk_init(NULL, NULL);

    return true;
}

void dred_platform_uninit__gtk()
{
}

int dred_platform_run__gtk()
{
    gtk_main();
    return g_GTKMainLoopResultCode;
}

void dred_platform_post_quit_message__gtk(int resultCode)
{
    g_GTKMainLoopResultCode = resultCode;
    gtk_main_quit();
}




static gboolean dred_gtk_cb__on_close(GtkWidget* pGTKWindow, GdkEvent* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;
    (void)pEvent;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    if (pWindow->onClose) {
        pWindow->onClose(pWindow);
    }

    return true;
}

static void dred_gtk_cb__on_hide(GtkWidget* pGTKWindow, gpointer pUserData)
{
    (void)pGTKWindow;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return;
    }

    if ((pWindow->onHideFlags & DRED_HIDE_BLOCKED) != 0) {
        pWindow->onHideFlags &= ~DRED_HIDE_BLOCKED;
        return;
    }

    if (pWindow->onHide) {
        if (!pWindow->onHide(pWindow, pWindow->onHideFlags)) {
            dred_window_show(pWindow);    // The event handler returned false, so prevent the window from being hidden.
        }
    }
}

static void dred_gtk_cb__on_show(GtkWidget* pGTKWindow, gpointer pUserData)
{
    (void)pGTKWindow;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onShow) {
        if (!pWindow->onShow(pWindow)) {
            dred_window_hide(pWindow, DRED_HIDE_BLOCKED);    // The event handler returned false, so prevent the window from being shown.
        } else {
            gtk_widget_grab_focus(GTK_WIDGET(pWindow->pGTKWindow));
        }
    }
}




dred_window* dred_window_create__gtk()
{
    GtkWidget* pGTKWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (pGTKWindow == NULL) {
        printf("Failed to create window.\n");
    }

    dred_window* pWindow = (dred_window*)calloc(1, sizeof(*pWindow));
    if (pWindow == NULL) {
        goto on_error;
    }

    pWindow->pGTKWindow = pGTKWindow;


    // Because we are drawing the GUI ourselves rather than through GTK, we want to disable GTK's default painting
    // procedure. To do this we use gtk_widget_set_app_paintable(pWindow, TRUE) which will disable GTK's default
    // painting on the main window. We then connect to the "draw" signal which is where we'll do all of our custom
    // painting.
    gtk_widget_set_app_paintable(pGTKWindow, TRUE);

    // These are the types of events we care about.
    gtk_widget_add_events(pGTKWindow,
        GDK_ENTER_NOTIFY_MASK   |
        GDK_LEAVE_NOTIFY_MASK   |
        GDK_POINTER_MOTION_MASK |
        GDK_BUTTON_PRESS_MASK   |
        GDK_BUTTON_RELEASE_MASK |
        GDK_SCROLL_MASK         |
        GDK_KEY_PRESS_MASK      |
        GDK_KEY_RELEASE_MASK    |
        GDK_FOCUS_CHANGE_MASK);

    g_signal_connect(pGTKWindow, "delete-event",         G_CALLBACK(dred_gtk_cb__on_close),             pWindow);     // Close.
    g_signal_connect(pGTKWindow, "hide",                 G_CALLBACK(dred_gtk_cb__on_hide),              pWindow);     // Hide.
    g_signal_connect(pGTKWindow, "show",                 G_CALLBACK(dred_gtk_cb__on_show),              pWindow);     // Show.

    return pWindow;

on_error:
    if (pWindow != NULL) {
        free(pWindow);
    }
    
    gtk_widget_destroy(pGTKWindow);
    return NULL;
}

void dred_window_delete__gtk(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    gtk_widget_destroy(pWindow->pGTKWindow);
    free(pWindow);
}


void dred_window_set_size__gtk(dred_window* pWindow, unsigned int newWidth, unsigned int newHeight)
{
    gtk_window_resize(GTK_WINDOW(pWindow->pGTKWindow), (int)newWidth, (int)newHeight);
}

void dred_window_get_size__gtk(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
    int width;
    int height;
    gtk_window_get_size(GTK_WINDOW(pWindow->pGTKWindow), &width, &height);

    if (*pWidthOut) *pWidthOut = width;
    if (*pHeightOut) *pHeightOut = height;
}


void dred_window_show__gtk(dred_window* pWindow)
{
    gtk_window_present(GTK_WINDOW(pWindow->pGTKWindow));
}

void dred_window_show_maximized__gtk(dred_window* pWindow)
{
    gtk_window_present(GTK_WINDOW(pWindow->pGTKWindow));    // <-- Is this needed?
    gtk_window_maximize(GTK_WINDOW(pWindow->pGTKWindow));
}

void dred_window_hide__gtk(dred_window* pWindow)
{
    gtk_widget_hide(GTK_WIDGET(pWindow->pGTKWindow));
}
#endif




bool dred_platform_init()
{
#ifdef DRED_WIN32
    return dred_platform_init__win32();
#endif

#ifdef DRED_GTK
    return dred_platform_init__gtk();
#endif
}

void dred_platform_uninit()
{
#ifdef DRED_WIN32
    dred_platform_uninit__win32();
#endif

#ifdef DRED_GTK
    dred_platform_uninit__gtk();
#endif
}

int dred_platform_run()
{
#ifdef DRED_WIN32
    return dred_platform_run__win32();
#endif

#ifdef DRED_GTK
    return dred_platform_run__gtk();
#endif
}

void dred_platform_post_quit_message(int resultCode)
{
#ifdef DRED_WIN32
    dred_platform_post_quit_message__win32(resultCode);
#endif

#ifdef DRED_GTK
    dred_platform_post_quit_message__gtk(resultCode);
#endif
}




dred_window* dred_window_create()
{
#ifdef DRED_WIN32
    return dred_window_create__win32();
#endif

#ifdef DRED_GTK
    return dred_window_create__gtk();
#endif
}

void dred_window_delete(dred_window* pWindow)
{
#ifdef DRED_WIN32
    dred_window_delete__win32(pWindow);
#endif

#ifdef DRED_GTK
    dred_window_delete__gtk(pWindow);
#endif
}


void dred_window_set_size(dred_window* pWindow, unsigned int newWidth, unsigned int newHeight)
{
#ifdef DRED_WIN32
    dred_window_set_size__win32(pWindow, newWidth, newHeight);
#endif

#ifdef DRED_GTK
    dred_window_set_size__gtk(pWindow, newWidth, newHeight);
#endif
}

void dred_window_get_size(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
#ifdef DRED_WIN32
    dred_window_get_size__win32(pWindow, pWidthOut, pHeightOut);
#endif

#ifdef DRED_GTK
    dred_window_get_size__gtk(pWindow, pWidthOut, pHeightOut);
#endif
}


void dred_window_show(dred_window* pWindow)
{
#ifdef DRED_WIN32
    dred_window_show__win32(pWindow);
#endif

#ifdef DRED_GTK
    dred_window_show__gtk(pWindow);
#endif
}

void dred_window_show_maximized(dred_window* pWindow)
{
#ifdef DRED_WIN32
    dred_window_show_maximized__win32(pWindow);
#endif

#ifdef DRED_GTK
    dred_window_show_maximized__gtk(pWindow);
#endif
}

void dred_window_show_sized(dred_window* pWindow, unsigned int width, unsigned int height)
{
    dred_window_set_size(pWindow, width, height);
    dred_window_show(pWindow);
}

void dred_window_hide(dred_window* pWindow, unsigned int flags)
{
    pWindow->onHideFlags = flags;

#ifdef DRED_WIN32
    dred_window_hide__win32(pWindow);
#endif

#ifdef DRED_GTK
    dred_window_hide__gtk(pWindow);
#endif
}

// Copyright (C) 2017 David Reid. See included LICENSE file.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// WIN32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
#define DTK_WIN32_WINDOW_CLASS              "dtk.window"
#define DTK_WIN32_WINDOW_CLASS_POPUP        "dtk.window.popup"
#define DTK_WIN32_WINDOW_CLASS_MESSAGING    "dtk.window.messaging"

#define DTK_GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define DTK_GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))

static void dtk_track_mouse_leave_event__win32(HWND hWnd)
{
    TRACKMOUSEEVENT tme;
    ZeroMemory(&tme, sizeof(tme));
    tme.cbSize    = sizeof(tme);
    tme.dwFlags   = TME_LEAVE;
    tme.hwndTrack = hWnd;
    TrackMouseEvent(&tme);
}

dtk_bool32 dtk_is_win32_mouse_button_key_code(WPARAM wParam)
{
    return wParam == VK_LBUTTON || wParam == VK_RBUTTON || wParam == VK_MBUTTON || wParam == VK_XBUTTON1 || wParam == VK_XBUTTON2;
}

static dtk_uint32 dtk_get_modifier_key_state_flags__win32()
{
    int stateFlags = 0;

    SHORT keyState = GetAsyncKeyState(VK_SHIFT);
    if (keyState & 0x8000) {
        stateFlags |= DTK_MODIFIER_SHIFT;
    }

    keyState = GetAsyncKeyState(VK_CONTROL);
    if (keyState & 0x8000) {
        stateFlags |= DTK_MODIFIER_CTRL;
    }

    keyState = GetAsyncKeyState(VK_MENU);
    if (keyState & 0x8000) {
        stateFlags |= DTK_MODIFIER_ALT;
    }

    return stateFlags;
}

static dtk_uint32 dtk_get_mouse_event_state_flags__win32(WPARAM wParam)
{
    int stateFlags = 0;

    if ((wParam & MK_LBUTTON) != 0) {
        stateFlags |= DTK_MODIFIER_MOUSE_BUTTON_LEFT;
    }
    if ((wParam & MK_RBUTTON) != 0) {
        stateFlags |= DTK_MODIFIER_MOUSE_BUTTON_RIGHT;
    }
    if ((wParam & MK_MBUTTON) != 0) {
        stateFlags |= DTK_MODIFIER_MOUSE_BUTTON_MIDDLE;
    }
    if ((wParam & MK_XBUTTON1) != 0) {
        stateFlags |= DTK_MODIFIER_MOUSE_BUTTON_4;
    }
    if ((wParam & MK_XBUTTON2) != 0) {
        stateFlags |= DTK_MODIFIER_MOUSE_BUTTON_5;
    }

    if ((wParam & MK_CONTROL) != 0) {
        stateFlags |= DTK_MODIFIER_CTRL;
    }
    if ((wParam & MK_SHIFT) != 0) {
        stateFlags |= DTK_MODIFIER_SHIFT;
    }

    SHORT keyState = GetAsyncKeyState(VK_MENU);
    if (keyState & 0x8000) {
        stateFlags |= DTK_MODIFIER_ALT;
    }

    return stateFlags;
}

// This API is unused for now, but leaving it here in case I need to quickly add it again.
#if 0
static ACCEL dtk_win32_to_ACCEL(dtk_key key, uint32_t modifiers, WORD cmd)
{
    ACCEL a;
    a.key = dtk_convert_key_to_win32(key);
    a.cmd = cmd;

    a.fVirt = FVIRTKEY;
    if (modifiers & DTK_MODIFIER_SHIFT) {
        a.fVirt |= FSHIFT;
    }
    if (modifiers & DTK_MODIFIER_CTRL) {
        a.fVirt |= FCONTROL;
    }
    if (modifiers & DTK_MODIFIER_ALT) {
        a.fVirt |= FALT;
    }

    return a;
}
#endif

static dtk_uint32 dtk_wm_event_to_mouse_button__win32(UINT msg)
{
    switch (msg)
    {
        case WM_NCRBUTTONDOWN:
        case WM_NCRBUTTONUP:
        case WM_NCRBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        {
            return DTK_MOUSE_BUTTON_RIGHT;
        }

        case WM_NCMBUTTONDOWN:
        case WM_NCMBUTTONUP:
        case WM_NCMBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
        {
            return DTK_MOUSE_BUTTON_MIDDLE;
        }

        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONUP:
        case WM_NCLBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        default:
        {
            return DTK_MOUSE_BUTTON_LEFT;
        }
    }
}

static HWND dtk_get_root_top_level_HWND(HWND hWnd)
{
    HWND hTopLevelWindow = hWnd;
    do
    {
        dtk_window* pWindow = (dtk_window*)GetWindowLongPtrA(hTopLevelWindow, 0);
        if (pWindow != NULL) {
            if (pWindow->isTopLevel) {
                return hTopLevelWindow;
            }
        }

        hTopLevelWindow = GetWindow(hTopLevelWindow, GW_OWNER);
    } while (hTopLevelWindow != 0);

    return hTopLevelWindow;
}

static dtk_window* dtk_find_window_by_HWND(dtk_context* pTK, HWND hWnd)
{
    for (dtk_window* pWindow = pTK->pFirstWindow; pWindow != NULL; pWindow = pWindow->pNextWindow) {
        if (pWindow->win32.hWnd == hWnd) {
            return pWindow;
        }
    }

    return NULL;
}

static BOOL dtk_is_HWND_owned_by_this(dtk_context* pTK, HWND hWnd)
{
    return dtk_find_window_by_HWND(pTK, hWnd) != NULL;
}

LRESULT CALLBACK CALLBACK dtk_GenericWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    dtk_window* pWindow = (dtk_window*)GetWindowLongPtrA(hWnd, 0);
    if (pWindow == NULL) {
        return DefWindowProcA(hWnd, msg, wParam, lParam);
    }

    dtk_context* pTK = DTK_CONTROL(pWindow)->pTK;
    dtk_assert(pTK != NULL);

    dtk_event e;
    e.type = DTK_EVENT_NONE;
    e.pTK = pTK;
    e.pControl = DTK_CONTROL(pWindow);

    switch (msg)
    {
        case WM_CREATE:
        {
        } return 0;

        case WM_DESTROY:
        {
        } break;

        case WM_CLOSE:
        {
            e.type = DTK_EVENT_CLOSE;
            dtk_handle_global_event(&e);
        } return 0;

        case DTK_WM_PAINT_NOTIFICATION:
        {
            dtk_window* pPaintWindow = (dtk_window*)lParam;
            dtk_assert(pPaintWindow == pWindow);

            dtk_handle_paint_notification_event(pTK, pPaintWindow);
        } break;

        case WM_PAINT:
        {
#if 1
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd, &ps);
            if (hDC != NULL) {
                dtk_surface surface;
                if (dtk_surface_init_transient_HDC(e.pTK, hDC, DTK_CONTROL(pWindow)->width, DTK_CONTROL(pWindow)->height, &surface) == DTK_SUCCESS) {
                    e.type = DTK_EVENT_PAINT;
                    e.paint.rect.left = ps.rcPaint.left;
                    e.paint.rect.top = ps.rcPaint.top;
                    e.paint.rect.right = ps.rcPaint.right;
                    e.paint.rect.bottom = ps.rcPaint.bottom;
                    e.paint.pSurface = &surface;
                    dtk_handle_global_event(&e);

                    SelectClipRgn(hDC, NULL);       // Make sure the clip is restored to ensure future paint messages are handled correctly by the operating system.
                    dtk_surface_uninit(&surface);
                }

                EndPaint(hWnd, &ps);
            }
#else
            RECT rect;
            if (GetUpdateRect(hWnd, &rect, FALSE)) {
                dtk_surface surface;
                if (dtk_surface_init_transient_HDC(e.pTK, GetDC(hWnd), DTK_CONTROL(pWindow)->width, DTK_CONTROL(pWindow)->height, &surface) == DTK_SUCCESS) {
                    e.type = DTK_EVENT_PAINT;
                    e.paint.rect.left = rect.left;
                    e.paint.rect.top = rect.top;
                    e.paint.rect.right = rect.right;
                    e.paint.rect.bottom = rect.bottom;
                    e.paint.pSurface = &surface;
                    dtk_handle_global_event(&e);

                    dtk_surface_uninit(&surface);
                }
            }
#endif
        } break;

        case WM_SIZE:
        {
            e.type = DTK_EVENT_SIZE;
            e.size.width  = LOWORD(lParam);
            e.size.height = HIWORD(lParam);
            dtk_handle_global_event(&e);
        } break;

        case WM_MOVE:
        {
            e.type = DTK_EVENT_MOVE;

        #if 0
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

            dtk_handle_global_event(&e);
        } break;


        // show/hide
        case WM_WINDOWPOSCHANGING:
        {
            WINDOWPOS* pWindowPos = (WINDOWPOS*)lParam;
            assert(pWindowPos != NULL);

            if ((pWindowPos->flags & SWP_HIDEWINDOW) != 0) {
                e.type = DTK_EVENT_HIDE;
                dtk_handle_global_event(&e);
            }
            if ((pWindowPos->flags & SWP_SHOWWINDOW) != 0) {
                e.type = DTK_EVENT_SHOW;
                dtk_handle_global_event(&e);
            }
        } break;


        case WM_MOUSELEAVE:
        {
            pWindow->win32.isCursorOverClientArea = DTK_FALSE;

            e.type = DTK_EVENT_MOUSE_LEAVE;
            dtk_handle_global_event(&e);
        } break;

        case WM_MOUSEMOVE:
        {
            if (!pWindow->win32.isCursorOverClientArea) {
                pWindow->win32.isCursorOverClientArea = DTK_TRUE;
                e.type = DTK_EVENT_MOUSE_ENTER;
                dtk_handle_global_event(&e);

                dtk_track_mouse_leave_event__win32(hWnd);
            }
            
            e.type = DTK_EVENT_MOUSE_MOVE;
            e.mouseMove.x = DTK_GET_X_LPARAM(lParam);
            e.mouseMove.y = DTK_GET_Y_LPARAM(lParam);
            e.mouseMove.state = dtk_get_mouse_event_state_flags__win32(wParam);
            dtk_handle_global_event(&e);
        } break;

    // Uncomment this to enable mouse button events in the non-client area of the window. This is inconsistent with other backends, however.
    #if 0
        case WM_NCLBUTTONDOWN:
        case WM_NCRBUTTONDOWN:
        case WM_NCMBUTTONDOWN:
        {
            POINT p;
            p.x = DTK_GET_X_LPARAM(lParam);
            p.y = DTK_GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            e.type = DTK_EVENT_MOUSE_BUTTON_DOWN;
            e.mouseButton.x = p.x;
            e.mouseButton.y = p.y;
            e.mouseButton.button = dtk_wm_event_to_mouse_button__win32(msg);
            e.mouseButton.state = dtk_get_mouse_event_state_flags__win32(wParam) | dtk_get_mouse_button_modifier_flag(e.mouseButton.button);
            dtk_handle_global_event(&e);
        } break;

        case WM_NCLBUTTONUP:
        case WM_NCRBUTTONUP:
        case WM_NCMBUTTONUP:
        {
            POINT p;
            p.x = DTK_GET_X_LPARAM(lParam);
            p.y = DTK_GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            e.type = DTK_EVENT_MOUSE_BUTTON_UP;
            e.mouseButton.x = p.x;
            e.mouseButton.y = p.y;
            e.mouseButton.button = dtk_wm_event_to_mouse_button__win32(msg);
            e.mouseButton.state = dtk_get_mouse_event_state_flags__win32(wParam);
            dtk_handle_global_event(&e);
        } break;

        case WM_NCLBUTTONDBLCLK:
        case WM_NCRBUTTONDBLCLK:
        case WM_NCMBUTTONDBLCLK:
        {
            POINT p;
            p.x = DTK_GET_X_LPARAM(lParam);
            p.y = DTK_GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            // Special case for double click because we want to post a normal mouse button down event first before the double-click event.
            e.type = DTK_EVENT_MOUSE_BUTTON_DOWN;
            e.mouseButton.x = p.x;
            e.mouseButton.y = p.y;
            e.mouseButton.button = dtk_wm_event_to_mouse_button__win32(msg);
            e.mouseButton.state = dtk_get_mouse_event_state_flags__win32(wParam) | dtk_get_mouse_button_modifier_flag(e.mouseButton.button);
            dtk_handle_global_event(&e);

            e.type = DTK_EVENT_MOUSE_BUTTON_DBLCLICK;
            dtk_handle_global_event(&e);
        } break;
    #endif

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            POINT p;
            p.x = DTK_GET_X_LPARAM(lParam);
            p.y = DTK_GET_Y_LPARAM(lParam);

            e.type = DTK_EVENT_MOUSE_BUTTON_DOWN;
            e.mouseButton.x = p.x;
            e.mouseButton.y = p.y;
            e.mouseButton.button = dtk_wm_event_to_mouse_button__win32(msg);
            e.mouseButton.state = dtk_get_mouse_event_state_flags__win32(wParam) | dtk_get_mouse_button_modifier_flag(e.mouseButton.button);
            dtk_handle_global_event(&e);
        } break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            POINT p;
            p.x = DTK_GET_X_LPARAM(lParam);
            p.y = DTK_GET_Y_LPARAM(lParam);

            e.type = DTK_EVENT_MOUSE_BUTTON_UP;
            e.mouseButton.x = p.x;
            e.mouseButton.y = p.y;
            e.mouseButton.button = dtk_wm_event_to_mouse_button__win32(msg);
            e.mouseButton.state = dtk_get_mouse_event_state_flags__win32(wParam);
            dtk_handle_global_event(&e);
        } break;

        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        {
            POINT p;
            p.x = DTK_GET_X_LPARAM(lParam);
            p.y = DTK_GET_Y_LPARAM(lParam);

            // Special case for double click because we want to post a normal mouse button down event first before the double-click event.
            e.type = DTK_EVENT_MOUSE_BUTTON_DOWN;
            e.mouseButton.x = p.x;
            e.mouseButton.y = p.y;
            e.mouseButton.button = dtk_wm_event_to_mouse_button__win32(msg);
            e.mouseButton.state = dtk_get_mouse_event_state_flags__win32(wParam) | dtk_get_mouse_button_modifier_flag(e.mouseButton.button);
            dtk_handle_global_event(&e);

            e.type = DTK_EVENT_MOUSE_BUTTON_DBLCLICK;
            dtk_handle_global_event(&e);
        } break;

        case WM_MOUSEWHEEL:
        {
            dtk_int32 delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

            POINT p;
            p.x = DTK_GET_X_LPARAM(lParam);
            p.y = DTK_GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            e.type = DTK_EVENT_MOUSE_WHEEL;
            e.mouseWheel.x = p.x;
            e.mouseWheel.y = p.y;
            e.mouseWheel.delta = delta;
            e.mouseWheel.state = dtk_get_mouse_event_state_flags__win32(wParam);
            dtk_handle_global_event(&e);
        } break;


        case WM_KEYDOWN:
        {
            if (!dtk_is_win32_mouse_button_key_code(wParam)) {
                int stateFlags = dtk_get_modifier_key_state_flags__win32();
                if ((lParam & (1 << 30)) != 0) {
                    stateFlags |= DTK_KEY_STATE_AUTO_REPEATED;
                }

                e.pControl = DTK_CONTROL(pTK->pWindowWithKeyboardCapture);
                e.type = DTK_EVENT_KEY_DOWN;
                e.keyDown.key = dtk_convert_key_from_win32(wParam);
                e.keyDown.state = stateFlags;
                dtk_handle_global_event(&e);
            }
        } break;

        case WM_KEYUP:
        {
            if (!dtk_is_win32_mouse_button_key_code(wParam)) {
                e.pControl = DTK_CONTROL(pTK->pWindowWithKeyboardCapture);
                e.type = DTK_EVENT_KEY_UP;
                e.keyUp.key = dtk_convert_key_from_win32(wParam);
                e.keyUp.state = dtk_get_modifier_key_state_flags__win32();
                dtk_handle_global_event(&e);
            }
        } break;

        // NOTE: WM_UNICHAR is not posted by Windows itself, but rather intended to be posted by applications. Thus, we need to use WM_CHAR. WM_CHAR
        //       posts events as UTF-16 code points. When the code point is a surrogate pair, we need to store it and wait for the next WM_CHAR event
        //       which will contain the other half of the pair.
        case WM_CHAR:
        {
            // Windows will post WM_CHAR events for keys we don't particularly want. We'll filter them out here (they will be processed by WM_KEYDOWN).
            if (wParam < 32 || wParam == 127) {      // 127 = ASCII DEL (will be triggered by CTRL+Backspace)
                if (wParam != VK_TAB  &&
                    wParam != VK_RETURN) {   // VK_RETURN = Enter Key.
                    break;
                }
            }

            if ((lParam & (1U << 31)) == 0) {    // Bit 31 will be 1 if the key was pressed, 0 if it was released.
                if (IS_HIGH_SURROGATE(wParam)) {
                    dtk_assert(pWindow->win32.utf16HighSurrogate == 0);
                    pWindow->win32.utf16HighSurrogate = (unsigned short)wParam;
                } else {
                    unsigned int character = (unsigned int)wParam;
                    if (IS_LOW_SURROGATE(wParam)) {
                        assert(IS_HIGH_SURROGATE(pWindow->win32.utf16HighSurrogate) != 0);
                        character = dtk_utf16pair_to_utf32_ch(pWindow->win32.utf16HighSurrogate, (unsigned short)wParam);
                    }

                    pWindow->win32.utf16HighSurrogate = 0;

                    int stateFlags = dtk_get_modifier_key_state_flags__win32();
                    if ((lParam & (1 << 30)) != 0) {
                        stateFlags |= DTK_KEY_STATE_AUTO_REPEATED;
                    }

                    e.pControl = DTK_CONTROL(pTK->pWindowWithKeyboardCapture);
                    e.type = DTK_EVENT_PRINTABLE_KEY_DOWN;
                    e.printableKeyDown.utf32 = character;
                    e.printableKeyDown.state = stateFlags;

                    int repeatCount = lParam & 0x0000FFFF;
                    for (int i = 0; i < repeatCount; ++i) {
                        dtk_handle_global_event(&e);
                    }
                }
            }
        } break;


        case WM_SETFOCUS:
        {
            // Only receive focus if the window is allowed to receive the keyboard capture.
            if (pTK->pWindowWithKeyboardCapture != pWindow) {
                if (dtk_control_is_keyboard_capture_allowed(e.pControl)) {
                    e.type = DTK_EVENT_CAPTURE_KEYBOARD;
                    
                    HWND hOldFocusedWnd = (HWND)wParam;
                    if (hOldFocusedWnd == NULL) {
                        e.captureKeyboard.pOldCapturedControl = NULL;
                    } else {
                        e.captureKeyboard.pOldCapturedControl = (dtk_control*)GetWindowLongPtrA(hOldFocusedWnd, 0);
                    }
                    
                    dtk_handle_global_event(&e);
                }
            }
        } break;

        case WM_KILLFOCUS:
        {
            // There's a few issues with releasing keyboard focus with Win32. First, the internally focused window may not be the _actual_
            // focused window from the perspective of DTK because it may have been disallowed keyboard focus. In this case, from DTK's
            // perspective, it was never focused in the first place and should never receive a release event.
            HWND hNewFocusedWnd = (HWND)wParam;
            if (hNewFocusedWnd == NULL) {
                // In this case it means the newly focused window is not part of this instance. It means an entirely different program has
                // probably got focus. In this case, just kill the focus of the window that currently has the keyboard capture.
                e.type = DTK_EVENT_RELEASE_KEYBOARD;
                e.pControl = DTK_CONTROL(pTK->pWindowWithKeyboardCapture);
                e.releaseKeyboard.pNewCapturedControl = NULL;
                dtk_handle_global_event(&e);
            } else {
                // In this case it means the newly focused window _is_ part of this instance. If the newly focused window is not allowed to
                // receive keyboard capture, do _not_ post a release event (because there will be no corresponding capture event).
                dtk_control* pOldFocusedWindow = (dtk_control*)pWindow;
                dtk_control* pNewFocusedWindow = (dtk_control*)GetWindowLongPtrA(hNewFocusedWnd, 0);
                if (pTK->pWindowWithKeyboardCapture == pWindow && dtk_control_is_keyboard_capture_allowed(pNewFocusedWindow)) {
                    e.type = DTK_EVENT_RELEASE_KEYBOARD;
                    e.pControl = pOldFocusedWindow;
                    e.releaseKeyboard.pNewCapturedControl = pNewFocusedWindow;
                    dtk_handle_global_event(&e);
                }
            }
        } break;

        case WM_CAPTURECHANGED:
        {
            // This message is posted to the window that has _lost_ the mouse capture. Since mouse capture is always explicit, the capture
            // event is never handled through the Win32 event handler, and is instead posted manually from dtk__capture_mouse_window().
            e.type = DTK_EVENT_RELEASE_MOUSE;
            dtk_handle_global_event(&e);
        } break;


        case WM_COMMAND:
        {
            if (HIWORD(wParam) == 1) {
                WORD acceleratorIndex = LOWORD(wParam);
                e.type = DTK_EVENT_ACCELERATOR;
                e.accelerator.key       = e.pTK->win32.pAccelerators[acceleratorIndex].key;
                e.accelerator.modifiers = e.pTK->win32.pAccelerators[acceleratorIndex].modifiers;
                e.accelerator.id        = e.pTK->win32.pAccelerators[acceleratorIndex].id;
                dtk_handle_global_event(&e);
            }
        } break;

        case WM_MENUCOMMAND:
        {
            MENUINFO mi;
            dtk_zero_object(&mi);
            mi.cbSize = sizeof(mi);
            mi.fMask = MIM_MENUDATA;
            if (GetMenuInfo((HMENU)lParam, &mi)) {
                dtk_menu* pMenu = (dtk_menu*)mi.dwMenuData;
                dtk_assert(pMenu != NULL);

                e.type = DTK_EVENT_MENU;
                e.menu.pMenu = pMenu;
                e.menu.itemIndex = (dtk_uint32)wParam;
                dtk_menu_get_item_id(pMenu, e.menu.itemIndex, &e.menu.itemID);
                dtk_handle_global_event(&e);
            }
        } break;

        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) == HTCLIENT) {
                SetCursor((HCURSOR)pWindow->win32.hCursor);
                return TRUE;
            }
        } break;


        // The WM_NCACTIVATE and WM_ACTIVATE events below are used for making it so the main top level window does not
        // lose it's activation visual style when a child popup window is activated. We use popup windows for things like
        // combo box drop-downs and auto-complete popups which don't look right if the parent window loses activation.
        //
        // Look at http://www.catch22.net/tuts/docking-toolbars-part-1 for a detailed explanation of this.
        case WM_NCACTIVATE:
        {
            BOOL keepActive = (BOOL)wParam;
            BOOL syncOthers = TRUE;

            for (dtk_window* pTrackedWindow = pTK->pFirstWindow; pTrackedWindow != NULL; pTrackedWindow = pTrackedWindow->pNextWindow) {
                if (pTrackedWindow->win32.hWnd == (HWND)lParam) {
                    keepActive = TRUE;
                    syncOthers = FALSE;
                    break;
                }
            }

            if (lParam == -1) {
                return DefWindowProc(hWnd, msg, keepActive, 0);
            }

            if (syncOthers) {
                for (dtk_window* pTrackedWindow = pTK->pFirstWindow; pTrackedWindow != NULL; pTrackedWindow = pTrackedWindow->pNextWindow) {
                    if (hWnd != pTrackedWindow->win32.hWnd && hWnd != (HWND)lParam) {
                        SendMessage((HWND)pTrackedWindow->win32.hWnd, msg, keepActive, -1);
                    }
                }
            }

            return DefWindowProc(hWnd, msg, keepActive, lParam);
        } break;

        case WM_ACTIVATE:
        {
            HWND hActivatedWnd   = NULL;
            HWND hDeactivatedWnd = NULL;
            BOOL isActivatedWindowOwnedByThis   = FALSE;
            BOOL isDeactivatedWindowOwnedByThis = FALSE;

            if (LOWORD(wParam) != WA_INACTIVE) {
                // Activated.
                hActivatedWnd   = hWnd;
                hDeactivatedWnd = (HWND)lParam;
                isActivatedWindowOwnedByThis = TRUE;
            } else {
                // Deactivated.
                hActivatedWnd   = (HWND)lParam;
                hDeactivatedWnd = hWnd;
                isDeactivatedWindowOwnedByThis = TRUE;
            }

            if (!isActivatedWindowOwnedByThis) {
                isActivatedWindowOwnedByThis = dtk_is_HWND_owned_by_this(pTK, hActivatedWnd);
            }
            if (!isDeactivatedWindowOwnedByThis) {
                isDeactivatedWindowOwnedByThis = dtk_is_HWND_owned_by_this(pTK, hDeactivatedWnd);
            }

            if (isActivatedWindowOwnedByThis && isDeactivatedWindowOwnedByThis) {
                // Both windows are owned the by application.
                if (LOWORD(wParam) != WA_INACTIVE) {
                    hActivatedWnd   = dtk_get_root_top_level_HWND(hActivatedWnd);
                    hDeactivatedWnd = dtk_get_root_top_level_HWND(hDeactivatedWnd);

                    if (hActivatedWnd != hDeactivatedWnd) {
                        if (hDeactivatedWnd != NULL) {
                            // on_deactivate(hDeactivatedWnd)
                        }

                        if (hActivatedWnd != NULL) {
                            // on_activate(hActivatedWnd)
                        }
                    }
                }
            } else {
                // The windows are not both owned by this manager.
                if (isDeactivatedWindowOwnedByThis) {
                    hDeactivatedWnd = dtk_get_root_top_level_HWND(hDeactivatedWnd);
                    if (hDeactivatedWnd != NULL) {
                        // on_deactivate(hDeactivatedWnd)
                    }
                }

                if (isActivatedWindowOwnedByThis) {
                    hActivatedWnd = dtk_get_root_top_level_HWND(hActivatedWnd);
                    if (hActivatedWnd != NULL) {
                        // on_activate(hDeactivatedWnd)
                    }
                }
            }
        } break;

        case 0x02E0: // WM_DPICHANGED
        {
            // TODO: Do something... maybe update an internal DPI scaling factor in dtk_window?
        } break;
        
        default: break;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

dtk_result dtk_window_init__win32(dtk_context* pTK, dtk_control* pParent, dtk_window_type type, const char* title, dtk_uint32 width, dtk_uint32 height, dtk_window* pWindow)
{
    (void)pTK;

    DWORD dwStyleEx = 0;
    DWORD dwStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    if (type == dtk_window_type_toplevel) {
        dwStyle   |= WS_OVERLAPPEDWINDOW;
    }
    if (type == dtk_window_type_dialog) {
        dwStyleEx |= WS_EX_DLGMODALFRAME;
        dwStyle   |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    }
    if (type == dtk_window_type_popup) {
        dwStyle   |= WS_POPUP;
    }

    const char* pClassName = DTK_WIN32_WINDOW_CLASS;
    if (type == dtk_window_type_popup) {
        pClassName = DTK_WIN32_WINDOW_CLASS_POPUP;
    }

    HWND hWnd = CreateWindowExA(dwStyleEx, pClassName, title, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, width, height, (pParent != NULL) ? (HWND)DTK_WINDOW(pParent)->win32.hWnd : NULL, NULL, NULL, NULL);
    if (hWnd == NULL) {
        return DTK_ERROR;
    }

    pWindow->win32.hWnd = (dtk_handle)hWnd;
    pWindow->win32.hCursor = pTK->win32.hCursorArrow;

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

dtk_result dtk_window_set_title__win32(dtk_window* pWindow, const char* title)
{
    if (!SetWindowTextA((HWND)pWindow->win32.hWnd, title)) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_window_set_size__win32(dtk_window* pWindow, dtk_uint32 width, dtk_uint32 height)
{
    RECT windowRect;
    RECT clientRect;
    if (!GetWindowRect((HWND)pWindow->win32.hWnd, &windowRect)) return DTK_ERROR;
    if (!GetClientRect((HWND)pWindow->win32.hWnd, &clientRect)) return DTK_ERROR;

    int windowFrameX = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
    int windowFrameY = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);

    assert(windowFrameX >= 0);
    assert(windowFrameY >= 0);

    int scaledWidth  = width  + windowFrameX;
    int scaledHeight = height + windowFrameY;
    if (!SetWindowPos((HWND)pWindow->win32.hWnd, NULL, 0, 0, scaledWidth, scaledHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE)) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_window_get_size__win32(dtk_window* pWindow, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    RECT rect;
    if (!GetClientRect((HWND)pWindow->win32.hWnd, &rect)) {
        return DTK_ERROR;
    }

    if (pWidth  != NULL) *pWidth  = rect.right - rect.left;
    if (pHeight != NULL) *pHeight = rect.bottom - rect.top;
    return DTK_SUCCESS;
}

dtk_result dtk_window_get_client_size__win32(dtk_window* pWindow, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    return dtk_window_get_size__win32(pWindow, pWidth, pHeight);
}

dtk_result dtk_window_set_absolute_position__win32(dtk_window* pWindow, dtk_int32 screenPosX, dtk_int32 screenPosY)
{
    // The absolute position of window's is relative to the screen.
    if (!SetWindowPos((HWND)pWindow->win32.hWnd, NULL, screenPosX, screenPosY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE)) {
        return DTK_ERROR;
    }

    DTK_CONTROL(pWindow)->absolutePosX = screenPosX;
    DTK_CONTROL(pWindow)->absolutePosY = screenPosY;

    return DTK_SUCCESS;
}

dtk_result dtk_window_get_absolute_position__win32(dtk_window* pWindow, dtk_int32* pScreenPosX, dtk_int32* pScreenPosY)
{
#if 1
    // NOTE: This is returning incorrect results for some reason. When fullscreen, it reports an extra 8 pixels
    //       on each side for some reason. Windows 10. http://stackoverflow.com/questions/34139450/getwindowrect-returns-a-size-including-invisible-borders
    RECT rect;
    if (!GetWindowRect((HWND)pWindow->win32.hWnd, &rect)) {
        return DTK_ERROR;
    }

    MapWindowPoints(HWND_DESKTOP, GetParent((HWND)pWindow->win32.hWnd), (LPPOINT) &rect, 2);

    if (pScreenPosX) *pScreenPosX = rect.left;
    if (pScreenPosY) *pScreenPosY = rect.top;
#else
    POINT pt;
    pt.x = 0;
    pt.y = 0;
    if (!ClientToScreen((HWND)pWindow->win32.hWnd, &pt)) {
        return DTK_ERROR;
    }

    if (pScreenPosX) *pScreenPosX = pt.x;
    if (pScreenPosY) *pScreenPosY = pt.y;
#endif

    return DTK_SUCCESS;
}

dtk_result dtk_window_get_client_absolute_position__win32(dtk_window* pWindow, dtk_int32* pScreenPosX, dtk_int32* pScreenPosY)
{
    POINT pt;
    pt.x = 0;
    pt.y = 0;
    if (!ClientToScreen((HWND)pWindow->win32.hWnd, &pt)) {
        return DTK_ERROR;
    }

    if (pScreenPosX) *pScreenPosX = pt.x;
    if (pScreenPosY) *pScreenPosY = pt.y;
    return DTK_SUCCESS;
}

dtk_result dtk_window_move_to_center_of_screen__win32(dtk_window* pWindow)
{
    MONITORINFO mi;
    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfoA(MonitorFromWindow((HWND)pWindow->win32.hWnd, MONITOR_DEFAULTTONEAREST), &mi)) {
        return DTK_ERROR;
    }

    LONG screenSizeX = mi.rcMonitor.right - mi.rcMonitor.left;
    LONG screenSizeY = mi.rcMonitor.bottom - mi.rcMonitor.top;

    dtk_uint32 windowSizeX;
    dtk_uint32 windowSizeY;
    dtk_window_get_size(pWindow, &windowSizeX, &windowSizeY);

    return dtk_window_set_absolute_position(pWindow, (screenSizeX - windowSizeX)/2, (screenSizeY - windowSizeY)/2);
}

dtk_result dtk_window_show__win32(dtk_window* pWindow, int mode)
{
    int nCmdShow[] = {
        SW_HIDE,            // DTK_HIDE
        SW_SHOWNORMAL,      // DTK_SHOW_NORMAL
        SW_SHOWMAXIMIZED,   // DTK_SHOW_MAXIMIZED
    };

    ShowWindow((HWND)pWindow->win32.hWnd, nCmdShow[mode]);
    return DTK_SUCCESS;
}

dtk_result dtk_window_bring_to_top__win32(dtk_window* pWindow)
{
    if (IsZoomed((HWND)pWindow->win32.hWnd)) {
        ShowWindow((HWND)pWindow->win32.hWnd, SW_SHOWMAXIMIZED);
    } else if (IsIconic((HWND)pWindow->win32.hWnd)) {
        ShowWindow((HWND)pWindow->win32.hWnd, SW_RESTORE);
    }

    SetForegroundWindow((HWND)pWindow->win32.hWnd);
    return DTK_SUCCESS;
}

dtk_bool32 dtk_window_is_maximized__win32(dtk_window* pWindow)
{
    return IsZoomed((HWND)pWindow->win32.hWnd);
}


dtk_result dtk_window_set_cursor__win32(dtk_window* pWindow, dtk_system_cursor_type cursor)
{
    dtk_context* pTK = DTK_CONTROL(pWindow)->pTK;

    HCURSOR hCursor = NULL;
    switch (cursor)
    {
        case dtk_system_cursor_type_text:
        {
            hCursor = (HCURSOR)pTK->win32.hCursorIBeam;
        } break;

        case dtk_system_cursor_type_cross:
        {
            hCursor = (HCURSOR)pTK->win32.hCursorCross;
        } break;

        case dtk_system_cursor_type_double_arrow_h:
        {
            hCursor = (HCURSOR)pTK->win32.hCursorSizeWE;
        } break;

        case dtk_system_cursor_type_double_arrow_v:
        {
            hCursor = (HCURSOR)pTK->win32.hCursorSizeNS;
        } break;


        case dtk_system_cursor_type_none:
        {
            hCursor = NULL;
        } break;

        //case dtk_systemcursor_type_arrow:
        case dtk_system_cursor_type_default:
        default:
        {
            hCursor = (HCURSOR)pTK->win32.hCursorArrow;
        } break;
    }

    pWindow->win32.hCursor = (HCURSOR)hCursor;

    // If the cursor is currently inside the window it needs to be changed right now.
    if (dtk_window_is_cursor_over(pWindow)) {
        SetCursor(hCursor);
    }

    return DTK_SUCCESS;
}

dtk_bool32 dtk_window_is_cursor_over__win32(dtk_window* pWindow)
{
    return pWindow->win32.isCursorOverClientArea;
}

dtk_result dtk_window_set_menu__win32(dtk_window* pWindow, dtk_menu* pMenu)
{
    if (!SetMenu((HWND)pWindow->win32.hWnd, (pMenu == NULL) ? NULL : (HMENU)pMenu->win32.hMenu)) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_window_show_popup_menu__win32(dtk_window* pWindow, dtk_menu* pMenu, int posX, int posY)
{
    POINT screenCoords;
    screenCoords.x = posX;
    screenCoords.y = posY;
    ClientToScreen((HWND)pWindow->win32.hWnd, &screenCoords);

    UINT flags = TPM_RIGHTBUTTON | TPM_HORIZONTAL | TPM_VERTICAL;
    int alignment = GetSystemMetrics(SM_MENUDROPALIGNMENT);
    if (alignment == 0) {
        flags |= TPM_RIGHTALIGN;
    }

    if (!TrackPopupMenuEx((HMENU)pMenu->win32.hMenu, flags, screenCoords.x, screenCoords.y, (HWND)pWindow->win32.hWnd, NULL)) {
        return DTK_FALSE;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_window_immediate_redraw__win32(dtk_window* pWindow, dtk_rect rect)
{
    RECT rectWin32;
    rectWin32.left   = (LONG)rect.left;
    rectWin32.top    = (LONG)rect.top;
    rectWin32.right  = (LONG)rect.right;
    rectWin32.bottom = (LONG)rect.bottom;

#if 0
    // Scheduled redraw.
    if (!InvalidateRect((HWND)pWindow->win32.hWnd, &rect, FALSE)) {
        return DTK_ERROR;
    }
#else
    // Immediate redraw.
    if (!RedrawWindow((HWND)pWindow->win32.hWnd, &rectWin32, NULL, RDW_INVALIDATE | RDW_UPDATENOW)) {
        return DTK_ERROR;
    }
#endif

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
#define DTK_WINDOW_GTK_PROPERTY_NAME_USERDATA    "dtk.userdata"

static int dtk_get_modifier_state_flags__gtk(guint stateFromGTK)
{
    int result = 0;

    if ((stateFromGTK & GDK_SHIFT_MASK) != 0) {
        result |= DTK_MODIFIER_SHIFT;
    }
    if ((stateFromGTK & GDK_CONTROL_MASK) != 0) {
        result |= DTK_MODIFIER_CTRL;
    }
    if ((stateFromGTK & GDK_MOD1_MASK) != 0) {
        result |= DTK_MODIFIER_ALT;
    }

    if ((stateFromGTK & GDK_BUTTON1_MASK) != 0) {
        result |= DTK_MODIFIER_MOUSE_BUTTON_LEFT;
    }
    if ((stateFromGTK & GDK_BUTTON2_MASK) != 0) {
        result |= DTK_MODIFIER_MOUSE_BUTTON_MIDDLE;
    }
    if ((stateFromGTK & GDK_BUTTON3_MASK) != 0) {
        result |= DTK_MODIFIER_MOUSE_BUTTON_RIGHT;
    }
    if ((stateFromGTK & GDK_BUTTON4_MASK) != 0) {
        result |= DTK_MODIFIER_MOUSE_BUTTON_4;
    }
    if ((stateFromGTK & GDK_BUTTON5_MASK) != 0) {
        result |= DTK_MODIFIER_MOUSE_BUTTON_5;
    }

    return result;
}

GdkModifierType dtk_accelerator_modifiers_to_gtk(dtk_uint32 modifiers)
{
    guint result = 0;
    if (modifiers & DTK_MODIFIER_SHIFT) {
        result |= GDK_SHIFT_MASK;
    }
    if (modifiers & DTK_MODIFIER_CTRL) {
        result |= GDK_CONTROL_MASK;
    }
    if (modifiers & DTK_MODIFIER_ALT) {
        result |= GDK_MOD1_MASK;
    }

    return (GdkModifierType)result;
}

dtk_uint32 dtk_accelerator_modifiers_from_gtk(GdkModifierType modifiers)
{
    dtk_uint32 result = 0;
    if (modifiers & GDK_SHIFT_MASK) {
        result |= DTK_MODIFIER_SHIFT;
    }
    if (modifiers & GDK_CONTROL_MASK) {
        result |= DTK_MODIFIER_CTRL;
    }
    if (modifiers & GDK_MOD1_MASK) {
        result |= DTK_MODIFIER_ALT;
    }

    return result;
}

static int dtk_from_gtk_mouse_button(guint buttonGTK)
{
    switch (buttonGTK) {
        case 1: return DTK_MOUSE_BUTTON_LEFT;
        case 2: return DTK_MOUSE_BUTTON_MIDDLE;
        case 3: return DTK_MOUSE_BUTTON_RIGHT;
        default: return (int)buttonGTK;
    }
}


static gboolean dtk_window__on_close__gtk(GtkWidget* pWidget, GdkEvent* pEvent, gpointer pUserData)
{
    (void)pWidget;
    (void)pEvent;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) return DTK_TRUE;
    
    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_CLOSE, DTK_CONTROL(pWindow));
    dtk_handle_global_event(&e);
    
    return DTK_TRUE;
}

static gboolean dtk_window__on_configure__gtk(GtkWidget* pWidget, GdkEventConfigure* pEvent, gpointer pUserData)
{
    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_FALSE;
    }

    if (pEvent->x != DTK_CONTROL(pWindow)->absolutePosX || pEvent->y != DTK_CONTROL(pWindow)->absolutePosY) {    
        // Position has changed.
        dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_MOVE, DTK_CONTROL(pWindow));
        e.move.x = pEvent->x;
        e.move.y = pEvent->y;
        dtk_handle_global_event(&e);
    }

    return DTK_FALSE;
}

static void dtk_window__on_hide__gtk(GtkWidget* pWidget, gpointer pUserData)
{
    (void)pWidget;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_HIDE, DTK_CONTROL(pWindow));
    dtk_handle_global_event(&e);
}

static void dtk_window__on_show__gtk(GtkWidget* pWidget, gpointer pUserData)
{
    (void)pWidget;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_SHOW, DTK_CONTROL(pWindow));
    dtk_handle_global_event(&e);

    if (pWindow->gtk.repositionOnShow) {
        gtk_window_move(GTK_WINDOW(pWidget), pWindow->gtk.desiredPositionX, pWindow->gtk.desiredPositionY);
        pWindow->gtk.repositionOnShow = DTK_FALSE;
    }

    gtk_widget_grab_focus(GTK_WIDGET(pWidget)); // <-- Is this needed?
}

static gboolean dtk_window__on_key_down__gtk(GtkWidget* pWidget, GdkEventKey* pEvent, gpointer pUserData)
{
    (void)pWidget;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_TRUE;
    }

    dtk_context* pTK = DTK_CONTROL(pWindow)->pTK;

    int stateFlags = dtk_get_modifier_state_flags__gtk(pEvent->state);
    // TODO: Check here if key is auto-repeated.

    // If the key is a tab key and there are modifiers we will need to simulate an accelerator because GTK doesn't let
    // us bind the tab key to an accelerator... sigh...
    if ((pEvent->keyval == GDK_KEY_Tab || pEvent->keyval == GDK_KEY_ISO_Left_Tab) && stateFlags != 0) {
        for (size_t i = 0; i < pTK->gtk.acceleratorCount; ++i) {
            dtk_accelerator_gtk* pAccel = &pTK->gtk.pAccelerators[i];
            if ((pAccel->accelerator.key == '\t' || pAccel->accelerator.key == DTK_KEY_TAB) && (int)pAccel->accelerator.modifiers == stateFlags) {
                dtk_event e;
                e.type = DTK_EVENT_ACCELERATOR;
                e.pTK = pTK;
                e.pControl = DTK_CONTROL(pWindow);
                e.accelerator.key = pTK->gtk.pAccelerators[i].accelerator.key;
                e.accelerator.modifiers = pTK->gtk.pAccelerators[i].accelerator.modifiers;
                e.accelerator.id = pTK->gtk.pAccelerators[i].accelerator.id;
                dtk_handle_global_event(&e);
                return DTK_FALSE;
            }
        }
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_KEY_DOWN, DTK_CONTROL(pWindow));
    e.keyDown.key = dtk_convert_key_from_gtk(pEvent->keyval);
    e.keyDown.state = stateFlags;
    dtk_handle_global_event(&e);


    // Printable keys. These are posted as UTF-32 code points.
    guint32 utf32 = gdk_keyval_to_unicode(pEvent->keyval);
    if (utf32 == 0) {
        if (pEvent->keyval == GDK_KEY_KP_Enter) {
            utf32 = '\r';
        }
        if (pEvent->keyval == GDK_KEY_KP_Tab || pEvent->keyval == GDK_KEY_ISO_Left_Tab || pEvent->keyval == GDK_KEY_Tab) {
            utf32 = '\t';
        }
    }

    if (utf32 != 0 && (stateFlags & DTK_MODIFIER_CTRL) == 0 && (stateFlags & DTK_MODIFIER_ALT) == 0) {
        if (!(utf32 < 32 || utf32 == 127) || utf32 == '\t' || utf32 == '\r') {
            e.type = DTK_EVENT_PRINTABLE_KEY_DOWN;
            e.printableKeyDown.utf32 = utf32;
            e.printableKeyDown.state = stateFlags;
            dtk_handle_global_event(&e);
        }
    }

    return DTK_FALSE;
}

static gboolean dtk_window__on_key_up__gtk(GtkWidget* pWidget, GdkEventKey* pEvent, gpointer pUserData)
{
    (void)pWidget;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_TRUE;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_KEY_UP, DTK_CONTROL(pWindow));
    e.keyUp.key = dtk_convert_key_from_gtk(pEvent->keyval);
    e.keyUp.state = dtk_get_modifier_state_flags__gtk(pEvent->state);
    dtk_handle_global_event(&e);

    return DTK_FALSE;
}

static gboolean dtk_window__on_receive_focus__gtk(GtkWidget* pWidget, GdkEventFocus* pEvent, gpointer pUserData)
{
    (void)pWidget;
    (void)pEvent;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_TRUE;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_CAPTURE_KEYBOARD, DTK_CONTROL(pWindow));
    dtk_handle_global_event(&e);

    return DTK_FALSE;
}

static gboolean dtk_window__on_lose_focus__gtk(GtkWidget* pWidget, GdkEventFocus* pEvent, gpointer pUserData)
{
    (void)pWidget;
    (void)pEvent;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_TRUE;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_RELEASE_KEYBOARD, DTK_CONTROL(pWindow));
    dtk_handle_global_event(&e);

    return DTK_FALSE;
}




static gboolean dtk_window_clientarea__on_draw__gtk(GtkWidget* pClientArea, cairo_t* cr, gpointer pUserData)
{
    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) return DTK_FALSE;

    double clipLeft;
    double clipTop;
    double clipRight;
    double clipBottom;
    cairo_clip_extents(cr, &clipLeft, &clipTop, &clipRight, &clipBottom);

    dtk_surface surface;
    if (dtk_surface_init_transient_cairo(DTK_CONTROL(pWindow)->pTK, cr, DTK_CONTROL(pWindow)->width, DTK_CONTROL(pWindow)->height, &surface) != DTK_SUCCESS) {
        return DTK_FALSE;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_PAINT, DTK_CONTROL(pWindow));
    e.paint.rect.left = clipLeft;
    e.paint.rect.top = clipTop;
    e.paint.rect.right = clipRight;
    e.paint.rect.bottom = clipBottom;
    e.paint.pSurface = &surface;
    dtk_handle_global_event(&e);
    
    dtk_surface_uninit(&surface);
    return DTK_FALSE;
}

static gboolean dtk_window_clientarea__on_configure__gtk(GtkWidget* pClientArea, GdkEventConfigure* pEvent, gpointer pUserData)
{
    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_FALSE;
    }

    // If the window's size has changed, it's panel and surface need to be resized, and then redrawn.
    if (pEvent->width != pWindow->gtk.configureClientWidth || pEvent->height != pWindow->gtk.configureClientHeight) {
        pWindow->gtk.configureClientWidth  = pEvent->width;
        pWindow->gtk.configureClientHeight = pEvent->height;

        // Size has changed.
        dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_SIZE, DTK_CONTROL(pWindow));
        e.size.width = pEvent->width;
        e.size.height = pEvent->height;
        dtk_handle_global_event(&e);

        // Invalidate the window to force a redraw.
        gtk_widget_queue_draw(pClientArea);
    }

    return DTK_FALSE;
}

static gboolean dtk_window_clientarea__on_mouse_enter__gtk(GtkWidget* pClientArea, GdkEventCrossing* pEvent, gpointer pUserData)
{
    (void)pClientArea;
    (void)pEvent;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_TRUE;
    }

    pWindow->gtk.isCursorOverClientArea = DTK_TRUE;
    gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(pWindow->gtk.pWidget)), GDK_CURSOR(pWindow->gtk.pCursor));

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_MOUSE_ENTER, DTK_CONTROL(pWindow));
    dtk_handle_global_event(&e);

    return DTK_FALSE;
}

static gboolean dtk_window_clientarea__on_mouse_leave__gtk(GtkWidget* pClientArea, GdkEventCrossing* pEvent, gpointer pUserData)
{
    (void)pClientArea;
    (void)pEvent;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_TRUE;
    }

    pWindow->gtk.isCursorOverClientArea = DTK_FALSE;

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_MOUSE_LEAVE, DTK_CONTROL(pWindow));
    dtk_handle_global_event(&e);

    return DTK_TRUE;
}

static gboolean dtk_window_clientarea__on_mouse_move__gtk(GtkWidget* pClientArea, GdkEventMotion* pEvent, gpointer pUserData)
{
    (void)pClientArea;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_TRUE;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_MOUSE_MOVE, DTK_CONTROL(pWindow));
    e.mouseMove.x = pEvent->x;
    e.mouseMove.y = pEvent->y;
    e.mouseMove.state = dtk_get_modifier_state_flags__gtk(pEvent->state);
    dtk_handle_global_event(&e);

    return DTK_FALSE;
}

static gboolean dtk_window_clientarea__on_mouse_button_down__gtk(GtkWidget* pClientArea, GdkEventButton* pEvent, gpointer pUserData)
{
    (void)pClientArea;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_TRUE;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_NONE, DTK_CONTROL(pWindow));
    if (pEvent->type == GDK_BUTTON_PRESS) {
        e.type = DTK_EVENT_MOUSE_BUTTON_DOWN;
    } else if (pEvent->type == GDK_2BUTTON_PRESS) {
        e.type = DTK_EVENT_MOUSE_BUTTON_DBLCLICK;
    }

    e.mouseButton.x = pEvent->x;
    e.mouseButton.y = pEvent->y;
    e.mouseButton.button = dtk_from_gtk_mouse_button(pEvent->button);
    e.mouseButton.state = dtk_get_modifier_state_flags__gtk(pEvent->state);
    dtk_handle_global_event(&e);

    return DTK_TRUE;
}

static gboolean dtk_window_clientarea__on_mouse_button_up__gtk(GtkWidget* pClientArea, GdkEventButton* pEvent, gpointer pUserData)
{
    (void)pClientArea;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_TRUE;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_MOUSE_BUTTON_UP, DTK_CONTROL(pWindow));
    e.mouseButton.x = pEvent->x;
    e.mouseButton.y = pEvent->y;
    e.mouseButton.button = dtk_from_gtk_mouse_button(pEvent->button);
    e.mouseButton.state = dtk_get_modifier_state_flags__gtk(pEvent->state);
    dtk_handle_global_event(&e);

    return DTK_TRUE;
}

static gboolean dtk_window_clientarea__on_mouse_wheel__gtk(GtkWidget* pClientArea, GdkEventScroll* pEvent, gpointer pUserData)
{
    (void)pClientArea;

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_TRUE;
    }

    gdouble delta_y = 0;
    if (pEvent->direction == GDK_SCROLL_UP) {
        delta_y = 1;
    } else if (pEvent->direction == GDK_SCROLL_DOWN) {
        delta_y = -1;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_MOUSE_WHEEL, DTK_CONTROL(pWindow));
    e.mouseWheel.x = pEvent->x;
    e.mouseWheel.y = pEvent->y;
    e.mouseWheel.delta = (dtk_int32)delta_y;
    e.mouseWheel.state = dtk_get_modifier_state_flags__gtk(pEvent->state);
    dtk_handle_global_event(&e);

    return DTK_TRUE;
}

static gboolean dtk_window_clientarea__on_grab_broken__gtk(GtkWidget* pWidget, GdkEventGrabBroken* pEvent, gpointer pUserData)
{
    (void)pWidget;

    // Only using this for mouse grabs.
    if (pEvent->keyboard == TRUE) {
        return DTK_FALSE;
    }

    // We only care about implicit broken grabs. Explicit ungrabs have their respective events posted manually.
    if (pEvent->implicit == TRUE) {
        return DTK_FALSE;
    }

    dtk_window* pWindow = (dtk_window*)pUserData;
    if (pWindow == NULL) {
        return DTK_TRUE;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pWindow)->pTK, DTK_EVENT_RELEASE_MOUSE, DTK_CONTROL(pWindow));
    dtk_handle_global_event(&e);

    return DTK_FALSE;
}


dtk_result dtk_window_init__gtk(dtk_context* pTK, dtk_control* pParent, dtk_window_type type, const char* title, dtk_uint32 width, dtk_uint32 height, dtk_window* pWindow)
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
        
    g_signal_connect(pClientArea, "draw",                 G_CALLBACK(dtk_window_clientarea__on_draw__gtk),              pWindow);
    g_signal_connect(pClientArea, "configure-event",      G_CALLBACK(dtk_window_clientarea__on_configure__gtk),         pWindow);
    g_signal_connect(pClientArea, "enter-notify-event",   G_CALLBACK(dtk_window_clientarea__on_mouse_enter__gtk),       pWindow);     // Mouse enter.
    g_signal_connect(pClientArea, "leave-notify-event",   G_CALLBACK(dtk_window_clientarea__on_mouse_leave__gtk),       pWindow);     // Mouse leave.
    g_signal_connect(pClientArea, "motion-notify-event",  G_CALLBACK(dtk_window_clientarea__on_mouse_move__gtk),        pWindow);     // Mouse move.
    g_signal_connect(pClientArea, "button-press-event",   G_CALLBACK(dtk_window_clientarea__on_mouse_button_down__gtk), pWindow);     // Mouse button down.
    g_signal_connect(pClientArea, "button-release-event", G_CALLBACK(dtk_window_clientarea__on_mouse_button_up__gtk),   pWindow);     // Mouse button up.
    g_signal_connect(pClientArea, "scroll-event",         G_CALLBACK(dtk_window_clientarea__on_mouse_wheel__gtk),       pWindow);     // Mouse wheel.
    g_signal_connect(pClientArea, "grab-broken-event",    G_CALLBACK(dtk_window_clientarea__on_grab_broken__gtk),       pWindow);     // Grab broken event (for mouse grabs)
    
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

    g_object_set_data(G_OBJECT(pWidget), DTK_WINDOW_GTK_PROPERTY_NAME_USERDATA, pWindow);
    
    gtk_widget_add_events(pWidget,
        GDK_KEY_PRESS_MASK      |
        GDK_KEY_RELEASE_MASK    |
        GDK_FOCUS_CHANGE_MASK);


    
    gtk_window_set_title(GTK_WINDOW(pWidget), title);
    if (type == dtk_window_type_toplevel) {
        gtk_window_set_resizable(GTK_WINDOW(pWidget), TRUE);
        gtk_window_resize(GTK_WINDOW(pWidget), (gint)width, (gint)height);
    } else if (type == dtk_window_type_dialog) {
        gtk_window_set_resizable(GTK_WINDOW(pWidget), FALSE);
        gtk_widget_set_size_request(GTK_WIDGET(pWidget), (gint)width, (gint)height);
        gtk_window_set_type_hint(GTK_WINDOW(pWidget), GDK_WINDOW_TYPE_HINT_DIALOG);
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pWidget), TRUE);
    } else if (type == dtk_window_type_popup) {
        gtk_window_set_resizable(GTK_WINDOW(pWidget), TRUE);
        gtk_widget_set_size_request(GTK_WIDGET(pWidget), (gint)width, (gint)height);
        gtk_window_set_type_hint(GTK_WINDOW(pWidget), GDK_WINDOW_TYPE_HINT_DOCK);
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pWidget), TRUE);
    }
    
    g_signal_connect(pWidget, "delete-event",      G_CALLBACK(dtk_window__on_close__gtk),         pWindow);     // Close
    g_signal_connect(pWidget, "configure-event",   G_CALLBACK(dtk_window__on_configure__gtk),     pWindow);     // Size/Move
    g_signal_connect(pWidget, "hide",              G_CALLBACK(dtk_window__on_hide__gtk),          pWindow);     // Hide.
    g_signal_connect(pWidget, "show",              G_CALLBACK(dtk_window__on_show__gtk),          pWindow);     // Show.
    g_signal_connect(pWidget, "key-press-event",   G_CALLBACK(dtk_window__on_key_down__gtk),      pWindow);     // Key down.
    g_signal_connect(pWidget, "key-release-event", G_CALLBACK(dtk_window__on_key_up__gtk),        pWindow);     // Key up.
    g_signal_connect(pWidget, "focus-in-event",    G_CALLBACK(dtk_window__on_receive_focus__gtk), pWindow);     // Receive focus.
    g_signal_connect(pWidget, "focus-out-event",   G_CALLBACK(dtk_window__on_lose_focus__gtk),    pWindow);     // Lose focus.
    
    gtk_container_add(GTK_CONTAINER(pWidget), pBox);

    pWindow->gtk.pWidget     = pWidget;
    pWindow->gtk.pBox        = pBox;
    pWindow->gtk.pClientArea = pClientArea;
    
    gtk_widget_show_all(GTK_WIDGET(pBox));
    gtk_widget_realize(pWidget);
    g_object_set_data(G_OBJECT(gtk_widget_get_window(pWidget)), "GtkWindow", pWidget);

    // Bind the accelerator group to every window for now.
    gtk_window_add_accel_group(GTK_WINDOW(pWidget), GTK_ACCEL_GROUP(pTK->gtk.pAccelGroup));

    return DTK_SUCCESS;
}

dtk_result dtk_window_uninit__gtk(dtk_window* pWindow)
{
    gtk_widget_destroy(GTK_WIDGET(pWindow->gtk.pClientArea));
    gtk_widget_destroy(GTK_WIDGET(pWindow->gtk.pBox));
    gtk_widget_destroy(GTK_WIDGET(pWindow->gtk.pWidget));
    return DTK_SUCCESS;
}

dtk_result dtk_window_set_title__gtk(dtk_window* pWindow, const char* title)
{
    gtk_window_set_title(GTK_WINDOW(pWindow->gtk.pWidget), title);
    return DTK_SUCCESS;
}

dtk_result dtk_window_set_size__gtk(dtk_window* pWindow, dtk_uint32 width, dtk_uint32 height)
{
    if (pWindow->gtk.pMenu != NULL) {
        GtkAllocation alloc;
        gtk_widget_get_allocation(GTK_WIDGET(pWindow->gtk.pMenu->gtk.pWidget), &alloc);

        height += alloc.height;
    }

    gtk_window_resize(GTK_WINDOW(pWindow->gtk.pWidget), (int)width, (int)height);
    return DTK_SUCCESS;
}

dtk_result dtk_window_get_size__gtk(dtk_window* pWindow, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    gint width;
    gint height;
    gtk_window_get_size(GTK_WINDOW(pWindow->gtk.pWidget), &width, &height);

    if (pWidth) *pWidth = width;
    if (pHeight) *pHeight = height;
    return DTK_SUCCESS;
}

dtk_result dtk_window_get_client_size__gtk(dtk_window* pWindow, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    GtkAllocation alloc;
    gtk_widget_get_allocation(GTK_WIDGET(pWindow->gtk.pClientArea), &alloc);

    if (pWidth) *pWidth = alloc.width;
    if (pHeight) *pHeight = alloc.height;
    return DTK_SUCCESS;
}

dtk_result dtk_window_set_absolute_position__gtk(dtk_window* pWindow, dtk_int32 screenPosX, dtk_int32 screenPosY)
{
    gtk_window_move(GTK_WINDOW(pWindow->gtk.pWidget), (gint)screenPosX, (gint)screenPosY);

    if (!gtk_widget_get_visible(pWindow->gtk.pWidget)) {
        pWindow->gtk.repositionOnShow = DTK_TRUE;
        pWindow->gtk.desiredPositionX = screenPosX;
        pWindow->gtk.desiredPositionY = screenPosY;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_window_get_absolute_position__gtk(dtk_window* pWindow, dtk_int32* pScreenPosX, dtk_int32* pScreenPosY)
{
    gint posX = 0;
    gint posY = 0;
    gtk_window_get_position(GTK_WINDOW(pWindow->gtk.pWidget), &posX, &posY);

    if (pScreenPosX) *pScreenPosX = (dtk_int32)posX;
    if (pScreenPosY) *pScreenPosY = (dtk_int32)posY;
    return DTK_SUCCESS;
}

dtk_result dtk_window_get_client_absolute_position__gtk(dtk_window* pWindow, dtk_int32* pPosX, dtk_int32* pPosY)
{
    GtkAllocation alloc;
    dtk_zero_object(&alloc);
    if (pWindow->gtk.pMenu != NULL) {
        gtk_widget_get_allocation(GTK_WIDGET(pWindow->gtk.pMenu->gtk.pWidget), &alloc);
    }

    if (pPosX) *pPosX = pWindow->control.absolutePosX;
    if (pPosY) *pPosY = pWindow->control.absolutePosY + alloc.height;
    return DTK_SUCCESS;
}

dtk_result dtk_window_move_to_center_of_screen__gtk(dtk_window* pWindow)
{
    gtk_window_set_position(GTK_WINDOW(pWindow->gtk.pWidget), GTK_WIN_POS_CENTER);
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

dtk_result dtk_window_bring_to_top__gtk(dtk_window* pWindow)
{
    gtk_window_present(GTK_WINDOW(pWindow->gtk.pWidget));
    gdk_flush();    // <-- Is this needed?

    return DTK_SUCCESS;
}

dtk_bool32 dtk_window_is_maximized__gtk(dtk_window* pWindow)
{
    return gtk_window_is_maximized(GTK_WINDOW(pWindow->gtk.pWidget));
}

dtk_result dtk_window_set_cursor__gtk(dtk_window* pWindow, dtk_system_cursor_type cursor)
{
    dtk_context* pTK = DTK_CONTROL(pWindow)->pTK;

    GdkCursor* pGTKCursor = NULL;
    switch (cursor)
    {
        case dtk_system_cursor_type_text:
        {
            pGTKCursor = GDK_CURSOR(pTK->gtk.pCursorIBeam);
        } break;

        case dtk_system_cursor_type_cross:
        {
            pGTKCursor = GDK_CURSOR(pTK->gtk.pCursorCross);
        } break;

        case dtk_system_cursor_type_double_arrow_h:
        {
            pGTKCursor = GDK_CURSOR(pTK->gtk.pCursorDoubleArrowH);
        } break;

        case dtk_system_cursor_type_double_arrow_v:
        {
            pGTKCursor = GDK_CURSOR(pTK->gtk.pCursorDoubleArrowV);
        } break;


        case dtk_system_cursor_type_none:
        {
            pGTKCursor = NULL;
        } break;

        //case dtk_systemcursor_type_arrow:
        case dtk_system_cursor_type_default:
        default:
        {
            pGTKCursor = GDK_CURSOR(pTK->gtk.pCursorDefault);
        } break;
    }

    pWindow->gtk.pCursor = pGTKCursor;

    // If the cursor is currently inside the window it needs to be changed right now.
    if (dtk_window_is_cursor_over(pWindow)) {
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(pWindow->gtk.pWidget)), GDK_CURSOR(pWindow->gtk.pCursor));
    }

    return DTK_SUCCESS;
}

dtk_bool32 dtk_window_is_cursor_over__gtk(dtk_window* pWindow)
{
    return pWindow->gtk.isCursorOverClientArea;
}

dtk_result dtk_window_set_menu__gtk(dtk_window* pWindow, dtk_menu* pMenu)
{
    // The old menu bar needs to be removed.
    if (pWindow->gtk.pMenu != NULL) {
        gtk_container_remove(GTK_CONTAINER(pWindow->gtk.pBox), GTK_WIDGET(pWindow->gtk.pMenu->gtk.pWidget));
    }

    // Add the new menu to the top.
    if (pMenu != NULL) {
        gtk_box_pack_start(GTK_BOX(pWindow->gtk.pBox), GTK_WIDGET(pMenu->gtk.pWidget), FALSE, FALSE, 0);
        gtk_box_reorder_child(GTK_BOX(pWindow->gtk.pBox), GTK_WIDGET(pMenu->gtk.pWidget), 0);
        gtk_widget_show(GTK_WIDGET(pMenu->gtk.pWidget));
    }

    pWindow->gtk.pMenu = pMenu;
    return DTK_SUCCESS;
}


#if !GTK_CHECK_VERSION(3, 22, 0)
typedef struct
{
    dtk_int32 posX;
    dtk_int32 posY;
} dtk_window_show_popup_menu__gtk__position_cb_data;

void dtk_window_show_popup_menu__gtk__position_cb(GtkMenu* pMenu, gint* pX, gint* pY, gboolean* pPushIn, gpointer pUserData)
{
    (void)pMenu;

    dtk_window_show_popup_menu__gtk__position_cb_data* pData = (dtk_window_show_popup_menu__gtk__position_cb_data*)pUserData;
    dtk_assert(pData != NULL);

    *pX = pData->posX;
    *pY = pData->posY;
    *pPushIn = FALSE;
}
#endif

dtk_result dtk_window_show_popup_menu__gtk(dtk_window* pWindow, dtk_menu* pMenu, dtk_int32 posX, dtk_int32 posY)
{
    GtkRequisition size;
    gtk_widget_get_preferred_size(GTK_WIDGET(pMenu->gtk.pWidget), NULL, &size);

    GtkRequisition menubarSize = {0, 0};
    if (pWindow->gtk.pMenu) {
        gtk_widget_get_preferred_size(GTK_WIDGET(pWindow->gtk.pMenu->gtk.pWidget), NULL, &menubarSize);
    }

#if GTK_CHECK_VERSION(3, 22, 0)
    GdkRectangle rect;
    rect.x = posX;
    rect.y = posY + menubarSize.height;
    rect.width = size.width;
    rect.height = size.height;
    gtk_menu_popup_at_rect(GTK_MENU(pMenu->gtk.pWidget), gtk_widget_get_window(GTK_WIDGET(pWindow->gtk.pWidget)), &rect, GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_WEST, NULL);
#else
    dtk_int32 windowPosX = pWindow->control.absolutePosX;
    dtk_int32 windowPosY = pWindow->control.absolutePosY;
    //dtk_window_get_absolute_position(pWindow, &windowPosX, &windowPosY);

    dtk_window_show_popup_menu__gtk__position_cb_data data;
    data.posX = posX + windowPosX;
    data.posY = posY + windowPosY + menubarSize.height;
    gtk_menu_popup(GTK_MENU(pMenu->gtk.pWidget), NULL, NULL, dtk_window_show_popup_menu__gtk__position_cb, &data, 0, gtk_get_current_event_time());
#endif

    return DTK_SUCCESS;
}

dtk_result dtk_window_immediate_redraw__gtk(dtk_window* pWindow, dtk_rect rect)
{
    gtk_widget_queue_draw_area(GTK_WIDGET(pWindow->gtk.pClientArea), (gint)rect.left, (gint)rect.top, (gint)(rect.right - rect.left), (gint)(rect.bottom - rect.top));

    // Redraw immediately.
    GdkWindow* pGDKWindow = gtk_widget_get_window(GTK_WIDGET(pWindow->gtk.pClientArea));
    if (pGDKWindow != NULL) {
        gdk_window_process_updates(pGDKWindow, TRUE);
    }

    return DTK_SUCCESS;
}
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void dtk_window__check_and_handle_mouse_enter_and_leave(dtk_window* pWindow, dtk_control* pNewControlUnderMouse)
{
    dtk_assert(pWindow != NULL);

    dtk_context* pTK = DTK_CONTROL(pWindow)->pTK;
    dtk_assert(pTK != NULL);

    dtk_control* pOldControlUnderMouse = pTK->pControlUnderMouse;
    if (pOldControlUnderMouse != pNewControlUnderMouse) {
        if (pTK->pControlWithMouseCapture == NULL) {    // <-- We don't change the enter/leave state when a control has captured the mouse.
            pTK->pControlUnderMouse = pNewControlUnderMouse;

            dtk_system_cursor_type newCursor = dtk_system_cursor_type_default;
            if (pNewControlUnderMouse != NULL) {
                newCursor = pNewControlUnderMouse->cursor;
            }

            // It's intuitive to check that the new cursor is different to the old one before trying to change it, but that is not actually
            // what we want to do because it's possible that the old and new controls are on different windows.
            dtk_window_set_cursor(pWindow, newCursor);

            dtk__post_mouse_leave_event_recursive(pTK, pNewControlUnderMouse, pOldControlUnderMouse);
            dtk__post_mouse_enter_event_recursive(pTK, pNewControlUnderMouse, pOldControlUnderMouse);
        }
    }
}

dtk_result dtk_window_init(dtk_context* pTK, dtk_control* pParent, dtk_window_type type, const char* title, dtk_uint32 width, dtk_uint32 height, dtk_event_proc onEvent, dtk_window* pWindow)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pWindow);

    if (title  == NULL) title = "";
    if (width  == 0) width  = 1;
    if (height == 0) height = 1;
    
    dtk_result result = dtk_control_init(pTK, pParent, DTK_CONTROL_TYPE_WINDOW, onEvent, DTK_CONTROL(pWindow));
    if (result != DTK_SUCCESS) {
        return result;
    }
    
    // Do an upwards traversal until we find the overarching window control.
    dtk_window* pParentWindow = dtk_control_get_window(pParent);
    
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_window_init__win32(pTK, DTK_CONTROL(pParentWindow), type, title, width, height, pWindow);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_window_init__gtk(pTK, DTK_CONTROL(pParentWindow), type, title, width, height, pWindow);
    }
#endif
    if (result != DTK_SUCCESS) {
        return result;
    }

    // Make sure the position attributes of the structure are updated.
    dtk_window_get_absolute_position(pWindow, &DTK_CONTROL(pWindow)->absolutePosX, &DTK_CONTROL(pWindow)->absolutePosY);

    // The size needs to be set to that of the client area.
    dtk_window_get_client_size(pWindow, &DTK_CONTROL(pWindow)->width, &DTK_CONTROL(pWindow)->height);

    if (type == dtk_window_type_toplevel) {
        pWindow->isTopLevel = DTK_TRUE;
    }
    if (type == dtk_window_type_dialog) {
        pWindow->isDialog = DTK_TRUE;
    }
    if (type == dtk_window_type_popup) {
        pWindow->isPopup = DTK_TRUE;

        // Popup windows are not currently allowed to receive the keyboard capture.
        dtk_control_forbid_keyboard_capture(DTK_CONTROL(pWindow));
    }

    // Window's are not shown by default.
    DTK_CONTROL(pWindow)->isHidden = DTK_TRUE;

    dtk__track_window(pTK, pWindow);
    return DTK_SUCCESS;
}

dtk_result dtk_window_uninit(dtk_window* pWindow)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    dtk__untrack_window(DTK_CONTROL(pWindow)->pTK, pWindow);

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


dtk_bool32 dtk_window__on_paint_control(dtk_control* pControl, dtk_rect* pRelativeRect, void* pUserData)
{
    dtk_assert(pControl != NULL);
    dtk_assert(pRelativeRect != NULL);

    // Don't draw window controls, but keep iterating.
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return DTK_TRUE;
    }

    dtk_event* pEvent = (dtk_event*)pUserData;
    dtk_assert(pEvent != NULL);

    dtk_int32 relativePosX;
    dtk_int32 relativePosY;
    dtk_control_get_relative_position(pControl, &relativePosX, &relativePosY);

    dtk_surface_push(pEvent->paint.pSurface);
    dtk_surface_translate(pEvent->paint.pSurface, relativePosX, relativePosY);
    dtk_surface_set_clip(pEvent->paint.pSurface, dtk_control_get_local_rect(pControl));

    dtk_event e = *pEvent;
    e.pControl = pControl;
    e.paint.rect = *pRelativeRect;
    dtk_handle_local_event(e.pTK, &e);

    return DTK_TRUE;
}

dtk_bool32 dtk_window__on_paint_control_finished(dtk_control* pControl, dtk_rect* pRelativeRect, void* pUserData)
{
    (void)pRelativeRect;

    dtk_assert(pControl != NULL);
    dtk_assert(pRelativeRect != NULL);

    // Don't draw window controls, but keep iterating.
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return DTK_TRUE;
    }

    dtk_event* pEvent = (dtk_event*)pUserData;
    dtk_assert(pEvent != NULL);

    dtk_surface_pop(pEvent->paint.pSurface);
    return DTK_TRUE;
}

dtk_bool32 dtk_window_default_event_handler(dtk_event* pEvent)
{
    if (pEvent == NULL) return DTK_FALSE;

    dtk_context* pTK = pEvent->pTK;
    dtk_assert(pTK != NULL);
    
    dtk_window* pWindow = DTK_WINDOW(pEvent->pControl);
    dtk_assert(pWindow != NULL);

    // The window's event handler is a bit different to other controls, because window's are the controls that generate events
    // for non-window child controls.
    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dtk_control_iterate_visible_controls(DTK_CONTROL(pWindow), pEvent->paint.rect, dtk_window__on_paint_control, dtk_window__on_paint_control_finished, DTK_CONTROL_ITERATION_SKIP_WINDOWS | DTK_CONTROL_ITERATION_ALWAYS_INCLUDE_CHILDREN, pEvent);
        } break;

        case DTK_EVENT_SIZE:
        {
            DTK_CONTROL(pWindow)->width  = pEvent->size.width;
            DTK_CONTROL(pWindow)->height = pEvent->size.height;
        } break;
            
        case DTK_EVENT_MOVE:
        {
            DTK_CONTROL(pWindow)->absolutePosX = pEvent->move.x;
            DTK_CONTROL(pWindow)->absolutePosY = pEvent->move.y;
        } break;

        case DTK_EVENT_MOUSE_LEAVE:
        {
            dtk_window__check_and_handle_mouse_enter_and_leave(pWindow, NULL);
            pTK->pWindowUnderMouse = NULL;
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            pTK->pWindowUnderMouse = pWindow;
            pTK->lastMousePosX = pEvent->mouseMove.x;
            pTK->lastMousePosY = pEvent->mouseMove.y;

            dtk_control* pNewControlUnderMouse = dtk_window_refresh_mouse_enter_leave_state(pWindow, pEvent->mouseMove.x, pEvent->mouseMove.y);

            dtk_control* pEventReceiver = pTK->pControlWithMouseCapture;
            if (pEventReceiver == NULL) {
                pEventReceiver = pNewControlUnderMouse;
            }

            if (pEventReceiver != NULL && pEventReceiver != DTK_CONTROL(pWindow)) {
                dtk_int32 relativeMousePosX = pEvent->mouseMove.x;
                dtk_int32 relativeMousePosY = pEvent->mouseMove.y;
                dtk_control_absolute_to_relative(pEventReceiver, &relativeMousePosX, &relativeMousePosY);

                dtk_event e = *pEvent;
                e.pControl = pEventReceiver;
                e.mouseMove.x = relativeMousePosX;
                e.mouseMove.y = relativeMousePosY;
                dtk_handle_local_event(pTK, &e);
            }
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        case DTK_EVENT_MOUSE_BUTTON_UP:
        case DTK_EVENT_MOUSE_BUTTON_DBLCLICK:
        {
            dtk_control* pEventReceiver = pTK->pControlWithMouseCapture;
            if (pEventReceiver == NULL) {
                pEventReceiver = pTK->pControlUnderMouse;
                if (pEventReceiver == NULL) {
                    // We'll get here if this message is posted without a prior mouse move event.
                    pEventReceiver = dtk_window_find_control_under_point(pWindow, pEvent->mouseButton.x, pEvent->mouseButton.y);
                }
            }

            if (pEventReceiver != NULL && pEventReceiver != DTK_CONTROL(pWindow)) {
                dtk_int32 relativeMousePosX = pEvent->mouseMove.x;
                dtk_int32 relativeMousePosY = pEvent->mouseMove.y;
                dtk_control_absolute_to_relative(pEventReceiver, &relativeMousePosX, &relativeMousePosY);

                dtk_event e = *pEvent;
                e.pControl = pEventReceiver;
                e.mouseButton.x = relativeMousePosX;
                e.mouseButton.y = relativeMousePosY;
                dtk_handle_local_event(pTK, &e);
            }
        } break;

        case DTK_EVENT_MOUSE_WHEEL:
        {
            dtk_control* pEventReceiver = pTK->pControlWithMouseCapture;
            if (pEventReceiver == NULL) {
                pEventReceiver = pTK->pControlUnderMouse;
                if (pEventReceiver == NULL) {
                    // We'll get here if this message is posted without a prior mouse move event.
                    pEventReceiver = dtk_window_find_control_under_point(pWindow, pEvent->mouseButton.x, pEvent->mouseButton.y);
                }
            }

            if (pEventReceiver != NULL && pEventReceiver != DTK_CONTROL(pWindow)) {
                dtk_int32 relativeMousePosX = pEvent->mouseMove.x;
                dtk_int32 relativeMousePosY = pEvent->mouseMove.y;
                dtk_control_absolute_to_relative(pEventReceiver, &relativeMousePosX, &relativeMousePosY);

                dtk_event e = *pEvent;
                e.pControl = pEventReceiver;
                e.mouseWheel.x = relativeMousePosX;
                e.mouseWheel.y = relativeMousePosY;
                dtk_handle_local_event(pTK, &e);
            }
        } break;

        case DTK_EVENT_KEY_DOWN:
        case DTK_EVENT_KEY_UP:
        case DTK_EVENT_PRINTABLE_KEY_DOWN:
        {
            if (pTK->pControlWithKeyboardCapture != NULL && pTK->pControlWithKeyboardCapture != DTK_CONTROL(pWindow)) {
                dtk_event e = *pEvent;
                e.pControl = pTK->pControlWithKeyboardCapture;
                dtk_handle_local_event(pTK, &e);
            }
        } break;

        case DTK_EVENT_CAPTURE_KEYBOARD:
        {
            pTK->pWindowWithKeyboardCapture = pWindow;
            if (!pWindow->isNextKeyboardCaptureExplicit) {
                if (pWindow->pLastDescendantWithKeyboardFocus != NULL) {
                    pTK->pControlWithKeyboardCapture = pWindow->pLastDescendantWithKeyboardFocus;

                    dtk_event eCapture = dtk_event_init(pTK, DTK_EVENT_CAPTURE_KEYBOARD, pWindow->pLastDescendantWithKeyboardFocus);
                    eCapture.captureKeyboard.pOldCapturedControl = pTK->pControlWithKeyboardCapture;
                    dtk_handle_local_event(pTK, &eCapture);
                }
            }

            pWindow->isNextKeyboardCaptureExplicit = DTK_FALSE;
        } break;

        case DTK_EVENT_RELEASE_KEYBOARD:
        {
            pTK->pWindowWithKeyboardCapture = NULL;

            // If the this keyboard release was explicit there is no need to post an event. If it was implicit (done by the operating
            // system in response to a window focus change or whatnot) we need to make sure we release the control with keyboard focus.
            if (!pWindow->isNextKeyboardReleaseExplicit) {
                if (pWindow->pLastDescendantWithKeyboardFocus != NULL) {
                    pTK->pControlWithKeyboardCapture = NULL;

                    dtk_event eRelease = dtk_event_init(pTK, DTK_EVENT_RELEASE_KEYBOARD, pWindow->pLastDescendantWithKeyboardFocus);
                    eRelease.releaseKeyboard.pNewCapturedControl = NULL;
                    dtk_handle_local_event(pTK, &eRelease);
                }
            }

            pWindow->isNextKeyboardReleaseExplicit = DTK_FALSE;
        } break;

        case DTK_EVENT_CAPTURE_MOUSE:
        {
            pTK->pWindowWithMouseCapture = pWindow;
            if (!pWindow->isNextMouseCaptureExplicit) {
                if (pWindow->pLastDescendantWithMouseCapture != NULL) {
                    pTK->pControlWithMouseCapture = pWindow->pLastDescendantWithMouseCapture;

                    dtk_event eCapture = dtk_event_init(pTK, DTK_EVENT_CAPTURE_MOUSE, pWindow->pLastDescendantWithMouseCapture);
                    eCapture.captureMouse.pOldCapturedControl = pTK->pControlWithMouseCapture;
                    dtk_post_local_event(pTK, &eCapture);   // Note that we post the event rather than handle it straight away because we need to ensure the event handler does not change the capture mid-event-handling due to a restriction with Win32 (and possibly others).
                }
            }

            pWindow->isNextMouseCaptureExplicit = DTK_FALSE;
        } break;

        case DTK_EVENT_RELEASE_MOUSE:
        {
            pTK->pWindowWithMouseCapture = NULL;
            if (!pWindow->isNextMouseReleaseExplicit) {
                if (pWindow->pLastDescendantWithMouseCapture != NULL) {
                    pTK->pControlWithMouseCapture = NULL;
                
                    dtk_event eRelease = dtk_event_init(pTK, DTK_EVENT_RELEASE_MOUSE, pWindow->pLastDescendantWithMouseCapture);
                    eRelease.releaseMouse.pNewCapturedControl = NULL;
                    dtk_post_local_event(pTK, &eRelease);   // Note that we post the event rather than handle it straight away because we need to ensure the event handler does not change the capture mid-event-handling due to a restriction with Win32 (and possibly others).
                }
            }

            pWindow->isNextMouseReleaseExplicit = DTK_FALSE;
        } break;
    }

    // NOTE: Do not call dtk_control_default_event_handler() here. Just return DTK_TRUE directly. Windows are handled in a special way.
    return DTK_TRUE;
}


dtk_result dtk_window_set_title(dtk_window* pWindow, const char* title)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    if (title == NULL) {
        title = "";
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_set_title__win32(pWindow, title);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_set_title__gtk(pWindow, title);
    }
#endif

    return result;
}


dtk_result dtk_window_set_size(dtk_window* pWindow, dtk_uint32 width, dtk_uint32 height)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;
    if (width  == 0) width  = 1;
    if (height == 0) height = 1;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_set_size__win32(pWindow, width, height);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_set_size__gtk(pWindow, width, height);
    }
#endif
    if (result != DTK_SUCCESS) {
        return result;
    }

    dtk_window_get_size(pWindow, &DTK_CONTROL(pWindow)->width, &DTK_CONTROL(pWindow)->height);
    return result;
}

dtk_result dtk_window_get_size(dtk_window* pWindow, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    if (pWidth) *pWidth = 0;
    if (pHeight) *pHeight = 0;
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_get_size__win32(pWindow, pWidth, pHeight);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_get_size__gtk(pWindow, pWidth, pHeight);
    }
#endif

    return result;
}

dtk_result dtk_window_get_client_size(dtk_window* pWindow, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    if (pWidth) *pWidth = 0;
    if (pHeight) *pHeight = 0;
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_get_client_size__win32(pWindow, pWidth, pHeight);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_get_client_size__gtk(pWindow, pWidth, pHeight);
    }
#endif

    return result;
}

dtk_result dtk_window_set_absolute_position(dtk_window* pWindow, dtk_int32 screenPosX, dtk_int32 screenPosY)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_set_absolute_position__win32(pWindow, screenPosX, screenPosY);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_set_absolute_position__gtk(pWindow, screenPosX, screenPosY);
    }
#endif

    return result;
}

dtk_result dtk_window_get_absolute_position(dtk_window* pWindow, dtk_int32* pScreenPosX, dtk_int32* pScreenPosY)
{
    if (pScreenPosX) *pScreenPosX = 0;
    if (pScreenPosY) *pScreenPosY = 0;
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_get_absolute_position__win32(pWindow, pScreenPosX, pScreenPosY);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_get_absolute_position__gtk(pWindow, pScreenPosX, pScreenPosY);
    }
#endif

    return result;
}

dtk_result dtk_window_set_relative_position(dtk_window* pWindow, dtk_int32 relativePosX, dtk_int32 relativePosY)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    dtk_control* pParent = dtk_control_get_parent(DTK_CONTROL(pWindow));
    if (pParent == NULL) {
        return dtk_window_set_absolute_position(pWindow, relativePosX, relativePosY);
    }

    // The relative position of a window needs to be relative to it's parent, but positioned in screen coordinates.
    dtk_assert(pParent != NULL);

    dtk_int32 parentScreenPosX = 0;
    dtk_int32 parentScreenPosY = 0;
    if (pParent->type == DTK_CONTROL_TYPE_WINDOW) {
        dtk_result result = dtk_window_get_client_absolute_position(DTK_WINDOW(pParent), &parentScreenPosX, &parentScreenPosY);
        if (result != DTK_SUCCESS) {
            return result;
        }
    } else {
        dtk_result result = dtk_control_get_screen_position(pParent, &parentScreenPosX, &parentScreenPosY);
        if (result != DTK_SUCCESS) {
            return result;
        }
    }
    
    return dtk_window_set_absolute_position(pWindow, parentScreenPosX + relativePosX, parentScreenPosY + relativePosY);
}

dtk_result dtk_window_get_relative_position(dtk_window* pWindow, dtk_int32* pRelativePosX, dtk_int32* pRelativePosY)
{
    dtk_control* pParent = dtk_control_get_parent(DTK_CONTROL(pWindow));
    if (pParent == NULL) {
        return dtk_window_get_absolute_position(pWindow, pRelativePosX, pRelativePosY);
    }

    dtk_assert(pParent != NULL);

    dtk_int32 parentScreenPosX = 0;
    dtk_int32 parentScreenPosY = 0;
    if (pParent->type == DTK_CONTROL_TYPE_WINDOW) {
        dtk_result result = dtk_window_get_client_absolute_position(DTK_WINDOW(pParent), &parentScreenPosX, &parentScreenPosY);
        if (result != DTK_SUCCESS) {
            return result;
        }
    } else {
        dtk_result result = dtk_control_get_screen_position(pParent, &parentScreenPosX, &parentScreenPosY);
        if (result != DTK_SUCCESS) {
            return result;
        }
    }

    if (pRelativePosX) *pRelativePosX = DTK_CONTROL(pWindow)->absolutePosX - parentScreenPosX;
    if (pRelativePosY) *pRelativePosY = DTK_CONTROL(pWindow)->absolutePosY - parentScreenPosY;
    return DTK_SUCCESS;
}

dtk_result dtk_window_get_client_absolute_position(dtk_window* pWindow, dtk_int32* pPosX, dtk_int32* pPosY)
{
    if (pPosX) *pPosX = 0;
    if (pPosY) *pPosY = 0;
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_get_client_absolute_position__win32(pWindow, pPosX, pPosY);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_get_client_absolute_position__gtk(pWindow, pPosX, pPosY);
    }
#endif

    return result;
}

dtk_result dtk_window_move_to_center(dtk_window* pWindow)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    if (DTK_CONTROL(pWindow)->pParent) {
        dtk_uint32 parentSizeX;
        dtk_uint32 parentSizeY;
        if (dtk_control_get_size(DTK_CONTROL(pWindow)->pParent, &parentSizeX, &parentSizeY) != DTK_SUCCESS) {
            return DTK_ERROR;
        }

        dtk_uint32 sizeX;
        dtk_uint32 sizeY;
        if (dtk_window_get_size(pWindow, &sizeX, &sizeY) != DTK_SUCCESS) {
            return DTK_ERROR;
        }

        dtk_int32 newRelativePosX = (parentSizeX - sizeX)/2;
        dtk_int32 newRelativePosY = (parentSizeY - sizeY)/2;
        return dtk_window_set_relative_position(pWindow, newRelativePosX, newRelativePosY);
    } else {
        dtk_result result = DTK_NO_BACKEND;
    #ifdef DTK_WIN32
        if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
            result = dtk_window_move_to_center_of_screen__win32(pWindow);
        }
    #endif
    #ifdef DTK_GTK
        if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
            result = dtk_window_move_to_center_of_screen__gtk(pWindow);
        }
    #endif

        return result;
    }
}

dtk_bool32 dtk_window_is_in_view(dtk_window* pWindow)
{
    if (pWindow == NULL) return DTK_FALSE;

    dtk_rect rect = dtk_control_get_absolute_rect(DTK_CONTROL(pWindow));
    if (rect.right < 0 || rect.bottom < 0) {
        return DTK_FALSE;
    }

    dtk_uint32 screenSizeX;
    dtk_uint32 screenSizeY;
    dtk_result result = dtk_get_screen_size(DTK_CONTROL(pWindow)->pTK, &screenSizeX, &screenSizeY);
    if (result != DTK_SUCCESS) {
        return DTK_FALSE;
    }

    if (rect.left > (dtk_int32)screenSizeX || rect.top > (dtk_int32)screenSizeY) {
        return DTK_FALSE;
    }

    return DTK_TRUE;
}

dtk_result dtk_window_move_into_view(dtk_window* pWindow)
{
    if (dtk_window_is_in_view(pWindow)) {
        return DTK_SUCCESS; // The window is already in view.
    }

    return dtk_window_move_to_center(pWindow);
}


dtk_rect dtk_window_get_client_rect(dtk_window* pWindow)
{
    if (pWindow == NULL) return dtk_rect_init(0, 0, 0, 0);
    return dtk_rect_init(0, 0, (dtk_int32)DTK_CONTROL(pWindow)->width, (dtk_int32)DTK_CONTROL(pWindow)->height);
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

    if (result == DTK_SUCCESS) {
        if (mode == DTK_HIDE) {
            DTK_CONTROL(pWindow)->isHidden = DTK_TRUE;
        } else {
            DTK_CONTROL(pWindow)->isHidden = DTK_FALSE;
        }
    }

    return result;
}

dtk_result dtk_window_bring_to_top(dtk_window* pWindow)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_bring_to_top__win32(pWindow);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_bring_to_top__gtk(pWindow);
    }
#endif

    return result;
}

dtk_bool32 dtk_window_is_maximized(dtk_window* pWindow)
{
    if (pWindow == NULL) return DTK_FALSE;

    dtk_bool32 result = DTK_FALSE;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_is_maximized__win32(pWindow);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_is_maximized__gtk(pWindow);
    }
#endif

    return result;
}


dtk_result dtk_window_set_cursor(dtk_window* pWindow, dtk_system_cursor_type cursor)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_set_cursor__win32(pWindow, cursor);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_set_cursor__gtk(pWindow, cursor);
    }
#endif

    return result;
}

dtk_bool32 dtk_window_is_cursor_over(dtk_window* pWindow)
{
    if (pWindow == NULL) return DTK_FALSE;

    dtk_bool32 result = DTK_FALSE;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_is_cursor_over__win32(pWindow);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_is_cursor_over__gtk(pWindow);
    }
#endif

    return result;
}

dtk_result dtk_window_set_menu(dtk_window* pWindow, dtk_menu* pMenu)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    // Only menu bars are allowed to be set on a window.
    if (pMenu != NULL && pMenu->type != dtk_menu_type_menubar) {
        return DTK_INVALID_ARGS;
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_set_menu__win32(pWindow, pMenu);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_set_menu__gtk(pWindow, pMenu);
    }
#endif

    return result;
}

dtk_result dtk_window_show_popup_menu(dtk_window* pWindow, dtk_menu* pMenu, int posX, int posY)
{
    if (pWindow == NULL || pMenu == NULL) return DTK_INVALID_ARGS;

    if (pMenu->type != dtk_menu_type_popup) {
        return DTK_INVALID_ARGS;
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_show_popup_menu__win32(pWindow, pMenu, posX, posY);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_show_popup_menu__gtk(pWindow, pMenu, posX, posY);
    }
#endif

    return result;
}


dtk_result dtk_window_scheduled_redraw(dtk_window* pWindow, dtk_rect rect)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;

    // If the rectangle does not have any volume, just return immediately and pretend it was drawn.
    if (!dtk_rect_has_volume(rect)) {
        return DTK_SUCCESS;
    }

    // When scheduling a redraw, we first enqueue an event into the paint queue. After that we post an event to the main message queue
    // to let it know there's a pending paint.
    return dtk_paint_queue_enqueue(&DTK_CONTROL(pWindow)->pTK->paintQueue, pWindow, rect);
}

dtk_result dtk_window_immediate_redraw(dtk_window* pWindow, dtk_rect rect)
{
    if (pWindow == NULL) return DTK_INVALID_ARGS;
    
    // If the rectangle does not have any volume, just return immediately and pretend it was drawn.
    if (!dtk_rect_has_volume(rect)) {
        return DTK_SUCCESS;
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_win32) {
        result = dtk_window_immediate_redraw__win32(pWindow, rect);
    }
#endif
#ifdef DTK_GTK
    if (DTK_CONTROL(pWindow)->pTK->platform == dtk_platform_gtk) {
        result = dtk_window_immediate_redraw__gtk(pWindow, rect);
    }
#endif

    return result;
}


typedef struct
{
    dtk_control* pControlUnderPoint;
    dtk_window* pWindow;
    dtk_int32 posX;
    dtk_int32 posY;
} dtk_window__find_control_under_point_iterator_cb__data;

dtk_bool32 dtk_window__find_control_under_point_iterator_cb(dtk_control* pControl, dtk_rect* pRelativeRect, void* pUserData)
{
    dtk_window__find_control_under_point_iterator_cb__data* pData = (dtk_window__find_control_under_point_iterator_cb__data*)pUserData;
    
    dtk_int32 relativePosX = pData->posX;
    dtk_int32 relativePosY = pData->posY;
    if (pControl != DTK_CONTROL(pData->pWindow)) {
        dtk_control_absolute_to_relative(pControl, &relativePosX, &relativePosY);
    }

    if (dtk_rect_contains_point(*pRelativeRect, relativePosX, relativePosY)) {
        if (pControl->onHitTest) {
            if (pControl->onHitTest(pControl, relativePosX, relativePosY)) {
                pData->pControlUnderPoint = pControl;
            }
        } else {
            pData->pControlUnderPoint = pControl;
        }
    }

    return DTK_TRUE; // Always return DR_TRUE to ensure the entire hierarchy is checked.
}

dtk_control* dtk_window_find_control_under_point(dtk_window* pWindow, dtk_int32 posX, dtk_int32 posY)
{
    if (pWindow == NULL) return NULL;

    dtk_window__find_control_under_point_iterator_cb__data data;
    data.pControlUnderPoint = NULL;
    data.pWindow = pWindow;
    data.posX = posX;
    data.posY = posY;
    dtk_control_iterate_visible_controls(DTK_CONTROL(pWindow), dtk_control_get_local_rect(DTK_CONTROL(pWindow)), dtk_window__find_control_under_point_iterator_cb, NULL, DTK_CONTROL_ITERATION_SKIP_WINDOWS, &data);

    return data.pControlUnderPoint;
}

dtk_control* dtk_window_refresh_mouse_enter_leave_state(dtk_window* pWindow, dtk_int32 mousePosX, dtk_int32 mousePosY)
{
    if (pWindow == NULL) return NULL;

    dtk_control* pNewControlUnderMouse = dtk_window_find_control_under_point(pWindow, mousePosX, mousePosY);
    dtk_window__check_and_handle_mouse_enter_and_leave(pWindow, pNewControlUnderMouse);

    return pNewControlUnderMouse;
}

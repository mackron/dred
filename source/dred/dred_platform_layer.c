// Copyright (C) 2016 David Reid. See included LICENSE file.

//////////////////////////////////////////////////////////////////
//
// Private Cross Platform
//
//////////////////////////////////////////////////////////////////

// Helper for creating the root GUI element of a window.
dred_control* dred_platform__create_root_gui_element(dred_context* pDred, dred_window* pWindow)
{
    dred_control* pRootGUIElement = dred_control_create(pDred, NULL, "RootGUIElement", sizeof(pWindow));
    if (pRootGUIElement == NULL) {
        return NULL;
    }

    memcpy(dred_control_get_extra_data(pRootGUIElement), &pWindow, sizeof(pWindow));
    return pRootGUIElement;
}


//////////////////////////////////////////////////////////////////
//
// Win32
//
//////////////////////////////////////////////////////////////////

#ifdef DRED_WIN32
static const char* g_WindowClass = "dred_WindowClass";
static const char* g_WindowClassTimer = "dred_WindowClass_Timer";

HWND g_hTimerWnd = NULL;

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

dred_key dred_win32_to_dred_key(WPARAM wParam)
{
    switch (wParam)
    {
    case VK_BACK:   return DRED_GUI_BACKSPACE;
    case VK_SHIFT:  return DRED_GUI_SHIFT;
    case VK_ESCAPE: return DRED_GUI_ESCAPE;
    case VK_PRIOR:  return DRED_GUI_PAGE_UP;
    case VK_NEXT:   return DRED_GUI_PAGE_DOWN;
    case VK_END:    return DRED_GUI_END;
    case VK_HOME:   return DRED_GUI_HOME;
    case VK_LEFT:   return DRED_GUI_ARROW_LEFT;
    case VK_UP:     return DRED_GUI_ARROW_UP;
    case VK_RIGHT:  return DRED_GUI_ARROW_RIGHT;
    case VK_DOWN:   return DRED_GUI_ARROW_DOWN;
    case VK_DELETE: return DRED_GUI_DELETE;
    case VK_F1:     return DRED_GUI_F1;
    case VK_F2:     return DRED_GUI_F2;
    case VK_F3:     return DRED_GUI_F3;
    case VK_F4:     return DRED_GUI_F4;
    case VK_F5:     return DRED_GUI_F5;
    case VK_F6:     return DRED_GUI_F6;
    case VK_F7:     return DRED_GUI_F7;
    case VK_F8:     return DRED_GUI_F8;
    case VK_F9:     return DRED_GUI_F9;
    case VK_F10:    return DRED_GUI_F10;
    case VK_F11:    return DRED_GUI_F11;
    case VK_F12:    return DRED_GUI_F12;

    default: break;
    }

    return (dred_key)wParam;
}

WORD dred_dred_key_to_win32(dred_key key)
{
    switch (key)
    {
    case DRED_GUI_BACKSPACE:   return VK_BACK;
    case DRED_GUI_SHIFT:       return VK_SHIFT;
    case DRED_GUI_ESCAPE:      return VK_ESCAPE;
    case DRED_GUI_PAGE_UP:     return VK_PRIOR;
    case DRED_GUI_PAGE_DOWN:   return VK_NEXT;
    case DRED_GUI_END:         return VK_END;
    case DRED_GUI_HOME:        return VK_HOME;
    case DRED_GUI_ARROW_LEFT:  return VK_LEFT;
    case DRED_GUI_ARROW_UP:    return VK_UP;
    case DRED_GUI_ARROW_RIGHT: return VK_RIGHT;
    case DRED_GUI_ARROW_DOWN:  return VK_DOWN;
    case DRED_GUI_DELETE:      return VK_DELETE;
    case DRED_GUI_F1:          return VK_F1;
    case DRED_GUI_F2:          return VK_F2;
    case DRED_GUI_F3:          return VK_F3;
    case DRED_GUI_F4:          return VK_F4;
    case DRED_GUI_F5:          return VK_F5;
    case DRED_GUI_F6:          return VK_F6;
    case DRED_GUI_F7:          return VK_F7;
    case DRED_GUI_F8:          return VK_F8;
    case DRED_GUI_F9:          return VK_F9;
    case DRED_GUI_F10:         return VK_F10;
    case DRED_GUI_F11:         return VK_F11;
    case DRED_GUI_F12:         return VK_F12;

    default: break;
    }

    return (WORD)key;
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


ACCEL dred_win32_to_ACCEL(dred_key key, uint32_t modifiers, WORD cmd)
{
    ACCEL a;
    a.key = dred_dred_key_to_win32(key);
    a.cmd = cmd;

    a.fVirt = FVIRTKEY;
    if (modifiers & DRED_KEY_STATE_SHIFT_DOWN) {
        a.fVirt |= FSHIFT;
    }
    if (modifiers & DRED_KEY_STATE_CTRL_DOWN) {
        a.fVirt |= FCONTROL;
    }
    if (modifiers & DRED_KEY_STATE_ALT_DOWN) {
        a.fVirt |= FALT;
    }

    return a;
}


LRESULT CALLBACK CALLBACK GenericWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
            dred_window_on_close(pWindow);
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


        case WM_MOUSELEAVE:
        {
            pWindow->isCursorOver = false;
            dred_window_on_mouse_leave(pWindow);
        } break;

        case WM_MOUSEMOVE:
        {
            // On Win32 we need to explicitly tell the operating system to post a WM_MOUSELEAVE event. The problem is that it needs to be re-issued when the
            // mouse re-enters the window. The easiest way to do this is to just call it in response to every WM_MOUSEMOVE event.
            if (!pWindow->isCursorOver) {
                dred_win32_track_mouse_leave_event(hWnd);

                pWindow->isCursorOver = true;
                dred_window_on_mouse_enter(pWindow);
            }

            dred_window_on_mouse_move(pWindow, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam));
        } break;



        case WM_NCLBUTTONDOWN:
        {
            POINT p;
            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_LEFT, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_LEFT_DOWN);
        } break;
        case WM_NCLBUTTONUP:
        {
            POINT p;
            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            dred_window_on_mouse_button_up(pWindow, DRED_GUI_MOUSE_BUTTON_LEFT, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam));

        } break;
        case WM_NCLBUTTONDBLCLK:
        {
            POINT p;
            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_LEFT, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_LEFT_DOWN);
            dred_window_on_mouse_button_dblclick(pWindow, DRED_GUI_MOUSE_BUTTON_LEFT, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_LEFT_DOWN);

        } break;

        case WM_LBUTTONDOWN:
        {
            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_LEFT_DOWN);
        } break;
        case WM_LBUTTONUP:
        {
            dred_window_on_mouse_button_up(pWindow, DRED_GUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam));
        } break;
        case WM_LBUTTONDBLCLK:
        {
            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_LEFT_DOWN);
            dred_window_on_mouse_button_dblclick(pWindow, DRED_GUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_LEFT_DOWN);
        } break;


        case WM_NCRBUTTONDOWN:
        {
            POINT p;
            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_RIGHT, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_RIGHT_DOWN);
        } break;
        case WM_NCRBUTTONUP:
        {
            POINT p;
            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            dred_window_on_mouse_button_up(pWindow, DRED_GUI_MOUSE_BUTTON_RIGHT, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam));
        } break;
        case WM_NCRBUTTONDBLCLK:
        {
            POINT p;
            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_RIGHT, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_RIGHT_DOWN);
            dred_window_on_mouse_button_dblclick(pWindow, DRED_GUI_MOUSE_BUTTON_RIGHT, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_RIGHT_DOWN);
        } break;

        case WM_RBUTTONDOWN:
        {
            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_RIGHT_DOWN);
        } break;
        case WM_RBUTTONUP:
        {
            dred_window_on_mouse_button_up(pWindow, DRED_GUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam));
        } break;
        case WM_RBUTTONDBLCLK:
        {
            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_RIGHT_DOWN);
            dred_window_on_mouse_button_dblclick(pWindow, DRED_GUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_RIGHT_DOWN);
        } break;


        case WM_NCMBUTTONDOWN:
        {
            POINT p;
            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_MIDDLE_DOWN);
        } break;
        case WM_NCMBUTTONUP:
        {
            POINT p;
            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            dred_window_on_mouse_button_up(pWindow, DRED_GUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam));
        } break;
        case WM_NCMBUTTONDBLCLK:
        {
            POINT p;
            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_MIDDLE_DOWN);
            dred_window_on_mouse_button_dblclick(pWindow, DRED_GUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_MIDDLE_DOWN);
        } break;

        case WM_MBUTTONDOWN:
        {
            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_MIDDLE_DOWN);
        } break;
        case WM_MBUTTONUP:
        {
            dred_window_on_mouse_button_up(pWindow, DRED_GUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam));
        } break;
        case WM_MBUTTONDBLCLK:
        {
            dred_window_on_mouse_button_down(pWindow, DRED_GUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_MIDDLE_DOWN);
            dred_window_on_mouse_button_dblclick(pWindow, DRED_GUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), dred_win32_get_mouse_event_state_flags(wParam) | DRED_MOUSE_BUTTON_MIDDLE_DOWN);
        } break;

        case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

            POINT p;
            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &p);

            dred_window_on_mouse_wheel(pWindow, delta, p.x, p.y, dred_win32_get_mouse_event_state_flags(wParam));
        } break;


        case WM_KEYDOWN:
        {
            if (!dred_is_win32_mouse_button_key_code(wParam))
            {
                int stateFlags = dred_win32_get_modifier_key_state_flags();
                if ((lParam & (1 << 30)) != 0) {
                    stateFlags |= DRED_KEY_STATE_AUTO_REPEATED;
                }

                dred_window_on_key_down(pWindow, dred_win32_to_dred_key(wParam), stateFlags);
            }
        } break;

        case WM_KEYUP:
        {
            if (!dred_is_win32_mouse_button_key_code(wParam))
            {
                int stateFlags = dred_win32_get_modifier_key_state_flags();
                dred_window_on_key_up(pWindow, dred_win32_to_dred_key(wParam), stateFlags);
            }
        } break;

        // NOTE: WM_UNICHAR is not posted by Windows itself, but rather intended to be posted by applications. Thus, we need to use WM_CHAR. WM_CHAR
        //       posts events as UTF-16 code points. When the code point is a surrogate pair, we need to store it and wait for the next WM_CHAR event
        //       which will contain the other half of the pair.
        case WM_CHAR:
        {
            // Windows will post WM_CHAR events for keys we don't particularly want. We'll filter them out here (they will be processed by WM_KEYDOWN).
            if (wParam < 32 || wParam == 127)       // 127 = ASCII DEL (will be triggered by CTRL+Backspace)
            {
                if (wParam != VK_TAB  &&
                    wParam != VK_RETURN)    // VK_RETURN = Enter Key.
                {
                    break;
                }
            }


            if ((lParam & (1U << 31)) == 0)     // Bit 31 will be 1 if the key was pressed, 0 if it was released.
            {
                if (IS_HIGH_SURROGATE(wParam))
                {
                    assert(pWindow->utf16HighSurrogate == 0);
                    pWindow->utf16HighSurrogate = (unsigned short)wParam;
                }
                else
                {
                    unsigned int character = (unsigned int)wParam;
                    if (IS_LOW_SURROGATE(wParam))
                    {
                        assert(IS_HIGH_SURROGATE(pWindow->utf16HighSurrogate) != 0);
                        character = dr_utf16pair_to_utf32_ch(pWindow->utf16HighSurrogate, (unsigned short)wParam);
                    }

                    pWindow->utf16HighSurrogate = 0;


                    int repeatCount = lParam & 0x0000FFFF;
                    for (int i = 0; i < repeatCount; ++i)
                    {
                        int stateFlags = dred_win32_get_modifier_key_state_flags();
                        if ((lParam & (1 << 30)) != 0) {
                            stateFlags |= DRED_KEY_STATE_AUTO_REPEATED;
                        }

                        dred_window_on_printable_key_down(pWindow, character, stateFlags);
                    }
                }
            }
        } break;


        case WM_MOVE:
        {
            // WM_MOVE passes it's position in client coordinates, but for dred's purposes the actual window position what we need.
            RECT rect;
            GetWindowRect(hWnd, &rect);
            dred_window_on_move(pWindow, rect.left, rect.top);

            // This technique will use the position of the client area.
            //dred_window_on_move(pWindow, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
        } break;

        case WM_SIZE:
        {
            pWindow->isMaximized = (wParam & SIZE_MAXIMIZED) != 0;
            dred_window_on_size(pWindow, LOWORD(lParam), HIWORD(lParam));
        } break;


        case WM_SETFOCUS:
        {
            dred_window_on_focus(pWindow);
        } break;

        case WM_KILLFOCUS:
        {
            dred_window_on_unfocus(pWindow);
        } break;


        case WM_COMMAND:
        {
            if (HIWORD(wParam) == 1) {
                WORD acceleratorIndex = LOWORD(wParam);
                dred_on_accelerator(pWindow->pDred, pWindow, acceleratorIndex);
            } else if (HIWORD(wParam) == 0) {
                WORD menuItemID = LOWORD(wParam);
                dred_menu_item* pMenuItem = dred_window_find_menu_item_by_id(pWindow, menuItemID);
                if (pMenuItem != NULL) {
                    dred_exec(pWindow->pDred, pMenuItem->command, NULL);
                }
            }
        } break;


        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) == HTCLIENT) {
                SetCursor(pWindow->hCursor);
                return TRUE;
            }
        } break;

        case WM_PAINT:
        {
            RECT rect;
            if (GetUpdateRect(hWnd, &rect, FALSE)) {
                drgui_draw(pWindow->pRootGUIElement, drgui_make_rect((float)rect.left, (float)rect.top, (float)rect.right, (float)rect.bottom), pWindow->pDrawingSurface);
            }
        } break;

        case WM_ERASEBKGND:  return 1;       // Never draw the background of the window - always leave that to dr_gui.
        default: break;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK TimerWindowProcWin32(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}


bool dred_platform_init__win32()
{
    // We'll be handling DPI ourselves. This should be done at the top.
    dr_win32_make_dpi_aware();

    // Need to call this to enable visual styles.
    INITCOMMONCONTROLSEX ctls;
    ctls.dwSize = sizeof(ctls);
    ctls.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&ctls);

    // Window classes.
    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.cbWndExtra    = sizeof(dred_window*);
    wc.lpfnWndProc   = (WNDPROC)GenericWindowProc;
    wc.lpszClassName = g_WindowClass;
    wc.hCursor       = LoadCursorA(NULL, MAKEINTRESOURCEA(32512));
    wc.hIcon         = LoadIconA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(101));
    wc.style         = CS_DBLCLKS;
    if (!RegisterClassExA(&wc)) {
        UnregisterClassA(g_WindowClass, NULL);
        return false;
    }

    WNDCLASSEXA timerWC;
    ZeroMemory(&timerWC, sizeof(timerWC));
    timerWC.cbSize        = sizeof(timerWC);
    timerWC.lpfnWndProc   = (WNDPROC)TimerWindowProcWin32;
    timerWC.lpszClassName = g_WindowClassTimer;
    timerWC.style         = CS_OWNDC;
    if (!RegisterClassExA(&timerWC)) {
        UnregisterClassA(g_WindowClass, NULL);
        return false;
    }

    g_hTimerWnd = CreateWindowExA(0, g_WindowClassTimer, "", 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
    if (g_hTimerWnd == NULL) {
        UnregisterClassA(g_WindowClass, NULL);
        UnregisterClassA(g_WindowClassTimer, NULL);
        return false;
    }

    return true;
}

void dred_platform_uninit__win32()
{
    DestroyWindow(g_hTimerWnd);
    g_hTimerWnd = NULL;

    UnregisterClassA(g_WindowClass, NULL);
    UnregisterClassA(g_WindowClassTimer, NULL);
}

int dred_platform_run__win32()
{
    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessageA(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            return -42; // Unknown error.
        }

        WNDPROC wndproc = (WNDPROC)GetWindowLongPtrA(msg.hwnd, GWLP_WNDPROC);
        if (wndproc == GenericWindowProc) {
            dred_window* pWindow = (dred_window*)GetWindowLongPtrA(msg.hwnd, 0);
            if (pWindow != NULL) {
                if (TranslateAcceleratorA(pWindow->hWnd, pWindow->hAccel, &msg)) {
                    continue;
                }
            }
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



dred_window* dred_window_create__win32__internal(dred_context* pDred, HWND hWnd)
{
    dred_window* pWindow = (dred_window*)calloc(1, sizeof(*pWindow));
    if (pWindow == NULL) {
        goto on_error;
    }

    pWindow->pDred = pDred;
    pWindow->hWnd = hWnd;
    pWindow->hCursor = LoadCursor(NULL, IDC_ARROW);
    pWindow->isShowingMenu = true;

    pWindow->pDrawingSurface = dr2d_create_surface_gdi_HWND(pDred->pDrawingContext, hWnd);
    if (pWindow->pDrawingSurface == NULL) {
        goto on_error;
    }

    pWindow->pRootGUIElement = dred_platform__create_root_gui_element(pDred, pWindow);
    if (pWindow->pRootGUIElement == NULL) {
        goto on_error;
    }

    // The GUI panel needs to have it's initial size set.
    unsigned int windowWidth;
    unsigned int windowHeight;
    dred_window_get_client_size(pWindow, &windowWidth, &windowHeight);
    drgui_set_size(pWindow->pRootGUIElement, (float)windowWidth, (float)windowHeight);


    // The dred window needs to be linked to the Win32 window handle so it can be accessed from the event handler.
    SetWindowLongPtrA(hWnd, 0, (LONG_PTR)pWindow);

    return pWindow;

on_error:
    dred_window_delete(pWindow);
    return NULL;
}

dred_window* dred_window_create__win32(dred_context* pDred)
{
    DWORD dwStyleEx = 0;
    DWORD dwStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW;
    HWND hWnd = CreateWindowExA(dwStyleEx, g_WindowClass, "dred", dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, NULL, NULL);
    if (hWnd == NULL) {
        return NULL;
    }

    return dred_window_create__win32__internal(pDred, hWnd);
}

dred_window* dred_window_create_dialog__win32(dred_window* pParentWindow, const char* title, unsigned int width, unsigned int height)
{
    if (pParentWindow == NULL) {
        return NULL;
    }

    DWORD dwExStyle = WS_EX_DLGMODALFRAME;
    DWORD dwStyle   = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    HWND hWnd = CreateWindowExA(dwExStyle, g_WindowClass, title, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, (int)width, (int)height, pParentWindow->hWnd, NULL, NULL, NULL);
    if (hWnd == NULL) {
        return NULL;
    }

    return dred_window_create__win32__internal(pParentWindow->pDred, hWnd);
}

void dred_window_delete__win32(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->pRootGUIElement) {
        dred_control_delete(pWindow->pRootGUIElement);
        pWindow->pRootGUIElement = NULL;
    }

    if (pWindow->pDrawingSurface) {
        dr2d_delete_surface(pWindow->pDrawingSurface);
        pWindow->pDrawingSurface = NULL;
    }

    if (pWindow->hAccel) {
        DestroyAcceleratorTable(pWindow->hAccel);
    }

    DestroyWindow(pWindow->hWnd);
    free(pWindow);
}

void dred_window_set_title__win32(dred_window* pWindow, const char* title)
{
    if (pWindow == NULL) {
        return;
    }

    if (title == NULL) {
        title = "dred";
    }

    SetWindowTextA(pWindow->hWnd, title);
}


void dred_window_set_size__win32(dred_window* pWindow, unsigned int newWidth, unsigned int newHeight)
{
    if (pWindow == NULL) {
        return;
    }

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
    if (pWindow == NULL) {
        if (pWidthOut) *pWidthOut = 0;
        if (pHeightOut) *pHeightOut = 0;
    }

    RECT rect;
    GetClientRect(pWindow->hWnd, &rect);

    if (pWidthOut != NULL) {
        *pWidthOut = rect.right - rect.left;
    }
    if (pHeightOut != NULL) {
        *pHeightOut = rect.bottom - rect.top;
    }
}

void dred_window_get_client_size__win32(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
    if (pWindow == NULL) {
        if (pWidthOut) *pWidthOut = 0;
        if (pHeightOut) *pHeightOut = 0;
    }

    RECT rect;
    GetClientRect(pWindow->hWnd, &rect);

    if (pWidthOut != NULL) {
        *pWidthOut = rect.right - rect.left;
    }
    if (pHeightOut != NULL) {
        *pHeightOut = rect.bottom - rect.top;
    }
}


void dred_window_set_position__win32(dred_window* pWindow, int posX, int posY)
{
    if (pWindow == NULL) {
        return;
    }

    SetWindowPos(pWindow->hWnd, NULL, posX, posY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void dred_window_get_position__win32(dred_window* pWindow, int* pPosXOut, int* pPosYOut)
{
    if (pWindow == NULL) {
        if (pPosXOut) *pPosXOut = 0;
        if (pPosYOut) *pPosYOut = 0;
    }

    RECT rect;
    GetWindowRect(pWindow->hWnd, &rect);

    if (pPosXOut) *pPosXOut = rect.left;
    if (pPosYOut) *pPosYOut = rect.top;
}


void dred_window_move_to_center__win32(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    int parentPosX = 0;
    int parentPosY = 0;
    int parentWidth  = 0;
    int parentHeight = 0;

    int windowWidth;
    int windowHeight;

    RECT rect;
    GetClientRect(pWindow->hWnd, &rect);
    windowWidth = rect.right - rect.left;
    windowHeight = rect.bottom - rect.top;

    HWND hParentWnd = GetParent(pWindow->hWnd);
    if (hParentWnd == NULL) {
        hParentWnd = GetWindow(pWindow->hWnd, GW_OWNER);
    }

    if (hParentWnd != NULL)
    {
        // Center on the parent.
        GetClientRect(hParentWnd, &rect);
        parentWidth = rect.right - rect.left;
        parentHeight = rect.bottom - rect.top;

        GetWindowRect(hParentWnd, &rect);
        parentPosX = rect.left;
        parentPosY = rect.top;
    }
    else
    {
        // Center on the monitor.
        MONITORINFO mi;
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(MONITORINFO);
        if (GetMonitorInfoA(MonitorFromWindow(pWindow->hWnd, 0), &mi))
        {
            parentWidth  = mi.rcMonitor.right - mi.rcMonitor.left;
            parentHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
        }
    }

    int windowPosX = dr_max(((parentWidth  - windowWidth)  / 2) + parentPosX, 0);
    int windowPosY = dr_max(((parentHeight - windowHeight) / 2) + parentPosY, 0);
    SetWindowPos(pWindow->hWnd, NULL, windowPosX, windowPosY, windowWidth, windowHeight, SWP_NOZORDER | SWP_NOSIZE);
}


void dred_window_show__win32(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    ShowWindow(pWindow->hWnd, SW_SHOWNORMAL);
}

void dred_window_show_maximized__win32(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    ShowWindow(pWindow->hWnd, SW_SHOWMAXIMIZED);
}

void dred_window_hide__win32(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    ShowWindow(pWindow->hWnd, SW_HIDE);
}

bool dred_window_is_maximized__win32(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return false;
    }

    return pWindow->isMaximized;
}


void dred_window_set_cursor__win32(dred_window* pWindow, dred_cursor_type cursor)
{
    if (pWindow == NULL) {
        return;
    }

    switch (cursor)
    {
        case dred_cursor_type_text:
        {
            pWindow->hCursor = LoadCursor(NULL, IDC_IBEAM);
        } break;

        case dred_cursor_type_cross:
        {
            pWindow->hCursor = LoadCursor(NULL, IDC_CROSS);
        } break;

        case dred_cursor_type_double_arrow_h:
        {
            pWindow->hCursor = LoadCursor(NULL, IDC_SIZEWE);
        } break;

        case dred_cursor_type_double_arrow_v:
        {
            pWindow->hCursor = LoadCursor(NULL, IDC_SIZENS);
        } break;


        case dred_cursor_type_none:
        {
            pWindow->hCursor = NULL;
        } break;

        //case cursor_type_arrow:
        case dred_cursor_type_default:
        default:
        {
            pWindow->hCursor = LoadCursor(NULL, IDC_ARROW);
        } break;
    }

    // If the cursor is currently inside the window it needs to be changed right now.
    if (dred_window_is_cursor_over(pWindow)) {
        SetCursor(pWindow->hCursor);
    }
}

bool dred_window_is_cursor_over__win32(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return false;
    }

    return pWindow->isCursorOver;
}


void dred_window_bind_accelerators__win32(dred_window* pWindow, dred_accelerator_table* pAcceleratorTable)
{
    if (pWindow == NULL) {
        return;
    }

    // The table needs to be deleted and recreated right from the start.
    if (pWindow->hAccel != NULL) {
        DestroyAcceleratorTable(pWindow->hAccel);
        pWindow->hAccel = NULL;
    }

    // pAcceleratorTable is allowed to be null, in which case this function is equivalent to simply unbinding the accelerators.
    if (pAcceleratorTable != NULL) {
        if (pAcceleratorTable->count < 1 || pAcceleratorTable->count > 32767) {
            return;
        }

        ACCEL* pAccels = (ACCEL*)malloc(pAcceleratorTable->count * sizeof(*pAccels));
        if (pAccels == NULL) {
            return;
        }

        for (size_t i = 0; i < pAcceleratorTable->count; ++i) {
            pAccels[i] = dred_win32_to_ACCEL(pAcceleratorTable->pAccelerators[i].key, pAcceleratorTable->pAccelerators[i].modifiers, (WORD)i);
        }

        pWindow->hAccel = CreateAcceleratorTableA(pAccels, (WORD)pAcceleratorTable->count);
        free(pAccels);
    }
}

void dred_window_set_menu__win32(dred_window* pWindow, dred_menu* pMenu)
{
    if (pWindow == NULL) {
        return;
    }

    // This validation check isn't actually needed for Win32, but I'm keeping it here for consistency with Linux/GTK.
    if (pMenu && pMenu->type != dred_menu_type_menubar) {
        return;
    }

    if (pMenu != NULL) {
        if (pWindow->isShowingMenu) {
            SetMenu(pWindow->hWnd, pMenu->hMenu);
        }
    } else {
        SetMenu(pWindow->hWnd, NULL);
    }

    pWindow->pMenu = pMenu;
}

void dred_window_hide_menu__win32(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->pMenu != NULL) {
        SetMenu(pWindow->hWnd, NULL);
    }

    pWindow->isShowingMenu = false;
}

void dred_window_show_menu__win32(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->pMenu != NULL) {
        SetMenu(pWindow->hWnd, pWindow->pMenu->hMenu);
    }

    pWindow->isShowingMenu = true;
}

void dred_window_show_popup_menu__win32(dred_window* pWindow, dred_menu* pMenu, int posX, int posY)
{
    if (pWindow == NULL || pMenu == NULL) {
        return;
    }

    POINT screenCoords;
    screenCoords.x = posX;
    screenCoords.y = posY;
    ClientToScreen(pWindow->hWnd, &screenCoords);

    UINT flags = TPM_RIGHTBUTTON | TPM_HORIZONTAL | TPM_VERTICAL;
    int alignment = GetSystemMetrics(SM_MENUDROPALIGNMENT);
    if (alignment == 0) {
        flags |= TPM_RIGHTALIGN;
    }

    TrackPopupMenuEx(pMenu->hMenu, flags, screenCoords.x, screenCoords.y, pWindow->hWnd, NULL);
}


//// MENUS ////

dred_menu* dred_menu_create__win32(dred_context* pDred, dred_menu_type type)
{
    if (pDred == NULL) {
        return NULL;
    }

    HMENU hMenu = NULL;
    if (type == dred_menu_type_popup) {
        hMenu = CreatePopupMenu();
    } else {
        hMenu = CreateMenu();
    }

    if (hMenu == NULL) {
        return NULL;
    }

    dred_menu* pMenu = (dred_menu*)calloc(1, sizeof(*pMenu));
    if (pMenu == NULL) {
        DestroyMenu(hMenu);
        return NULL;
    }

    pMenu->pDred = pDred;
    pMenu->type  = type;
    pMenu->hMenu = hMenu;

    return pMenu;
}

void dred_menu_delete__win32(dred_menu* pMenu)
{
    if (pMenu == NULL) {
        return;
    }

    if (pMenu->hMenu) {
        DestroyMenu(pMenu->hMenu);
    }

    free(pMenu);
}


dred_menu_item* dred_menu_item_create_and_append__win32__internal(dred_menu* pMenu, const char* text, uint16_t id, const char* command, dred_shortcut shortcut, dred_menu* pSubMenu, bool separator)
{
    if (pMenu == NULL) {
        return NULL;
    }


    if (separator)
    {
        AppendMenuA(pMenu->hMenu, MF_SEPARATOR, 0, NULL);
    }
    else
    {
        if (text == NULL) {
            text = "";
        }

        // The text needs to be transformed to show the shortcut string.
        char shortcutStr[256];
        dred_shortcut_to_string(shortcut, shortcutStr, sizeof(shortcutStr));

        char transformedText[256];
        if (shortcutStr[0] == '\0') {
            strncpy_s(transformedText, sizeof(transformedText), text, _TRUNCATE);
        } else {
            snprintf(transformedText, sizeof(transformedText), "%s\t%s", text, shortcutStr);
        }

        if (pSubMenu == NULL) {
            if (!AppendMenuA(pMenu->hMenu, MF_STRING, id, transformedText)) {
                return NULL;
            }
        } else {
            if (!AppendMenuA(pMenu->hMenu, MF_POPUP, (UINT_PTR)pSubMenu->hMenu, text)) {
                return NULL;
            }
        }
    }


    dred_menu_item* pItem = (dred_menu_item*)calloc(1, sizeof(*pItem));
    if (pItem == NULL) {
        return NULL;
    }

    pItem->id = id;
    pItem->index = pMenu->menuItemCount;
    pItem->command = gb_make_string(command);
    pItem->pSubMenu = pSubMenu;
    pItem->pOwnerMenu = pMenu;


    // Add the item to the list.
    dred_menu_item** ppNewMenuItems = (dred_menu_item**)realloc(pMenu->ppMenuItems, (pMenu->menuItemCount + 1) * sizeof(*ppNewMenuItems));
    if (ppNewMenuItems == NULL) {
        free(pItem);
        return NULL;
    }

    pMenu->ppMenuItems = ppNewMenuItems;
    pMenu->ppMenuItems[pMenu->menuItemCount++] = pItem;

    return pItem;
}

dred_menu_item* dred_menu_item_create_and_append__win32(dred_menu* pMenu, const char* text, uint16_t id, const char* command, dred_shortcut shortcut, dred_menu* pSubMenu)
{
    return dred_menu_item_create_and_append__win32__internal(pMenu, text, id, command, shortcut, pSubMenu, false);
}

dred_menu_item* dred_menu_item_create_and_append_separator__win32(dred_menu* pMenu)
{
    return dred_menu_item_create_and_append__win32__internal(pMenu, NULL, 0, NULL, dred_shortcut_none(), NULL, true);
}

void dred_menu_item_delete__win32(dred_menu_item* pItem)
{
    if (pItem == NULL) {
        return;
    }

    RemoveMenu(pItem->pOwnerMenu->hMenu, (UINT)pItem->index, MF_BYPOSITION);

    // Remove the item from the list.
    assert(pItem->pOwnerMenu->menuItemCount > 0);
    for (size_t i = pItem->index; i < pItem->pOwnerMenu->menuItemCount-1; ++i) {
        pItem->pOwnerMenu->ppMenuItems[i] = pItem->pOwnerMenu->ppMenuItems[i+1];
        pItem->pOwnerMenu->ppMenuItems[i]->index -= 1;
    }
    pItem->pOwnerMenu->menuItemCount -= 1;


    if (pItem->command) {
        gb_free_string(pItem->command);
    }

    free(pItem);
}

void dred_menu_item_enable__win32(dred_menu_item* pItem)
{
    if (pItem == NULL) {
        return;
    }

    EnableMenuItem(pItem->pOwnerMenu->hMenu, (UINT)pItem->index, MF_BYPOSITION | MF_ENABLED);
}

void dred_menu_item_disable__win32(dred_menu_item* pItem)
{
    if (pItem == NULL) {
        return;
    }

    EnableMenuItem(pItem->pOwnerMenu->hMenu, (UINT)pItem->index, MF_BYPOSITION | MF_GRAYED);
}



//// TIMERS ////

static VOID CALLBACK dred_timer_proc_win32(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    (void)hWnd;
    (void)uMsg;
    (void)dwTime;

    dred_timer* pTimer = (dred_timer*)idEvent;
    if (pTimer == NULL) {
        assert(false);
    }

    if (pTimer->callback != NULL) {
        pTimer->callback(pTimer, pTimer->pUserData);
    }
}

dred_timer* dred_timer_create__win32(unsigned int timeoutInMilliseconds, dred_timer_proc callback, void* pUserData)
{
    dred_timer* pTimer = malloc(sizeof(*pTimer));
    if (pTimer == NULL) {
        return NULL;
    }

    // On Win32 we need to associate the timer with a window.
    pTimer->tagWin32 = SetTimer(g_hTimerWnd, (UINT_PTR)pTimer, timeoutInMilliseconds, dred_timer_proc_win32);
    if (pTimer->tagWin32 == 0) {
        free(pTimer);
        return NULL;
    }

    pTimer->timeoutInMilliseconds = timeoutInMilliseconds;
    pTimer->callback              = callback;
    pTimer->pUserData             = pUserData;

    return pTimer;
}

void dred_timer_delete__win32(dred_timer* pTimer)
{
    if (pTimer == NULL) {
        return;
    }

    KillTimer(g_hTimerWnd, pTimer->tagWin32);
    free(pTimer);
}


//// Clipboard ////

bool dred_clipboard_set_text__win32(const char* text, size_t textLength)
{
    if (textLength == (size_t)-1) {
        textLength = strlen(text);
    }

    // We must ensure line endlings are normalized to \r\n. If we don't do this pasting won't work
    // correctly in things like Notepad.
    //
    // We allocate a buffer x2 the size of the original string to guarantee there will be enough room
    // for the extra \r character's we'll be adding.
    HGLOBAL hTextMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, ((textLength*2 + 1) * sizeof(char)));
    if (hTextMem == NULL) {
        return false;
    }

    char* textRN = GlobalLock(hTextMem);
    if (textRN == NULL) {
        GlobalFree(hTextMem);
        return false;
    }

    if (!OpenClipboard(NULL)) {
        GlobalFree(hTextMem);
        return false;
    }

    if (!EmptyClipboard()) {
        GlobalFree(hTextMem);
        CloseClipboard();
        return false;
    }

    while (*text != '\0' && textLength > 0)
    {
        if (text[0] == '\r' && textLength > 1 && text[1] == '\n')
        {
            *textRN++ = '\r';
            *textRN++ = '\n';

            text += 2;
            textLength -= 2;
        }
        else
        {
            if (*text == '\n') {
                *textRN++ = '\r';
            }

            *textRN++ = *text++;

            textLength -= 1;
        }
    }

    *textRN = '\0';


    GlobalUnlock(hTextMem);


    if (SetClipboardData(CF_TEXT, hTextMem) == NULL) {
        GlobalFree(hTextMem);
        CloseClipboard();
        return false;
    }

    CloseClipboard();

    return true;
}

char* dred_clipboard_get_text__win32()
{
    if (!IsClipboardFormatAvailable(CF_TEXT)) {
        return 0;
    }

    if (!OpenClipboard(NULL)) {
        return 0;
    }

    HGLOBAL hTextMem = GetClipboardData(CF_TEXT);
    if (hTextMem == NULL) {
        CloseClipboard();
        return 0;
    }

    char* textRN = GlobalLock(hTextMem);
    if (textRN == NULL) {
        CloseClipboard();
        return false;
    }

    size_t textRNLength = strlen(textRN);
    char* result = malloc(textRNLength + 1);
    strcpy_s(result, textRNLength + 1, textRN);

    GlobalUnlock(hTextMem);
    CloseClipboard();

    return result;
}

void dred_clipboard_free_text__win32(char* text)
{
    free(text);
}



//// WIN32 <-> GUI BINDING ////

static void dred_platform__on_global_capture_mouse__win32(dred_element* pElement)
{
    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow != NULL) {
        SetCapture(pWindow->hWnd);
    }
}

static void dred_platform__on_global_release_mouse__win32(dred_element* pElement)
{
    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow != NULL) {
        ReleaseCapture();
    }
}

static void dred_platform__on_global_capture_keyboard__win32(dred_element* pElement, dred_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;

    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow != NULL) {
        pWindow->pElementWithKeyboardCapture = pElement;
        SetFocus(pWindow->hWnd);
    }
}

static void dred_platform__on_global_release_keyboard__win32(dred_element* pElement, dred_element* pNewCapturedElement)
{
    (void)pNewCapturedElement;

    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow != NULL) {
        dred_window* pNewWindow = dred_get_element_window(pNewCapturedElement);
        if (pWindow != pNewWindow) {
            SetFocus(NULL);
        }
    }
}

static void dred_platform__on_global_dirty__win32(dred_element* pElement, dred_rect relativeRect)
{
    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow != NULL)
    {
        dred_rect absoluteRect = relativeRect;
        drgui_make_rect_absolute(pElement, &absoluteRect);


        RECT rect;
        rect.left   = (LONG)absoluteRect.left;
        rect.top    = (LONG)absoluteRect.top;
        rect.right  = (LONG)absoluteRect.right;
        rect.bottom = (LONG)absoluteRect.bottom;

#if 0
        // Scheduled redraw.
        InvalidateRect(pWindow->hWnd, &rect, FALSE);
#else
        // Immediate redraw.
        RedrawWindow(pWindow->hWnd, &rect, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
#endif
    }
}
#endif




//////////////////////////////////////////////////////////////////
//
// GTK+ 3
//
//////////////////////////////////////////////////////////////////

#ifdef DRED_GTK
int g_GTKMainLoopResultCode = 0;
GdkCursor* g_GTKCursor_Default = NULL;
GdkCursor* g_GTKCursor_IBeam = NULL;
GdkCursor* g_GTKCursor_Cross = NULL;
GdkCursor* g_GTKCursor_DoubleArrowH = NULL;
GdkCursor* g_GTKCursor_DoubleArrowV = NULL;

static dred_key dred_gtk_to_dred_key(guint keyval)
{
    switch (keyval)
    {
    case GDK_KEY_BackSpace: return DRED_GUI_BACKSPACE;
    case GDK_KEY_Shift_L:   return DRED_GUI_SHIFT;
    case GDK_KEY_Shift_R:   return DRED_GUI_SHIFT;
    case GDK_KEY_Escape:    return DRED_GUI_ESCAPE;
    case GDK_KEY_Page_Up:   return DRED_GUI_PAGE_UP;
    case GDK_KEY_Page_Down: return DRED_GUI_PAGE_DOWN;
    case GDK_KEY_End:       return DRED_GUI_END;
    case GDK_KEY_Home:      return DRED_GUI_HOME;
    case GDK_KEY_Left:      return DRED_GUI_ARROW_LEFT;
    case GDK_KEY_Up:        return DRED_GUI_ARROW_UP;
    case GDK_KEY_Right:     return DRED_GUI_ARROW_RIGHT;
    case GDK_KEY_Down:      return DRED_GUI_ARROW_DOWN;
    case GDK_KEY_Delete:    return DRED_GUI_DELETE;
    case GDK_KEY_F1:        return DRED_GUI_F1;
    case GDK_KEY_F2:        return DRED_GUI_F2;
    case GDK_KEY_F3:        return DRED_GUI_F3;
    case GDK_KEY_F4:        return DRED_GUI_F4;
    case GDK_KEY_F5:        return DRED_GUI_F5;
    case GDK_KEY_F6:        return DRED_GUI_F6;
    case GDK_KEY_F7:        return DRED_GUI_F7;
    case GDK_KEY_F8:        return DRED_GUI_F8;
    case GDK_KEY_F9:        return DRED_GUI_F9;
    case GDK_KEY_F10:       return DRED_GUI_F10;
    case GDK_KEY_F11:       return DRED_GUI_F11;
    case GDK_KEY_F12:       return DRED_GUI_F12;

    default: break;
    }

    if (keyval == GDK_KEY_Tab) {
        return '\t';
    }

    return (dred_key)keyval;
}

guint dred_dred_key_to_gtk(dred_key key)
{
    switch (key)
    {
    case DRED_GUI_BACKSPACE:   return GDK_KEY_BackSpace;
    case DRED_GUI_SHIFT:       return GDK_KEY_Shift_L;
    //case DRED_GUI_SHIFT:       return GDK_KEY_Shift_R;
    case DRED_GUI_ESCAPE:      return GDK_KEY_Escape;
    case DRED_GUI_PAGE_UP:     return GDK_KEY_Page_Up;
    case DRED_GUI_PAGE_DOWN:   return GDK_KEY_Page_Down;
    case DRED_GUI_END:         return GDK_KEY_End;
    case DRED_GUI_HOME:        return GDK_KEY_Begin;
    case DRED_GUI_ARROW_LEFT:  return GDK_KEY_Left;
    case DRED_GUI_ARROW_UP:    return GDK_KEY_Up;
    case DRED_GUI_ARROW_RIGHT: return GDK_KEY_Right;
    case DRED_GUI_ARROW_DOWN:  return GDK_KEY_Down;
    case DRED_GUI_DELETE:      return GDK_KEY_Delete;
    case DRED_GUI_F1:          return GDK_KEY_F1;
    case DRED_GUI_F2:          return GDK_KEY_F2;
    case DRED_GUI_F3:          return GDK_KEY_F3;
    case DRED_GUI_F4:          return GDK_KEY_F4;
    case DRED_GUI_F5:          return GDK_KEY_F5;
    case DRED_GUI_F6:          return GDK_KEY_F6;
    case DRED_GUI_F7:          return GDK_KEY_F7;
    case DRED_GUI_F8:          return GDK_KEY_F8;
    case DRED_GUI_F9:          return GDK_KEY_F9;
    case DRED_GUI_F10:         return GDK_KEY_F10;
    case DRED_GUI_F11:         return GDK_KEY_F11;
    case DRED_GUI_F12:         return GDK_KEY_F12;

    default: break;
    }

    if (key == '\t') {
        return GDK_KEY_Tab;
    }

    return (guint)key;
}

static int dred_gtk_get_modifier_state_flags(guint stateFromGTK)
{
    int result = 0;

    if ((stateFromGTK & GDK_SHIFT_MASK) != 0) {
        result |= DRED_KEY_STATE_SHIFT_DOWN;
    }
    if ((stateFromGTK & GDK_CONTROL_MASK) != 0) {
        result |= DRED_KEY_STATE_CTRL_DOWN;
    }
    if ((stateFromGTK & GDK_MOD1_MASK) != 0) {
        result |= DRED_KEY_STATE_ALT_DOWN;
    }

    if ((stateFromGTK & GDK_BUTTON1_MASK) != 0) {
        result |= DRED_MOUSE_BUTTON_LEFT_DOWN;
    }
    if ((stateFromGTK & GDK_BUTTON2_MASK) != 0) {
        result |= DRED_MOUSE_BUTTON_MIDDLE_DOWN;
    }
    if ((stateFromGTK & GDK_BUTTON3_MASK) != 0) {
        result |= DRED_MOUSE_BUTTON_RIGHT_DOWN;
    }
    if ((stateFromGTK & GDK_BUTTON4_MASK) != 0) {
        result |= DRED_MOUSE_BUTTON_4_DOWN;
    }
    if ((stateFromGTK & GDK_BUTTON5_MASK) != 0) {
        result |= DRED_MOUSE_BUTTON_5_DOWN;
    }

    return result;
}

guint dred_accelerator_modifiers_to_gtk(uint32_t modifiers)
{
    guint result = 0;
    if (modifiers & DRED_KEY_STATE_SHIFT_DOWN) {
        result |= GDK_SHIFT_MASK;
    }
    if (modifiers & DRED_KEY_STATE_CTRL_DOWN) {
        result |= GDK_CONTROL_MASK;
    }
    if (modifiers & DRED_KEY_STATE_ALT_DOWN) {
        result |= GDK_MOD1_MASK;
    }

    return result;
}

static int dred_from_gtk_mouse_button(guint buttonGTK)
{
    switch (buttonGTK) {
        case 1: return DRED_GUI_MOUSE_BUTTON_LEFT;
        case 2: return DRED_GUI_MOUSE_BUTTON_MIDDLE;
        case 3: return DRED_GUI_MOUSE_BUTTON_RIGHT;
        default: return (int)buttonGTK;
    }
}


bool dred_platform_init__gtk()
{
    gtk_init(NULL, NULL);

    g_GTKCursor_Default      = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_LEFT_PTR);
    g_GTKCursor_IBeam        = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_XTERM);
    g_GTKCursor_Cross        = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_CROSSHAIR);
    g_GTKCursor_DoubleArrowH = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_SB_H_DOUBLE_ARROW);
    g_GTKCursor_DoubleArrowV = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_SB_V_DOUBLE_ARROW);

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

    dred_window_on_close(pWindow);
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

    if (!dred_window_on_hide(pWindow, pWindow->onHideFlags)) {
        dred_window_show(pWindow);    // The event handler returned false, so prevent the window from being hidden.
    }
}

static void dred_gtk_cb__on_show(GtkWidget* pGTKWindow, gpointer pUserData)
{
    (void)pGTKWindow;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return;
    }

    if (!dred_window_on_show(pWindow)) {
        dred_window_hide(pWindow, DRED_HIDE_BLOCKED);    // The event handler returned false, so prevent the window from being shown.
    } else {
        gtk_widget_grab_focus(GTK_WIDGET(pWindow->pGTKWindow));
    }
}

static void dred_gtk_cb__on_paint(GtkWidget* pGTKWindow, cairo_t* pCairoContext, gpointer pUserData)
{
    (void)pGTKWindow;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return;
    }

    // NOTE: Because we are using dr_2d to draw the GUI, the last argument to drgui_draw() must be a pointer
    //       to the relevant dr2d_surface object.

    double clipLeft;
    double clipTop;
    double clipRight;
    double clipBottom;
    cairo_clip_extents(pCairoContext, &clipLeft, &clipTop, &clipRight, &clipBottom);

    dred_rect drawRect;
    drawRect.left   = (float)clipLeft;
    drawRect.top    = (float)clipTop;
    drawRect.right  = (float)clipRight;
    drawRect.bottom = (float)clipBottom;
    drgui_draw(pWindow->pRootGUIElement, drawRect, pWindow->pDrawingSurface);

    // At this point the GUI has been drawn, however nothing has been drawn to the window yet. To do this we will
    // use cairo directly with a cairo_set_source_surface() / cairo_paint() pair. We can get a pointer to dr_2d's
    // internal cairo_surface_t object as shown below.
    cairo_surface_t* pCairoSurface = dr2d_get_cairo_surface_t(pWindow->pDrawingSurface);
    if (pCairoSurface != NULL)
    {
        cairo_set_source_surface(pCairoContext, pCairoSurface, 0, 0);
        cairo_paint(pCairoContext);
    }
}

static bool dred_gtk_cb__on_configure(GtkWidget* pGTKWindow, GdkEventConfigure* pEvent, gpointer pUserData)
{
    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return false;
    }

    // If the window's size has changed, it's panel and surface need to be resized, and then redrawn.
    if (pEvent->width != dr2d_get_surface_width(pWindow->pDrawingSurface) || pEvent->height != dr2d_get_surface_height(pWindow->pDrawingSurface))
    {
        // Size has changed.

        // NOTE
        //
        // There's either an issue with GTK or I'm just not understanding something, but when the user resizes the window, for some
        // reason the internal clip used by GTK does not expand (it does shrink, though). The only way I've found to fix this is to
        // forcefully request an explicit resize which seems to cause GTK to correct it's clip. This is a bad hack and at some point
        // we should look at addressing this properly.
        //
        // UPDATE: It appears having everything contained within a child widget within a GtkBox container addresses this issue.
        //gtk_window_resize(GTK_WINDOW(pWindow->pGTKWindow), pEvent->width, pEvent->height);


        // dr_2d does not support dynamic resizing of surfaces. Thus, we need to delete and recreate it.
        if (pWindow->pDrawingSurface != NULL) {
            dr2d_delete_surface(pWindow->pDrawingSurface);
        }
        pWindow->pDrawingSurface = dr2d_create_surface(pWindow->pDred->pDrawingContext, (float)pEvent->width, (float)pEvent->height);


        // Post the event.
        dred_window_on_size(pWindow, pEvent->width, pEvent->height);

        // Invalidate the window to force a redraw.
        gtk_widget_queue_draw(pGTKWindow);
    }

    pWindow->absoluteClientPosX = (int)pEvent->x;
    pWindow->absoluteClientPosY = (int)pEvent->y;

    return false;
}

static bool dred_gtk_cb__on_configure__move(GtkWidget* pGTKWindow, GdkEventConfigure* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return false;
    }

    if (pWindow->windowPosX != pEvent->x || pWindow->windowPosY != pEvent->y) {
        int posX;
        int posY;
        dred_window_get_position(pWindow, &posX, &posY);
        dred_window_on_move(pWindow, posX, posY);

        //dred_window_on_move(pWindow, pEvent->x, pEvent->y);
    }

    pWindow->windowPosX = pEvent->x;
    pWindow->windowPosY = pEvent->y;

    return false;
}

static gboolean dred_gtk_cb__on_mouse_enter(GtkWidget* pGTKWindow, GdkEventCrossing* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;
    (void)pEvent;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    pWindow->isCursorOver = true;
    gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(pWindow->pGTKWindow)), pWindow->pGTKCursor);

    dred_window_on_mouse_enter(pWindow);
    return false;
}

static gboolean dred_gtk_cb__on_mouse_leave(GtkWidget* pGTKWindow, GdkEventCrossing* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;
    (void)pEvent;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    pWindow->isCursorOver = false;

    dred_window_on_mouse_leave(pWindow);
    return true;
}

static gboolean dred_gtk_cb__on_mouse_move(GtkWidget* pGTKWindow, GdkEventMotion* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    dred_window_on_mouse_move(pWindow, pEvent->x, pEvent->y, dred_gtk_get_modifier_state_flags(pEvent->state));
    return false;
}

static gboolean dred_gtk_cb__on_mouse_button_down(GtkWidget* pGTKWindow, GdkEventButton* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    if (pEvent->type == GDK_BUTTON_PRESS) {
        dred_window_on_mouse_button_down(pWindow, dred_from_gtk_mouse_button(pEvent->button), pEvent->x, pEvent->y, dred_gtk_get_modifier_state_flags(pEvent->state));
    } else if (pEvent->type == GDK_2BUTTON_PRESS) {
        dred_window_on_mouse_button_dblclick(pWindow, dred_from_gtk_mouse_button(pEvent->button), pEvent->x, pEvent->y, dred_gtk_get_modifier_state_flags(pEvent->state));
    }

    return true;
}

static gboolean dred_gtk_cb__on_mouse_button_up(GtkWidget* pGTKWindow, GdkEventButton* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    dred_window_on_mouse_button_up(pWindow, dred_from_gtk_mouse_button(pEvent->button), pEvent->x, pEvent->y, dred_gtk_get_modifier_state_flags(pEvent->state));
    return true;
}

static gboolean dred_gtk_cb__on_mouse_wheel(GtkWidget* pGTKWindow, GdkEventScroll* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    gdouble delta_y = 0;
    if (pEvent->direction == GDK_SCROLL_UP) {
        delta_y = -1;
    } else if (pEvent->direction == GDK_SCROLL_DOWN) {
        delta_y = 1;
    }

    dred_window_on_mouse_wheel(pWindow, (int)-delta_y, pEvent->x, pEvent->y, dred_gtk_get_modifier_state_flags(pEvent->state));

    return true;
}

static gboolean dred_gtk_cb__on_key_down(GtkWidget* pGTKWindow, GdkEventKey* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    int stateFlags = dred_gtk_get_modifier_state_flags(pEvent->state);
    // TODO: Check here if key is auto-repeated.

    // If the key is a tab key and there are modifiers we will need to simulate an accelerator because GTK does let
    // us bind the tab key to an accelerator... sigh...
    if ((pEvent->keyval == GDK_KEY_Tab || pEvent->keyval == GDK_KEY_ISO_Left_Tab) && stateFlags != 0) {
        for (size_t i = 0; i < pWindow->accelCount; ++i) {
            dred_gtk_accelerator* pAccel = &pWindow->pAccels[i];
            if (pAccel->accelerator.key == '\t' && (int)pAccel->accelerator.modifiers == stateFlags) {
                dred_on_accelerator(pAccel->pWindow->pDred, pAccel->pWindow, pAccel->index);
                return false;
            }
        }
    }

    dred_window_on_key_down(pWindow, dred_gtk_to_dred_key(pEvent->keyval), stateFlags);

    guint32 utf32 = gdk_keyval_to_unicode(pEvent->keyval);
    if (utf32 == 0) {
        if (pEvent->keyval == GDK_KEY_KP_Enter) {
            utf32 = '\r';
        }
    }

    if (utf32 != 0 && (stateFlags & DRED_KEY_STATE_CTRL_DOWN) == 0 && (stateFlags & DRED_KEY_STATE_ALT_DOWN) == 0) {
        if (!(utf32 < 32 || utf32 == 127) || utf32 == '\t' || utf32 == '\r') {
            dred_window_on_printable_key_down(pWindow, (unsigned int)utf32, stateFlags);
        }
    }

    return false;
}

static gboolean dred_gtk_cb__on_key_up(GtkWidget* pGTKWindow, GdkEventKey* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    dred_window_on_key_up(pWindow, dred_gtk_to_dred_key(pEvent->keyval), dred_gtk_get_modifier_state_flags(pEvent->state));
    return false;
}

static gboolean dred_gtk_cb__on_receive_focus(GtkWidget* pGTKWindow, GdkEventFocus* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;
    (void)pEvent;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    dred_window_on_focus(pWindow);
    return false;
}

static gboolean dred_gtk_cb__on_lose_focus(GtkWidget* pGTKWindow, GdkEventFocus* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;
    (void)pEvent;

    dred_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    dred_window_on_unfocus(pWindow);
    return false;
}


static void dred_gtk_cb__on_menu_item_activate(GtkWidget *pGTKMenuItem, gpointer pUserData)
{
    (void)pGTKMenuItem;
    //printf("Menu Pressed: %s\n", (const char*)pUserData);

    dred_menu_item* pItem = (dred_menu_item*)pUserData;
    if (pItem == NULL) {
        return;
    }

    if (pItem->command) {
        dred_exec(pItem->pDred, pItem->command, NULL);
    }
}

gboolean dred_gtk_cb__on_accelerator(GtkAccelGroup *pAccelGroup, GObject *acceleratable, guint keyval, GdkModifierType modifier, gpointer pUserData)
{
    (void)pAccelGroup;
    (void)acceleratable;
    (void)keyval;
    (void)modifier;

    dred_gtk_accelerator* pAccel = (dred_gtk_accelerator*)pUserData;
    assert(pAccel != NULL);

    dred_on_accelerator(pAccel->pWindow->pDred, pAccel->pWindow, pAccel->index);
    return true;    // Returning true here is important because it ensures the accelerator is handled only once.
}

GtkAccelGroup* dred_gtk__create_accels(dred_accelerator_table* pAcceleratorTable, dred_gtk_accelerator** ppAccelsOut, dred_window* pWindow, dred_menu* pMenu)
{
    if (pAcceleratorTable == NULL || ppAccelsOut == NULL) {
        return NULL;
    }

    GtkAccelGroup* pGTKAccelGroup = gtk_accel_group_new();
    if (pGTKAccelGroup == NULL) {
        return NULL;
    }

    dred_gtk_accelerator* pAccels = (dred_gtk_accelerator*)malloc(pAcceleratorTable->count * sizeof(*pAccels));
    if (pAccels == NULL) {
        g_object_unref(G_OBJECT(pGTKAccelGroup));
        pGTKAccelGroup = NULL;
        return NULL;
    }

    for (size_t i = 0; i < pAcceleratorTable->count; ++i) {
        dred_gtk_accelerator* pAccel = &pAccels[i];
        pAccel->index = i;
        pAccel->accelerator = pAcceleratorTable->pAccelerators[i];
        pAccel->pClosure = g_cclosure_new(G_CALLBACK(dred_gtk_cb__on_accelerator), pAccel, NULL);
        pAccel->pWindow = pWindow;
        pAccel->pMenu = pMenu;

        guint keyGTK = dred_dred_key_to_gtk(pAcceleratorTable->pAccelerators[i].key);
        GdkModifierType modifiersGTK = dred_accelerator_modifiers_to_gtk(pAcceleratorTable->pAccelerators[i].modifiers);
        if (keyGTK > 0) {
            gtk_accel_group_connect(pGTKAccelGroup, keyGTK, modifiersGTK, 0, pAccel->pClosure);
        }
    }

    *ppAccelsOut = pAccels;
    return pGTKAccelGroup;
}

void dred_gtk__delete_accels(dred_gtk_accelerator* pAccels, size_t accelCount)
{
    for (size_t i = 0; i < accelCount; ++i) {
        g_closure_unref(pAccels[i].pClosure);
    }

    free(pAccels);
}


dred_window* dred_window_create__gtk__internal(dred_context* pDred, GtkWidget* pGTKWindow)
{
    GtkWidget* pGTKBox = NULL;
    GtkWidget* pGTKClientArea = NULL;

    dred_window* pWindow = (dred_window*)calloc(1, sizeof(*pWindow));
    if (pWindow == NULL) {
        goto on_error;
    }

    pWindow->pDred = pDred;
    pWindow->pGTKWindow = pGTKWindow;
    pWindow->isShowingMenu = true;

    pWindow->pRootGUIElement = dred_platform__create_root_gui_element(pDred, pWindow);
    if (pWindow->pRootGUIElement == NULL) {
        goto on_error;
    }


    // These are the types of events we care about.
    gtk_widget_add_events(pGTKWindow,
        GDK_KEY_PRESS_MASK      |
        GDK_KEY_RELEASE_MASK    |
        GDK_FOCUS_CHANGE_MASK);

    g_signal_connect(pGTKWindow, "delete-event",         G_CALLBACK(dred_gtk_cb__on_close),             pWindow);     // Close.
    g_signal_connect(pGTKWindow, "configure-event",      G_CALLBACK(dred_gtk_cb__on_configure__move),   pWindow);     // Reposition and resize.
    g_signal_connect(pGTKWindow, "hide",                 G_CALLBACK(dred_gtk_cb__on_hide),              pWindow);     // Hide.
    g_signal_connect(pGTKWindow, "show",                 G_CALLBACK(dred_gtk_cb__on_show),              pWindow);     // Show.
    g_signal_connect(pGTKWindow, "key-press-event",      G_CALLBACK(dred_gtk_cb__on_key_down),          pWindow);     // Key down.
    g_signal_connect(pGTKWindow, "key-release-event",    G_CALLBACK(dred_gtk_cb__on_key_up),            pWindow);     // Key up.
    g_signal_connect(pGTKWindow, "focus-in-event",       G_CALLBACK(dred_gtk_cb__on_receive_focus),     pWindow);     // Receive focus.
    g_signal_connect(pGTKWindow, "focus-out-event",      G_CALLBACK(dred_gtk_cb__on_lose_focus),        pWindow);     // Lose focus.

    pGTKBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    if (pGTKBox == NULL) {
        goto on_error;
    }

    gtk_container_add(GTK_CONTAINER(pGTKWindow), pGTKBox);


    // Client area. This is the main content of the window.
    pGTKClientArea = gtk_drawing_area_new();
    if (pGTKClientArea == NULL) {
        goto on_error;
    }

    gtk_box_pack_start(GTK_BOX(pGTKBox), pGTKClientArea, TRUE, TRUE, 0);
    gtk_widget_show(pGTKClientArea);

    gtk_widget_add_events(pGTKClientArea,
        GDK_ENTER_NOTIFY_MASK   |
        GDK_LEAVE_NOTIFY_MASK   |
        GDK_POINTER_MOTION_MASK |
        GDK_BUTTON_PRESS_MASK   |
        GDK_BUTTON_RELEASE_MASK |
        GDK_SCROLL_MASK);

    g_signal_connect(pGTKClientArea, "draw",                 G_CALLBACK(dred_gtk_cb__on_paint),             pWindow);     // Paint
    g_signal_connect(pGTKClientArea, "configure-event",      G_CALLBACK(dred_gtk_cb__on_configure),         pWindow);     // Reposition and resize.
    g_signal_connect(pGTKClientArea, "enter-notify-event",   G_CALLBACK(dred_gtk_cb__on_mouse_enter),       pWindow);     // Mouse enter.
    g_signal_connect(pGTKClientArea, "leave-notify-event",   G_CALLBACK(dred_gtk_cb__on_mouse_leave),       pWindow);     // Mouse leave.
    g_signal_connect(pGTKClientArea, "motion-notify-event",  G_CALLBACK(dred_gtk_cb__on_mouse_move),        pWindow);     // Mouse move.
    g_signal_connect(pGTKClientArea, "button-press-event",   G_CALLBACK(dred_gtk_cb__on_mouse_button_down), pWindow);     // Mouse button down.
    g_signal_connect(pGTKClientArea, "button-release-event", G_CALLBACK(dred_gtk_cb__on_mouse_button_up),   pWindow);     // Mouse button up.
    g_signal_connect(pGTKClientArea, "scroll-event",         G_CALLBACK(dred_gtk_cb__on_mouse_wheel),       pWindow);     // Mouse wheel.


    // Show the box only after everything has been created.
    gtk_widget_show_all(pGTKBox);


    pWindow->pGTKBox = pGTKBox;
    pWindow->pGTKClientArea = pGTKClientArea;

    dred_window_get_position(pWindow, &pWindow->windowPosX, &pWindow->windowPosY);

    return pWindow;

on_error:
    dred_window_delete(pWindow);
    return NULL;
}

dred_window* dred_window_create__gtk(dred_context* pDred)
{
    GtkWidget* pGTKWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (pGTKWindow == NULL) {
        return NULL;
    }

    gtk_window_set_resizable(GTK_WINDOW(pGTKWindow),TRUE);


    return dred_window_create__gtk__internal(pDred, pGTKWindow);
}

dred_window* dred_window_create_dialog__gtk(dred_window* pParentWindow, const char* title, unsigned int width, unsigned int height)
{
    if (pParentWindow == NULL) {
        return NULL;
    }

    GtkWidget* pGTKWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (pGTKWindow == NULL) {
        return NULL;
    }

    gtk_window_set_type_hint(GTK_WINDOW(pGTKWindow), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pGTKWindow), TRUE);
    gtk_window_set_title(GTK_WINDOW(pGTKWindow), title);
    gtk_window_resize(GTK_WINDOW(pGTKWindow), (gint)width, (gint)height);
    gtk_widget_set_size_request(pGTKWindow, (gint)width, (gint)height);
    gtk_window_set_resizable(GTK_WINDOW(pGTKWindow),FALSE);

    return dred_window_create__gtk__internal(pParentWindow->pDred, pGTKWindow);
}

void dred_window_delete__gtk(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->pGTKAccelGroup) {
        g_object_unref(G_OBJECT(pWindow->pGTKAccelGroup));
    }

    if (pWindow->pAccels != NULL) {
        dred_gtk__delete_accels(pWindow->pAccels, pWindow->accelCount);
    }

    dred_control_delete(pWindow->pRootGUIElement);
    dr2d_delete_surface(pWindow->pDrawingSurface);

    gtk_widget_destroy(pWindow->pGTKClientArea);
    gtk_widget_destroy(pWindow->pGTKBox);
    gtk_widget_destroy(pWindow->pGTKWindow);
    free(pWindow);
}


void dred_window_set_title__gtk(dred_window* pWindow, const char* title)
{
    if (pWindow == NULL) {
        return;
    }

    if (title == NULL) {
        title = "dred";
    }

    gtk_window_set_title(GTK_WINDOW(pWindow->pGTKWindow), title);
}


void dred_window_set_size__gtk(dred_window* pWindow, unsigned int newWidth, unsigned int newHeight)
{
    // We need to adjust the height of the window based on the menu.
    if (pWindow->pMenu != NULL) {
        GtkAllocation alloc;
        gtk_widget_get_allocation(pWindow->pMenu->pGTKMenu, &alloc);

        newHeight += alloc.height;
    }

    gtk_window_resize(GTK_WINDOW(pWindow->pGTKWindow), (int)newWidth, (int)newHeight);
}

void dred_window_get_size__gtk(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
    int width;
    int height;
    gtk_window_get_size(GTK_WINDOW(pWindow->pGTKWindow), &width, &height);

    if (pWidthOut) *pWidthOut = width;
    if (pHeightOut) *pHeightOut = height;
}

void dred_window_get_client_size__gtk(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
    GtkAllocation alloc;
    gtk_widget_get_allocation(pWindow->pGTKClientArea, &alloc);

    if (pWidthOut) *pWidthOut = alloc.width;
    if (pHeightOut) *pHeightOut = alloc.height;
}


void dred_window_set_position__gtk(dred_window* pWindow, int posX, int posY)
{
    if (pWindow == NULL) {
        return;
    }

    gtk_window_move(GTK_WINDOW(pWindow->pGTKWindow), posX, posY);
}

void dred_window_get_position__gtk(dred_window* pWindow, int* pPosXOut, int* pPosYOut)
{
    if (pWindow == NULL) {
        if (pPosXOut) *pPosXOut = 0;
        if (pPosYOut) *pPosYOut = 0;
    }

    gtk_window_get_position(GTK_WINDOW(pWindow->pGTKWindow), pPosXOut, pPosYOut);
}


void dred_window_move_to_center__gtk(dred_window* pWindow)
{
    gtk_window_set_position(GTK_WINDOW(pWindow->pGTKWindow), GTK_WIN_POS_CENTER);
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

bool dred_window_is_maximized__gtk(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return false;
    }

    return gtk_window_is_maximized(GTK_WINDOW(pWindow->pGTKWindow));
}


void dred_window_set_cursor__gtk(dred_window* pWindow, dred_cursor_type cursor)
{
    switch (cursor)
    {
        case dred_cursor_type_text:
        {
            pWindow->pGTKCursor = g_GTKCursor_IBeam;
        } break;

        case dred_cursor_type_cross:
        {
            pWindow->pGTKCursor = g_GTKCursor_Cross;
        } break;

        case dred_cursor_type_double_arrow_h:
        {
            pWindow->pGTKCursor = g_GTKCursor_DoubleArrowH;
        } break;

        case dred_cursor_type_double_arrow_v:
        {
            pWindow->pGTKCursor = g_GTKCursor_DoubleArrowH;
        } break;


        case dred_cursor_type_none:
        {
            pWindow->pGTKCursor = NULL;
        } break;

        //case cursor_type_arrow:
        case dred_cursor_type_default:
        default:
        {
            pWindow->pGTKCursor = g_GTKCursor_Default;
        } break;
    }

    // If the cursor is currently inside the window it needs to be changed right now.
    if (dred_window_is_cursor_over(pWindow)) {
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(pWindow->pGTKWindow)), pWindow->pGTKCursor);
    }
}

bool dred_window_is_cursor_over__gtk(dred_window* pWindow)
{
    return pWindow->isCursorOver;
}



void dred_window_bind_accelerators__gtk(dred_window* pWindow, dred_accelerator_table* pAcceleratorTable)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->pGTKAccelGroup != NULL) {
        g_object_unref(G_OBJECT(pWindow->pGTKAccelGroup));
        pWindow->pGTKAccelGroup = NULL;

        dred_gtk__delete_accels(pWindow->pAccels, pWindow->accelCount);
        pWindow->accelCount = 0;
    }

    // pAcceleratorTable is allowed to be null, in which case it is equivalent to simply unbinding the table.
    if (pAcceleratorTable != NULL) {
        pWindow->pGTKAccelGroup = dred_gtk__create_accels(pAcceleratorTable, &pWindow->pAccels, pWindow, NULL);
        if (pWindow->pGTKAccelGroup != NULL) {
            gtk_window_add_accel_group(GTK_WINDOW(pWindow->pGTKWindow), pWindow->pGTKAccelGroup);
        }

        pWindow->accelCount = pAcceleratorTable->count;
    }
}

void dred_window_set_menu__gtk(dred_window* pWindow, dred_menu* pMenu)
{
    if (pWindow == NULL) {
        return;
    }

    // Only menu bars are allowed to be set on a window.
    if (pMenu != NULL && pMenu->type != dred_menu_type_menubar) {
        return;
    }


    // The old menu bar needs to be removed.
    if (pWindow->pMenu != NULL) {
        gtk_container_remove(GTK_CONTAINER(pWindow->pGTKBox), pWindow->pMenu->pGTKMenu);
    }

    // Add the new menu to the top.
    if (pWindow->isShowingMenu) {
        gtk_box_pack_start(GTK_BOX(pWindow->pGTKBox), pMenu->pGTKMenu, FALSE, FALSE, 0);
        gtk_box_reorder_child(GTK_BOX(pWindow->pGTKBox), pMenu->pGTKMenu, 0);
        gtk_widget_show(pMenu->pGTKMenu);
    }

    pWindow->pMenu = pMenu;
}

void dred_window_hide_menu__gtk(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->pMenu != NULL) {
        gtk_container_remove(GTK_CONTAINER(pWindow->pGTKBox), pWindow->pMenu->pGTKMenu);
    }

    pWindow->isShowingMenu = false;
}

void dred_window_show_menu__gtk(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->pMenu != NULL) {
        gtk_box_pack_start(GTK_BOX(pWindow->pGTKBox), pWindow->pMenu->pGTKMenu, FALSE, FALSE, 0);
        gtk_box_reorder_child(GTK_BOX(pWindow->pGTKBox), pWindow->pMenu->pGTKMenu, 0);
        gtk_widget_show(pWindow->pMenu->pGTKMenu);
    }

    pWindow->isShowingMenu = true;
}


static int popupPosX = 0;
static int popupPosY = 0;

void dred_gtk_cb__on_menu_position(GtkMenu* pGTKMenu, gint* x, gint* y, gboolean* push_in, gpointer pUserData)
{
    (void)pGTKMenu;

    dred_window* pWindow = (dred_window*)pUserData;
    assert(pWindow != NULL);

    gint windowPosX;
    gint windowPosY;
    gtk_window_get_position(GTK_WINDOW(pWindow->pGTKWindow), &windowPosX, &windowPosY);

    gint clientPosX;
    gint clientPosY;
    gtk_widget_translate_coordinates(pWindow->pGTKClientArea, gtk_widget_get_toplevel(pWindow->pGTKClientArea), 0, 0, &clientPosX, &clientPosY);


    *x = windowPosX + clientPosX + popupPosX;
    *y = windowPosY + clientPosY + popupPosY;
    *push_in = TRUE;
}

void dred_window_show_popup_menu__gtk(dred_window* pWindow, dred_menu* pMenu, int posX, int posY)
{
    popupPosX = posX;
    popupPosY = posY;
    gtk_menu_popup(GTK_MENU(pMenu->pGTKMenu), NULL, NULL, dred_gtk_cb__on_menu_position, pWindow, 0, 0);
}


//// MENUS ////

static gboolean dred_gtk_cb__on_mouse_enter__menu(GtkWidget* pGTKMenu, GdkEventCrossing* pEvent, gpointer pUserData)
{
    (void)pGTKMenu;
    (void)pEvent;
    (void)pUserData;

    gdk_window_set_cursor(gtk_widget_get_window(pGTKMenu), g_GTKCursor_Default);
    return false;
}


dred_menu* dred_menu_create__gtk(dred_context* pDred, dred_menu_type type)
{
    if (pDred == NULL) {
        return NULL;
    }

    GtkWidget* pGTKMenu = NULL;
    if (type == dred_menu_type_popup) {
        pGTKMenu = gtk_menu_new();
    } else {
        pGTKMenu = gtk_menu_bar_new();
    }

    if (pGTKMenu == NULL) {
        return NULL;
    }


    dred_menu* pMenu = (dred_menu*)calloc(1, sizeof(*pMenu));
    if (pMenu == NULL) {
        gtk_widget_destroy(pGTKMenu);
        return NULL;
    }

    pMenu->pDred    = pDred;
    pMenu->type     = type;
    pMenu->pGTKMenu = pGTKMenu;

    pMenu->pGTKAccelGroup = dred_gtk__create_accels(&pDred->shortcutTable.acceleratorTable, &pMenu->pAccels, NULL, pMenu);
    pMenu->accelCount = pDred->shortcutTable.acceleratorTable.count;

    g_signal_connect(pGTKMenu, "enter-notify-event", G_CALLBACK(dred_gtk_cb__on_mouse_enter__menu), NULL);

    // When switching out menus we use the gtk_container_remove() function which decrements the reference counter
    // and may delete the widget. We don't want this behaviour so we just grab a reference to it from here.
    g_object_ref(pMenu->pGTKMenu);

    return pMenu;
}

void dred_menu_delete__gtk(dred_menu* pMenu)
{
    if (pMenu == NULL) {
        return;
    }

    gtk_widget_destroy(pMenu->pGTKMenu);
    free(pMenu);
}


dred_menu_item* dred_menu_item_create_and_append__gtk__internal(dred_menu* pMenu, const char* text, uint16_t id, const char* command, dred_shortcut shortcut, dred_menu* pSubMenu, bool separator)
{
    if (pMenu == NULL) {
        return NULL;
    }

    GtkWidget* pGTKMenuItem = NULL;

    if (separator) {
        pGTKMenuItem = gtk_separator_menu_item_new();
        if (pGTKMenuItem == NULL) {
            return NULL;
        }
    } else {
        if (text == NULL) {
            text = "";
        }

        // The input string will have "&" characters for the mnemonic symbol, but GTK expects "_".
        char transformedText[256];
        strncpy_s(transformedText, sizeof(transformedText), text, _TRUNCATE);

        bool useMnemonic = true;
        for (char* c = transformedText; c[0] != '\0'; c += 1) {
            // If the text has an underscore, don't use mnemonics. Otherwise we'll end up with incorrect text.
            if (c[0] == '_') {
                useMnemonic = false;
                break;
            }

            if (c[0] == '&') {
                c[0] = '_';
                break;
            }
        }

        if (useMnemonic) {
            pGTKMenuItem = gtk_menu_item_new_with_mnemonic(transformedText);
        } else {
            pGTKMenuItem = gtk_menu_item_new_with_label(transformedText);
        }

        if (pGTKMenuItem == NULL) {
            return NULL;
        }

        if (pMenu->pGTKAccelGroup) {
            dred_accelerator accel0 = shortcut.accelerators[0];
            dred_accelerator accel1 = shortcut.accelerators[1];
            //gtk_widget_add_accelerator(pGTKMenuItem, "activate", pMenu->pGTKAccelGroup, dred_dred_key_to_gtk(accel0.key), dred_accelerator_modifiers_to_gtk(accel0.modifiers), GTK_ACCEL_VISIBLE);

            if (accel0.key != 0) {
                // TODO: Think about how we'll present multi-key shortcuts. For now we are just not presenting them at all which is not ideal.
                if (accel1.key == 0) {
                    GtkWidget* pGTKAccelLabel = gtk_bin_get_child(GTK_BIN(pGTKMenuItem));
                    gtk_accel_label_set_accel(GTK_ACCEL_LABEL(pGTKAccelLabel), dred_dred_key_to_gtk(accel0.key), dred_accelerator_modifiers_to_gtk(accel0.modifiers));
                }
            }
        }
    }


    dred_menu_item* pItem = (dred_menu_item*)calloc(1, sizeof(*pItem));
    if (pItem == NULL) {
        gtk_widget_destroy(pGTKMenuItem);
        return NULL;
    }

    pItem->id = id;
    pItem->index = pMenu->menuItemCount;
    pItem->command = gb_make_string(command);
    pItem->pSubMenu = pSubMenu;
    pItem->pOwnerMenu = pMenu;
    pItem->pGTKMenuItem = pGTKMenuItem;
    pItem->pDred = pMenu->pDred;


    // Add the item to the list.
    dred_menu_item** ppNewMenuItems = (dred_menu_item**)realloc(pMenu->ppMenuItems, (pMenu->menuItemCount + 1) * sizeof(*ppNewMenuItems));
    if (ppNewMenuItems == NULL) {
        free(pItem);
        return NULL;
    }

    pMenu->ppMenuItems = ppNewMenuItems;
    pMenu->ppMenuItems[pMenu->menuItemCount++] = pItem;


    g_signal_connect(pGTKMenuItem, "activate", G_CALLBACK(dred_gtk_cb__on_menu_item_activate), pItem);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu->pGTKMenu), pGTKMenuItem);
    gtk_widget_show(pGTKMenuItem);

    if (pSubMenu != NULL) {
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(pGTKMenuItem), pSubMenu->pGTKMenu);
    }

    return pItem;
}

dred_menu_item* dred_menu_item_create_and_append__gtk(dred_menu* pMenu, const char* text, uint16_t id, const char* command, dred_shortcut shortcut, dred_menu* pSubMenu)
{
    return dred_menu_item_create_and_append__gtk__internal(pMenu, text, id, command, shortcut, pSubMenu, false);
}

dred_menu_item* dred_menu_item_create_and_append_separator__gtk(dred_menu* pMenu)
{
    return dred_menu_item_create_and_append__gtk__internal(pMenu, NULL, 0, NULL, dred_shortcut_none(), NULL, true);
}

void dred_menu_item_delete__gtk(dred_menu_item* pItem)
{
    if (pItem == NULL) {
        return;
    }


    // Remove the item from the list.
    assert(pItem->pOwnerMenu->menuItemCount > 0);
    for (size_t i = pItem->index; i < pItem->pOwnerMenu->menuItemCount-1; ++i) {
        pItem->pOwnerMenu->ppMenuItems[i] = pItem->pOwnerMenu->ppMenuItems[i+1];
        pItem->pOwnerMenu->ppMenuItems[i]->index -= 1;
    }
    pItem->pOwnerMenu->menuItemCount -= 1;


    if (pItem->pGTKMenuItem) {
        gtk_widget_destroy(pItem->pGTKMenuItem);
    }

    free(pItem);
}

void dred_menu_item_enable__gtk(dred_menu_item* pItem)
{
    if (pItem == NULL) {
        return;
    }

    gtk_widget_set_sensitive(pItem->pGTKMenuItem, TRUE);
}

void dred_menu_item_disable__gtk(dred_menu_item* pItem)
{
    if (pItem == NULL) {
        return;
    }

    gtk_widget_set_sensitive(pItem->pGTKMenuItem, FALSE);
}



//// DPI SCALING ////

void dred_get_base_dpi__gtk(int* pDPIXOut, int* pDPIYOut)
{
    if (pDPIXOut) *pDPIXOut = 96;
    if (pDPIYOut) *pDPIYOut = 96;
}

void dred_get_system_dpi__gtk(int* pDPIXOut, int* pDPIYOut)
{
    // TODO: Read this from a setting... no idea which one, though...
    if (pDPIXOut) *pDPIXOut = 96;
    if (pDPIYOut) *pDPIYOut = 96;
}



//// TIMERS ////

static gboolean dred_timer_proc_gtk(gpointer data)
{
    dred_timer* pTimer = (dred_timer*)data;
    if (pTimer == NULL) {
        assert(false);
        return false;
    }

    if (pTimer->callback != NULL) {
        pTimer->callback(pTimer, pTimer->pUserData);
    }

    return true;
}

dred_timer* dred_timer_create__gtk(unsigned int timeoutInMilliseconds, dred_timer_proc callback, void* pUserData)
{
    dred_timer* pTimer = (dred_timer*)malloc(sizeof(*pTimer));
    if (pTimer == NULL) {
        return NULL;
    }

    pTimer->timerID               = g_timeout_add(timeoutInMilliseconds, dred_timer_proc_gtk, pTimer);
    pTimer->timeoutInMilliseconds = timeoutInMilliseconds;
    pTimer->callback              = callback;
    pTimer->pUserData             = pUserData;

    return pTimer;
}

void dred_timer_delete__gtk(dred_timer* pTimer)
{
    if (pTimer == NULL) {
        return;
    }

    g_source_remove(pTimer->timerID);
    free(pTimer);
}



//// Clipboard ////

bool dred_clipboard_set_text__gtk(const char* text, size_t textLength)
{
    if (textLength == (size_t)-1) {
        textLength = strlen(text);
    }

    gtk_clipboard_set_text(gtk_clipboard_get_default(gdk_display_get_default()), text, textLength);
    return true;
}

char* dred_clipboard_get_text__gtk()
{
    return gtk_clipboard_wait_for_text(gtk_clipboard_get_default(gdk_display_get_default()));
}

void dred_clipboard_free_text__gtk(char* text)
{
    g_free(text);
}



//// GTK <-> GUI BINDING ////

static void dred_platform__on_global_capture_mouse__gtk(dred_element* pElement)
{
    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow != NULL) {
        gdk_seat_grab(gdk_display_get_default_seat(gdk_display_get_default()),
            gtk_widget_get_window(pWindow->pGTKClientArea), GDK_SEAT_CAPABILITY_POINTER, FALSE, NULL, NULL, NULL, NULL);
    }
}

static void dred_platform__on_global_release_mouse__gtk(dred_element* pElement)
{
    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow != NULL) {
        gdk_seat_ungrab(gdk_display_get_default_seat(gdk_display_get_default()));
    }
}

static void dred_platform__on_global_capture_keyboard__gtk(dred_element* pElement, dred_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;

    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow != NULL) {
        pWindow->pElementWithKeyboardCapture = pElement;
        gtk_widget_grab_focus(GTK_WIDGET(pWindow->pGTKWindow));
    }
}

static void dred_platform__on_global_release_keyboard__gtk(dred_element* pElement, dred_element* pNewCapturedElement)
{
    (void)pNewCapturedElement;

    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow != NULL) {
        dred_window* pNewWindow = dred_get_element_window(pNewCapturedElement);
        if (pWindow != pNewWindow) {
            //gtk_widget_grab_focus(NULL);
        }
    }
}

static void dred_platform__on_global_dirty__gtk(dred_element* pElement, dred_rect relativeRect)
{
    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow != NULL && pWindow->pGTKWindow != NULL)
    {
        dred_rect absoluteRect = relativeRect;
        drgui_make_rect_absolute(pElement, &absoluteRect);

        if (dred_rect_has_volume(absoluteRect)) {
            gtk_widget_queue_draw_area(pWindow->pGTKClientArea,
                (gint)absoluteRect.left, (gint)absoluteRect.top, (gint)(absoluteRect.right - absoluteRect.left), (gint)(absoluteRect.bottom - absoluteRect.top));

            // Redraw immediately.
            GdkWindow* pGDKWindow = gtk_widget_get_window(pWindow->pGTKClientArea);
            if (pGDKWindow != NULL) {
                gdk_window_process_updates(pGDKWindow, TRUE);
            }
        }
    }
}
#endif




//////////////////////////////////////////////////////////////////
//
// Cross Platform
//
//////////////////////////////////////////////////////////////////

static void dred_platform__on_global_change_cursor(dred_element* pElement, dred_cursor_type cursor)
{
    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow == NULL) {
        return;
    }

    switch (cursor)
    {
    case dred_cursor_none:    dred_window_set_cursor(pWindow, dred_cursor_type_none);           break;
    case dred_cursor_text:    dred_window_set_cursor(pWindow, dred_cursor_type_text);           break;
    case dred_cursor_cross:   dred_window_set_cursor(pWindow, dred_cursor_type_cross);          break;
    case dred_cursor_size_ns: dred_window_set_cursor(pWindow, dred_cursor_type_double_arrow_h); break;
    case dred_cursor_size_we: dred_window_set_cursor(pWindow, dred_cursor_type_double_arrow_v); break;

    case dred_cursor_default:
    default:
        {
            dred_window_set_cursor(pWindow, dred_cursor_type_default);
        } break;
    }
}

void dred_platform__on_delete_gui_element(dred_element* pElement)
{
    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->pElementWithKeyboardCapture == pElement) {
        pWindow->pElementWithKeyboardCapture = NULL;
    }
}


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

void dred_platform_bind_gui(dred_gui* pGUI)
{
#ifdef DRED_WIN32
    drgui_set_global_on_capture_mouse(pGUI, dred_platform__on_global_capture_mouse__win32);
    drgui_set_global_on_release_mouse(pGUI, dred_platform__on_global_release_mouse__win32);
    drgui_set_global_on_capture_keyboard(pGUI, dred_platform__on_global_capture_keyboard__win32);
    drgui_set_global_on_release_keyboard(pGUI, dred_platform__on_global_release_keyboard__win32);
    drgui_set_global_on_dirty(pGUI, dred_platform__on_global_dirty__win32);
#endif

#ifdef DRED_GTK
    drgui_set_global_on_capture_mouse(pGUI, dred_platform__on_global_capture_mouse__gtk);
    drgui_set_global_on_release_mouse(pGUI, dred_platform__on_global_release_mouse__gtk);
    drgui_set_global_on_capture_keyboard(pGUI, dred_platform__on_global_capture_keyboard__gtk);
    drgui_set_global_on_release_keyboard(pGUI, dred_platform__on_global_release_keyboard__gtk);
    drgui_set_global_on_dirty(pGUI, dred_platform__on_global_dirty__gtk);
#endif

    drgui_set_global_on_change_cursor(pGUI, dred_platform__on_global_change_cursor);
    drgui_set_on_delete_element(pGUI, dred_platform__on_delete_gui_element);
}




dred_window* dred_window_create(dred_context* pDred)
{
#ifdef DRED_WIN32
    return dred_window_create__win32(pDred);
#endif

#ifdef DRED_GTK
    return dred_window_create__gtk(pDred);
#endif
}

dred_window* dred_window_create_dialog(dred_window* pParentWindow, const char* title, unsigned int width, unsigned int height)
{
#ifdef DRED_WIN32
    return dred_window_create_dialog__win32(pParentWindow, title, width, height);
#endif

#ifdef DRED_GTK
    return dred_window_create_dialog__gtk(pParentWindow, title, width, height);
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


void dred_window_set_title(dred_window* pWindow, const char* title)
{
#ifdef DRED_WIN32
    dred_window_set_title__win32(pWindow, title);
#endif

#ifdef DRED_GTK
    dred_window_set_title__gtk(pWindow, title);
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


void dred_window_set_position(dred_window* pWindow, int posX, int posY)
{
#ifdef DRED_WIN32
    dred_window_set_position__win32(pWindow, posX, posY);
#endif

#ifdef DRED_GTK
    dred_window_set_position__gtk(pWindow, posX, posY);
#endif
}

void dred_window_get_position(dred_window* pWindow, int* pPosXOut, int* pPosYOut)
{
#ifdef DRED_WIN32
    dred_window_get_position__win32(pWindow, pPosXOut, pPosYOut);
#endif

#ifdef DRED_GTK
    dred_window_get_position__gtk(pWindow, pPosXOut, pPosYOut);
#endif
}


void dred_window_get_client_size(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
#ifdef DRED_WIN32
    dred_window_get_client_size__win32(pWindow, pWidthOut, pHeightOut);
#endif

#ifdef DRED_GTK
    dred_window_get_client_size__gtk(pWindow, pWidthOut, pHeightOut);
#endif
}


void dred_window_move_to_center(dred_window* pWindow)
{
#ifdef DRED_WIN32
    dred_window_move_to_center__win32(pWindow);
#endif

#ifdef DRED_GTK
    dred_window_move_to_center__gtk(pWindow);
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
    if (pWindow == NULL) {
        return;
    }

    pWindow->onHideFlags = flags;

#ifdef DRED_WIN32
    dred_window_hide__win32(pWindow);
#endif

#ifdef DRED_GTK
    dred_window_hide__gtk(pWindow);
#endif
}

bool dred_window_is_maximized(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return false;
    }

#ifdef DRED_WIN32
    return dred_window_is_maximized__win32(pWindow);
#endif

#ifdef DRED_GTK
    return dred_window_is_maximized__gtk(pWindow);
#endif
}


void dred_window_set_cursor(dred_window* pWindow, dred_cursor_type cursor)
{
#ifdef DRED_WIN32
    dred_window_set_cursor__win32(pWindow, cursor);
#endif

#ifdef DRED_GTK
    dred_window_set_cursor__gtk(pWindow, cursor);
#endif
}

bool dred_window_is_cursor_over(dred_window* pWindow)
{
#ifdef DRED_WIN32
    return dred_window_is_cursor_over__win32(pWindow);
#endif

#ifdef DRED_GTK
    return dred_window_is_cursor_over__gtk(pWindow);
#endif
}


void dred_window_bind_accelerators(dred_window* pWindow, dred_accelerator_table* pAcceleratorTable)
{
#ifdef DRED_WIN32
    dred_window_bind_accelerators__win32(pWindow, pAcceleratorTable);
#endif

#ifdef DRED_GTK
    dred_window_bind_accelerators__gtk(pWindow, pAcceleratorTable);
#endif
}

void dred_window_set_menu(dred_window* pWindow, dred_menu* pMenu)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->pMenu == pMenu) {
        return; // It's the same menu.
    }

#ifdef DRED_WIN32
    dred_window_set_menu__win32(pWindow, pMenu);
#endif

#ifdef DRED_GTK
    dred_window_set_menu__gtk(pWindow, pMenu);
#endif
}

void dred_window_hide_menu(dred_window* pWindow)
{
    if (!dred_window_is_showing_menu(pWindow)) {
        return;
    }

#ifdef DRED_WIN32
    dred_window_hide_menu__win32(pWindow);
#endif

#ifdef DRED_GTK
    dred_window_hide_menu__gtk(pWindow);
#endif
}

void dred_window_show_menu(dred_window* pWindow)
{
    if (dred_window_is_showing_menu(pWindow)) {
        return;
    }

#ifdef DRED_WIN32
    dred_window_show_menu__win32(pWindow);
#endif

#ifdef DRED_GTK
    dred_window_show_menu__gtk(pWindow);
#endif
}

bool dred_window_is_showing_menu(dred_window* pWindow)
{
    if (pWindow == NULL) {
        return false;
    }

    return pWindow->isShowingMenu;
}

dred_menu_item* dred_window_find_menu_item_by_id(dred_window* pWindow, uint16_t id)
{
    if (pWindow == NULL || pWindow->pMenu == NULL) {
        return NULL;
    }

    return dred_menu_find_menu_item_by_id(pWindow->pMenu, id);
}

void dred_window_show_popup_menu(dred_window* pWindow, dred_menu* pMenu, int posX, int posY)
{
#ifdef DRED_WIN32
    dred_window_show_popup_menu__win32(pWindow, pMenu, posX, posY);
#endif

#ifdef DRED_GTK
    dred_window_show_popup_menu__gtk(pWindow, pMenu, posX, posY);
#endif
}


void dred_window_on_close(dred_window* pWindow)
{
    if (pWindow->onClose) {
        pWindow->onClose(pWindow);
    }
}

bool dred_window_on_hide(dred_window* pWindow, unsigned int flags)
{
    if (pWindow->onHide) {
        return pWindow->onHide(pWindow, flags);
    }

    return true;    // Returning true means to process the message as per normal.
}

bool dred_window_on_show(dred_window* pWindow)
{
    if (pWindow->onShow) {
        return pWindow->onShow(pWindow);
    }

    return true;    // Returning true means to process the message as per normal.
}

void dred_window_on_activate(dred_window* pWindow)
{
    if (pWindow->onActivate) {
        pWindow->onActivate(pWindow);
    }
}

void dred_window_on_deactivate(dred_window* pWindow)
{
    if (pWindow->onDeactivate) {
        pWindow->onDeactivate(pWindow);
    }
}

void dred_window_on_size(dred_window* pWindow, unsigned int newWidth, unsigned int newHeight)
{
    if (pWindow->onSize) {
        pWindow->onSize(pWindow, newWidth, newHeight);
    }

    // Always resize the root GUI element so that it's the exact same size as the window.
    drgui_set_size(pWindow->pRootGUIElement, (float)newWidth, (float)newHeight);
}

void dred_window_on_move(dred_window* pWindow, int newPosX, int newPosY)
{
    if (pWindow->onMove) {
        pWindow->onMove(pWindow, newPosX, newPosY);
    }
}

void dred_window_on_mouse_enter(dred_window* pWindow)
{
    if (pWindow->onActivate) {
        pWindow->onActivate(pWindow);
    }
}

void dred_window_on_mouse_leave(dred_window* pWindow)
{
    if (pWindow->onMouseLeave) {
        pWindow->onMouseLeave(pWindow);
    }

    drgui_post_inbound_event_mouse_leave(pWindow->pRootGUIElement);
}

void dred_window_on_mouse_move(dred_window* pWindow, int mousePosX, int mousePosY, unsigned int stateFlags)
{
    if (pWindow->onMouseMove) {
        pWindow->onMouseMove(pWindow, mousePosX, mousePosY, stateFlags);
    }

    drgui_post_inbound_event_mouse_move(pWindow->pRootGUIElement, mousePosX, mousePosY, stateFlags);
}

void dred_window_on_mouse_button_down(dred_window* pWindow, int mouseButton, int mousePosX, int mousePosY, unsigned int stateFlags)
{
    if (pWindow->onMouseButtonDown) {
        pWindow->onMouseButtonDown(pWindow, mouseButton, mousePosX, mousePosY, stateFlags);
    }

    drgui_post_inbound_event_mouse_button_down(pWindow->pRootGUIElement, mouseButton, mousePosX, mousePosY, stateFlags);
}

void dred_window_on_mouse_button_up(dred_window* pWindow, int mouseButton, int mousePosX, int mousePosY, unsigned int stateFlags)
{
    if (pWindow->onMouseButtonUp) {
        pWindow->onMouseButtonUp(pWindow, mouseButton, mousePosX, mousePosY, stateFlags);
    }

    drgui_post_inbound_event_mouse_button_up(pWindow->pRootGUIElement, mouseButton, mousePosX, mousePosY, stateFlags);
}

void dred_window_on_mouse_button_dblclick(dred_window* pWindow, int mouseButton, int mousePosX, int mousePosY, unsigned int stateFlags)
{
    if (pWindow->onMouseButtonDblClick) {
        pWindow->onMouseButtonDblClick(pWindow, mouseButton, mousePosX, mousePosY, stateFlags);
    }

    drgui_post_inbound_event_mouse_button_dblclick(pWindow->pRootGUIElement, mouseButton, mousePosX, mousePosY, stateFlags);
}

void dred_window_on_mouse_wheel(dred_window* pWindow, int delta, int mousePosX, int mousePosY, unsigned int stateFlags)
{
    if (pWindow->onMouseWheel) {
        pWindow->onMouseWheel(pWindow, delta, mousePosX, mousePosY, stateFlags);
    }

    drgui_post_inbound_event_mouse_wheel(pWindow->pRootGUIElement, delta, mousePosX, mousePosY, stateFlags);
}

void dred_window_on_key_down(dred_window* pWindow, dred_key key, unsigned int stateFlags)
{
    if (pWindow->onKeyDown) {
        pWindow->onKeyDown(pWindow, key, stateFlags);
    }

    if (pWindow->pRootGUIElement) {
        drgui_post_inbound_event_key_down(pWindow->pRootGUIElement->pContext, key, stateFlags);
    }
}

void dred_window_on_key_up(dred_window* pWindow, dred_key key, unsigned int stateFlags)
{
    if (pWindow->onKeyUp) {
        pWindow->onKeyUp(pWindow, key, stateFlags);
    }

    if (pWindow->pRootGUIElement) {
        drgui_post_inbound_event_key_up(pWindow->pRootGUIElement->pContext, key, stateFlags);
    }
}

void dred_window_on_printable_key_down(dred_window* pWindow, unsigned int character, unsigned int stateFlags)
{
    if (pWindow->onPrintableKeyDown) {
        pWindow->onPrintableKeyDown(pWindow, character, stateFlags);
    }

    if (pWindow->pRootGUIElement) {
        drgui_post_inbound_event_printable_key_down(pWindow->pRootGUIElement->pContext, character, stateFlags);
    }
}

void dred_window_on_focus(dred_window* pWindow)
{
    if (pWindow->onFocus) {
        pWindow->onFocus(pWindow);
    }

    // Make sure the GUI element is given the keyboard capture if it hasn't already got it.
    if (!drgui_has_keyboard_capture(pWindow->pElementWithKeyboardCapture)) {
        drgui_capture_keyboard(pWindow->pElementWithKeyboardCapture);
    }
}

void dred_window_on_unfocus(dred_window* pWindow)
{
    if (pWindow->onUnfocus) {
        pWindow->onUnfocus(pWindow);
    }

    // Make sure the GUI element is released of the keyboard capture, but don't clear the variable.
    if (drgui_has_keyboard_capture(pWindow->pElementWithKeyboardCapture)) {
        drgui_release_keyboard(pWindow->pDred->pGUI);
    }
}


void dred_window__stock_event__hide_on_close(dred_window* pWindow)
{
    assert(pWindow != NULL);
    dred_window_hide(pWindow, 0);
}


dred_window* dred_get_element_window(dred_element* pElement)
{
    if (pElement == NULL) {
        return NULL;
    }

    dred_element* pRootGUIElement = drgui_find_top_level_element(pElement);
    if (pRootGUIElement == NULL) {
        return NULL;
    }

    if (!dred_control_is_of_type(pRootGUIElement, "RootGUIElement")) {
        return NULL;
    }

    dred_window** ppWindow = dred_control_get_extra_data(pRootGUIElement);
    if (ppWindow == NULL) {
        return NULL;
    }

    return *ppWindow;
}


//// MENUS ////

dred_menu* dred_menu_create(dred_context* pDred, dred_menu_type type)
{
#ifdef DRED_WIN32
    return dred_menu_create__win32(pDred, type);
#endif

#ifdef DRED_GTK
    return dred_menu_create__gtk(pDred, type);
#endif
}

void dred_menu_delete(dred_menu* pMenu)
{
#ifdef DRED_WIN32
    dred_menu_delete__win32(pMenu);
#endif

#ifdef DRED_GTK
    dred_menu_delete__gtk(pMenu);
#endif
}

dred_menu_item* dred_menu_find_menu_item_by_id(dred_menu* pMenu, uint16_t id)
{
    if (pMenu == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < pMenu->menuItemCount; ++i) {
        if (pMenu->ppMenuItems[i]->id == id) {
            return pMenu->ppMenuItems[i];
        }

        if (pMenu->ppMenuItems[i]->pSubMenu != NULL) {
            dred_menu_item* pItem = dred_menu_find_menu_item_by_id(pMenu->ppMenuItems[i]->pSubMenu, id);
            if (pItem != NULL) {
                return pItem;
            }
        }
    }

    return NULL;
}


dred_menu_item* dred_menu_item_create_and_append(dred_menu* pMenu, const char* text, uint16_t id, const char* command, dred_shortcut shortcut, dred_menu* pSubMenu)
{
#ifdef DRED_WIN32
    return dred_menu_item_create_and_append__win32(pMenu, text, id, command, shortcut, pSubMenu);
#endif

#ifdef DRED_GTK
    return dred_menu_item_create_and_append__gtk(pMenu, text, id, command, shortcut, pSubMenu);
#endif
}

dred_menu_item* dred_menu_item_create_and_append_with_shortcut(dred_menu* pMenu, const char* text, uint16_t id, const char* shortcutName)
{
    size_t shortcutIndex;
    if (!dred_shortcut_table_find_by_name(&pMenu->pDred->shortcutTable, shortcutName, &shortcutIndex)) {
        goto on_error;
    }

    dred_shortcut shortcut;
    if (!dred_shortcut_table_get_shortcut_by_index(&pMenu->pDred->shortcutTable, shortcutIndex, &shortcut)) {
        goto on_error;
    }

    const char* commandStr = dred_shortcut_table_get_command_string_by_index(&pMenu->pDred->shortcutTable, shortcutIndex);
    if (commandStr == NULL) {
        goto on_error;
    }

    return dred_menu_item_create_and_append(pMenu, text, id, commandStr, shortcut, NULL);


on_error:
    dred_warningf(pMenu->pDred, "Failed to create menu item (%s) with shortcut (%s). Menu item will be present, but not functional.", text, shortcutName);
    return dred_menu_item_create_and_append(pMenu, text, id, "", dred_shortcut_none(), NULL);
}

dred_menu_item* dred_menu_item_create_and_append_separator(dred_menu* pMenu)
{
#ifdef DRED_WIN32
    return dred_menu_item_create_and_append_separator__win32(pMenu);
#endif

#ifdef DRED_GTK
    return dred_menu_item_create_and_append_separator__gtk(pMenu);
#endif
}

void dred_menu_item_delete(dred_menu_item* pItem)
{
#ifdef DRED_WIN32
    dred_menu_item_delete__win32(pItem);
#endif

#ifdef DRED_GTK
    dred_menu_item_delete__gtk(pItem);
#endif
}

void dred_menu_delete_all_items(dred_menu* pMenu)
{
    if (pMenu == NULL) {
        return;
    }

    while (pMenu->menuItemCount > 0) {
        dred_menu_item_delete(pMenu->ppMenuItems[0]);
    }
}


void dred_menu_item_enable(dred_menu_item* pItem)
{
    if (pItem == NULL) {
        return;
    }

#ifdef DRED_WIN32
    dred_menu_item_enable__win32(pItem);
#endif

#ifdef DRED_GTK
    dred_menu_item_enable__gtk(pItem);
#endif
}

void dred_menu_item_disable(dred_menu_item* pItem)
{
    if (pItem == NULL) {
        return;
    }

#ifdef DRED_WIN32
    dred_menu_item_disable__win32(pItem);
#endif

#ifdef DRED_GTK
    dred_menu_item_disable__gtk(pItem);
#endif
}



//// DPI SCALING ////

void dred_get_base_dpi(int* pDPIXOut, int* pDPIYOut)
{
#ifdef DRED_WIN32
    dr_win32_get_base_dpi(pDPIXOut, pDPIYOut);
#endif

#ifdef DRED_GTK
    dred_get_base_dpi__gtk(pDPIXOut, pDPIYOut);
#endif
}

void dred_get_system_dpi(int* pDPIXOut, int* pDPIYOut)
{
#ifdef DRED_WIN32
    dr_win32_get_system_dpi(pDPIXOut, pDPIYOut);
#endif

#ifdef DRED_GTK
    dred_get_system_dpi__gtk(pDPIXOut, pDPIYOut);
#endif
}




//// TIMERS ////

dred_timer* dred_timer_create(unsigned int timeoutInMilliseconds, dred_timer_proc callback, void* pUserData)
{
#ifdef DRED_WIN32
    return dred_timer_create__win32(timeoutInMilliseconds, callback, pUserData);
#endif

#ifdef DRED_GTK
    return dred_timer_create__gtk(timeoutInMilliseconds, callback, pUserData);
#endif
}

void dred_timer_delete(dred_timer* pTimer)
{
#ifdef DRED_WIN32
    dred_timer_delete__win32(pTimer);
#endif

#ifdef DRED_GTK
    dred_timer_delete__gtk(pTimer);
#endif
}




//// Clipboard ////

bool dred_clipboard_set_text(const char* text, size_t textLength)
{
#ifdef DRED_WIN32
    return dred_clipboard_set_text__win32(text, textLength);
#endif

#ifdef DRED_GTK
    return dred_clipboard_set_text__gtk(text, textLength);
#endif
}

char* dred_clipboard_get_text()
{
#ifdef DRED_WIN32
    return dred_clipboard_get_text__win32();
#endif

#ifdef DRED_GTK
    return dred_clipboard_get_text__gtk();
#endif
}

void dred_clipboard_free_text(char* text)
{
#ifdef DRED_WIN32
    dred_clipboard_free_text__win32(text);
#endif

#ifdef DRED_GTK
    dred_clipboard_free_text__gtk(text);
#endif
}

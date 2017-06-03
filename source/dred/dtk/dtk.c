// Copyright (C) 2017 David Reid. See included LICENSE file.

#include "dtk.h"

// Platform headers.
#ifdef DTK_WIN32
    #include <windows.h>
    #include <commctrl.h>
#endif
#ifdef DTK_GTK
    #include <gdk/gdk.h>
    #include <gtk/gtk.h>
#endif

#include <assert.h>
#include <ctype.h>  // For toupper()
#include <math.h>   // For ceil(), round(), etc.
#include <stdio.h>  // For sprintf() and family.

#if !defined(DTK_64BIT) && !defined(DTK_32BIT)
#ifdef _WIN32
#ifdef _WIN64
#define DTK_64BIT
#else
#define DTK_32BIT
#endif
#endif
#endif

#if !defined(DTK_64BIT) && !defined(DTK_32BIT)
#ifdef __GNUC__
#ifdef __LP64__
#define DTK_64BIT
#else
#define DTK_32BIT
#endif
#endif
#endif

#if !defined(DTK_64BIT) && !defined(DTK_32BIT)
#include <stdint.h>
#if INTPTR_MAX == INT64_MAX
#define DTK_64BIT
#else
#define DTK_32BIT
#endif
#endif

// Atomics.
#if defined(DTK_WIN32) && defined(_MSC_VER)
#define dtk_memory_barrier()            MemoryBarrier()
#define dtk_atomic_exchange_32(a, b)    InterlockedExchange((LONG*)a, (LONG)b)
#define dtk_atomic_exchange_64(a, b)    InterlockedExchange64((LONGLONG*)a, (LONGLONG)b)
#define dtk_atomic_increment_32(a)      InterlockedIncrement((LONG*)a)
#define dtk_atomic_decrement_32(a)      InterlockedDecrement((LONG*)a)
#else
#define dtk_memory_barrier()            __sync_synchronize()
#define dtk_atomic_exchange_32(a, b)    (void)__sync_lock_test_and_set(a, b); __sync_synchronize()
#define dtk_atomic_exchange_64(a, b)    (void)__sync_lock_test_and_set(a, b); __sync_synchronize()
#define dtk_atomic_increment_32(a)      __sync_add_and_fetch(a, 1)
#define dtk_atomic_decrement_32(a)      __sync_sub_and_fetch(a, 1)
#endif

#ifdef DTK_64BIT
#define dtk_atomic_exchange_ptr dtk_atomic_exchange_64
#endif
#ifdef DTK_32BIT
#define dtk_atomic_exchange_ptr dtk_atomic_exchange_32
#endif

// When capturing the keyboard and mouse on a control it must be completed by capturing the window
// that owns said control. The functions below are used for this.
dtk_result dtk__capture_keyboard_window(dtk_context* pTK, dtk_window* pWindow);
dtk_result dtk__release_keyboard_window(dtk_context* pTK);
dtk_result dtk__capture_mouse_window(dtk_context* pTK, dtk_window* pWindow);
dtk_result dtk__release_mouse_window(dtk_context* pTK);

// Private event posting APIs.
void dtk__post_mouse_leave_event_recursive(dtk_context* pTK, dtk_control* pNewControlUnderMouse, dtk_control* pOldControlUnderMouse);
void dtk__post_mouse_enter_event_recursive(dtk_context* pTK, dtk_control* pNewControlUnderMouse, dtk_control* pOldControlUnderMouse);

DTK_INLINE dtk_event dtk_event_init(dtk_context* pTK, dtk_event_type type, dtk_control* pControl)
{
    dtk_event e;
    dtk_zero_object(&e);
    e.pTK = pTK;
    e.type = type;
    e.pControl = pControl;

    return e;
}

dtk_result dtk__preprocess_event(dtk_event* pEvent)  // Returns DTK_CANCELLED if the event should be cancelled.
{
    dtk_assert(pEvent != NULL);

    dtk_context* pTK = pEvent->pTK;
    dtk_assert(pTK != NULL);

    // If the event is a capture change, make sure some of the state is updated. Capture changes are _always_
    // posted as a DTK_EVENT_CAPTURE_KEYBOARD event. When a release event is required it will be posted from
    // here.
    if (pEvent->type == DTK_EVENT_CHANGE_KEYBOARD_CAPTURE) {
        dtk_control* pOldCapturedControl = pTK->pControlWithKeyboardCapture;
        dtk_control* pNewCapturedControl = pEvent->pControl;

        // Don't do anything if the captured control isn't changing.
        if (pOldCapturedControl == pNewCapturedControl) {
            return DTK_CANCELLED;
        }

        pTK->pControlWithKeyboardCapture = pNewCapturedControl;

        // If the window is different then we'll need to change the keyboard focus on those too.
        dtk_window* pOldCapturedWindow = dtk_control_get_window(pOldCapturedControl);
        dtk_window* pNewCapturedWindow = dtk_control_get_window(pNewCapturedControl);

        if (pNewCapturedWindow != NULL) {
            if (pNewCapturedControl != DTK_CONTROL(pNewCapturedWindow)) {
                pNewCapturedWindow->pLastDescendantWithKeyboardFocus = pNewCapturedControl;
            } else {
                pNewCapturedWindow->pLastDescendantWithKeyboardFocus = NULL;
            }
        }

        if (pOldCapturedControl != NULL) {
            dtk_event eRelease = dtk_event_init(pTK, DTK_EVENT_RELEASE_KEYBOARD, pOldCapturedControl);
            eRelease.releaseKeyboard.pNewCapturedControl = pNewCapturedControl;
            dtk_handle_local_event(&eRelease);
        }

        if (pNewCapturedControl != NULL) {
            dtk_event eCapture = dtk_event_init(pTK, DTK_EVENT_CAPTURE_KEYBOARD, pNewCapturedControl);
            eCapture.captureKeyboard.pOldCapturedControl = pOldCapturedControl;
            dtk_handle_local_event(&eCapture);
        }

        // If the windows are different make sure the new one is given capture.
        if (pOldCapturedWindow != pNewCapturedWindow) {
            if (pNewCapturedWindow != NULL) {
                dtk__capture_keyboard_window(pTK, pNewCapturedWindow);
            } else {
                dtk__release_keyboard_window(pTK);
            }
        }

        return DTK_CANCELLED;   // <-- The keyboard capture/release events are always handled from this function or the window's event handlers.
    }

    if (pEvent->type == DTK_EVENT_CHANGE_MOUSE_CAPTURE) {
        dtk_control* pOldCapturedControl = pTK->pControlWithMouseCapture;
        dtk_control* pNewCapturedControl = pEvent->pControl;

        // Don't do anything if the captured control isn't changing.
        if (pOldCapturedControl == pNewCapturedControl) {
            return DTK_CANCELLED;
        }

        pTK->pControlWithMouseCapture = pNewCapturedControl;

        // If the window is different then we'll need to change the mouse focus on those too.
        dtk_window* pOldCapturedWindow = dtk_control_get_window(pOldCapturedControl);
        dtk_window* pNewCapturedWindow = dtk_control_get_window(pNewCapturedControl);

        if (pNewCapturedWindow != NULL) {
            if (pNewCapturedControl != DTK_CONTROL(pNewCapturedWindow)) {
                pNewCapturedWindow->pLastDescendantWithMouseCapture = pNewCapturedControl;
            } else {
                pNewCapturedWindow->pLastDescendantWithMouseCapture = NULL;
            }
        }

        if (pOldCapturedControl != NULL) {
            dtk_event eRelease = dtk_event_init(pTK, DTK_EVENT_RELEASE_MOUSE, pOldCapturedControl);
            eRelease.releaseMouse.pNewCapturedControl = pNewCapturedControl;
            dtk_handle_local_event(&eRelease);
        }

        if (pNewCapturedControl != NULL) {
            dtk_event eCapture = dtk_event_init(pTK, DTK_EVENT_CAPTURE_MOUSE, pNewCapturedControl);
            eCapture.captureMouse.pOldCapturedControl = pOldCapturedControl;
            dtk_handle_local_event(&eCapture);
        }

        // If the windows are different make sure the new one is given capture.
        if (pOldCapturedWindow != pNewCapturedWindow) {
            if (pNewCapturedWindow != NULL) {
                dtk__capture_mouse_window(pTK, pNewCapturedWindow);
            } else {
                dtk__release_mouse_window(pTK);
            }
        }

        return DTK_CANCELLED;   // <-- The keyboard capture/release events are always handled from this function or the window's event handlers.
    }

    return DTK_SUCCESS;
}

dtk_result dtk_handle_global_event(dtk_event* pEvent)
{
    dtk_assert(pEvent != NULL);
    
    dtk_context* pTK = pEvent->pTK;
    dtk_assert(pTK != NULL);

    // If there's any garbage the event will need to be normalized before executing. If the event references a garbaged control it
    // needs to be cancelled.
    dtk_result result = dtk__preprocess_event(pEvent);
    if (result != DTK_SUCCESS) {
        return result;
    }

    dtk_event_proc onEventGlobal = pTK->onEvent;
    if (onEventGlobal == NULL) {
        onEventGlobal = dtk_default_event_handler;
    }

    onEventGlobal(pEvent);
    return DTK_SUCCESS;
}


dtk_result dtk__track_window(dtk_context* pTK, dtk_window* pWindow)
{
    if (pTK == NULL || pWindow == NULL) return DTK_INVALID_ARGS;

    pWindow->pNextWindow = pTK->pFirstWindow;
    pTK->pFirstWindow = pWindow;

    return DTK_SUCCESS;
}

dtk_result dtk__untrack_window(dtk_context* pTK, dtk_window* pWindow)
{
    if (pTK == NULL || pWindow == NULL) return DTK_INVALID_ARGS;

    dtk_window** ppNext = &pTK->pFirstWindow;
    while (*ppNext != NULL) {
        if (*ppNext == pWindow) {
            *ppNext = (*ppNext)->pNextWindow;
            return DTK_SUCCESS;
        }

        ppNext = &(*ppNext)->pNextWindow;
    }

    return DTK_ERROR;
}


dtk_result dtk_errno_to_result(int err)
{
#if 0
    switch (err)
    {
        default: return DTK_ERROR;
    }
#endif

    (void)err;
    return DTK_ERROR;
}


#ifdef DTK_WIN32
#define DTK_WM_LOCAL                        (WM_USER + 0)
#define DTK_WM_CUSTOM                       (WM_USER + 1)
#define DTK_WM_PAINT_NOTIFICATION           (WM_USER + 2)

typedef enum DTK_PROCESS_DPI_AWARENESS {
    DTK_PROCESS_DPI_UNAWARE = 0,
    DTK_PROCESS_SYSTEM_DPI_AWARE = 1,
    DTK_PROCESS_PER_MONITOR_DPI_AWARE = 2
} DTK_PROCESS_DPI_AWARENESS;

typedef enum DTK_MONITOR_DPI_TYPE {
    DTK_MDT_EFFECTIVE_DPI = 0,
    DTK_MDT_ANGULAR_DPI = 1,
    DTK_MDT_RAW_DPI = 2,
    DTK_MDT_DEFAULT = DTK_MDT_EFFECTIVE_DPI
} DTK_MONITOR_DPI_TYPE;

typedef BOOL    (__stdcall * DTK_PFN_SetProcessDPIAware)     (void);
typedef HRESULT (__stdcall * DTK_PFN_SetProcessDpiAwareness) (DTK_PROCESS_DPI_AWARENESS);
typedef HRESULT (__stdcall * DTK_PFN_GetDpiForMonitor)       (HMONITOR hmonitor, DTK_MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

typedef BOOL    (WINAPI * DTK_PFN_InitCommonControlsEx)(const LPINITCOMMONCONTROLSEX lpInitCtrls);
typedef HRESULT (WINAPI * DTK_PFN_OleInitialize)       (LPVOID pvReserved);
typedef void    (WINAPI * DTK_PFN_OleUninitialize)     ();
typedef BOOL    (WINAPI * DTK_PFN_AlphaBlend)          (HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest, HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn);

// This function is not thread safe, and the returned value is a pointer to a buffer that's managed
// by the context. Should probably change this to use thread-local storage or something...
//
// TODO: Improve this API to make it thread-safe. Consider thread-local storage for the returned buffer.
wchar_t* dtk__mb_to_wchar__win32(dtk_context* pTK, const char* text, size_t textSizeInBytes, size_t* pCharacterCount);

// Converts a Win32 error code returned by GetLastError to a dtk_result.
dtk_result dtk_win32_error_to_result(DWORD error);
#endif

#include "dtk_rect.c"
#include "dtk_string.c"
#include "dtk_threading.c"
#include "dtk_monitor.c"
#include "dtk_graphics.c"
#include "dtk_input.c"
#include "dtk_accelerators.c"
#include "dtk_control.c"
#include "dtk_label.c"
#include "dtk_button.c"
#include "dtk_checkbox.c"
#include "dtk_color_button.c"
#include "dtk_tabbar.c"
#include "dtk_tabgroup.c"
#include "dtk_scrollbar.c"
#include "dtk_textbox.c"
#include "dtk_window.c"
#include "dtk_menu.c"
#include "dtk_dialogs.c"
#include "dtk_timer.c"
#include "dtk_clipboard.c"
#include "dtk_paint_queue.c"
#include "dtk_command_line.c"

typedef struct
{
    dtk_context* pTK;
    dtk_control* pControl;
    dtk_uint32 eventID;
    size_t dataSize;
    dtk_uint8 pData[1];
} dtk_custom_event_data;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// WIN32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
static dtk_uint32 g_dtkInitCounter_Win32 = 0;

void dtk_make_dpi_aware__win32()
{
    dtk_bool32 fallBackToDiscouragedAPI = DTK_FALSE;

    // We can't call SetProcessDpiAwareness() directly because otherwise on versions of Windows < 8.1 we'll get an error at load time about
    // a missing DLL.
    HMODULE hSHCoreDLL = LoadLibraryW(L"shcore.dll");
    if (hSHCoreDLL != NULL) {
        DTK_PFN_SetProcessDpiAwareness _SetProcessDpiAwareness = (DTK_PFN_SetProcessDpiAwareness)GetProcAddress(hSHCoreDLL, "SetProcessDpiAwareness");
        if (_SetProcessDpiAwareness != NULL) {
            if (_SetProcessDpiAwareness(DTK_PROCESS_PER_MONITOR_DPI_AWARE) != S_OK) {
                fallBackToDiscouragedAPI = DTK_TRUE;
            }
        } else {
            fallBackToDiscouragedAPI = DTK_TRUE;
        }

        FreeLibrary(hSHCoreDLL);
    } else {
        fallBackToDiscouragedAPI = DTK_TRUE;
    }

    if (fallBackToDiscouragedAPI) {
        HMODULE hUser32DLL = LoadLibraryW(L"user32.dll");
        if (hUser32DLL != NULL) {
            DTK_PFN_SetProcessDPIAware _SetProcessDPIAware = (DTK_PFN_SetProcessDPIAware)GetProcAddress(hUser32DLL, "SetProcessDPIAware");
            if (_SetProcessDPIAware != NULL) {
                _SetProcessDPIAware();
            }

            FreeLibrary(hUser32DLL);
        }
    }
}

dtk_result dtk_uninit_backend_apis__win32(dtk_context* pTK)
{
    dtk_assert(pTK != NULL);

    FreeLibrary((HMODULE)pTK->win32.hOle32DLL);
    FreeLibrary((HMODULE)pTK->win32.hComctl32DLL);

    return DTK_SUCCESS;
}

dtk_result dtk_init_backend_apis__win32(dtk_context* pTK)
{
    dtk_assert(pTK != NULL);

    // Comctl32.dll
    HMODULE hComctl32DLL = LoadLibraryW(L"comctl32.dll");
    if (hComctl32DLL == NULL) {
        return DTK_ERROR;
    }

    pTK->win32.hComctl32DLL = (dtk_handle)hComctl32DLL;
    pTK->win32.InitCommonControlsEx = (dtk_proc)GetProcAddress(hComctl32DLL, "InitCommonControlsEx");


    // Ole32.dll
    HMODULE hOle32DLL = LoadLibraryW(L"ole32.dll");
    if (hOle32DLL == NULL) {
        dtk_uninit_backend_apis__win32(pTK);
        return DTK_ERROR;
    }

    pTK->win32.hOle32DLL = (dtk_handle)hOle32DLL;
    pTK->win32.OleInitialize   = (dtk_proc)GetProcAddress(hOle32DLL, "OleInitialize");
    pTK->win32.OleUninitialize = (dtk_proc)GetProcAddress(hOle32DLL, "OleUninitialize");


    // Msimg32.dll
    HMODULE hMsimg32DLL = LoadLibraryW(L"msimg32.dll");
    if (hMsimg32DLL == NULL) {
        return DTK_ERROR;
    }

    pTK->win32.hMsimg32DLL = (dtk_handle)hMsimg32DLL;
    pTK->win32.AlphaBlend = (dtk_proc)GetProcAddress(hMsimg32DLL, "AlphaBlend");

    return DTK_SUCCESS;
}

LRESULT CALLBACK dtk_MessagingWindowProcWin32(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case DTK_WM_LOCAL:
        {
            dtk_event* pEvent = (dtk_event*)lParam;
            dtk_assert(pEvent != NULL);

            dtk_handle_local_event(pEvent);
            dtk_free(pEvent);
        } break;

        case DTK_WM_CUSTOM:
        {
            dtk_custom_event_data* pEventData = (dtk_custom_event_data*)lParam;
            dtk_assert(pEventData != NULL);

            dtk_handle_custom_event(pEventData->pTK, pEventData->pControl, pEventData->eventID, pEventData->pData, pEventData->dataSize);
            dtk_free(pEventData);
        } break;

        default: break;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

dtk_result dtk_init__win32(dtk_context* pTK)
{
    g_dtkInitCounter_Win32 += 1;
    if (g_dtkInitCounter_Win32 == 1) {
        // We'll be handling DPI ourselves. This should be done at the top.
        dtk_make_dpi_aware__win32();

        // Initialize backend APIs.
        dtk_init_backend_apis__win32(pTK);

        // For drag and drop.
        if (pTK->win32.OleInitialize) {
            ((DTK_PFN_OleInitialize)pTK->win32.OleInitialize)(NULL);
        }

        // Need to call this to enable visual styles.
        if (pTK->win32.InitCommonControlsEx) {
            INITCOMMONCONTROLSEX ctls;
            ctls.dwSize = sizeof(ctls);
            ctls.dwICC = ICC_STANDARD_CLASSES;
            ((DTK_PFN_InitCommonControlsEx)pTK->win32.InitCommonControlsEx)(&ctls);
        }

        // Window classes.
        WNDCLASSEXA wc;
        dtk_zero_object(&wc);
        wc.cbSize        = sizeof(wc);
        wc.cbWndExtra    = sizeof(dtk_window*);
        wc.lpfnWndProc   = (WNDPROC)dtk_GenericWindowProc;
        wc.lpszClassName = DTK_WIN32_WINDOW_CLASS;
        wc.hCursor       = LoadCursorA(NULL, MAKEINTRESOURCEA(32512));
        wc.hIcon         = LoadIconA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(101));
        wc.style         = CS_OWNDC | CS_DBLCLKS;
        if (!RegisterClassExA(&wc)) {
            return DTK_ERROR;
        }

        wc.lpszClassName = DTK_WIN32_WINDOW_CLASS_POPUP;
        wc.style         = CS_OWNDC | CS_DBLCLKS /*| CS_DROPSHADOW*/;
        if (!RegisterClassExA(&wc)) {
            UnregisterClassA(DTK_WIN32_WINDOW_CLASS, NULL);
            return DTK_ERROR;
        }

        wc.lpfnWndProc   = (WNDPROC)dtk_MessagingWindowProcWin32;
        wc.lpszClassName = DTK_WIN32_WINDOW_CLASS_MESSAGING;
        if (!RegisterClassExA(&wc)) {
            UnregisterClassA(DTK_WIN32_WINDOW_CLASS_POPUP, NULL);
            UnregisterClassA(DTK_WIN32_WINDOW_CLASS, NULL);
            return DTK_ERROR;
        }
    }

    // Hidden windows.
    pTK->win32.hMessagingWindow = (dtk_handle)CreateWindowExA(0, DTK_WIN32_WINDOW_CLASS_MESSAGING, "", 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
    if (pTK->win32.hMessagingWindow == NULL) {
        return DTK_ERROR;
    }

    // Cursors.
    pTK->win32.hCursorArrow  = (dtk_handle)LoadCursor(NULL, IDC_ARROW);
    pTK->win32.hCursorIBeam  = (dtk_handle)LoadCursor(NULL, IDC_IBEAM);
    pTK->win32.hCursorCross  = (dtk_handle)LoadCursor(NULL, IDC_CROSS);
    pTK->win32.hCursorSizeWE = (dtk_handle)LoadCursor(NULL, IDC_SIZEWE);
    pTK->win32.hCursorSizeNS = (dtk_handle)LoadCursor(NULL, IDC_SIZENS);

    // Graphics.
    pTK->win32.hGraphicsDC = (dtk_handle)CreateCompatibleDC(NULL);

    pTK->platform = dtk_platform_win32;
    return DTK_SUCCESS;
}

dtk_result dtk_uninit__win32(dtk_context* pTK)
{
    (void)pTK;

    dtk_assert(g_dtkInitCounter_Win32 > 0); // Oh no! You have an init/uninit mismatch! Remember, init() and uninit() are _not_ thread-safe.
    if (g_dtkInitCounter_Win32 == 0) {
        return DTK_ERROR;
    }

    DeleteDC((HDC)pTK->win32.hGraphicsDC);
    dtk_free(pTK->win32.pGlyphCache);
    dtk_free(pTK->win32.pCharConvBuffer);

    g_dtkInitCounter_Win32 -= 1;
    if (g_dtkInitCounter_Win32 == 0) {
        dtk_free(pTK->win32.pAccelerators);
        DestroyAcceleratorTable((HACCEL)pTK->win32.hAccel);

        UnregisterClassA(DTK_WIN32_WINDOW_CLASS_MESSAGING, NULL);
        UnregisterClassA(DTK_WIN32_WINDOW_CLASS_POPUP, NULL);
        UnregisterClassA(DTK_WIN32_WINDOW_CLASS, NULL);

        if (pTK->win32.OleUninitialize) {
            ((DTK_PFN_OleUninitialize)pTK->win32.OleUninitialize)();
        }
    }

    return DTK_SUCCESS;
}

dtk_result dtk_next_event__win32(dtk_context* pTK, dtk_bool32 blocking)
{
    MSG msg;
    if (blocking) {
        BOOL result = GetMessageA(&msg, NULL, 0, 0);
        if (result == -1) {
            return DTK_ERROR;   // Unknown error.
        }
        if (result == 0) {
            pTK->exitCode = (int)msg.wParam;
            return DTK_QUIT;
        }
    } else {
        BOOL result = PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE);
        if (result == 0) {
            return DTK_NO_EVENT;
        }
        if (msg.message == WM_QUIT) {
            return DTK_QUIT;
        }
    }

    // Handle accelerator keys. If an accelerator key is processed with TranslateAccelerator() we do _not_ want to handle
    // the event with TranslateMessage/DispatchMessage.
    if (TranslateAcceleratorA(msg.hwnd, (HACCEL)pTK->win32.hAccel, &msg)) {
        return DTK_SUCCESS;
    }

    TranslateMessage(&msg);
    DispatchMessageA(&msg);

    return DTK_SUCCESS;
}

dtk_result dtk_post_local_event__win32(dtk_event* pEvent)
{
    dtk_assert(pEvent != NULL);
    dtk_assert(pEvent->pTK != NULL);

    dtk_event* pEventCopy = (dtk_event*)dtk_malloc(sizeof(*pEventCopy));
    if (pEventCopy == NULL) {
        return DTK_OUT_OF_MEMORY;
    }

    *pEventCopy = *pEvent;
    PostMessageA((HWND)pEvent->pTK->win32.hMessagingWindow, DTK_WM_LOCAL, (WPARAM)0, (LPARAM)pEventCopy);

    return DTK_SUCCESS;
}

dtk_result dtk_post_custom_event__win32(dtk_context* pTK, dtk_control* pControl, dtk_uint32 eventID, const void* pData, size_t dataSize)
{
    // We need a copy of the data. This will be freed in dtk_GenericWindowProc().
    dtk_custom_event_data* pEventData = (dtk_custom_event_data*)dtk_malloc(sizeof(dtk_custom_event_data) + dataSize);
    if (pEventData == NULL) {
        return DTK_OUT_OF_MEMORY;
    }

    pEventData->pTK = pTK;
    pEventData->pControl = pControl;
    pEventData->eventID = eventID;
    pEventData->dataSize = dataSize;
    if (pData != NULL && dataSize > 0) memcpy(pEventData->pData, pData, dataSize);
    PostMessageA((HWND)pTK->win32.hMessagingWindow, DTK_WM_CUSTOM, (WPARAM)eventID, (LPARAM)pEventData);

    return DTK_SUCCESS;
}

dtk_result dtk_post_paint_notification_event__win32(dtk_context* pTK, dtk_window* pWindow)
{
    (void)pTK;
    PostMessageA((HWND)pWindow->win32.hWnd, DTK_WM_PAINT_NOTIFICATION, (WPARAM)0, (LPARAM)pWindow);

    return DTK_SUCCESS;
}


dtk_result dtk_post_quit_event__win32(dtk_context* pTK, int exitCode)
{
    (void)pTK;

    PostQuitMessage(exitCode);
    return DTK_SUCCESS;
}


dtk_result dtk_recreate_HACCEL__win32(dtk_context* pTK)
{
    ACCEL* pAccels = (ACCEL*)dtk_malloc(pTK->win32.acceleratorCount * sizeof(*pAccels));
    if (pAccels == NULL) {
        return DTK_OUT_OF_MEMORY;
    }

    for (dtk_uint32 i = 0; i < pTK->win32.acceleratorCount; ++i) {
        dtk_uint32 modifiers = pTK->win32.pAccelerators[i].modifiers;

        ACCEL a;
        a.key = dtk_convert_key_to_win32(pTK->win32.pAccelerators[i].key);
        a.cmd = (WORD)i;    // <-- The command is set to the index. In the WM_COMMAND event handler we'll use this as a lookup into an array.

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

        pAccels[i] = a;
    }

    HACCEL hAccel = CreateAcceleratorTableA(pAccels, (int)pTK->win32.acceleratorCount);
    if (hAccel == NULL) {
        dtk_free(pAccels);
        return DTK_ERROR;
    }

    DestroyAcceleratorTable((HACCEL)pTK->win32.hAccel);
    pTK->win32.hAccel = hAccel;

    dtk_free(pAccels);
    return DTK_SUCCESS;
}

dtk_bool32 dtk_find_accelerator__win32(dtk_context* pTK, dtk_key key, dtk_uint32 modifiers, dtk_uint32* pIndex)
{
    if (pIndex) *pIndex = (dtk_uint32)-1;
    for (dtk_uint32 i = 0; i < pTK->win32.acceleratorCount; ++i) {
        if (pTK->win32.pAccelerators[i].key == key && pTK->win32.pAccelerators[i].modifiers == modifiers) {
            if (pIndex) *pIndex = i;
            return DTK_TRUE;
        }
    }

    return DTK_FALSE;
}

dtk_result dtk_bind_accelerators__win32(dtk_context* pTK, dtk_accelerator* pAccelerators, dtk_uint32 count)
{
    // Whenever a new accelerator is bound we need to delete and recreate the entire table.
    dtk_uint32 newCount = pTK->win32.acceleratorCount + count;
    if (newCount < 1 || newCount > 65536) {
        return DTK_OUT_OF_RANGE;    // 16-bit maximum for Win32.
    }

    if (newCount > pTK->win32.acceleratorCapacity) {
        dtk_uint32 newCapacity = pTK->win32.acceleratorCapacity + count;
        dtk_accelerator* pNewAccelerators = (dtk_accelerator*)dtk_realloc(pTK->win32.pAccelerators, newCapacity * sizeof(*pNewAccelerators));
        if (pNewAccelerators == NULL) {
            return DTK_OUT_OF_MEMORY;
        }

        pTK->win32.pAccelerators = pNewAccelerators;
        pTK->win32.acceleratorCapacity = newCapacity;
    }

    dtk_assert(newCount <= pTK->win32.acceleratorCapacity);

    for (dtk_uint32 i = 0; i < count; ++i) {
        dtk_uint32 existingIndex;
        if (dtk_find_accelerator__win32(pTK, pAccelerators[i].key, pAccelerators[i].modifiers, &existingIndex)) {
            // Already exists. Replace.
            pTK->win32.pAccelerators[existingIndex].id = pAccelerators[i].id;
        } else {
            // Does not exist. Insert.
            pTK->win32.pAccelerators[pTK->win32.acceleratorCount] = pAccelerators[i];
            pTK->win32.acceleratorCount += 1;
        }
    }

    return dtk_recreate_HACCEL__win32(pTK);
}

dtk_result dtk_unbind_accelerator__win32(dtk_context* pTK, dtk_accelerator accelerator)
{
    dtk_uint32 i;
    if (!dtk_find_accelerator__win32(pTK, accelerator.key, accelerator.modifiers, &i)) {
        return DTK_ERROR;   // Accelerator is not bound.
    }

    for (dtk_uint32 j = i; j < pTK->win32.acceleratorCount-1; ++j) {
        pTK->win32.pAccelerators[j] = pTK->win32.pAccelerators[j+1];
    }
    pTK->win32.acceleratorCount -= 1;

    return dtk_recreate_HACCEL__win32(pTK);
}


dtk_result dtk_get_screen_size__win32(dtk_context* pTK, dtk_uint32* pSizeX, dtk_uint32* pSizeY)
{
    assert(pTK != NULL);
    (void)pTK;

    HDC hDC = GetDC(NULL);
    if (pSizeX) *pSizeX = GetDeviceCaps(hDC, HORZRES);
    if (pSizeY) *pSizeY = GetDeviceCaps(hDC, VERTRES);

    return DTK_SUCCESS;
}


float dtk_get_dpi_scale__win32(dtk_context* pTK)
{
    if (pTK == NULL) return 1;
    return GetDeviceCaps(GetDC(NULL), LOGPIXELSX) / 96.0f;
}


dtk_result dtk__capture_keyboard_window__win32(dtk_context* pTK, dtk_window* pWindow)
{
    (void)pTK;

    SetFocus((HWND)pWindow->win32.hWnd);
    return DTK_SUCCESS;
}

dtk_result dtk__release_keyboard_window__win32(dtk_context* pTK)
{
    (void)pTK;

    SetFocus(NULL);
    return DTK_SUCCESS;
}

dtk_result dtk__capture_mouse_window__win32(dtk_context* pTK, dtk_window* pWindow)
{
    (void)pTK;

    SetCapture((HWND)pWindow->win32.hWnd);
    return DTK_SUCCESS;
}

dtk_result dtk__release_mouse_window__win32(dtk_context* pTK)
{
    (void)pTK;

    if (!ReleaseCapture()) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

wchar_t* dtk__mb_to_wchar__win32(dtk_context* pTK, const char* text, size_t textSizeInBytes, size_t* pCharacterCount)
{
    int wcharCount = 0;

    // We first try to copy the string into the already-allocated buffer. If it fails we fall back to the slow path which requires
    // two conversions.
    if (pTK->win32.pCharConvBuffer == NULL) {
        goto fallback;
    }

    wcharCount = MultiByteToWideChar(CP_UTF8, 0, text, (int)textSizeInBytes, (wchar_t*)pTK->win32.pCharConvBuffer, (int)pTK->win32.charConvBufferSize);
    if (wcharCount != 0) {
        if (pCharacterCount) *pCharacterCount = (size_t)wcharCount;
        return (wchar_t*)pTK->win32.pCharConvBuffer;
    }


fallback:;
    wcharCount = MultiByteToWideChar(CP_UTF8, 0, text, (int)textSizeInBytes, NULL, 0);
    if (wcharCount == 0) {
        return NULL;
    }

    if (pTK->win32.charConvBufferSize < (size_t)(wcharCount+1) * sizeof(wchar_t)) {
        dtk_free(pTK->win32.pCharConvBuffer);
        pTK->win32.pCharConvBuffer    = dtk_malloc((wcharCount+1) * sizeof(wchar_t));
        pTK->win32.charConvBufferSize = wcharCount + 1;
    }

    wcharCount = MultiByteToWideChar(CP_UTF8, 0, text, (int)textSizeInBytes, (wchar_t*)pTK->win32.pCharConvBuffer, (int)pTK->win32.charConvBufferSize);
    if (wcharCount == 0) {
        return NULL;
    }

    if (pCharacterCount != NULL) *pCharacterCount = wcharCount;
    return (wchar_t*)pTK->win32.pCharConvBuffer;
}

dtk_result dtk_win32_error_to_result(DWORD error)
{
#if 0
    switch (error)
    {
        default: return DTK_ERROR;  // Generic error.
    }
#endif

    (void)error;
    return DTK_ERROR;
}


dtk_result dtk_init_default_font_by_type__win32(dtk_context* pTK, dtk_application_font_type type, dtk_font* pFont)
{
    dtk_assert(pTK != NULL);
    dtk_assert(pFont != NULL);

    switch (type)
    {
        case dtk_application_font_type_monospace:
        {
            // Use Consolas by default, except for when XP is being used in which case we'll use Courier New.
            OSVERSIONINFOEXA version;
            ZeroMemory(&version, sizeof(version));
            version.dwOSVersionInfoSize = sizeof(version);
            version.dwMajorVersion = 5;
            if (VerifyVersionInfoA(&version, VER_MAJORVERSION, VerSetConditionMask(0, VER_MAJORVERSION, VER_LESS_EQUAL))) {
                // XP
                return dtk_font_init(pTK, "Courier New", 13, dtk_font_weight_normal, dtk_font_slant_none, 0, pFont);
            } else {
                // Vista+
                return dtk_font_init(pTK, "Consolas", 13, dtk_font_weight_normal, dtk_font_slant_none, 0, pFont);
            }
        };

        case dtk_application_font_type_ui:
        default:
        {
            return dtk_font_init(pTK, "Segoe UI", 12, dtk_font_weight_normal, dtk_font_slant_none, 0, pFont);
        };
    }
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
static dtk_uint32 g_dtkInitCounter_GTK = 0;
static dtk_bool32 g_dtkHasQuit = DTK_FALSE;

dtk_result dtk_init_backend_apis__gtk(dtk_context* pTK)
{
    dtk_assert(pTK != NULL);

    // GTK
    
    
    // Cairo


    return DTK_SUCCESS;
}

dtk_result dtk_uninit_backend_apis__gtk(dtk_context* pTK)
{
    dtk_assert(pTK != NULL);

    return DTK_SUCCESS;
}

void dtk_log_handler__gtk(const gchar *domain, GLogLevelFlags level, const gchar *message, gpointer pUserData)
{
    dtk_context* pTK = (dtk_context*)pUserData;
    dtk_assert(pTK != NULL);

    const char* tag = "";

    dtk_log_proc onLog = pTK->onLog;
    if (onLog) {
        if ((level & (G_LOG_FLAG_FATAL | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL)) != 0) {
            tag = "[ERROR] ";
        }
        if ((level & (G_LOG_LEVEL_WARNING)) != 0) {
            tag = "[WARNING] ";
        }
        if ((level & (G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO)) != 0) {
            tag = "";
        }

        char* formattedMessage = dtk_make_stringf("%s %s - %s", tag, domain, message);
        if (formattedMessage != NULL) {
            onLog(pTK, formattedMessage);
            dtk_free_string(formattedMessage);
        } else {
            g_log_default_handler(domain, level, message, NULL);
        }
    } else {
        g_log_default_handler(domain, level, message, NULL);
    }
}

#if GLIB_CHECK_VERSION(2, 50, 0)
GLogWriterOutput dtk_log_writer_handler__gtk(GLogLevelFlags level, const GLogField *fields, gsize fieldCount, gpointer pUserData)
{
    dtk_context* pTK = (dtk_context*)pUserData;
    dtk_assert(pTK != NULL);

    const char* tag = "";
    const char* domain = "";
    const char* message = "";
    for (gsize i = 0; i < fieldCount; ++i) {
        if (strcmp(fields[i].key, "GLIB_DOMAIN") == 0) {
            domain = (const char*)fields[i].value;
        }
        if (strcmp(fields[i].key, "MESSAGE") == 0) {
            message = (const char*)fields[i].value;
        }
    }

    dtk_log_proc onLog = pTK->onLog;
    if (onLog) {
        if ((level & (G_LOG_FLAG_FATAL | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL)) != 0) {
            tag = "[ERROR] ";
        }
        if ((level & (G_LOG_LEVEL_WARNING)) != 0) {
            tag = "[WARNING] ";
        }
        if ((level & (G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO)) != 0) {
            tag = "";
        }

        char* formattedMessage = dtk_make_stringf("%s%s - %s", tag, domain, message);
        if (formattedMessage != NULL) {
            onLog(pTK, formattedMessage);
            dtk_free_string(formattedMessage);
            return G_LOG_WRITER_HANDLED;
        } else {
            return g_log_writer_default(level, fields, fieldCount, pUserData);
        }
    } else {
        return g_log_writer_default(level, fields, fieldCount, pUserData);
    }
}
#endif

dtk_result dtk_init__gtk(dtk_context* pTK)
{
    g_dtkInitCounter_GTK += 1;
    if (g_dtkInitCounter_GTK == 1) {
        // Initialize backend APIs.
        dtk_init_backend_apis__gtk(pTK);
        
        if (!gtk_init_check(0, NULL)) {
            dtk_uninit_backend_apis__gtk(pTK);
            return DTK_FAILED_TO_INIT_BACKEND;
        }

        // Logging.
        g_log_set_handler(NULL,   (GLogLevelFlags)(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL), dtk_log_handler__gtk, pTK);
        g_log_set_handler("GLib", (GLogLevelFlags)(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL), dtk_log_handler__gtk, pTK);
        g_log_set_handler("Gtk",  (GLogLevelFlags)(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL), dtk_log_handler__gtk, pTK);

#if GLIB_CHECK_VERSION(2, 50, 0)
        g_log_set_writer_func(dtk_log_writer_handler__gtk, pTK, NULL);
#endif

        pTK->gtk.pCursorDefault      = (dtk_ptr)gdk_cursor_new_for_display(gdk_display_get_default(), GDK_LEFT_PTR);
        pTK->gtk.pCursorIBeam        = (dtk_ptr)gdk_cursor_new_for_display(gdk_display_get_default(), GDK_XTERM);
        pTK->gtk.pCursorCross        = (dtk_ptr)gdk_cursor_new_for_display(gdk_display_get_default(), GDK_CROSSHAIR);
        pTK->gtk.pCursorDoubleArrowH = (dtk_ptr)gdk_cursor_new_for_display(gdk_display_get_default(), GDK_SB_H_DOUBLE_ARROW);
        pTK->gtk.pCursorDoubleArrowV = (dtk_ptr)gdk_cursor_new_for_display(gdk_display_get_default(), GDK_SB_V_DOUBLE_ARROW);

        pTK->gtk.pAccelGroup = (dtk_ptr)gtk_accel_group_new();
        if (pTK->gtk.pAccelGroup == NULL) {
            dtk_uninit_backend_apis__gtk(pTK);
            return DTK_FAILED_TO_INIT_BACKEND;
        }
    }

    pTK->platform = dtk_platform_gtk;
    return DTK_SUCCESS;
}

dtk_result dtk_uninit__gtk(dtk_context* pTK)
{
    dtk_assert(g_dtkInitCounter_GTK > 0); // Oh no! You have an init/uninit mismatch! Remember, init() and uninit() are _not_ thread-safe.
    if (g_dtkInitCounter_GTK == 0) {
        return DTK_ERROR;
    }

    g_dtkInitCounter_GTK -= 1;
    if (g_dtkInitCounter_GTK == 0) {
        g_object_unref(G_OBJECT(pTK->gtk.pAccelGroup));

        g_object_unref(G_OBJECT(pTK->gtk.pCursorDefault));
        g_object_unref(G_OBJECT(pTK->gtk.pCursorIBeam));
        g_object_unref(G_OBJECT(pTK->gtk.pCursorCross));
        g_object_unref(G_OBJECT(pTK->gtk.pCursorDoubleArrowH));
        g_object_unref(G_OBJECT(pTK->gtk.pCursorDoubleArrowV));
        
        // Note: There is no gtk_uninit()

        dtk_init_backend_apis__gtk(pTK);
    }

    return DTK_SUCCESS;
}

dtk_result dtk_next_event__gtk(dtk_context* pTK, dtk_bool32 blocking)
{
    if (!blocking) {
        if (!gtk_events_pending()) {
            return DTK_NO_EVENT;
        }
    }

    gtk_main_iteration();
    
    // If we just handled a quit message make sure we return appropraitely.
    if (g_dtkHasQuit) {
        dtk_atomic_exchange_32(&g_dtkHasQuit, DTK_FALSE); // <-- Reset this to false to ensure we can get back into a new loop later on.
        return DTK_QUIT;
    }
    
    return DTK_SUCCESS;
}


static gboolean dtk_post_local_event_cb__gtk(dtk_event* pEvent)
{
    dtk_handle_local_event(pEvent);

    dtk_free(pEvent);
    return FALSE;
}

dtk_result dtk_post_local_event__gtk(dtk_event* pEvent)
{
    // We need a copy of the data.
    dtk_event* pEventCopy = (dtk_event*)dtk_malloc(sizeof(*pEventCopy));
    if (pEventCopy == NULL) {
        return DTK_OUT_OF_MEMORY;
    }

    *pEventCopy = *pEvent;
    
    g_idle_add((GSourceFunc)dtk_post_local_event_cb__gtk, pEventCopy);
    return DTK_SUCCESS;
}


static gboolean dtk_post_custom_event_cb__gtk(dtk_custom_event_data* pEventData)
{
    dtk_handle_custom_event(pEventData->pTK, pEventData->pControl, pEventData->eventID, pEventData->pData, pEventData->dataSize);

    dtk_free(pEventData);
    return FALSE;
}

dtk_result dtk_post_custom_event__gtk(dtk_context* pTK, dtk_control* pControl, dtk_uint32 eventID, const void* pData, size_t dataSize)
{
    // We need a copy of the data.
    dtk_custom_event_data* pEventData = (dtk_custom_event_data*)dtk_malloc(sizeof(dtk_custom_event_data) + dataSize);
    if (pEventData == NULL) {
        return DTK_OUT_OF_MEMORY;
    }

    pEventData->pTK = pTK;
    pEventData->pControl = pControl;
    pEventData->eventID = eventID;
    pEventData->dataSize = dataSize;
    if (pData != NULL && dataSize > 0) memcpy(pEventData->pData, pData, dataSize);
    
    g_idle_add((GSourceFunc)dtk_post_custom_event_cb__gtk, pEventData);
    return DTK_SUCCESS;
}


static gboolean dtk_post_paint_notification_event_cb__gtk(dtk_window* pWindow)
{
    dtk_handle_paint_notification_event(DTK_CONTROL(pWindow)->pTK, pWindow);
    return FALSE;
}

dtk_result dtk_post_paint_notification_event__gtk(dtk_context* pTK, dtk_window* pWindow)
{
    g_idle_add((GSourceFunc)dtk_post_paint_notification_event_cb__gtk, pWindow);
    return DTK_SUCCESS;
}


dtk_result dtk_post_quit_event__gtk(dtk_context* pTK, int exitCode)
{
    // When this is called, there's a chance we're waiting on gtk_main_iteration(). We'll need to
    // make sure we wake that up so it can see we have quit.
    dtk_atomic_exchange_32(&g_dtkHasQuit, DTK_TRUE);
    dtk_atomic_exchange_32(&pTK->exitCode, exitCode);
    g_main_context_wakeup(NULL);
    
    return DTK_SUCCESS;
}


dtk_window* dtk_find_window_by_gobject__gtk(dtk_context* pTK, GObject* pObject)
{
    for (dtk_window* pWindow = pTK->pFirstWindow; pWindow != NULL; pWindow = pWindow->pNextWindow) {
        if (pWindow->gtk.pWidget == pObject) {
            return pWindow;
        }
    }

    return NULL;
}

dtk_bool32 dtk_find_accelerator__gtk(dtk_context* pTK, dtk_key key, dtk_uint32 modifiers, dtk_uint32* pIndex)
{
    if (pIndex) *pIndex = (dtk_uint32)-1;
    for (dtk_uint32 i = 0; i < pTK->gtk.acceleratorCount; ++i) {
        if (pTK->gtk.pAccelerators[i].accelerator.key == key && pTK->gtk.pAccelerators[i].accelerator.modifiers == modifiers) {
            if (pIndex) *pIndex = i;
            return DTK_TRUE;
        }
    }

    return DTK_FALSE;
}

static gboolean dtk__on_accelerator__gtk(GtkAccelGroup *pAccelGroup, GObject *acceleratable, guint keyvalGTK, GdkModifierType modifiersGTK, gpointer pUserData)
{
    (void)pAccelGroup;
    (void)acceleratable;

    dtk_context* pTK = (dtk_context*)pUserData;
    dtk_assert(pTK != NULL);

    dtk_key key = dtk_convert_key_from_gtk(gdk_keyval_to_upper(keyvalGTK));
    dtk_uint32 modifiers = dtk_accelerator_modifiers_from_gtk(modifiersGTK);

    dtk_uint32 index;
    if (dtk_find_accelerator__gtk(pTK, key, modifiers, &index)) {
        dtk_event e;
        e.type = DTK_EVENT_ACCELERATOR;
        e.pTK = pTK;
        e.pControl = DTK_CONTROL(dtk_find_window_by_gobject__gtk(pTK, acceleratable));
        e.accelerator.key = pTK->gtk.pAccelerators[index].accelerator.key;
        e.accelerator.modifiers = pTK->gtk.pAccelerators[index].accelerator.modifiers;
        e.accelerator.id = pTK->gtk.pAccelerators[index].accelerator.id;
        dtk_handle_global_event(&e);
    }

    return DTK_TRUE;    // Returning true here is important because it ensures the accelerator is handled only once.
}

dtk_result dtk_bind_accelerators__gtk(dtk_context* pTK, dtk_accelerator* pAccelerators, dtk_uint32 count)
{
    dtk_uint32 newCount = pTK->gtk.acceleratorCount + count;
    if (newCount > pTK->gtk.acceleratorCapacity) {
        dtk_uint32 newCapacity = pTK->gtk.acceleratorCapacity + count;
        dtk_accelerator_gtk* pNewAccelerators = (dtk_accelerator_gtk*)dtk_realloc(pTK->gtk.pAccelerators, newCapacity * sizeof(*pNewAccelerators));
        if (pNewAccelerators == NULL) {
            return DTK_OUT_OF_MEMORY;
        }

        pTK->gtk.pAccelerators = pNewAccelerators;
        pTK->gtk.acceleratorCapacity = newCapacity;
    }

    dtk_assert(newCount <= pTK->gtk.acceleratorCapacity);

    for (dtk_uint32 i = 0; i < count; ++i) {
        guint keyvalGTK = dtk_convert_key_to_gtk(pAccelerators[i].key);
        GdkModifierType modifiersGTK = dtk_accelerator_modifiers_to_gtk(pAccelerators[i].modifiers);
        if (keyvalGTK > 0) {
            dtk_unbind_accelerator(pTK, pAccelerators[i]);   // With GTK it's easier to just unbind the existing accelerator completely and start over.

            dtk_uint32 index = pTK->gtk.acceleratorCount;
            pTK->gtk.pAccelerators[index].accelerator = pAccelerators[i];
            pTK->gtk.pAccelerators[index].pClosure = g_cclosure_new(G_CALLBACK(dtk__on_accelerator__gtk), pTK, NULL);
            pTK->gtk.acceleratorCount += 1;

            gtk_accel_group_connect(GTK_ACCEL_GROUP(pTK->gtk.pAccelGroup), keyvalGTK, modifiersGTK, (GtkAccelFlags)0, (GClosure*)pTK->gtk.pAccelerators[index].pClosure);
        }
    }

    return DTK_SUCCESS;
}

dtk_result dtk_unbind_accelerator__gtk(dtk_context* pTK, dtk_accelerator accelerator)
{
    dtk_uint32 index;
    if (!dtk_find_accelerator__gtk(pTK, accelerator.key, accelerator.modifiers, &index)) {
        return DTK_ERROR;   // Accelerator is not bound.
    }

    guint keyvalGTK = dtk_convert_key_to_gtk(accelerator.key);
    GdkModifierType modifiersGTK = dtk_accelerator_modifiers_to_gtk(accelerator.modifiers);
    if (!gtk_accel_group_disconnect_key(GTK_ACCEL_GROUP(pTK->gtk.pAccelGroup), keyvalGTK, modifiersGTK)) {  // <-- This will unref the closure.
        return DTK_ERROR;   // Failed to disconnect from the GTK accelerator group.
    }

    for (dtk_uint32 j = index; j < pTK->gtk.acceleratorCount-1; ++j) {
        pTK->gtk.pAccelerators[j] = pTK->gtk.pAccelerators[j+1];
    }
    pTK->gtk.acceleratorCount -= 1;

    return DTK_SUCCESS;
}


dtk_result dtk_get_screen_size__gtk(dtk_context* pTK, dtk_uint32* pSizeX, dtk_uint32* pSizeY)
{
    assert(pTK != NULL);
    (void)pTK;

    // Naturally, GTK has deprecated it's simple APIs and replaced them with complex ones. *sigh*
#if GTK_CHECK_VERSION(3, 22, 0)
    // 3.22+ (Monitor API)
    //
    // With the monitor API we will iterate over each monitor attached to the default display and union each of their
    // rectangles to determine the full screen size.
    GdkDisplay* pDisplay = gdk_display_get_default();
    if (pDisplay == NULL) {
        return DTK_NO_DISPLAY;
    }

    dtk_rect screenRect = dtk_rect_init(0, 0, 0, 0);

    gint monitorCount = gdk_display_get_n_monitors(pDisplay);
    for (gint iMonitor = 0; iMonitor < monitorCount; ++iMonitor) {
        GdkMonitor* pMonitor = gdk_display_get_monitor(pDisplay, iMonitor);
        if (pMonitor != NULL) {
            GdkRectangle monitorRect;
            gdk_monitor_get_geometry(pMonitor, &monitorRect);
            screenRect = dtk_rect_union(screenRect, dtk_rect_init(monitorRect.x, monitorRect.y, monitorRect.x + monitorRect.width, monitorRect.y + monitorRect.height));
        }
    }

    if (pSizeX) *pSizeX = (dtk_uint32)(screenRect.right - screenRect.left);
    if (pSizeY) *pSizeY = (dtk_uint32)(screenRect.bottom - screenRect.top);
#else
    // < 3.22 (Screen API)
    GdkScreen* pScreen = gdk_screen_get_default();
    if (pScreen == NULL) {
        return DTK_NO_SCREEN;
    }

    if (pSizeX) *pSizeX = (dtk_uint32)gdk_screen_get_width(pScreen);
    if (pSizeY) *pSizeY = (dtk_uint32)gdk_screen_get_height(pScreen);
#endif

    return DTK_SUCCESS;
}


float dtk_get_dpi_scale__gtk(dtk_context* pTK)
{
    if (pTK == NULL) return 1;

    // Not really sure if GTK has any notion of a system wide DPI scale.
    return 1;
}


dtk_result dtk__capture_keyboard_window__gtk(dtk_context* pTK, dtk_window* pWindow)
{
    (void)pTK;

    gtk_widget_grab_focus(GTK_WIDGET(pWindow->gtk.pWidget));
    return DTK_SUCCESS;
}

dtk_result dtk__release_keyboard_window__gtk(dtk_context* pTK)
{
    (void)pTK;

    // From what I can tell it appears there isn't actually a way to ungrab the focus. Passing NULL to gtk_widget_grab_focus() results in an
    // error, so I'm not quite sure how do it, or if it's even needed...
    return DTK_SUCCESS;
}

dtk_result dtk__capture_mouse_window__gtk(dtk_context* pTK, dtk_window* pWindow)
{
    (void)pTK;

#if (GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 20) // GTK 3.20+
    GdkDevice* pPointerDevice = gdk_seat_get_pointer(gdk_display_get_default_seat(gdk_display_get_default()));
    gdk_seat_grab(gdk_device_get_seat(pPointerDevice),
        gtk_widget_get_window(GTK_WIDGET(pWindow->gtk.pClientArea)), GDK_SEAT_CAPABILITY_ALL_POINTING, FALSE, NULL, NULL, NULL, NULL);
#else
    GdkDevice* pPointerDevice = gdk_device_manager_get_client_pointer(gdk_display_get_device_manager(gdk_display_get_default()));
	gdk_device_grab(pPointerDevice, gtk_widget_get_window(GTK_WIDGET(pWindow->gtk.pClientArea)), GDK_OWNERSHIP_APPLICATION, FALSE,
		GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK, NULL, GDK_CURRENT_TIME);
#endif

    // NOTE: Unlike dtk__release_mouse_window__gtk() below, we don't manually post the event here. Instead, it is done generically in dtk__capture_mouse_window().

    return DTK_SUCCESS;
}

dtk_result dtk__release_mouse_window__gtk(dtk_context* pTK)
{
#if (GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 20) // GTK 3.20+
    GdkDevice* pPointerDevice = gdk_seat_get_pointer(gdk_display_get_default_seat(gdk_display_get_default()));
    gdk_seat_ungrab(gdk_device_get_seat(pPointerDevice));
#else
    GdkDevice* pPointerDevice = gdk_device_manager_get_client_pointer(gdk_display_get_device_manager(gdk_display_get_default()));
	gdk_device_ungrab(pPointerDevice, GDK_CURRENT_TIME);
#endif

    // GTK/GDK is a little bit different when it comes to when the mouse release event is posted to the window compared to Win32. It appears
    // to not post the event when the device is ungrab explicitly (possibly because I'm doing something wrong, but I don't know what). To
    // work around this we just post it manually.
    dtk_event eRelease = dtk_event_init(pTK, DTK_EVENT_RELEASE_MOUSE, DTK_CONTROL(pTK->pWindowWithMouseCapture));
    eRelease.releaseMouse.pNewCapturedControl = NULL;
    dtk_post_local_event(&eRelease);

    return DTK_SUCCESS;
}


dtk_font_weight dtk_font_weight_from_pango(PangoWeight weight)
{
    if (weight == PANGO_WEIGHT_THIN) {
        return dtk_font_weight_thin;
    } else if (weight == PANGO_WEIGHT_ULTRALIGHT) {
        return dtk_font_weight_extra_light;
    } else if (weight == PANGO_WEIGHT_LIGHT) {
        return dtk_font_weight_light;
    } else if (weight == PANGO_WEIGHT_SEMILIGHT) {
        return dtk_font_weight_semi_light;
    } else if (weight == PANGO_WEIGHT_BOOK) {
        return dtk_font_weight_book;
    } else if (weight == PANGO_WEIGHT_NORMAL) {
        return dtk_font_weight_normal;
    } else if (weight == PANGO_WEIGHT_MEDIUM) {
        return dtk_font_weight_medium;
    } else if (weight == PANGO_WEIGHT_SEMIBOLD) {
        return dtk_font_weight_semi_bold;
    } else if (weight == PANGO_WEIGHT_BOLD) {
        return dtk_font_weight_bold;
    } else if (weight == PANGO_WEIGHT_ULTRABOLD) {
        return dtk_font_weight_extra_bold;
    } else if (weight == PANGO_WEIGHT_HEAVY) {
        return dtk_font_weight_heavy;
    } else if (weight == PANGO_WEIGHT_ULTRAHEAVY) {
        return dtk_font_weight_extra_heavy;
    } else {
        return dtk_font_weight_normal;
    }
}

dtk_font_slant dtk_font_slant_from_pango(PangoStyle slant)
{
    if (slant == PANGO_STYLE_OBLIQUE) {
        return dtk_font_slant_oblique;
    } else if (slant == PANGO_STYLE_ITALIC) {
        return dtk_font_slant_italic;
    } else {
        return dtk_font_slant_none;
    }
}

dtk_result dtk_font_init_from_pango_description(dtk_context* pTK, PangoFontDescription* pPangoDesc, dtk_font* pFont)
{
    dtk_assert(pTK != NULL);
    dtk_assert(pPangoDesc != NULL);
    dtk_assert(pFont != NULL);

    const char* family = pango_font_description_get_family(pPangoDesc);

    gint size = pango_font_description_get_size(pPangoDesc);
    if (size > 0) {
        if (!pango_font_description_get_size_is_absolute(pPangoDesc)) {
            size = (unsigned int)(size/PANGO_SCALE * (96.0/72.0));
        }
    }

    dtk_font_weight weight = dtk_font_weight_from_pango(pango_font_description_get_weight(pPangoDesc));
    dtk_font_slant slant   = dtk_font_slant_from_pango(pango_font_description_get_style(pPangoDesc));

    return dtk_font_init(pTK, family, size, weight, slant, 0, pFont);
}

dtk_result dtk_init_default_font_by_type__gtk(dtk_context* pTK, dtk_application_font_type type, dtk_font* pFont)
{
    dtk_assert(pTK != NULL);
    dtk_assert(pFont != NULL);

    switch (type)
    {
        case dtk_application_font_type_monospace:
        {
            dtk_result result = DTK_ERROR;

            GSettings* settings = g_settings_new("org.gnome.desktop.interface");
            if (settings != NULL) {
                char* fontName = g_settings_get_string(settings, "monospace-font-name");
                if (fontName != NULL) {
                    PangoFontDescription* pPangoDesc = pango_font_description_from_string(fontName);
                    if (pPangoDesc != NULL) {
                        result = dtk_font_init_from_pango_description(pTK, pPangoDesc, pFont);
                        pango_font_description_free(pPangoDesc);
                    }
                }

                g_object_unref(settings);
            }
        };

        case dtk_application_font_type_ui:
        default:
        {
            dtk_result result = DTK_ERROR;
            GSettings* settings = g_settings_new("org.gnome.desktop.interface");
            if (settings != NULL) {
                char* fontName = g_settings_get_string(settings, "font-name");
                if (fontName != NULL) {
                    PangoFontDescription* pPangoDesc = pango_font_description_from_string(fontName);
                    if (pPangoDesc != NULL) {
                        result = dtk_font_init_from_pango_description(pTK, pPangoDesc, pFont);
                        pango_font_description_free(pPangoDesc);
                    }
                }

                g_object_unref(settings);
            }
        };
    }

    // Should never get here.
    return DTK_ERROR;
}
#endif

dtk_result dtk_init(dtk_context* pTK, dtk_event_proc onEvent, void* pUserData)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pTK);
    pTK->onEvent = onEvent;
    pTK->pUserData = pUserData;

    // Detect the platform first.
    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (result != DTK_SUCCESS) {
        result = dtk_init__win32(pTK);
    }
#endif
#ifdef DTK_GTK
    if (result != DTK_SUCCESS) {
        result = dtk_init__gtk(pTK);
    }
#endif
    if (result != DTK_SUCCESS) {
        return result;
    }

    result = dtk_paint_queue_init(&pTK->paintQueue);
    if (result != DTK_SUCCESS) {
        return result;
    }

    return result;
}

dtk_result dtk_uninit(dtk_context* pTK)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    if (pTK->isMonospaceFontInitialized) {
        dtk_font_uninit(&pTK->monospaceFont);
    }
    if (pTK->isUIFontInitialized) {
        dtk_font_uninit(&pTK->uiFont);
    }

    dtk_paint_queue_uninit(&pTK->paintQueue);
    
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        dtk_uninit__win32(pTK);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        dtk_uninit__gtk(pTK);
    }
#endif

    return DTK_SUCCESS;
}

dtk_result dtk_set_event_callback(dtk_context* pTK, dtk_event_proc proc)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    dtk_atomic_exchange_ptr(&pTK->onEvent, proc);
    return DTK_SUCCESS;
}

dtk_result dtk_next_event(dtk_context* pTK, dtk_bool32 blocking, int* pExitCode)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_next_event__win32(pTK, blocking);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_next_event__gtk(pTK, blocking);
    }
#endif

    if (pExitCode) *pExitCode = pTK->exitCode;
    return result;
}

void dtk_flush_event_queue(dtk_context* pTK)
{
    int exitCode;
    for (;;) {
        dtk_result result = dtk_next_event(pTK, DTK_FALSE, &exitCode);
        if (result == DTK_NO_EVENT) {
            break;
        }

        // Put the quit event back to the queue so the application's main loop can handle it.
        if (result == DTK_QUIT) {
            dtk_post_quit_event(pTK, exitCode);
        }
    }
}

dtk_result dtk_post_local_event(dtk_event* pEvent)
{
    if (pEvent == NULL || pEvent->pTK == NULL) return DTK_INVALID_ARGS;

    if (pEvent->pControl != NULL && pEvent->pControl->isUninitialized) {
        return DTK_INVALID_ARGS;    // Cannot post an event for controls that are being uninitialised.
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pEvent->pTK->platform == dtk_platform_win32) {
        result = dtk_post_local_event__win32(pEvent);
    }
#endif
#ifdef DTK_GTK
    if (pEvent->pTK->platform == dtk_platform_gtk) {
        result = dtk_post_local_event__gtk(pEvent);
    }
#endif

    return result;
}

dtk_bool32 dtk_handle_local_event(dtk_event* pEvent)
{
    if (pEvent == NULL || pEvent->pTK == NULL) return DTK_FALSE;

    // Make sure the event is cancelled if the associated control has been uninitialized.
    dtk_result result = dtk__preprocess_event(pEvent);
    if (result != DTK_SUCCESS) {
        return DTK_FALSE;   // The event was probably cancelled.
    }

    // Do this check _after_ dtk__preprocess_event() because it's possible the event is being called for a keyboard/mouse capture
    // change which allows for this to be NULL. Beyond this point, however, it is not allowed.
    if (pEvent->pControl == NULL) {
        return DTK_FALSE;
    }

    if (pEvent->pControl->isUninitialized) {
        return DTK_FALSE;   // Cannot post an event for controls that are being uninitialised.
    }

    dtk_event_proc onEvent = pEvent->pControl->onEvent;
    if (onEvent) {
        return onEvent(pEvent);
    }

    return DTK_FALSE;
}

dtk_result dtk_post_custom_event(dtk_context* pTK, dtk_control* pControl, dtk_uint32 eventID, const void* pData, size_t dataSize)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    if (pControl != NULL && pControl->isUninitialized) {
        return DTK_INVALID_ARGS;    // Cannot post an event for controls that are being uninitialised.
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_post_custom_event__win32(pTK, pControl, eventID, pData, dataSize);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_post_custom_event__gtk(pTK, pControl, eventID, pData, dataSize);
    }
#endif

    return result;
}

dtk_result dtk_handle_custom_event(dtk_context* pTK, dtk_control* pControl, dtk_uint32 eventID, const void* pData, size_t dataSize)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    if (pControl != NULL && pControl->isUninitialized) {
        return DTK_INVALID_ARGS;    // Cannot post an event for controls that are being uninitialised.
    }

    dtk_event e;
    e.type = DTK_EVENT_CUSTOM;
    e.pTK = pTK;
    e.pControl = pControl;
    e.custom.id = eventID;
    e.custom.dataSize = dataSize;
    e.custom.pData = pData;
    return dtk_handle_global_event(&e);
}

dtk_result dtk_post_paint_notification_event(dtk_context* pTK, dtk_window* pWindow)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_post_paint_notification_event__win32(pTK, pWindow);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_post_paint_notification_event__gtk(pTK, pWindow);
    }
#endif

    return result;
}

dtk_result dtk_handle_paint_notification_event(dtk_context* pTK, dtk_window* pWindow)
{
    if (pTK == NULL || pWindow == NULL) return DTK_INVALID_ARGS;

    // All we do here is an immediate redraw of the window.
    dtk_paint_queue_item item;
    if (dtk_paint_queue_dequeue(&pTK->paintQueue, &item) != DTK_SUCCESS) {
        return DTK_ERROR;
    }

    return dtk_window_immediate_redraw(item.pWindow, item.rect);
}


dtk_bool32 dtk_default_event_handler(dtk_event* pEvent)
{
    if (pEvent == NULL) return DTK_FALSE;

    dtk_context* pTK = pEvent->pTK;
    dtk_assert(pTK != NULL);

    switch (pEvent->type)
    {
        case DTK_EVENT_APPLICATION_SCALE:
        {
            pEvent->applicationScale.scale = 1;
        } return DTK_FALSE; // <-- Don't break here! Return straight away because we don't care about handling this locally.

        default: break;
    }

    return dtk_handle_local_event(pEvent);
}

dtk_result dtk_post_quit_event(dtk_context* pTK, int exitCode)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_post_quit_event__win32(pTK, exitCode);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_post_quit_event__gtk(pTK, exitCode);
    }
#endif

    return result;
}


dtk_result dtk_set_log_callback(dtk_context* pTK, dtk_log_proc proc)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    dtk_atomic_exchange_ptr(&pTK->onLog, proc);
    return DTK_SUCCESS;
}


//// Accelerators ////

dtk_result dtk_bind_accelerator(dtk_context* pTK, dtk_accelerator accelerator)
{
    return dtk_bind_accelerators(pTK, &accelerator, 1);
}

dtk_result dtk_bind_accelerators(dtk_context* pTK, dtk_accelerator* pAccelerators, dtk_uint32 count)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    // Some shortcuts are forbidden as per DevNote 1.1 (Restricted Shortcuts). In this case we just zero out the accelerator.
    for (dtk_uint32 i = 0; i < count; ++i) {
        dtk_accelerator* pAccelerator = &pAccelerators[i];

        dtk_bool32 shouldAcceleratorBeCleared = DTK_FALSE;
        if (pAccelerator->key == DTK_KEY_TAB || pAccelerator->key == '\t') {
            if (pAccelerator->modifiers == 0) {
                shouldAcceleratorBeCleared = DTK_TRUE;  // Tab
            }
            if (pAccelerator->modifiers == DTK_MODIFIER_SHIFT) {
                shouldAcceleratorBeCleared = DTK_TRUE;  // Shift + Tab
            }
        }
        if (pAccelerator->key == DTK_KEY_DELETE) {
            if (pAccelerator->modifiers == 0) {
                shouldAcceleratorBeCleared = DTK_TRUE;  // Delete
            }
        }

        if (shouldAcceleratorBeCleared) {
            dtk_zero_object(pAccelerator);
        }
    }


    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_bind_accelerators__win32(pTK, pAccelerators, count);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_bind_accelerators__gtk(pTK, pAccelerators, count);
    }
#endif

    return result;
}

dtk_result dtk_unbind_accelerator(dtk_context* pTK, dtk_accelerator accelerator)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_unbind_accelerator__win32(pTK, accelerator);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_unbind_accelerator__gtk(pTK, accelerator);
    }
#endif

    return result;
}

dtk_result dtk_get_screen_size(dtk_context* pTK, dtk_uint32* pSizeX, dtk_uint32* pSizeY)
{
    if (pSizeX) *pSizeX = 0;
    if (pSizeY) *pSizeY = 0;
    if (pTK == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_get_screen_size__win32(pTK, pSizeX, pSizeY);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_get_screen_size__gtk(pTK, pSizeX, pSizeY);
    }
#endif

    return DTK_SUCCESS;
}


//// DPI Scaling ////

float dtk_get_system_dpi_scale(dtk_context* pTK)
{
    if (pTK == NULL) return 1;

    float scale = 1;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        scale = dtk_get_dpi_scale__win32(pTK);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        scale = dtk_get_dpi_scale__gtk(pTK);
    }
#endif

    return scale;
}

float dtk_get_application_scaling_factor(dtk_context* pTK)
{
    if (pTK == NULL) return 1;

    dtk_event e = dtk_event_init(pTK, DTK_EVENT_APPLICATION_SCALE, NULL);
    e.applicationScale.scale = 1;
    dtk_handle_global_event(&e);

    return e.applicationScale.scale;
}



//// Input ////

dtk_result dtk_capture_keyboard(dtk_context* pTK, dtk_control* pControl)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    // The control must be allowed to receive capture.
    if (pControl != NULL && !dtk_control_is_keyboard_capture_allowed(pControl)) {
        return DTK_INVALID_ARGS;
    }

    // All we do here is post a capture change event. When the event is taken off the event queue and about to be handled, it will
    // be transformed based on the state at that point in time. Look at dtk__preprocess_event().
    dtk_event eCapture = dtk_event_init(pTK, DTK_EVENT_CHANGE_KEYBOARD_CAPTURE, pControl);
    return dtk_post_local_event(&eCapture);
}

dtk_result dtk_release_keyboard(dtk_context* pTK)
{
    return dtk_capture_keyboard(pTK, NULL);
}

dtk_control* dtk_get_control_with_keyboard_capture(dtk_context* pTK)
{
    if (pTK == NULL) return NULL;
    return pTK->pControlWithKeyboardCapture;
}


dtk_result dtk_capture_mouse(dtk_context* pTK, dtk_control* pControl)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;

    // The control must be allowed to receive capture.
    if (pControl != NULL && !dtk_control_is_mouse_capture_allowed(pControl)) {
        return DTK_INVALID_ARGS;
    }

    // All we do here is post a capture change event. When the event is taken off the event queue and about to be handled, it will
    // be transformed based on the state at that point in time. Look at dtk__preprocess_event().
    dtk_event eCapture = dtk_event_init(pTK, DTK_EVENT_CHANGE_MOUSE_CAPTURE, pControl);
    return dtk_post_local_event(&eCapture);
}

dtk_result dtk_release_mouse(dtk_context* pTK)
{
    return dtk_capture_mouse(pTK, NULL);
}

dtk_control* dtk_get_control_with_mouse_capture(dtk_context* pTK)
{
    if (pTK == NULL) return NULL;
    return pTK->pControlWithMouseCapture;
}


//// Graphics ////

dtk_result dtk_init_default_font_by_type(dtk_context* pTK, dtk_application_font_type type, dtk_font* pFont)
{
    dtk_assert(pTK != NULL);
    dtk_assert(pFont != NULL);

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_init_default_font_by_type__win32(pTK, type, pFont);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_init_default_font_by_type__gtk(pTK, type, pFont);
    }
#endif

    return result;
}

dtk_font* dtk__get_application_font_by_type(dtk_context* pTK, dtk_application_font_type type)
{
    dtk_assert(pTK != NULL);
    
    dtk_event e = dtk_event_init(pTK, DTK_EVENT_APPLICATION_FONT, NULL);
    e.applicationFont.type = type;
    e.applicationFont.pFont = NULL;
    dtk_handle_global_event(&e);

    return e.applicationFont.pFont;
}

dtk_font* dtk_get_ui_font(dtk_context* pTK)
{
    if (pTK == NULL) return NULL;

    dtk_font* pFont = dtk__get_application_font_by_type(pTK, dtk_application_font_type_ui);
    if (pFont != NULL) {
        return pFont;
    }

    if (!pTK->isUIFontInitialized) {
        dtk_result result = dtk_init_default_font_by_type(pTK, dtk_application_font_type_ui, &pTK->uiFont);
        if (result != DTK_SUCCESS) {
            return NULL;
        }

        pTK->isUIFontInitialized = DTK_TRUE;
        pFont = &pTK->uiFont;
    }

    return pFont;
}

dtk_font* dtk_get_monospace_font(dtk_context* pTK)
{
    if (pTK == NULL) return NULL;

    dtk_font* pFont = dtk__get_application_font_by_type(pTK, dtk_application_font_type_monospace);
    if (pFont != NULL) {
        return pFont;
    }

    if (!pTK->isUIFontInitialized) {
        dtk_result result = dtk_init_default_font_by_type(pTK, dtk_application_font_type_monospace, &pTK->monospaceFont);
        if (result != DTK_SUCCESS) {
            return NULL;
        }

        pTK->isUIFontInitialized = DTK_TRUE;
        pFont = &pTK->monospaceFont;
    }

    return pFont;
}


///////////////////////////////////////////////////////////////////////////////
//
// Internal APIs
//
///////////////////////////////////////////////////////////////////////////////
dtk_result dtk__capture_keyboard_window(dtk_context* pTK, dtk_window* pWindow)
{
    dtk_assert(pTK != NULL);
    dtk_assert(pWindow != NULL);

    if (pTK->pWindowWithKeyboardCapture == pWindow) {
        return DTK_SUCCESS; // This window already has the keyboard capture.
    }

    if (pTK->pWindowWithKeyboardCapture != NULL) {
        pTK->pWindowWithKeyboardCapture->isNextKeyboardReleaseExplicit = DTK_TRUE;
    }

    pWindow->isNextKeyboardCaptureExplicit = DTK_TRUE;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk__capture_keyboard_window__win32(pTK, pWindow);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk__capture_keyboard_window__gtk(pTK, pWindow);
    }
#endif

    return result;
}

dtk_result dtk__release_keyboard_window(dtk_context* pTK)
{
    dtk_assert(pTK != NULL);

    if (pTK->pWindowWithKeyboardCapture != NULL) {
        pTK->pWindowWithKeyboardCapture->isNextKeyboardReleaseExplicit = DTK_TRUE;
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk__release_keyboard_window__win32(pTK);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk__release_keyboard_window__gtk(pTK);
    }
#endif

    return result;
}

dtk_result dtk__capture_mouse_window(dtk_context* pTK, dtk_window* pWindow)
{
    dtk_assert(pTK != NULL);
    dtk_assert(pWindow != NULL);

    if (pTK->pWindowWithMouseCapture == pWindow) {
        return DTK_SUCCESS; // This window already has the keyboard capture.
    }

    if (pTK->pWindowWithMouseCapture != NULL) {
        pTK->pWindowWithMouseCapture->isNextMouseReleaseExplicit = DTK_TRUE;
    }

    pWindow->isNextMouseCaptureExplicit = DTK_TRUE;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk__capture_mouse_window__win32(pTK, pWindow);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk__capture_mouse_window__gtk(pTK, pWindow);
    }
#endif

    dtk_event eCapture = dtk_event_init(pTK, DTK_EVENT_CAPTURE_MOUSE, DTK_CONTROL(pWindow));
    eCapture.captureMouse.pOldCapturedControl = pTK->pControlWithMouseCapture;
    dtk_post_local_event(&eCapture);

    return result;
}

dtk_result dtk__release_mouse_window(dtk_context* pTK)
{
    dtk_assert(pTK != NULL);

    if (pTK->pWindowWithMouseCapture != NULL) {
        pTK->pWindowWithMouseCapture->isNextMouseReleaseExplicit = DTK_TRUE;
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk__release_mouse_window__win32(pTK);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk__release_mouse_window__gtk(pTK);
    }
#endif

    return result;
}


void dtk__post_mouse_leave_event_recursive(dtk_context* pTK, dtk_control* pNewControlUnderMouse, dtk_control* pOldControlUnderMouse)
{
    dtk_control* pOldAncestor = pOldControlUnderMouse;
    while (pOldAncestor != NULL) {
        dtk_bool32 isOldControlUnderMouse = pNewControlUnderMouse == pOldAncestor || dtk_control_is_ancestor(pOldAncestor, pNewControlUnderMouse);
        if (!isOldControlUnderMouse) {
            dtk_event e = dtk_event_init(pTK, DTK_EVENT_MOUSE_LEAVE, pOldAncestor);
            dtk_handle_local_event(&e);
        }

        pOldAncestor = pOldAncestor->pParent;
    }
}

void dtk__post_mouse_enter_event_recursive(dtk_context* pTK, dtk_control* pNewControlUnderMouse, dtk_control* pOldControlUnderMouse)
{
    if (pNewControlUnderMouse == NULL) return;

    if (pNewControlUnderMouse->pParent != NULL) {
        dtk__post_mouse_enter_event_recursive(pTK, pNewControlUnderMouse->pParent, pOldControlUnderMouse);
    }

    dtk_bool32 wasNewControlUnderMouse = pOldControlUnderMouse == pNewControlUnderMouse || dtk_control_is_ancestor(pNewControlUnderMouse, pOldControlUnderMouse);
    if (!wasNewControlUnderMouse) {
        dtk_event e = dtk_event_init(pTK, DTK_EVENT_MOUSE_ENTER, pNewControlUnderMouse);
        dtk_handle_local_event(&e);
    }
}
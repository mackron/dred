// Copyright (C) 2016 David Reid. See included LICENSE file.

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

#include "dtk_graphics.c"
#include "dtk_controls.c"
#include "dtk_window.c"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// WIN32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
typedef BOOL (WINAPI * DTK_PFN_InitCommonControlsEx)(const LPINITCOMMONCONTROLSEX lpInitCtrls);

static dtk_uint32 g_dtkInitCounter_Win32 = 0;

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
                fallBackToDiscouragedAPI = DTK_FALSE;
            }
        } else {
            fallBackToDiscouragedAPI = DTK_FALSE;
        }

        FreeLibrary(hSHCoreDLL);
    } else {
        fallBackToDiscouragedAPI = DTK_FALSE;
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

dtk_result dtk_init_backend_apis__win32(dtk_context* pTK)
{
    dtk_assert(pTK != NULL);

    // Comctl32.dll
    HMODULE hComctl32DLL = LoadLibraryW(L"comctl32.dll");
    if (hComctl32DLL == NULL) {
        return DTK_ERROR;
    }

    pTK->win32.hComctl32DLL = hComctl32DLL;
    pTK->win32.InitCommonControlsEx = (dtk_proc)GetProcAddress(hComctl32DLL, "InitCommonControlsEx");


    // Ole32.dll



    return DTK_SUCCESS;
}

dtk_result dtk_uninit_backend_apis__win32(dtk_context* pTK)
{
    dtk_assert(pTK != NULL);

    FreeLibrary(pTK->win32.hComctl32DLL);

    return DTK_SUCCESS;
}

LRESULT CALLBACK dtk_TimerWindowProcWin32(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
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
        OleInitialize(NULL);

        // Need to call this to enable visual styles.
        if (pTK->win32.InitCommonControlsEx) {
            INITCOMMONCONTROLSEX ctls;
            ctls.dwSize = sizeof(ctls);
            ctls.dwICC = ICC_STANDARD_CLASSES;
            ((DTK_PFN_InitCommonControlsEx)pTK->win32.InitCommonControlsEx)(&ctls);
        }

        // Window classes.
        WNDCLASSEXA wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize        = sizeof(wc);
        wc.cbWndExtra    = sizeof(dtk_window*);
        wc.lpfnWndProc   = (WNDPROC)dtk_GenericWindowProc;
        wc.lpszClassName = DTK_WIN32_WINDOW_CLASS;
        wc.hCursor       = LoadCursorA(NULL, MAKEINTRESOURCEA(32512));
        wc.hIcon         = LoadIconA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(101));
        wc.style         = CS_OWNDC | CS_DBLCLKS;
        if (!RegisterClassExA(&wc)) {
            UnregisterClassA(DTK_WIN32_WINDOW_CLASS, NULL);
            return DTK_ERROR;
        }
    }

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

    g_dtkInitCounter_Win32 -= 1;
    if (g_dtkInitCounter_Win32 == 0) {
        UnregisterClassA(DTK_WIN32_WINDOW_CLASS, GetModuleHandleA(NULL));
        OleUninitialize();
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
    }

    // Handle accelerator keys. If an accelerator key is processed with TranslateAccelerator() we do _not_ want to handle
    // the event with TranslateMessage/DispatchMessage.
    WNDPROC wndproc = (WNDPROC)GetWindowLongPtrA(msg.hwnd, GWLP_WNDPROC);
    if (wndproc == dtk_GenericWindowProc) {
        dtk_window* pWindow = (dtk_window*)GetWindowLongPtrA(msg.hwnd, 0);
        if (pWindow != NULL) {
            if (TranslateAcceleratorA((HWND)pWindow->win32.hWnd, pWindow->win32.hAccel, &msg)) {
                return DTK_SUCCESS;
            }
        }
    }


    TranslateMessage(&msg);
    DispatchMessageA(&msg);

    return DTK_SUCCESS;
}

dtk_result dtk_post_quit_event__win32(dtk_context* pTK, int exitCode)
{
    (void)pTK;

    PostQuitMessage(exitCode);
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

dtk_result dtk_init(dtk_context* pTK, dtk_event_proc onEvent)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pTK);
    pTK->onEvent = onEvent;

    // Detect the platform first.
    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (result != DTK_SUCCESS) {
        result = dtk_init__win32(pTK);
    }
#endif
#if DTK_GTK
    if (result != DTK_SUCCESS) {
        result = dtk_init__gtk(pGTK);
    }
#endif

    return result;
}

dtk_result dtk_uninit(dtk_context* pTK)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;
    
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

dtk_result dtk_next_event(dtk_context* pTK, dtk_bool32 blocking)
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

    return result;
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



#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

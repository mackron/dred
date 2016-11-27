// Copyright (C) 2016 David Reid. See included LICENSE file.

// Show window constants.
#define DTK_HIDE            0
#define DTK_SHOW_NORMAL     1
#define DTK_SHOW_MAXIMIZED  2

#define DTK_WINDOW(p) ((dtk_window*)(p))
struct dtk_window
{
    dtk_control control;
    dtk_surface surface;    // A window always has it's own surface.

    union
    {
    #ifdef DTK_WIN32
        struct
        {
            /*HWND*/ dtk_handle hWnd;
            /*HACCEL*/ dtk_handle hAccel; // The Win32 accelerator table handle. This is deleted and re-created whenever a new accelerator table is bound.
        } win32;
    #endif
    #ifdef DTK_GTK
        struct
        {
            /*GtkWidget**/ dtk_ptr pWidget;
            /*GtkWidget**/ dtk_ptr pBox;
            /*GtkWidget**/ dtk_ptr pClientArea;
        } gtk;
    #endif
    #ifdef DTK_X11
        struct
        {
        } x11;
    #endif
    };
};

// Initializes a window control.
dtk_result dtk_window_init(dtk_context* pTK, dtk_control* pParent, const char* title, dtk_uint32 width, dtk_uint32 height, dtk_event_proc onEvent, dtk_window* pWindow);

// Uninitializes a window control.
dtk_result dtk_window_uninit(dtk_window* pWindow);

// Shows or hides a window.
//
// The <mode> parameter should be set to one of the following:
//   DTK_HIDE
//   DTK_SHOW_NORMAL
//   DTK_SHOW_MAXIMIZED
dtk_result dtk_window_show(dtk_window* pWindow, int mode);

// Hides the given window.
//
// This is equivalent to dtk_window_show(pWindow, DTK_HIDE).
DTK_INLINE dtk_result dtk_window_hide(dtk_window* pWindow) { return dtk_window_show(pWindow, DTK_HIDE); }


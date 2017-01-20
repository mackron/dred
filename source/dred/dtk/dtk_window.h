// Copyright (C) 2016 David Reid. See included LICENSE file.

// Show window constants.
#define DTK_HIDE                0
#define DTK_SHOW_NORMAL         1
#define DTK_SHOW_MAXIMIZED      2

#define DTK_WINDOW_FLAG_TOPLEVEL    (1 << 0)    // Set when the window is a top-level window.
#define DTK_WINDOW_FLAG_DIALOG      (1 << 1)    // Set when the window is a dialog.
#define DTK_WINDOW_FLAG_POPUP       (1 << 2)    // Set when the window is a popup.

typedef enum
{
    dtk_window_type_toplevel,
    dtk_window_type_dialog,
    dtk_window_type_popup
} dtk_window_type;

typedef enum
{
    dtk_system_cursor_type_none,
    dtk_system_cursor_type_default,
    dtk_system_cursor_type_arrow = dtk_system_cursor_type_default,
    dtk_system_cursor_type_text,
    dtk_system_cursor_type_cross,
    dtk_system_cursor_type_double_arrow_h,
    dtk_system_cursor_type_double_arrow_v,
} dtk_system_cursor_type;

#define DTK_WINDOW(p) ((dtk_window*)(p))
struct dtk_window
{
    dtk_control control;
    dtk_uint32 flags;
    dtk_window* pNextWindow;

    union
    {
    #ifdef DTK_WIN32
        struct
        {
            /*HWND*/ dtk_handle hWnd;
            /*HCURSOR*/ dtk_handle hCursor; // Used when the window receives WM_SETCURSOR
            dtk_bool32 isCursorOverClientArea;

            // The high-surrogate from a WM_CHAR message. This is used in order to build a surrogate pair from a couple of WM_CHAR messages. When
            // a WM_CHAR message is received when code point is not a high surrogate, this is set to 0.
            dtk_uint16 utf16HighSurrogate;
        } win32;
    #endif
    #ifdef DTK_GTK
        struct
        {
            /*GtkWidget**/ dtk_ptr pWidget;
            /*GtkWidget**/ dtk_ptr pBox;
            /*GtkWidget**/ dtk_ptr pClientArea;
            /*GdkCursor**/ dtk_ptr pCursor;
            dtk_menu* pMenu;
            int configureClientWidth;     // The size of the client area based on the last "configure" event. This is used to know whether or not the event is for a move or size.
            int configureClientHeight;    // ^
            dtk_bool32 isCursorOverClientArea;
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
dtk_result dtk_window_init(dtk_context* pTK, dtk_control* pParent, dtk_window_type type, const char* title, dtk_uint32 width, dtk_uint32 height, dtk_event_proc onEvent, dtk_window* pWindow);

// Uninitializes a window control.
dtk_result dtk_window_uninit(dtk_window* pWindow);

// Sets the title of a window.
dtk_result dtk_window_set_title(dtk_window* pWindow, const char* title);


// Sets the size of a window.
dtk_result dtk_window_set_size(dtk_window* pWindow, dtk_uint32 width, dtk_uint32 height);
dtk_result dtk_window_get_size(dtk_window* pWindow, dtk_uint32* pWidth, dtk_uint32* pHeight);

// Retrieves the size of the client are of a window. This does not include the size of the menu.
dtk_result dtk_window_get_client_size(dtk_window* pWindow, dtk_uint32* pWidth, dtk_uint32* pHeight);

// Sets the absolute position of a window. This will position it relative to the screen.
dtk_result dtk_window_set_absolute_position(dtk_window* pWindow, dtk_int32 posX, dtk_int32 posY);
dtk_result dtk_window_get_absolute_position(dtk_window* pWindow, dtk_int32* pPosX, dtk_int32* pPosY);

// Sets the position of a window.
dtk_result dtk_window_set_relative_position(dtk_window* pWindow, dtk_int32 posX, dtk_int32 posY);
dtk_result dtk_window_get_relative_position(dtk_window* pWindow, dtk_int32* pPosX, dtk_int32* pPosY);

// Retrieves the absolute position of teh client area of the window.
dtk_result dtk_window_get_client_absolute_position(dtk_window* pWindow, dtk_int32* pPosX, dtk_int32* pPosY);

// Center's a window onto it's parent control. If it does not have a parent it'll be centered on the monitor.
dtk_result dtk_window_move_to_center(dtk_window* pWindow);


// Retrieves the rectangle of the client area of the given window.
//
// The left and top coordinates will always be set to 0, and the right and bottom coordinates will be set to
// the width and height respectively.
dtk_rect dtk_window_get_client_rect(dtk_window* pWindow);


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

// Brings the given window to the top of the z order and activates it.
dtk_result dtk_window_bring_to_top(dtk_window* pWindow);

// Determines whether or not the window is maximized.
dtk_bool32 dtk_window_is_maximized(dtk_window* pWindow);


// Sets the cursor to use for the given window.
dtk_result dtk_window_set_cursor(dtk_window* pWindow, dtk_system_cursor_type cursor);

// Determines whether or not the cursor is currently sitting on top of the given window's client area.
dtk_bool32 dtk_window_is_cursor_over(dtk_window* pWindow);

// Sets the menu for a window. Pass null to remove the menu.
dtk_result dtk_window_set_menu(dtk_window* pWindow, dtk_menu* pMenu);

// Shows a popup menu on the given window.
dtk_result dtk_window_show_popup_menu(dtk_window* pWindow, dtk_menu* pMenu, int posX, int posY);


// Requests a redraw of the given control.
//
// If the redrawing is currently being batched, the control will only be redrawn when the batching has
// completed.
dtk_result dtk_window_redraw(dtk_window* pWindow, dtk_rect rect);
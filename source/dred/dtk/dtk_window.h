// Copyright (C) 2017 David Reid. See included LICENSE file.

// Show window constants.
#define DTK_HIDE                0
#define DTK_SHOW_NORMAL         1
#define DTK_SHOW_MAXIMIZED      2

typedef enum
{
    dtk_window_type_toplevel,
    dtk_window_type_dialog,
    dtk_window_type_popup
} dtk_window_type;

#define DTK_WINDOW(p) ((dtk_window*)(p))
struct dtk_window
{
    dtk_control control;
    dtk_window* pNextWindow;
    dtk_control* pLastDescendantWithKeyboardFocus;
    dtk_control* pLastDescendantWithMouseCapture;
    dtk_bool32 isTopLevel : 1;
    dtk_bool32 isDialog   : 1;
    dtk_bool32 isPopup    : 1;
    dtk_bool32 isNextKeyboardCaptureExplicit : 1;   // Set to true when the keyboard capture is explicitly set.
    dtk_bool32 isNextKeyboardReleaseExplicit : 1;
    dtk_bool32 isNextMouseCaptureExplicit    : 1;
    dtk_bool32 isNextMouseReleaseExplicit    : 1;
    dtk_monitor monitor;                            // The monitor the window is currently sitting on. Used for DPI scaling. Initially set in dtk_window_init(), and then update whenever the window moves.
    float dpiScale;                                 // The cached DPI scale. This is used in painting routines so it's important it's available for fast access.

    union
    {
    #ifdef DTK_WIN32
        struct
        {
            /*HWND*/ dtk_handle hWnd;
            /*HCURSOR*/ dtk_handle hCursor; // Used when the window receives WM_SETCURSOR
            dtk_bool32 isCursorOverClientArea;

            // The high-surrogate from a WM_CHAR message. This is used to build a surrogate pair from a couple of WM_CHAR messages. When
            // a WM_CHAR message is received where the code point is not a high surrogate, this is set to 0.
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
            int configureClientWidth;       // The size of the client area based on the last "configure" event. This is used to know whether or not the event is for a move or size.
            int configureClientHeight;      // ^
            dtk_int32 desiredPositionX;     // Used when a window want's to move while invisible. When the window is made visible, it will be positioned based on this if repositionOnShow is set.
            dtk_int32 desiredPositionY; 
            dtk_bool32 isCursorOverClientArea : 1;
            dtk_bool32 repositionOnShow : 1;
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

// The default event handler for windows.
dtk_bool32 dtk_window_default_event_handler(dtk_event* pEvent);


// Sets the title of a window.
dtk_result dtk_window_set_title(dtk_window* pWindow, const char* title);


// Sets the size of a window.
dtk_result dtk_window_set_size(dtk_window* pWindow, dtk_uint32 width, dtk_uint32 height);
dtk_result dtk_window_get_size(dtk_window* pWindow, dtk_uint32* pWidth, dtk_uint32* pHeight);

// Retrieves the size of the client are of a window. This does not include the size of the menu.
dtk_result dtk_window_get_client_size(dtk_window* pWindow, dtk_uint32* pWidth, dtk_uint32* pHeight);

// Sets the absolute position of a window. This will position it relative to the screen.
dtk_result dtk_window_set_absolute_position(dtk_window* pWindow, dtk_int32 screenPosX, dtk_int32 screenPosY);
dtk_result dtk_window_get_absolute_position(dtk_window* pWindow, dtk_int32* pScreenPosX, dtk_int32* pScreenPosY);

// Sets the position of a window.
dtk_result dtk_window_set_relative_position(dtk_window* pWindow, dtk_int32 posX, dtk_int32 posY);
dtk_result dtk_window_get_relative_position(dtk_window* pWindow, dtk_int32* pPosX, dtk_int32* pPosY);

// Retrieves the absolute position of the client area of the window.
dtk_result dtk_window_get_client_absolute_position(dtk_window* pWindow, dtk_int32* pScreenPosX, dtk_int32* pScreenPosY);

// Center's a window onto it's parent control. If it does not have a parent it'll be centered on the monitor.
dtk_result dtk_window_move_to_center(dtk_window* pWindow);

// Determines whether or not the given window is visible on a screen.
dtk_bool32 dtk_window_is_in_view(dtk_window* pWindow);

// Checks if the given window is visible on the screen, and if not, moves it onto the screen.
dtk_result dtk_window_move_into_view(dtk_window* pWindow);


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


// Schedules a redraw of the given window.
//
// This does not immediately redraw the window, but instead stores the redraw request in a queue for execution
// later via the main loop. The advantage of this over immediate redrawing is that it combines redraws into
// larger, more monolithic redraws which improves efficiency.
dtk_result dtk_window_scheduled_redraw(dtk_window* pWindow, dtk_rect rect);

// Immediately redraws a section of the given window.
dtk_result dtk_window_immediate_redraw(dtk_window* pWindow, dtk_rect rect);


// Finds the control sitting under the mouse, using a window as the root level control.
dtk_control* dtk_window_find_control_under_point(dtk_window* pWindow, dtk_int32 posX, dtk_int32 posY);

// Updates the state that determines which controls have changed mouse enter/leave states. The return value a
// pointer to the control that is sitting directly under the mouse.
dtk_control* dtk_window_refresh_mouse_enter_leave_state(dtk_window* pWindow, dtk_int32 mousePosX, dtk_int32 mousePosY);
// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_MESSAGE_BOX_YES     1
#define DRED_MESSAGE_BOX_NO      2
#define DRED_MESSAGE_BOX_CANCEL  3

// Initializes the platform layer. Should be the first function to be called.
dr_bool32 dred_platform_init(dtk_context* pTK);

// Uninitializes the platform layer. Should be the last function to be called.
void dred_platform_uninit();

// Runs the main application loop.
int dred_platform_run();

// Posts a quit message to main application loop to force it to break.
void dred_platform_post_quit_message(int resultCode);

// Binds the platform-specific global GUI event handlers.
void dred_platform_bind_gui(dred_gui* pGUI);

// Connects the platform-specific logging system to dred's logging system.
void dred_platform_bind_logging(dred_context* pDred);


//// Windows ////
typedef void (* dred_window_on_close_proc)             (dred_window* pWindow);
typedef dr_bool32 (* dred_window_on_hide_proc)         (dred_window* pWindow, unsigned int flags);
typedef dr_bool32 (* dred_window_on_show_proc)         (dred_window* pWindow);
typedef void (* dred_window_on_activate_proc)          (dred_window* pWindow);
typedef void (* dred_window_on_deactivate_proc)        (dred_window* pWindow);
typedef void (* dred_window_on_size_proc)              (dred_window* pWindow, unsigned int newWidth, unsigned int newHeight);
typedef void (* dred_window_on_move_proc)              (dred_window* pWindow, int newPosX, int newPosY);
typedef void (* dred_window_on_mouse_enter_proc)       (dred_window* pWindow);
typedef void (* dred_window_on_mouse_leave_proc)       (dred_window* pWindow);
typedef void (* dred_window_on_mouse_move_proc)        (dred_window* pWindow, int mousePosX, int mousePosY, unsigned int stateFlags);
typedef void (* dred_window_on_mouse_button_proc)      (dred_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY, unsigned int stateFlags);
typedef void (* dred_window_on_mouse_wheel_proc)       (dred_window* pWindow, int delta, int relativeMousePosX, int relativeMousePosY, unsigned int stateFlags);
typedef void (* dred_window_on_key_down_proc)          (dred_window* pWindow, dtk_key key, unsigned int stateFlags);
typedef void (* dred_window_on_key_up_proc)            (dred_window* pWindow, dtk_key key, unsigned int stateFlags);
typedef void (* dred_window_on_printable_key_down_proc)(dred_window* pWindow, unsigned int character, unsigned int stateFlags);
typedef void (* dred_window_on_focus_proc)             (dred_window* pWindow);
typedef void (* dred_window_on_unfocus_proc)           (dred_window* pWindow);
typedef void (* dred_window_on_ipc_message_proc)       (dred_window* pWindow, unsigned int messageID, const void* pMessageData);


struct dred_window
{
    dtk_window windowDTK;

    // The main context that owns the window.
    dred_context* pDred;

    // The current menu bar of the window.
    dtk_menu* pMenu;

    // The window's top level GUI element.
    dred_control rootGUIControl;
    dred_control* pRootGUIControl;  // <-- Always set to &rootGUIControl, and exists only for convenience.

    // Event handlers.
    dred_window_on_close_proc onClose;
    dred_window_on_hide_proc onHide;
    dred_window_on_show_proc onShow;
    dred_window_on_activate_proc onActivate;
    dred_window_on_deactivate_proc onDeactivate;
    dred_window_on_size_proc onSize;
    dred_window_on_move_proc onMove;
    dred_window_on_mouse_enter_proc onMouseEnter;
    dred_window_on_mouse_leave_proc onMouseLeave;
    dred_window_on_mouse_move_proc onMouseMove;
    dred_window_on_mouse_button_proc onMouseButtonDown;
    dred_window_on_mouse_button_proc onMouseButtonUp;
    dred_window_on_mouse_button_proc onMouseButtonDblClick;
    dred_window_on_mouse_wheel_proc onMouseWheel;
    dred_window_on_key_down_proc onKeyDown;
    dred_window_on_key_up_proc onKeyUp;
    dred_window_on_printable_key_down_proc onPrintableKeyDown;
    dred_window_on_focus_proc onFocus;
    dred_window_on_unfocus_proc onUnfocus;
    dred_window_on_ipc_message_proc onIPCMessage;

    // A pointer to the GUI element that belongs to this window that should be given the keyboard capture when this window
    // receives focus.
    dtk_control* pControlWithKeyboardCapture;

    // Whether or not the menu is being shown.
    dr_bool32 isShowingMenu;

    // External user data.
    void* pUserData;
};

// Creates a top-level window.
dred_window* dred_window_create(dred_context* pDred);

// Creates a dialog window.
dred_window* dred_window_create_dialog(dred_window* pParentWindow, const char* title, unsigned int width, unsigned int height);

// Creates a popup window.
dred_window* dred_window_create_popup(dred_window* pParentWindow, unsigned int width, unsigned int height);

// Deletes a window.
void dred_window_delete(dred_window* pWindow);

// Sets the title of the given window.
void dred_window_set_title(dred_window* pWindow, const char* title);

// Sets the size of the window.
void dred_window_set_size(dred_window* pWindow, unsigned int newWidth, unsigned int newHeight);
void dred_window_get_size(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut);

// Sets the position of the window.
void dred_window_set_position(dred_window* pWindow, int posX, int posY);
void dred_window_get_position(dred_window* pWindow, int* pPosXOut, int* pPosYOut);

// Retrieves the size of the client area of the given window. This does not include the size of the menu.
void dred_window_get_client_size(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut);
void dred_window_get_client_position(dred_window* pWindow, int* pPosXOut, int* pPosYOut);

// Center's the given window. If the window has a parent it'll be centered on the parent, otherwise it'll be centered on the monitor.
void dred_window_move_to_center(dred_window* pWindow);

// Show/hide the window.
void dred_window_show(dred_window* pWindow);
void dred_window_show_maximized(dred_window* pWindow);
void dred_window_show_sized(dred_window* pWindow, unsigned int width, unsigned int height);
void dred_window_hide(dred_window* pWindow);

// Brings the given window to the top of the z order and activates it.
void dred_window_bring_to_top(dred_window* pWindow);

// Determines whether or not the window is maximized.
dr_bool32 dred_window_is_maximized(dred_window* pWindow);

// Sets the cursor to use with the window.
void dred_window_set_cursor(dred_window* pWindow, dtk_system_cursor_type cursor);
dr_bool32 dred_window_is_cursor_over(dred_window* pWindow);

// Sets the menu for the given window. Can be null, in which case the menu is removed.
void dred_window_set_menu(dred_window* pWindow, dtk_menu* pMenu);

// Hides the menu for the given window.
void dred_window_hide_menu(dred_window* pWindow);

// Shows the menu for the given window.
void dred_window_show_menu(dred_window* pWindow);

// Determines whether or not the menu is being shown for the given window.
dr_bool32 dred_window_is_showing_menu(dred_window* pWindow);

// Shows a popup menu on the given window.
void dred_window_show_popup_menu(dred_window* pWindow, dtk_menu* pMenu, int posX, int posY);

// Sends an IPC message to the event queue for the given window.
void dred_window_send_ipc_message_event(dred_window* pWindow, unsigned int messageID, const void* pMessageData, size_t messageDataSize);


// Event posting.
void dred_window_on_close(dred_window* pWindow);
dr_bool32 dred_window_on_hide(dred_window* pWindow, unsigned int flags);
dr_bool32 dred_window_on_show(dred_window* pWindow);
void dred_window_on_activate(dred_window* pWindow);
void dred_window_on_deactivate(dred_window* pWindow);
void dred_window_on_size(dred_window* pWindow, unsigned int newWidth, unsigned int newHeight);
void dred_window_on_move(dred_window* pWindow, int newPosX, int newPosY);
void dred_window_on_mouse_enter(dred_window* pWindow);
void dred_window_on_mouse_leave(dred_window* pWindow);
void dred_window_on_mouse_move(dred_window* pWindow, int mousePosX, int mousePosY, unsigned int stateFlags);
void dred_window_on_mouse_button_down(dred_window* pWindow, int mouseButton, int mousePosX, int mousePosY, unsigned int stateFlags);
void dred_window_on_mouse_button_up(dred_window* pWindow, int mouseButton, int mousePosX, int mousePosY, unsigned int stateFlags);
void dred_window_on_mouse_button_dblclick(dred_window* pWindow, int mouseButton, int mousePosX, int mousePosY, unsigned int stateFlags);
void dred_window_on_mouse_wheel(dred_window* pWindow, int delta, int mousePosX, int mousePosY, unsigned int stateFlags);
void dred_window_on_key_down(dred_window* pWindow, dtk_key key, unsigned int stateFlags);
void dred_window_on_key_up(dred_window* pWindow, dtk_key key, unsigned int stateFlags);
void dred_window_on_printable_key_down(dred_window* pWindow, unsigned int character, unsigned int stateFlags);
void dred_window_on_focus(dred_window* pWindow);
void dred_window_on_unfocus(dred_window* pWindow);
void dred_window_on_ipc_message(dred_window* pWindow, unsigned int messageID, const void* pMessageData);



// Stock window events.

// Hides the window when the close button is pressed.
void dred_window__stock_event__hide_on_close(dred_window* pWindow);


// Helper function for retrieving the window that owns the given GUI element.
dred_window* dred_get_control_window(dred_control* pControl);



//// Drag and Drop ////

typedef enum
{
    dred_data_type_generic,
    dred_data_type_text
} dred_data_type;

// Begins a drag and drop operation.
//
// This will make a copy of the data.
dr_bool32 dred_begin_drag_and_drop(dred_data_type dataType, const void* pData, size_t dataSize);

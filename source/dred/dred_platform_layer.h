// Copyright (C) 2016 David Reid. See included LICENSE file.


// This flag is posted on on_hide events when a popup window is automatically hidden as a result of the user clicking
// outside of it's region.
#define DRED_AUTO_HIDE_FROM_LOST_FOCUS        (1 << 0)
#define DRED_HIDE_BLOCKED                     (1 << 1)

// The flags below are posted on on key down and up events.
#define DRED_MOUSE_BUTTON_LEFT_DOWN   (1 << 0)
#define DRED_MOUSE_BUTTON_RIGHT_DOWN  (1 << 1)
#define DRED_MOUSE_BUTTON_MIDDLE_DOWN (1 << 2)
#define DRED_MOUSE_BUTTON_4_DOWN      (1 << 3)
#define DRED_MOUSE_BUTTON_5_DOWN      (1 << 4)
#define DRED_KEY_STATE_SHIFT_DOWN     (1 << 5)        // Whether or not a shift key is down at the time the input event is handled.
#define DRED_KEY_STATE_CTRL_DOWN      (1 << 6)        // Whether or not a ctrl key is down at the time the input event is handled.
#define DRED_KEY_STATE_ALT_DOWN       (1 << 7)        // Whether or not an alt key is down at the time the input event is handled.
#define DRED_KEY_STATE_AUTO_REPEATED  (1 << 31)       // Whether or not the key press is generated due to auto-repeating. Only used with key down events.

#define DRED_MESSAGE_BOX_YES     1
#define DRED_MESSAGE_BOX_NO      2
#define DRED_MESSAGE_BOX_CANCEL  3

typedef enum
{
    dred_menu_type_popup,
    dred_menu_type_menubar
} dred_menu_type;


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
typedef void (* dred_window_on_key_down_proc)          (dred_window* pWindow, dred_key key, unsigned int stateFlags);
typedef void (* dred_window_on_key_up_proc)            (dred_window* pWindow, dred_key key, unsigned int stateFlags);
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

    // The surface we'll be drawing to when drawing the GUI.
    dr2d_surface* pDrawingSurface;


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
    dred_control* pControlWithKeyboardCapture;

    // Whether or not the menu is being shown.
    dr_bool32 isShowingMenu;

    // External user data.
    void* pUserData;

#if 0
    // Platform specific.
#ifdef _WIN32
    // The Win32 window handle.
    HWND hWnd;

    // The Win32 accelerator table handle. This is deleted and re-created whenever a new accelerator table is bound.
    HACCEL hAccel;

    // The current cursor of the window.
    HCURSOR hCursor;

    // Keeps track of whether or not the cursor is over this window.
    dr_bool32 isCursorOver;

    // The high-surrogate from a WM_CHAR message. This is used in order to build a surrogate pair from a couple of WM_CHAR messages. When
    // a WM_CHAR message is received when code point is not a high surrogate, this is set to 0.
    unsigned short utf16HighSurrogate;
#else
    // The GTK window.
    GtkWidget* pGTKWindow;
    GtkWidget* pGTKBox;
    GtkWidget* pGTKClientArea;

    // The GTK accelerator group tied to this window.
    GtkAccelGroup* pGTKAccelGroup;
    dred_gtk_accelerator* pAccels;
    size_t accelCount;

    // The cursor to use with this window.
    GdkCursor* pGTKCursor;

    // Keeps track of whether or not the cursor is over this window.
    dr_bool32 isCursorOver;

    // The position of the inner section of the window. This is set in the configure event handler.
    int absoluteClientPosX;
    int absoluteClientPosY;

    // The position of the window. This is set by the configure event handler, and is used to detect whether or not the window has
    // moved and thus need to have the on_move event posted.
    int windowPosX;
    int windowPosY;
#endif
#endif
};

// Creates a top-level window.
dred_window* dred_window_create(dred_context* pDred);

// Creates a dialog window.
dred_window* dred_window_create_dialog(dred_window* pParentWindow, const char* title, unsigned int width, unsigned int height);

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

// Finds the menu item with the given id. The search is recursive.
//dred_menu_item* dred_window_find_menu_item_by_id(dred_window* pWindow, uint16_t id);

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
void dred_window_on_key_down(dred_window* pWindow, dred_key key, unsigned int stateFlags);
void dred_window_on_key_up(dred_window* pWindow, dred_key key, unsigned int stateFlags);
void dred_window_on_printable_key_down(dred_window* pWindow, unsigned int character, unsigned int stateFlags);
void dred_window_on_focus(dred_window* pWindow);
void dred_window_on_unfocus(dred_window* pWindow);
void dred_window_on_ipc_message(dred_window* pWindow, unsigned int messageID, const void* pMessageData);



// Stock window events.

// Hides the window when the close button is pressed.
void dred_window__stock_event__hide_on_close(dred_window* pWindow);


// Helper function for retrieving the window that owns the given GUI element.
dred_window* dred_get_control_window(dred_control* pControl);




#if 0
//// MENUS ////
#define DRED_MENU_ITEM_SEPARATOR    (1 << 0)
#define DRED_MENU_ITEM_CHECK        (1 << 1)

struct dred_menu_item
{
    // The ID of the menu item. Due to the way Win32 notifies the application of the menu being pressed this is restricted to 16 bits.
    uint16_t id;

    // The index of the menu item.
    size_t index;

    // The command associated with the menu item, it any.
    char* command;

    // The sub-menu, if any.
    dred_menu* pSubMenu;

    // The menu that owns the this item.
    dred_menu* pOwnerMenu;

#ifdef DRED_GTK
    // The GtkMenuItem object.
    GtkWidget* pGTKMenuItem;

    // Menu items in GTK need access to the main context in order to handle the command.
    dred_context* pDred;

    // The options flags that were passed into the creation function for this menu item.
    unsigned int flags;

    // When a check menu item is checked with gtk_check_menu_item_set_active() it results in the "activate" event getting
    // fired which is inconsistent with Win32. This variable is used to block events in these situations.
    dr_bool32 blockNextActivateSignal;
#endif
};

struct dred_menu
{
    // The main context that owns the menu.
    dred_context* pDred;

    // The menu type.
    //   dred_menu_type_popup (Use this for any kind of menu that's not a menu bar.)
    //   dred_menu_type_menubar
    dred_menu_type type;

    // The number of menu items attached to the menu.
    size_t menuItemCount;

    // The list of menu items in the menu.
    dred_menu_item** ppMenuItems;

#ifdef DRED_WIN32
    // A handle to the menu.
    HMENU hMenu;
#endif

#ifdef DRED_GTK
    // The GtkMenuBar or GtkMenu object, depending on the menu type.
    GtkWidget* pGTKMenu;

    // The GTK accelerator group tied to this window.
    GtkAccelGroup* pGTKAccelGroup;
    dred_gtk_accelerator* pAccels;
    size_t accelCount;
#endif
};

// Creates a menu of the given type.
dred_menu* dred_menu_create(dred_context* pDred, dred_menu_type type);

// Deletes the given menu.
void dred_menu_delete(dred_menu* pMenu);

// Recursively searches for the first menu item with the given ID.
dred_menu_item* dred_menu_find_menu_item_by_id(dred_menu* pMenu, uint16_t id);


// Creates a menu item.
dred_menu_item* dred_menu_item_create_and_append(dred_menu* pMenu, const char* text, uint16_t id, const char* command, dred_shortcut shortcut, unsigned int options, dred_menu* pSubMenu);

// Helper for creating a menu item that's tied to a shortcut.
dred_menu_item* dred_menu_item_create_and_append_with_shortcut(dred_menu* pMenu, const char* text, uint16_t id, const char* shortcutName, unsigned int options);

// Creates an appends a separator.
dred_menu_item* dred_menu_item_create_and_append_separator(dred_menu* pMenu);

// Deletes the given menu item.
void dred_menu_item_delete(dred_menu_item* pItem);

// Deletes ever menu item in the given menu.
void dred_menu_delete_all_items(dred_menu* pMenu);


// Enables the given menu item.
void dred_menu_item_enable(dred_menu_item* pItem);

// Disables the given menu item.
void dred_menu_item_disable(dred_menu_item* pItem);


// Checks the given menu item.
void dred_menu_item_check(dred_menu_item* pItem);

// Unchecks the given menu item.
void dred_menu_item_uncheck(dred_menu_item* pItem);

// Sets whether or not the menu item is checked.
void dred_menu_item_set_checked(dred_menu_item* pItem, dr_bool32 checked);

// Determines whether or not the given menu item is checked.
dr_bool32 dred_menu_item_is_checked(dred_menu_item* pItem);
#endif


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

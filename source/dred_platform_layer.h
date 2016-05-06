

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



// Initializes the platform layer. Should be the first function to be called.
bool dred_platform_init();

// Uninitializes the platform layer. Should be the last function to be called.
void dred_platform_uninit();

// Runs the main application loop.
int dred_platform_run();

// Posts a quit message to main application loop to force it to break.
void dred_platform_post_quit_message(int resultCode);

// Binds the platform-specific global GUI event handlers.
void dred_platform_bind_gui_event_handlers(drgui_context* pGUI);


//// Windows ////
typedef void (* dred_window_on_close_proc)             (dred_window* pWindow);
typedef bool (* dred_window_on_hide_proc)              (dred_window* pWindow, unsigned int flags);
typedef bool (* dred_window_on_show_proc)              (dred_window* pWindow);

typedef enum
{
    dred_cursor_type_none,
    dred_cursor_type_default,

    dred_cursor_type_arrow = dred_cursor_type_default,
    dred_cursor_type_text,
    dred_cursor_type_cross,
    dred_cursor_type_size_ns,           // North/South resize arrows.
    dred_cursor_type_size_we,           // West/East resize arrows.
    dred_cursor_type_size_nesw,         // North/East, South/West resize arrows.
    dred_cursor_type_size_nwse          // North/West, South/East resize arrows.
} dred_cursor_type;

struct dred_window
{
    // Event handlers.
    dred_window_on_close_proc onClose;
    dred_window_on_hide_proc onHide;
    dred_window_on_show_proc onShow;

    // A pointer to the GUI element that belongs to this window that should be given the keyboard capture when this window
    // receives focus.
    drgui_element* pElementWithKeyboardCapture;

    // The flags to pass to the onHide event handler.
    unsigned int onHideFlags;


    // Platform specific.
#ifdef _WIN32
    // The Win32 window handle.
    HWND hWnd;

    // The current cursor of the window.
    HCURSOR hCursor;

    // Keeps track of whether or not the cursor is over this window.
    bool isCursorOver;

    // The high-surrogate from a WM_CHAR message. This is used in order to build a surrogate pair from a couple of WM_CHAR messages. When
    // a WM_CHAR message is received when code point is not a high surrogate, this is set to 0.
    unsigned short utf16HighSurrogate;
#endif

#ifdef __linux__
    // The GTK window.
    GtkWidget* pGTKWindow;

    // The cursor to use with this window.
    GdkCursor* pGTKCursor;

    // Keeps track of whether or not the cursor is over this window.
    bool isCursorOver;
#endif
};

// Creates a top-level window.
dred_window* dred_window_create();

// Deletes a window.
void dred_window_delete(dred_window* pWindow);

// Sets the size of the window.
void dred_window_set_size(dred_window* pWindow, unsigned int newWidth, unsigned int newHeight);
void dred_window_get_size(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut);

// Show/hide the window.
void dred_window_show(dred_window* pWindow);
void dred_window_show_maximized(dred_window* pWindow);
void dred_window_show_sized(dred_window* pWindow, unsigned int width, unsigned int height);
void dred_window_hide(dred_window* pWindow, unsigned int flags);

// Sets the cursor to use with the window.
void dred_window_set_cursor(dred_window* pWindow, dred_cursor_type cursor);
bool dred_window_is_cursor_over(dred_window* pWindow);

// Helper function for retrieving the window that owns the given GUI element.
dred_window* dred_get_element_window(drgui_element* pElement);
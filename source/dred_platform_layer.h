

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


//// Windows ////
typedef void (* dred_window_on_close_proc)             (dred_window* pWindow);
typedef bool (* dred_window_on_hide_proc)              (dred_window* pWindow, unsigned int flags);
typedef bool (* dred_window_on_show_proc)              (dred_window* pWindow);

struct dred_window
{
    // Event handlers.
    dred_window_on_close_proc onClose;
    dred_window_on_hide_proc onHide;
    dred_window_on_show_proc onShow;


    // The flags to pass to the onHide event handler.
    unsigned int onHideFlags;


    // Platform specific.
#ifdef _WIN32
    // The Win32 window handle.
    HWND hWnd;
#endif

#ifdef __linux__
    // The GTK window.
    GtkWidget* pGTKWindow;
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

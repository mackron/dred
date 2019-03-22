// Copyright (C) 2019 David Reid. See included LICENSE file.

#ifndef DTK_H
#define DTK_H

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4201)   // nonstandard extension used: nameless struct/union
#endif

// Platform/backend detection.
#ifdef _WIN32
    #define DTK_WIN32
    #if (!defined(WINAPI_FAMILY) || WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
        #define DTK_WIN32_DESKTOP
    #endif
#else
    #define DTK_POSIX
    #ifdef __linux__
        #define DTK_LINUX
    #endif

    #include <pthread.h>
    #include <semaphore.h>
#endif

#ifdef DTK_LINUX
    #define DTK_GTK
    //#define DTK_X11
#endif

typedef enum
{
    dtk_platform_win32,
    dtk_platform_gtk,
    //dtk_platform_x11  // Not yet.
} dtk_platform;

// Sized types.
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef   signed char    dtk_int8;
typedef unsigned char    dtk_uint8;
typedef   signed short   dtk_int16;
typedef unsigned short   dtk_uint16;
typedef   signed int     dtk_int32;
typedef unsigned int     dtk_uint32;
typedef   signed __int64 dtk_int64;
typedef unsigned __int64 dtk_uint64;
#else
#include <stdint.h>
typedef int8_t           dtk_int8;
typedef uint8_t          dtk_uint8;
typedef int16_t          dtk_int16;
typedef uint16_t         dtk_uint16;
typedef int32_t          dtk_int32;
typedef uint32_t         dtk_uint32;
typedef int64_t          dtk_int64;
typedef uint64_t         dtk_uint64;
#endif
typedef dtk_uint8        dtk_bool8;
typedef dtk_uint32       dtk_bool32;
#define DTK_TRUE         1
#define DTK_FALSE        0

typedef void* dtk_handle;
typedef void* dtk_ptr;
typedef void (* dtk_proc)();

// Result codes.
typedef int dtk_result;
#define DTK_SUCCESS                  0
#define DTK_NO_EVENT                 1      // Not an error. Returned by dtk_next_event() to indicate there are no events currently in the queue.
#define DTK_ERROR                   -1
#define DTK_INVALID_ARGS            -2
#define DTK_INVALID_OPERATION       -3
#define DTK_OUT_OF_MEMORY           -4
#define DTK_NO_BACKEND              -5
#define DTK_FAILED_TO_INIT_BACKEND  -6
#define DTK_OUT_OF_RANGE            -7
#define DTK_CANCELLED               -8
#define DTK_FAILED_TO_OPEN_DIALOG   -9
#define DTK_FAILED_TO_OPEN_FILE     -10
#define DTK_FAILED_TO_READ_FILE     -11
#define DTK_FAILED_TO_WRITE_FILE    -12
#define DTK_FILE_TOO_BIG            -13
#define DTK_PATH_TOO_LONG           -14
#define DTK_NAME_TOO_LONG           -15
#define DTK_DOES_NOT_EXIST          -16
#define DTK_ALREADY_EXISTS          -17
#define DTK_ACCESS_DENIED           -18
#define DTK_TOO_MANY_OPEN_FILES     -19
#define DTK_END_OF_FILE             -20
#define DTK_NO_SPACE                -21
#define DTK_NEGATIVE_SEEK           -22
#define DTK_TIMEOUT                 -23
#define DTK_MEMORY_ALREADY_MAPPED   -24
#define DTK_NO_DISPLAY              -256
#define DTK_NO_SCREEN               -257
#define DTK_QUIT                    -1024   // Returned by dtk_next_event() when a quit message is received.

// Standard library stuff.
#include <string.h>
#include <stdlib.h>
#include <stdio.h>  // For FILE
#include <stdarg.h>
#include <errno.h>
#include <time.h>

#ifndef dtk_assert
#define dtk_assert(condition)               assert(condition)
#endif
#ifndef dtk_malloc
#define dtk_malloc(sz)                      malloc(sz);
#endif
#ifndef dtk_calloc
#define dtk_calloc(c, sz)                   calloc((c), (sz))
#endif
#ifndef dtk_realloc
#define dtk_realloc(p, sz)                  realloc((p), (sz))
#endif
#ifndef dtk_free
#define dtk_free(p)                         free(p)
#endif
#ifndef dtk_copy_memory
#define dtk_copy_memory(dst, src, sz)       memcpy((dst), (src), (sz))
#endif
#ifndef dtk_zero_memory
#define dtk_zero_memory(p, sz)              memset((p), 0, (sz))
#endif
#define dtk_zero_object(p)                  dtk_zero_memory((p), sizeof(*(p)))

#define dtk_count_of(obj)                   (sizeof(obj) / sizeof(obj[0]))
#define dtk_offset_ptr(p, offset)           (((dtk_uint8*)(p)) + (offset))
#define dtk_min(x, y)                       (((x) < (y)) ? (x) : (y))
#define dtk_max(x, y)                       (((x) > (y)) ? (x) : (y))
#define dtk_clamp(x, lo, hi)                (((x) < (lo)) ? (lo) : (((x) > (hi)) ? (hi) : (x)))
#define dtk_round_up(x, multiple)           ((((x) + ((multiple) - 1)) / (multiple)) * (multiple))
#define dtk_round_up_signed(x, multiple)    ((((x) + (((x) >= 0)*((multiple) - 1))) / (multiple)) * (multiple))

#if defined(_MSC_VER)
#define DTK_INLINE static __inline
#else
#define DTK_INLINE static inline
#endif

typedef struct dtk_context dtk_context;
typedef struct dtk_event dtk_event;
typedef struct dtk_control dtk_control;
typedef struct dtk_label dtk_label;
typedef struct dtk_button dtk_button;
typedef struct dtk_checkbox dtk_checkbox;
typedef struct dtk_color_button dtk_color_button;
typedef struct dtk_tabbar dtk_tabbar;
typedef struct dtk_tabgroup dtk_tabgroup;
typedef struct dtk_scrollbar dtk_scrollbar;
typedef struct dtk_textbox dtk_textbox;
typedef struct dtk_window dtk_window;
typedef struct dtk_menu dtk_menu;
typedef struct dtk_timer dtk_timer;

typedef enum
{
    dtk_system_cursor_type_none,
    dtk_system_cursor_type_default,
    dtk_system_cursor_type_arrow = dtk_system_cursor_type_default,
    dtk_system_cursor_type_hand,
    dtk_system_cursor_type_text,
    dtk_system_cursor_type_cross,
    dtk_system_cursor_type_double_arrow_h,
    dtk_system_cursor_type_double_arrow_v,
} dtk_system_cursor_type;

typedef enum
{
    dtk_application_font_type_ui,
    dtk_application_font_type_monospace,
    dtk_application_font_type_default = dtk_application_font_type_ui
} dtk_application_font_type;

// The callback function for handing log messages.
typedef void (* dtk_log_proc)(dtk_context* pTK, const char* message);

// The callback function to call when an event is received and needs handling. The return value controls whether or
// not the event should be propagated. Returning true will propagate the event, false will cancel the event. You will
// almost always want to return true.
typedef dtk_bool32 (* dtk_event_proc)(dtk_event* pEvent);

#include "nanosvg.h"
#include "nanosvgrast.h"
#include "stb_image_resize.h"

#include "dtk_dl.h"
#include "dtk_rect.h"
#include "dtk_string.h"
#include "dtk_path.h"
#include "dtk_time.h"
#include "dtk_io.h"
#include "dtk_threading.h"
#include "dtk_ipc.h"
#include "dtk_math.h"
#include "dtk_monitor.h"
#include "dtk_svg.h"
#include "dtk_graphics.h"
#include "dtk_image.h"
#include "dtk_stock_images.h"
#include "dtk_input.h"
#include "dtk_accelerators.h"
#include "dtk_control.h"
#include "dtk_label.h"
#include "dtk_button.h"
#include "dtk_checkbox.h"
#include "dtk_color_button.h"
#include "dtk_tabbar.h"
#include "dtk_tabgroup.h"
#include "dtk_scrollbar.h"
#include "dtk_textbox.h"
#include "dtk_window.h"
#include "dtk_menu.h"
#include "dtk_dialogs.h"
#include "dtk_timer.h"
#include "dtk_clipboard.h"
#include "dtk_paint_queue.h"
#include "dtk_command_line.h"
#include "dtk_webgen.h"

// Event types.
typedef int dtk_event_type;
#define DTK_EVENT_NONE                                  0
#define DTK_EVENT_QUIT                                  1
#define DTK_EVENT_MENU                                  2
#define DTK_EVENT_ACCELERATOR                           3
#define DTK_EVENT_CLOSE                                 4
#define DTK_EVENT_PAINT                                 5
#define DTK_EVENT_SIZE                                  6
#define DTK_EVENT_MOVE                                  7
#define DTK_EVENT_SHOW                                  8
#define DTK_EVENT_HIDE                                  9
#define DTK_EVENT_MOUSE_LEAVE                           10
#define DTK_EVENT_MOUSE_ENTER                           11
#define DTK_EVENT_MOUSE_MOVE                            12
#define DTK_EVENT_MOUSE_BUTTON_DOWN                     13
#define DTK_EVENT_MOUSE_BUTTON_UP                       14
#define DTK_EVENT_MOUSE_BUTTON_DBLCLICK                 15
#define DTK_EVENT_MOUSE_WHEEL                           16
#define DTK_EVENT_KEY_DOWN                              17
#define DTK_EVENT_KEY_UP                                18
#define DTK_EVENT_PRINTABLE_KEY_DOWN                    19
#define DTK_EVENT_CAPTURE_KEYBOARD                      20
#define DTK_EVENT_RELEASE_KEYBOARD                      21
#define DTK_EVENT_CAPTURE_MOUSE                         22
#define DTK_EVENT_RELEASE_MOUSE                         23
#define DTK_EVENT_DPI_CHANGED                           24
#define DTK_EVENT_TOOLTIP                               25
#define DTK_EVENT_BUTTON_PRESSED                        128
#define DTK_EVENT_CHECKBOX_CHECK_CHANGED                129
#define DTK_EVENT_COLOR_BUTTON_COLOR_CHANGED            130
#define DTK_EVENT_SCROLLBAR_SCROLL                      132

#define DTK_EVENT_TABBAR_MOUSE_BUTTON_DOWN_TAB          133
#define DTK_EVENT_TABBAR_MOUSE_BUTTON_UP_TAB            134
#define DTK_EVENT_TABBAR_MOUSE_BUTTON_DBLCLICK_TAB      135
#define DTK_EVENT_TABBAR_CHANGE_TAB                     136
#define DTK_EVENT_TABBAR_CLOSE_TAB                      137
#define DTK_EVENT_TABBAR_PIN_TAB                        138
#define DTK_EVENT_TABBAR_UNPIN_TAB                      139
#define DTK_EVENT_TABBAR_REMOVE_TAB                     140
#define DTK_EVENT_TABGROUP_MOUSE_BUTTON_DOWN_TAB        141
#define DTK_EVENT_TABGROUP_MOUSE_BUTTON_UP_TAB          142
#define DTK_EVENT_TABGROUP_MOUSE_BUTTON_DBLCLICK_TAB    143
#define DTK_EVENT_TABGROUP_CHANGE_TAB                   144
#define DTK_EVENT_TABGROUP_CLOSE_TAB                    145
#define DTK_EVENT_TABGROUP_PIN_TAB                      146
#define DTK_EVENT_TABGROUP_UNPIN_TAB                    147

#define DTK_EVENT_REFRESH_LAYOUT                        251     // A special event that's used to indicate to a control that it needs to refresh the layout of it's children and inner elements.
#define DTK_EVENT_APPLICATION_FONT                      252     // A special event for retrieving an application-defined font for certain categories.
#define DTK_EVENT_APPLICATION_SCALE                     253     // A special event for retrieving an application-defined scaling factor for GUI elements.
#define DTK_EVENT_CHANGE_KEYBOARD_CAPTURE               254     // A special event used internally for handling keyboard capture changes.
#define DTK_EVENT_CHANGE_MOUSE_CAPTURE                  255     // ^^^
#define DTK_EVENT_CUSTOM                                256

struct dtk_event
{
    dtk_event_type type;
    dtk_context* pTK;
    dtk_control* pControl;

    union
    {
        struct
        {
            int result;
        } quit;
        
        struct
        {
            int unused;
        } close;

        struct
        {
            dtk_menu* pMenu;
            dtk_uint32 itemIndex;
            dtk_uint32 itemID;
        } menu;

        struct
        {
            dtk_key key;
            dtk_uint32 modifiers;
            dtk_uint32 id;
        } accelerator;

        struct
        {
            dtk_rect rect;
            dtk_surface* pSurface;
        } paint;

        struct
        {
            dtk_int32 width;
            dtk_int32 height;
        } size;

        struct
        {
            dtk_int32 x;
            dtk_int32 y;
        } move;

        struct
        {
            int unused;
        } show;

        struct
        {
            int unused;
        } hide;

        struct
        {
            int unused;
        } mouseLeave;

        struct
        {
            int unused;
        } mouseEnter;

        struct
        {
            dtk_int32 x;
            dtk_int32 y;
            dtk_uint32 state;
        } mouseMove;

        struct
        {
            dtk_int32 x;
            dtk_int32 y;
            dtk_mouse_button button;
            dtk_uint32 state;
        } mouseButton;

        struct
        {
            dtk_int32 x;
            dtk_int32 y;
            dtk_int32 delta;
            dtk_uint32 state;
        } mouseWheel;

        struct
        {
            dtk_key key;
            dtk_uint32 state;
        } keyDown;

        struct
        {
            dtk_key key;
            dtk_uint32 state;
        } keyUp;

        struct
        {
            dtk_uint32 utf32;
            dtk_uint32 state;
        } printableKeyDown;

        struct
        {
            dtk_control* pOldCapturedControl;
        } captureKeyboard, captureMouse;

        struct
        {
            dtk_control* pNewCapturedControl;
        } releaseKeyboard, releaseMouse;

        struct
        {
            float newDPIScale;
            dtk_int32 suggestedPosX;        // <-- Absolute position.
            dtk_int32 suggestedPosY;
            dtk_int32 suggestedWidth;
            dtk_int32 suggestedHeight;
        } dpiChanged;

        struct
        {
            dtk_int32 x;
            dtk_int32 y;
            dtk_int32 absoluteX;
            dtk_int32 absoluteY;
            dtk_tooltip tooltip;
        } tooltip;

        struct
        {
            int unused;
        } refreshInnerLayout;

        struct
        {
            dtk_application_font_type type;
            dtk_font* pFont;                // <-- Set by the event handler.
        } applicationFont;

        struct
        {
            float scale;                    // <-- Set by the event handler.
        } applicationScale;

        struct
        {
            int unused;
        } button;

        struct
        {
            dtk_bool32 checked;
        } checkbox;

        struct
        {
            dtk_color color;
        } colorButton;

        struct
        {
            dtk_uint32 scrollPos;
        } scrollbar;

        struct
        {
            dtk_int32 newTabIndex;
            dtk_int32 oldTabIndex;
            dtk_int32 tabIndex;
            struct
            {
                dtk_int32 x;
                dtk_int32 y;
                dtk_mouse_button button;
                dtk_uint32 state;
            } mouseButton;
        } tabbar;

        struct
        {
            dtk_int32 newTabIndex;
            dtk_int32 oldTabIndex;
            dtk_int32 tabIndex;
            struct
            {
                dtk_int32 x;
                dtk_int32 y;
                dtk_mouse_button button;
                dtk_uint32 state;
            } mouseButton;
        } tabgroup;

        struct
        {
            dtk_uint32 id;
            const void* pData;  // <-- Marked as const to discourage modification since it's only a _copy_ of the original input data.
            size_t dataSize;
        } custom;
    };
};

#ifdef DTK_GTK
typedef struct
{
    dtk_accelerator accelerator;
    /*GClosure**/ dtk_ptr pClosure;
} dtk_accelerator_gtk;
#endif

// The main toolkit context.
struct dtk_context
{
    dtk_platform platform;
    dtk_event_proc onEvent;
    dtk_log_proc onLog;
    int exitCode;
    dtk_window* pFirstWindow;
    dtk_window* pWindowWithKeyboardCapture;     // This is set in the window's DTK_CAPTURE_KEYBOARD / DTK_RELEASE_KEYBOARD event handlers in dtk_window_default_event_handler()
    dtk_window* pWindowWithMouseCapture;        // ^^^
    dtk_control* pControlWantingKeyboardCapture;
    dtk_control* pControlWithKeyboardCapture;
    dtk_control* pControlWithMouseCapture;
    dtk_window*  pWindowUnderMouse;
    dtk_control* pControlUnderMouse;            // Used for mouse enter/leave state management.
    dtk_int32 lastMousePosX;
    dtk_int32 lastMousePosY;
    void* pUserData;
    dtk_paint_queue paintQueue;
    dtk_font uiFont;
    dtk_font monospaceFont;
    dtk_image stockImages[DTK_STOCK_IMAGE_COUNT];
    dtk_bool32 isUIFontInitialized        : 1;
    dtk_bool32 isMonospaceFontInitialized : 1;

    union
    {
#ifdef DTK_WIN32
        struct
        {
            /*HMODULE*/ dtk_handle hComctl32DLL;
            dtk_proc InitCommonControlsEx;

            /*HMODULE*/ dtk_handle hOle32DLL;
            dtk_proc OleInitialize;
            dtk_proc OleUninitialize;

            /*HMODULE*/ dtk_handle hMsimg32DLL;
            dtk_proc AlphaBlend;

            /*HCURSOR*/ dtk_handle hCursorArrow;
            /*HCURSOR*/ dtk_handle hCursorHand;
            /*HCURSOR*/ dtk_handle hCursorIBeam;
            /*HCURSOR*/ dtk_handle hCursorCross;
            /*HCURSOR*/ dtk_handle hCursorSizeWE;
            /*HCURSOR*/ dtk_handle hCursorSizeNS;

            /*HACCEL*/ dtk_handle hAccel;           // The global accelerator table.
            dtk_accelerator* pAccelerators;
            dtk_uint32 acceleratorCount;
            dtk_uint32 acceleratorCapacity;

            /*HWND*/ dtk_handle hMessagingWindow;   // A special hidden window which is only used for pumping messages, usually custom ones.
            /*HDC*/ dtk_handle hGraphicsDC;         // A special device context for use by the graphics sub-system (usually for font management).

            /*HWND*/ dtk_handle hTooltipWindow;     // The window used for tooltips. The same tooltip window is shared across the entire application.
            /*HWND*/ dtk_handle hLastTooltipOwner;  // The window that last owned the tooltip.
            dtk_string tooltipText;

            void* pCharConvBuffer;                  // For wchar_t <-> char conversions.
            size_t charConvBufferSize;

            dtk_int32* pGlyphCache;                 // The cache of glyph character positions. Used by the graphics sub-system.
            size_t glyphCacheSize;
        } win32;
#endif
#ifdef DTK_GTK
        struct
        {
            /*GdkCursor**/ dtk_ptr pCursorDefault;
            /*GdkCursor**/ dtk_ptr pCursorIBeam;
            /*GdkCursor**/ dtk_ptr pCursorCross;
            /*GdkCursor**/ dtk_ptr pCursorDoubleArrowH;
            /*GdkCursor**/ dtk_ptr pCursorDoubleArrowV;

            /*GtkAccelGroup**/ dtk_ptr pAccelGroup;
            dtk_accelerator_gtk* pAccelerators;
            dtk_uint32 acceleratorCount;
            dtk_uint32 acceleratorCapacity;
        } gtk;

        struct
        {
            int unused;
        } cairo;
#endif
    };
};
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

// Initializes the toolkit.
//
// This can be called multiple times, but each call must be paired with a matching call to dtk_uninit().
//
// You can set onEvent to NULL, but you'll want to call dtk_set_event_callback() later on to hook up an
// event handler.
//
// Once a context has been initialized it's location in memory cannot be changed. The reason for this is
// that controls and whatnot need to reference the context that owns it, which is done via a pointer.
// Consider using malloc() to allocate memory for the context if this becomes an issue for you.
//
// Thread Safety: UNSAFE
dtk_result dtk_init(dtk_context* pTK, dtk_event_proc onEvent, void* pUserData);

// Uninitializes the toolkit.
//
// Thread Safety: UNSAFE
dtk_result dtk_uninit(dtk_context* pTK);

// Sets the global event handler callback.
//
// Thread Safety: SAFE
//   This is implemented as an atomic assignment.
dtk_result dtk_set_event_callback(dtk_context* pTK, dtk_event_proc proc);

// Waits for an event and processes it.
//
// Set <blocking> to DTK_TRUE to block until a message is received. Set to DTK_FALSE to return immediately
// if no event is in the queue, in which case DTK_NO_EVENT will be returned.
//
// <pExitCode> will store the exit code when the loop has been terminated, and can be null. 
//
// Thread Safety: UNSAFE
//   Do not call this from multiple threads. Have a single thread that does all event handling.
dtk_result dtk_next_event(dtk_context* pTK, dtk_bool32 blocking, int* pExitCode);

// Flushes the event queue by handling any events currently sitting in the event queue.
//
// This will not remove any quit messages from the queue. The rationale for this is to ensure the application's
// main loop is given the opportunity to handle it and terminate the application.
//
// An event queue flush is performed whenever a control is uninitialized to ensure there are no lingering events
// that refer to an uninitialized control.
//
// This is non-blocking, but should be considered inefficient. 
void dtk_flush_event_queue(dtk_context* pTK);

// Posts an event to the queue which will later be handled by the event handler of the respective control.
//
// To handle the event immediately, use dtk_handle_control_event().
dtk_result dtk_post_local_event(dtk_event* pEvent);

// Same as dtk_post_local_event(), except handles it immediately instead of posting it to the queue. The return
// value is the value returned by the event handler.
dtk_bool32 dtk_handle_local_event(dtk_event* pEvent);

// Posts a custom event.
//
// This will post an event of type DTK_EVENT_CUSTOM. This will make a copy of the data.
//
// Thread Safety: SAFE
dtk_result dtk_post_custom_event(dtk_context* pTK, dtk_control* pControl, dtk_uint32 eventID, const void* pData, size_t dataSize);

// Same as dtk_post_custom_event(), except handles it immediately rather than posting it to the queue.
//
// This will not make a copy of the data.
dtk_result dtk_handle_custom_event(dtk_context* pTK, dtk_control* pControl, dtk_uint32 eventID, const void* pData, size_t dataSize);

// Posts a paint notification to the event queue to let it know there is a pending paint request for a window.
dtk_result dtk_post_paint_notification_event(dtk_context* pTK, dtk_window* pWindow);

// Handles a paint notification event.
dtk_result dtk_handle_paint_notification_event(dtk_context* pTK, dtk_window* pWindow);

// The default event handler.
//
// Applications should call this from their own global event handler.
dtk_bool32 dtk_default_event_handler(dtk_event* pEvent);

// Posts a quit event to the event queue. This will cause the main loop to terminate and dtk_next_event() to
// return DTK_QUIT.
//
// Thread Safety: SAFE
dtk_result dtk_post_quit_event(dtk_context* pTK, int exitCode);

// Posts DTK_EVENT_TOOLTIP events.
dtk_result dtk_do_tooltip(dtk_context* pTK);

// Sets the logging callback.
dtk_result dtk_set_log_callback(dtk_context* pTK, dtk_log_proc proc);


//// Accelerators ////

// Binds an accelerator.
//
// If you need to bind multiple accelerators, consider binding them in bulk with dtk_bind_accelerators(). Binding
// acceleratos individually on Windows is particularly inefficient.
dtk_result dtk_bind_accelerator(dtk_context* pTK, dtk_accelerator accelerator);

// Binds a group of accelerators.
dtk_result dtk_bind_accelerators(dtk_context* pTK, dtk_accelerator* pAccelerators, dtk_uint32 count);

// Unbinds an accelerator.
dtk_result dtk_unbind_accelerator(dtk_context* pTK, dtk_accelerator accelerator);


//// Screens and Monitors ////

// Retrieves the dimensions of the main screen in pixels. This is _not_ scaled.
dtk_result dtk_get_screen_size(dtk_context* pTK, dtk_uint32* pSizeX, dtk_uint32* pSizeY);


//// DPI Scaling ////

// Retrieves the base DPI scaling factor.
float dtk_get_system_dpi_scale(dtk_context* pTK);

// Retrieves an application-defined scaling factor for GUI elements.
float dtk_get_application_scaling_factor(dtk_context* pTK);



//// Input ////

// Sets the control that should receive keyboard capture. This will fail if the control is not allowed to receive capture.
//
// Setting pControl to NULL is equivalent to dtk_release_keyboard().
dtk_result dtk_capture_keyboard(dtk_context* pTK, dtk_control* pControl);

// Releases the keyboard capture from whatever control currently has the capture.
dtk_result dtk_release_keyboard(dtk_context* pTK);

// Retrieves a pointer to the control with the keyboard capture.
dtk_control* dtk_get_control_with_keyboard_capture(dtk_context* pTK);


// Sets the control that should receive mouse capture.
//
// Setting pControl to NULL is equivalent to dtk_release_mouse().
dtk_result dtk_capture_mouse(dtk_context* pTK, dtk_control* pControl);

// Releases the mouse capture from whatever control currently has the capture.
dtk_result dtk_release_mouse(dtk_context* pTK);

// Retrieves a pointer to the control with the mouse capture.
dtk_control* dtk_get_control_with_mouse_capture(dtk_context* pTK);


//// Graphics ////

// Retrieves the default font for UI elements.
dtk_font* dtk_get_ui_font(dtk_context* pTK);

// Retrieves the default monospace font.
//
// This will first try retrieving the font from the application. If the application does not define a monospace font, it
// will be based on the operating system.
dtk_font* dtk_get_monospace_font(dtk_context* pTK);

// Retrieves a stock image by it's ID.
dtk_image* dtk_get_stock_image(dtk_context* pTK, dtk_uint32 stockImageID);


//// Known Folders ////

// Retrieves the full path of the executable.
//
// The return value is the length of the returned string, including the null terminator. Returns 0 on error.
size_t dtk_get_executable_path(char* pathOut, size_t pathOutSize);

// Retrieves the directory of the executable.
//
// The return value is the length of the returned string, including the null terminator. Returns 0 on error.
//
// The returned string will not include the name of the executable.
size_t dtk_get_executable_directory_path(char* pathOut, size_t pathOutSize);

// Retrieves the path of the user's config directory.
//
// On Windows this will typically be %APPDATA% and on Linux it will usually be ~/.config
size_t dtk_get_config_directory_path(char* pathOut, size_t pathOutSize);


//// User Accounts and Process Management ////

// Retrieves the user name of the user running the application.
size_t dtk_get_username(char* usernameOut, size_t usernameOutSize);

// Retrieves the ID of the current process.
unsigned int dtk_get_process_id();

#endif  // DTK_H

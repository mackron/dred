// Copyright (C) 2019 David Reid. See included LICENSE file.

#define DTK_CONTROL_TYPE_DRED       DTK_CONTROL_TYPE_CUSTOM + 1     // <-- Temporary until dred_control is removed entirely.
#define DRED_CONTROL_TYPE_INFO_BAR  DTK_CONTROL_TYPE_CUSTOM + 2
#define DRED_CONTROL_TYPE_CMDBAR    DTK_CONTROL_TYPE_CUSTOM + 3


typedef struct dred_control dred_control;

// Casts a pointer to any structure to a dred_control*. Note that this will only work if the dred_control object
// is at the first byte of the structure.
#define DRED_CONTROL(a) ((dred_control*)(a))

typedef struct
{
    dtk_color bgColor;
    dtk_color fgColor;
    dtk_font* pFont;
} dred_text_style;

typedef void (* dred_gui_on_move_proc)                 (dred_control* pControl, int newRelativePosX, int newRelativePosY);
typedef void (* dred_gui_on_size_proc)                 (dred_control* pControl, int newWidth, int newHeight);
typedef void (* dred_gui_on_mouse_enter_proc)          (dred_control* pControl);
typedef void (* dred_gui_on_mouse_leave_proc)          (dred_control* pControl);
typedef void (* dred_gui_on_mouse_move_proc)           (dred_control* pControl, int relativeMousePosX, int relativeMousePosY, int stateFlags);
typedef void (* dred_gui_on_mouse_button_down_proc)    (dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);
typedef void (* dred_gui_on_mouse_button_up_proc)      (dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);
typedef void (* dred_gui_on_mouse_button_dblclick_proc)(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);
typedef void (* dred_gui_on_mouse_wheel_proc)          (dred_control* pControl, int delta, int relativeMousePosX, int relativeMousePosY, int stateFlags);
typedef void (* dred_gui_on_key_down_proc)             (dred_control* pControl, dtk_key key, int stateFlags);
typedef void (* dred_gui_on_key_up_proc)               (dred_control* pControl, dtk_key key, int stateFlags);
typedef void (* dred_gui_on_printable_key_down_proc)   (dred_control* pControl, unsigned int character, int stateFlags);
typedef void (* dred_gui_on_paint_proc)                (dred_control* pControl, dtk_rect relativeRect, dtk_surface* pSurface);
typedef void (* dred_gui_on_dirty_proc)                (dred_control* pControl, dtk_rect relativeRect);
typedef void (* dred_gui_on_capture_mouse_proc)        (dred_control* pControl);
typedef void (* dred_gui_on_release_mouse_proc)        (dred_control* pControl);
typedef void (* dred_gui_on_capture_keyboard_proc)     (dred_control* pControl, dtk_control* pPrevCapturedControl);
typedef void (* dred_gui_on_release_keyboard_proc)     (dred_control* pControl, dtk_control* pNewCapturedControl);

struct dred_control
{
    // The base DTK control.
    dtk_control baseControl;

    /// The type of the element, as a string. This is only every used by the host application, and is intended to be used as way
    /// to selectively perform certain operations on specific types of GUI elements.
    char type[64];

    /// The function to call when the element's relative position moves.
    dred_gui_on_move_proc onMove;

    /// The function to call when the element's size changes.
    dred_gui_on_size_proc onSize;

    /// The function to call when the mouse enters the given element.
    dred_gui_on_mouse_enter_proc onMouseEnter;

    /// The function to call when the mouse leaves the given element.
    dred_gui_on_mouse_leave_proc onMouseLeave;

    /// The function to call when the mouse is moved while over the element.
    dred_gui_on_mouse_move_proc onMouseMove;

    /// The function to call when a mouse buttonis pressed while over the element.
    dred_gui_on_mouse_button_down_proc onMouseButtonDown;

    /// The function to call when a mouse button is released while over the element.
    dred_gui_on_mouse_button_up_proc onMouseButtonUp;

    /// The function to call when a mouse button is double-clicked while over the element.
    dred_gui_on_mouse_button_dblclick_proc onMouseButtonDblClick;

    /// The function to call when the mouse wheel it turned while over the element.
    dred_gui_on_mouse_wheel_proc onMouseWheel;

    /// The function to call when a key on the keyboard is pressed or auto-repeated.
    dred_gui_on_key_down_proc onKeyDown;

    /// The function to call when a key on the keyboard is released.
    dred_gui_on_key_up_proc onKeyUp;

    /// The function to call when a printable character is pressed or auto-repeated. This would be used for text editing.
    dred_gui_on_printable_key_down_proc onPrintableKeyDown;

    /// The function to call when the paint event is received.
    dred_gui_on_paint_proc onPaint;

    /// The function to call when the element is marked as dirty.
    dred_gui_on_dirty_proc onDirty;

    /// The event handler to call when an element receives the mouse focus.
    dred_gui_on_capture_mouse_proc onCaptureMouse;

    /// The event handler to call when an element loses the mouse focus.
    dred_gui_on_release_mouse_proc onReleaseMouse;

    /// The event handler to call when an element receives the keyboard focus.
    dred_gui_on_capture_keyboard_proc onCaptureKeyboard;

    /// The event handler to call when an element loses the keyboard focus.
    dred_gui_on_release_keyboard_proc onReleaseKeyboard;
};




/////////////////////////////////////////////////////////////////
// Controls

// The default event handler for dred_control's.
dtk_bool32 dred_control_event_handler(dtk_event* pEvent);

/// Creates an element.
dtk_bool32 dred_control_init(dred_control* pControl, dred_context* pDred, dtk_control* pParent, const char* type, dtk_event_proc onEvent);

/// Deletes and element.
void dred_control_uninit(dred_control* pControl);


/// Sets the type of the element.
///
/// The type name cannot be more than 63 characters in length.
dtk_bool32 dred_control_set_type(dred_control* pControl, const char* type);

/// Retrieves the type fo the element.
const char* dred_control_get_type(dred_control* pControl);

/// Determines whether or not the given element is of the given type.
dtk_bool32 dred_control_is_of_type(dred_control* pControl, const char* type);
dtk_bool32 dred_is_control_type_of_type(const char* type, const char* base);



//// Events ////

/// Registers the on_move event callback.
void dred_control_set_on_move(dred_control* pControl, dred_gui_on_move_proc callback);

/// Registers the on_size event callback.
void dred_control_set_on_size(dred_control* pControl, dred_gui_on_size_proc callback);

/// Registers the on_mouse_enter event callback.
void dred_control_set_on_mouse_enter(dred_control* pControl, dred_gui_on_mouse_enter_proc callback);

/// Registers the on_mouse_leave event callback.
void dred_control_set_on_mouse_leave(dred_control* pControl, dred_gui_on_mouse_leave_proc callback);

/// Registers the on_mouse_move event callback.
void dred_control_set_on_mouse_move(dred_control* pControl, dred_gui_on_mouse_move_proc callback);

/// Registers the on_mouse_button_down event callback.
void dred_control_set_on_mouse_button_down(dred_control* pControl, dred_gui_on_mouse_button_down_proc callback);

/// Registers the on_mouse_button_up event callback.
void dred_control_set_on_mouse_button_up(dred_control* pControl, dred_gui_on_mouse_button_up_proc callback);

/// Registers the on_mouse_button_down event callback.
void dred_control_set_on_mouse_button_dblclick(dred_control* pControl, dred_gui_on_mouse_button_dblclick_proc callback);

/// Registers the on_mouse_wheel event callback.
void dred_control_set_on_mouse_wheel(dred_control* pControl, dred_gui_on_mouse_wheel_proc callback);

/// Registers the on_key_down event callback.
void dred_control_set_on_key_down(dred_control* pControl, dred_gui_on_key_down_proc callback);

/// Registers the on_key_up event callback.
void dred_control_set_on_key_up(dred_control* pControl, dred_gui_on_key_up_proc callback);

/// Registers the on_printable_key_down event callback.
void dred_control_set_on_printable_key_down(dred_control* pControl, dred_gui_on_printable_key_down_proc callback);

/// Registers the on_paint event callback.
void dred_control_set_on_paint(dred_control* pControl, dred_gui_on_paint_proc callback);

/// Registers the on_dirty event callback.
void dred_control_set_on_dirty(dred_control* pControl, dred_gui_on_dirty_proc callback);

/// Registers the on_capture_mouse event callback.
void dred_control_set_on_capture_mouse(dred_control* pControl, dred_gui_on_capture_mouse_proc callback);

/// Registers the on_release_mouse event callback.
void dred_control_set_on_release_mouse(dred_control* pControl, dred_gui_on_release_mouse_proc callback);

/// Registers the on_capture_keyboard event callback.
void dred_control_set_on_capture_keyboard(dred_control* pControl, dred_gui_on_capture_keyboard_proc callback);

/// Registers the on_release_keyboard event callback.
void dred_control_set_on_release_keyboard(dred_control* pControl, dred_gui_on_release_keyboard_proc callback);

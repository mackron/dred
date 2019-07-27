// Copyright (C) 2019 David Reid. See included LICENSE file.

// QUICK NOTES
//
// General
// - dr_gui is a low-level GUI system that works on generic objects referred to as "elements".
// - An element is the most basic unit in dr_gui. It contains basic information about it's layout and hierarchy.
// - Controls can be used as the building blocks for more complex controls such as list boxes and scrollbars.
// - The layout of elements use floats instead of integers. The rationale for this is that it makes it easier to do certain
//   layout arithmetic. For example, if you want to evenly distribute 3 elements across a fixed area, the integer based
//   arithmetic can cause rounding errors which cause the elements to not sit flush against the area. By using float-based
//   arithmetic we can avoid that particular issue.
//
// Hierarchy
// - An element can have a parent and any number of children. If an element does not have a parent, it is referred to as the
//   top-level element.
// - When an element is deleted, it's children will be deleted as well.
// - Top-level elements do not have siblings.
//
// Event Handling
// - The application must notify dr_gui of application-generated events such as key strokes and mouse movements. These are
//   referred to as inbound events. An event that is generated by dr_gui are referred to as outbound events.
// - Inbound events are used to generate outbound events. For example, a mouse-move inbound event will generate an outbound
//   mouse-move event, and perhaps a mouse leave/enter pair.
// - Outbound events are posted and handled immediately. A call to dred_gui_post_inbound_event_*() will not return until all of
//   the outbound events it generates have been handled.
// - Inbound events are not thread safe, however an application is free to post an inbound event from any thread so long as
//   it does it's own synchronization.
// - Inbound events will typically specify the relevant top-level element and let dr_gui do the relevant processing required
//   to generate the appropriate outbound events. For example, the mouse-move event will be specified with respect to the top-
//   level element, but dr_gui will determine the exact child element that the mouse moved on and thus should receive the
//   relevant outbound mouse-move event.
// - There are some special events that are handled differently to normal events. The best example is the paint events. The
//   paint event is only called from dred_control_draw().
// - Key press/release events are only ever posted to the element that has the keyboard capture/focus which is set with
//   dred_gui_capture_keyboard(). Thus, when posting an inbound key event, a top-level element is not required when posting
//   those events. The relevant context is still required, however.
//
// Global Outbound Event Handling
// - At times dr_gui will need to notify the host application in order for certain functionality to work properly. For example.
//   when the mouse is captured it won't work 100% correct unless the host application has a chance to capture the mouse against
//   the container window. Because dr_gui has no notion of a window system it relies on the host application to handle this
//   properly.
// - A global outbound event handler should be implemented for each of the following events:
//   - on_dirty: Called when a region of an element is marked as dirty and needs to be redrawn. The application will want to
//     invalidate the container window to trigger an operating system redraw. Set this with dred_gui_set_global_on_dirty().
//   - on_capture_mouse: Called when the mouse is captured and gives the application the opportunity to capture the mouse against
//     the container window at the operating system level. Set with dred_gui_set_global_on_capture_mouse().
//   - on_release_mouse: Called when the mouse is released. The opposite of on_capture_mouse.
//   - on_capture_keyboard: Called when an element is given the keyboard focus and gives the application the opportunity to
//     apply the keyboard focus to the container window. Set with dred_gui_set_global_on_capture_keyboard().
//   - on_release_keyboard: Called when an element loses the keyboard focus. The opposite of on_capture_keyboard.
//   - on_change_cursor: Called when the current cursor needs to be changed as a result of the mouse moving over a new element.
//
// Layout
// - An element's data structure does not store it's relative position but instead stores it's absolute position. The rationale
//   for this is that storing it as relative complicates absolute positioning calculations because it would need to do a recursive
//   traversal of the element's ancestors.
// - Child elements can be scaled by setting an element's inner scale. The inner scale does not scale the element itself - only
//   it's children.
// - When an element is drawn, everything is scaled by it's inner scale. For example, if the inner scale is 2x and a 100x100 quad
//   is drawn, the quad will be scaled to 200x200. An exception to this rule is fonts, which are never scaled. This 0s because
//   text is always drawn based on the size of the font.
// - Applications should only need to work on unscaled coordinates. That is, an application should never need to worry about
//   manual scaling, except for fonts. When positioning and sizing child elements, they should be done based on unscaled
//   coordinates.
// - Use the inner scale system for DPI awareness.
// - The inner scale is applied recursively. That is, if a top level element has it's inner scale set to 2x and one of it's
//   children has an inner scale of 2x, the actual inner scale of the child element will be 4x.
//
//
// Drawing/Painting
// - Drawing is one of the more complex parts of the GUI because it can be a bit unintuitive regarding exactly when an element
//   is drawn and when a drawing function is allowed to be called.
// - To draw an element, call dred_control_draw(). This takes a pointer to the element to draw and the rectangle region that should
//   be redrawn. Any children that fall inside the specified rectangle region will be redrawn as well. You do not want to call
//   dred_control_draw() on a parent element and then again on it's children because dr_gui will do that automatically.
// - dred_control_draw() does not draw anything directly, but rather calls painting callback routines which is where the actual
//   drawing takes place.
// - Sometimes an application will need to be told when a region of an element is dirty and needs redrawing. An example is
//   event-driven, non real-time applications such as normal desktop applications. To mark an element as dirty, you call the
//   dred_control_dirty() function which takes the element that is dirty, and the rectangle region that needs to be redrawn. This
//   does not redraw the element immediately, but instead posts an on_dirty event for the application. Marking regions as dirty
//   is not strictly required, but you should prefer it for event-driven applications that require painting operations to be
//   performed at specific times (such as inside Win32's WM_PAINT messages).
// - Some operations will cause a region of an element to become dirty - such as when it is resized. dr_gui will
//   automatically mark the relevant regions as dirty which in turn will cause a paint message to be posted. If this is not
//   required, it can be disabled with dred_gui_disable_auto_dirty(). You may want to disable automatic dirtying if you are
//   running a real-time application like a game which would redraw the entire GUI every frame anyway and thus not require
//   handling of the paint message.
// - Real-time application guidelines (games, etc.):
//   - dred_gui_disable_auto_dirty()
//   - dred_control_draw(pTopLevelControl, 0, 0, viewportWidth, viewportHeight) at the end of every frame after your main loop.


//
// EXAMPLES
//
// Basic Drawing:
//
// dred_control_draw(pTopLevelControl, 0, 0, dred_control_get_width(pTopLevelControl), dred_control_get_height(pTopLevelControl));
//
// -------------------------
//
// Event-Driven Drawing (Win32):
//
// void my_global_on_dirty_win32(dred_control* pControl, dtk_rect relativeRect) {
//     dtk_rect absoluteRect = relativeRect;
//     dred_make_rect_absolute(pControl, &absoluteRect);
//
//     RECT rect;
//     rect.left   = absoluteRect.left;
//     rect.top    = absoluteRect.top;
//     rect.right  = absoluteRect.right;
//     rect.height = absoluteRect.bottom;
//     InvalidateRect((HWND)dred_control_get_user_data(dred_control_find_top_level_control(pControl)), &rect, FALSE);
// }
//
// ...
//
// LRESULT CALLBACK MyWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//     ...
//     dred_control* pTopLevelControl = (dred_control*)GetWindowLongPtr(hWnd, 0);
//     if (pTopLevelControl != NULL) {
//         switch (msg) {
//             ...
//             case WM_PAINT:
//             {
//                 RECT rect;
//                 if (GetUpdateRect(hWnd, &rect, FALSE)) {
//                     dred_control_draw(pTopLevelControl, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
//                 }
//
//                 break;
//             }
//             ...
//         }
//     }
//     ...
// }
//

#ifndef DRED_MAX_FONT_FAMILY_LENGTH
#define DRED_MAX_FONT_FAMILY_LENGTH  128
#endif

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
dtk_bool32 dred_control_init(dred_control* pControl, dred_context* pDred, dred_control* pParent, dtk_control* pDTKParent, const char* type, dtk_event_proc onEvent);

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

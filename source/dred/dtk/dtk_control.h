// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef dtk_uint32 dtk_control_type;
#define DTK_CONTROL_TYPE_EMPTY          0
#define DTK_CONTROL_TYPE_WINDOW         1
#define DTK_CONTROL_TYPE_LABEL          2
#define DTK_CONTROL_TYPE_BUTTON         3
#define DTK_CONTROL_TYPE_CHECKBOX       4
#define DTK_CONTROL_TYPE_COLOR_BUTTON   5
#define DTK_CONTROL_TYPE_TABBAR         6
#define DTK_CONTROL_TYPE_TABGROUP       7
#define DTK_CONTROL_TYPE_SCROLLBAR      8
#define DTK_CONTROL_TYPE_TEXTBOX        9
#define DTK_CONTROL_TYPE_COUNT          10  // <-- Update this when a new built-in type is added. Also remember to register the default event handler in dtk_init().
#define DTK_CONTROL_TYPE_CUSTOM         65536

#define DTK_CONTROL_ITERATION_SKIP_WINDOWS              (1 << 0)    // When set, skips over children that a window controls. Needed for painting.
#define DTK_CONTROL_ITERATION_ALWAYS_INCLUDE_CHILDREN   (1 << 1)    // When set, always iterates over remaining children when the callback for a child returns false. Needed for painting. When unset, iteration will terminate immediately as soon as false is returned from a callback.

// Return false to stop iteration. pRelativeRect is both an input and output property. On output it will be
// used as the new clipping rectangle for children.
typedef dtk_bool32 (* dtk_control_visibility_iteration_proc)(dtk_control* pControl, dtk_rect* pRelativeRect, void* pUserData);

// Used to check if a point is sitting on top of a control. This is intended for controls that have an
// irregular, non-rectangular shape. Return true if the point is inside the control; false otherwise.
typedef dtk_bool32 (* dtk_control_hit_test_proc)(dtk_control* pControl, dtk_int32 relativePosX, dtk_int32 relativePosY);

#define DTK_CONTROL(p) ((dtk_control*)(p))
struct dtk_control
{
    dtk_context* pTK;
    dtk_event_proc onEvent;
    dtk_control_hit_test_proc onHitTest;
    dtk_control* pParent;
    dtk_control* pFirstChild;
    dtk_control* pLastChild;
    dtk_control* pNextSibling;
    dtk_control* pPrevSibling;
    void* pUserData;        // For use by the application.
    dtk_control_type type;
    dtk_bool32 isHidden                   : 1;
    dtk_bool32 isClippingDisabled         : 1;
    dtk_bool32 isKeyboardCaptureForbidden : 1;
    dtk_bool32 isMouseCaptureForbidden    : 1;
    dtk_int32 absolutePosX;
    dtk_int32 absolutePosY;
    dtk_uint32 width;
    dtk_uint32 height;
    dtk_system_cursor_type cursor;
};

// Initializes a control.
//
// Once initialized, the location of the control in memory cannot be changed. The reason for this is that a pointer
// to the control is used for managing the hierarchy and referencing.
dtk_result dtk_control_init(dtk_context* pTK, dtk_control* pParent, dtk_control_type type, dtk_event_proc onEvent, dtk_control* pControl);

// Uninitializes a control.
dtk_result dtk_control_uninit(dtk_control* pControl);

// The default event handler for a control.
//
// Event handler procedures should call this function after handling each event, unless they want to skip the default
// handling, which is unlikely.
dtk_bool32 dtk_control_default_event_handler(dtk_event* pEvent);


// Posts an event for this control.
dtk_result dtk_control_post_event(dtk_control* pControl, dtk_event* pEvent);

// Sets the event handler for a control.
dtk_result dtk_control_set_event_handler(dtk_control* pControl, dtk_event_proc onEvent);

// Sets the function to call when a custom hit-test needs to be used for this control.
dtk_result dtk_control_set_hit_test_proc(dtk_control* pControl, dtk_control_hit_test_proc onHitTest);

// Shows a control.
dtk_result dtk_control_show(dtk_control* pControl);

// Hides a control.
dtk_result dtk_control_hide(dtk_control* pControl);

// Determines whether or not the control is visible.
dtk_bool32 dtk_control_is_visible(dtk_control* pControl);

// Recursively determines whether or not the control is visible.
dtk_bool32 dtk_control_is_visible_recursive(dtk_control* pControl);


// Disables clipping against the parent for the given element.
void dtk_control_disable_clipping(dtk_control* pControl);

// Enables clipping against the parent for the given element.
void dtk_control_enable_clipping(dtk_control* pControl);

// Determines whether or not clipping is enabled for the given element.
dtk_bool32 dtk_control_is_clipping_enabled(const dtk_control* pControl);


// Sets the size of a control.
dtk_result dtk_control_set_size(dtk_control* pControl, dtk_uint32 width, dtk_uint32 height);
dtk_result dtk_control_get_size(dtk_control* pControl, dtk_uint32* pWidth, dtk_uint32* pHeight);
dtk_uint32 dtk_control_get_width(dtk_control* pControl);
dtk_uint32 dtk_control_get_height(dtk_control* pControl);

// Sets the absolute position of a control.
dtk_result dtk_control_set_absolute_position(dtk_control* pControl, dtk_int32 posX, dtk_int32 posY);
dtk_result dtk_control_get_absolute_position(dtk_control* pControl, dtk_int32* pPosX, dtk_int32* pPosY);

// Sets the relative position of a control.
dtk_result dtk_control_set_relative_position(dtk_control* pControl, dtk_int32 posX, dtk_int32 posY);
dtk_result dtk_control_get_relative_position(dtk_control* pControl, dtk_int32* pPosX, dtk_int32* pPosY);

// Converts a relative position to absolute. On input, the position represents the relative position. Out output they
// will be set to the absolute position.
dtk_result dtk_control_relative_to_absolute(dtk_control* pControl, dtk_int32* pPosX, dtk_int32* pPosY);

// Makes an absolute position relative to the given control.
dtk_result dtk_control_absolute_to_relative(dtk_control* pControl, dtk_int32* pPosX, dtk_int32* pPosY);


// Retrieves the absolute position and size of a control as a rectangle.
dtk_rect dtk_control_get_absolute_rect(dtk_control* pControl);

// Retrieves the relative position and size of a control as a rectangle.
dtk_rect dtk_control_get_relative_rect(dtk_control* pControl);

// Retrieves a rectangle whose left/top coordinates are set to 0 and the right/bottom coordinates are set to
// the width and height of the control respectively.
dtk_rect dtk_control_get_local_rect(dtk_control* pControl);

// Converts a relative rectangle to absolute.
dtk_rect dtk_control_relative_to_absolute_rect(dtk_control* pControl, dtk_rect relativeRect);

// Converts an absolute rectangle to relative.
dtk_rect dtk_control_absolute_to_relative_rect(dtk_control* pControl, dtk_rect absoluteRect);

// Clamps the given rectangle to a control's rectangle and returns whether or not it is contained within the controls rectangle.
dtk_bool32 dtk_control_clamp_rect(dtk_control* pControl, dtk_rect* pRelativeRect);


// Sets the cursor to use for this control.
dtk_result dtk_control_set_cursor(dtk_control* pControl, dtk_system_cursor_type cursor);

// Retrieves the cursor of this control.
dtk_system_cursor_type dtk_control_get_cursor(dtk_control* pControl);


// Determines whether or not this control is under the mouse.
dtk_bool32 dtk_control_is_under_mouse(dtk_control* pControl);


// Detaches a control from it's parent.
dtk_result dtk_control_detach(dtk_control* pChildControl);

// Attaches a control to a parent, making it the last child.
dtk_result dtk_control_append(dtk_control* pChildControl, dtk_control* pParentControl);

// Attaches a control to a parent, making it the first child.
dtk_result dtk_control_prepend(dtk_control* pChildControl, dtk_control* pParentControl);

// Appends the given control to a sibling.
dtk_result dtk_control_append_sibling(dtk_control* pControlToAppend, dtk_control* pControlToAppendTo);

// Prepends the given control to a sibling.
dtk_result dtk_control_prepend_sibling(dtk_control* pControlToPrepend, dtk_control* pControlToPrependTo);

// Finds the top level control for the given control.
dtk_control* dtk_control_find_top_level_control(dtk_control* pControl);

// Determines whether or not the given element is the parent of the other.
//
// @remarks
//     This is not recursive. Use dred_control_is_ancestor() to do a recursive traversal.
dtk_bool32 dtk_control_is_parent(dtk_control* pParentControl, dtk_control* pChildControl);

// Determines whether or not the given element is a child of the other.
//
// @remarks
//     This is not recursive. Use dred_control_is_descendant() to do a recursive traversal.
dtk_bool32 dtk_control_is_child(dtk_control* pChildControl, dtk_control* pParentControl);

// Determines whether or not the given element is an ancestor of the other.
dtk_bool32 dtk_control_is_ancestor(dtk_control* pAncestorControl, dtk_control* pChildControl);

// Determines whether or not the given element is a descendant of the other.
dtk_bool32 dtk_control_is_descendant(dtk_control* pChildControl, dtk_control* pAncestorControl);

// Determines whether or not the given element is itself or a descendant.
dtk_bool32 dtk_control_is_self_or_ancestor(dtk_control* pAncestorControl, dtk_control* pChildControl);

// Determines whether or not the given element is itself or a descendant.
dtk_bool32 dtk_control_is_self_or_descendant(dtk_control* pChildControl, dtk_control* pAncestorControl);

// Retrieves a pointer to the parent control.
dtk_control* dtk_control_get_parent(dtk_control* pControl);


// Finds the window that the given control is part of. All this does is performs an updwards traversal of
// the hierarchy and returns the first occurance of a dtk_window control. If <pControl> itself is a window,
// this will return <pControl>.
dtk_window* dtk_control_get_window(dtk_control* pControl);


// Recursively iterates over visible controls, including the given top-level control (pControl).
//
// <relativeRect> should be relative to <pControl>
//
// If <callback> returns false, iteration will be terminated and false will be returned. <callbackPost> is called
// after <callback> _and_ the controls children. It's used to allow the callbacks to save and restore state for
// recursive traversals.
//
// <options> is any combination of DTK_CONTROL_ITERATION_*.
dtk_bool32 dtk_control_iterate_visible_controls(dtk_control* pControl, dtk_rect relativeRect, dtk_control_visibility_iteration_proc callback, dtk_control_visibility_iteration_proc callbackPost, dtk_uint32 options, void* pUserData);


// Schedules a redraw of the given control.
//
// This API will combine redraws to improve efficiency, but will not execute them immediately. Instead it
// posts a redraw request to an internal queue which will be handled by the main loop.
//
// Use dtk_control_immediate_redraw() to redraw the control immediately. Note, however, that this can be less
// efficient since it doesn't do batching.
//
// Note that redrawing a control will also redraw ancestors and descendants.
dtk_result dtk_control_scheduled_redraw(dtk_control* pControl, dtk_rect relativeRect);

// Immediately redraws the given control.
//
// This API can have some performance issues if you issue a lot of small redraws at once. Consider using
// dtk_control_scheduled_redraw() in this scenario since that API will automatically combine redraws where
// possible.
//
// Note that redrawing a control will also redraw ancestors and descendants.
dtk_result dtk_control_immediate_redraw(dtk_control* pControl, dtk_rect relativeRect);


// Enables the ability for the control to receive keyboard focus (default).
dtk_result dtk_control_allow_keyboard_capture(dtk_control* pControl);

// Disables the ability for the control to receive keyboard focus.
//
// If the control has the keyboard focus at the time this is called, it will be unfocused.
dtk_result dtk_control_forbid_keyboard_capture(dtk_control* pControl);

// Determines whether or not the control has the ability to receive keyboard focus.
//
// This is recursive because capture allowance depends on the ancestory.
dtk_bool32 dtk_control_is_keyboard_capture_allowed(dtk_control* pControl);


// Helper function for giving a control the keyboard capture. This will fail if keyboard capture is forbidden for the control.
dtk_result dtk_control_capture_keyboard(dtk_control* pControl);

// Helper function for determining whether or not the given control has the keyboard capture.
dtk_bool32 dtk_control_has_keyboard_capture(dtk_control* pControl);


// Helper function for giving a control the mouse capture. This will fail if mouse capture is forbidden for the control.
dtk_result dtk_control_capture_mouse(dtk_control* pControl);

// Helper function for determining whether or not the given control has the mouse capture.
dtk_bool32 dtk_control_has_mouse_capture(dtk_control* pControl);


// Determines whether or not the given point, in absolute coordinates, is inside the bounds of the control. This does not
// perform a hit test, but instead only checks the bounds. Use dtk_control_is_point_inside() to also check against the hit
// test callback.
dtk_bool32 dtk_control_is_point_inside_bounds(dtk_control* pControl, dtk_int32 absolutePosX, dtk_int32 absolutePosY);

// The same as dtk_control_is_point_inside_bounds(), except also checks against the hit test callback.
dtk_bool32 dtk_control_is_point_inside(dtk_control* pControl, dtk_int32 absolutePosX, dtk_int32 absolutePosY);

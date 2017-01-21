// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef dtk_uint32 dtk_control_type;
#define DTK_CONTROL_TYPE_EMPTY      0
#define DTK_CONTROL_TYPE_WINDOW     1
#define DTK_CONTROL_TYPE_SCROLLBAR  2
#define DTK_CONTROL_TYPE_COUNT      3   // <-- Update this when a new built-in type is added. Also remember to register the default event handler in dtk_init().
#define DTK_CONTROL_TYPE_CUSTOM     65536

#define DTK_CONTROL_FLAG_HIDDEN                     (1 << 0)
#define DTK_CONTROL_FLAG_FORBID_KEYBOARD_CAPTURE    (1 << 1)
#define DTK_CONTROL_FLAG_FORBID_MOUSE_CAPTURE       (1 << 2)

#define DTK_CONTROL(p) ((dtk_control*)(p))
struct dtk_control
{
    dtk_context* pTK;
    dtk_event_proc onEvent;
    dtk_control* pParent;
    dtk_control* pFirstChild;
    dtk_control* pLastChild;
    dtk_control* pNextSibling;
    dtk_control* pPrevSibling;
    void* pUserData;        // For use by the application.
    dtk_control_type type;
    dtk_uint32 flags;       // DTK_CONTROL_FLAG_*
    dtk_int32 absolutePosX;
    dtk_int32 absolutePosY;
    dtk_uint32 width;
    dtk_uint32 height;
};

// Initializes a control.
//
// Once initialized, the location of the control in memory cannot be changed. The reason for this is that a pointer
// to the control is used for managing the hierarchy and referencing.
dtk_result dtk_control_init(dtk_context* pTK, dtk_control* pParent, dtk_control_type type, dtk_event_proc onEvent, dtk_control* pControl);

// Uninitializes a control.
dtk_result dtk_control_uninit(dtk_control* pControl);

// Sets the event handler for a control.
dtk_result dtk_control_set_event_handler(dtk_control* pControl, dtk_event_proc onEvent);

// Shows a control.
dtk_result dtk_control_show(dtk_control* pControl);

// Hides a control.
dtk_result dtk_control_hide(dtk_control* pControl);

// Determines whether or not the control is visible.
dtk_bool32 dtk_control_is_visible(dtk_control* pControl);


// Sets the size of a window.
dtk_result dtk_control_set_size(dtk_control* pControl, dtk_uint32 width, dtk_uint32 height);
dtk_result dtk_control_get_size(dtk_control* pControl, dtk_uint32* pWidth, dtk_uint32* pHeight);

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


// Finds the top level control for the given control.
dtk_control* dtk_control_find_top_level_control(dtk_control* pControl);

// Finds the window that the given control is part of. All this does is performs an updwards traversal of
// the hierarchy and returns the first occurance of a dtk_window control. If <pControl> itself is a window,
// this will return <pControl>.
dtk_window* dtk_control_get_window(dtk_control* pControl);


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

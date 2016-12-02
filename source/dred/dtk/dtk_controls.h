// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef dtk_uint32 dtk_control_type;
#define DTK_CONTROL_TYPE_EMPTY      0
#define DTK_CONTROL_TYPE_WINDOW     1
#define DTK_CONTROL_TYPE_CUSTOM     65536

#define DTK_CONTROL(p) ((dtk_control*)(p))
struct dtk_control
{
    dtk_context* pTK;
    dtk_control_type type;
    dtk_event_proc onEvent;
    dtk_surface* pSurface;  // This is usually set to the surface of a top-level window control and will change when the hierarchy changes where appropriate.
    dtk_control* pParent;
    dtk_int32 absolutePosX;
    dtk_int32 absolutePosY;
    dtk_uint32 width;
    dtk_uint32 height;
    void* pUserData;        // For use by the application.

    // TESTING
    dtk_font font;
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


// Finds the top level control for the given control.
dtk_control* dtk_control_find_top_level_control(dtk_control* pControl);

// Finds the window that the given control is part of. All this does is performs an updwards traversal of
// the hierarchy and returns the first occurance of a dtk_window control. If <pControl> itself is a window,
// this will return <pControl>.
dtk_window* dtk_control_get_window(dtk_control* pControl);

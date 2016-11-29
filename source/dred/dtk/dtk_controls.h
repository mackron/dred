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

    // TESTING
    dtk_font font;
};

// Initializes a control.
//
// Once initialized, the location of the control in memory cannot be changed. The reason for this is that a pointer
// to the control is used for managing the hierarchy and referencing.
dtk_result dtk_control_init(dtk_context* pTK, dtk_control_type type, dtk_event_proc onEvent, dtk_control* pControl);

// Uninitializes a control.
dtk_result dtk_control_uninit(dtk_control* pControl);

// Sets the event handler for a control.
dtk_result dtk_control_set_event_handler(dtk_control* pControl, dtk_event_proc onEvent);

// Shows a control.
dtk_result dtk_control_show(dtk_control* pControl);

// Hides a control.
dtk_result dtk_control_hide(dtk_control* pControl);
// Copyright (C) 2019 David Reid. See included LICENSE file.

#define DTK_CONTROL_TYPE_DRED               DTK_CONTROL_TYPE_CUSTOM + 1     // <-- Temporary until dred_control is removed entirely.
#define DRED_CONTROL_TYPE_INFO_BAR          DTK_CONTROL_TYPE_CUSTOM + 2
#define DRED_CONTROL_TYPE_CMDBAR            DTK_CONTROL_TYPE_CUSTOM + 3
#define DRED_CONTROL_TYPE_TEXTVIEW          DTK_CONTROL_TYPE_CUSTOM + 4
#define DRED_CONTROL_TYPE_TEXTBOX           DTK_CONTROL_TYPE_CUSTOM + 5
#define DRED_CONTROL_TYPE_SETTINGS_EDITOR   DTK_CONTROL_TYPE_CUSTOM + 6
#define DRED_CONTROL_TYPE_TEXT_EDITOR2      DTK_CONTROL_TYPE_CUSTOM + 7 // Rename to DRED_CONTROL_TYPE_TEXT_EDITOR when dred_control is removed entirely.

/* This is temporary until the new editor system is in place. This system will use the event system to handle editor functionality. */
static dtk_bool32 dred_is_control_editor(dtk_control* pControl)
{
    return pControl->type == DRED_CONTROL_TYPE_SETTINGS_EDITOR || pControl->type == DRED_CONTROL_TYPE_TEXT_EDITOR2;
}

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

struct dred_control
{
    // The base DTK control.
    dtk_control baseControl;

    /// The type of the element, as a string. This is only every used by the host application, and is intended to be used as way
    /// to selectively perform certain operations on specific types of GUI elements.
    char type[64];
};




/////////////////////////////////////////////////////////////////
// Controls

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

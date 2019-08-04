// Copyright (C) 2019 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_INFO_BAR          DTK_CONTROL_TYPE_CUSTOM + 1
#define DRED_CONTROL_TYPE_CMDBAR            DTK_CONTROL_TYPE_CUSTOM + 2
#define DRED_CONTROL_TYPE_TEXTVIEW          DTK_CONTROL_TYPE_CUSTOM + 3
#define DRED_CONTROL_TYPE_TEXTBOX           DTK_CONTROL_TYPE_CUSTOM + 4
#define DRED_CONTROL_TYPE_SETTINGS_EDITOR   DTK_CONTROL_TYPE_CUSTOM + 5
#define DRED_CONTROL_TYPE_TEXT_EDITOR       DTK_CONTROL_TYPE_CUSTOM + 6

/* This is temporary until the new editor system is in place. This system will use the event system to handle editor functionality. */
static dtk_bool32 dred_is_control_editor(dtk_control* pControl)
{
    return pControl->type == DRED_CONTROL_TYPE_SETTINGS_EDITOR || pControl->type == DRED_CONTROL_TYPE_TEXT_EDITOR;
}

typedef struct
{
    dtk_color bgColor;
    dtk_color fgColor;
    dtk_font* pFont;
} dred_text_style;

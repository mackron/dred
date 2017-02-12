// Copyright (C) 2016 David Reid. See included LICENSE file.

struct dtk_textbox
{
    dtk_control control;
};

dtk_result dtk_textbox_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_textbox* pTextBox);
dtk_result dtk_textbox_uninit(dtk_textbox* pTextBox);
dtk_bool32 dtk_textbox_default_event_handler(dtk_event* pEvent);
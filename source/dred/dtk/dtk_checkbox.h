// Copyright (C) 2017 David Reid. See included LICENSE file.

struct dtk_checkbox
{
    dtk_control control;
};

dtk_result dtk_checkbox_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_checkbox* pCheckbox);
dtk_result dtk_checkbox_uninit(dtk_checkbox* pCheckbox);
dtk_bool32 dtk_checkbox_default_event_handler(dtk_event* pEvent);
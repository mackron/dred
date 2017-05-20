// Copyright (C) 2017 David Reid. See included LICENSE file.

struct dtk_button
{
    dtk_control control;
};

dtk_result dtk_button_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_button* pButton);
dtk_result dtk_button_uninit(dtk_button* pButton);
dtk_bool32 dtk_button_default_event_handler(dtk_event* pEvent);
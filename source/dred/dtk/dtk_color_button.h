// Copyright (C) 2017 David Reid. See included LICENSE file.

struct dtk_color_button
{
    dtk_control control;
};

dtk_result dtk_color_button_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_color_button* pColorButton);
dtk_result dtk_color_button_uninit(dtk_color_button* pColorButton);
dtk_bool32 dtk_color_button_default_event_handler(dtk_event* pEvent);
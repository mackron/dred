// Copyright (C) 2019 David Reid. See included LICENSE file.

struct dtk_label
{
    dtk_control control;
};

dtk_result dtk_label_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_label* pLabel);
dtk_result dtk_label_uninit(dtk_label* pLabel);
dtk_bool32 dtk_label_default_event_handler(dtk_event* pEvent);
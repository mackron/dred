// Copyright (C) 2016 David Reid. See included LICENSE file.

struct dtk_tabbar
{
    dtk_control control;
};

dtk_result dtk_tabbar_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_uninit(dtk_tabbar* pTabBar);
dtk_bool32 dtk_tabbar_default_event_handler(dtk_event* pEvent);
// Copyright (C) 2016 David Reid. See included LICENSE file.

struct dtk_tabgroup
{
    dtk_control control;
    dtk_tabbar tabbar;
};

dtk_result dtk_tabgroup_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_tabgroup* pTabGroup);
dtk_result dtk_tabgroup_uninit(dtk_tabgroup* pTabGroup);
dtk_bool32 dtk_tabgroup_default_event_handler(dtk_event* pEvent);
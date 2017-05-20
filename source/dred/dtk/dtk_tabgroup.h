// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef enum
{
    dtk_tabgroup_tabbar_edge_top,
    dtk_tabgroup_tabbar_edge_left,
    dtk_tabgroup_tabbar_edge_bottom,
    dtk_tabgroup_tabbar_edge_right
} dtk_tabgroup_tabbar_edge;

#define DTK_TABGROUP(p) ((dtk_tabgroup*)(p))
struct dtk_tabgroup
{
    dtk_control control;
    dtk_tabbar tabbar;
    dtk_control container;  // <-- The parent control for tab pages. This is automatically positioned and sized based on the main control and the tabbar.
    dtk_tabgroup_tabbar_edge tabbarEdge;
};

dtk_result dtk_tabgroup_init(dtk_context* pTK, dtk_control* pParent, dtk_tabgroup_tabbar_edge tabbarEdge, dtk_tabbar_flow tabbarFlow, dtk_tabbar_text_direction tabbarTextDirection, dtk_event_proc onEvent, dtk_tabgroup* pTabGroup);
dtk_result dtk_tabgroup_uninit(dtk_tabgroup* pTabGroup);
dtk_bool32 dtk_tabgroup_default_event_handler(dtk_event* pEvent);
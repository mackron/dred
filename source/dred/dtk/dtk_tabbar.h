// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef enum
{
    dtk_tabbar_orientation_top,
    dtk_tabbar_orientation_bottom,
    dtk_tabbar_orientation_left,
    dtk_tabbar_orientation_right
} dtk_tabbar_orientation;

typedef struct
{
    const char* text;
} dtk_tabbar_tab;

struct dtk_tabbar
{
    dtk_control control;
    dtk_tabbar_orientation orientation;
    dtk_tabbar_tab* pTabs;
    dtk_uint32 tabCount;
    dtk_uint32 tabCapacity;
    dtk_bool32 isAutoResizeEnabled : 1;

    // Styling.
    dtk_font* pFont;
    dtk_color textFGColor;
    dtk_color textBGColor;
};

dtk_result dtk_tabbar_init(dtk_context* pTK, dtk_control* pParent, dtk_tabbar_orientation orientation, dtk_event_proc onEvent, dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_uninit(dtk_tabbar* pTabBar);
dtk_bool32 dtk_tabbar_default_event_handler(dtk_event* pEvent);
dtk_result dtk_tabbar_set_font(dtk_tabbar* pTabBar, dtk_font* pFont);
dtk_result dtk_tabbar_set_text_fg_color(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_text_bg_color(dtk_tabbar* pTabBar, dtk_color color);

dtk_result dtk_tabbar_append_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage, dtk_tabbar_tab** ppTabOut);
dtk_result dtk_tabbar_prepend_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage, dtk_tabbar_tab** ppTabOut);
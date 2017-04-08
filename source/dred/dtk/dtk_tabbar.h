// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef enum
{
    dtk_tabbar_flow_left_to_right,
    dtk_tabbar_flow_top_to_bottom,
    dtk_tabbar_flow_right_to_left,
    dtk_tabbar_flow_bottom_to_top
} dtk_tabbar_flow;

typedef enum
{
    dtk_tabbar_text_direction_horizontal,
    dtk_tabbar_text_direction_vertical
} dtk_tabbar_text_direction;

typedef struct
{
    char* text;
    dtk_control* pPage;
} dtk_tabbar_tab;

#define DTK_TABBAR(p) ((dtk_tabbar*)(p))
struct dtk_tabbar
{
    dtk_control control;
    dtk_tabbar_flow flow;
    dtk_tabbar_text_direction textDirection;
    dtk_tabbar_tab* pTabs;
    dtk_uint32 tabCount;
    dtk_uint32 tabCapacity;
    dtk_bool32 isAutoResizeEnabled : 1;

    // Styling.
    dtk_font* pFont;
    dtk_color textFGColor;
    dtk_color textBGColor;
};

dtk_result dtk_tabbar_init(dtk_context* pTK, dtk_control* pParent, dtk_tabbar_flow flow, dtk_tabbar_text_direction textDirection, dtk_event_proc onEvent, dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_uninit(dtk_tabbar* pTabBar);
dtk_bool32 dtk_tabbar_default_event_handler(dtk_event* pEvent);
dtk_result dtk_tabbar_set_font(dtk_tabbar* pTabBar, dtk_font* pFont);
dtk_font* dtk_tabbar_get_font(dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_set_text_fg_color(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_text_bg_color(dtk_tabbar* pTabBar, dtk_color color);

dtk_result dtk_tabbar_append_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage);
dtk_result dtk_tabbar_prepend_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage);
dtk_result dtk_tabbar_remove_tab_by_index(dtk_tabbar* pTabBar, size_t tabIndex);
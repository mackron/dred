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

dtk_result dtk_tabgroup_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, dtk_tabgroup_tabbar_edge tabbarEdge, dtk_tabbar_flow tabbarFlow, dtk_tabbar_text_direction tabbarTextDirection, dtk_tabgroup* pTabGroup);
dtk_result dtk_tabgroup_uninit(dtk_tabgroup* pTabGroup);
dtk_bool32 dtk_tabgroup_default_event_handler(dtk_event* pEvent);

dtk_control* dtk_tabgroup_get_tab_page_container(dtk_tabgroup* pTabGroup);

dtk_result dtk_tabgroup_append_tab(dtk_tabgroup* pTabGroup, const char* text, dtk_control* pTabPage, dtk_uint32* pTabIndexOut);
dtk_result dtk_tabgroup_prepend_tab(dtk_tabgroup* pTabGroup, const char* text, dtk_control* pTabPage, dtk_uint32* pTabIndexOut);
dtk_result dtk_tabgroup_remove_tab_by_index(dtk_tabgroup* pTabGroup, dtk_uint32 tabIndex);
dtk_uint32 dtk_tabgroup_get_tab_count(dtk_tabgroup* pTabGroup);

dtk_control* dtk_tabgroup_get_tab_page(dtk_tabgroup* pTabGroup, dtk_uint32 tabIndex);
dtk_uint32 dtk_tabgroup_get_active_tab_index(dtk_tabgroup* pTabGroup);
dtk_result dtk_tabgroup_activate_tab(dtk_tabgroup* pTabGroup, dtk_uint32 tabIndex);
dtk_result dtk_tabgroup_activate_next_tab(dtk_tabgroup* pTabGroup);
dtk_result dtk_tabgroup_activate_prev_tab(dtk_tabgroup* pTabGroup);

dtk_result dtk_tabgroup_set_tab_text(dtk_tabgroup* pTabGroup, dtk_uint32 tabIndex, const char* text);

// Calculates the size of the content area of the given tab group. Will be different depending on whether or not the tab bar is visible.
dtk_result dtk_tabgroup_get_container_size(dtk_tabgroup* pTabGroup, dtk_int32* pWidth, dtk_int32* pHeight);

dtk_result dtk_tabgroup_show_tabbar(dtk_tabgroup* pTabGroup);
dtk_result dtk_tabgroup_hide_tabbar(dtk_tabgroup* pTabGroup);
dtk_bool32 dtk_tabgroup_is_showing_tabbar(const dtk_tabgroup* pTabGroup);


// Styling
dtk_result dtk_tabgroup_set_tabbar_background_color(dtk_tabgroup* pTabGroup, dtk_color color);
dtk_result dtk_tabgroup_set_tab_padding(dtk_tabgroup* pTabGroup, dtk_uint32 padding);
dtk_result dtk_tabgroup_set_tab_background_color(dtk_tabgroup* pTabGroup, dtk_color color);
dtk_result dtk_tabgroup_set_tab_background_color_active(dtk_tabgroup* pTabGroup, dtk_color color);
dtk_result dtk_tabgroup_set_tab_background_color_hovered(dtk_tabgroup* pTabGroup, dtk_color color);
dtk_result dtk_tabgroup_set_tab_font(dtk_tabgroup* pTabGroup, dtk_font* pFont);
dtk_result dtk_tabgroup_set_tab_text_color(dtk_tabgroup* pTabGroup, dtk_color color);
dtk_result dtk_tabgroup_set_tab_text_color_active(dtk_tabgroup* pTabGroup, dtk_color color);
dtk_result dtk_tabgroup_set_tab_text_color_hovered(dtk_tabgroup* pTabGroup, dtk_color color);
dtk_result dtk_tabgroup_set_tab_close_button_left_padding(dtk_tabgroup* pTabGroup, dtk_uint32 padding);
dtk_result dtk_tabgroup_set_tab_close_button_image(dtk_tabgroup* pTabGroup, dtk_image* pImage);
dtk_result dtk_tabgroup_set_tab_close_button_color(dtk_tabgroup* pTabGroup, dtk_color color);
dtk_result dtk_tabgroup_set_tab_close_button_color_hovered(dtk_tabgroup* pTabGroup, dtk_color color);
dtk_result dtk_tabgroup_set_tab_close_button_color_pressed(dtk_tabgroup* pTabGroup, dtk_color color);
dtk_result dtk_tabgroup_show_tab_close_buttons(dtk_tabgroup* pTabGroup);
dtk_result dtk_tabgroup_hide_tab_close_buttons(dtk_tabgroup* pTabGroup);

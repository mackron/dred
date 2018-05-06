// Copyright (C) 2017 David Reid. See included LICENSE file.

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
    dtk_int32 tabIndex;     // <-- Will be set to -1 if the point is not over any tab.
    dtk_int32 relativePosX; // <-- The position relative to the tab itself (not the whole bar)
    dtk_int32 relativePosY;
    dtk_rect tabRect;
    dtk_bool32 isOverCloseButton;
    dtk_bool32 isOverPinButton;
} dtk_tabbar_hit_test_result;

typedef struct
{
    char* pText;
    dtk_control* pPage;
    dtk_string pTooltipText;
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
    dtk_bool32 isTooltipVisible : 1;
    dtk_int32 hoveredTabIndex;  // Set to -1 if no tab is hovered.
    dtk_int32 activeTabIndex;   // Set to -1 when no tab is active.

    // Styling.
    dtk_font* pFont;
    dtk_image* pCloseButtonImage;
    dtk_color bgColor;  // <-- The background color of the control itself.
    dtk_color bgColorTab;
    dtk_color bgColorActiveTab;
    dtk_color bgColorHoveredTab;
    dtk_color textColor;
    dtk_uint32 paddingLeft;
    dtk_uint32 paddingTop;
    dtk_uint32 paddingRight;
    dtk_uint32 paddingBottom;
    dtk_uint32 closeButtonPaddingLeft;
    dtk_uint32 closeButtonPaddingTop;
    dtk_uint32 closeButtonPaddingRight;
    dtk_uint32 closeButtonPaddingBottom;
    dtk_uint32 closeButtonWidth;
    dtk_uint32 closeButtonHeight;
    dtk_color closeButtonColor;
    dtk_color closeButtonColorHovered;
    dtk_color closeButtonColorPressed;
    dtk_bool32 isShowingCloseButton : 1;
    dtk_bool32 isShowingPinButton   : 1;
};

dtk_result dtk_tabbar_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, dtk_tabbar_flow flow, dtk_tabbar_text_direction textDirection, dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_uninit(dtk_tabbar* pTabBar);
dtk_bool32 dtk_tabbar_default_event_handler(dtk_event* pEvent);

dtk_result dtk_tabbar_set_font(dtk_tabbar* pTabBar, dtk_font* pFont);
dtk_font* dtk_tabbar_get_font(dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_set_close_button_image(dtk_tabbar* pTabBar, dtk_image* pImage);
dtk_image* dtk_tabbar_get_close_button_image(dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_set_text_color(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_bg_color(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_bg_color_active(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_bg_color_hovered(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_padding(dtk_tabbar* pTabBar, dtk_uint32 paddingLeft, dtk_uint32 paddingTop, dtk_uint32 paddingRight, dtk_uint32 paddingBottom);

dtk_result dtk_tabbar_show_close_button(dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_hide_close_button(dtk_tabbar* pTabBar);
dtk_bool32 dtk_tabbar_is_showing_close_button(dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_set_close_button_size(dtk_tabbar* pTabBar, dtk_uint32 width, dtk_uint32 height);
dtk_result dtk_tabbar_get_close_button_size(dtk_tabbar* pTabBar, dtk_uint32* pWidth, dtk_uint32* pHeight);
dtk_uint32 dtk_tabbar_get_close_button_width(dtk_tabbar* pTabBar);
dtk_uint32 dtk_tabbar_get_close_button_height(dtk_tabbar* pTabBar);

dtk_result dtk_tabbar_append_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage);
dtk_result dtk_tabbar_prepend_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage);
dtk_result dtk_tabbar_remove_tab_by_index(dtk_tabbar* pTabBar, dtk_uint32 tabIndex);

dtk_result dtk_tabbar_set_tab_tooltip(dtk_tabbar* pTabBar, dtk_uint32 tabIndex, const char* pTooltipText);

// Performs a hit test against the tabs in the tab bar.
//
// Returns true if the point is over a tab; false otherwise.
dtk_bool32 dtk_tabbar_hit_test(dtk_tabbar* pTabBar, dtk_int32 x, dtk_int32 y, dtk_tabbar_hit_test_result* pResult);
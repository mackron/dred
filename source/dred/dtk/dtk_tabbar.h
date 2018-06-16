// Copyright (C) 2017 David Reid. See included LICENSE file.

// Events
//
// DTK_EVENT_TABBAR_MOUSE_BUTTON_DOWN_TAB
//   Called when a mouse button is pressed while over a tab.
//
// DTK_EVENT_TABBAR_MOUSE_BUTTON_UP_TAB
//   Called when a mouse button is released while over a tab.
//
// DTK_EVENT_TABBAR_MOUSE_BUTTON_DBLCLICK_TAB
//   Called when a mouse button is double clicked while over a tab.
//
// DTK_EVENT_TABBAR_CHANGE_TAB
//   Called when the active tab changes. The default handler will change the tab.
//
// DTK_EVENT_TABBAR_CLOSE_TAB
//   Called when the close button on a tab is pressed. The default handle does nothing (left entirely up to the application).
//
// DTK_EVENT_TABBAR_PIN_TAB
//   Called when a tab is pinned. The default handler will pin the tab. To prevent pinning, don't call the default handler.
//
// DTK_EVENT_TABBAR_UNPIN_TAB
//   Called when a tab is unpinned. The default handler will unpin the tab. To prevent unpinning, don't call the default handler.
//
// DTK_EVENT_TABBAR_REMOVE_TAB
//   Called when a tab is removed. The default handler will remove the tab. To prevent removal, don't call the default handler.

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
    dtk_bool32 isPinned : 1;
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
    dtk_bool32 isAutoResizeEnabled          : 1;
    dtk_bool32 isTooltipVisible             : 1;
    dtk_bool32 isShowingCloseButton         : 1;
    dtk_bool32 isShowingPinButton           : 1;
    dtk_bool32 isMouseOverCloseButton       : 1;
    dtk_bool32 isMouseOverPinButton         : 1;
    dtk_bool32 isCloseButtonPressed         : 1;
    dtk_bool32 isPinButtonPressed           : 1;
    dtk_bool32 isCloseOnMiddleClientEnabled : 1;
    dtk_int32 hoveredTabIndex;          // Set to -1 if no tab is hovered.
    dtk_int32 activeTabIndex;           // Set to -1 when no tab is active.
    dtk_int32 closeButtonHeldTabIndex;  // Set to the index of the tab whose close button is being held. Set to -1 if none.
    dtk_int32 pinButtonHeldTabIndex;    // Set to the index of the tab whose pin button is being held. Set to -1 if none.

    // Styling.
    dtk_font* pFont;
    dtk_image* pCloseButtonImage;
    dtk_color bgColor;  // <-- The background color of the control itself.
    dtk_color bgColorTab;
    dtk_color bgColorActiveTab;
    dtk_color bgColorHoveredTab;
    dtk_color textColor;
    dtk_color textColorActive;
    dtk_color textColorHovered;
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
    dtk_uint32 pinButtonPaddingLeft;
    dtk_uint32 pinButtonPaddingTop;
    dtk_uint32 pinButtonPaddingRight;
    dtk_uint32 pinButtonPaddingBottom;
    dtk_uint32 pinButtonWidth;
    dtk_uint32 pinButtonHeight;
    dtk_color pinButtonColor;
    dtk_color pinButtonColorHovered;
    dtk_color pinButtonColorPressed;
};

dtk_result dtk_tabbar_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, dtk_tabbar_flow flow, dtk_tabbar_text_direction textDirection, dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_uninit(dtk_tabbar* pTabBar);
dtk_bool32 dtk_tabbar_default_event_handler(dtk_event* pEvent);

dtk_result dtk_tabbar_set_bg_color(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_font(dtk_tabbar* pTabBar, dtk_font* pFont);
dtk_font* dtk_tabbar_get_font(const dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_set_close_button_image(dtk_tabbar* pTabBar, dtk_image* pImage);
dtk_image* dtk_tabbar_get_close_button_image(const dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_set_close_button_color(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_close_button_color_hovered(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_close_button_color_pressed(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_text_color(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_text_color_active(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_text_color_hovered(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_tab_bg_color(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_tab_bg_color_active(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_tab_bg_color_hovered(dtk_tabbar* pTabBar, dtk_color color);
dtk_result dtk_tabbar_set_tab_padding(dtk_tabbar* pTabBar, dtk_uint32 paddingLeft, dtk_uint32 paddingTop, dtk_uint32 paddingRight, dtk_uint32 paddingBottom);
dtk_result dtk_tabbar_set_close_button_padding(dtk_tabbar* pTabBar, dtk_uint32 paddingLeft, dtk_uint32 paddingTop, dtk_uint32 paddingRight, dtk_uint32 paddingBottom);
dtk_result dtk_tabbar_set_pin_button_padding(dtk_tabbar* pTabBar, dtk_uint32 paddingLeft, dtk_uint32 paddingTop, dtk_uint32 paddingRight, dtk_uint32 paddingBottom);

dtk_result dtk_tabbar_show_close_button(dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_hide_close_button(dtk_tabbar* pTabBar);
dtk_bool32 dtk_tabbar_is_showing_close_button(const dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_set_close_button_size(dtk_tabbar* pTabBar, dtk_uint32 width, dtk_uint32 height);
dtk_result dtk_tabbar_get_close_button_size(const dtk_tabbar* pTabBar, dtk_uint32* pWidth, dtk_uint32* pHeight);
dtk_uint32 dtk_tabbar_get_close_button_width(const dtk_tabbar* pTabBar);
dtk_uint32 dtk_tabbar_get_close_button_height(const dtk_tabbar* pTabBar);

dtk_result dtk_tabbar_append_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage, dtk_uint32* pTabIndexOut);
dtk_result dtk_tabbar_prepend_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage, dtk_uint32* pTabIndexOut);
dtk_result dtk_tabbar_remove_tab_by_index(dtk_tabbar* pTabBar, dtk_uint32 tabIndex);
dtk_uint32 dtk_tabbar_get_tab_count(dtk_tabbar* pTabBar);

dtk_control* dtk_tabbar_get_tab_page(dtk_tabbar* pTabBar, dtk_uint32 tabIndex);
dtk_uint32 dtk_tabbar_get_active_tab_index(dtk_tabbar* pTabBar);
dtk_result dtk_tabbar_activate_tab(dtk_tabbar* pTabBar, dtk_uint32 tabIndex);
dtk_result dtk_tabbar_activate_next_tab(dtk_tabbar* pTabBar);   // Loops
dtk_result dtk_tabbar_activate_prev_tab(dtk_tabbar* pTabBar);   // Loops

dtk_result dtk_tabbar_set_tab_text(dtk_tabbar* pTabBar, dtk_uint32 tabIndex, const char* pTabText);
dtk_result dtk_tabbar_set_tab_tooltip(dtk_tabbar* pTabBar, dtk_uint32 tabIndex, const char* pTooltipText);

dtk_result dtk_tabbar_pin_tab(dtk_tabbar* pTabBar, dtk_uint32 tabIndex);
dtk_result dtk_tabbar_unpin_tab(dtk_tabbar* pTabBar, dtk_uint32 tabIndex);

// Performs a hit test against the tabs in the tab bar.
//
// Returns true if the point is over a tab; false otherwise.
dtk_bool32 dtk_tabbar_hit_test(dtk_tabbar* pTabBar, dtk_int32 x, dtk_int32 y, dtk_tabbar_hit_test_result* pResult);


// Enables auto sizing.
dtk_result dtk_tabbar_enable_auto_resize(dtk_tabbar* pTabBar);

// Disables auto sizing.
dtk_result dtk_tabbar_disable_auto_resize(dtk_tabbar* pTabBar);

// Determines whether or not auto sizing is enabled.
dtk_bool32 dtk_tabbar_is_auto_resize_enabled(const dtk_tabbar* pTabBar);

// Auto sizes the tab bar.
//
// The calculated size of the tab bar depends on it's flow. For dtk_tabbar_flow_left_to_right and dtk_tabbar_flow_right_to_left the
// width of the tab bar will be set to that of it's parent and the height will be sized based on the maximum height of the tabs. For
// other flows it will be the other way around.
//
// This API performs the actual resizing. Whenever a change is made that may change the size of the tabs this function will need to
// be called again.
dtk_result dtk_tabbar_auto_resize(dtk_tabbar* pTabBar);

// Same as dtk_tabbar_auto_resize(), but only performs the resize is auto-sizing is enabled. If auto-sizing is disabled, returns
// DTK_INVALID_OPERATION.
dtk_result dtk_tabbar_try_auto_resize(dtk_tabbar* pTabBar);


// Enables closing of tabs on middle click.
dtk_result dtk_tabbar_enable_close_on_middle_click(dtk_tabbar* pTabBar);

// Disables closing of tabs on middle click.
dtk_result dtk_tabbar_disable_close_on_middle_click(dtk_tabbar* pTabBar);

// Determines whether or not closing of tabs on middle click is enabled.
dtk_bool32 dtk_tabbar_is_close_on_middle_click_enabled(const dtk_tabbar* pTabBar);


// Transforms a point relative to a tab to relative to the main control.
dtk_result dtk_tabbar_transform_point_from_tab(const dtk_tabbar* pTabBar, dtk_uint32 tabIndex, dtk_int32* pX, dtk_int32* pY);
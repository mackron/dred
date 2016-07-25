// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_TABBAR   "dred.common.tabbar"

typedef struct dred_tabbar dred_tabbar;
#define DRED_TABBAR(a) ((dred_tabbar*)(a));

#define DRED_GUI_MAX_TAB_TEXT_LENGTH   256

typedef enum
{
    dred_tabbar_orientation_top,
    dred_tabbar_orientation_bottom,
    dred_tabbar_orientation_left,
    dred_tabbar_orientation_right
} dred_tabbar_orientation;

typedef struct dred_tab dred_tab;

typedef void (* dred_tabbar_on_measure_tab_proc)        (dred_tabbar* pTabBar, dred_tab* pTab, float* pWidthOut, float* pHeightOut);
typedef void (* dred_tabbar_on_paint_tab_proc)          (dred_tabbar* pTabBar, dred_tab* pTab, dred_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData);
typedef void (* dred_tabbar_on_tab_activated_proc)      (dred_tabbar* pTabBar, dred_tab* pTab, dred_tab* pOldActiveTab);
typedef void (* dred_tabbar_on_tab_deactivated_proc)    (dred_tabbar* pTabBar, dred_tab* pTab, dred_tab* pNewActiveTab);
typedef void (* dred_tabbar_on_tab_close_proc)          (dred_tabbar* pTabBar, dred_tab* pTab);
typedef void (* dred_tabbar_on_tab_mouse_button_up_proc)(dred_tabbar* pTabBar, dred_tab* pTab, int mouseButton, int mouseRelativePosX, int mouseRelativePosY, int stateFlags);


struct dred_tabbar
{
    // The base control.
    dred_control control;


    /// The orientation.
    dred_tabbar_orientation orientation;


    /// A pointer to the first tab.
    dred_tab* pFirstTab;

    /// A pointer to the last tab.
    dred_tab* pLastTab;


    /// A pointer to the hovered tab.
    dred_tab* pHoveredTab;

    /// A pointer to the active tab.
    dred_tab* pActiveTab;

    /// The tab whose close button is currently pressed, if any.
    dred_tab* pTabWithCloseButtonPressed;


    /// The default font to use for tab bar items.
    dred_gui_font* pFont;

    /// The default color to use for tab bar item text.
    dred_color tabTextColor;

    /// The default color to use for tab bar item text while active.
    dred_color tabTextColorActivated;

    /// The default color to use for tab bar item text while hovered.
    dred_color tabTextColorHovered;

    /// The default background color of tab bar items.
    dred_color tabBackgroundColor;

    /// The background color of tab bar items while hovered.
    dred_color tabBackgroundColorHovered;

    /// The background color of tab bar items while selected.
    dred_color tabBackbroundColorActivated;

    /// The padding to apply to the text of tabs.
    float tabPadding;

    /// The image to use for the close button.
    dred_gui_image* pCloseButtonImage;

    /// The padding to the left of the close button.
    float closeButtonPaddingLeft;

    /// The default color of the close button.
    dred_color closeButtonColorDefault;

    /// The color of the close button when the tab is hovered, but not the close button itself.
    dred_color closeButtonColorTabHovered;

    /// The color of the close button when it is hovered.
    dred_color closeButtonColorHovered;

    /// The color of the close button when it is pressed.
    dred_color closeButtonColorPressed;


    /// Whether or not auto-sizing is enabled. Disabled by default.
    bool isAutoSizeEnabled;

    /// Whether or not the close buttons are being shown.
    bool isShowingCloseButton;

    /// Whether or not close-on-middle-click is enabled.
    bool isCloseOnMiddleClickEnabled;

    /// Whether or not the close button is hovered.
    bool isCloseButtonHovered;


    /// The function to call when a tab needs to be measured.
    dred_tabbar_on_measure_tab_proc onMeasureTab;

    /// The function to call when a tab needs to be painted.
    dred_tabbar_on_paint_tab_proc onPaintTab;

    /// The function to call when a tab is activated.
    dred_tabbar_on_tab_activated_proc onTabActivated;

    /// The function to call when a tab is deactivated.
    dred_tabbar_on_tab_deactivated_proc onTabDeactivated;

    /// The function to call when a tab is closed via the close button.
    dred_tabbar_on_tab_close_proc onTabClose;

    // The function to call when a mouse button is released while over a tab.
    dred_tabbar_on_tab_mouse_button_up_proc onTabMouseButtonUp;


    /// The size of the extra data.
    size_t extraDataSize;

    /// A pointer to the extra data.
    char pExtraData[1];
};


///////////////////////////////////////////////////////////////////////////////
//
// Tab Bar
//
///////////////////////////////////////////////////////////////////////////////

/// Creates a new tab bar control.
bool dred_tabbar_init(dred_tabbar* pTabBar, dred_context* pDred, dred_control* pParent, dred_tabbar_orientation orientation);

/// Deletes the given tab bar control.
void dred_tabbar_uninit(dred_tabbar* pTabBar);


/// Retrieves the orientation of the given scrollbar.
dred_tabbar_orientation dred_tabbar_get_orientation(dred_tabbar* pTabBar);


/// Sets the default font to use for tabs.
void dred_tabbar_set_font(dred_tabbar* pTabBar, dred_gui_font* pFont);

/// Retrieves the default font to use for tabs.
dred_gui_font* dred_tabbar_get_font(dred_tabbar* pTabBar);

// Sets the color of the text to use on tabs.
void dred_tabbar_set_text_color(dred_tabbar* pTabBar, dred_color color);

// Retrieves the color of the text to use on tabs.
dred_color dred_tabbar_get_text_color(dred_tabbar* pTabBar);

// Sets the color of the text to use on active tabs.
void dred_tabbar_set_text_color_active(dred_tabbar* pTabBar, dred_color color);

// Sets the color of the text to use on hovered tabs.
void dred_tabbar_set_text_color_hovered(dred_tabbar* pTabBar, dred_color color);

/// Sets the image to use for close buttons.
void dred_tabbar_set_close_button_image(dred_tabbar* pTabBar, dred_gui_image* pImage);

/// Retrieves the image being used for the close buttons.
dred_gui_image* dred_tabbar_get_close_button_image(dred_tabbar* pTabBar);

// Sets the default color of the close button.
void dred_tabbar_set_close_button_color(dred_tabbar* pTabBar, dred_color color);

// Sets the padding to apply the the text of each tab.
void dred_tabbar_set_tab_padding(dred_tabbar* pTabBar, float padding);

// Retrieves the padding to apply to the text of each tab.
float dred_tabbar_get_tab_padding(dred_tabbar* pTabBar);

// Sets the padding to apply the the left of the close button.
void dred_tabbar_set_close_button_left_padding(dred_tabbar* pTabBar, float padding);

// Retrieves the padding to apply to the left of the close button.
float dred_tabbar_get_close_button_left_padding(dred_tabbar* pTabBar);

// Sets the default background color of tabs. This is the color of inactive tabs.
void dred_tabbar_set_tab_background_color(dred_tabbar* pTabBar, dred_color color);

// Retrieves the default background color of tabs while inactive.
dred_color dred_tabbar_get_tab_background_color(dred_tabbar* pTabBar);

// Sets the background color of tabs while hovered.
void dred_tabbar_set_tab_background_color_hovered(dred_tabbar* pTabBar, dred_color color);

// Retrieves the background color of tabs while hovered.
dred_color dred_tabbar_get_tab_background_color_hovered(dred_tabbar* pTabBar);

// Sets the background color of tabs while activated.
void dred_tabbar_set_tab_background_color_active(dred_tabbar* pTabBar, dred_color color);

// Retrieves the background color of tabs while activated.
dred_color dred_tabbar_get_tab_background_color_actived(dred_tabbar* pTabBar);


/// Sets the function to call when a tab needs to be measured.
void dred_tabbar_set_on_measure_tab(dred_tabbar* pTabBar, dred_tabbar_on_measure_tab_proc proc);

/// Sets the function to call when a tab needs to be painted.
void dred_tabbar_set_on_paint_tab(dred_tabbar* pTabBar, dred_tabbar_on_paint_tab_proc proc);

/// Sets the function to call when a tab is activated.
void dred_tabbar_set_on_tab_activated(dred_tabbar* pTabBar, dred_tabbar_on_tab_activated_proc proc);

/// Sets the function to call when a tab is deactivated.
void dred_tabbar_set_on_tab_deactivated(dred_tabbar* pTabBar, dred_tabbar_on_tab_deactivated_proc proc);

/// Sets the function to call when a tab is closed with the close button.
void dred_tabbar_set_on_tab_closed(dred_tabbar* pTabBar, dred_tabbar_on_tab_close_proc proc);

// Sets the function to call when a tab has a mouse button released on it.
void dred_tabbar_set_on_tab_mouse_button_up(dred_tabbar* pTabBar, dred_tabbar_on_tab_mouse_button_up_proc proc);


/// Measures the given tab.
void dred_tabbar_measure_tab(dred_tabbar* pTabBar, dred_tab* pTab, float* pWidthOut, float* pHeightOut);

/// Paints the given tab.
void dred_tabbar_paint_tab(dred_tabbar* pTabBar, dred_tab* pTab, dred_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData);


/// Sets the width or height of the tab bar to that of it's tabs based on it's orientation.
///
/// @remarks
///     If the orientation is set to top or bottom, the height will be resized and the width will be left alone. If the orientation
///     is left or right, the width will be resized and the height will be left alone.
///     @par
///     If there is no tab measuring callback set, this will do nothing.
void dred_tabbar_resize_by_tabs(dred_tabbar* pTabBar);

/// Enables auto-resizing based on tabs.
///
/// @remarks
///     This follows the same resizing rules as per dred_tabbar_resize_by_tabs().
///
/// @see
///     dred_tabbar_resize_by_tabs()
void dred_tabbar_enable_auto_size(dred_tabbar* pTabBar);

/// Disables auto-resizing based on tabs.
void dred_tabbar_disable_auto_size(dred_tabbar* pTabBar);

/// Determines whether or not auto-sizing is enabled.
bool dred_tabbar_is_auto_size_enabled(dred_tabbar* pTabBar);


// Retrieves a pointer to the first tab in the given tab bar.
dred_tab* dred_tabbar_get_first_tab(dred_tabbar* pTabBar);

// Retrieves a pointer to the last tab in the given tab bar.
dred_tab* dred_tabbar_get_last_tab(dred_tabbar* pTabBar);

// Retrieves a pointer to the next tab in the given tab bar.
dred_tab* dred_tabbar_get_next_tab(dred_tabbar* pTabBar, dred_tab* pTab);

// Retrieves a pointer to the previous tab in the given tab bar.
dred_tab* dred_tabbar_get_prev_tab(dred_tabbar* pTabBar, dred_tab* pTab);


/// Activates the given tab.
void dred_tabbar_activate_tab(dred_tabbar* pTabBar, dred_tab* pTab);

// Activates the tab to the right of the currently active tab, looping back to the start if necessary.
void dred_tabbar_activate_next_tab(dred_tabbar* pTabBar);

// Activates the tab to the left of the currently active tab, looping back to the end if necessary.
void dred_tabbar_activate_prev_tab(dred_tabbar* pTabBar);

/// Retrieves a pointer to the currently active tab.
dred_tab* dred_tabbar_get_active_tab(dred_tabbar* pTabBar);


/// Determines whether or not the given tab is in view.
bool dred_tabbar_is_tab_in_view(dred_tabbar* pTabBar, dred_tab* pTab);


/// Shows the close buttons on each tab.
void dred_tabbar_show_close_buttons(dred_tabbar* pTabBar);

/// Hides the close buttons on each tab.
void dred_tabbar_hide_close_buttons(dred_tabbar* pTabBar);

/// Enables the on_close event on middle click.
void dred_tabbar_enable_close_on_middle_click(dred_tabbar* pTabBar);

/// Disables the on_close event on middle click.
void dred_tabbar_disable_close_on_middle_click(dred_tabbar* pTabBar);

/// Determines whether or not close-on-middle-click is enabled.
bool dred_tabbar_is_close_on_middle_click_enabled(dred_tabbar* pTabBar);


/// Called when the mouse leave event needs to be processed for the given tab bar control.
void dred_tabbar_on_mouse_leave(dred_control* pControl);

/// Called when the mouse move event needs to be processed for the given tab bar control.
void dred_tabbar_on_mouse_move(dred_control* pControl, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when the mouse button down event needs to be processed for the given tab bar control.
void dred_tabbar_on_mouse_button_down(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when the mouse button up event needs to be processed for the given tab bar control.
void dred_tabbar_on_mouse_button_up(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when the paint event needs to be processed for the given tab control.
void dred_tabbar_on_paint(dred_control* pControl, dred_rect relativeClippingRect, void* pPaintData);




///////////////////////////////////////////////////////////////////////////////
//
// Tab
//
///////////////////////////////////////////////////////////////////////////////

/// Creates and appends a tab
dred_tab* dred_tabbar_create_and_append_tab(dred_tabbar* pTabBar, const char* text, dred_control* pControl);

/// Creates and prepends a tab.
dred_tab* dred_tabbar_create_and_prepend_tab(dred_tabbar* pTabBar, const char* text, dred_control* pControl);

/// Recursively deletes a tree view item.
void dred_tab_delete(dred_tab* pTab);

/// Retrieves the tab bar GUI element that owns the given item.
dred_tabbar* dred_tab_get_tab_bar(dred_tab* pTab);

/// Retrieves the size of the extra data associated with the given tree-view item.
size_t dred_tab_get_extra_data_size(dred_tab* pTab);

/// Retrieves a pointer to the extra data associated with the given tree-view item.
void* dred_tab_get_extra_data(dred_tab* pTab);


/// Sets the text of the given tab bar item.
void dred_tab_set_text(dred_tab* pTab, const char* text);

/// Retrieves the text of the given tab bar item.
const char* dred_tab_get_text(dred_tab* pTab);

// Sets the control associated with the given tab.
void dred_tab_set_control(dred_tab* pTab, dred_control* pControl);

// Retrieves the control associated with the given tab.
dred_control* dred_tab_get_control(dred_tab* pTab);


/// Retrieves a pointer to the next tab in the tab bar.
dred_tab* dred_tab_get_next_tab(dred_tab* pTab);

/// Retrieves a pointer to the previous tab in the tab bar.
dred_tab* dred_tab_get_prev_tab(dred_tab* pTab);


/// Moves the given tab to the front of the tab bar that owns it.
void dred_tab_move_to_front(dred_tab* pTab);

/// Determines whether or not the given tab is in view.
bool dred_tab_is_in_view(dred_tab* pTab);

/// Moves the given tab into view, if it's not already.
///
/// If the tab is out of view, it will be repositioned to the front of the tab bar.
void dred_tab_move_into_view(dred_tab* pTab);


// Retrieves the tab group the tab is attached to.
dred_tabgroup* dred_tab_get_tabgroup(dred_tab* pTab);
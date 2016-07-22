// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRGUI_MAX_TAB_TEXT_LENGTH   256

typedef enum
{
    drgui_tabbar_orientation_top,
    drgui_tabbar_orientation_bottom,
    drgui_tabbar_orientation_left,
    drgui_tabbar_orientation_right
} drgui_tabbar_orientation;

typedef struct drgui_tab drgui_tab;

typedef void (* drgui_tabbar_on_measure_tab_proc)        (drgui_element* pTBElement, drgui_tab* pTab, float* pWidthOut, float* pHeightOut);
typedef void (* drgui_tabbar_on_paint_tab_proc)          (drgui_element* pTBElement, drgui_tab* pTab, drgui_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData);
typedef void (* drgui_tabbar_on_tab_activated_proc)      (drgui_element* pTBElement, drgui_tab* pTab, drgui_tab* pOldActiveTab);
typedef void (* drgui_tabbar_on_tab_deactivated_proc)    (drgui_element* pTBElement, drgui_tab* pTab, drgui_tab* pNewActiveTab);
typedef void (* drgui_tabbar_on_tab_close_proc)          (drgui_element* pTBElement, drgui_tab* pTab);
typedef void (* drgui_tabbar_on_tab_mouse_button_up_proc)(drgui_element* pTBElement, drgui_tab* pTab, int mouseButton, int mouseRelativePosX, int mouseRelativePosY, int stateFlags);


///////////////////////////////////////////////////////////////////////////////
//
// Tab Bar
//
///////////////////////////////////////////////////////////////////////////////

/// Creates a new tab bar control.
drgui_element* drgui_create_tab_bar(drgui_context* pContext, drgui_element* pParent, drgui_tabbar_orientation orientation, size_t extraDataSize, const void* pExtraData);

/// Deletes the given tab bar control.
void drgui_delete_tab_bar(drgui_element* pTBElement);


/// Retrieves the size of the extra data associated with the scrollbar.
size_t drgui_tabbar_get_extra_data_size(drgui_element* pTBElement);

/// Retrieves a pointer to the extra data associated with the scrollbar.
void* drgui_tabbar_get_extra_data(drgui_element* pTBElement);

/// Retrieves the orientation of the given scrollbar.
drgui_tabbar_orientation drgui_tabbar_get_orientation(drgui_element* pTBElement);


/// Sets the default font to use for tabs.
void drgui_tabbar_set_font(drgui_element* pTBElement, drgui_font* pFont);

/// Retrieves the default font to use for tabs.
drgui_font* drgui_tabbar_get_font(drgui_element* pTBElement);

// Sets the color of the text to use on tabs.
void drgui_tabbar_set_text_color(drgui_element* pTBElement, drgui_color color);

// Retrieves the color of the text to use on tabs.
drgui_color drgui_tabbar_get_text_color(drgui_element* pTBElement);

// Sets the color of the text to use on active tabs.
void drgui_tabbar_set_text_color_active(drgui_element* pTBElement, drgui_color color);

// Sets the color of the text to use on hovered tabs.
void drgui_tabbar_set_text_color_hovered(drgui_element* pTBElement, drgui_color color);

/// Sets the image to use for close buttons.
void drgui_tabbar_set_close_button_image(drgui_element* pTBElement, drgui_image* pImage);

/// Retrieves the image being used for the close buttons.
drgui_image* drgui_tabbar_get_close_button_image(drgui_element* pTBElement);

// Sets the default color of the close button.
void drgui_tabbar_set_close_button_color(drgui_element* pTBElement, drgui_color color);

// Sets the padding to apply the the text of each tab.
void drgui_tabbar_set_tab_padding(drgui_element* pTBElement, float padding);

// Retrieves the padding to apply to the text of each tab.
float drgui_tabbar_get_tab_padding(drgui_element* pTBElement);

// Sets the padding to apply the the left of the close button.
void drgui_tabbar_set_close_button_left_padding(drgui_element* pTBElement, float padding);

// Retrieves the padding to apply to the left of the close button.
float drgui_tabbar_get_close_button_left_padding(drgui_element* pTBElement);

// Sets the default background color of tabs. This is the color of inactive tabs.
void drgui_tabbar_set_tab_background_color(drgui_element* pTBElement, drgui_color color);

// Retrieves the default background color of tabs while inactive.
drgui_color drgui_tabbar_get_tab_background_color(drgui_element* pTBElement);

// Sets the background color of tabs while hovered.
void drgui_tabbar_set_tab_background_color_hovered(drgui_element* pTBElement, drgui_color color);

// Retrieves the background color of tabs while hovered.
drgui_color drgui_tabbar_get_tab_background_color_hovered(drgui_element* pTBElement);

// Sets the background color of tabs while activated.
void drgui_tabbar_set_tab_background_color_active(drgui_element* pTBElement, drgui_color color);

// Retrieves the background color of tabs while activated.
drgui_color drgui_tabbar_get_tab_background_color_actived(drgui_element* pTBElement);


/// Sets the function to call when a tab needs to be measured.
void drgui_tabbar_set_on_measure_tab(drgui_element* pTBElement, drgui_tabbar_on_measure_tab_proc proc);

/// Sets the function to call when a tab needs to be painted.
void drgui_tabbar_set_on_paint_tab(drgui_element* pTBElement, drgui_tabbar_on_paint_tab_proc proc);

/// Sets the function to call when a tab is activated.
void drgui_tabbar_set_on_tab_activated(drgui_element* pTBElement, drgui_tabbar_on_tab_activated_proc proc);

/// Sets the function to call when a tab is deactivated.
void drgui_tabbar_set_on_tab_deactivated(drgui_element* pTBElement, drgui_tabbar_on_tab_deactivated_proc proc);

/// Sets the function to call when a tab is closed with the close button.
void drgui_tabbar_set_on_tab_closed(drgui_element* pTBElement, drgui_tabbar_on_tab_close_proc proc);

// Sets the function to call when a tab has a mouse button released on it.
void drgui_tabbar_set_on_tab_mouse_button_up(drgui_element* pTBElement, drgui_tabbar_on_tab_mouse_button_up_proc proc);


/// Measures the given tab.
void drgui_tabbar_measure_tab(drgui_element* pTBElement, drgui_tab* pTab, float* pWidthOut, float* pHeightOut);

/// Paints the given tab.
void drgui_tabbar_paint_tab(drgui_element* pTBElement, drgui_tab* pTab, drgui_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData);


/// Sets the width or height of the tab bar to that of it's tabs based on it's orientation.
///
/// @remarks
///     If the orientation is set to top or bottom, the height will be resized and the width will be left alone. If the orientation
///     is left or right, the width will be resized and the height will be left alone.
///     @par
///     If there is no tab measuring callback set, this will do nothing.
void drgui_tabbar_resize_by_tabs(drgui_element* pTBElement);

/// Enables auto-resizing based on tabs.
///
/// @remarks
///     This follows the same resizing rules as per drgui_tabbar_resize_by_tabs().
///
/// @see
///     drgui_tabbar_resize_by_tabs()
void drgui_tabbar_enable_auto_size(drgui_element* pTBElement);

/// Disables auto-resizing based on tabs.
void drgui_tabbar_disable_auto_size(drgui_element* pTBElement);

/// Determines whether or not auto-sizing is enabled.
bool drgui_tabbar_is_auto_size_enabled(drgui_element* pTBElement);


// Retrieves a pointer to the first tab in the given tab bar.
drgui_tab* drgui_tabbar_get_first_tab(drgui_element* pTBElement);

// Retrieves a pointer to the last tab in the given tab bar.
drgui_tab* drgui_tabbar_get_last_tab(drgui_element* pTBElement);

// Retrieves a pointer to the next tab in the given tab bar.
drgui_tab* drgui_tabbar_get_next_tab(drgui_element* pTBElement, drgui_tab* pTab);

// Retrieves a pointer to the previous tab in the given tab bar.
drgui_tab* drgui_tabbar_get_prev_tab(drgui_element* pTBElement, drgui_tab* pTab);


/// Activates the given tab.
void drgui_tabbar_activate_tab(drgui_element* pTBElement, drgui_tab* pTab);

// Activates the tab to the right of the currently active tab, looping back to the start if necessary.
void drgui_tabbar_activate_next_tab(drgui_element* pTBElement);

// Activates the tab to the left of the currently active tab, looping back to the end if necessary.
void drgui_tabbar_activate_prev_tab(drgui_element* pTBElement);

/// Retrieves a pointer to the currently active tab.
drgui_tab* drgui_tabbar_get_active_tab(drgui_element* pTBElement);


/// Determines whether or not the given tab is in view.
bool drgui_tabbar_is_tab_in_view(drgui_element* pTBElement, drgui_tab* pTab);


/// Shows the close buttons on each tab.
void drgui_tabbar_show_close_buttons(drgui_element* pTBElement);

/// Hides the close buttons on each tab.
void drgui_tabbar_hide_close_buttons(drgui_element* pTBElement);

/// Enables the on_close event on middle click.
void drgui_tabbar_enable_close_on_middle_click(drgui_element* pTBElement);

/// Disables the on_close event on middle click.
void drgui_tabbar_disable_close_on_middle_click(drgui_element* pTBElement);

/// Determines whether or not close-on-middle-click is enabled.
bool drgui_tabbar_is_close_on_middle_click_enabled(drgui_element* pTBElement);


/// Called when the mouse leave event needs to be processed for the given tab bar control.
void drgui_tabbar_on_mouse_leave(drgui_element* pTBElement);

/// Called when the mouse move event needs to be processed for the given tab bar control.
void drgui_tabbar_on_mouse_move(drgui_element* pTBElement, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when the mouse button down event needs to be processed for the given tab bar control.
void drgui_tabbar_on_mouse_button_down(drgui_element* pTBElement, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when the mouse button up event needs to be processed for the given tab bar control.
void drgui_tabbar_on_mouse_button_up(drgui_element* pTBElement, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when the paint event needs to be processed for the given tab control.
void drgui_tabbar_on_paint(drgui_element* pTBElement, drgui_rect relativeClippingRect, void* pPaintData);




///////////////////////////////////////////////////////////////////////////////
//
// Tab
//
///////////////////////////////////////////////////////////////////////////////

/// Creates and appends a tab
drgui_tab* drgui_tabbar_create_and_append_tab(drgui_element* pTBElement, const char* text, size_t extraDataSize, const void* pExtraData);

/// Creates and prepends a tab.
drgui_tab* drgui_tabbar_create_and_prepend_tab(drgui_element* pTBElement, const char* text, size_t extraDataSize, const void* pExtraData);

/// Recursively deletes a tree view item.
void drgui_tab_delete(drgui_tab* pTab);

/// Retrieves the tab bar GUI element that owns the given item.
drgui_element* drgui_tab_get_tab_bar_element(drgui_tab* pTab);

/// Retrieves the size of the extra data associated with the given tree-view item.
size_t drgui_tab_get_extra_data_size(drgui_tab* pTab);

/// Retrieves a pointer to the extra data associated with the given tree-view item.
void* drgui_tab_get_extra_data(drgui_tab* pTab);


/// Sets the text of the given tab bar item.
void drgui_tab_set_text(drgui_tab* pTab, const char* text);

/// Retrieves the text of the given tab bar item.
const char* drgui_tab_get_text(drgui_tab* pTab);


/// Retrieves a pointer to the next tab in the tab bar.
drgui_tab* drgui_tab_get_next_tab(drgui_tab* pTab);

/// Retrieves a pointer to the previous tab in the tab bar.
drgui_tab* drgui_tab_get_prev_tab(drgui_tab* pTab);


/// Moves the given tab to the front of the tab bar that owns it.
void drgui_tab_move_to_front(drgui_tab* pTab);

/// Determines whether or not the given tab is in view.
bool drgui_tab_is_in_view(drgui_tab* pTab);

/// Moves the given tab into view, if it's not already.
///
/// If the tab is out of view, it will be repositioned to the front of the tab bar.
void drgui_tab_move_into_view(drgui_tab* pTab);

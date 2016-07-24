// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef struct dred_tabbar_data dred_tabbar_data;
struct dred_tabbar_data
{
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

struct dred_tab
{
    /// The tab bar that owns the tab.
    dred_tabbar* pTabBar;

    /// A pointer to the next tab in the tab bar.
    dred_tab* pNextTab;

    /// A pointer to the previous tab in the tab bar.
    dred_tab* pPrevTab;


    /// The tab bar's text.
    char text[DRED_GUI_MAX_TAB_TEXT_LENGTH];

    // The control that's associated with the tab. When the tab is activated, this control is made visible.
    dred_control* pControl;


    /// The size of the extra data.
    size_t extraDataSize;

    /// A pointer to the extra data buffer.
    char pExtraData[1];
};


///////////////////////////////////////////////////////////////////////////////
//
// Tab
//
///////////////////////////////////////////////////////////////////////////////

/// Default implementation of the item measure event.
DRED_GUI_PRIVATE void dred_tabbar_on_measure_tab_default(dred_tabbar* pTabBar, dred_tab* pTab, float* pWidthOut, float* pHeightOut);

/// Paints the given menu item.
DRED_GUI_PRIVATE void dred_tabbar_on_paint_tab_default(dred_tabbar* pTabBar, dred_tab* pTab, dred_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData);

/// Finds the tab sitting under the given point, if any.
DRED_GUI_PRIVATE dred_tab* dred_tabbar_find_tab_under_point(dred_tabbar* pTabBar, float relativePosX, float relativePosY, bool* pIsOverCloseButtonOut);

dred_tabbar* dred_tabbar_create(dred_context* pDred, dred_control* pParent, dred_tabbar_orientation orientation, size_t extraDataSize, const void* pExtraData)
{
    dred_tabbar* pTabBar = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_TABBAR, sizeof(dred_tabbar_data) + extraDataSize);
    if (pTabBar == NULL) {
        return NULL;
    }

    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    assert(pTB != NULL);

    pTB->orientation                 = orientation;
    pTB->pFirstTab                   = NULL;
    pTB->pLastTab                    = NULL;
    pTB->pHoveredTab                 = NULL;
    pTB->pActiveTab                  = NULL;
    pTB->pTabWithCloseButtonPressed  = NULL;

    pTB->pFont                       = NULL;
    pTB->tabTextColor                = dred_rgb(224, 224, 224);
    pTB->tabTextColorActivated       = dred_rgb(224, 224, 224);
    pTB->tabTextColorHovered         = dred_rgb(224, 224, 224);
    pTB->tabBackgroundColor          = dred_rgb(58, 58, 58);
    pTB->tabBackgroundColorHovered   = dred_rgb(16, 92, 160);
    pTB->tabBackbroundColorActivated = dred_rgb(32, 128, 192); //dred_rgb(80, 80, 80);
    pTB->tabPadding                  = 4;
    pTB->pCloseButtonImage           = NULL;
    pTB->closeButtonPaddingLeft      = 6;
    pTB->closeButtonColorDefault     = pTB->tabBackgroundColor;
    pTB->closeButtonColorTabHovered  = dred_rgb(192, 192, 192);
    pTB->closeButtonColorHovered     = dred_rgb(255, 96, 96);
    pTB->closeButtonColorPressed     = dred_rgb(192, 32, 32);
    pTB->isAutoSizeEnabled           = false;
    pTB->isShowingCloseButton        = false;
    pTB->isCloseOnMiddleClickEnabled = false;
    pTB->isCloseButtonHovered        = false;

    pTB->onMeasureTab                = dred_tabbar_on_measure_tab_default;
    pTB->onPaintTab                  = dred_tabbar_on_paint_tab_default;
    pTB->onTabActivated              = NULL;
    pTB->onTabDeactivated            = NULL;
    pTB->onTabClose                  = NULL;


    pTB->extraDataSize = extraDataSize;
    if (pExtraData != NULL) {
        memcpy(pTB->pExtraData, pExtraData, extraDataSize);
    }


    // Event handlers.
    dred_control_set_on_mouse_leave(pTabBar, dred_tabbar_on_mouse_leave);
    dred_control_set_on_mouse_move(pTabBar, dred_tabbar_on_mouse_move);
    dred_control_set_on_mouse_button_down(pTabBar, dred_tabbar_on_mouse_button_down);
    dred_control_set_on_mouse_button_up(pTabBar, dred_tabbar_on_mouse_button_up);
    dred_control_set_on_paint(pTabBar, dred_tabbar_on_paint);

    return pTabBar;
}

void dred_tabbar_delete(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    while (pTB->pFirstTab != NULL) {
        dred_tab_delete(pTB->pFirstTab);
    }


    dred_control_delete(pTabBar);
}


size_t dred_tabbar_get_extra_data_size(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return 0;
    }

    return pTB->extraDataSize;
}

void* dred_tabbar_get_extra_data(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pExtraData;
}

dred_tabbar_orientation dred_tabbar_get_orientation(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return dred_tabbar_orientation_top;
    }

    return pTB->orientation;
}


void dred_tabbar_set_font(dred_tabbar* pTabBar, dred_gui_font* pFont)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->pFont = pFont;

    // A change in font may have changed the size of the tabbar.
    if (pTB->isAutoSizeEnabled) {
        dred_tabbar_resize_by_tabs(pTabBar);
    }

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

dred_gui_font* dred_tabbar_get_font(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pFont;
}


void dred_tabbar_set_text_color(dred_tabbar* pTabBar, dred_color color)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->tabTextColor = color;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

dred_color dred_tabbar_get_text_color(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTB->tabTextColor;
}

void dred_tabbar_set_text_color_active(dred_tabbar* pTabBar, dred_color color)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->tabTextColorActivated = color;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

void dred_tabbar_set_text_color_hovered(dred_tabbar* pTabBar, dred_color color)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->tabTextColorHovered = color;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}


void dred_tabbar_set_close_button_image(dred_tabbar* pTabBar, dred_gui_image* pImage)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->pCloseButtonImage = pImage;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

dred_gui_image* dred_tabbar_get_close_button_image(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pCloseButtonImage;
}

void dred_tabbar_set_close_button_color(dred_tabbar* pTabBar, dred_color color)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->closeButtonColorDefault = color;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}


void dred_tabbar_set_tab_padding(dred_tabbar* pTabBar, float padding)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->tabPadding = padding;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

float dred_tabbar_get_tab_padding(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return 0;
    }

    return pTB->tabPadding;
}

void dred_tabbar_set_close_button_left_padding(dred_tabbar* pTabBar, float padding)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->closeButtonPaddingLeft = padding;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

float dred_tabbar_get_close_button_left_padding(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return 0;
    }

    return pTB->closeButtonPaddingLeft;
}


void dred_tabbar_set_tab_background_color(dred_tabbar* pTabBar, dred_color color)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->tabBackgroundColor = color;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

dred_color dred_tabbar_get_tab_background_color(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTB->tabBackgroundColor;
}

void dred_tabbar_set_tab_background_color_hovered(dred_tabbar* pTabBar, dred_color color)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->tabBackgroundColorHovered = color;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

dred_color dred_tabbar_get_tab_background_color_hovered(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTB->tabBackgroundColorHovered;
}

void dred_tabbar_set_tab_background_color_active(dred_tabbar* pTabBar, dred_color color)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->tabBackbroundColorActivated = color;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

dred_color dred_tabbar_get_tab_background_color_actived(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTB->tabBackbroundColorActivated;
}


void dred_tabbar_set_on_measure_tab(dred_tabbar* pTabBar, dred_tabbar_on_measure_tab_proc proc)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->onMeasureTab = proc;
}

void dred_tabbar_set_on_paint_tab(dred_tabbar* pTabBar, dred_tabbar_on_paint_tab_proc proc)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->onPaintTab = proc;
}

void dred_tabbar_set_on_tab_activated(dred_tabbar* pTabBar, dred_tabbar_on_tab_activated_proc proc)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->onTabActivated = proc;
}

void dred_tabbar_set_on_tab_deactivated(dred_tabbar* pTabBar, dred_tabbar_on_tab_deactivated_proc proc)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->onTabDeactivated = proc;
}

void dred_tabbar_set_on_tab_closed(dred_tabbar* pTabBar, dred_tabbar_on_tab_close_proc proc)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->onTabClose = proc;
}

void dred_tabbar_set_on_tab_mouse_button_up(dred_tabbar* pTabBar, dred_tabbar_on_tab_mouse_button_up_proc proc)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->onTabMouseButtonUp = proc;
}


void dred_tabbar_measure_tab(dred_tabbar* pTabBar, dred_tab* pTab, float* pWidthOut, float* pHeightOut)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    if (pTB->onMeasureTab) {
        pTB->onMeasureTab(pTabBar, pTab, pWidthOut, pHeightOut);
    }
}

void dred_tabbar_paint_tab(dred_tabbar* pTabBar, dred_tab* pTab, dred_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    if (pTB->onPaintTab) {
        pTB->onPaintTab(pTabBar, pTab, relativeClippingRect, offsetX, offsetY, width, height, pPaintData);
    }
}


void dred_tabbar_resize_by_tabs(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    if (pTB->onMeasureTab == NULL) {
        return;
    }

    float maxWidth  = 0;
    float maxHeight = 0;
    if (pTB->pFirstTab == NULL) {
        // There are no tabs. Set initial size based on the line height of the font.
        dred_gui_font_metrics fontMetrics;
        if (dred_gui_get_font_metrics(pTB->pFont, &fontMetrics)) {
            if (pTB->orientation == dred_tabbar_orientation_top || pTB->orientation == dred_tabbar_orientation_bottom) {
                maxHeight = fontMetrics.lineHeight + (pTB->tabPadding*2);
            } else {
                maxWidth = fontMetrics.lineHeight + (pTB->tabPadding*2);
            }
        }
    } else {
        for (dred_tab* pTab = pTB->pFirstTab; pTab != NULL; pTab = pTab->pNextTab) {
            float tabWidth  = 0;
            float tabHeight = 0;
            dred_tabbar_measure_tab(pTabBar, pTab, &tabWidth, &tabHeight);

            maxWidth  = (tabWidth  > maxWidth)  ? tabWidth  : maxWidth;
            maxHeight = (tabHeight > maxHeight) ? tabHeight : maxHeight;
        }
    }

    


    if (pTB->orientation == dred_tabbar_orientation_top || pTB->orientation == dred_tabbar_orientation_bottom) {
        dred_control_set_size(pTabBar, dred_control_get_width(pTabBar), maxHeight);
    } else {
        dred_control_set_size(pTabBar, maxWidth, dred_control_get_height(pTabBar));
    }
}

void dred_tabbar_enable_auto_size(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->isAutoSizeEnabled = true;
}

void dred_tabbar_disable_auto_size(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->isAutoSizeEnabled = false;
}

bool dred_tabbar_is_auto_size_enabled(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return false;
    }

    return pTB->isAutoSizeEnabled;
}


dred_tab* dred_tabbar_get_first_tab(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pFirstTab;
}

dred_tab* dred_tabbar_get_last_tab(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pLastTab;
}

dred_tab* dred_tabbar_get_next_tab(dred_tabbar* pTabBar, dred_tab* pTab)
{
    (void)pTabBar;
    return dred_tab_get_next_tab(pTab);
}

dred_tab* dred_tabbar_get_prev_tab(dred_tabbar* pTabBar, dred_tab* pTab)
{
    (void)pTabBar;
    return dred_tab_get_prev_tab(pTab);
}


void dred_tabbar_activate_tab(dred_tabbar* pTabBar, dred_tab* pTab)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    dred_tab* pOldActiveTab = pTB->pActiveTab;
    dred_tab* pNewActiveTab = pTab;

    if (pOldActiveTab == pNewActiveTab) {
        return;     // The tab is already active - nothing to do.
    }


    pTB->pActiveTab = pNewActiveTab;

    if (pTB->onTabDeactivated && pOldActiveTab != NULL) {
        pTB->onTabDeactivated(pTabBar, pOldActiveTab, pNewActiveTab);
    }

    if (pTB->onTabActivated && pNewActiveTab != NULL) {
        pTB->onTabActivated(pTabBar, pNewActiveTab, pOldActiveTab);
    }


    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

void dred_tabbar_activate_next_tab(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    if (pTB->pActiveTab == NULL) {
        dred_tabbar_activate_tab(pTabBar, pTB->pFirstTab);
        return;
    }


    dred_tab* pNextTab = pTB->pActiveTab->pNextTab;
    if (pNextTab == NULL) {
        pNextTab = pTB->pFirstTab;
    }

    dred_tabbar_activate_tab(pTabBar, pNextTab);
}

void dred_tabbar_activate_prev_tab(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    if (pTB->pActiveTab == NULL) {
        dred_tabbar_activate_tab(pTabBar, pTB->pLastTab);
        return;
    }


    dred_tab* pPrevTab = pTB->pActiveTab->pPrevTab;
    if (pPrevTab == NULL) {
        pPrevTab = pTB->pLastTab;
    }

    dred_tabbar_activate_tab(pTabBar, pPrevTab);
}

dred_tab* dred_tabbar_get_active_tab(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pActiveTab;
}


bool dred_tabbar_is_tab_in_view(dred_tabbar* pTabBar, dred_tab* pTabIn)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return false;
    }

    float tabbarWidth  = 0;
    float tabbarHeight = 0;
    dred_control_get_size(pTabBar, &tabbarWidth, &tabbarHeight);


    // Each tab.
    float runningPosX = 0;
    float runningPosY = 0;
    for (dred_tab* pTab = pTB->pFirstTab; pTab != NULL; pTab = pTab->pNextTab)
    {
        float tabWidth  = 0;
        float tabHeight = 0;
        dred_tabbar_measure_tab(pTabBar, pTab, &tabWidth, &tabHeight);

        if (pTab == pTabIn) {
            return runningPosX + tabWidth <= tabbarWidth && runningPosY + tabHeight <= tabbarHeight;
        }


        if (pTB->orientation == dred_tabbar_orientation_top || pTB->orientation == dred_tabbar_orientation_bottom) {
            runningPosX += tabWidth;
        } else {
            runningPosY += tabHeight;
        }
    }

    return false;
}


void dred_tabbar_show_close_buttons(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->isShowingCloseButton = true;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

void dred_tabbar_hide_close_buttons(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->isShowingCloseButton = false;

    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}


void dred_tabbar_enable_close_on_middle_click(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->isCloseOnMiddleClickEnabled = true;
}

void dred_tabbar_disable_close_on_middle_click(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    pTB->isCloseOnMiddleClickEnabled = false;
}

bool dred_tabbar_is_close_on_middle_click_enabled(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return false;
    }

    return pTB->isCloseOnMiddleClickEnabled;
}


void dred_tabbar_on_mouse_leave(dred_tabbar* pTabBar)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    if (pTB->pHoveredTab != NULL)
    {
        pTB->pHoveredTab = NULL;
        pTB->isCloseButtonHovered = false;

        if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
            dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
        }
    }
}

void dred_tabbar_on_mouse_move(dred_tabbar* pTabBar, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    bool isCloseButtonHovered = false;

    dred_tab* pOldHoveredTab = pTB->pHoveredTab;
    dred_tab* pNewHoveredTab = dred_tabbar_find_tab_under_point(pTabBar, (float)relativeMousePosX, (float)relativeMousePosY, &isCloseButtonHovered);

    if (pOldHoveredTab != pNewHoveredTab || pTB->isCloseButtonHovered != isCloseButtonHovered)
    {
        pTB->pHoveredTab = pNewHoveredTab;
        pTB->isCloseButtonHovered = isCloseButtonHovered;

        if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
            dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
        }
    }
}

void dred_tabbar_on_mouse_button_down(dred_tabbar* pTabBar, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT || mouseButton == DRED_GUI_MOUSE_BUTTON_RIGHT)
    {
        bool isOverCloseButton = false;

        dred_tab* pOldActiveTab = pTB->pActiveTab;
        dred_tab* pNewActiveTab = dred_tabbar_find_tab_under_point(pTabBar, (float)relativeMousePosX, (float)relativeMousePosY, &isOverCloseButton);

        if (pNewActiveTab != NULL && pOldActiveTab != pNewActiveTab && !isOverCloseButton) {
            dred_tabbar_activate_tab(pTabBar, pNewActiveTab);
        }

        if (isOverCloseButton && mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
            pTB->pTabWithCloseButtonPressed = pNewActiveTab;

            if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
                dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
            }
        }
    }
    else if (mouseButton == DRED_GUI_MOUSE_BUTTON_MIDDLE)
    {
        if (pTB->isCloseOnMiddleClickEnabled)
        {
            dred_tab* pHoveredTab = dred_tabbar_find_tab_under_point(pTabBar, (float)relativeMousePosX, (float)relativeMousePosY, NULL);
            if (pHoveredTab != NULL) {
                if (pTB->onTabClose) {
                    pTB->onTabClose(pTabBar, pHoveredTab);
                }
            }
        }
    }
}

void dred_tabbar_on_mouse_button_up(dred_tabbar* pTabBar, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }


    bool releasedOverCloseButton = false;
    dred_tab* pTabUnderMouse = dred_tabbar_find_tab_under_point(pTabBar, (float)relativeMousePosX, (float)relativeMousePosY, &releasedOverCloseButton);

    if (pTB->pTabWithCloseButtonPressed != NULL && mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT)
    {
        if (releasedOverCloseButton && pTabUnderMouse == pTB->pTabWithCloseButtonPressed) {
            if (pTB->onTabClose) {
                pTB->onTabClose(pTabBar, pTB->pTabWithCloseButtonPressed);
            }
        }


        pTB->pTabWithCloseButtonPressed = NULL;

        if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
            dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
        }
    }
    else
    {
        if (!releasedOverCloseButton && pTB->onTabMouseButtonUp) {
            // TODO: Improve this by passing the mouse position relative to the tab. Currently it is relative to the tab BAR. Can have
            // the dred_tabbar_find_tab_under_point() function return the position relative to the tab.
            pTB->onTabMouseButtonUp(pTabBar, pTabUnderMouse, mouseButton, relativeMousePosX, relativeMousePosY, stateFlags);
        }
    }
}

void dred_tabbar_on_paint(dred_tabbar* pTabBar, dred_rect relativeClippingRect, void* pPaintData)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }


    float tabbarWidth  = 0;
    float tabbarHeight = 0;
    dred_control_get_size(pTabBar, &tabbarWidth, &tabbarHeight);


    // Each tab.
    float runningPosX = 0;
    float runningPosY = 0;
    for (dred_tab* pTab = pTB->pFirstTab; pTab != NULL; pTab = pTab->pNextTab)
    {
        float tabWidth  = 0;
        float tabHeight = 0;
        dred_tabbar_measure_tab(pTabBar, pTab, &tabWidth, &tabHeight);

        // If a part of the tab is out of bounds, stop drawing.
        if (runningPosX > tabbarWidth || runningPosY > tabbarHeight) {
            break;
        }


        dred_tabbar_paint_tab(pTabBar, pTab, relativeClippingRect, runningPosX, runningPosY, tabWidth, tabHeight, pPaintData);

        // After painting the tab, there may be a region of the background that was not drawn by the tab painting callback. We'll need to
        // draw that here.
        if (pTB->orientation == dred_tabbar_orientation_top || pTB->orientation == dred_tabbar_orientation_bottom) {
            dred_control_draw_rect(pTabBar, dred_make_rect(runningPosX, runningPosY + tabHeight, tabbarWidth, tabbarHeight), pTB->tabBackgroundColor, pPaintData);
        } else {
            dred_control_draw_rect(pTabBar, dred_make_rect(runningPosX + tabWidth, runningPosY, tabbarWidth, runningPosY + tabHeight), pTB->tabBackgroundColor, pPaintData);
        }



        if (pTB->orientation == dred_tabbar_orientation_top || pTB->orientation == dred_tabbar_orientation_bottom) {
            runningPosX += tabWidth;
        } else {
            runningPosY += tabHeight;
        }
    }


    // Background. We just draw a quad around the region that is not covered by items.
    dred_control_draw_rect(pTabBar, dred_make_rect(runningPosX, runningPosY, tabbarWidth, tabbarHeight), pTB->tabBackgroundColor, pPaintData);
}


DRED_GUI_PRIVATE void dred_tabbar_on_measure_tab_default(dred_tabbar* pTabBar, dred_tab* pTab, float* pWidthOut, float* pHeightOut)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    float textWidth  = 0;
    float textHeight = 0;

    if (pTab != NULL) {
        dred_gui_measure_string(pTB->pFont, pTab->text, strlen(pTab->text), &textWidth, &textHeight);
    }


    float closeButtonWidth  = 0;
    if (pTB->isShowingCloseButton && pTB->pCloseButtonImage != NULL) {
        unsigned int closeImageWidth;
        dred_gui_get_image_size(pTB->pCloseButtonImage, &closeImageWidth, NULL);

        closeButtonWidth  = closeImageWidth + pTB->closeButtonPaddingLeft;
    }


    if (pWidthOut) {
        *pWidthOut = textWidth + closeButtonWidth + pTB->tabPadding*2;
    }
    if (pHeightOut) {
        *pHeightOut = textHeight + pTB->tabPadding*2;
    }
}

DRED_GUI_PRIVATE void dred_tabbar_on_paint_tab_default(dred_tabbar* pTabBar, dred_tab* pTab, dred_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData)
{
    (void)relativeClippingRect;

    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return;
    }

    // Background.
    dred_color bgcolor = pTB->tabBackgroundColor;
    dred_color closeButtonColor = pTB->closeButtonColorDefault;
    dred_color textColor = pTB->tabTextColor;

    if (pTB->pHoveredTab == pTab) {
        bgcolor = pTB->tabBackgroundColorHovered;
        closeButtonColor = pTB->closeButtonColorTabHovered;
        textColor = pTB->tabTextColorHovered;
    }
    if (pTB->pActiveTab == pTab) {
        bgcolor = pTB->tabBackbroundColorActivated;
        closeButtonColor = pTB->closeButtonColorTabHovered;
        textColor = pTB->tabTextColorActivated;
    }

    if (pTB->pHoveredTab == pTab && pTB->isCloseButtonHovered) {
        closeButtonColor = pTB->closeButtonColorHovered;

        if (pTB->pTabWithCloseButtonPressed == pTB->pHoveredTab) {
            closeButtonColor = pTB->closeButtonColorPressed;
        }
    }

    dred_control_draw_rect_outline(pTabBar, dred_make_rect(offsetX, offsetY, offsetX + width, offsetY + height), bgcolor, pTB->tabPadding, pPaintData);


    // Text.
    float textPosX = offsetX + pTB->tabPadding;
    float textPosY = offsetY + pTB->tabPadding;
    if (pTab != NULL) {
        dred_control_draw_text(pTabBar, pTB->pFont, pTab->text, (int)strlen(pTab->text), textPosX, textPosY, textColor, bgcolor, pPaintData);
    }


    // Close button.
    if (pTB->isShowingCloseButton && pTB->pCloseButtonImage != NULL)
    {
        float textWidth  = 0;
        float textHeight = 0;
        if (pTab != NULL) {
            dred_gui_measure_string(pTB->pFont, pTab->text, strlen(pTab->text), &textWidth, &textHeight);
        }

        float closeButtonPosX = textPosX + textWidth + pTB->closeButtonPaddingLeft;
        float closeButtonPosY = textPosY;

        unsigned int iconWidth;
        unsigned int iconHeight;
        dred_gui_get_image_size(pTB->pCloseButtonImage, &iconWidth, &iconHeight);

        dred_gui_draw_image_args args;
        args.dstX            = closeButtonPosX;
        args.dstY            = closeButtonPosY;
        args.dstWidth        = (float)iconWidth;
        args.dstHeight       = (float)iconHeight;
        args.srcX            = 0;
        args.srcY            = 0;
        args.srcWidth        = (float)iconWidth;
        args.srcHeight       = (float)iconHeight;
        args.dstBoundsX      = args.dstX;
        args.dstBoundsY      = args.dstY;
        args.dstBoundsWidth  = (float)iconWidth;
        args.dstBoundsHeight = height - (pTB->tabPadding*2);
        args.foregroundTint  = closeButtonColor;
        args.backgroundColor = bgcolor;
        args.boundsColor     = bgcolor;
        args.options         = DRED_GUI_IMAGE_DRAW_BACKGROUND | DRED_GUI_IMAGE_DRAW_BOUNDS | DRED_GUI_IMAGE_CLIP_BOUNDS | DRED_GUI_IMAGE_ALIGN_CENTER;
        dred_control_draw_image(pTabBar, pTB->pCloseButtonImage, &args, pPaintData);


        /// Space between the text and the padding.
        dred_control_draw_rect(pTabBar, dred_make_rect(textPosX + textWidth, textPosY, closeButtonPosX, textPosY + textHeight), bgcolor, pPaintData);
    }
}

DRED_GUI_PRIVATE dred_tab* dred_tabbar_find_tab_under_point(dred_tabbar* pTabBar, float relativePosX, float relativePosY, bool* pIsOverCloseButtonOut)
{
    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    if (pTB == NULL) {
        return NULL;
    }

    unsigned int closeButtonWidth;
    unsigned int closeButtonHeight;
    dred_gui_get_image_size(pTB->pCloseButtonImage, &closeButtonWidth, &closeButtonHeight);

    float runningPosX = 0;
    float runningPosY = 0;
    for (dred_tab* pTab = pTB->pFirstTab; pTab != NULL; pTab = pTab->pNextTab)
    {
        float tabWidth  = 0;
        float tabHeight = 0;
        dred_tabbar_measure_tab(pTabBar, pTab, &tabWidth, &tabHeight);

        if (relativePosX >= runningPosX && relativePosX < runningPosX + tabWidth && relativePosY >= runningPosY && relativePosY < runningPosY + tabHeight)
        {
            if (pIsOverCloseButtonOut)
            {
                // The close button is in the center, vertically.
                dred_rect closeButtonRect;
                closeButtonRect.left   = runningPosX + tabWidth - (pTB->tabPadding + closeButtonWidth);
                closeButtonRect.right  = closeButtonRect.left + closeButtonWidth;
                closeButtonRect.top    = runningPosY + (tabHeight - (pTB->tabPadding + closeButtonHeight))/2;
                closeButtonRect.bottom = closeButtonRect.top + closeButtonHeight;

                if (pTB->isShowingCloseButton && dred_rect_contains_point(closeButtonRect, relativePosX, relativePosY)) {
                    *pIsOverCloseButtonOut = true;
                } else {
                    *pIsOverCloseButtonOut = false;
                }
            }

            return pTab;
        }

        if (pTB->orientation == dred_tabbar_orientation_top || pTB->orientation == dred_tabbar_orientation_bottom) {
            runningPosX += tabWidth;
        } else {
            runningPosY += tabHeight;
        }
    }


    if (pIsOverCloseButtonOut) {
        *pIsOverCloseButtonOut = false;
    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// Tab
//
///////////////////////////////////////////////////////////////////////////////

/// Appends the given tab to the given tab bar.
DRED_GUI_PRIVATE void tab_append(dred_tab* pTab, dred_tabbar* pTabBar);

/// Prepends the given tab to the given tab bar.
DRED_GUI_PRIVATE void tab_prepend(dred_tab* pTab, dred_tabbar* pTabBar);

/// Detaches the given tab bar from it's tab bar element's hierarchy.
///
/// @remarks
///     This does not deactivate the tab or what - it only detaches the tab from the hierarchy.
DRED_GUI_PRIVATE void tab_detach_from_hierarchy(dred_tab* pTab);

/// Detaches the given tab bar from it's tab bar element.
DRED_GUI_PRIVATE void tab_detach(dred_tab* pTab);

DRED_GUI_PRIVATE dred_tab* tb_create_tab(dred_tabbar* pTabBar, const char* text, dred_control* pControl)
{
    if (pTabBar == NULL) {
        return NULL;
    }

    dred_tab* pTab = (dred_tab*)malloc(sizeof(*pTab));
    if (pTab == NULL) {
        return NULL;
    }

    pTab->pTabBar  = NULL;
    pTab->pNextTab = NULL;
    pTab->pPrevTab = NULL;
    pTab->text[0]  = '\0';
    pTab->pControl = pControl;

    if (text != NULL) {
        strncpy_s(pTab->text, sizeof(pTab->text), text, (size_t)-1); // -1 = _TRUNCATE
    }

    return pTab;
}

dred_tab* dred_tabbar_create_and_append_tab(dred_tabbar* pTabBar, const char* text, dred_control* pControl)
{
    dred_tab* pTab = (dred_tab*)tb_create_tab(pTabBar, text, pControl);
    if (pTab == NULL) {
        return NULL;
    }

    tab_append(pTab, pTabBar);

    return pTab;
}

dred_tab* dred_tabbar_create_and_prepend_tab(dred_tabbar* pTabBar, const char* text, dred_control* pControl)
{
    dred_tab* pTab = (dred_tab*)tb_create_tab(pTabBar, text, pControl);
    if (pTab == NULL) {
        return NULL;
    }

    tab_prepend(pTab, pTabBar);

    return pTab;
}

void dred_tab_delete(dred_tab* pTab)
{
    if (pTab == NULL) {
        return;
    }

    tab_detach(pTab);
    free(pTab);
}

dred_control* dred_tab_get_tab_bar_element(dred_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    return pTab->pTabBar;
}

size_t dred_tab_get_extra_data_size(dred_tab* pTab)
{
    if (pTab == NULL) {
        return 0;
    }

    return pTab->extraDataSize;
}

void* dred_tab_get_extra_data(dred_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    return pTab->pExtraData;
}


void dred_tab_set_text(dred_tab* pTab, const char* text)
{
    if (pTab == NULL) {
        return;
    }

    if (text != NULL) {
        strncpy_s(pTab->text, sizeof(pTab->text), text, (size_t)-1); // -1 = _TRUNCATE
    } else {
        pTab->text[0] = '\0';
    }

    // The content of the menu has changed so we'll need to schedule a redraw.
    if (dred_gui_is_auto_dirty_enabled(pTab->pTabBar->pGUI)) {
        dred_control_dirty(pTab->pTabBar, dred_control_get_local_rect(pTab->pTabBar));
    }
}

const char* dred_tab_get_text(dred_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    return pTab->text;
}


void dred_tab_set_control(dred_tab* pTab, dred_control* pControl)
{
    if (pTab == NULL) {
        return;
    }

    pTab->pControl = pControl;
}

dred_control* dred_tab_get_control(dred_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    return pTab->pControl;
}


dred_tab* dred_tab_get_next_tab(dred_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    return pTab->pNextTab;
}

dred_tab* dred_tab_get_prev_tab(dred_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    return pTab->pPrevTab;
}


void dred_tab_move_to_front(dred_tab* pTab)
{
    if (pTab == NULL) {
        return;
    }

    dred_tabbar* pTabBar = pTab->pTabBar;

    tab_detach_from_hierarchy(pTab);
    tab_prepend(pTab, pTabBar);
}

bool dred_tab_is_in_view(dred_tab* pTab)
{
    if (pTab == NULL) {
        return false;
    }

    return dred_tabbar_is_tab_in_view(pTab->pTabBar, pTab);
}

void dred_tab_move_into_view(dred_tab* pTab)
{
    if (!dred_tab_is_in_view(pTab)) {
        dred_tab_move_to_front(pTab);
    }
}


dred_control* dred_tab_get_tabgroup(dred_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    // The tab group is the parent of the tab bar.
    return dred_control_get_parent(pTab->pTabBar);
}




DRED_GUI_PRIVATE void tab_append(dred_tab* pTab, dred_tabbar* pTabBar)
{
    if (pTab == NULL || pTabBar == NULL) {
        return;
    }

    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    assert(pTB != NULL);

    pTab->pTabBar = pTabBar;
    if (pTB->pFirstTab == NULL)
    {
        assert(pTB->pLastTab == NULL);

        pTB->pFirstTab = pTab;
        pTB->pLastTab  = pTab;
    }
    else
    {
        assert(pTB->pLastTab != NULL);

        pTab->pPrevTab = pTB->pLastTab;

        pTB->pLastTab->pNextTab = pTab;
        pTB->pLastTab = pTab;
    }


    if (pTB->isAutoSizeEnabled) {
        dred_tabbar_resize_by_tabs(pTabBar);
    }

    // The content of the menu has changed so we'll need to schedule a redraw.
    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

DRED_GUI_PRIVATE void tab_prepend(dred_tab* pTab, dred_tabbar* pTabBar)
{
    if (pTab == NULL || pTabBar == NULL) {
        return;
    }

    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    assert(pTB != NULL);

    pTab->pTabBar = pTabBar;
    if (pTB->pFirstTab == NULL)
    {
        assert(pTB->pLastTab == NULL);

        pTB->pFirstTab = pTab;
        pTB->pLastTab  = pTab;
    }
    else
    {
        assert(pTB->pLastTab != NULL);

        pTab->pNextTab = pTB->pFirstTab;

        pTB->pFirstTab->pPrevTab = pTab;
        pTB->pFirstTab = pTab;
    }


    if (pTB->isAutoSizeEnabled) {
        dred_tabbar_resize_by_tabs(pTabBar);
    }

    // The content of the menu has changed so we'll need to schedule a redraw.
    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}

DRED_GUI_PRIVATE void tab_detach_from_hierarchy(dred_tab* pTab)
{
    if (pTab == NULL) {
        return;
    }

    dred_tabbar* pTabBar = pTab->pTabBar;
    if (pTabBar == NULL) {
        return;
    }

    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    assert(pTB != NULL);


    if (pTab->pNextTab != NULL) {
        pTab->pNextTab->pPrevTab = pTab->pPrevTab;
    }

    if (pTab->pPrevTab != NULL) {
        pTab->pPrevTab->pNextTab = pTab->pNextTab;
    }


    if (pTab == pTB->pFirstTab) {
        pTB->pFirstTab = pTab->pNextTab;
    }

    if (pTab == pTB->pLastTab) {
        pTB->pLastTab = pTab->pPrevTab;
    }


    pTab->pNextTab   = NULL;
    pTab->pPrevTab   = NULL;
    pTab->pTabBar = NULL;
}

DRED_GUI_PRIVATE void tab_detach(dred_tab* pTab)
{
    if (pTab == NULL) {
        return;
    }

    dred_tabbar* pTabBar = pTab->pTabBar;
    if (pTabBar == NULL) {
        return;
    }

    dred_tabbar_data* pTB = (dred_tabbar_data*)dred_control_get_extra_data(pTabBar);
    assert(pTB != NULL);

    if (pTB->pHoveredTab == pTab) {
        pTB->pHoveredTab = NULL;
        pTB->isCloseButtonHovered = false;
    }

    if (pTB->pActiveTab == pTab) {
        pTB->pActiveTab = NULL;
    }

    if (pTB->pTabWithCloseButtonPressed == pTab) {
        pTB->pTabWithCloseButtonPressed = NULL;
    }


    tab_detach_from_hierarchy(pTab);


    if (pTB->isAutoSizeEnabled) {
        dred_tabbar_resize_by_tabs(pTabBar);
    }

    // The content of the menu has changed so we'll need to schedule a redraw.
    if (dred_gui_is_auto_dirty_enabled(pTabBar->pGUI)) {
        dred_control_dirty(pTabBar, dred_control_get_local_rect(pTabBar));
    }
}
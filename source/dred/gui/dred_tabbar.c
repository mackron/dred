// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef struct drgui_tab_bar drgui_tab_bar;

struct drgui_tab_bar
{
    /// The orientation.
    drgui_tabbar_orientation orientation;


    /// A pointer to the first tab.
    drgui_tab* pFirstTab;

    /// A pointer to the last tab.
    drgui_tab* pLastTab;


    /// A pointer to the hovered tab.
    drgui_tab* pHoveredTab;

    /// A pointer to the active tab.
    drgui_tab* pActiveTab;

    /// The tab whose close button is currently pressed, if any.
    drgui_tab* pTabWithCloseButtonPressed;


    /// The default font to use for tab bar items.
    drgui_font* pFont;

    /// The default color to use for tab bar item text.
    drgui_color tabTextColor;

    /// The default color to use for tab bar item text while active.
    drgui_color tabTextColorActivated;

    /// The default color to use for tab bar item text while hovered.
    drgui_color tabTextColorHovered;

    /// The default background color of tab bar items.
    drgui_color tabBackgroundColor;

    /// The background color of tab bar items while hovered.
    drgui_color tabBackgroundColorHovered;

    /// The background color of tab bar items while selected.
    drgui_color tabBackbroundColorActivated;

    /// The padding to apply to the text of tabs.
    float tabPadding;

    /// The image to use for the close button.
    drgui_image* pCloseButtonImage;

    /// The padding to the left of the close button.
    float closeButtonPaddingLeft;

    /// The default color of the close button.
    drgui_color closeButtonColorDefault;

    /// The color of the close button when the tab is hovered, but not the close button itself.
    drgui_color closeButtonColorTabHovered;

    /// The color of the close button when it is hovered.
    drgui_color closeButtonColorHovered;

    /// The color of the close button when it is pressed.
    drgui_color closeButtonColorPressed;


    /// Whether or not auto-sizing is enabled. Disabled by default.
    bool isAutoSizeEnabled;

    /// Whether or not the close buttons are being shown.
    bool isShowingCloseButton;

    /// Whether or not close-on-middle-click is enabled.
    bool isCloseOnMiddleClickEnabled;

    /// Whether or not the close button is hovered.
    bool isCloseButtonHovered;


    /// The function to call when a tab needs to be measured.
    drgui_tabbar_on_measure_tab_proc onMeasureTab;

    /// The function to call when a tab needs to be painted.
    drgui_tabbar_on_paint_tab_proc onPaintTab;

    /// The function to call when a tab is activated.
    drgui_tabbar_on_tab_activated_proc onTabActivated;

    /// The function to call when a tab is deactivated.
    drgui_tabbar_on_tab_deactivated_proc onTabDeactivated;

    /// The function to call when a tab is closed via the close button.
    drgui_tabbar_on_tab_close_proc onTabClose;

    // The function to call when a mouse button is released while over a tab.
    drgui_tabbar_on_tab_mouse_button_up_proc onTabMouseButtonUp;


    /// The size of the extra data.
    size_t extraDataSize;

    /// A pointer to the extra data.
    char pExtraData[1];
};

struct drgui_tab
{
    /// The tab bar that owns the tab.
    drgui_element* pTBElement;

    /// A pointer to the next tab in the tab bar.
    drgui_tab* pNextTab;

    /// A pointer to the previous tab in the tab bar.
    drgui_tab* pPrevTab;


    /// The tab bar's text.
    char text[DRGUI_MAX_TAB_TEXT_LENGTH];


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
DRGUI_PRIVATE void drgui_tabbar_on_measure_tab_default(drgui_element* pTBElement, drgui_tab* pTab, float* pWidthOut, float* pHeightOut);

/// Paints the given menu item.
DRGUI_PRIVATE void drgui_tabbar_on_paint_tab_default(drgui_element* pTBElement, drgui_tab* pTab, drgui_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData);

/// Finds the tab sitting under the given point, if any.
DRGUI_PRIVATE drgui_tab* drgui_tabbar_find_tab_under_point(drgui_element* pTBElement, float relativePosX, float relativePosY, bool* pIsOverCloseButtonOut);

drgui_element* drgui_create_tab_bar(drgui_context* pContext, drgui_element* pParent, drgui_tabbar_orientation orientation, size_t extraDataSize, const void* pExtraData)
{
    if (pContext == NULL) {
        return NULL;
    }

    drgui_element* pTBElement = drgui_create_element(pContext, pParent, sizeof(drgui_tab_bar) + extraDataSize, NULL);
    if (pTBElement == NULL) {
        return NULL;
    }

    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    assert(pTB != NULL);

    pTB->orientation                 = orientation;
    pTB->pFirstTab                   = NULL;
    pTB->pLastTab                    = NULL;
    pTB->pHoveredTab                 = NULL;
    pTB->pActiveTab                  = NULL;
    pTB->pTabWithCloseButtonPressed  = NULL;

    pTB->pFont                       = NULL;
    pTB->tabTextColor                = drgui_rgb(224, 224, 224);
    pTB->tabTextColorActivated       = drgui_rgb(224, 224, 224);
    pTB->tabTextColorHovered         = drgui_rgb(224, 224, 224);
    pTB->tabBackgroundColor          = drgui_rgb(58, 58, 58);
    pTB->tabBackgroundColorHovered   = drgui_rgb(16, 92, 160);
    pTB->tabBackbroundColorActivated = drgui_rgb(32, 128, 192); //drgui_rgb(80, 80, 80);
    pTB->tabPadding                  = 4;
    pTB->pCloseButtonImage           = NULL;
    pTB->closeButtonPaddingLeft      = 6;
    pTB->closeButtonColorDefault     = pTB->tabBackgroundColor;
    pTB->closeButtonColorTabHovered  = drgui_rgb(192, 192, 192);
    pTB->closeButtonColorHovered     = drgui_rgb(255, 96, 96);
    pTB->closeButtonColorPressed     = drgui_rgb(192, 32, 32);
    pTB->isAutoSizeEnabled           = false;
    pTB->isShowingCloseButton        = false;
    pTB->isCloseOnMiddleClickEnabled = false;
    pTB->isCloseButtonHovered        = false;

    pTB->onMeasureTab                = drgui_tabbar_on_measure_tab_default;
    pTB->onPaintTab                  = drgui_tabbar_on_paint_tab_default;
    pTB->onTabActivated              = NULL;
    pTB->onTabDeactivated            = NULL;
    pTB->onTabClose                  = NULL;


    pTB->extraDataSize = extraDataSize;
    if (pExtraData != NULL) {
        memcpy(pTB->pExtraData, pExtraData, extraDataSize);
    }


    // Event handlers.
    drgui_set_on_mouse_leave(pTBElement, drgui_tabbar_on_mouse_leave);
    drgui_set_on_mouse_move(pTBElement, drgui_tabbar_on_mouse_move);
    drgui_set_on_mouse_button_down(pTBElement, drgui_tabbar_on_mouse_button_down);
    drgui_set_on_mouse_button_up(pTBElement, drgui_tabbar_on_mouse_button_up);
    drgui_set_on_paint(pTBElement, drgui_tabbar_on_paint);

    return pTBElement;
}

void drgui_delete_tab_bar(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    while (pTB->pFirstTab != NULL) {
        drgui_tab_delete(pTB->pFirstTab);
    }


    drgui_delete_element(pTBElement);
}


size_t drgui_tabbar_get_extra_data_size(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return 0;
    }

    return pTB->extraDataSize;
}

void* drgui_tabbar_get_extra_data(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pExtraData;
}

drgui_tabbar_orientation drgui_tabbar_get_orientation(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return drgui_tabbar_orientation_top;
    }

    return pTB->orientation;
}


void drgui_tabbar_set_font(drgui_element* pTBElement, drgui_font* pFont)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->pFont = pFont;

    // A change in font may have changed the size of the tabbar.
    if (pTB->isAutoSizeEnabled) {
        drgui_tabbar_resize_by_tabs(pTBElement);
    }

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

drgui_font* drgui_tabbar_get_font(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pFont;
}


void drgui_tabbar_set_text_color(drgui_element* pTBElement, drgui_color color)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->tabTextColor = color;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

drgui_color drgui_tabbar_get_text_color(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pTB->tabTextColor;
}

void drgui_tabbar_set_text_color_active(drgui_element* pTBElement, drgui_color color)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->tabTextColorActivated = color;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

void drgui_tabbar_set_text_color_hovered(drgui_element* pTBElement, drgui_color color)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->tabTextColorHovered = color;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}


void drgui_tabbar_set_close_button_image(drgui_element* pTBElement, drgui_image* pImage)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->pCloseButtonImage = pImage;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

drgui_image* drgui_tabbar_get_close_button_image(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pCloseButtonImage;
}

void drgui_tabbar_set_close_button_color(drgui_element* pTBElement, drgui_color color)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->closeButtonColorDefault = color;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}


void drgui_tabbar_set_tab_padding(drgui_element* pTBElement, float padding)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->tabPadding = padding;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

float drgui_tabbar_get_tab_padding(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return 0;
    }

    return pTB->tabPadding;
}

void drgui_tabbar_set_close_button_left_padding(drgui_element* pTBElement, float padding)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->closeButtonPaddingLeft = padding;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

float drgui_tabbar_get_close_button_left_padding(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return 0;
    }

    return pTB->closeButtonPaddingLeft;
}


void drgui_tabbar_set_tab_background_color(drgui_element* pTBElement, drgui_color color)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->tabBackgroundColor = color;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

drgui_color drgui_tabbar_get_tab_background_color(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pTB->tabBackgroundColor;
}

void drgui_tabbar_set_tab_background_color_hovered(drgui_element* pTBElement, drgui_color color)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->tabBackgroundColorHovered = color;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

drgui_color drgui_tabbar_get_tab_background_color_hovered(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pTB->tabBackgroundColorHovered;
}

void drgui_tabbar_set_tab_background_color_active(drgui_element* pTBElement, drgui_color color)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->tabBackbroundColorActivated = color;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

drgui_color drgui_tabbar_get_tab_background_color_actived(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pTB->tabBackbroundColorActivated;
}


void drgui_tabbar_set_on_measure_tab(drgui_element* pTBElement, drgui_tabbar_on_measure_tab_proc proc)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->onMeasureTab = proc;
}

void drgui_tabbar_set_on_paint_tab(drgui_element* pTBElement, drgui_tabbar_on_paint_tab_proc proc)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->onPaintTab = proc;
}

void drgui_tabbar_set_on_tab_activated(drgui_element* pTBElement, drgui_tabbar_on_tab_activated_proc proc)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->onTabActivated = proc;
}

void drgui_tabbar_set_on_tab_deactivated(drgui_element* pTBElement, drgui_tabbar_on_tab_deactivated_proc proc)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->onTabDeactivated = proc;
}

void drgui_tabbar_set_on_tab_closed(drgui_element* pTBElement, drgui_tabbar_on_tab_close_proc proc)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->onTabClose = proc;
}

void drgui_tabbar_set_on_tab_mouse_button_up(drgui_element* pTBElement, drgui_tabbar_on_tab_mouse_button_up_proc proc)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->onTabMouseButtonUp = proc;
}


void drgui_tabbar_measure_tab(drgui_element* pTBElement, drgui_tab* pTab, float* pWidthOut, float* pHeightOut)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    if (pTB->onMeasureTab) {
        pTB->onMeasureTab(pTBElement, pTab, pWidthOut, pHeightOut);
    }
}

void drgui_tabbar_paint_tab(drgui_element* pTBElement, drgui_tab* pTab, drgui_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    if (pTB->onPaintTab) {
        pTB->onPaintTab(pTBElement, pTab, relativeClippingRect, offsetX, offsetY, width, height, pPaintData);
    }
}


void drgui_tabbar_resize_by_tabs(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
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
        drgui_font_metrics fontMetrics;
        if (drgui_get_font_metrics(pTB->pFont, &fontMetrics)) {
            if (pTB->orientation == drgui_tabbar_orientation_top || pTB->orientation == drgui_tabbar_orientation_bottom) {
                maxHeight = fontMetrics.lineHeight + (pTB->tabPadding*2);
            } else {
                maxWidth = fontMetrics.lineHeight + (pTB->tabPadding*2);
            }
        }
    } else {
        for (drgui_tab* pTab = pTB->pFirstTab; pTab != NULL; pTab = pTab->pNextTab) {
            float tabWidth  = 0;
            float tabHeight = 0;
            drgui_tabbar_measure_tab(pTBElement, pTab, &tabWidth, &tabHeight);

            maxWidth  = (tabWidth  > maxWidth)  ? tabWidth  : maxWidth;
            maxHeight = (tabHeight > maxHeight) ? tabHeight : maxHeight;
        }
    }

    


    if (pTB->orientation == drgui_tabbar_orientation_top || pTB->orientation == drgui_tabbar_orientation_bottom) {
        drgui_set_size(pTBElement, drgui_get_width(pTBElement), maxHeight);
    } else {
        drgui_set_size(pTBElement, maxWidth, drgui_get_height(pTBElement));
    }
}

void drgui_tabbar_enable_auto_size(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->isAutoSizeEnabled = true;
}

void drgui_tabbar_disable_auto_size(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->isAutoSizeEnabled = false;
}

bool drgui_tabbar_is_auto_size_enabled(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return false;
    }

    return pTB->isAutoSizeEnabled;
}


drgui_tab* drgui_tabbar_get_first_tab(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pFirstTab;
}

drgui_tab* drgui_tabbar_get_last_tab(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pLastTab;
}

drgui_tab* drgui_tabbar_get_next_tab(drgui_element* pTBElement, drgui_tab* pTab)
{
    (void)pTBElement;
    return drgui_tab_get_next_tab(pTab);
}

drgui_tab* drgui_tabbar_get_prev_tab(drgui_element* pTBElement, drgui_tab* pTab)
{
    (void)pTBElement;
    return drgui_tab_get_prev_tab(pTab);
}


void drgui_tabbar_activate_tab(drgui_element* pTBElement, drgui_tab* pTab)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    drgui_tab* pOldActiveTab = pTB->pActiveTab;
    drgui_tab* pNewActiveTab = pTab;

    if (pOldActiveTab == pNewActiveTab) {
        return;     // The tab is already active - nothing to do.
    }


    pTB->pActiveTab = pNewActiveTab;

    if (pTB->onTabDeactivated && pOldActiveTab != NULL) {
        pTB->onTabDeactivated(pTBElement, pOldActiveTab, pNewActiveTab);
    }

    if (pTB->onTabActivated && pNewActiveTab != NULL) {
        pTB->onTabActivated(pTBElement, pNewActiveTab, pOldActiveTab);
    }


    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

void drgui_tabbar_activate_next_tab(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    if (pTB->pActiveTab == NULL) {
        drgui_tabbar_activate_tab(pTBElement, pTB->pFirstTab);
        return;
    }


    drgui_tab* pNextTab = pTB->pActiveTab->pNextTab;
    if (pNextTab == NULL) {
        pNextTab = pTB->pFirstTab;
    }

    drgui_tabbar_activate_tab(pTBElement, pNextTab);
}

void drgui_tabbar_activate_prev_tab(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    if (pTB->pActiveTab == NULL) {
        drgui_tabbar_activate_tab(pTBElement, pTB->pLastTab);
        return;
    }


    drgui_tab* pPrevTab = pTB->pActiveTab->pPrevTab;
    if (pPrevTab == NULL) {
        pPrevTab = pTB->pLastTab;
    }

    drgui_tabbar_activate_tab(pTBElement, pPrevTab);
}

drgui_tab* drgui_tabbar_get_active_tab(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pActiveTab;
}


bool drgui_tabbar_is_tab_in_view(drgui_element* pTBElement, drgui_tab* pTabIn)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return false;
    }

    float tabbarWidth  = 0;
    float tabbarHeight = 0;
    drgui_get_size(pTBElement, &tabbarWidth, &tabbarHeight);


    // Each tab.
    float runningPosX = 0;
    float runningPosY = 0;
    for (drgui_tab* pTab = pTB->pFirstTab; pTab != NULL; pTab = pTab->pNextTab)
    {
        float tabWidth  = 0;
        float tabHeight = 0;
        drgui_tabbar_measure_tab(pTBElement, pTab, &tabWidth, &tabHeight);

        if (pTab == pTabIn) {
            return runningPosX + tabWidth <= tabbarWidth && runningPosY + tabHeight <= tabbarHeight;
        }


        if (pTB->orientation == drgui_tabbar_orientation_top || pTB->orientation == drgui_tabbar_orientation_bottom) {
            runningPosX += tabWidth;
        } else {
            runningPosY += tabHeight;
        }
    }

    return false;
}


void drgui_tabbar_show_close_buttons(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->isShowingCloseButton = true;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

void drgui_tabbar_hide_close_buttons(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->isShowingCloseButton = false;

    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}


void drgui_tabbar_enable_close_on_middle_click(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->isCloseOnMiddleClickEnabled = true;
}

void drgui_tabbar_disable_close_on_middle_click(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    pTB->isCloseOnMiddleClickEnabled = false;
}

bool drgui_tabbar_is_close_on_middle_click_enabled(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return false;
    }

    return pTB->isCloseOnMiddleClickEnabled;
}


void drgui_tabbar_on_mouse_leave(drgui_element* pTBElement)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    if (pTB->pHoveredTab != NULL)
    {
        pTB->pHoveredTab = NULL;
        pTB->isCloseButtonHovered = false;

        if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
            drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
        }
    }
}

void drgui_tabbar_on_mouse_move(drgui_element* pTBElement, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    bool isCloseButtonHovered = false;

    drgui_tab* pOldHoveredTab = pTB->pHoveredTab;
    drgui_tab* pNewHoveredTab = drgui_tabbar_find_tab_under_point(pTBElement, (float)relativeMousePosX, (float)relativeMousePosY, &isCloseButtonHovered);

    if (pOldHoveredTab != pNewHoveredTab || pTB->isCloseButtonHovered != isCloseButtonHovered)
    {
        pTB->pHoveredTab = pNewHoveredTab;
        pTB->isCloseButtonHovered = isCloseButtonHovered;

        if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
            drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
        }
    }
}

void drgui_tabbar_on_mouse_button_down(drgui_element* pTBElement, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    if (mouseButton == DRGUI_MOUSE_BUTTON_LEFT || mouseButton == DRGUI_MOUSE_BUTTON_RIGHT)
    {
        bool isOverCloseButton = false;

        drgui_tab* pOldActiveTab = pTB->pActiveTab;
        drgui_tab* pNewActiveTab = drgui_tabbar_find_tab_under_point(pTBElement, (float)relativeMousePosX, (float)relativeMousePosY, &isOverCloseButton);

        if (pNewActiveTab != NULL && pOldActiveTab != pNewActiveTab && !isOverCloseButton) {
            drgui_tabbar_activate_tab(pTBElement, pNewActiveTab);
        }

        if (isOverCloseButton && mouseButton == DRGUI_MOUSE_BUTTON_LEFT) {
            pTB->pTabWithCloseButtonPressed = pNewActiveTab;

            if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
                drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
            }
        }
    }
    else if (mouseButton == DRGUI_MOUSE_BUTTON_MIDDLE)
    {
        if (pTB->isCloseOnMiddleClickEnabled)
        {
            drgui_tab* pHoveredTab = drgui_tabbar_find_tab_under_point(pTBElement, (float)relativeMousePosX, (float)relativeMousePosY, NULL);
            if (pHoveredTab != NULL) {
                if (pTB->onTabClose) {
                    pTB->onTabClose(pTBElement, pHoveredTab);
                }
            }
        }
    }
}

void drgui_tabbar_on_mouse_button_up(drgui_element* pTBElement, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }


    bool releasedOverCloseButton = false;
    drgui_tab* pTabUnderMouse = drgui_tabbar_find_tab_under_point(pTBElement, (float)relativeMousePosX, (float)relativeMousePosY, &releasedOverCloseButton);

    if (pTB->pTabWithCloseButtonPressed != NULL && mouseButton == DRGUI_MOUSE_BUTTON_LEFT)
    {
        if (releasedOverCloseButton && pTabUnderMouse == pTB->pTabWithCloseButtonPressed) {
            if (pTB->onTabClose) {
                pTB->onTabClose(pTBElement, pTB->pTabWithCloseButtonPressed);
            }
        }


        pTB->pTabWithCloseButtonPressed = NULL;

        if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
            drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
        }
    }
    else
    {
        if (!releasedOverCloseButton && pTB->onTabMouseButtonUp) {
            // TODO: Improve this by passing the mouse position relative to the tab. Currently it is relative to the tab BAR. Can have
            // the drgui_tabbar_find_tab_under_point() function return the position relative to the tab.
            pTB->onTabMouseButtonUp(pTBElement, pTabUnderMouse, mouseButton, relativeMousePosX, relativeMousePosY, stateFlags);
        }
    }
}

void drgui_tabbar_on_paint(drgui_element* pTBElement, drgui_rect relativeClippingRect, void* pPaintData)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }


    float tabbarWidth  = 0;
    float tabbarHeight = 0;
    drgui_get_size(pTBElement, &tabbarWidth, &tabbarHeight);


    // Each tab.
    float runningPosX = 0;
    float runningPosY = 0;
    for (drgui_tab* pTab = pTB->pFirstTab; pTab != NULL; pTab = pTab->pNextTab)
    {
        float tabWidth  = 0;
        float tabHeight = 0;
        drgui_tabbar_measure_tab(pTBElement, pTab, &tabWidth, &tabHeight);

        // If a part of the tab is out of bounds, stop drawing.
        if (runningPosX > tabbarWidth || runningPosY > tabbarHeight) {
            break;
        }


        drgui_tabbar_paint_tab(pTBElement, pTab, relativeClippingRect, runningPosX, runningPosY, tabWidth, tabHeight, pPaintData);

        // After painting the tab, there may be a region of the background that was not drawn by the tab painting callback. We'll need to
        // draw that here.
        if (pTB->orientation == drgui_tabbar_orientation_top || pTB->orientation == drgui_tabbar_orientation_bottom) {
            drgui_draw_rect(pTBElement, drgui_make_rect(runningPosX, runningPosY + tabHeight, tabbarWidth, tabbarHeight), pTB->tabBackgroundColor, pPaintData);
        } else {
            drgui_draw_rect(pTBElement, drgui_make_rect(runningPosX + tabWidth, runningPosY, tabbarWidth, runningPosY + tabHeight), pTB->tabBackgroundColor, pPaintData);
        }



        if (pTB->orientation == drgui_tabbar_orientation_top || pTB->orientation == drgui_tabbar_orientation_bottom) {
            runningPosX += tabWidth;
        } else {
            runningPosY += tabHeight;
        }
    }


    // Background. We just draw a quad around the region that is not covered by items.
    drgui_draw_rect(pTBElement, drgui_make_rect(runningPosX, runningPosY, tabbarWidth, tabbarHeight), pTB->tabBackgroundColor, pPaintData);
}


DRGUI_PRIVATE void drgui_tabbar_on_measure_tab_default(drgui_element* pTBElement, drgui_tab* pTab, float* pWidthOut, float* pHeightOut)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    float textWidth  = 0;
    float textHeight = 0;

    if (pTab != NULL) {
        drgui_measure_string(pTB->pFont, pTab->text, strlen(pTab->text), &textWidth, &textHeight);
    }


    float closeButtonWidth  = 0;
    if (pTB->isShowingCloseButton && pTB->pCloseButtonImage != NULL) {
        unsigned int closeImageWidth;
        drgui_get_image_size(pTB->pCloseButtonImage, &closeImageWidth, NULL);

        closeButtonWidth  = closeImageWidth + pTB->closeButtonPaddingLeft;
    }


    if (pWidthOut) {
        *pWidthOut = textWidth + closeButtonWidth + pTB->tabPadding*2;
    }
    if (pHeightOut) {
        *pHeightOut = textHeight + pTB->tabPadding*2;
    }
}

DRGUI_PRIVATE void drgui_tabbar_on_paint_tab_default(drgui_element* pTBElement, drgui_tab* pTab, drgui_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData)
{
    (void)relativeClippingRect;

    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return;
    }

    // Background.
    drgui_color bgcolor = pTB->tabBackgroundColor;
    drgui_color closeButtonColor = pTB->closeButtonColorDefault;
    drgui_color textColor = pTB->tabTextColor;

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

    drgui_draw_rect_outline(pTBElement, drgui_make_rect(offsetX, offsetY, offsetX + width, offsetY + height), bgcolor, pTB->tabPadding, pPaintData);


    // Text.
    float textPosX = offsetX + pTB->tabPadding;
    float textPosY = offsetY + pTB->tabPadding;
    if (pTab != NULL) {
        drgui_draw_text(pTBElement, pTB->pFont, pTab->text, (int)strlen(pTab->text), textPosX, textPosY, textColor, bgcolor, pPaintData);
    }


    // Close button.
    if (pTB->isShowingCloseButton && pTB->pCloseButtonImage != NULL)
    {
        float textWidth  = 0;
        float textHeight = 0;
        if (pTab != NULL) {
            drgui_measure_string(pTB->pFont, pTab->text, strlen(pTab->text), &textWidth, &textHeight);
        }

        float closeButtonPosX = textPosX + textWidth + pTB->closeButtonPaddingLeft;
        float closeButtonPosY = textPosY;

        unsigned int iconWidth;
        unsigned int iconHeight;
        drgui_get_image_size(pTB->pCloseButtonImage, &iconWidth, &iconHeight);

        drgui_draw_image_args args;
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
        args.options         = DRGUI_IMAGE_DRAW_BACKGROUND | DRGUI_IMAGE_DRAW_BOUNDS | DRGUI_IMAGE_CLIP_BOUNDS | DRGUI_IMAGE_ALIGN_CENTER;
        drgui_draw_image(pTBElement, pTB->pCloseButtonImage, &args, pPaintData);


        /// Space between the text and the padding.
        drgui_draw_rect(pTBElement, drgui_make_rect(textPosX + textWidth, textPosY, closeButtonPosX, textPosY + textHeight), bgcolor, pPaintData);
    }
}

DRGUI_PRIVATE drgui_tab* drgui_tabbar_find_tab_under_point(drgui_element* pTBElement, float relativePosX, float relativePosY, bool* pIsOverCloseButtonOut)
{
    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    if (pTB == NULL) {
        return NULL;
    }

    unsigned int closeButtonWidth;
    unsigned int closeButtonHeight;
    drgui_get_image_size(pTB->pCloseButtonImage, &closeButtonWidth, &closeButtonHeight);

    float runningPosX = 0;
    float runningPosY = 0;
    for (drgui_tab* pTab = pTB->pFirstTab; pTab != NULL; pTab = pTab->pNextTab)
    {
        float tabWidth  = 0;
        float tabHeight = 0;
        drgui_tabbar_measure_tab(pTBElement, pTab, &tabWidth, &tabHeight);

        if (relativePosX >= runningPosX && relativePosX < runningPosX + tabWidth && relativePosY >= runningPosY && relativePosY < runningPosY + tabHeight)
        {
            if (pIsOverCloseButtonOut)
            {
                // The close button is in the center, vertically.
                drgui_rect closeButtonRect;
                closeButtonRect.left   = runningPosX + tabWidth - (pTB->tabPadding + closeButtonWidth);
                closeButtonRect.right  = closeButtonRect.left + closeButtonWidth;
                closeButtonRect.top    = runningPosY + (tabHeight - (pTB->tabPadding + closeButtonHeight))/2;
                closeButtonRect.bottom = closeButtonRect.top + closeButtonHeight;

                if (pTB->isShowingCloseButton && drgui_rect_contains_point(closeButtonRect, relativePosX, relativePosY)) {
                    *pIsOverCloseButtonOut = true;
                } else {
                    *pIsOverCloseButtonOut = false;
                }
            }

            return pTab;
        }

        if (pTB->orientation == drgui_tabbar_orientation_top || pTB->orientation == drgui_tabbar_orientation_bottom) {
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
DRGUI_PRIVATE void tab_append(drgui_tab* pTab, drgui_element* pTBElement);

/// Prepends the given tab to the given tab bar.
DRGUI_PRIVATE void tab_prepend(drgui_tab* pTab, drgui_element* pTBElement);

/// Detaches the given tab bar from it's tab bar element's hierarchy.
///
/// @remarks
///     This does not deactivate the tab or what - it only detaches the tab from the hierarchy.
DRGUI_PRIVATE void tab_detach_from_hierarchy(drgui_tab* pTab);

/// Detaches the given tab bar from it's tab bar element.
DRGUI_PRIVATE void tab_detach(drgui_tab* pTab);

DRGUI_PRIVATE drgui_tab* tb_create_tab(drgui_element* pTBElement, const char* text, size_t extraDataSize, const void* pExtraData)
{
    if (pTBElement == NULL) {
        return NULL;
    }

    drgui_tab* pTab = (drgui_tab*)malloc(sizeof(*pTab) + extraDataSize);
    if (pTab == NULL) {
        return NULL;
    }

    pTab->pTBElement = NULL;
    pTab->pNextTab   = NULL;
    pTab->pPrevTab   = NULL;
    pTab->text[0]    = '\0';

    pTab->extraDataSize = extraDataSize;
    if (pExtraData) {
        memcpy(pTab->pExtraData, pExtraData, extraDataSize);
    }

    if (text != NULL) {
        drgui__strncpy_s(pTab->text, sizeof(pTab->text), text, (size_t)-1); // -1 = _TRUNCATE
    }

    return pTab;
}

drgui_tab* drgui_tabbar_create_and_append_tab(drgui_element* pTBElement, const char* text, size_t extraDataSize, const void* pExtraData)
{
    drgui_tab* pTab = (drgui_tab*)tb_create_tab(pTBElement, text, extraDataSize, pExtraData);
    if (pTab != NULL)
    {
        tab_append(pTab, pTBElement);
    }

    return pTab;
}

drgui_tab* drgui_tabbar_create_and_prepend_tab(drgui_element* pTBElement, const char* text, size_t extraDataSize, const void* pExtraData)
{
    drgui_tab* pTab = (drgui_tab*)tb_create_tab(pTBElement, text, extraDataSize, pExtraData);
    if (pTab != NULL)
    {
        tab_prepend(pTab, pTBElement);
    }

    return pTab;
}

void drgui_tab_delete(drgui_tab* pTab)
{
    if (pTab == NULL) {
        return;
    }

    tab_detach(pTab);
    free(pTab);
}

drgui_element* drgui_tab_get_tab_bar_element(drgui_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    return pTab->pTBElement;
}

size_t drgui_tab_get_extra_data_size(drgui_tab* pTab)
{
    if (pTab == NULL) {
        return 0;
    }

    return pTab->extraDataSize;
}

void* drgui_tab_get_extra_data(drgui_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    return pTab->pExtraData;
}


void drgui_tab_set_text(drgui_tab* pTab, const char* text)
{
    if (pTab == NULL) {
        return;
    }

    if (text != NULL) {
        drgui__strncpy_s(pTab->text, sizeof(pTab->text), text, (size_t)-1); // -1 = _TRUNCATE
    } else {
        pTab->text[0] = '\0';
    }

    // The content of the menu has changed so we'll need to schedule a redraw.
    if (drgui_is_auto_dirty_enabled(pTab->pTBElement->pContext)) {
        drgui_dirty(pTab->pTBElement, drgui_get_local_rect(pTab->pTBElement));
    }
}

const char* drgui_tab_get_text(drgui_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    return pTab->text;
}


drgui_tab* drgui_tab_get_next_tab(drgui_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    return pTab->pNextTab;
}

drgui_tab* drgui_tab_get_prev_tab(drgui_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    return pTab->pPrevTab;
}


void drgui_tab_move_to_front(drgui_tab* pTab)
{
    if (pTab == NULL) {
        return;
    }

    drgui_element* pTBElement = pTab->pTBElement;

    tab_detach_from_hierarchy(pTab);
    tab_prepend(pTab, pTBElement);
}

bool drgui_tab_is_in_view(drgui_tab* pTab)
{
    if (pTab == NULL) {
        return false;
    }

    return drgui_tabbar_is_tab_in_view(pTab->pTBElement, pTab);
}

void drgui_tab_move_into_view(drgui_tab* pTab)
{
    if (!drgui_tab_is_in_view(pTab)) {
        drgui_tab_move_to_front(pTab);
    }
}




DRGUI_PRIVATE void tab_append(drgui_tab* pTab, drgui_element* pTBElement)
{
    if (pTab == NULL || pTBElement == NULL) {
        return;
    }

    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    assert(pTB != NULL);

    pTab->pTBElement = pTBElement;
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
        drgui_tabbar_resize_by_tabs(pTBElement);
    }

    // The content of the menu has changed so we'll need to schedule a redraw.
    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

DRGUI_PRIVATE void tab_prepend(drgui_tab* pTab, drgui_element* pTBElement)
{
    if (pTab == NULL || pTBElement == NULL) {
        return;
    }

    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
    assert(pTB != NULL);

    pTab->pTBElement = pTBElement;
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
        drgui_tabbar_resize_by_tabs(pTBElement);
    }

    // The content of the menu has changed so we'll need to schedule a redraw.
    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}

DRGUI_PRIVATE void tab_detach_from_hierarchy(drgui_tab* pTab)
{
    if (pTab == NULL) {
        return;
    }

    drgui_element* pTBElement = pTab->pTBElement;
    if (pTBElement == NULL) {
        return;
    }

    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
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
    pTab->pTBElement = NULL;
}

DRGUI_PRIVATE void tab_detach(drgui_tab* pTab)
{
    if (pTab == NULL) {
        return;
    }

    drgui_element* pTBElement = pTab->pTBElement;
    if (pTBElement == NULL) {
        return;
    }

    drgui_tab_bar* pTB = (drgui_tab_bar*)drgui_get_extra_data(pTBElement);
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
        drgui_tabbar_resize_by_tabs(pTBElement);
    }

    // The content of the menu has changed so we'll need to schedule a redraw.
    if (drgui_is_auto_dirty_enabled(pTBElement->pContext)) {
        drgui_dirty(pTBElement, drgui_get_local_rect(pTBElement));
    }
}
// Copyright (C) 2017 David Reid. See included LICENSE file.

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
DRED_GUI_PRIVATE void dred_tabbar_on_paint_tab_default(dred_tabbar* pTabBar, dred_tab* pTab, dred_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, dtk_surface* pSurface);

/// Finds the tab sitting under the given point, if any.
DRED_GUI_PRIVATE dred_tab* dred_tabbar_find_tab_under_point(dred_tabbar* pTabBar, float relativePosX, float relativePosY, dr_bool32* pIsOverCloseButtonOut);

dr_bool32 dred_tabbar_init(dred_tabbar* pTabBar, dred_context* pDred, dred_control* pParent, dred_tabbar_orientation orientation)
{
    if (pTabBar == NULL) {
        return DR_FALSE;
    }

    memset(pTabBar, 0, sizeof(*pTabBar));


    if (!dred_control_init(DRED_CONTROL(pTabBar), pDred, pParent, NULL, DRED_CONTROL_TYPE_TABBAR)) {
        return DR_FALSE;
    }

    pTabBar->orientation                 = orientation;
    pTabBar->pFirstTab                   = NULL;
    pTabBar->pLastTab                    = NULL;
    pTabBar->pHoveredTab                 = NULL;
    pTabBar->pActiveTab                  = NULL;
    pTabBar->pTabWithCloseButtonPressed  = NULL;

    pTabBar->pFont                       = NULL;
    pTabBar->tabTextColor                = dred_rgb(224, 224, 224);
    pTabBar->tabTextColorActivated       = dred_rgb(224, 224, 224);
    pTabBar->tabTextColorHovered         = dred_rgb(224, 224, 224);
    pTabBar->tabBackgroundColor          = dred_rgb(58, 58, 58);
    pTabBar->tabBackgroundColorHovered   = dred_rgb(16, 92, 160);
    pTabBar->tabBackbroundColorActivated = dred_rgb(32, 128, 192); //dred_rgb(80, 80, 80);
    pTabBar->tabPadding                  = 4;
    pTabBar->pCloseButtonImage           = NULL;
    pTabBar->closeButtonPaddingLeft      = 6;
    pTabBar->closeButtonColorDefault     = pTabBar->tabBackgroundColor;
    pTabBar->closeButtonColorTabHovered  = dred_rgb(192, 192, 192);
    pTabBar->closeButtonColorHovered     = dred_rgb(255, 96, 96);
    pTabBar->closeButtonColorPressed     = dred_rgb(192, 32, 32);
    pTabBar->isAutoSizeEnabled           = DR_FALSE;
    pTabBar->isShowingCloseButton        = DR_FALSE;
    pTabBar->isCloseOnMiddleClickEnabled = DR_FALSE;
    pTabBar->isCloseButtonHovered        = DR_FALSE;

    pTabBar->onMeasureTab                = dred_tabbar_on_measure_tab_default;
    pTabBar->onPaintTab                  = dred_tabbar_on_paint_tab_default;
    pTabBar->onTabActivated              = NULL;
    pTabBar->onTabDeactivated            = NULL;
    pTabBar->onTabClose                  = NULL;


    // Event handlers.
    dred_control_set_on_mouse_leave(DRED_CONTROL(pTabBar), dred_tabbar_on_mouse_leave);
    dred_control_set_on_mouse_move(DRED_CONTROL(pTabBar), dred_tabbar_on_mouse_move);
    dred_control_set_on_mouse_button_down(DRED_CONTROL(pTabBar), dred_tabbar_on_mouse_button_down);
    dred_control_set_on_mouse_button_up(DRED_CONTROL(pTabBar), dred_tabbar_on_mouse_button_up);
    dred_control_set_on_paint(DRED_CONTROL(pTabBar), dred_tabbar_on_paint);

    return DR_TRUE;
}

void dred_tabbar_uninit(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return;
    }

    while (pTabBar->pFirstTab != NULL) {
        dred_tab_delete(pTabBar->pFirstTab);
    }


    dred_control_uninit(DRED_CONTROL(pTabBar));
}


dred_tabbar_orientation dred_tabbar_get_orientation(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return dred_tabbar_orientation_top;
    }

    return pTabBar->orientation;
}


void dred_tabbar_set_font(dred_tabbar* pTabBar, dtk_font* pFont)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->pFont = pFont;

    // A change in font may have changed the size of the tabbar.
    if (pTabBar->isAutoSizeEnabled) {
        dred_tabbar_resize_by_tabs(pTabBar);
    }

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

dtk_font* dred_tabbar_get_font(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return NULL;
    }

    return pTabBar->pFont;
}


void dred_tabbar_set_text_color(dred_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->tabTextColor = color;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

dtk_color dred_tabbar_get_text_color(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTabBar->tabTextColor;
}

void dred_tabbar_set_text_color_active(dred_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->tabTextColorActivated = color;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

void dred_tabbar_set_text_color_hovered(dred_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->tabTextColorHovered = color;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}


void dred_tabbar_set_close_button_image(dred_tabbar* pTabBar, dred_gui_image* pImage)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->pCloseButtonImage = pImage;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

dred_gui_image* dred_tabbar_get_close_button_image(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return NULL;
    }

    return pTabBar->pCloseButtonImage;
}

void dred_tabbar_set_close_button_color(dred_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->closeButtonColorDefault = color;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}


void dred_tabbar_set_tab_padding(dred_tabbar* pTabBar, float padding)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->tabPadding = padding;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

float dred_tabbar_get_tab_padding(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return 0;
    }

    return pTabBar->tabPadding;
}

void dred_tabbar_set_close_button_left_padding(dred_tabbar* pTabBar, float padding)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->closeButtonPaddingLeft = padding;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

float dred_tabbar_get_close_button_left_padding(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return 0;
    }

    return pTabBar->closeButtonPaddingLeft;
}


void dred_tabbar_set_tab_background_color(dred_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->tabBackgroundColor = color;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

dtk_color dred_tabbar_get_tab_background_color(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTabBar->tabBackgroundColor;
}

void dred_tabbar_set_tab_background_color_hovered(dred_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->tabBackgroundColorHovered = color;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

dtk_color dred_tabbar_get_tab_background_color_hovered(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTabBar->tabBackgroundColorHovered;
}

void dred_tabbar_set_tab_background_color_active(dred_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->tabBackbroundColorActivated = color;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

dtk_color dred_tabbar_get_tab_background_color_actived(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTabBar->tabBackbroundColorActivated;
}


void dred_tabbar_set_on_measure_tab(dred_tabbar* pTabBar, dred_tabbar_on_measure_tab_proc proc)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->onMeasureTab = proc;
}

void dred_tabbar_set_on_paint_tab(dred_tabbar* pTabBar, dred_tabbar_on_paint_tab_proc proc)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->onPaintTab = proc;
}

void dred_tabbar_set_on_tab_activated(dred_tabbar* pTabBar, dred_tabbar_on_tab_activated_proc proc)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->onTabActivated = proc;
}

void dred_tabbar_set_on_tab_deactivated(dred_tabbar* pTabBar, dred_tabbar_on_tab_deactivated_proc proc)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->onTabDeactivated = proc;
}

void dred_tabbar_set_on_tab_closed(dred_tabbar* pTabBar, dred_tabbar_on_tab_close_proc proc)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->onTabClose = proc;
}

void dred_tabbar_set_on_tab_mouse_button_up(dred_tabbar* pTabBar, dred_tabbar_on_tab_mouse_button_up_proc proc)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->onTabMouseButtonUp = proc;
}


void dred_tabbar_measure_tab(dred_tabbar* pTabBar, dred_tab* pTab, float* pWidthOut, float* pHeightOut)
{
    if (pTabBar == NULL) {
        return;
    }

    if (pTabBar->onMeasureTab) {
        pTabBar->onMeasureTab(pTabBar, pTab, pWidthOut, pHeightOut);
    }
}

void dred_tabbar_paint_tab(dred_tabbar* pTabBar, dred_tab* pTab, dred_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, dtk_surface* pSurface)
{
    if (pTabBar == NULL) {
        return;
    }

    if (pTabBar->onPaintTab) {
        pTabBar->onPaintTab(pTabBar, pTab, relativeClippingRect, offsetX, offsetY, width, height, pSurface);
    }
}


void dred_tabbar_resize_by_tabs(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return;
    }

    if (pTabBar->onMeasureTab == NULL) {
        return;
    }

    float maxWidth  = 0;
    float maxHeight = 0;

#if 0
    if (pTabBar->pFirstTab == NULL) {
        // There are no tabs. Set initial size based on the line height of the font.
        dtk_font_metrics fontMetrics;
        if (dred_gui_get_font_metrics(pTabBar->pFont, &fontMetrics)) {
            if (pTabBar->orientation == dred_tabbar_orientation_top || pTabBar->orientation == dred_tabbar_orientation_bottom) {
                maxHeight = fontMetrics.lineHeight + (pTabBar->tabPadding*2);
            } else {
                maxWidth = fontMetrics.lineHeight + (pTabBar->tabPadding*2);
            }
        }
    } else {
        for (dred_tab* pTab = pTabBar->pFirstTab; pTab != NULL; pTab = pTab->pNextTab) {
            float tabWidth  = 0;
            float tabHeight = 0;
            dred_tabbar_measure_tab(pTabBar, pTab, &tabWidth, &tabHeight);

            maxWidth  = (tabWidth  > maxWidth)  ? tabWidth  : maxWidth;
            maxHeight = (tabHeight > maxHeight) ? tabHeight : maxHeight;
        }
    }
#endif

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pTabBar));

    dtk_font_metrics fontMetrics;
    if (dtk_font_get_metrics(pTabBar->pFont, uiScale, &fontMetrics) == DTK_SUCCESS) {
        if (pTabBar->orientation == dred_tabbar_orientation_top || pTabBar->orientation == dred_tabbar_orientation_bottom) {
            maxHeight = fontMetrics.lineHeight + (pTabBar->tabPadding*2);
        } else {
            maxWidth = fontMetrics.lineHeight + (pTabBar->tabPadding*2);
        }
    }

    
    if (pTabBar->orientation == dred_tabbar_orientation_top || pTabBar->orientation == dred_tabbar_orientation_bottom) {
        dred_control_set_size(DRED_CONTROL(pTabBar), dred_control_get_width(DRED_CONTROL(pTabBar)), maxHeight);
    } else {
        dred_control_set_size(DRED_CONTROL(pTabBar), maxWidth, dred_control_get_height(DRED_CONTROL(pTabBar)));
    }
}

void dred_tabbar_enable_auto_size(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->isAutoSizeEnabled = DR_TRUE;
}

void dred_tabbar_disable_auto_size(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->isAutoSizeEnabled = DR_FALSE;
}

dr_bool32 dred_tabbar_is_auto_size_enabled(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return DR_FALSE;
    }

    return pTabBar->isAutoSizeEnabled;
}


dred_tab* dred_tabbar_get_first_tab(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return NULL;
    }

    return pTabBar->pFirstTab;
}

dred_tab* dred_tabbar_get_last_tab(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return NULL;
    }

    return pTabBar->pLastTab;
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
    if (pTabBar == NULL) {
        return;
    }

    dred_tab* pOldActiveTab = pTabBar->pActiveTab;
    dred_tab* pNewActiveTab = pTab;

    if (pOldActiveTab == pNewActiveTab) {
        return;     // The tab is already active - nothing to do.
    }


    pTabBar->pActiveTab = pNewActiveTab;

    if (pTabBar->onTabDeactivated && pOldActiveTab != NULL) {
        pTabBar->onTabDeactivated(pTabBar, pOldActiveTab, pNewActiveTab);
    }

    if (pTabBar->onTabActivated && pNewActiveTab != NULL) {
        pTabBar->onTabActivated(pTabBar, pNewActiveTab, pOldActiveTab);
    }


    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

void dred_tabbar_activate_next_tab(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return;
    }

    if (pTabBar->pActiveTab == NULL) {
        dred_tabbar_activate_tab(pTabBar, pTabBar->pFirstTab);
        return;
    }


    dred_tab* pNextTab = pTabBar->pActiveTab->pNextTab;
    if (pNextTab == NULL) {
        pNextTab = pTabBar->pFirstTab;
    }

    dred_tabbar_activate_tab(pTabBar, pNextTab);
}

void dred_tabbar_activate_prev_tab(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return;
    }

    if (pTabBar->pActiveTab == NULL) {
        dred_tabbar_activate_tab(pTabBar, pTabBar->pLastTab);
        return;
    }


    dred_tab* pPrevTab = pTabBar->pActiveTab->pPrevTab;
    if (pPrevTab == NULL) {
        pPrevTab = pTabBar->pLastTab;
    }

    dred_tabbar_activate_tab(pTabBar, pPrevTab);
}

dred_tab* dred_tabbar_get_active_tab(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return NULL;
    }

    return pTabBar->pActiveTab;
}


dr_bool32 dred_tabbar_is_tab_in_view(dred_tabbar* pTabBar, dred_tab* pTabIn)
{
    if (pTabBar == NULL) {
        return DR_FALSE;
    }

    float tabbarWidth  = 0;
    float tabbarHeight = 0;
    dred_control_get_size(DRED_CONTROL(pTabBar), &tabbarWidth, &tabbarHeight);


    // Each tab.
    float runningPosX = 0;
    float runningPosY = 0;
    for (dred_tab* pTab = pTabBar->pFirstTab; pTab != NULL; pTab = pTab->pNextTab)
    {
        float tabWidth  = 0;
        float tabHeight = 0;
        dred_tabbar_measure_tab(pTabBar, pTab, &tabWidth, &tabHeight);

        if (pTab == pTabIn) {
            return runningPosX + tabWidth <= tabbarWidth && runningPosY + tabHeight <= tabbarHeight;
        }


        if (pTabBar->orientation == dred_tabbar_orientation_top || pTabBar->orientation == dred_tabbar_orientation_bottom) {
            runningPosX += tabWidth;
        } else {
            runningPosY += tabHeight;
        }
    }

    return DR_FALSE;
}


void dred_tabbar_show_close_buttons(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->isShowingCloseButton = DR_TRUE;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

void dred_tabbar_hide_close_buttons(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->isShowingCloseButton = DR_FALSE;

    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}


void dred_tabbar_enable_close_on_middle_click(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->isCloseOnMiddleClickEnabled = DR_TRUE;
}

void dred_tabbar_disable_close_on_middle_click(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return;
    }

    pTabBar->isCloseOnMiddleClickEnabled = DR_FALSE;
}

dr_bool32 dred_tabbar_is_close_on_middle_click_enabled(dred_tabbar* pTabBar)
{
    if (pTabBar == NULL) {
        return DR_FALSE;
    }

    return pTabBar->isCloseOnMiddleClickEnabled;
}


void dred_tabbar_on_mouse_leave(dred_control* pControl)
{
    dred_tabbar* pTabBar = DRED_TABBAR(pControl);
    if (pTabBar == NULL) {
        return;
    }

    if (pTabBar->pHoveredTab != NULL)
    {
        pTabBar->pHoveredTab = NULL;
        pTabBar->isCloseButtonHovered = DR_FALSE;

        dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
    }
}

void dred_tabbar_on_mouse_move(dred_control* pControl, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_tabbar* pTabBar = DRED_TABBAR(pControl);
    if (pTabBar == NULL) {
        return;
    }

    dr_bool32 isCloseButtonHovered = DR_FALSE;

    dred_tab* pOldHoveredTab = pTabBar->pHoveredTab;
    dred_tab* pNewHoveredTab = dred_tabbar_find_tab_under_point(pTabBar, (float)relativeMousePosX, (float)relativeMousePosY, &isCloseButtonHovered);

    if (pOldHoveredTab != pNewHoveredTab || pTabBar->isCloseButtonHovered != isCloseButtonHovered)
    {
        pTabBar->pHoveredTab = pNewHoveredTab;
        pTabBar->isCloseButtonHovered = isCloseButtonHovered;

        dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
    }
}

void dred_tabbar_on_mouse_button_down(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_tabbar* pTabBar = DRED_TABBAR(pControl);
    if (pTabBar == NULL) {
        return;
    }

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT || mouseButton == DTK_MOUSE_BUTTON_RIGHT)
    {
        dr_bool32 isOverCloseButton = DR_FALSE;

        dred_tab* pOldActiveTab = pTabBar->pActiveTab;
        dred_tab* pNewActiveTab = dred_tabbar_find_tab_under_point(pTabBar, (float)relativeMousePosX, (float)relativeMousePosY, &isOverCloseButton);

        if (pNewActiveTab != NULL && pOldActiveTab != pNewActiveTab && !isOverCloseButton) {
            dred_tabbar_activate_tab(pTabBar, pNewActiveTab);
        }

        if (isOverCloseButton && mouseButton == DTK_MOUSE_BUTTON_LEFT) {
            pTabBar->pTabWithCloseButtonPressed = pNewActiveTab;

            dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
        }
    }
    else if (mouseButton == DTK_MOUSE_BUTTON_MIDDLE)
    {
        if (pTabBar->isCloseOnMiddleClickEnabled)
        {
            dred_tab* pHoveredTab = dred_tabbar_find_tab_under_point(pTabBar, (float)relativeMousePosX, (float)relativeMousePosY, NULL);
            if (pHoveredTab != NULL) {
                if (pTabBar->onTabClose) {
                    pTabBar->onTabClose(pTabBar, pHoveredTab);
                }
            }
        }
    }
}

void dred_tabbar_on_mouse_button_up(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_tabbar* pTabBar = DRED_TABBAR(pControl);
    if (pTabBar == NULL) {
        return;
    }


    dr_bool32 releasedOverCloseButton = DR_FALSE;
    dred_tab* pTabUnderMouse = dred_tabbar_find_tab_under_point(pTabBar, (float)relativeMousePosX, (float)relativeMousePosY, &releasedOverCloseButton);

    if (pTabBar->pTabWithCloseButtonPressed != NULL && mouseButton == DTK_MOUSE_BUTTON_LEFT)
    {
        if (releasedOverCloseButton && pTabUnderMouse == pTabBar->pTabWithCloseButtonPressed) {
            if (pTabBar->onTabClose) {
                pTabBar->onTabClose(pTabBar, pTabBar->pTabWithCloseButtonPressed);
            }
        }


        pTabBar->pTabWithCloseButtonPressed = NULL;

        dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
    }
    else
    {
        if (!releasedOverCloseButton && pTabBar->onTabMouseButtonUp) {
            // TODO: Improve this by passing the mouse position relative to the tab. Currently it is relative to the tab BAR. Can have
            // the dred_tabbar_find_tab_under_point() function return the position relative to the tab.
            pTabBar->onTabMouseButtonUp(pTabBar, pTabUnderMouse, mouseButton, relativeMousePosX, relativeMousePosY, stateFlags);
        }
    }
}

void dred_tabbar_on_paint(dred_control* pControl, dred_rect relativeClippingRect, dtk_surface* pSurface)
{
    dred_tabbar* pTabBar = DRED_TABBAR(pControl);
    if (pTabBar == NULL) {
        return;
    }


    float tabbarWidth  = 0;
    float tabbarHeight = 0;
    dred_control_get_size(pControl, &tabbarWidth, &tabbarHeight);

    // Each tab.
    float runningPosX = 0;
    float runningPosY = 0;
    for (dred_tab* pTab = pTabBar->pFirstTab; pTab != NULL; pTab = pTab->pNextTab)
    {
        float tabWidth  = 0;
        float tabHeight = 0;
        dred_tabbar_measure_tab(pTabBar, pTab, &tabWidth, &tabHeight);

        // If a part of the tab is out of bounds, stop drawing.
        if (runningPosX > tabbarWidth || runningPosY > tabbarHeight) {
            break;
        }


        dred_tabbar_paint_tab(pTabBar, pTab, relativeClippingRect, runningPosX, runningPosY, tabWidth, tabHeight, pSurface);

        // After painting the tab, there may be a region of the background that was not drawn by the tab painting callback. We'll need to
        // draw that here.
        if (pTabBar->orientation == dred_tabbar_orientation_top || pTabBar->orientation == dred_tabbar_orientation_bottom) {
            dred_control_draw_rect(pControl, dred_make_rect(runningPosX, runningPosY + tabHeight, tabbarWidth, tabbarHeight), pTabBar->tabBackgroundColor, pSurface);
        } else {
            dred_control_draw_rect(pControl, dred_make_rect(runningPosX + tabWidth, runningPosY, tabbarWidth, runningPosY + tabHeight), pTabBar->tabBackgroundColor, pSurface);
        }



        if (pTabBar->orientation == dred_tabbar_orientation_top || pTabBar->orientation == dred_tabbar_orientation_bottom) {
            runningPosX += tabWidth;
        } else {
            runningPosY += tabHeight;
        }
    }


    // Background. We just draw a quad around the region that is not covered by items.
    dred_control_draw_rect(pControl, dred_make_rect(runningPosX, runningPosY, tabbarWidth, tabbarHeight), pTabBar->tabBackgroundColor, pSurface);
}


DRED_GUI_PRIVATE void dred_tabbar_on_measure_tab_default(dred_tabbar* pTabBar, dred_tab* pTab, float* pWidthOut, float* pHeightOut)
{
    if (pTabBar == NULL) {
        return;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pTabBar));

    float textWidth  = 0;
    float textHeight = 0;

    if (pTab != NULL) {
        dtk_font_measure_string(pTabBar->pFont, uiScale, pTab->text, strlen(pTab->text), &textWidth, &textHeight);
    }


    float closeButtonWidth  = 0;
    if (pTabBar->isShowingCloseButton && pTabBar->pCloseButtonImage != NULL) {
        unsigned int closeImageWidth;
        dred_gui_get_image_size(pTabBar->pCloseButtonImage, &closeImageWidth, NULL);

        closeButtonWidth  = closeImageWidth + pTabBar->closeButtonPaddingLeft;
    }


    if (pWidthOut) {
        *pWidthOut = textWidth + closeButtonWidth + pTabBar->tabPadding*2;
    }
    if (pHeightOut) {
        *pHeightOut = textHeight + pTabBar->tabPadding*2;
    }
}

DRED_GUI_PRIVATE void dred_tabbar_on_paint_tab_default(dred_tabbar* pTabBar, dred_tab* pTab, dred_rect relativeClippingRect, float offsetX, float offsetY, float width, float height, dtk_surface* pSurface)
{
    (void)relativeClippingRect;

    if (pTabBar == NULL) {
        return;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pTabBar));

    // Background.
    dtk_color bgcolor = pTabBar->tabBackgroundColor;
    dtk_color closeButtonColor = pTabBar->closeButtonColorDefault;
    dtk_color textColor = pTabBar->tabTextColor;

    if (pTabBar->pHoveredTab == pTab) {
        bgcolor = pTabBar->tabBackgroundColorHovered;
        closeButtonColor = pTabBar->closeButtonColorTabHovered;
        textColor = pTabBar->tabTextColorHovered;
    }
    if (pTabBar->pActiveTab == pTab) {
        bgcolor = pTabBar->tabBackbroundColorActivated;
        closeButtonColor = pTabBar->closeButtonColorTabHovered;
        textColor = pTabBar->tabTextColorActivated;
    }

    if (pTabBar->pHoveredTab == pTab && pTabBar->isCloseButtonHovered) {
        closeButtonColor = pTabBar->closeButtonColorHovered;

        if (pTabBar->pTabWithCloseButtonPressed == pTabBar->pHoveredTab) {
            closeButtonColor = pTabBar->closeButtonColorPressed;
        }
    }

    dred_control_draw_rect_outline(DRED_CONTROL(pTabBar), dred_make_rect(offsetX, offsetY, offsetX + width, offsetY + height), bgcolor, pTabBar->tabPadding, pSurface);


    // Text.
    float textPosX = offsetX + pTabBar->tabPadding;
    float textPosY = offsetY + pTabBar->tabPadding;
    if (pTab != NULL) {
        dred_control_draw_text(DRED_CONTROL(pTabBar), pTabBar->pFont, uiScale, pTab->text, (int)strlen(pTab->text), textPosX, textPosY, textColor, bgcolor, pSurface);
    }


    // Close button.
    if (pTabBar->isShowingCloseButton && pTabBar->pCloseButtonImage != NULL)
    {
        float textWidth  = 0;
        float textHeight = 0;
        if (pTab != NULL) {
            dtk_font_measure_string(pTabBar->pFont, uiScale, pTab->text, strlen(pTab->text), &textWidth, &textHeight);
        }

        float closeButtonPosX = textPosX + textWidth + pTabBar->closeButtonPaddingLeft;
        float closeButtonPosY = textPosY;

        unsigned int iconWidth;
        unsigned int iconHeight;
        dred_gui_get_image_size(pTabBar->pCloseButtonImage, &iconWidth, &iconHeight);

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
        args.dstBoundsHeight = height - (pTabBar->tabPadding*2);
        args.foregroundTint  = closeButtonColor;
        args.backgroundColor = bgcolor;
        args.boundsColor     = bgcolor;
        args.options         = DRED_GUI_IMAGE_DRAW_BOUNDS | DRED_GUI_IMAGE_CLIP_BOUNDS | DRED_GUI_IMAGE_ALIGN_CENTER;
        dred_control_draw_image(DRED_CONTROL(pTabBar), pTabBar->pCloseButtonImage, &args, pSurface);


        // Space between the text and the padding.
        dred_control_draw_rect(DRED_CONTROL(pTabBar), dred_make_rect(textPosX + textWidth, textPosY, closeButtonPosX, textPosY + textHeight), bgcolor, pSurface);
    }
}

DRED_GUI_PRIVATE dred_tab* dred_tabbar_find_tab_under_point(dred_tabbar* pTabBar, float relativePosX, float relativePosY, dr_bool32* pIsOverCloseButtonOut)
{
    if (pTabBar == NULL) {
        return NULL;
    }

    unsigned int closeButtonWidth;
    unsigned int closeButtonHeight;
    dred_gui_get_image_size(pTabBar->pCloseButtonImage, &closeButtonWidth, &closeButtonHeight);

    float runningPosX = 0;
    float runningPosY = 0;
    for (dred_tab* pTab = pTabBar->pFirstTab; pTab != NULL; pTab = pTab->pNextTab)
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
                closeButtonRect.left   = runningPosX + tabWidth - (pTabBar->tabPadding + closeButtonWidth);
                closeButtonRect.right  = closeButtonRect.left + closeButtonWidth;
                closeButtonRect.top    = runningPosY + (tabHeight - (pTabBar->tabPadding + closeButtonHeight))/2;
                closeButtonRect.bottom = closeButtonRect.top + closeButtonHeight;

                if (pTabBar->isShowingCloseButton && dred_rect_contains_point(closeButtonRect, relativePosX, relativePosY)) {
                    *pIsOverCloseButtonOut = DR_TRUE;
                } else {
                    *pIsOverCloseButtonOut = DR_FALSE;
                }
            }

            return pTab;
        }

        if (pTabBar->orientation == dred_tabbar_orientation_top || pTabBar->orientation == dred_tabbar_orientation_bottom) {
            runningPosX += tabWidth;
        } else {
            runningPosY += tabHeight;
        }
    }


    if (pIsOverCloseButtonOut) {
        *pIsOverCloseButtonOut = DR_FALSE;
    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// Tab
//
///////////////////////////////////////////////////////////////////////////////

/// Appends the given tab to the given tab bar.
DRED_GUI_PRIVATE void dred_tab_append(dred_tab* pTab, dred_tabbar* pTabBar);

/// Prepends the given tab to the given tab bar.
DRED_GUI_PRIVATE void dred_tab_prepend(dred_tab* pTab, dred_tabbar* pTabBar);

/// Detaches the given tab bar from it's tab bar element's hierarchy.
///
/// @remarks
///     This does not deactivate the tab or what - it only detaches the tab from the hierarchy.
DRED_GUI_PRIVATE void dred_tab_detach_from_hierarchy(dred_tab* pTab);

/// Detaches the given tab bar from it's tab bar element.
DRED_GUI_PRIVATE void dred_tab_detach(dred_tab* pTab);

DRED_GUI_PRIVATE dred_tab* dred_tabbar_create_tab_internal(dred_tabbar* pTabBar, const char* text, dred_control* pControl)
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
    dred_tab* pTab = (dred_tab*)dred_tabbar_create_tab_internal(pTabBar, text, pControl);
    if (pTab == NULL) {
        return NULL;
    }

    dred_tab_append(pTab, pTabBar);

    return pTab;
}

dred_tab* dred_tabbar_create_and_prepend_tab(dred_tabbar* pTabBar, const char* text, dred_control* pControl)
{
    dred_tab* pTab = (dred_tab*)dred_tabbar_create_tab_internal(pTabBar, text, pControl);
    if (pTab == NULL) {
        return NULL;
    }

    dred_tab_prepend(pTab, pTabBar);

    return pTab;
}

void dred_tab_delete(dred_tab* pTab)
{
    if (pTab == NULL) {
        return;
    }

    dred_tab_detach(pTab);
    free(pTab);
}

dred_tabbar* dred_tab_get_tab_bar(dred_tab* pTab)
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
    dred_control_dirty(DRED_CONTROL(pTab->pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTab->pTabBar)));
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

    dred_tab_detach_from_hierarchy(pTab);
    dred_tab_prepend(pTab, pTabBar);
}

dr_bool32 dred_tab_is_in_view(dred_tab* pTab)
{
    if (pTab == NULL) {
        return DR_FALSE;
    }

    return dred_tabbar_is_tab_in_view(pTab->pTabBar, pTab);
}

void dred_tab_move_into_view(dred_tab* pTab)
{
    if (!dred_tab_is_in_view(pTab)) {
        dred_tab_move_to_front(pTab);
    }
}


dred_tabgroup* dred_tab_get_tabgroup(dred_tab* pTab)
{
    if (pTab == NULL) {
        return NULL;
    }

    // The tab group is the parent of the tab bar.
    return DRED_TABGROUP(dtk_control_get_parent(DTK_CONTROL(pTab->pTabBar)));
}




DRED_GUI_PRIVATE void dred_tab_append(dred_tab* pTab, dred_tabbar* pTabBar)
{
    if (pTab == NULL || pTabBar == NULL) {
        return;
    }

    pTab->pTabBar = pTabBar;
    if (pTabBar->pFirstTab == NULL)
    {
        assert(pTabBar->pLastTab == NULL);

        pTabBar->pFirstTab = pTab;
        pTabBar->pLastTab  = pTab;
    }
    else
    {
        assert(pTabBar->pLastTab != NULL);

        pTab->pPrevTab = pTabBar->pLastTab;

        pTabBar->pLastTab->pNextTab = pTab;
        pTabBar->pLastTab = pTab;
    }


    if (pTabBar->isAutoSizeEnabled) {
        dred_tabbar_resize_by_tabs(pTabBar);
    }

    // The content of the menu has changed so we'll need to schedule a redraw.
    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

DRED_GUI_PRIVATE void dred_tab_prepend(dred_tab* pTab, dred_tabbar* pTabBar)
{
    if (pTab == NULL || pTabBar == NULL) {
        return;
    }

    pTab->pTabBar = pTabBar;
    if (pTabBar->pFirstTab == NULL)
    {
        assert(pTabBar->pLastTab == NULL);

        pTabBar->pFirstTab = pTab;
        pTabBar->pLastTab  = pTab;
    }
    else
    {
        assert(pTabBar->pLastTab != NULL);

        pTab->pNextTab = pTabBar->pFirstTab;

        pTabBar->pFirstTab->pPrevTab = pTab;
        pTabBar->pFirstTab = pTab;
    }


    if (pTabBar->isAutoSizeEnabled) {
        dred_tabbar_resize_by_tabs(pTabBar);
    }

    // The content of the menu has changed so we'll need to schedule a redraw.
    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}

DRED_GUI_PRIVATE void dred_tab_detach_from_hierarchy(dred_tab* pTab)
{
    if (pTab == NULL) {
        return;
    }

    dred_tabbar* pTabBar = pTab->pTabBar;
    if (pTabBar == NULL) {
        return;
    }

    if (pTab->pNextTab != NULL) {
        pTab->pNextTab->pPrevTab = pTab->pPrevTab;
    }

    if (pTab->pPrevTab != NULL) {
        pTab->pPrevTab->pNextTab = pTab->pNextTab;
    }


    if (pTab == pTabBar->pFirstTab) {
        pTabBar->pFirstTab = pTab->pNextTab;
    }

    if (pTab == pTabBar->pLastTab) {
        pTabBar->pLastTab = pTab->pPrevTab;
    }


    pTab->pNextTab   = NULL;
    pTab->pPrevTab   = NULL;
    pTab->pTabBar = NULL;
}

DRED_GUI_PRIVATE void dred_tab_detach(dred_tab* pTab)
{
    if (pTab == NULL) {
        return;
    }

    dred_tabbar* pTabBar = pTab->pTabBar;
    if (pTabBar == NULL) {
        return;
    }

    if (pTabBar->pHoveredTab == pTab) {
        pTabBar->pHoveredTab = NULL;
        pTabBar->isCloseButtonHovered = DR_FALSE;
    }

    if (pTabBar->pActiveTab == pTab) {
        pTabBar->pActiveTab = NULL;
    }

    if (pTabBar->pTabWithCloseButtonPressed == pTab) {
        pTabBar->pTabWithCloseButtonPressed = NULL;
    }


    dred_tab_detach_from_hierarchy(pTab);


    if (pTabBar->isAutoSizeEnabled) {
        dred_tabbar_resize_by_tabs(pTabBar);
    }

    // The content of the menu has changed so we'll need to schedule a redraw.
    dred_control_dirty(DRED_CONTROL(pTabBar), dred_control_get_local_rect(DRED_CONTROL(pTabBar)));
}
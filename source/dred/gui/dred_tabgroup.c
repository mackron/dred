// Copyright (C) 2017 David Reid. See included LICENSE file.

float dred_tabgroup__get_tabbar_height(dred_tabgroup* pTabGroup)
{
    assert(pTabGroup != NULL);
    return (dred_control_is_visible(DRED_CONTROL(pTabGroup->pTabBar))) ? dred_control_get_height(DRED_CONTROL(pTabGroup->pTabBar)) : 0;
}

void dred_tabgroup__resize_and_reposition_control(dred_control *pControl, float parentWidth, float parentHeight, float tabbarHeight)
{
    float controlPosX   = 0;
    float controlPosY   = tabbarHeight;
    float controlWidth  = parentWidth;
    float controlHeight = parentHeight - controlPosY;

    dred_control_set_relative_position(pControl, controlPosX, controlPosY);
    dred_control_set_size(pControl, controlWidth, controlHeight);
}

void dred_tabgroup__refresh_control_layout(dred_tabgroup* pTabGroup, dred_control* pControl)
{
    float parentWidth;
    float parentHeight;
    dred_control_get_size(DRED_CONTROL(pTabGroup), &parentWidth, &parentHeight);

    dred_tabgroup__resize_and_reposition_control(pControl, parentWidth, parentHeight, dred_tabgroup__get_tabbar_height(pTabGroup));
}

void dred_tabgroup__refresh_layout(dred_tabgroup* pTabGroup, float newWidth, float newHeight)
{
    assert(pTabGroup != NULL);

    dred_control_set_size(DRED_CONTROL(pTabGroup->pTabBar), newWidth, dred_control_get_height(DRED_CONTROL(pTabGroup->pTabBar)));

    float tabbarHeight = dred_tabgroup__get_tabbar_height(pTabGroup);
    for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
        dred_control* pControl = dred_tab_get_control(pTab);
        if (pControl != NULL) {
            dred_tabgroup__resize_and_reposition_control(pControl, newWidth, newHeight, tabbarHeight);
        }
    }
}

void dred_tabgroup__on_size(dred_control* pControl, float newWidth, float newHeight)
{
    dred_tabgroup* pTabGroup = DRED_TABGROUP(pControl);
    assert(pTabGroup != NULL);

    dred_tabgroup__refresh_layout(pTabGroup, newWidth, newHeight);
}

void dred_tabgroup__on_paint(dred_control* pControl, dred_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dred_tabgroup* pTabGroup = DRED_TABGROUP(pControl);
    assert(pTabGroup != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pTabGroup));
    assert(pDred != NULL);

    if (dred_tabgroup_get_active_tab(pTabGroup) == NULL)
    {
        // There is no active tab. Draw a blank background.
        dred_rect bodyRect = dred_control_get_local_rect(DRED_CONTROL(pTabGroup));
        bodyRect.top += dred_control_get_height(DRED_CONTROL(pTabGroup->pTabBar));

        dred_control_draw_rect(DRED_CONTROL(pTabGroup), bodyRect, pDred->config.tabgroupBGColor, pSurface);
    }
}


void dred_tabbar__on_tab_activated(dred_tabbar* pTabBar, dred_tab* pTab, dred_tab* pOldActiveTab)
{
#if 0
    // The tab group is the parent of the tab bar.
    dred_tabgroup* pTabGroup = DRED_TABGROUP(dtk_control_get_parent(DTK_CONTROL(pTabBar)));
    if (pTabGroup == NULL) {
        return;
    }

    dred_control* pControl = dred_tab_get_control(pTab);
    if (pControl == NULL) {
        return;
    }

    dred_control_show(pControl);
    dred_capture_keyboard(dred_control_get_context(pControl), pControl);

    dred_on_tab_activated(dred_control_get_context(DRED_CONTROL(pTabGroup)), pTab, pOldActiveTab);
#endif

    (void)pTabBar;
    (void)pTab;
    (void)pOldActiveTab;
}

void dred_tabbar__on_tab_deactivated(dred_tabbar* pTabBar, dred_tab* pTab, dred_tab* pNewActiveTab)
{
#if 0
    // The tab group is the parent of the tab bar.
    dred_tabgroup* pTabGroup = DRED_TABGROUP(dtk_control_get_parent(DTK_CONTROL(pTabBar)));
    if (pTabGroup == NULL) {
        return;
    }

    dred_control* pControl = dred_tab_get_control(pTab);
    if (pControl == NULL) {
        return;
    }

    dred_control_hide(pControl);
    if (dtk_control_has_keyboard_capture(DTK_CONTROL(pControl))) {
        dred_release_keyboard(dred_control_get_context(pControl));
    }

    dred_on_tab_deactivated(dred_control_get_context(DRED_CONTROL(pTabGroup)), pTab, pNewActiveTab);
#endif

    (void)pTabBar;
    (void)pTab;
    (void)pNewActiveTab;
}

void dred_tabbar__on_tab_close(dred_tabbar* pTabBar, dred_tab* pTab)
{
    (void)pTabBar;
    (void)pTab;

    //dred_tabgroup* pTabGroup = DRED_TABGROUP(dtk_control_get_parent(DTK_CONTROL(pTabBar)));
    //if (pTabGroup == NULL) {
    //    return;
    //}
    //
    //dred_close_tab_with_confirmation(dred_control_get_context(DRED_CONTROL(pTabGroup)), pTab);
}

void dred_tabbar__on_tab_mouse_button_up(dred_tabbar* pTabBar, dred_tab* pTab, int mouseButton, int mouseRelativePosX, int mouseRelativePosY, int stateFlags)
{
    (void)pTab;
    (void)stateFlags;

    dred_tabgroup* pTabGroup = DRED_TABGROUP(dtk_control_get_parent(DTK_CONTROL(pTabBar)));
    if (pTabGroup == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pTabGroup));
    if (pDred == NULL) {
        return;
    }

    if (mouseButton == DTK_MOUSE_BUTTON_RIGHT) {
        dred_control* pControl = dred_tab_get_control(pTab);
        if (pControl != NULL) {
            if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
                dred_control_show_popup_menu(DRED_CONTROL(pTabGroup), &pDred->menus.tabpopup, mouseRelativePosX, mouseRelativePosY);
            }
        }
    }
}


dtk_bool32 dred_tabgroup_init(dred_tabgroup* pTabGroup, dred_context* pDred, dred_control* pParent)
{
    if (pTabGroup == NULL) {
        return DTK_FALSE;
    }

    memset(pTabGroup, 0, sizeof(*pTabGroup));


    if (!dred_control_init(DRED_CONTROL(pTabGroup), pDred, pParent, NULL, DRED_CONTROL_TYPE_TABGROUP, NULL)) {
        return DTK_FALSE;
    }

    pTabGroup->pTabBar = &pTabGroup->tabBar;
    if (!dred_tabbar_init(pTabGroup->pTabBar, pDred, DRED_CONTROL(pTabGroup), dred_tabbar_orientation_top)) {
        dred_control_uninit(DRED_CONTROL(pTabGroup));
        return DTK_FALSE;
    }

    pTabGroup->pNextTabGroup = NULL;
    pTabGroup->pPrevTabGroup = NULL;

    dred_tabbar_enable_close_on_middle_click(pTabGroup->pTabBar);
    dred_tabbar_enable_auto_size(pTabGroup->pTabBar);
    dred_tabbar_set_on_tab_activated(pTabGroup->pTabBar, dred_tabbar__on_tab_activated);
    dred_tabbar_set_on_tab_deactivated(pTabGroup->pTabBar, dred_tabbar__on_tab_deactivated);
    dred_tabbar_set_on_tab_closed(pTabGroup->pTabBar, dred_tabbar__on_tab_close);
    dred_tabbar_set_on_tab_mouse_button_up(pTabGroup->pTabBar, dred_tabbar__on_tab_mouse_button_up);

    dred_tabgroup_refresh_styling(pTabGroup);


    // Events.
    dred_control_set_on_size(DRED_CONTROL(pTabGroup), dred_tabgroup__on_size);
    dred_control_set_on_paint(DRED_CONTROL(pTabGroup), dred_tabgroup__on_paint);


    if (!pDred->config.showTabBar) {
        dred_tabgroup_hide_tabbar(pTabGroup);
    }

    return DTK_TRUE;
}

void dred_tabgroup_uninit(dred_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return;
    }

    dred_tabbar_uninit(pTabGroup->pTabBar);
    dred_control_uninit(DRED_CONTROL(pTabGroup));
}


void dred_tabgroup_refresh_styling(dred_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pTabGroup));
    if (pDred == NULL) {
        return;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pTabGroup));


    // Update tab bar.
    dred_tabbar_set_tab_padding(pTabGroup->pTabBar, pDred->config.tabPadding * uiScale);
    dred_tabbar_set_tab_background_color(pTabGroup->pTabBar, pDred->config.tabBGColorInvactive);
    dred_tabbar_set_tab_background_color_active(pTabGroup->pTabBar, pDred->config.tabBGColorActive);
    dred_tabbar_set_tab_background_color_hovered(pTabGroup->pTabBar, pDred->config.tabBGColorHovered);
    dred_tabbar_set_font(pTabGroup->pTabBar, &pDred->config.tabFont->fontDTK);
    dred_tabbar_set_text_color(pTabGroup->pTabBar, pDred->config.tabTextColor);
    dred_tabbar_set_text_color_active(pTabGroup->pTabBar, pDred->config.tabTextColorActive);
    dred_tabbar_set_text_color_hovered(pTabGroup->pTabBar, pDred->config.tabTextColorHovered);
    dred_tabbar_set_close_button_left_padding(pTabGroup->pTabBar, 6 * uiScale);
    dred_tabbar_set_close_button_image(pTabGroup->pTabBar, dred_image_acquire_subimage(pDred->config.pImageCross, uiScale));
    dred_tabbar_set_close_button_color(pTabGroup->pTabBar, pDred->config.tabCloseButtonColor);
    if (pDred->config.tabShowCloseButton) {
        dred_tabbar_show_close_buttons(pTabGroup->pTabBar);
    } else {
        dred_tabbar_hide_close_buttons(pTabGroup->pTabBar);
    }


    // The size of some elements may have changed, so update the layout also.
    dred_tabgroup__refresh_layout(pTabGroup, dred_control_get_width(DRED_CONTROL(pTabGroup)), dred_control_get_height(DRED_CONTROL(pTabGroup)));

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pTabGroup), dred_control_get_local_rect(DRED_CONTROL(pTabGroup)));
}

void dred_tabgroup_get_body_size(dred_tabgroup* pTabGroup, float* pSizeXOut, float* pSizeYOut)
{
    if (pSizeXOut) *pSizeXOut = 0;
    if (pSizeYOut) *pSizeYOut = 0;

    if (pTabGroup == NULL) {
        return;
    }

    float parentSizeX;
    float parentSizeY;
    dred_control_get_size(DRED_CONTROL(pTabGroup), &parentSizeX, &parentSizeY);

    if (pSizeXOut) *pSizeXOut = parentSizeX;
    if (pSizeYOut) *pSizeYOut = parentSizeY - dred_tabgroup__get_tabbar_height(pTabGroup);
}


void dred_tabgroup_set_next_tabgroup(dred_tabgroup* pTabGroup, dred_tabgroup* pNextTabGroup)
{
    if (pTabGroup == NULL) {
        return;
    }

    pTabGroup->pNextTabGroup = pNextTabGroup;
}

void dred_tabgroup_set_prev_tabgroup(dred_tabgroup* pTabGroup, dred_tabgroup* pPrevTabGroup)
{
    if (pTabGroup == NULL) {
        return;
    }

    pTabGroup->pPrevTabGroup = pPrevTabGroup;
}

dred_tabgroup* dred_tabgroup_next_tabgroup(dred_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return NULL;
    }

    return pTabGroup->pNextTabGroup;
}

dred_tabgroup* dred_tabgroup_prev_tabgroup(dred_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return NULL;
    }

    return pTabGroup->pPrevTabGroup;
}


void dred_tabgroup_activate_tab(dred_tabgroup* pTabGroup, dred_tab* pTab)
{
    dred_tabbar_activate_tab(pTabGroup->pTabBar, pTab);
}

void dred_tabgroup_activate_next_tab(dred_tabgroup* pTabGroup)
{
    dred_tabbar_activate_next_tab(pTabGroup->pTabBar);
}

void dred_tabgroup_activate_prev_tab(dred_tabgroup* pTabGroup)
{
    dred_tabbar_activate_prev_tab(pTabGroup->pTabBar);
}

dred_tab* dred_tabgroup_get_active_tab(dred_tabgroup* pTabGroup)
{
    return dred_tabbar_get_active_tab(pTabGroup->pTabBar);
}


dred_tab* dred_tabgroup_first_tab(dred_tabgroup* pTabGroup)
{
    return dred_tabbar_get_first_tab(pTabGroup->pTabBar);
}

dred_tab* dred_tabgroup_last_tab(dred_tabgroup* pTabGroup)
{
    return dred_tabbar_get_last_tab(pTabGroup->pTabBar);
}

dred_tab* dred_tabgroup_next_tab(dred_tabgroup* pTabGroup, dred_tab* pTab)
{
    return dred_tabbar_get_next_tab(pTabGroup->pTabBar, pTab);
}

dred_tab* dred_tabgroup_prev_tab(dred_tabgroup* pTabGroup, dred_tab* pTab)
{
    return dred_tabbar_get_prev_tab(pTabGroup->pTabBar, pTab);
}


void dred_tabgroup__init_tab(dred_tabgroup* pTabGroup, dred_tab* pTab, dred_control* pControl)
{
    dred_tab_set_control(pTab, pControl);

    // The control needs to be re-parented the tab group. Also, the tab will not be active at this point, so make sure the
    // control is hidden.
    dred_control_hide(pControl);
    dred_control_append(pControl, DRED_CONTROL(pTabGroup));

    // Make sure the control is given the correct initial layout.
    dred_tabgroup__refresh_control_layout(pTabGroup, pControl);
}

dred_tab* dred_tabgroup_append_tab(dred_tabgroup* pTabGroup, const char* text, dred_control* pControl)
{
    dred_tab* pTab = dred_tabbar_create_and_append_tab(pTabGroup->pTabBar, text, pControl);
    if (pTab == NULL) {
        return NULL;
    }

    dred_tabgroup__init_tab(pTabGroup, pTab, pControl);
    return pTab;
}

dred_tab* dred_tabgroup_prepend_tab(dred_tabgroup* pTabGroup, const char* text, dred_control* pControl)
{
    dred_tab* pTab = dred_tabbar_create_and_prepend_tab(pTabGroup->pTabBar, text, pControl);
    if (pTab == NULL) {
        return NULL;
    }

    dred_tabgroup__init_tab(pTabGroup, pTab, pControl);
    return pTab;
}


void dred_tabgroup_delete_tab(dred_tabgroup* pTabGroup, dred_tab* pTab)
{
    // If the tab is active, activate the neigbouring tab before deleting anything.
    if (dred_tabgroup_get_active_tab(pTabGroup) == pTab)
    {
        if (pTab->pNextTab != NULL) {
            dred_tabgroup_activate_tab(pTabGroup, pTab->pNextTab);
        } else if (pTab->pPrevTab != NULL) {
            dred_tabgroup_activate_tab(pTabGroup, pTab->pPrevTab);
        } else {
            dred_tabgroup_activate_tab(pTabGroup, NULL);
        }
    }

    assert(dred_tabgroup_get_active_tab(pTabGroup) != pTab);

    dred_tab_delete(pTab);
}


void dred_tabgroup_hide_tabbar(dred_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return;
    }

    dred_control_hide(DRED_CONTROL(pTabGroup->pTabBar));
    dred_tabgroup__refresh_layout(pTabGroup, dred_control_get_width(DRED_CONTROL(pTabGroup)), dred_control_get_height(DRED_CONTROL(pTabGroup)));
}

void dred_tabgroup_show_tabbar(dred_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return;
    }

    dred_control_show(DRED_CONTROL(pTabGroup->pTabBar));
    dred_tabgroup__refresh_layout(pTabGroup, dred_control_get_width(DRED_CONTROL(pTabGroup)), dred_control_get_height(DRED_CONTROL(pTabGroup)));
}

dtk_bool32 dred_tabgroup_is_showing_tabbar(dred_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return DTK_FALSE;
    }

    return dred_control_is_visible(DRED_CONTROL(pTabGroup->pTabBar));
}

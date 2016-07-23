// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef struct
{
    dred_control* pTabBar;
    dred_tabgroup* pNextTabGroup;
    dred_tabgroup* pPrevTabGroup;
} dred_tabgroup_data;

dred_control* dred_tabgroup__get_tabbar(dred_tabgroup* pTabGroup)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data == NULL) {
        return NULL;
    }

    return data->pTabBar;
}

float dred_tabgroup__get_tabbar_height(dred_tabgroup* pTabGroup)
{
    dred_control* pTabBar = dred_tabgroup__get_tabbar(pTabGroup);
    if (pTabBar == NULL) {
        return 0;
    }

    return (dred_control_is_visible(pTabBar)) ? dred_control_get_height(pTabBar) : 0;
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
    dred_control_get_size(pTabGroup, &parentWidth, &parentHeight);

    dred_tabgroup__resize_and_reposition_control(pControl, parentWidth, parentHeight, dred_tabgroup__get_tabbar_height(pTabGroup));
}

void dred_tabgroup__refresh_layout(dred_tabgroup* pTabGroup, float newWidth, float newHeight)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    assert(data != NULL);

    dred_control_set_size(data->pTabBar, newWidth, dred_control_get_height(data->pTabBar));

    float tabbarHeight = dred_tabgroup__get_tabbar_height(pTabGroup);
    for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
        dred_control* pControl = dred_tab_get_control(pTab);
        if (pControl != NULL) {
            dred_tabgroup__resize_and_reposition_control(pControl, newWidth, newHeight, tabbarHeight);
        }
    }
}

void dred_tabgroup__on_size(dred_tabgroup* pTabGroup, float newWidth, float newHeight)
{
    dred_tabgroup__refresh_layout(pTabGroup, newWidth, newHeight);
}

void dred_tabgroup__on_paint(dred_tabgroup* pTabGroup, dred_rect rect, void* pPaintData)
{
    (void)rect;

    dred_context* pDred = dred_control_get_gui(pTabGroup);
    assert(pDred != NULL);

    if (dred_tabgroup_get_active_tab(pTabGroup) == NULL)
    {
        // There is no active tab. Draw a blank background.
        dred_rect bodyRect = dred_control_get_local_rect(pTabGroup);
        bodyRect.top += dred_control_get_height(dred_tabgroup__get_tabbar(pTabGroup));

        dred_control_draw_rect(pTabGroup, bodyRect, pDred->config.tabgroupBGColor, pPaintData);
    }
}


void dred_tabbar__on_tab_activated(dred_control* pTabBar, drgui_tab* pTab, drgui_tab* pOldActiveTab)
{
    // The tab group is the parent of the tab bar.
    dred_tabgroup* pTabGroup = dred_control_get_parent(pTabBar);
    if (pTabGroup == NULL) {
        return;
    }

    dred_control* pControl = dred_tab_get_control(pTab);
    if (pControl == NULL) {
        return;
    }

    dred_control_show(pControl);
    dred_capture_keyboard(dred_control_get_gui(pControl), pControl);

    dred_on_tab_activated(dred_control_get_gui(pTabGroup), pTab, pOldActiveTab);
}

void dred_tabbar__on_tab_deactivated(dred_control* pTabBar, drgui_tab* pTab, drgui_tab* pNewActiveTab)
{
    // The tab group is the parent of the tab bar.
    dred_tabgroup* pTabGroup = dred_control_get_parent(pTabBar);
    if (pTabGroup == NULL) {
        return;
    }

    dred_control* pControl = dred_tab_get_control(pTab);
    if (pControl == NULL) {
        return;
    }

    dred_control_hide(pControl);
    if (dred_control_has_keyboard_capture(pControl)) {
        dred_release_keyboard(dred_control_get_gui(pControl));
    }

    dred_on_tab_deactivated(dred_control_get_gui(pTabGroup), pTab, pNewActiveTab);
}

void dred_tabbar__on_tab_close(dred_control* pTabBar, drgui_tab* pTab)
{
    dred_tabgroup* pTabGroup = dred_control_get_parent(pTabBar);
    if (pTabGroup == NULL) {
        return;
    }

    dred_close_tab_with_confirmation(dred_control_get_gui(pTabGroup), (dred_tab*)pTab);
}

void dred_tabbar__on_tab_mouse_button_up(dred_control* pTabBar, drgui_tab* pTab, int mouseButton, int mouseRelativePosX, int mouseRelativePosY, int stateFlags)
{
    (void)pTab;
    (void)stateFlags;

    dred_tabgroup* pTabGroup = dred_control_get_parent(pTabBar);
    if (pTabGroup == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_gui(pTabGroup);
    if (pDred == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_RIGHT) {
        dred_control_show_popup_menu(pTabGroup, pDred->menuLibrary.pPopupMenu_Tab, mouseRelativePosX, mouseRelativePosY);
    }
}


dred_tabgroup* dred_tabgroup_create(dred_context* pDred, dred_control* pParent)
{
    dred_tabgroup* pTabGroup = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_TABGROUP, sizeof(dred_tabgroup_data));
    if (pTabGroup == NULL) {
        return NULL;
    }

    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    assert(data != NULL);

    data->pTabBar = dred_tabbar_create(pDred, pTabGroup, dred_tabbar_orientation_top, 0, NULL);
    if (data->pTabBar == NULL) {
        dred_control_delete(pTabGroup);
        return NULL;
    }

    data->pNextTabGroup = NULL;
    data->pPrevTabGroup = NULL;

    dred_tabbar_enable_close_on_middle_click(data->pTabBar);
    dred_tabbar_enable_auto_size(data->pTabBar);
    dred_tabbar_set_on_tab_activated(data->pTabBar, dred_tabbar__on_tab_activated);
    dred_tabbar_set_on_tab_deactivated(data->pTabBar, dred_tabbar__on_tab_deactivated);
    dred_tabbar_set_on_tab_closed(data->pTabBar, dred_tabbar__on_tab_close);
    dred_tabbar_set_on_tab_mouse_button_up(data->pTabBar, dred_tabbar__on_tab_mouse_button_up);

    dred_tabgroup_refresh_styling(pTabGroup);


    // Events.
    dred_control_set_on_size(pTabGroup, dred_tabgroup__on_size);
    dred_control_set_on_paint(pTabGroup, dred_tabgroup__on_paint);


    if (!pDred->config.showTabBar) {
        dred_tabgroup_hide_tabbar(pTabGroup);
    }

    return pTabGroup;
}

void dred_tabgroup_delete(dred_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return;
    }

    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data != NULL) {
        dred_tabbar_delete(data->pTabBar);
    }

    dred_control_delete(pTabGroup);
}


void dred_tabgroup_refresh_styling(dred_tabgroup* pTabGroup)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_gui(pTabGroup);
    if (pDred == NULL) {
        return;
    }


    // Update tab bar.
    dred_tabbar_set_tab_padding(data->pTabBar, pDred->config.tabPadding * pDred->uiScale);
    dred_tabbar_set_tab_background_color(data->pTabBar, pDred->config.tabBGColorInvactive);
    dred_tabbar_set_tab_background_color_active(data->pTabBar, pDred->config.tabBGColorActive);
    dred_tabbar_set_tab_background_color_hovered(data->pTabBar, pDred->config.tabBGColorHovered);
    dred_tabbar_set_font(data->pTabBar, dred_font_acquire_subfont(pDred->config.tabFont, pDred->uiScale));
    dred_tabbar_set_text_color(data->pTabBar, pDred->config.tabTextColor);
    dred_tabbar_set_text_color_active(data->pTabBar, pDred->config.tabTextColorActive);
    dred_tabbar_set_text_color_hovered(data->pTabBar, pDred->config.tabTextColorHovered);
    dred_tabbar_set_close_button_left_padding(data->pTabBar, 6 * pDred->uiScale);
    dred_tabbar_set_close_button_image(data->pTabBar, dred_image_acquire_subimage(pDred->config.pImageCross, pDred->uiScale));
    dred_tabbar_set_close_button_color(data->pTabBar, pDred->config.tabCloseButtonColor);
    if (pDred->config.tabShowCloseButton) {
        dred_tabbar_show_close_buttons(data->pTabBar);
    } else {
        dred_tabbar_hide_close_buttons(data->pTabBar);
    }


    // The size of some elements may have changed, so update the layout also.
    dred_tabgroup__refresh_layout(pTabGroup, dred_control_get_width(pTabGroup), dred_control_get_height(pTabGroup));

    // Redraw.
    dred_control_dirty(pTabGroup, dred_control_get_local_rect(pTabGroup));
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
    dred_control_get_size(pTabGroup, &parentSizeX, &parentSizeY);

    if (pSizeXOut) *pSizeXOut = parentSizeX;
    if (pSizeYOut) *pSizeYOut = parentSizeY - dred_tabgroup__get_tabbar_height(pTabGroup);
}


void dred_tabgroup_set_next_tabgroup(dred_tabgroup* pTabGroup, dred_tabgroup* pNextTabGroup)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data == NULL) {
        return;
    }

    data->pNextTabGroup = pNextTabGroup;
}

void dred_tabgroup_set_prev_tabgroup(dred_tabgroup* pTabGroup, dred_tabgroup* pPrevTabGroup)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data == NULL) {
        return;
    }

    data->pPrevTabGroup = pPrevTabGroup;
}

dred_tabgroup* dred_tabgroup_next_tabgroup(dred_tabgroup* pTabGroup)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data == NULL) {
        return NULL;
    }

    return data->pNextTabGroup;
}

dred_tabgroup* dred_tabgroup_prev_tabgroup(dred_tabgroup* pTabGroup)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data == NULL) {
        return NULL;
    }

    return data->pPrevTabGroup;
}


void dred_tabgroup_activate_tab(dred_tabgroup* pTabGroup, dred_tab* pTab)
{
    dred_tabbar_activate_tab(dred_tabgroup__get_tabbar(pTabGroup), pTab);
}

void dred_tabgroup_activate_next_tab(dred_tabgroup* pTabGroup)
{
    dred_tabbar_activate_next_tab(dred_tabgroup__get_tabbar(pTabGroup));
}

void dred_tabgroup_activate_prev_tab(dred_tabgroup* pTabGroup)
{
    dred_tabbar_activate_prev_tab(dred_tabgroup__get_tabbar(pTabGroup));
}

dred_tab* dred_tabgroup_get_active_tab(dred_tabgroup* pTabGroup)
{
    return dred_tabbar_get_active_tab(dred_tabgroup__get_tabbar(pTabGroup));
}


dred_tab* dred_tabgroup_first_tab(dred_tabgroup* pTabGroup)
{
    return dred_tabbar_get_first_tab(dred_tabgroup__get_tabbar(pTabGroup));
}

dred_tab* dred_tabgroup_last_tab(dred_tabgroup* pTabGroup)
{
    return dred_tabbar_get_last_tab(dred_tabgroup__get_tabbar(pTabGroup));
}

dred_tab* dred_tabgroup_next_tab(dred_tabgroup* pTabGroup, dred_tab* pTab)
{
    return dred_tabbar_get_next_tab(dred_tabgroup__get_tabbar(pTabGroup), pTab);
}

dred_tab* dred_tabgroup_prev_tab(dred_tabgroup* pTabGroup, dred_tab* pTab)
{
    return dred_tabbar_get_prev_tab(dred_tabgroup__get_tabbar(pTabGroup), pTab);
}


void dred_tabgroup__init_tab(dred_tabgroup* pTabGroup, dred_tab* pTab, dred_control* pControl)
{
    dred_tab_set_control(pTab, pControl);

    // The control needs to be re-parented the tab group. Also, the tab will not be active at this point, so make sure the
    // control is hidden.
    dred_control_hide(pControl);
    dred_control_append(pControl, pTabGroup);

    // Make sure the control is given the correct initial layout.
    dred_tabgroup__refresh_control_layout(pTabGroup, pControl);
}

dred_tab* dred_tabgroup_append_tab(dred_tabgroup* pTabGroup, const char* text, dred_control* pControl)
{
    dred_tab* pTab = dred_tab_create_and_append(dred_tabgroup__get_tabbar(pTabGroup), text, pControl);
    if (pTab == NULL) {
        return NULL;
    }

    dred_tabgroup__init_tab(pTabGroup, pTab, pControl);
    return pTab;
}

dred_tab* dred_tabgroup_prepend_tab(dred_tabgroup* pTabGroup, const char* text, dred_control* pControl)
{
    dred_tab* pTab = dred_tab_create_and_prepend(dred_tabgroup__get_tabbar(pTabGroup), text, pControl);
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
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data == NULL) {
        return;
    }

    dred_control_hide(data->pTabBar);
    dred_tabgroup__refresh_layout(pTabGroup, dred_control_get_width(pTabGroup), dred_control_get_height(pTabGroup));
}

void dred_tabgroup_show_tabbar(dred_tabgroup* pTabGroup)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data == NULL) {
        return;
    }

    dred_control_show(data->pTabBar);
    dred_tabgroup__refresh_layout(pTabGroup, dred_control_get_width(pTabGroup), dred_control_get_height(pTabGroup));
}

bool dred_tabgroup_is_showing_tabbar(dred_tabgroup* pTabGroup)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data == NULL) {
        return false;
    }

    return dred_control_is_visible(data->pTabBar);
}

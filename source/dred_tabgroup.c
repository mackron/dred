
typedef struct
{
    drgui_element* pTabBar;
} dred_tabgroup_data;

drgui_element* dred_tabgroup__get_tabbar(dred_tabgroup* pTabGroup)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data == NULL) {
        return NULL;
    }

    return data->pTabBar;
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

    dred_tabgroup__resize_and_reposition_control(pControl, parentWidth, parentHeight, dred_control_get_height(dred_tabgroup__get_tabbar(pTabGroup)));
}

void dred_tabgroup__on_size(dred_tabgroup* pTabGroup, float newWidth, float newHeight)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    assert(data != NULL);

    drgui_set_size(data->pTabBar, newWidth, drgui_get_height(data->pTabBar));

    float tabbarHeight = dred_control_get_height(dred_tabgroup__get_tabbar(pTabGroup));
    for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
        dred_control* pControl = dred_tab_get_control(pTab);
        if (pControl != NULL) {
            dred_tabgroup__resize_and_reposition_control(pControl, newWidth, newHeight, tabbarHeight);
        }
    }
}

void dred_tabgroup__on_paint(dred_tabgroup* pTabGroup, drgui_rect rect, void* pPaintData)
{
    (void)rect;

    if (dred_tabgroup_get_active_tab(pTabGroup) == NULL)
    {
        // There is no active tab. Draw a blank background.
        drgui_rect bodyRect = drgui_get_local_rect(pTabGroup);
        bodyRect.top += dred_control_get_height(dred_tabgroup__get_tabbar(pTabGroup));

        drgui_draw_rect(pTabGroup, bodyRect, drgui_rgb(48, 48, 48), pPaintData);
    }
}


void dred_tabbar__on_tab_activated(drgui_element* pTabBar, drgui_tab* pTab)
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
    dred_capture_keyboard(dred_control_get_context(pControl), pControl);
}

void dred_tabbar__on_tab_deactivated(drgui_element* pTabBar, drgui_tab* pTab)
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
        dred_release_keyboard(dred_control_get_context(pControl));
    }
}

void dred_tabbar__on_tab_close(drgui_element* pTabBar, drgui_tab* pTab)
{
    dred_tabgroup* pTabGroup = dred_control_get_parent(pTabBar);
    if (pTabGroup == NULL) {
        return;
    }

    dred_close_tab_with_confirmation(dred_control_get_context(pTabGroup), (dred_tab*)pTab);
}


dred_tabgroup* dred_tabgroup_create(dred_context* pDred, dred_control* pParent)
{
    dred_tabgroup* pTabGroup = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_TABGROUP, sizeof(dred_tabgroup_data));
    if (pTabGroup == NULL) {
        return NULL;
    }

    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    assert(data != NULL);

    data->pTabBar = drgui_create_tab_bar(pDred->pGUI, pTabGroup, drgui_tabbar_orientation_top, 0, NULL);
    if (data->pTabBar == NULL) {
        dred_control_delete(pTabGroup);
        return NULL;
    }

    drgui_tabbar_set_close_button_image(data->pTabBar, dred_image_acquire_subimage(pDred->config.pImageCross, 1));
    drgui_tabbar_show_close_buttons(data->pTabBar);
    drgui_tabbar_enable_close_on_middle_click(data->pTabBar);
    drgui_tabbar_enable_auto_size(data->pTabBar);
    drgui_tabbar_set_font(data->pTabBar, dred_font_acquire_subfont(pDred->config.pUIFont, 1));
    drgui_tabbar_set_on_tab_activated(data->pTabBar, dred_tabbar__on_tab_activated);
    drgui_tabbar_set_on_tab_deactivated(data->pTabBar, dred_tabbar__on_tab_deactivated);
    drgui_tabbar_set_on_tab_closed(data->pTabBar, dred_tabbar__on_tab_close);


    // Events.
    dred_control_set_on_size(pTabGroup, dred_tabgroup__on_size);
    dred_control_set_on_paint(pTabGroup, dred_tabgroup__on_paint);


    return pTabGroup;
}

void dred_tabgroup_delete(dred_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return;
    }

    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_extra_data(pTabGroup);
    if (data != NULL) {
        drgui_delete_tab_bar(data->pTabBar);
    }

    dred_control_delete(pTabGroup);
}


void dred_tabgroup_activate_tab(dred_tabgroup* pTabGroup, dred_tab* pTab)
{
    drgui_tabbar_activate_tab(dred_tabgroup__get_tabbar(pTabGroup), pTab);
}

dred_tab* dred_tabgroup_get_active_tab(dred_tabgroup* pTabGroup)
{
    return drgui_tabbar_get_active_tab(dred_tabgroup__get_tabbar(pTabGroup));
}


dred_tab* dred_tabgroup_first_tab(dred_tabgroup* pTabGroup)
{
    return drgui_tabbar_get_first_tab(dred_tabgroup__get_tabbar(pTabGroup));
}

dred_tab* dred_tabgroup_last_tab(dred_tabgroup* pTabGroup)
{
    return drgui_tabbar_get_last_tab(dred_tabgroup__get_tabbar(pTabGroup));
}

dred_tab* dred_tabgroup_next_tab(dred_tabgroup* pTabGroup, dred_tab* pTab)
{
    return drgui_tabbar_get_next_tab(dred_tabgroup__get_tabbar(pTabGroup), pTab);
}

dred_tab* dred_tabgroup_prev_tab(dred_tabgroup* pTabGroup, dred_tab* pTab)
{
    return drgui_tabbar_get_prev_tab(dred_tabgroup__get_tabbar(pTabGroup), pTab);
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

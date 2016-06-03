
typedef struct
{
    drgui_element* pTabBar;
} dred_tabgroup_data;

drgui_element* dred_tabgroup__get_tabbar(dred_tabgroup* pTabGroup)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_data(pTabGroup);
    if (data == NULL) {
        return NULL;
    }

    return data->pTabBar;
}

void dred_tabgroup__on_size(dred_tabgroup* pTabGroup, float newWidth, float newHeight)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_data(pTabGroup);
    assert(data != NULL);

    drgui_set_size(data->pTabBar, newWidth, 24);

    float controlPosX   = 0;
    float controlPosY   = dred_control_get_height(dred_tabgroup__get_tabbar(pTabGroup));
    float controlWidth  = newWidth;
    float controlHeight = newHeight - controlPosY;
    for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
        dred_control* pControl = dred_tab_get_control(pTab);
        if (pControl == NULL) {
            dred_control_set_relative_position(pControl, controlPosX, controlPosY);
            dred_control_set_size(pControl, controlWidth, controlHeight);
        }
    }
}

void dred_tabgroup__on_paint(dred_tabgroup* pTabGroup, drgui_rect rect, void* pPaintData)
{
    if (dred_tabgroup_get_active_tab(pTabGroup) == NULL)
    {
        // There is no active tab. Draw a blank background.
        drgui_rect bodyRect = drgui_get_local_rect(pTabGroup);
        bodyRect.top += dred_control_get_height(dred_tabgroup__get_tabbar(pTabGroup));

        drgui_draw_rect_with_outline(pTabGroup, bodyRect, drgui_rgb(48, 48, 48), 2, drgui_rgb(0, 0, 0), pPaintData);
    }
}

dred_tabgroup* dred_tabgroup_create(dred_context* pDred, dred_control* pParent)
{
    dred_tabgroup* pTabGroup = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_TABGROUP, sizeof(dred_tabgroup_data));
    if (pTabGroup == NULL) {
        return NULL;
    }

    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_data(pTabGroup);
    assert(data != NULL);

    data->pTabBar = drgui_create_tab_bar(pDred->pGUI, pTabGroup, drgui_tabbar_orientation_top, 0, NULL);
    if (data->pTabBar == NULL) {
        dred_control_delete(pTabGroup);
        return NULL;
    }

    //drgui_tabbar_show_close_buttons(data->pTabBar);
    drgui_tabbar_enable_close_on_middle_click(data->pTabBar);
    drgui_tabbar_set_font(data->pTabBar, pDred->pGUIFont);


    // Events.
    dred_control_set_on_size(pTabGroup, dred_tabgroup__on_size);
    dred_control_set_on_paint(pTabGroup, dred_tabgroup__on_paint);


    //dred_tabgroup_append_tab(pTabGroup, "Testing 1", NULL);
    //dred_tabgroup_append_tab(pTabGroup, "Testing 2", NULL);

    return pTabGroup;
}

void dred_tabgroup_delete(dred_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return;
    }

    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_data(pTabGroup);
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
    (void)pTabGroup;
    dred_tab_set_control(pTab, pControl);

    // The control needs to be re-parented the tab group. Also, the tab will not be active at this point, so make sure the
    // control is hidden.
    dred_control_hide(pControl);
    dred_control_append(pControl, pTabGroup);
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

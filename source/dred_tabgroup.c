
typedef struct
{
    drgui_element* pTabBar;
} dred_tabgroup_data;

void dred_tabgroup__on_size(dred_tabgroup* pTabGroup, float newWidth, float newHeight)
{
    dred_tabgroup_data* data = (dred_tabgroup_data*)dred_control_get_data(pTabGroup);
    assert(data != NULL);

    drgui_set_size(data->pTabBar, newWidth, 24);
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


    drgui_tabbar_create_and_append_tab(data->pTabBar, "Testing 1", 0, NULL);
    drgui_tabbar_create_and_append_tab(data->pTabBar, "Testing 2", 0, NULL);

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

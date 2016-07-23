// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef struct
{
    dred_control* pControl;
} dred_tab_data;


void dred_tab__init(dred_tab* pTab, dred_control* pControl)
{
    dred_tab_data* data = (dred_tab_data*)drgui_tab_get_extra_data(pTab);
    assert(data != NULL);

    data->pControl = pControl;
}

dred_tab* dred_tab_create_and_append(dred_control* pTabBar, const char* text, dred_control* pControl)
{
    drgui_tab* pTab = dred_tabbar_create_and_append_tab(pTabBar, text, sizeof(dred_tab_data), NULL);
    if (pTab == NULL) {
        return NULL;
    }

    dred_tab__init(pTab, pControl);
    return pTab;
}

dred_tab* dred_tab_create_and_prepend(dred_control* pTabBar, const char* text, dred_control* pControl)
{
    drgui_tab* pTab = dred_tabbar_create_and_prepend_tab(pTabBar, text, sizeof(dred_tab_data), NULL);
    if (pTab == NULL) {
        return NULL;
    }

    dred_tab__init(pTab, pControl);
    return pTab;
}

void dred_tab_delete(dred_tab* pTab)
{
    drgui_tab_delete(pTab);
}


void dred_tab_set_text(dred_tab* pTab, const char* text)
{
    drgui_tab_set_text(pTab, text);
}

const char* dred_tab_get_text(dred_tab* pTab)
{
    return drgui_tab_get_text(pTab);
}

void dred_tab_set_control(dred_tab* pTab, dred_control* pControl)
{
    dred_tab_data* data = (dred_tab_data*)drgui_tab_get_extra_data(pTab);
    if (data == NULL) {
        return;
    }

    data->pControl = pControl;
}

dred_control* dred_tab_get_control(dred_tab* pTab)
{
    dred_tab_data* data = (dred_tab_data*)drgui_tab_get_extra_data(pTab);
    if (data == NULL) {
        return NULL;
    }

    return data->pControl;
}


dred_control* dred_tab_get_tabgroup(dred_tab* pTab)
{
    dred_tab_data* data = (dred_tab_data*)drgui_tab_get_extra_data(pTab);
    if (data == NULL) {
        return NULL;
    }

    // The tab group is the parent of the tab bar.
    return drgui_get_parent(pTab->pTabBar);
}
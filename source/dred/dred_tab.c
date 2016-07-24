// Copyright (C) 2016 David Reid. See included LICENSE file.

dred_tab* dred_tab_create_and_append(dred_control* pTabBar, const char* text, dred_control* pControl)
{
    drgui_tab* pTab = dred_tabbar_create_and_append_tab(pTabBar, text, pControl, 0, NULL);
    if (pTab == NULL) {
        return NULL;
    }

    return pTab;
}

dred_tab* dred_tab_create_and_prepend(dred_control* pTabBar, const char* text, dred_control* pControl)
{
    drgui_tab* pTab = dred_tabbar_create_and_prepend_tab(pTabBar, text, pControl, 0, NULL);
    if (pTab == NULL) {
        return NULL;
    }

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

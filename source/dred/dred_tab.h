// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef drgui_tab dred_tab;

// Creates and appends a new tab.
dred_tab* dred_tab_create_and_append(dred_control* pTabBar, const char* text, dred_control* pControl);

// Creates and preprends a new tab.
dred_tab* dred_tab_create_and_prepend(dred_control* pTabBar, const char* text, dred_control* pControl);

// Deletes the given tab.
void dred_tab_delete(dred_tab* pTab);


// Sets the text of the given tab bar item.
void dred_tab_set_text(dred_tab* pTab, const char* text);

// Retrieves the text of the given tab.
const char* dred_tab_get_text(dred_tab* pTab);

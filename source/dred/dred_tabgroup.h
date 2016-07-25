// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_TABGROUP  "dred.tabgroup"

typedef struct dred_tabgroup dred_tabgroup;
#define DRED_TABGROUP(a) ((dred_tabgroup*)(a))

struct dred_tabgroup
{
    // The base control.
    dred_control control;

    dred_tabbar* pTabBar;
    dred_tabgroup* pNextTabGroup;
    dred_tabgroup* pPrevTabGroup;
};

// dred_tabgroup_create()
dred_tabgroup* dred_tabgroup_create(dred_context* pDred, dred_control* pParent);

// dred_tabgroup_delete()
void dred_tabgroup_delete(dred_tabgroup* pTabGroup);


// Refreshes the styling of the tab group.
void dred_tabgroup_refresh_styling(dred_tabgroup* pTabGroup);

// Calculates the size of the content area of the given tab group.
void dred_tabgroup_get_body_size(dred_tabgroup* pTabGroup, float* pSizeXOut, float* pSizeYOut);


// Sets the next tab group. This is just a simple setter.
void dred_tabgroup_set_next_tabgroup(dred_tabgroup* pTabGroup, dred_tabgroup* pNextTabGroup);

// Sets the previous tab group. This is just a simple setter.
void dred_tabgroup_set_prev_tabgroup(dred_tabgroup* pTabGroup, dred_tabgroup* pPrevTabGroup);

// dred_tabgroup_next_tabgroup()
dred_tabgroup* dred_tabgroup_next_tabgroup(dred_tabgroup* pTabGroup);

// dred_tabgroup_prev_tabgroup()
dred_tabgroup* dred_tabgroup_prev_tabgroup(dred_tabgroup* pTabGroup);


// dred_tabgroup_activate_tab()
void dred_tabgroup_activate_tab(dred_tabgroup* pTabGroup, dred_tab* pTab);

// Activates the tab to the right of the currently active tab, looping back to the start if necessary.
void dred_tabgroup_activate_next_tab(dred_tabgroup* pTabGroup);

// Activates the tab to the left of the currently active tab, looping back to the end if necessary.
void dred_tabgroup_activate_prev_tab(dred_tabgroup* pTabGroup);

// dred_tabgroup_get_active_tab()
dred_tab* dred_tabgroup_get_active_tab(dred_tabgroup* pTabGroup);


// Retrieves a pointer to the first tab in the given tab group.
dred_tab* dred_tabgroup_first_tab(dred_tabgroup* pTabGroup);

// Retrieves a pointer to the last tab in the given tab group.
dred_tab* dred_tabgroup_last_tab(dred_tabgroup* pTabGroup);

// Retrieves a pointer to the next tab in the given tab group.
dred_tab* dred_tabgroup_next_tab(dred_tabgroup* pTabGroup, dred_tab* pTab);

// Retrieves a pointer to the previous tab in the given tab group.
dred_tab* dred_tabgroup_prev_tab(dred_tabgroup* pTabGroup, dred_tab* pTab);


// pControl [in] A pointer to the control that will be associated with the tab.
//
// pControl will be re-parented upon being added to the group.
dred_tab* dred_tabgroup_append_tab(dred_tabgroup* pTabGroup, const char* text, dred_control* pControl);

// pControl [in] A pointer to the control that will be associated with the tab.
//
// pControl will be re-parented upon being added to the group.
dred_tab* dred_tabgroup_prepend_tab(dred_tabgroup* pTabGroup, const char* text, dred_control* pControl);


// Deletes the given tab from the tab group.
void dred_tabgroup_delete_tab(dred_tabgroup* pTabGroup, dred_tab* pTab);


// Hides the tab bar. The tab bar will still exist, only it'll be hidden from view.
void dred_tabgroup_hide_tabbar(dred_tabgroup* pTabGroup);

// Shows the tab bar.
void dred_tabgroup_show_tabbar(dred_tabgroup* pTabGroup);

// Determines whether or not the tab bar is being show.
bool dred_tabgroup_is_showing_tabbar(dred_tabgroup* pTabGroup);

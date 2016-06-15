
#define DRED_CONTROL_TYPE_TABGROUP  "dred.tabgroup"

typedef dred_control dred_tabgroup;

// dred_tabgroup_create()
dred_tabgroup* dred_tabgroup_create(dred_context* pDred, dred_control* pParent);

// dred_tabgroup_delete()
void dred_tabgroup_delete(dred_tabgroup* pTabGroup);


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

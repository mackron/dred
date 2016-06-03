
#define DRED_CONTROL_TYPE_TABGROUP  "dred.tabgroup"

typedef dred_control dred_tabgroup;

// dred_tabgroup_create()
dred_tabgroup* dred_tabgroup_create(dred_context* pDred, dred_control* pParent);

// dred_tabgroup_delete()
void dred_tabgroup_delete(dred_tabgroup* pTabGroup);
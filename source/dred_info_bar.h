// The info bar contains information about a specific type of control. It is contextual depending on the type.

#define DRED_CONTROL_TYPE_INFO_BAR   "dred.infobar"

typedef dred_control dred_info_bar;

// dred_info_bar_create()
dred_info_bar* dred_info_bar_create(dred_context* pDred, dred_control* pParent);

// dred_info_bar_delete()
void dred_info_bar_delete(dred_info_bar* pInfoBar);

// Updates the info bar based on the given control.
void dred_info_bar_update(dred_info_bar* pInfoBar, dred_control* pControl);
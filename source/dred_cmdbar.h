// The command bar is the control that sits along the bottom of the main window.

#define DRED_CONTROL_TYPE_CMDBAR   "dred.cmdbar"

typedef dred_control dred_cmdbar;

// dred_cmdbar_create()
dred_cmdbar* dred_cmdbar_create(dred_context* pDred, dred_control* pParent);

// dred_cmdbar_delete()
void dred_cmdbar_delete(dred_cmdbar* pCmdBar);
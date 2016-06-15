// The command bar is the control that sits along the bottom of the main window.

#define DRED_CONTROL_TYPE_CMDBAR   "dred.cmdbar"

typedef dred_control dred_cmdbar;

// dred_cmdbar_create()
dred_cmdbar* dred_cmdbar_create(dred_context* pDred, dred_control* pParent);

// dred_cmdbar_delete()
void dred_cmdbar_delete(dred_cmdbar* pCmdBar);

// Sets the text in the text box of the given command bar.
//
// This will move the caret to the end of the text.
void dred_cmdbar_set_text(dred_cmdbar* pCmdBar, const char* text);

// Determines whether or not the given command bar has the keyboard focus.
bool dred_cmdbar_has_keyboard_focus(dred_cmdbar* pCmdBar);

// Sets the message on the command bar.
void dred_cmdbar_set_message(dred_cmdbar* pCmdBar, const char* text);

// Clears the message on the command bar.
void dred_cmdbar_clear_message(dred_cmdbar* pCmdBar);
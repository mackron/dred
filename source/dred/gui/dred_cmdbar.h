// Copyright (C) 2016 David Reid. See included LICENSE file.

// The command bar is the control that sits along the bottom of the main window.

#define DRED_CONTROL_TYPE_CMDBAR   "dred.cmdbar"

typedef struct dred_cmdbar dred_cmdbar;
#define DRED_CMDBAR(a) ((dred_cmdbar*)(a))

struct dred_cmdbar
{
    // The base control.
    dred_control control;

    dred_context* pDred;    // TODO: Remove this.

    dred_textbox textBox;
    dred_textbox* pTextBox;

    char message[256];
    dred_info_bar infoBar;
    dred_info_bar* pInfoBar;
    unsigned int iPrevCommand;
    char* workingCommand;
};


// dred_cmdbar_create()
dr_bool32 dred_cmdbar_init(dred_cmdbar* pCmdBar, dred_context* pDred, dred_control* pParent);

// dred_cmdbar_delete()
void dred_cmdbar_uninit(dred_cmdbar* pCmdBar);

// Sets the text in the text box of the given command bar.
//
// This will move the caret to the end of the text.
void dred_cmdbar_set_text(dred_cmdbar* pCmdBar, const char* text);
dr_bool32 dred_cmdbar_set_text_to_previous_command(dred_cmdbar* pCmdBar, unsigned int iPrevCommand);

// Retrieves the text of the command bar.
size_t dred_cmdbar_get_text(dred_cmdbar* pCmdBar, char* pTextOut, size_t textOutSize);
char* dred_cmdbar_get_text_malloc(dred_cmdbar* pCmdBar);

// Determines whether or not the given command bar has the keyboard focus.
dr_bool32 dred_cmdbar_has_keyboard_focus(dred_cmdbar* pCmdBar);

// Sets the message on the command bar.
void dred_cmdbar_set_message(dred_cmdbar* pCmdBar, const char* text);

// Clears the message on the command bar.
void dred_cmdbar_clear_message(dred_cmdbar* pCmdBar);

// Updates the info bar based on the given control.
void dred_cmdbar_update_info_bar(dred_cmdbar* pCmdBar, dred_control* pControl);

// Refreshes the styling of the command bar.
void dred_cmdbar_refresh_styling(dred_cmdbar* pCmdBar);

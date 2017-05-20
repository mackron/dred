// Copyright (C) 2017 David Reid. See included LICENSE file.

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
    char* manualTextEntry;  // Used for the tab and arrow key autocomplete stuff.
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

// Retrieves the word under the cursor of the command bar.
size_t dred_cmdbar_get_word_under_cursor(dred_cmdbar* pCmdBar, char* pTextOut, size_t textOutSize);


// Selects a region of text in the command bar.
void dred_cmdbar_select_text(dred_cmdbar* pCmdBar, size_t firstCharacter, size_t lastCharacter);


// Highlights the next argument in the current text of the command bar. This will do nothing if there are no arguments.
void dred_cmdbar_highlight_next_argument(dred_cmdbar* pCmdBar);

// Highlights the previous argument in the current text of the command bar. This will do nothing if there are no arguments.
void dred_cmdbar_highlight_prev_argument(dred_cmdbar* pCmdBar);

// Highlights the value of the argument at the given index. This will do nothing if the index is not valid.
void dred_cmdbar_highlight_argument_by_index(dred_cmdbar* pCmdBar, unsigned int argumentIndex);

// Retrieves the index of the command that the cursor is currently sitting on. Returns -1 if it is not over any arguments.
unsigned int dred_cmdbar_get_argument_index_by_cursor(dred_cmdbar* pCmdBar);
unsigned int dred_cmdbar_get_argument_index_by_character_index(dred_cmdbar* pCmdBar, size_t characterIndex);

// Retrieves the number of commands currently sitting on the command bar.
unsigned int dred_cmdbar_get_argument_count(dred_cmdbar* pCmdBar);

// Retrieves the character range of the argument at the given index.
dr_bool32 dred_cmdbar_get_argument_character_range_by_index(dred_cmdbar* pCmdBar, unsigned int argumentIndex, size_t* pCharBeg, size_t* pCharEnd);


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

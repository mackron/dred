// Copyright (C) 2016 David Reid. See included LICENSE file.

// Commands are not usable until they are added to the list below. The pre-build stage will look at this file and auto-generate an
// optimized data structure for finding and executing commands based on the list below.

// BEGIN COMMAND LIST : <command name> <proc> <flags>
//
// !                        dred_command__system_command            DRED_CMDBAR_RELEASE_KEYBOARD
// cmdbar                   dred_command__cmdbar                    DRED_CMDBAR_NO_CLEAR
// bind                     dred_command__bind                      DRED_CMDBAR_RELEASE_KEYBOARD
// load-config              dred_command__load_config               DRED_CMDBAR_RELEASE_KEYBOARD
// set                      dred_command__set                       DRED_CMDBAR_RELEASE_KEYBOARD
// set-default              dred_command__set_default               DRED_CMDBAR_RELEASE_KEYBOARD
// show-menu-bar            dred_command__show_menu_bar             DRED_CMDBAR_RELEASE_KEYBOARD
// hide-menu-bar            dred_command__hide_menu_bar             DRED_CMDBAR_RELEASE_KEYBOARD
// toggle-menu-bar          dred_command__toggle_menu_bar           DRED_CMDBAR_RELEASE_KEYBOARD
// show-tab-bar             dred_command__show_tab_bar              DRED_CMDBAR_RELEASE_KEYBOARD
// hide-tab-bar             dred_command__hide_tab_bar              DRED_CMDBAR_RELEASE_KEYBOARD
// toggle-tab-bar           dred_command__toggle_tab_bar            DRED_CMDBAR_RELEASE_KEYBOARD
// enable-auto-hide-cmdbar  dred_command__enable_auto_hide_cmdbar   DRED_CMDBAR_RELEASE_KEYBOARD
// disable-auto-hide-cmdbar dred_command__disable_auto_hide_cmdbar  DRED_CMDBAR_RELEASE_KEYBOARD
// toggle-auto-hide-cmdbar  dred_command__toggle_auto_hide_cmdbar   DRED_CMDBAR_RELEASE_KEYBOARD
// next-tab                 dred_command__next_tab                  DRED_CMDBAR_RELEASE_KEYBOARD
// prev-tab                 dred_command__prev_tab                  DRED_CMDBAR_RELEASE_KEYBOARD
// cd                       dred_command__cd                        0
// new                      dred_command__new                       DRED_CMDBAR_RELEASE_KEYBOARD
// open                     dred_command__open                      DRED_CMDBAR_RELEASE_KEYBOARD
// save                     dred_command__save                      DRED_CMDBAR_RELEASE_KEYBOARD
// save-all                 dred_command__save_all                  DRED_CMDBAR_RELEASE_KEYBOARD
// save-as                  dred_command__save_as                   DRED_CMDBAR_RELEASE_KEYBOARD
// close                    dred_command__close                     DRED_CMDBAR_RELEASE_KEYBOARD
// close-all                dred_command__close_all                 DRED_CMDBAR_RELEASE_KEYBOARD
// exit                     dred_command__exit                      DRED_CMDBAR_NO_CLEAR
// quit                     dred_command__exit                      DRED_CMDBAR_NO_CLEAR
// help                     dred_command__help                      DRED_CMDBAR_RELEASE_KEYBOARD
// about                    dred_command__about                     DRED_CMDBAR_RELEASE_KEYBOARD
// settings                 dred_command__settings                  DRED_CMDBAR_RELEASE_KEYBOARD
// print                    dred_command__print                     DRED_CMDBAR_RELEASE_KEYBOARD
// reload                   dred_command__reload                    DRED_CMDBAR_RELEASE_KEYBOARD
// undo                     dred_command__undo                      DRED_CMDBAR_NO_CLEAR
// redo                     dred_command__redo                      DRED_CMDBAR_NO_CLEAR
// cut                      dred_command__cut                       DRED_CMDBAR_NO_CLEAR
// copy                     dred_command__copy                      DRED_CMDBAR_NO_CLEAR
// paste                    dred_command__paste                     DRED_CMDBAR_NO_CLEAR
// delete                   dred_command__delete                    DRED_CMDBAR_NO_CLEAR
// select-all               dred_command__select_all                DRED_CMDBAR_NO_CLEAR
// goto                     dred_command__goto                      DRED_CMDBAR_RELEASE_KEYBOARD
// find                     dred_command__find                      DRED_CMDBAR_NO_CLEAR
// replace                  dred_command__replace                   DRED_CMDBAR_NO_CLEAR
// replace-all              dred_command__replace_all               DRED_CMDBAR_RELEASE_KEYBOARD
// show-line-numbers        dred_command__show_line_numbers         DRED_CMDBAR_RELEASE_KEYBOARD
// hide-line-numbers        dred_command__hide_line_numbers         DRED_CMDBAR_RELEASE_KEYBOARD
// toggle-line-numbers      dred_command__toggle_line_numbers       DRED_CMDBAR_RELEASE_KEYBOARD
// toggle-word-wrap         dred_command__toggle_word_wrap          DRED_CMDBAR_RELEASE_KEYBOARD
// zoom                     dred_command__zoom                      DRED_CMDBAR_RELEASE_KEYBOARD
// unindent                 dred_command__unindent                  DRED_CMDBAR_RELEASE_KEYBOARD
// insert-date              dred_command__insert_date               DRED_CMDBAR_RELEASE_KEYBOARD
// export2cstring           dred_command__export2cstring            DRED_CMDBAR_RELEASE_KEYBOARD
// add-favourite            dred_command__add_favourite             DRED_CMDBAR_RELEASE_KEYBOARD
// remove-favourite         dred_command__remove_favourite          DRED_CMDBAR_RELEASE_KEYBOARD
//
// END COMMAND LIST

#define DRED_CMDBAR_RELEASE_KEYBOARD    1   // Releases the keyboard if the command was executed from the command bar.
#define DRED_CMDBAR_NO_CLEAR            2   // Prevents the command bar from being cleared if the command was executed from the command bar.

typedef dr_bool32 (* dred_command_proc) (dred_context* pDred, const char* value);

typedef enum
{
    dred_command_separator_none,
    dred_command_separator_semicolon,
    dred_command_separator_and,
    dred_command_separator_or,
} dred_command_separator;

struct dred_command
{
    dred_command_proc proc;
    unsigned int flags;
};


//// General ////

// Handles the "!" command.
dr_bool32 dred_command__system_command(dred_context* pDred, const char* value);

// cmdbar
//
// This command puts keyboard focus onto the command bar and sets the text in the text box to the value.
dr_bool32 dred_command__cmdbar(dred_context* pDred, const char* value);

// bind
//
// Syntax:  bind <name> <shortcut> <command>
// Example: bind "SelectAll" "CTRL+A" select-all
dr_bool32 dred_command__bind(dred_context* pDred, const char* value);

// load-config
//
// Loads and applies a config file.
dr_bool32 dred_command__load_config(dred_context* pDred, const char* value);

// set
//
// Sets a config variable.
dr_bool32 dred_command__set(dred_context* pDred, const char* value);

// set
//
// Sets a config variable to it's default value.
dr_bool32 dred_command__set_default(dred_context* pDred, const char* value);


// show-menu-bar
dr_bool32 dred_command__show_menu_bar(dred_context* pDred, const char* value);

// hide-menu-bar
dr_bool32 dred_command__hide_menu_bar(dred_context* pDred, const char* value);

// toggle-menu-bar
//
// Toggles the visibility of the menu bar.
dr_bool32 dred_command__toggle_menu_bar(dred_context* pDred, const char* value);


// show-tabbars
dr_bool32 dred_command__show_tab_bar(dred_context* pDred, const char* value);

// hide-tabbars
dr_bool32 dred_command__hide_tab_bar(dred_context* pDred, const char* value);

// toggle-tabbars
//
// Toggles the visibility of the tab bars on each tab group.
dr_bool32 dred_command__toggle_tab_bar(dred_context* pDred, const char* value);


// enable-auto-hide-cmdbar
dr_bool32 dred_command__enable_auto_hide_cmdbar(dred_context* pDred, const char* value);

// disable-auto-hide-cmdbar
dr_bool32 dred_command__disable_auto_hide_cmdbar(dred_context* pDred, const char* value);

// toggle-auto-hide-cmdbar
dr_bool32 dred_command__toggle_auto_hide_cmdbar(dred_context* pDred, const char* value);


// next-tab
dr_bool32 dred_command__next_tab(dred_context* pDred, const char* value);

// prev-tab
dr_bool32 dred_command__prev_tab(dred_context* pDred, const char* value);


// cd
dr_bool32 dred_command__cd(dred_context* pDred, const char* value);



// new
dr_bool32 dred_command__new(dred_context* pDred, const char* value);

// open
dr_bool32 dred_command__open(dred_context* pDred, const char* value);

// save
dr_bool32 dred_command__save(dred_context* pDred, const char* value);

// save-all
dr_bool32 dred_command__save_all(dred_context* pDred, const char* value);

// save-as
dr_bool32 dred_command__save_as(dred_context* pDred, const char* value);

// close
dr_bool32 dred_command__close(dred_context* pDred, const char* value);

// close-all
dr_bool32 dred_command__close_all(dred_context* pDred, const char* value);

// exit
dr_bool32 dred_command__exit(dred_context* pDred, const char* value);

// help
dr_bool32 dred_command__help(dred_context* pDred, const char* value);

// about
dr_bool32 dred_command__about(dred_context* pDred, const char* value);

// settings
dr_bool32 dred_command__settings(dred_context* pDred, const char* value);

// print
dr_bool32 dred_command__print(dred_context* pDred, const char* value);

// reload
//
// Reloads the currently focused file.
dr_bool32 dred_command__reload(dred_context* pDred, const char* value);

// add-favourite
dr_bool32 dred_command__add_favourite(dred_context* pDred, const char* value);

// remove-favourite
dr_bool32 dred_command__remove_favourite(dred_context* pDred, const char* value);



//// General Editing ////

// undo
dr_bool32 dred_command__undo(dred_context* pDred, const char* value);

// redo
dr_bool32 dred_command__redo(dred_context* pDred, const char* value);

// cut
dr_bool32 dred_command__cut(dred_context* pDred, const char* value);

// copy
dr_bool32 dred_command__copy(dred_context* pDred, const char* value);

// paste
dr_bool32 dred_command__paste(dred_context* pDred, const char* value);

// delete
dr_bool32 dred_command__delete(dred_context* pDred, const char* value);

// select-all
dr_bool32 dred_command__select_all(dred_context* pDred, const char* value);



//// Text Editor ////

// goto
dr_bool32 dred_command__goto(dred_context* pDred, const char* value);

// find
dr_bool32 dred_command__find(dred_context* pDred, const char* value);

// replace
dr_bool32 dred_command__replace(dred_context* pDred, const char* value);

// replace-all
dr_bool32 dred_command__replace_all(dred_context* pDred, const char* value);

// show-line-numbers
dr_bool32 dred_command__show_line_numbers(dred_context* pDred, const char* value);

// hide-line-numbers
dr_bool32 dred_command__hide_line_numbers(dred_context* pDred, const char* value);

// toggle-line-numbers
dr_bool32 dred_command__toggle_line_numbers(dred_context* pDred, const char* value);

// toggle-word-wrap
dr_bool32 dred_command__toggle_word_wrap(dred_context* pDred, const char* value);

// zoom
dr_bool32 dred_command__zoom(dred_context* pDred, const char* value);

// unindent
dr_bool32 dred_command__unindent(dred_context* pDred, const char* value);

// insert-date
dr_bool32 dred_command__insert_date(dred_context* pDred, const char* value);

// export2cstring
dr_bool32 dred_command__export2cstring(dred_context* pDred, const char* value);




// Finds the index of a command.
dr_bool32 dred_find_command(const char* cmdStr, dred_command* pCommandOut, const char** pValueOut);

// Finds the index of the given command function. Returns -1 if the command could not be found.
size_t dred_find_command_index(const char* cmdFunc);

// Finds a list of commands that begin with the given string. Returns the count.
size_t dred_find_commands_starting_with(size_t* pIndexOut, size_t sizeOut, const char* cmdFunc);


// Retrieves the next sub-command in the given command string.
const char* dred_next_command_string(const char* cmdStr, char* cmdOut, size_t cmdOutSize, dred_command_separator* pSeparatorOut);


// Helper function for parsing the "bind" command.
dr_bool32 dred_parse_bind_command(const char* value, char* nameOut, size_t nameOutSize, dred_shortcut* pShortcutOut, char* pCmdOut, size_t cmdOutSize);
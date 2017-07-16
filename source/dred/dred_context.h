// Copyright (C) 2017 David Reid. See included LICENSE file.

struct dred_context
{
    // The command line that was passed to dred_init.
    dr_cmdline cmdline;

    // A pointer to the library of loaded packages. This is loaded before the context is created and passed in
    // as an argument to dred_init().
    dred_package_library* pPackageLibrary;

    // The log file.
    dred_file logFile;

    // The IPC thread.
    dtk_thread threadIPC;

    // The main toolkit context. This needs to be initialized before doing pretty much anything.
    dtk_context tk;

    // The string pool for infrequently changed, but dynamically allocated strings. Strings stored in this
    // pool include:
    // - Shortcut ID strings
    // - Menu item command strings
    dred_string_pool stringPool;


    // The main GUI context.
    dred_gui gui;
    dred_gui* pGUI; // <-- This is always set to &gui and exists for convenience.   // TODO: Remove this.


    // The font library. This just manages fonts to make it easier to avoid loading duplicate fonts.
    dred_font_library fontLibrary;

    // The image library.
    dred_image_library imageLibrary;


    // The menus.
    dred_stock_menus menus;

    // The menu item binding table. This is used for binding menu items to dred-specific data.
    dred_menu_item_table menuItemTable;


    // The shortcut table.
    dred_shortcut_table shortcutTable;

    // The queued accelerator for when the user is doing a double-accelerator shortcut.
    dtk_accelerator queuedAccelerator;


    // The main config.
    dred_config config;


    // The main window.
    dtk_window mainWindow;

    // The menu currently being shown on the main window. This will be set to one of the menus in the "menus" property.
    dtk_menu* pMainMenu;


    // The about window.
    dred_about_dialog aboutDialog;
    dred_about_dialog* pAboutDialog;        // Lazily initialized. Set to NULL by default, and then set to &aboutDialog after it's been initialized.

    // The settings dialog window.
    dred_settings_dialog settingsDialog;
    dred_settings_dialog* pSettingsDialog;  // Lazily initialized. Set to NULL by default, and then set to &aboutDialog after it's been initialized.


    // The background control that's displayed when there is nothing open.
    dtk_control backgroundControl;

    // The main tab group container. This is the root container where all other sub-containers will be placed.
    dred_tabgroup_container mainTabGroupContainer;

    // The main tab group. This is actually temporary until support for multiple tab groups (splitting) is implemented.
    dred_tabgroup mainTabGroup;

    // The command bar. This is is the control that runs along the bottom of the main window.
    dred_cmdbar cmdBar;

    // The command bar auto-complete popup window.
    dred_cmdbar_popup cmdbarPopup;


    
    dtk_bool32 isInitialized               : 1; // Whether or not the context is initialized.
    dtk_bool32 isClosing                   : 1; // Whether or not the context is closing. This is used to terminate loops.
    dtk_bool32 isTerminalOutputDisabled    : 1; // Whether or not the application is running in silent mode.
    dtk_bool32 isPortable                  : 1; // Whether or not we're running in portable mode.
    dtk_bool32 isShowingMainMenu           : 1; // Whether or not the main menu is being shown.
};

// dred_init
dr_bool32 dred_init(dred_context* pDred, dr_cmdline cmdline, dred_package_library* pPackageLibrary);

// dred_uninit
void dred_uninit(dred_context* pDred);


// dred_run
int dred_run(dred_context* pDred);

// Closes the application as if the user has clicked the close button on the main window.
void dred_close(dred_context* pDred);


// Posts a log message.
void dred_log(dred_context* pDred, const char* message);

// Posts a formatted log message.
void dred_logf(dred_context* pDred, const char* format, ...);

// Posts a warning log message.
void dred_warning(dred_context* pDred, const char* message);

// Posts a formatted warning log message.
void dred_warningf(dred_context* pDred, const char* format, ...);

// Posts an error log message.
void dred_error(dred_context* pDred, const char* message);

// Posts a formatted error log message.
void dred_errorf(dred_context* pDred, const char* format, ...);


// Loads a config file.
dr_bool32 dred_load_config(dred_context* pDred, const char* configFilePath);


// Executes a command.
dr_bool32 dred_exec(dred_context* pDred, const char* cmd, dred_command* pLastCmdOut);

// Binds a shortcut.
dr_bool32 dred_bind_shortcut(dred_context* pDred, dtk_uint32 id, const char* name, const char* cmdStr, dtk_uint32 acceleratorCount, dtk_accelerator* pAccelerators);


// Saves the .dredprivate file.
void dred_save_dredprivate(dred_context* pDred);


// Gives the given control the keyboard capture.
void dred_capture_keyboard(dred_context* pDred, dred_control* pControl);

// Releases the keyboard capture.
void dred_release_keyboard(dred_context* pDred);


// Retrieves a pointer to the first tab group.
dred_tabgroup* dred_first_tabgroup(dred_context* pDred);

// Retrieves a pointer to the next tab group.
dred_tabgroup* dred_next_tabgroup(dred_context* pDred, dred_tabgroup* pTabGroup);


// Retrieves the focused tab group.
//
// The focused tab group is where newly opened files will be placed by default.
dred_tabgroup* dred_get_focused_tabgroup(dred_context* pDred);

// Retrieves the focused tab.
//
// The focused tab is just the active tab of the focused tab group.
dred_tab* dred_get_focused_tab(dred_context* pDred);

// Retrieves the focused editor.
//
// The focused editor is the control of the focused tab. If the control is not of type DRED_CONTROL_TYPE_EDITOR, NULL will be returned.
dred_editor* dred_get_focused_editor(dred_context* pDred);

// Retrieves the element with the keyboard capture, if any.
dtk_control* dred_get_control_with_keyboard_capture(dred_context* pDred);


// Retrieves the control type of the editor to use for a file with the extension of the given file path.
const char* dred_get_editor_type_by_path(dred_context* pDred, const char* filePath);

// Finds the tab of the editor tied to the given absolute path.
dred_tab* dred_find_editor_tab_by_absolute_path(dred_context* pDred, const char* filePathAbsolute);


// Opens the file at the given path.
dr_bool32 dred_open_file(dred_context* pDred, const char* filePath);

// Opens the file at the given path as the given type.
dr_bool32 dred_open_file_by_type(dred_context* pDred, const char* filePath, const char* editorType);

// Closes the focused file.
void dred_close_focused_file(dred_context* pDred);

// Closes the focused file and shows a confirmation dialog if it's modified.
void dred_close_focused_file_with_confirmation(dred_context* pDred);

// Closes the given tab.
void dred_close_tab(dred_context* pDred, dred_tab* pTab);

// Closes the given tab, but shows a confirmation dialog if it's not currently saved.
void dred_close_tab_with_confirmation(dred_context* pDred, dred_tab* pTab);

// Closes every tab.
void dred_close_all_tabs(dred_context* pDred);

// Closes every tab with a confirmation dialog if there is at least one that's modified.
//
// This will show a single dialog which represents every modified file. Hitting cancel on this dialog will
// result in _none_ of the files being closed.
//
// True will be returned if every tab was closed. False will be returned if the tabs were not closed.
dr_bool32 dred_close_all_tabs_with_confirmation(dred_context* pDred);


// Finds the tab associated with the given control, usually an editor.
dred_tab* dred_find_control_tab(dred_control* pControl);


// Saves the currently focused file.
//
// The new file name can be null in which case the file is just saved over the top of the current file.
dr_bool32 dred_save_focused_file(dred_context* pDred, const char* newFilePath);

// Opens save dialog box and gives the user the opportunity to save the focused file as a different name.
dr_bool32 dred_save_focused_file_as(dred_context* pDred);

// Saves every open file, skipping those that would otherwise require a save-as dialog.
void dred_save_all_open_files(dred_context* pDred);

// Saves every open file and opens are save-as dialog for any applicable files. This will return DR_FALSE if any
// files could not be saved.
dr_bool32 dred_save_all_open_files_with_saveas(dred_context* pDred);


// Creates and opens a new file.
//
// The newFilePath argument can be null, in which case it simply creates an empty text file.
dr_bool32 dred_create_and_open_file(dred_context* pDred, const char* newFilePath);

// Opens an empty text file.
dr_bool32 dred_open_new_text_file(dred_context* pDred);


// Creates an editor by it's type.
dred_editor* dred_create_editor_by_type(dred_context* pDred, dred_tabgroup* pTabGroup, const char* editorType, const char* filePathAbsolute);

// Deletes the given editor based on it's type.
void dred_delete_editor_by_type(dred_editor* pEditor);


// Determines whether or not any open files are modified.
dr_bool32 dred_are_any_open_files_modified(dred_context* pDred);


// Reloads the focused file.
dr_bool32 dred_reload_focused_file(dred_context* pDred);

// Checks if the focused file is dirty and attempts to reload it if so.
dr_bool32 dred_check_if_focused_file_is_dirty_and_reload(dred_context* pDred);


// Show the Open File dialog.
dtk_dialog_result dred_show_open_file_dialog(dred_context* pDred);

// Shows the Save File dialog.
dr_bool32 dred_show_save_file_dialog(dred_context* pDred, const char* currentFilePath, char* absolutePathOut, size_t absolutePathOutSize);

// Shows a yes/no/cancel dialog and returns the result as DTK_DIALOG_RESULT_YES, DTK_DIALOG_RESULT_NO or DTK_DIALOG_RESULT_CANCEL.
dtk_dialog_result dred_show_yesnocancel_dialog(dred_context* pDred, const char* message, const char* title);

// Shows the font picker dialog. Returns DR_TRUE if the user chose a font, DR_FALSE if they hit the cancel button or an error occurs.
dr_bool32 dred_show_font_picker_dialog(dred_context* pDred, dtk_window* pOwnerWindow, const dred_font_desc* pDefaultFontDesc, dred_font_desc* pDescOut);

// Shows the color picker dialog. Returns DR_TRUE if the user chose a color, DR_FALSE if they hit the cancel button or an error occurs.
dtk_dialog_result dred_show_color_picker_dialog(dred_context* pDred, dtk_window* pOwnerWindow, dtk_color initialColor, dtk_color* pColorOut);

// Shows the "Print" dialog box.
//
// This is a little different to other platform-specific dialog boxes in that it performs the full printing operation from start
// to finish.
dr_bool32 dred_show_print_dialog(dred_context* pDred, dtk_window* pOwnerWindow, dred_print_info* pInfoOut);



// Shows the "About" dialog box.
void dred_show_about_dialog(dred_context* pDred);

// Shows the "Settings" dialog box.
void dred_show_settings_dialog(dred_context* pDred);




// Updates the layout of the main window.
//
// This will be called whenever something changes that would result in the main window's layout becoming invalid, such as changes
// to the size or visibility state of the command bar.
void dred_update_main_window_layout(dred_context* pDred);

// Updates the layout and styling of the entire UI. This will be called for global changes to the UI such as changes to DPI scaling.
void dred_refresh_layout(dred_context* pDred);


// Sets the text of the command bar.
void dred_set_command_bar_text(dred_context* pDred, const char* text);

// Places keyboard focus on the command bar.
void dred_focus_command_bar(dred_context* pDred);

// Places keyboard focus on the command bar and sets it's text.
void dred_focus_command_bar_and_set_text(dred_context* pDred, const char* text);

// Releases keyboard focus from the command bar.
void dred_unfocus_command_bar(dred_context* pDred);


// Updates the info bar based on the given control.
void dred_update_info_bar(dred_context* pDred, dred_control* pControl);


// Shows the menu bar.
void dred_show_menu_bar(dred_context* pDred);

// Hides the menu bar.
void dred_hide_menu_bar(dred_context* pDred);

// Toggles the menu bar.
void dred_toggle_menu_bar(dred_context* pDred);


// Shows the tab bars on tab groups.
void dred_show_tabbars(dred_context* pDred);

// Hides the tab bars on tab groups.
void dred_hide_tabbars(dred_context* pDred);

// Toggles the tab bars on tab groups.
void dred_toggle_tabbars(dred_context* pDred);


// Shows line numbers on text editors.
void dred_show_line_numbers(dred_context* pDred);

// Hides line numbers on text editors.
void dred_hide_line_numbers(dred_context* pDred);

// Toggles line numbers on text editors.
void dred_toggle_line_numbers(dred_context* pDred);


// Enables word wrap on text editors.
void dred_enable_word_wrap(dred_context* pDred);

// Disables word wrap on text editor.
void dred_disable_word_wrap(dred_context* pDred);

// Toggles word wrap on text editors.
void dred_toggle_word_wrap(dred_context* pDred);


// Shows the command bar.
void dred_show_command_bar(dred_context* pDred);

// Hides the command bar.
void dred_hide_command_bar(dred_context* pDred);

// Enable's auto-hiding of the command bar.
void dred_enable_auto_hide_command_bar(dred_context* pDred);

// Disable's auto-hiding of the command bar.
void dred_disable_auto_hide_command_bar(dred_context* pDred);

// Toggle's auto-hiding of the command bar.
void dred_toggle_auto_hide_command_bar(dred_context* pDred);


// Sets the scale of text editors.
void dred_set_text_editor_scale(dred_context* pDred, float scale);

// Retrieves the scale of text editors.
float dred_get_text_editor_scale(dred_context* pDred);


// Sets the value of a config variable.
void dred_set_config_variable(dred_context* pDred, const char* name, const char* value);


// Parses a font string and loads the applicable font.
dred_font* dred_parse_and_load_font(dred_context* pDred, const char* value);

// Retrieves the language string that's associated with the extension of the given file path by default.
const char* dred_get_language_by_file_path(dred_context* pDred, const char* filePath);


// Adds a favourite file.
dr_bool32 dred_add_favourite(dred_context* pDred, const char* absolutePath);

// Removes a favourite file.
dr_bool32 dred_remove_favourite(dred_context* pDred, const char* absolutePath);


// Called when a tab is activated. This is where contextual GUI elements are shown or hidden.
void dred_on_tab_activated(dred_context* pDred, dred_tab* pTab, dred_tab* pOldActiveTab);

// Called when a tab is deactivated. This is where contextual GUI elements are shown or hidden.
void dred_on_tab_deactivated(dred_context* pDred, dred_tab* pTab, dred_tab* pNewActiveTab);

// Called when an accelerator is triggered.
void dred_on_accelerator(dred_context* pDred, dtk_accelerator accelerator);

// Called from the main loop in the platform layer when an IPC message is received.
void dred_on_ipc_message(dred_context* pDred, unsigned int messageID, const void* pMessageData);


// Retrieves a pointer to the dred_context from the given DTK control.
//
// This function assumes the control was created from the dtk_context object that is owned by the dred_context object.
dred_context* dred_get_context_from_control(dtk_control* pControl);


// Sets the main menu.
void dred_set_main_menu(dred_context* pDred, dtk_menu* pMenu);

// Shows the main menu.
void dred_show_main_menu(dred_context* pDred);

// Hides the main menu.
void dred_hide_main_menu(dred_context* pDred);

// Determines whether or not the main menu is being shown.
dtk_bool32 dred_is_showing_main_menu(dred_context* pDred);

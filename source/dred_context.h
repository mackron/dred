
struct dred_context
{
    // The command line that was passed to dred_init.
    dr_cmdline cmdline;

    // The log file.
    dred_file logFile;

    // The context for the 2D graphics sub-system which will be used for drawing the GUI.
    dr2d_context* pDrawingContext;

    // The main GUI context.
    drgui_context* pGUI;


    // The font library. This just manages fonts to make it easier to avoid loading duplicate fonts.
    dred_font_library fontLibrary;

    // The default GUI font.
    dred_font* pGUIFont;


    // The accelerator table.
    dred_accelerator_table acceleratorTable;


    // The main config.
    dred_config config;


    // The main window.
    dred_window* pMainWindow;

    // The main tab group container. This is the root container where all other sub-containers will be placed.
    dred_tabgroup_container* pMainTabgroupContainer;

    // The main tab group. This is actually temporary until support for multiple tab groups (splitting) is implemented.
    dred_tabgroup* pMainTabGroup;

    // The command bar. This is is the control that runs along the bottom of the main window.
    dred_cmdbar* pCmdBar;


    // Whether or not the application is running in silent mode.
    bool isTerminalOutputDisabled;



    // TEMP
    dred_text_editor* pEditor0;
    dred_tab* pEditor0Tab;
    dred_text_editor* pEditor1;
    dred_tab* pEditor1Tab;
};

// dred_init
bool dred_init(dred_context* pDred, dr_cmdline cmdline);

// dred_uninit
void dred_uninit(dred_context* pDred);

// dred_run
int dred_run(dred_context* pDred);


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


// Executes a command.
void dred_exec(dred_context* pDred, const char* cmd);


// Gives the given control the keyboard capture.
void dred_capture_keyboard(dred_context* pDred, dred_control* pControl);

// Releases the keyboard capture.
void dred_release_keyboard(dred_context* pDred);


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


// Retrieves the control type of the editor to use for a file with the extension of the given file path.
const char* dred_get_editor_type_by_path(const char* filePath);

// Finds the tab of the editor tied to the given absolute path.
dred_tab* dred_find_editor_tab_by_absolute_path(dred_context* pDred, const char* filePathAbsolute);


// Opens the file at the given path.
bool dred_open_file(dred_context* pDred, const char* filePath);

// Opens the file at the given path as the given type.
bool dred_open_file_by_type(dred_context* pDred, const char* filePath, const char* editorType);

// Closes the focused file.
void dred_close_focused_file(dred_context* pDred);

// Closes the given editor.
void dred_close_tab(dred_context* pDred, dred_tab* pTab);

// Closes every tab.
void dred_close_all_tabs(dred_context* pDred);


// Finds the tab associated with the given control, usually an editor.
dred_tab* dred_find_control_tab(dred_control* pControl);


// Saves the currently focused file.
//
// The new file name can be null in which case the file is just saved over the top of the current file.
bool dred_save_focused_file(dred_context* pDred, const char* newFilePath);

// Opens save dialog box and gives the user the opportunity to save the focused file as a different name.
bool dred_save_focused_file_as(dred_context* pDred);

// Saves every open file, skipping those that would otherwise require a save-as dialog.
void dred_save_all_open_files(dred_context* pDred);


// Creates and opens a new file.
//
// The newFilePath argument can be null, in which case it simply creates an empty text file.
bool dred_create_and_open_file(dred_context* pDred, const char* newFilePath);

// Creates an empty text file.
bool dred_create_new_text_file(dred_context* pDred);


// Creates an editor by it's type.
dred_editor* dred_create_editor_by_type(dred_context* pDred, dred_tabgroup* pTabGroup, const char* editorType, const char* filePathAbsolute);

// Deletes the given editor based on it's type.
void dred_delete_editor_by_type(dred_editor* pEditor);


// Show the Open File dialog.
void dred_show_open_file_dialog(dred_context* pDred);

// Shows the Save File dialog.
bool dred_show_save_file_dialog(dred_context* pDred, char* absolutePathOut, size_t absolutePathOutSize);

// Shows a yes/no/cancel dialog and returns the result as DRED_MESSAGE_BOX_YES, DRED_MESSAGE_BOX_NO or DRED_MESSAGE_BOX_CANCEL.
unsigned int dred_show_yesnocancel_dialog(dred_context* pDred, const char* message, const char* title);


// Sets the text of the command bar.
void dred_set_command_bar_text(dred_context* pDred, const char* text);

// Places keyboard focus on the command bar.
void dred_focus_command_bar(dred_context* pDred);

// Places keyboard focus on the command bar and sets it's text.
void dred_focus_command_bar_and_set_text(dred_context* pDred, const char* text);

// Releases keyboard focus from the command bar.
void dred_unfocus_command_bar(dred_context* pDred);


// Called when an accelerator is triggered.
void dred_on_accelerator(dred_context* pDred, dred_window* pWindow, size_t acceleratorIndex);
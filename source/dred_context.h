
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


// Called when an accelerator is triggered.
void dred_on_accelerator(dred_context* pDred, dred_window* pWindow, size_t acceleratorIndex);
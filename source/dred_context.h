
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


    // The main config.
    dred_config config;


    // The main window.
    dred_window* pMainWindow;


    // Whether or not the application is running in silent mode.
    bool isTerminalOutputDisabled;
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
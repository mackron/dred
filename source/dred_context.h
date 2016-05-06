
struct dred_context
{
    // The command line that was passed to dred_init.
    dr_cmdline cmdline;

    // The context for the 2D graphics sub-system which will be used for drawing the GUI.
    dr2d_context* pDrawingContext;

    // The main GUI context.
    drgui_context* pGUI;


    // The main window.
    dred_window* pMainWindow;
};

// dred_init
bool dred_init(dred_context* pDred, dr_cmdline cmdline);

// dred_uninit
void dred_uninit(dred_context* pDred);

// dred_run
int dred_run(dred_context* pDred);
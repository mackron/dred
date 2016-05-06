
struct dred_context
{
    // The command line that was passed to dred_init.
    dr_cmdline cmdline;

    // The main window.
    dred_window* pMainWindow;
};

// dred_init
bool dred_init(dred_context* pDred, dr_cmdline cmdline);

// dred_uninit
void dred_uninit(dred_context* pDred);

// dred_run
int dred_run(dred_context* pDred);
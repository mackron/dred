
static void dred_window_cb__on_main_window_close(dred_window* pWindow)
{
    dred_platform_post_quit_message(0);
}

bool dred_init(dred_context* pDred, dr_cmdline cmdline)
{
    if (pDred == NULL) {
        return false;
    }

    pDred->cmdline = cmdline;

    // The main window.
    pDred->pMainWindow = dred_window_create();
    if (pDred->pMainWindow == NULL) {
        printf("Failed to create main window.");
        return false;
    }

    pDred->pMainWindow->onClose = dred_window_cb__on_main_window_close;

    // Show the window as soon as possible to give it the illusion of loading quickly.
    dred_window_show(pDred->pMainWindow);

    return true;
}

void dred_uninit(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_window_delete(pDred->pMainWindow);
}

int dred_run(dred_context* pDred)
{
    if (pDred == NULL) {
        return -1;
    }

    return dred_platform_run(pDred);
}
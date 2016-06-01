
static void dred_window_cb__on_main_window_close(dred_window* pWindow)
{
    dred_platform_post_quit_message(0);
}

static void dred_window_cb__on_main_window_paint_TEMP(drgui_element* pElement, drgui_rect rect, void* pPaintData)
{
    drgui_draw_rect_with_outline(pElement, drgui_get_local_rect(pElement), drgui_rgb(255, 255, 255), 4, drgui_rgb(0, 0, 0), pPaintData);
}


static dred_file dred__open_log_file()
{
    char logFilePath[DRED_MAX_PATH];
    if (!dred_get_log_path(logFilePath, sizeof(logFilePath))) {
        return NULL;
    }

    return dred_file_open(logFilePath, DRED_FILE_OPEN_MODE_WRITE);
}

static void dred_config__on_error(dred_config* pConfig, const char* configPath, const char* message, unsigned int line, void* pUserData)
{
    dred_context* pDred = (dred_context*)pUserData;
    assert(pDred != NULL);

    dred_warningf(pDred, "%s[%d] : %s", configPath, line, message);
}


bool dred_init(dred_context* pDred, dr_cmdline cmdline)
{
    if (pDred == NULL) {
        return false;
    }

    memset(pDred, 0, sizeof(*pDred));


    pDred->cmdline = cmdline;

    // Open the log file first to ensure we're able to log as soon as possible.
    pDred->logFile = dred__open_log_file();


    // The drawing context.
#ifdef DRED_WIN32
    pDred->pDrawingContext = dr2d_create_context_gdi();
#endif
#ifdef DRED_GTK
    pDred->pDrawingContext = dr2d_create_context_cairo();
#endif
    if (pDred->pDrawingContext == NULL) {
        return false;
    }


    // The GUI.
    pDred->pGUI = drgui_create_context_dr_2d(pDred->pDrawingContext);
    if (pDred->pGUI == NULL) {
        dr2d_delete_context(pDred->pDrawingContext);
        return false;
    }

    // The GUI needs to be linked to the window system.
    dred_platform_bind_gui(pDred->pGUI);



    // Accelerator table needs to be initialized before the config, because the config can specify bindings.
    if (!dred_accelerator_table_init(&pDred->acceleratorTable)) {
        return false;
    }

    dred_accelerator_table_bind(&pDred->acceleratorTable, 'A', DRED_KEY_STATE_CTRL_DOWN, "select-all");
    dred_accelerator_table_bind(&pDred->acceleratorTable, 'S', DRED_KEY_STATE_CTRL_DOWN, "save");



    // Config
    //
    // The config is loaded in 3 stages. The first initializes it to it's default values, the second loads the .dred file from the main
    // user directory and the 3rd loads the .dred file sitting in the working directory.
    dred_config_init_default(&pDred->config);

    char configPath[DRED_MAX_PATH];
    if (dred_get_config_path(configPath, sizeof(configPath))) {
        dred_config_load_file(&pDred->config, configPath, dred_config__on_error, pDred);
    } else {
        dred_warning(pDred, "Failed to load .dred config file from user directory. The most likely cause of this is that the path is too long.");
    }

    dred_config_load_file(&pDred->config, ".dred", dred_config__on_error, pDred);

    

    // The main window.
    pDred->pMainWindow = dred_window_create(pDred);
    if (pDred->pMainWindow == NULL) {
        printf("Failed to create main window.");
        return false;
    }

    dred_window_set_size(pDred->pMainWindow, 1280, 720);

    pDred->pMainWindow->onClose = dred_window_cb__on_main_window_close;
    drgui_set_on_paint(pDred->pMainWindow->pRootGUIElement, dred_window_cb__on_main_window_paint_TEMP);

    // Show the window as soon as possible to give it the illusion of loading quickly.
    dred_window_show(pDred->pMainWindow);


    dred_window_bind_accelerators(pDred->pMainWindow, &pDred->acceleratorTable);


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

    return dred_platform_run();
}



void dred_log(dred_context* pDred, const char* message)
{
    if (pDred == NULL || message == NULL) {
        return;
    }

    // Log file.
    if (pDred->logFile != NULL) {
        char dateTime[64];
        dr_datetime_short(dr_now(), dateTime, sizeof(dateTime));

        dred_file_write_string(pDred->logFile, "[");
        dred_file_write_string(pDred->logFile, dateTime);
        dred_file_write_string(pDred->logFile, "]");
        dred_file_write_line  (pDred->logFile, message);
        dred_file_flush(pDred->logFile);
    }


    // Terminal.
    if (!pDred->isTerminalOutputDisabled) {
        printf("%s\n", message);
    }
}

void dred_logf(dred_context* pDred, const char* format, ...)
{
    if (pDred == NULL || format == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);

        dred_log(pDred, msg);
    }
    va_end(args);
}

void dred_warning(dred_context* pDred, const char* message)
{
    dred_logf(pDred, "[WARNING] %s", message);
}

void dred_warningf(dred_context* pDred, const char* format, ...)
{
    if (pDred == NULL || format == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);

        dred_warning(pDred, msg);
    }
    va_end(args);
}

void dred_error(dred_context* pDred, const char* message)
{
    dred_logf(pDred, "[ERROR] %s", message);
}

void dred_errorf(dred_context* pDred, const char* format, ...)
{
    if (pDred == NULL || format == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);

        dred_error(pDred, msg);
    }
    va_end(args);
}


void dred_exec(dred_context* pDred, const char* cmd)
{
    if (pDred == NULL || cmd == NULL) {
        return;
    }

    const char* value;
    dred_command command;
    if (dred_find_command(cmd, &command, &value)) {
        command.proc(pDred, value);
    }
}


void dred_on_accelerator(dred_context* pDred, dred_window* pWindow, size_t acceleratorIndex)
{
    printf("Accelerator: %d\n", (int)acceleratorIndex);
}
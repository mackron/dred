// Copyright (C) 2016 David Reid. See included LICENSE file.

float dred__get_cmd_bar_height(dred_context* pDred)
{
    if (pDred == NULL || !drgui_is_visible(pDred->pCmdBar)) {
        return 0;
    }

    return dred_control_get_height(pDred->pCmdBar);
}

void dred__update_main_tab_group_container_layout(dred_context* pDred, dred_tabgroup_container* pContainer, float parentWidth, float parentHeight)
{
    if (pContainer == NULL) {
        return;
    }

    dred_control_set_size(pContainer, parentWidth, parentHeight - dred__get_cmd_bar_height(pDred));
}

void dred__update_cmdbar_layout(dred_context* pDred, dred_cmdbar* pCmdBar, float parentWidth, float parentHeight)
{
    (void)pDred;

    if (pCmdBar == NULL) {
        return;
    }

    dred_control_set_size(pCmdBar, parentWidth, dred_control_get_height(pCmdBar));
    dred_control_set_relative_position(pCmdBar, 0, parentHeight - dred__get_cmd_bar_height(pDred));
}

void dred__update_main_window_layout(dred_window* pWindow, float windowWidth, float windowHeight)
{
    dred_context* pDred = pWindow->pDred;
    assert(pDred != NULL);

    dred__update_main_tab_group_container_layout(pDred, pDred->pMainTabgroupContainer, windowWidth, windowHeight);
    dred__update_cmdbar_layout(pDred, pDred->pCmdBar, windowWidth, windowHeight);
}


bool dred_parse_cmdline__startup_files(const char* key, const char* value, void* pUserData)
{
    dred_context* pDred = (dred_context*)pUserData;
    assert(pDred != NULL);

    if (key == NULL) {
        dred_open_file(pDred, value);
        return true;
    }

    return true;
}


void dred__update_window_title(dred_context* pDred)
{
    assert(pDred != NULL);

    const char* title = NULL;

    // The window title depends on the currently focused tab.
    dred_tab* pFocusedTab = dred_get_focused_tab(pDred);
    if (pFocusedTab != NULL) {
        dred_control* pFocusedControl = dred_tab_get_control(pFocusedTab);
        if (pFocusedControl != NULL) {
            if (drgui_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_EDITOR)) {
                const char* filePath = dred_editor_get_file_path(pFocusedControl);
                if (filePath != NULL && filePath[0] != '\0') {
                    title = drpath_file_name(filePath);
                } else {
                    title = "Untitled";
                }
            }
        }
    }

    char formattedTitle[256];
    if (title == NULL || snprintf(formattedTitle, sizeof(formattedTitle), "%s - dred", title) < 0) {
        strcpy_s(formattedTitle, sizeof(formattedTitle), "dred");
    }

    dred_window_set_title(pDred->pMainWindow, formattedTitle);
}

void dred__refresh_editor_tab_text(dred_editor* pEditor, dred_tab* pTab)
{
    assert(pEditor != NULL);
    assert(pTab != NULL);

    char tabText[256];
    const char* filename = drpath_file_name(dred_editor_get_file_path(pEditor));
    const char* modified = "";
    const char* readonly = "";

    if (filename == NULL || filename[0] == '\0') {
        filename = "[New File]";
    }
    if (dred_editor_is_modified(pEditor)) {
        modified = "*";
    }
    if (dred_editor_is_read_only(pEditor)) {
        readonly = " [Read Only]";
    }

    snprintf(tabText, sizeof(tabText), "%s%s%s", filename, modified, readonly);
    dred_tab_set_text(pTab, tabText);

    dred_context* pDred = dred_control_get_context(pEditor);
    assert(pDred != NULL);

    if (dred_get_focused_tab(pDred) == pTab) {
        dred__update_window_title(pDred);
    }
}

void dred__on_editor_modified(dred_editor* pEditor)
{
    dred_tab* pTab = dred_find_control_tab(pEditor);
    if (pTab == NULL) {
        return;
    }

    // We need to show a "*" at the end of the tab's text to indicate that it's modified.
    dred__refresh_editor_tab_text(pEditor, pTab);
}

void dred__on_editor_unmodified(dred_editor* pEditor)
{
    dred_tab* pTab = dred_find_control_tab(pEditor);
    if (pTab == NULL) {
        return;
    }

    // We need to hide the "*" at the end of the tab's text to indicate that it's unmodified.
    dred__refresh_editor_tab_text(pEditor, pTab);
}


void dred_window_cb__on_main_window_close(dred_window* pWindow)
{
    assert(pWindow != NULL);
    dred_close(pWindow->pDred);
}

void dred_window_cb__on_main_window_move(dred_window* pWindow, int posX, int posY)
{
    assert(pWindow != NULL);

    pWindow->pDred->config.windowPosX = posX;
    pWindow->pDred->config.windowPosY = posY;
}

void dred_window_cb__on_main_window_size(dred_element* pElement, float width, float height)
{
    (void)height;

    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow == NULL) {
        return;
    }

    dred__update_main_window_layout(pWindow, width, height);


    // The config needs to be updated so that it's settings are serialized.
    unsigned int windowWidth;
    unsigned int windowHeight;
    dred_window_get_size(pWindow, &windowWidth, &windowHeight);

    pWindow->pDred->config.windowWidth = windowWidth;
    pWindow->pDred->config.windowHeight = windowHeight;
    pWindow->pDred->config.windowMaximized = dred_window_is_maximized(pWindow);
}


dred_file dred__open_log_file()
{
    char logFilePath[DRED_MAX_PATH];
    if (!dred_get_log_path(logFilePath, sizeof(logFilePath))) {
        return NULL;
    }

    return dred_file_open(logFilePath, DRED_FILE_OPEN_MODE_WRITE);
}

void dred_config__on_error(dred_config* pConfig, const char* configPath, const char* message, unsigned int line, void* pUserData)
{
    (void)pConfig;

    dred_context* pDred = (dred_context*)pUserData;
    assert(pDred != NULL);

    dred_warningf(pDred, "%s[%d] : %s", configPath, line, message);
}


void dred_create_config_file_if_not_exists(const char* fileName, const char* configString)
{
    char configFolderPath[DRED_MAX_PATH];
    if (!dred_get_config_folder_path(configFolderPath, sizeof(configFolderPath))) {
        return;
    }

    char configPathAbsolute[DRED_MAX_PATH];
    if (!drpath_copy_and_append(configPathAbsolute, sizeof(configPathAbsolute), configFolderPath, fileName)) {
        return;
    }

    if (dr_file_exists(configPathAbsolute)) {
        return;
    }

    dr_open_and_write_text_file(configPathAbsolute, configString);
}


bool dred_init(dred_context* pDred, dr_cmdline cmdline)
{
    // TODO: USE dred_error() AND FAMILY FOR PRINTING CRITICAL ERRORS INSTEAD OF printf()

    if (pDred == NULL) {
        return false;
    }

    memset(pDred, 0, sizeof(*pDred));


    pDred->cmdline = cmdline;


    // Make sure the user's config directory exists.
    char configFolderPath[DRED_MAX_PATH];
    dred_get_config_folder_path(configFolderPath, sizeof(configFolderPath));
    dr_mkdir_recursive(configFolderPath);


    // Open the log file first to ensure we're able to log as soon as possible.
    pDred->logFile = dred__open_log_file();


    // Grab the system DPI scaling early so it can be used to correctly size GUI elements at initialization time.
    int baseDPI;
    int systemDPI;
    dred_get_base_dpi(&baseDPI, NULL);
    dred_get_system_dpi(&systemDPI, NULL);

    pDred->dpiScale = (float)systemDPI / (float)baseDPI;
    pDred->uiScale = pDred->dpiScale;


    // The drawing context.
#ifdef DRED_WIN32
    pDred->pDrawingContext = dr2d_create_context_gdi(NULL);
#endif
#ifdef DRED_GTK
    pDred->pDrawingContext = dr2d_create_context_cairo();
#endif
    if (pDred->pDrawingContext == NULL) {
        goto on_error;
    }


    // The GUI.
    pDred->pGUI = drgui_create_context_dr_2d(pDred, pDred->pDrawingContext);
    if (pDred->pGUI == NULL) {
        goto on_error;
    }

    // The GUI needs to be linked to the window system.
    dred_platform_bind_gui(pDred->pGUI);


    // The font library. This needs to be initialized before loading any fonts and configs.
    if (!dred_font_library_init(&pDred->fontLibrary, pDred)) {
        goto on_error;
    }

    // The image library. This needs to be initialized before loading any images and configs.
    if (!dred_image_library_init(&pDred->imageLibrary, pDred)) {
        goto on_error;
    }


    // Shortcut table.
    if (!dred_shortcut_table_init(&pDred->shortcutTable)) {
        goto on_error;
    }

    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_NEW,        dred_shortcut_create_single(dred_accelerator_create('N', DRED_KEY_STATE_CTRL_DOWN)), "new");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_OPEN,       dred_shortcut_create_single(dred_accelerator_create('O', DRED_KEY_STATE_CTRL_DOWN)), "open");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_SAVE,       dred_shortcut_create_single(dred_accelerator_create('S', DRED_KEY_STATE_CTRL_DOWN)), "save");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_SAVE_ALL,   dred_shortcut_create_single(dred_accelerator_create('S', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), "save-all");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_CLOSE,      dred_shortcut_create_single(dred_accelerator_create('W', DRED_KEY_STATE_CTRL_DOWN)), "close");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_CLOSE_ALL,  dred_shortcut_create_single(dred_accelerator_create('W', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), "close-all");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_PRINT,      dred_shortcut_create_single(dred_accelerator_create('P', DRED_KEY_STATE_CTRL_DOWN)), "print");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_UNDO,       dred_shortcut_create_single(dred_accelerator_create('Z', DRED_KEY_STATE_CTRL_DOWN)), "undo");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_REDO,       dred_shortcut_create_single(dred_accelerator_create('Y', DRED_KEY_STATE_CTRL_DOWN)), "redo");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_CUT,        dred_shortcut_create_single(dred_accelerator_create('X', DRED_KEY_STATE_CTRL_DOWN)), "cut");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_COPY,       dred_shortcut_create_single(dred_accelerator_create('C', DRED_KEY_STATE_CTRL_DOWN)), "copy");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_PASTE,      dred_shortcut_create_single(dred_accelerator_create('V', DRED_KEY_STATE_CTRL_DOWN)), "paste");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_SELECT_ALL, dred_shortcut_create_single(dred_accelerator_create('A', DRED_KEY_STATE_CTRL_DOWN)), "select-all");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_UNINDENT,   dred_shortcut_create_single(dred_accelerator_create('\t', DRED_KEY_STATE_SHIFT_DOWN)), "unindent");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_GOTO,       dred_shortcut_create_single(dred_accelerator_create('G', DRED_KEY_STATE_CTRL_DOWN)), "cmdbar goto ");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_FIND,       dred_shortcut_create_single(dred_accelerator_create('F', DRED_KEY_STATE_CTRL_DOWN)), "cmdbar find ");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_REPLACE,    dred_shortcut_create_single(dred_accelerator_create('F', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), "cmdbar replace-all ");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_NEXT_TAB,   dred_shortcut_create_single(dred_accelerator_create('\t', DRED_KEY_STATE_CTRL_DOWN)), "next-tab");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_PREV_TAB,   dred_shortcut_create_single(dred_accelerator_create('\t', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), "prev-tab");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_RELOAD,     dred_shortcut_create_single(dred_accelerator_create(DRED_GUI_F5, 0)), "reload");


    // Before loading configs we want to make sure any stock themes and settings are present.
    dred_create_config_file_if_not_exists(".dred", "");
    dred_create_config_file_if_not_exists("dark.dredtheme", g_StockTheme_Dark);
    dred_create_config_file_if_not_exists("light.dredtheme", g_StockTheme_Light);


    // Config
    //
    // The config is loaded in 4 stages. The first initializes it to it's default values, the second loads the .dredprivate file from the main
    // user directory, the third reads the .dred file from the main user directory, and the4th loads the .dred file sitting in the working directory.
    dred_config_init(&pDred->config, pDred);

    char configPath[DRED_MAX_PATH];
    if (dred_get_config_path(configPath, sizeof(configPath))) {
        char configPathPrivate[DRED_MAX_PATH];
        strcpy_s(configPathPrivate, sizeof(configPathPrivate), configPath);
        strcat_s(configPathPrivate, sizeof(configPathPrivate), "private");
        dred_load_config(pDred, configPathPrivate);
        dred_load_config(pDred, configPath);
    } else {
        dred_warning(pDred, "Failed to load .dred config file from user directory. The most likely cause of this is that the path is too long.");
    }

    dred_load_config(pDred, ".dred");

    // The UI scale will be known only after loading the configs.
    pDred->uiScale = pDred->dpiScale * pDred->config.uiScale;



    // The menu library. This must be initialized after the shortcut table and configs because it will need access to the initial shortcut bindings
    // and recent files.
    if (!dred_menu_library_init(&pDred->menuLibrary, pDred)) {
        goto on_error;
    }



    // The main window.
    int windowPosX = pDred->config.windowPosX;
    int windowPosY = pDred->config.windowPosY;
    unsigned int windowWidth =  (unsigned int)(pDred->config.windowWidth*pDred->dpiScale);
    unsigned int windowHeight = (unsigned int)(pDred->config.windowHeight*pDred->dpiScale);
    bool showWindowMaximized = pDred->config.windowMaximized;

    pDred->pMainWindow = dred_window_create(pDred);
    if (pDred->pMainWindow == NULL) {
        printf("Failed to create main window.");
        goto on_error;
    }

    pDred->pMainWindow->onClose = dred_window_cb__on_main_window_close;
    pDred->pMainWindow->onMove = dred_window_cb__on_main_window_move;
    drgui_set_on_size(pDred->pMainWindow->pRootGUIElement, dred_window_cb__on_main_window_size);

    // Ensure the accelerators are bound. This needs to be done after loading the initial configs.
    dred_window_bind_accelerators(pDred->pMainWindow, &pDred->shortcutTable.acceleratorTable);



    // The main tab group container.
    pDred->pMainTabgroupContainer = dred_tabgroup_container_create(pDred, pDred->pMainWindow->pRootGUIElement);
    if (pDred->pMainTabgroupContainer == NULL) {
        printf("Failed to create main tab group container.\n");
        goto on_error;
    }

    pDred->pMainTabGroup = dred_tabgroup_create(pDred, pDred->pMainTabgroupContainer);
    if (pDred->pMainTabGroup == NULL) {
        printf("Failed to create main tab group.\n");
        goto on_error;
    }


    // The command bar. Ensure this is given a valid initial size.
    pDred->pCmdBar = dred_cmdbar_create(pDred, pDred->pMainWindow->pRootGUIElement);
    if (pDred->pCmdBar == NULL) {
        printf("Failed to create command bar.\n");
        goto on_error;
    }

    if (pDred->config.autoHideCmdBar) {
        dred_control_hide(pDred->pCmdBar);
    }



    // Show the window last to ensure child GUI elements have been initialized and in a valid state. This should be done before
    // opening the files passed on the command line, however, because the window needs to be shown in order for it to receive
    // keyboard focus.
    dred_window_set_title(pDred->pMainWindow, "dred");
    dred_window_set_menu(pDred->pMainWindow, pDred->menuLibrary.pMenu_Default);
    dred_window_set_size(pDred->pMainWindow, windowWidth, windowHeight);
    if (showWindowMaximized) {
        dred_window_show_maximized(pDred->pMainWindow);
    } else {
        dred_window_show(pDred->pMainWindow);
    }
    if (!pDred->config.showMenuBar) {
        dred_window_hide_menu(pDred->pMainWindow);
    }
    if (!pDred->config.useDefaultWindowPos) {
        dred_window_set_position(pDred->pMainWindow, windowPosX, windowPosY);
    }

    // We only want to use the default window position on first run.
    pDred->config.useDefaultWindowPos = false;




    // Load initial files from the command line.
    dr_parse_cmdline(&pDred->cmdline, dred_parse_cmdline__startup_files, pDred);

    // If there were no files passed on the command line, start with an empty text file. We can know this by simply finding the
    // focused editor. If it's null, nothing is open.
    if (dred_get_focused_editor(pDred) == NULL) {
        dred_open_new_text_file(pDred);
    }


    // This is a little hack to ensure the maximized state is saved correctly.
    pDred->config.windowMaximized = showWindowMaximized;

    pDred->isInitialized = true;
    return true;

on_error:
    dred_uninit(pDred);
    return false;
}

void dred_uninit(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    // Make sure any lingering tabs are forcefully closed. This should be done at a higher level so that the user
    // can be prompted to save any unsaved work or whatnot, but I'm keeping this here for sanity.
    dred_close_all_tabs(pDred);


    if (pDred->pAboutDialog) {
        dred_about_dialog_delete(pDred->pAboutDialog);
    }


    if (pDred->pCmdBar) {
        dred_cmdbar_delete(pDred->pCmdBar);
    }

    if (pDred->pMainTabGroup) {
        dred_tabgroup_delete(pDred->pMainTabGroup);
    }

    if (pDred->pMainTabgroupContainer) {
        dred_tabgroup_container_delete(pDred->pMainTabgroupContainer);
    }

    if (pDred->pMainWindow) {
        dred_window_delete(pDred->pMainWindow);
    }

    dred_config_uninit(&pDred->config);
    dred_shortcut_table_uninit(&pDred->shortcutTable);
    dred_font_library_uninit(&pDred->fontLibrary);

    if (pDred->pGUI) {
        drgui_delete_context(pDred->pGUI);
    }

    if (pDred->pDrawingContext) {
        dr2d_delete_context(pDred->pDrawingContext);
    }

    if (pDred->logFile) {
        dred_file_close(pDred->logFile);
    }
}


int dred_run(dred_context* pDred)
{
    if (pDred == NULL) {
        return -1;
    }

    return dred_platform_run();
}

void dred_close(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    // This will return false if the user hits the cancel button.
    if (!dred_close_all_tabs_with_confirmation(pDred)) {
        return;
    }

    // Make sure the settings file has been saved.
    dred_save_dredprivate(pDred);

    // Terminate from the main loop.
    dred_platform_post_quit_message(0);
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


bool dred_load_config(dred_context* pDred, const char* configFilePath)
{
    if (pDred == NULL || configFilePath == NULL) {
        return false;
    }

    return dred_config_load_file(&pDred->config, configFilePath, dred_config__on_error, pDred);
}


bool dred_exec(dred_context* pDred, const char* cmd, dred_command* pLastCmdOut)
{
    if (pDred == NULL || cmd == NULL) {
        return false;
    }

    dred_command_separator cmdSeparator = dred_command_separator_none;

    char subcmd[4096];
    while ((cmd = dred_next_command_string(cmd, subcmd, sizeof(subcmd), &cmdSeparator)) != NULL) {
        const char* value;
        dred_command command;
        if (dred_find_command(subcmd, &command, &value)) {
            if (pLastCmdOut) *pLastCmdOut = command;

            bool result = command.proc(pDred, value);
            if (result) {
                if (cmdSeparator == dred_command_separator_or) {
                    return true;
                }
            } else {
                if (cmdSeparator == dred_command_separator_and || cmdSeparator == dred_command_separator_none) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool dred_bind_shortcut(dred_context* pDred, const char* shortcutName, dred_shortcut shortcut, const char* commandStr)
{
    if (pDred == NULL) {
        return false;
    }

    if (!dred_shortcut_table_bind(&pDred->shortcutTable, shortcutName, shortcut, commandStr)) {
        return false;
    }

    // If we have a window we'll need to re-bind the accelerators.
    if (pDred->pMainWindow != NULL) {
        dred_window_bind_accelerators(pDred->pMainWindow, &pDred->shortcutTable.acceleratorTable);
    }

    return true;
}


void dred_save_dredprivate(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }


    char dredprivatePath[DRED_MAX_PATH];
    dred_get_config_folder_path(dredprivatePath, sizeof(dredprivatePath));

    drpath_append(dredprivatePath, sizeof(dredprivatePath), ".dredprivate");

    dred_file file = dred_file_open(dredprivatePath, DRED_FILE_OPEN_MODE_WRITE);
    if (file == NULL) {
        return;
    }

    dred_file_write_string(file, "# This file is generated by dred - do not modify. This file also includes information you may want\n");
    dred_file_write_string(file, "# to keep private such as paths to your most recent files, so you should not distribute this file.\n\n");

    dred_config_write_to_file__autogenerated(&pDred->config, file);

    // Bindings.
    for (size_t i = 0; i < pDred->shortcutTable.count; ++i) {
        char shortcutStr[256];
        dred_shortcut_to_string(pDred->shortcutTable.pShortcuts[i], shortcutStr, sizeof(shortcutStr));

        char bindingStr[4096];
        if (snprintf(bindingStr, sizeof(bindingStr), "bind \"%s\" \"%s\" \"%s\"\n", pDred->shortcutTable.ppNameStrings[i], shortcutStr, pDred->shortcutTable.ppCmdStrings[i]) < 0) {
            continue;   // Error parsing.
        }

        dred_file_write_string(file, bindingStr);
    }

    // Recent files.
    for (size_t i = pDred->config.recentFileCount; i > 0; --i) {
        dred_file_write_string(file, "recent-file \"");
        dred_file_write_string(file, pDred->config.recentFiles[i-1]);
        dred_file_write_string(file, "\"\n");
    }

    // Recent commands.
    for (size_t i = pDred->config.recentCommandsCount; i > 0; --i) {
        dred_file_write_string(file, "recent-cmd \"");
        dred_file_write_string(file, pDred->config.recentCommands[i-1]);
        dred_file_write_string(file, "\"\n");
    }

    // Aliases
    for (size_t i = 0; i < pDred->config.aliasMap.count; ++i) {
        dred_file_write_string(file, "alias ");
        dred_file_write_string(file, pDred->config.aliasMap.keys[i]);

        dred_file_write_string(file, " \"");
        dred_file_write_string(file, pDred->config.aliasMap.values[i]);
        dred_file_write_string(file, "\"\n");
    }

    dred_file_close(file);
}


void dred_capture_keyboard(dred_context* pDred, dred_control* pControl)
{
    if (pDred == NULL) {
        return;
    }

    if (pControl == NULL) {
        dred_release_keyboard(pDred);
        return;
    }

    drgui_capture_keyboard(pControl);
}

void dred_release_keyboard(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    drgui_release_keyboard(pDred->pGUI);
}


dred_tabgroup* dred_first_tabgroup(dred_context* pDred)
{
    if (pDred == NULL) {
        return NULL;
    }

    return pDred->pMainTabGroup;
}

dred_tabgroup* dred_next_tabgroup(dred_context* pDred, dred_tabgroup* pTabGroup)
{
    if (pDred == NULL || pTabGroup == NULL) {
        return NULL;
    }

    return dred_tabgroup_next_tabgroup(pTabGroup);
}


dred_tabgroup* dred_get_focused_tabgroup(dred_context* pDred)
{
    if (pDred == NULL) {
        return NULL;
    }

    return pDred->pMainTabGroup;
}

dred_tab* dred_get_focused_tab(dred_context* pDred)
{
    dred_tabgroup* pFocusedTabGroup = dred_get_focused_tabgroup(pDred);
    if (pFocusedTabGroup == NULL) {
        return NULL;
    }

    return dred_tabgroup_get_active_tab(pFocusedTabGroup);
}

dred_editor* dred_get_focused_editor(dred_context* pDred)
{
    dred_tab* pFocusedTab = dred_get_focused_tab(pDred);
    if (pFocusedTab == NULL) {
        return NULL;
    }

    dred_control* pControl = dred_tab_get_control(pFocusedTab);
    if (pControl == NULL) {
        return NULL;
    }

    if (!drgui_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
        return NULL;
    }

    return pControl;
}

dred_element* dred_get_element_with_keyboard_capture(dred_context* pDred)
{
    if (pDred == NULL) {
        return NULL;
    }

    dred_element* pElement = drgui_get_element_with_keyboard_capture(pDred->pGUI);
    if (pElement == NULL) {
        pElement = pDred->pMainWindow->pElementWithKeyboardCapture;
    }

    return pElement;
}


bool dred__save_editor(dred_editor* pEditor, const char* newFilePath, dred_tab* pTab)
{
    if (!dred_editor_save(pEditor, newFilePath)) {
        return false;
    }

    dred__refresh_editor_tab_text(pEditor, pTab);
    return true;
}

const char* dred_get_editor_type_by_path(const char* filePath)
{
    // TODO: Add to this list.
    // Check for known extensions first as a performance optimization. If that fails we'll want to open the file and inspect it.
    if (drpath_extension_equal(filePath, "txt")) {
        return DRED_CONTROL_TYPE_TEXT_EDITOR;
    }

    return NULL;
}

dred_tab* dred_find_editor_tab_by_absolute_path(dred_context* pDred, const char* filePathAbsolute)
{
    char filePathAbsoluteClean[DRED_MAX_PATH];
    drpath_clean(filePathAbsolute, filePathAbsoluteClean, sizeof(filePathAbsoluteClean));

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (pControl != NULL && drgui_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
                if (drpath_equal(dred_editor_get_file_path(pControl), filePathAbsoluteClean)) {
                    return pTab;
                }
            }
        }
    }

    return NULL;
}

bool dred_open_file(dred_context* pDred, const char* filePath)
{
    return dred_open_file_by_type(pDred, filePath, dred_get_editor_type_by_path(filePath));
}

bool dred_open_file_by_type(dred_context* pDred, const char* filePath, const char* editorType)
{
    char filePathAbsolute[DRED_MAX_PATH];
    if (filePath != NULL && filePath[0] != '\0') {
        if (!dred_to_absolute_path(filePath, filePathAbsolute, sizeof(filePathAbsolute))) {
            dred_errorf(pDred, "File path is too long %s\n", filePath);
            return false;
        }
    } else {
        filePathAbsolute[0] = '\0';
    }

    // If the file is already open, activate it's tab.
    if (filePath != NULL) {
        dred_tab* pExistingTab = dred_find_editor_tab_by_absolute_path(pDred, filePathAbsolute);
        if (pExistingTab != NULL) {
            dred_tabgroup_activate_tab(dred_tab_get_tabgroup(pExistingTab), pExistingTab);
            return true;
        }
    }


    // Before creating the editor we'll want to identify the tab group to attach it to.
    dred_tabgroup* pTabGroup = dred_get_focused_tabgroup(pDred);
    if (pTabGroup == NULL) {
        return false;   // TODO: This means there is no tab group so one need to be created.
    }

    dred_editor* pEditor = dred_create_editor_by_type(pDred, pTabGroup, editorType, filePathAbsolute);
    if (pEditor == NULL) {
        return false;
    }

    dred_editor_set_on_modified(pEditor, dred__on_editor_modified);
    dred_editor_set_on_unmodified(pEditor, dred__on_editor_unmodified);

    // We have the editor, so now we need to create a tab an associate the new editor with it.
    dred_tab* pTab = dred_tabgroup_prepend_tab(pTabGroup, NULL, pEditor);
    if (pTab == NULL) {
        dred_delete_editor_by_type(pEditor);
        return false;
    }
    dred__refresh_editor_tab_text(pEditor, pTab);
    dred_tabgroup_activate_tab(pTabGroup, pTab);


    // If there is only one other tab, and it's an new, unmodified file, close it.
    if (pTab->pNextTab != NULL && pTab->pNextTab->pNextTab == NULL) {
        dred_editor* pOtherEditor = dred_tab_get_control(pTab->pNextTab);
        if (drgui_is_of_type(pOtherEditor, DRED_CONTROL_TYPE_EDITOR) && !dred_editor_is_modified(pOtherEditor)) {
            const char* pOtherEditorFile = dred_editor_get_file_path(pOtherEditor);
            if (pOtherEditorFile == NULL || pOtherEditorFile[0] == '\0') {
                // It's a new unmodified file. Close it.
                dred_close_tab(pDred, pTab->pNextTab);
            }
        }
    }


    // The file should be added to the recent file list. This is done by adding it to the config and then refreshing the menu.
    if (filePathAbsolute[0] != '\0') {
        dred_config_push_recent_file(&pDred->config, filePathAbsolute);
        dred_menu_library_update_recent_files_menu(&pDred->menuLibrary);
    }

    return true;
}

void dred_close_focused_file(dred_context* pDred)
{
    dred_close_tab(pDred, dred_get_focused_tab(pDred));
}

void dred_close_focused_file_with_confirmation(dred_context* pDred)
{
    dred_close_tab_with_confirmation(pDred, dred_get_focused_tab(pDred));
}

void dred_close_tab(dred_context* pDred, dred_tab* pTab)
{
    if (pDred == NULL || pTab == NULL) {
        return;
    }

    dred_editor* pEditor = dred_tab_get_control(pTab);

    // Delete the tab.
    dred_tabgroup_delete_tab(dred_tab_get_tabgroup(pTab), pTab);

    // Delete the control.
    if (drgui_is_of_type(pEditor, DRED_CONTROL_TYPE_EDITOR)) {
        dred_delete_editor_by_type(pEditor);
    }

    // If after closing the tab there are no other active tabs, activate the command bar.
    if (dred_get_focused_tab(pDred) == NULL) {
        dred_focus_command_bar(pDred);
    }
}

void dred_close_tab_with_confirmation(dred_context* pDred, dred_tab* pTab)
{
    dred_editor* pEditor = dred_tab_get_control(pTab);
    if (pEditor == NULL) {
        dred_close_tab(pDred, pTab);
        return;
    }

    if (!drgui_is_of_type(pEditor, DRED_CONTROL_TYPE_EDITOR)) {
        dred_close_tab(pDred, pTab);
        return;
    }

    if (dred_editor_is_modified(pEditor)) {
        char msg[4096];

        // Activate the tab before showing the message box to given them some visual feedback as to which file it's referring to.
        dred_tabgroup_activate_tab(dred_tab_get_tabgroup(pTab), pTab);

        const char* filePath = dred_editor_get_file_path(pEditor);
        if (filePath == NULL || filePath[0] == '\0') {
            snprintf(msg, sizeof(msg), "Do you want to save this file before closing?");
        } else {
            snprintf(msg, sizeof(msg), "%s has been modified. Do you want to save it before closing?", filePath);
        }

        unsigned int result = dred_show_yesnocancel_dialog(pDred, msg, "Save changes?");
        if (result == DRED_MESSAGE_BOX_YES) {
            if (!dred__save_editor(pEditor, NULL, pTab)) {
                char newFilePath[DRED_MAX_PATH];
                if (!dred_show_save_file_dialog(pDred, filePath, newFilePath, sizeof(newFilePath))) {
                    return;
                }
                dred_editor_save(pEditor, newFilePath);
            }
            dred_close_tab(pDred, pTab);
        } else if (result == DRED_MESSAGE_BOX_NO) {
            dred_close_tab(pDred, pTab);
        }
    } else {
        dred_close_tab(pDred, pTab);
    }
}

void dred_close_all_tabs(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    // NOTE: This for loop is erroneous if closing the last tab in a group automatically deletes the group.
    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        while (dred_tabgroup_first_tab(pTabGroup) != NULL) {
            dred_close_tab(pDred, dred_tabgroup_first_tab(pTabGroup));
        }
    }
}

bool dred_close_all_tabs_with_confirmation(dred_context* pDred)
{
    if (pDred == NULL) {
        return false;
    }

    // If there's any modified files we need to show a dialog box.
    if (dred_are_any_open_files_modified(pDred)) {
        unsigned int result = dred_show_yesnocancel_dialog(pDred, "You have unsaved changes. Save changes?", "Save changes?");
        if (result == DRED_MESSAGE_BOX_YES) {
            if (!dred_save_all_open_files_with_saveas(pDred)) {
                return true;
            }
        } else if (result == DRED_MESSAGE_BOX_CANCEL) {
            return false;
        }
    }

    dred_close_all_tabs(pDred);
    return true;
}


dred_tab* dred_find_control_tab(dred_control* pControl)
{
    dred_context* pDred = dred_control_get_context(pControl);
    if (pDred == NULL) {
        return NULL;
    }

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            if (dred_tab_get_control(pTab) == pControl) {
                return pTab;
            }
        }
    }

    return NULL;
}


bool dred_save_focused_file(dred_context* pDred, const char* newFilePath)
{
    if (pDred == NULL) {
        return false;
    }

    dred_tab* pFocusedTab = dred_get_focused_tab(pDred);
    if (pFocusedTab == NULL) {
        return false;
    }

    dred_control* pFocusedControl = dred_tab_get_control(pFocusedTab);
    if (pFocusedControl == NULL) {
        return false;
    }

    // Editor.
    if (drgui_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_EDITOR)) {
        return dred__save_editor(pFocusedControl, newFilePath, pFocusedTab);
    }

    // Output.
    //
    // TODO: Add support for saving the output window.


    // Add the file to the recent file's list.
    char absoluteFilePath[DRED_MAX_PATH];
    dred_to_absolute_path(newFilePath, absoluteFilePath, sizeof(absoluteFilePath));
    if (absoluteFilePath[0] != '\0') {
        dred_config_push_recent_file(&pDred->config, absoluteFilePath);
        dred_menu_library_update_recent_files_menu(&pDred->menuLibrary);
    }

    return false;
}

bool dred_save_focused_file_as(dred_context* pDred)
{
    if (pDred == NULL) {
        return false;
    }

    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return false;
    }

    char newFilePath[DRED_MAX_PATH];
    if (!dred_show_save_file_dialog(pDred, dred_editor_get_file_path(pFocusedEditor), newFilePath, sizeof(newFilePath))) {
        return false;
    }

    return dred_save_focused_file(pDred, newFilePath);
}

void dred_save_all_open_files(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (drgui_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
                dred__save_editor(pControl, NULL, pTab);
            }
        }
    }
}

bool dred_save_all_open_files_with_saveas(dred_context* pDred)
{
    if (pDred == NULL) {
        return false;
    }

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (drgui_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
                if (!dred__save_editor(pControl, NULL, pTab)) {
                    char newFileName[DRED_MAX_PATH];
                    if (!dred_show_save_file_dialog(pDred, dred_editor_get_file_path(pControl), newFileName, sizeof(newFileName))) {
                        return false;
                    }

                    if (!dred_editor_save(pControl, newFileName)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}


bool dred_create_and_open_file(dred_context* pDred, const char* newFilePath)
{
    if (pDred == NULL) {
        return false;
    }

    if (newFilePath == NULL || newFilePath[0] == '\0') {
        return dred_open_new_text_file(pDred);
    } else {
        if (dr_file_exists(newFilePath)) {
            dred_errorf(pDred, "File already exists: %s", newFilePath);
            return false;
        }

        if (!dr_create_empty_file(newFilePath, true)) {
            dred_errorf(pDred, "Failed to create file: %s", newFilePath);
            return false;
        }

        return dred_open_file(pDred, newFilePath);
    }
}

bool dred_open_new_text_file(dred_context* pDred)
{
    return dred_open_file_by_type(pDred, NULL, DRED_CONTROL_TYPE_TEXT_EDITOR);
}


dred_editor* dred_create_editor_by_type(dred_context* pDred, dred_tabgroup* pTabGroup, const char* editorType, const char* filePathAbsolute)
{
    if (pDred == NULL) {
        return NULL;
    }

    // Note that the parent is always set to the tab group. This should be set at creation time to ensure it is
    // initialized properly. In particular, if we don't do this, the cursor will not be set to the correct value
    // because the sub-editor won't be attached to a window at creation time.

    float sizeX;
    float sizeY;
    dred_tabgroup_get_body_size(pTabGroup, &sizeX, &sizeY);

    dred_editor* pEditor = NULL;
    if (pEditor == NULL && dred_is_control_type_of_type(editorType, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        pEditor = dred_text_editor_create(pDred, pTabGroup, sizeX, sizeY, filePathAbsolute);
    }
    if (pEditor == NULL && dred_is_control_type_of_type(editorType, DRED_CONTROL_TYPE_SETTINGS_EDITOR)) {
        pEditor = dred_settings_editor_create(pDred, pTabGroup, filePathAbsolute);
    }

    // Fall back to a text editor if it's an unknown extension.
    if (pEditor == NULL) {
        pEditor = dred_text_editor_create(pDred, pTabGroup, sizeX, sizeY, filePathAbsolute);
    }

    return pEditor;
}

void dred_delete_editor_by_type(dred_editor* pEditor)
{
    if (drgui_is_of_type(pEditor, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        dred_text_editor_delete(pEditor);
        return;
    }
    if (drgui_is_of_type(pEditor, DRED_CONTROL_TYPE_SETTINGS_EDITOR)) {
        dred_settings_editor_delete(pEditor);
        return;
    }
}


bool dred_are_any_open_files_modified(dred_context* pDred)
{
    if (pDred == NULL) {
        return false;
    }

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (pControl != NULL) {
                if (drgui_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR) && dred_editor_is_modified(pControl)) {
                    return true;
                }
            }
        }
    }

    return false;
}


bool dred_reload_focused_file(dred_context* pDred)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return false;
    }

    return dred_editor_reload(pFocusedEditor);
}

bool dred_check_if_focused_file_is_dirty_and_reload(dred_context* pDred)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return false;
    }

    return dred_editor_check_if_dirty_and_reload(pFocusedEditor);
}


void dred_show_open_file_dialog(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

#ifdef _WIN32
    char filePaths[4096];
    filePaths[0] = '\0';

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = pDred->pMainWindow->hWnd;
    ofn.lpstrFile = filePaths;
    ofn.nMaxFile = sizeof(filePaths);
    ofn.lpstrFilter = "All\0*.*\0Text Files\0*.txt\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameA(&ofn)) {
        return;
    }

    // We need to determine whether or not mutliple files were selected. GetOpenFileName doesn't seem to provide a good way to
    // determine this so we need to figure this out ourselves. They way we do it is to check if the first entry is a directory
    // or a file. If it's a directory it means multiple files were selected.
    const char* directoryPath = filePaths;
    if (dr_directory_exists(directoryPath))
    {
        // Multiple files were selected.
        const char* nextFilePath = directoryPath + strlen(directoryPath) + 1;
        while (nextFilePath[0] != '\0') {
            char absolutePath[DRED_MAX_PATH];
            if (drpath_copy_and_append(absolutePath, sizeof(absolutePath), directoryPath, nextFilePath)) {
                dred_open_file(pDred, absolutePath);
            }

            nextFilePath += strlen(nextFilePath) + 1;
        }
    }
    else
    {
        // Only a single file was selected.
        dred_open_file(pDred, filePaths);
    }
#else
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Open", GTK_WINDOW(pDred->pMainWindow->pGTKWindow), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Open",   GTK_RESPONSE_ACCEPT,
        "_Cancel", GTK_RESPONSE_CANCEL, NULL);
    if (dialog == NULL) {
        return;
    }

    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        GSList* list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
        if (list != NULL) {
            while(list != NULL) {
                char* filename = (char*)list->data;
                if (filename != NULL && filename[0] != '\0') {
                    dred_open_file(pDred, filename);
                }

                g_free(filename);
                list = list->next;
            }

            g_slist_free(list);
        }
    }

    gtk_widget_destroy(dialog);
#endif
}

bool dred_show_save_file_dialog(dred_context* pDred, const char* currentFilePath, char* absolutePathOut, size_t absolutePathOutSize)
{
    if (pDred == NULL || absolutePathOut == NULL || absolutePathOutSize == 0) {
        return false;
    }

    absolutePathOut[0] = '\0';  // For safety, and also required for Win32.

#ifdef _WIN32
    if (currentFilePath != NULL) {
        strcpy_s(absolutePathOut, absolutePathOutSize, drpath_file_name(currentFilePath));
    }

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = pDred->pMainWindow->hWnd;
    ofn.lpstrFile = absolutePathOut;
    ofn.nMaxFile = (DWORD)absolutePathOutSize;
    ofn.lpstrFilter = "All\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetSaveFileNameA(&ofn)) {
        return false;
    }

    return true;
#else
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Save As", GTK_WINDOW(pDred->pMainWindow->pGTKWindow), GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Save",   GTK_RESPONSE_ACCEPT,
        "_Cancel", GTK_RESPONSE_CANCEL, NULL);
    if (dialog == NULL) {
        return false;
    }

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    if (currentFilePath != NULL && currentFilePath[0] != '\0') {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), currentFilePath);
    }


    bool result = false;
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        result = strcpy_s(absolutePathOut, absolutePathOutSize, filename) == 0;

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
    return result;
#endif
}

unsigned int dred_show_yesnocancel_dialog(dred_context* pDred, const char* message, const char* title)
{
    if (pDred == NULL) {
        return 0;
    }

    // TODO: Move this to the platform layer.
#ifdef _WIN32
    int result = MessageBoxA(pDred->pMainWindow->hWnd, message, title, MB_YESNOCANCEL);
    switch (result)
    {
        case IDCANCEL: return DRED_MESSAGE_BOX_CANCEL;
        case IDYES:    return DRED_MESSAGE_BOX_YES;
        case IDNO:     return DRED_MESSAGE_BOX_NO;

        default: break;
    }

    return 0;
#else
    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(pDred->pMainWindow->pGTKWindow), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s", message);
    if (dialog == NULL) {
        return 0;
    }

    gtk_window_set_title(GTK_WINDOW(dialog), title);

    GtkWidget* pYes = gtk_dialog_add_button(GTK_DIALOG(dialog), "Yes", DRED_MESSAGE_BOX_YES);
    if (pYes != NULL) {
    }

    GtkWidget* pNo = gtk_dialog_add_button(GTK_DIALOG(dialog), "No", DRED_MESSAGE_BOX_NO);
    if (pNo != NULL) {
        gtk_widget_set_margin_start(pNo, 8);
    }

    GtkWidget* pCancel = gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", DRED_MESSAGE_BOX_CANCEL);
    if (pCancel != NULL) {
        gtk_widget_set_margin_start(pCancel, 8);

    }

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return result;
#endif
}

bool dred_show_font_picker_dialog(dred_context* pDred, dred_window* pOwnerWindow, const dred_font_desc* pDefaultFontDesc, dred_font_desc* pDescOut)
{
    if (pDred == NULL || pDescOut == NULL) {
        return false;
    }

    if (pOwnerWindow == NULL) {
        pOwnerWindow = pDred->pMainWindow;
    }


#ifdef DRED_WIN32
    LOGFONTA lf;
    ZeroMemory(&lf, sizeof(lf));
    lf.lfHeight = -13;
    lf.lfWeight = FW_REGULAR;
    lf.lfItalic = FALSE;

    if (pDefaultFontDesc != NULL) {
        strncpy_s(lf.lfFaceName, sizeof(lf.lfFaceName), pDefaultFontDesc->family, _TRUNCATE);
        lf.lfHeight = -(LONG)pDefaultFontDesc->size;

        switch (pDefaultFontDesc->weight)
        {
        case dr2d_font_weight_medium:      lf.lfWeight = FW_MEDIUM;     break;
        case dr2d_font_weight_thin:        lf.lfWeight = FW_THIN;       break;
        case dr2d_font_weight_extra_light: lf.lfWeight = FW_EXTRALIGHT; break;
        case dr2d_font_weight_light:       lf.lfWeight = FW_LIGHT;      break;
        case dr2d_font_weight_semi_bold:   lf.lfWeight = FW_SEMIBOLD;   break;
        case dr2d_font_weight_bold:        lf.lfWeight = FW_BOLD;       break;
        case dr2d_font_weight_extra_bold:  lf.lfWeight = FW_EXTRABOLD;  break;
        case dr2d_font_weight_heavy:       lf.lfWeight = FW_HEAVY;      break;
        default: break;
        }

        if (pDefaultFontDesc->slant == dr2d_font_slant_italic || pDefaultFontDesc->slant == dr2d_font_slant_oblique) {
            lf.lfItalic = TRUE;
        }
    }

    CHOOSEFONTA cf;
    ZeroMemory(&cf, sizeof(cf));
    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = pOwnerWindow->hWnd;
    cf.hDC = GetDC(pOwnerWindow->hWnd);
    cf.lpLogFont = &lf;
    cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_BOTH;

    if (!ChooseFontA(&cf)) {
        return false;
    }

    strcpy_s(pDescOut->family, sizeof(pDescOut->family), lf.lfFaceName);

    if (lf.lfHeight < 0) {
        pDescOut->size = -lf.lfHeight;
    } else {
        pDescOut->size = lf.lfHeight;
    }

    pDescOut->weight = dred_gui_font_weight_default;
    switch (lf.lfWeight)
    {
    case FW_MEDIUM:     pDescOut->weight = dred_gui_font_weight_medium;      break;
    case FW_THIN:       pDescOut->weight = dred_gui_font_weight_thin;        break;
    case FW_EXTRALIGHT: pDescOut->weight = dred_gui_font_weight_extra_light; break;
    case FW_LIGHT:      pDescOut->weight = dred_gui_font_weight_light;       break;
    case FW_SEMIBOLD:   pDescOut->weight = dred_gui_font_weight_semi_bold;   break;
    case FW_BOLD:       pDescOut->weight = dred_gui_font_weight_bold;        break;
    case FW_EXTRABOLD:  pDescOut->weight = dred_gui_font_weight_extra_bold;  break;
    case FW_HEAVY:      pDescOut->weight = dred_gui_font_weight_heavy;       break;
    default: break;
    }

    pDescOut->slant = dred_gui_font_slant_none;
    if (lf.lfItalic) {
        pDescOut->slant = dred_gui_font_slant_italic;
    }

    pDescOut->flags = 0;
    pDescOut->rotation = 0;

    return true;
#endif

#ifdef DRED_GTK
    (void)pDefaultFontDesc;

    GtkWidget* dialog = gtk_font_chooser_dialog_new(NULL, GTK_WINDOW(pDred->pMainWindow->pGTKWindow));
    if (dialog == NULL) {
        return false;
    }

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    gchar* pangoFontStr = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dialog));
    if (pangoFontStr == NULL) {
        gtk_widget_destroy(dialog);
        return false;
    }

    PangoFontDescription* pPangoDesc = pango_font_description_from_string(pangoFontStr);
    if (pPangoDesc != NULL) {
        strcpy_s(pDescOut->family, sizeof(pDescOut->family), pango_font_description_get_family(pPangoDesc));

        gint size = pango_font_description_get_size(pPangoDesc);
        if (size > 0) {
            if (pango_font_description_get_size_is_absolute(pPangoDesc)) {
                pDescOut->size = size;
            } else {
                pDescOut->size = (unsigned int)(size/PANGO_SCALE * (96.0/72.0));
            }
        }

        pDescOut->slant = dred_font_slant_from_pango(pango_font_description_get_style(pPangoDesc));
        pDescOut->weight = dred_font_weight_from_pango(pango_font_description_get_weight(pPangoDesc));

        pango_font_description_free(pPangoDesc);
    }

    gtk_widget_destroy(dialog);
    return result == GTK_RESPONSE_OK;
#endif
}

bool dred_show_color_picker_dialog(dred_context* pDred, dred_window* pOwnerWindow, dred_color initialColor, dred_color* pColorOut)
{
    if (pDred == NULL || pColorOut == NULL) {
        return false;
    }

    if (pOwnerWindow == NULL) {
        pOwnerWindow = pDred->pMainWindow;
    }

#ifdef DRED_WIN32
    static COLORREF prevcolors[16] = {
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
    };

    CHOOSECOLORA cc;
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = pOwnerWindow->hWnd;
    cc.rgbResult = RGB(initialColor.r, initialColor.g, initialColor.b);
    cc.lpCustColors = prevcolors;
    cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN;

    if (!ChooseColorA(&cc)) {
        return false;
    }

    pColorOut->r = GetRValue(cc.rgbResult);
    pColorOut->g = GetGValue(cc.rgbResult);
    pColorOut->b = GetBValue(cc.rgbResult);
    pColorOut->a = 255;

    return true;
#endif

#ifdef DRED_GTK
    GtkWidget* dialog = gtk_color_chooser_dialog_new(NULL, GTK_WINDOW(pOwnerWindow->pGTKWindow));
    if (dialog == NULL) {
        return false;
    }

    GdkRGBA rgba;
    rgba.red   = initialColor.r / 255.0;
    rgba.green = initialColor.g / 255.0;
    rgba.blue  = initialColor.b / 255.0;
    rgba.alpha = initialColor.a / 255.0;
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &rgba);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &rgba);
    pColorOut->r = (uint8_t)(rgba.red * 255);
    pColorOut->g = (uint8_t)(rgba.green * 255);
    pColorOut->b = (uint8_t)(rgba.blue * 255);
    pColorOut->a = (uint8_t)(rgba.alpha * 255);

    gtk_widget_destroy(dialog);
    return result == GTK_RESPONSE_OK;
#endif
}


typedef struct
{
    dred_context* pDred;
    drte_engine* pTextEngine;
    dr2d_context* pPaintContext;
    dr2d_surface* pPaintSurface;
    dr2d_font* pFont;
    float offsetX;
    float offsetY;
    float pageSizeX;
    float pageSizeY;
    float scaleX;
    float scaleY;
} dred_print_data;

void dred__init_print_font(dred_print_data* pPrintData)
{
    dred_font* pFont = pPrintData->pDred->config.pTextEditorFont;
    dr2d_font_weight fontWeight = (dr2d_font_weight)pFont->desc.weight;
    dr2d_font_slant fontSlant = (dr2d_font_slant)pFont->desc.slant;

    pPrintData->pFont = dr2d_create_font(pPrintData->pPaintContext, pFont->desc.family, (unsigned int)(pFont->desc.size*pPrintData->scaleY), fontWeight, fontSlant, 0, 0);
    if (pPrintData->pFont == NULL) {
        return;
    }

    dr2d_font_metrics fontMetrics;
    dr2d_get_font_metrics(pPrintData->pFont, &fontMetrics);

    drte_engine_register_style_token(pPrintData->pTextEngine, (drte_style_token)pPrintData->pFont, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));
    drte_engine_set_default_style(pPrintData->pTextEngine, (drte_style_token)pPrintData->pFont);


    // Should probably move this to somewhere more appropriate.
    drte_engine_set_container_size(pPrintData->pTextEngine, pPrintData->pageSizeX, pPrintData->pageSizeY);
}

void dred__uninit_print_font(dred_print_data* pPrintData)
{
    dr2d_delete_font(pPrintData->pFont);
}

void dred__on_paint_rect_for_printing(drte_engine* pTextEngine, drte_style_token styleToken, drte_rect rect, void* pPaintData)
{
    (void)pTextEngine;
    (void)styleToken;
    (void)rect;
    (void)pPaintData;
}

void dred__on_paint_text_for_printing(drte_engine* pTextEngine, drte_style_token styleTokenFG, drte_style_token styleTokenBG, const char* text, size_t textLength, float posX, float posY, void* pPaintData)
{
    (void)pTextEngine;
    (void)styleTokenFG;
    (void)styleTokenBG;

    dred_print_data* pPrintData = pPaintData;
    assert(pPrintData != NULL);

    // Skip the line if it's partially visible - it'll be drawn on the next page.
    if (posY + drte_engine_get_line_height(pTextEngine) > drte_engine_get_container_height(pTextEngine)) {
        return;
    }

    dr2d_draw_text(pPrintData->pPaintSurface, pPrintData->pFont, text, (int)textLength, posX, posY, dr2d_rgb(0, 0, 0), dr2d_rgba(0, 0, 0, 0));
}

void dred__on_measure_string_for_printing(drte_engine* pTextEngine, drte_style_token styleToken, const char* text, size_t textLength, float* pWidthOut, float* pHeightOut)
{
    (void)pTextEngine;
    dr2d_measure_string((dr2d_font*)styleToken, text, textLength, pWidthOut, pHeightOut);
}

void dred__on_get_cursor_position_from_point_for_printing(drte_engine* pTextEngine, drte_style_token styleToken, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut)
{
    (void)pTextEngine;
    dr2d_get_text_cursor_position_from_point((dr2d_font*)styleToken, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosXOut, pCharacterIndexOut);
}

void dred__on_get_cursor_position_from_char_for_printing(drte_engine* pTextEngine, drte_style_token styleToken, const char* text, size_t characterIndex, float* pTextCursorPosXOut)
{
    (void)pTextEngine;
    dr2d_get_text_cursor_position_from_char((dr2d_font*)styleToken, text, characterIndex, pTextCursorPosXOut);
}

void dred__print_page(dred_print_data* pPrintData, size_t iPage)
{
    dr2d_begin_draw(pPrintData->pPaintSurface);
    {
        // Scroll to the page.
        drte_engine_set_inner_offset_y(pPrintData->pTextEngine, -(iPage * drte_engine_get_line_height(pPrintData->pTextEngine) * drte_engine_get_line_count_per_page(pPrintData->pTextEngine)));

        // Paint.
        drte_engine_paint(pPrintData->pTextEngine, drte_make_rect(0, 0, drte_engine_get_container_width(pPrintData->pTextEngine), drte_engine_get_container_height(pPrintData->pTextEngine)), pPrintData);
    }
    dr2d_end_draw(pPrintData->pPaintSurface);
}

#ifdef DRED_GTK
void dred_gtk__on_begin_print(GtkPrintOperation *pPrint, GtkPrintContext *context, gpointer user_data)
{
    dred_print_data* pPrintData = (dred_print_data*)user_data;
    assert(pPrintData != NULL);

    pPrintData->offsetX   = 0;
    pPrintData->offsetY   = 0;
    pPrintData->pageSizeX = gtk_print_context_get_width(context);
    pPrintData->pageSizeY = gtk_print_context_get_height(context);
    pPrintData->scaleX    = 1;
    pPrintData->scaleY    = 1;

    pPrintData->pPaintContext = dr2d_create_context_cairo();
    if (pPrintData->pPaintContext == NULL) {
        return;
    }

    pPrintData->pPaintSurface = dr2d_create_surface_cairo(pPrintData->pPaintContext, gtk_print_context_get_cairo_context(context));
    if (pPrintData->pPaintSurface == NULL) {
        return;
    }

    dred__init_print_font(pPrintData);

    gtk_print_operation_set_n_pages(pPrint, drte_engine_get_page_count(pPrintData->pTextEngine));
}

void dred_gtk__on_end_print(GtkPrintOperation *operation, GtkPrintContext *context, gpointer user_data)
{
    (void)operation;
    (void)context;

    dred_print_data* pPrintData = (dred_print_data*)user_data;
    assert(pPrintData != NULL);

    dred__uninit_print_font(pPrintData);
}

void dred_gtk__on_draw_page(GtkPrintOperation *pPrint, GtkPrintContext *context, gint page_nr, gpointer user_data)
{
    (void)pPrint;
    (void)context;

    dred__print_page((dred_print_data*)user_data, page_nr);
}
#endif

bool dred_show_print_dialog(dred_context* pDred, dred_window* pOwnerWindow, dred_print_info* pInfoOut)
{
    if (pDred == NULL || pInfoOut == NULL) {
        return false;
    }

    if (pOwnerWindow == NULL) {
        pOwnerWindow = pDred->pMainWindow;
    }

    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return false;
    }

    // Just return false if the focused editor does not support printing.
    if (!drgui_is_of_type(pFocusedEditor, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        return false;   // Focused editor does not support printing.
    }


    dred_print_data printData;
    printData.pDred = pDred;


    // When printing a text editor we need to use a different text engine for layout because the dimensions are different
    // and we need to force word wrap.
    printData.pTextEngine = drte_engine_create(&printData);
    if (printData.pTextEngine == NULL) {
        return false;
    }

    // Engine settings.
    drte_engine_enable_word_wrap(printData.pTextEngine);
    drte_engine_set_on_paint_text(printData.pTextEngine, dred__on_paint_text_for_printing);
    drte_engine_set_on_paint_rect(printData.pTextEngine, dred__on_paint_rect_for_printing);
    printData.pTextEngine->onMeasureString = dred__on_measure_string_for_printing;
    printData.pTextEngine->onGetCursorPositionFromPoint = dred__on_get_cursor_position_from_point_for_printing;
    printData.pTextEngine->onGetCursorPositionFromChar = dred__on_get_cursor_position_from_char_for_printing;

    // Set the text.
    size_t textLength = dred_text_editor_get_text(pFocusedEditor, NULL, 0);
    char* text = (char*)malloc(textLength + 1);
    if (text == NULL) {
        drte_engine_delete(printData.pTextEngine);
        return false;
    }

    dred_text_editor_get_text(pFocusedEditor, text, textLength+1);
    drte_engine_set_text(printData.pTextEngine, text);



#ifdef DRED_WIN32
    PRINTDLGA pd;
    ZeroMemory(&pd, sizeof(pd));
    pd.lStructSize = sizeof(pd);
    pd.hwndOwner = pOwnerWindow->hWnd;
    pd.Flags = PD_RETURNDC;
    if (!PrintDlgA(&pd)) {
        return false;
    }

    HDC hPrintDC = pd.hDC;

    int physicalWidth   = GetDeviceCaps(hPrintDC, PHYSICALWIDTH);
    int physicalHeight  = GetDeviceCaps(hPrintDC, PHYSICALHEIGHT);
    int physicalOffsetX = GetDeviceCaps(hPrintDC, PHYSICALOFFSETX);
    int physicalOffsetY = GetDeviceCaps(hPrintDC, PHYSICALOFFSETY);

    // TODO: Support custom margins.

    int printableWidth  = physicalWidth  - physicalOffsetX*2;
    int printableHeight = physicalHeight - physicalOffsetY*2;


    printData.offsetX   = (float)physicalOffsetX;
    printData.offsetY   = (float)physicalOffsetY;
    printData.pageSizeX = (float)printableWidth;
    printData.pageSizeY = (float)printableHeight;
    printData.scaleX    = GetDeviceCaps(hPrintDC, LOGPIXELSX) / 72.0f;
    printData.scaleY    = GetDeviceCaps(hPrintDC, LOGPIXELSY) / 72.0f;


    printData.pPaintContext = dr2d_create_context_gdi(hPrintDC);
    if (printData.pPaintContext == NULL) {
        return false;
    }

    printData.pPaintSurface = dr2d_create_surface_gdi_HDC(printData.pPaintContext, hPrintDC);
    if (printData.pPaintSurface == NULL) {
        return false;
    }

    dred__init_print_font(&printData);

    size_t pageCount = drte_engine_get_page_count(printData.pTextEngine);


    DOCINFOA di;
    ZeroMemory(&di, sizeof(di));
    di.cbSize = sizeof(di);
    di.lpszDocName = dred_editor_get_file_path(dred_get_focused_editor(pDred));
    if (StartDocA(hPrintDC, &di) <= 0) {
        return false;
    }

    // Pages.
    size_t iPageBeg = pd.nFromPage;
    size_t iPageEnd = pd.nToPage;
    if (iPageBeg == iPageEnd || iPageEnd > pageCount) {
        iPageEnd = pageCount;
    }

    for (WORD iCopy = 0; iCopy < pd.nCopies; ++iCopy) {
        for (size_t iPage = iPageBeg; iPage < iPageEnd; ++iPage) {
            if (StartPage(hPrintDC) > 0) {
                dred__print_page(&printData, iPage);
                EndPage(hPrintDC);
            }
        }
    }

    EndDoc(hPrintDC);


    dred__uninit_print_font(&printData);
    dr2d_delete_surface(printData.pPaintSurface);
    dr2d_delete_context(printData.pPaintContext);

    DeleteObject(hPrintDC);


    PDEVMODEA pDevMode = (PDEVMODEA)GlobalLock(pd.hDevMode);
    LPDEVNAMES pDevNames = (LPDEVNAMES)GlobalLock(pd.hDevNames); (void)pDevNames;

    //pDevMode->

    strcpy_s(pInfoOut->printerName, sizeof(pInfoOut->printerName), (char*)pDevMode->dmDeviceName);
    pInfoOut->firstPage = pd.nFromPage;
    pInfoOut->lastPage  = pd.nToPage;
    pInfoOut->copies    = pd.nCopies;

    if (pd.hDevMode != NULL) GlobalFree(pd.hDevMode);
    if (pd.hDevNames != NULL) GlobalFree(pd.hDevNames);
    return true;
#endif

#ifdef DRED_GTK
    GtkPrintOperation* pPrint = gtk_print_operation_new();
    if (pPrint == NULL) {
        return false;
    }

    // TODO: Set default settings based on previous settings.
    GtkPrintSettings* pSettings = gtk_print_settings_new();
    //gtk_print_settings_set_printer(pSettings, p)

    // TODO: Set page ranges based on settings reported by the currently focused editor.
    //GtkPageRange pageRange;
    //pageRange.start = 0;
    //pageRange.end = 128;
    //gtk_print_settings_set_page_ranges(pSettings, &pageRange, 1);

    gtk_print_operation_set_print_settings(pPrint, pSettings);

    g_signal_connect(pPrint, "begin_print", G_CALLBACK(dred_gtk__on_begin_print), &printData);
    g_signal_connect(pPrint, "draw_page", G_CALLBACK(dred_gtk__on_draw_page), &printData);

    GtkPrintOperationResult printResult = gtk_print_operation_run(pPrint, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW(pOwnerWindow->pGTKWindow), NULL);
    if (printResult != GTK_PRINT_OPERATION_RESULT_APPLY) {
        g_object_unref(pPrint);
        g_object_unref(pSettings);
        return false;
    }

    // Save settings so the next print operation can have it's default settings set to the previous print settings.
    GtkPrintSettings* pNewSettings = gtk_print_operation_get_print_settings(pPrint);
    if (pNewSettings != NULL) {
        strcpy_s(pInfoOut->printerName, sizeof(pInfoOut->printerName), gtk_print_settings_get_printer(pNewSettings));
        pInfoOut->copies = gtk_print_settings_get_n_copies(pNewSettings);

        // GTK supports multiple page ranges, however dred's abstraction currently does not. For now we just fall back to using
        // the first page range.
        //
        // TODO: Add support for page ranges.
        gint pageRangeCount;
        GtkPageRange* pPageRanges = gtk_print_settings_get_page_ranges(pNewSettings, &pageRangeCount);
        if (pPageRanges != NULL && pageRangeCount > 0) {
            pInfoOut->firstPage = pPageRanges[0].start;
            pInfoOut->lastPage = pPageRanges[0].end;
        } else {
            pInfoOut->firstPage = 0;
            pInfoOut->lastPage = 0;
        }

        //g_object_unref(pNewSettings);
    }

    g_object_unref(pSettings);
    g_object_unref(pPrint);
    return true;
#endif
}


void dred_show_about_dialog(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    if (dred_about_dialog_is_showing(pDred->pAboutDialog)) {
        return;
    }

    if (pDred->pAboutDialog == NULL) {
        pDred->pAboutDialog = dred_about_dialog_create(pDred);
        if (pDred->pAboutDialog == NULL) {
            return;
        }
    }

    assert(pDred->pAboutDialog != NULL);
    dred_about_dialog_show(pDred->pAboutDialog);
}

void dred_show_settings_dialog(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    if (dred_settings_dialog_is_showing(pDred->pSettingsDialog)) {
        return;
    }

    if (pDred->pSettingsDialog == NULL) {
        pDred->pSettingsDialog = dred_settings_dialog_create(pDred);
        if (pDred->pSettingsDialog == NULL) {
            return;
        }
    }

    assert(pDred->pSettingsDialog != NULL);
    dred_settings_dialog_show(pDred->pSettingsDialog);
}


void dred_update_main_window_layout(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    unsigned int windowWidth;
    unsigned int windowHeight;
    dred_window_get_client_size(pDred->pMainWindow, &windowWidth, &windowHeight);
    dred__update_main_window_layout(pDred->pMainWindow, (float)windowWidth, (float)windowHeight);
}


void dred_set_command_bar_text(dred_context* pDred, const char* text)
{
    if (pDred == NULL) {
        return;
    }

    dred_cmdbar_set_text(pDred->pCmdBar, text);
}

void dred_focus_command_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_capture_keyboard(pDred, pDred->pCmdBar);
}

void dred_focus_command_bar_and_set_text(dred_context* pDred, const char* text)
{
    dred_set_command_bar_text(pDred, text);
    dred_focus_command_bar(pDred);
}

void dred_unfocus_command_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    if (dred_cmdbar_has_keyboard_focus(pDred->pCmdBar)) {
        dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
        if (pFocusedEditor == NULL) {
            return;
        }

        dred_capture_keyboard(pDred, pFocusedEditor);
    }
}


void dred_update_info_bar(dred_context* pDred, dred_control* pControl)
{
    if (pDred == NULL) {
        return;
    }

    if (dred_get_focused_editor(pDred) != pControl) {
        return;
    }

    dred_cmdbar_update_info_bar(pDred->pCmdBar, pControl);
}


void dred_show_menu_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_window_show_menu(pDred->pMainWindow);
    pDred->config.showMenuBar = true;
}

void dred_hide_menu_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_window_hide_menu(pDred->pMainWindow);
    pDred->config.showMenuBar = false;
}

void dred_toggle_menu_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    if (pDred->config.showMenuBar) {
        dred_hide_menu_bar(pDred);
    } else {
        dred_show_menu_bar(pDred);
    }
}


void dred_show_tabbars(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
        dred_tabgroup_show_tabbar(pTabGroup);
    }

    pDred->config.showTabBar = true;
}

void dred_hide_tabbars(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
        dred_tabgroup_hide_tabbar(pTabGroup);
    }

    pDred->config.showTabBar = false;
}

void dred_toggle_tabbars(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    if (pDred->config.showTabBar) {
        dred_hide_tabbars(pDred);
    } else {
        dred_show_tabbars(pDred);
    }
}


void dred_show_line_numbers(dred_context* pDred)
{
    if (pDred == NULL || pDred->config.textEditorShowLineNumbers) {
        return;
    }

    pDred->config.textEditorShowLineNumbers = true;

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (drgui_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                dred_text_editor_show_line_numbers(pControl);
            }
        }
    }
}

void dred_hide_line_numbers(dred_context* pDred)
{
    if (pDred == NULL || !pDred->config.textEditorShowLineNumbers) {
        return;
    }

    pDred->config.textEditorShowLineNumbers = false;

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (drgui_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                dred_text_editor_hide_line_numbers(pControl);
            }
        }
    }
}

void dred_toggle_line_numbers(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    if (pDred->config.textEditorShowLineNumbers) {
        dred_hide_line_numbers(pDred);
    } else {
        dred_show_line_numbers(pDred);
    }
}


void dred_enable_word_wrap(dred_context* pDred)
{
    if (pDred == NULL || pDred->config.textEditorEnableWordWrap) {
        return;
    }

    pDred->config.textEditorEnableWordWrap = true;

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (drgui_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                dred_text_editor_enable_word_wrap(pControl);
            }
        }
    }
}

void dred_disable_word_wrap(dred_context* pDred)
{
    if (pDred == NULL || !pDred->config.textEditorEnableWordWrap) {
        return;
    }

    pDred->config.textEditorEnableWordWrap = false;

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (drgui_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                dred_text_editor_disable_word_wrap(pControl);
            }
        }
    }
}

void dred_toggle_word_wrap(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    if (pDred->config.textEditorEnableWordWrap) {
        dred_disable_word_wrap(pDred);
    } else {
        dred_enable_word_wrap(pDred);
    }
}


void dred_show_command_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_control_show(pDred->pCmdBar);
    dred_update_main_window_layout(pDred);
}

void dred_hide_command_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_control_hide(pDred->pCmdBar);
    dred_update_main_window_layout(pDred);
}

void dred_enable_auto_hide_command_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    pDred->config.autoHideCmdBar = true;

    if (!dred_cmdbar_has_keyboard_focus(pDred->pCmdBar)) {
        dred_hide_command_bar(pDred);
    }
}

void dred_disable_auto_hide_command_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    pDred->config.autoHideCmdBar = false;
    dred_show_command_bar(pDred);
}

void dred_toggle_auto_hide_command_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    if (pDred->config.autoHideCmdBar) {
        dred_disable_auto_hide_command_bar(pDred);
    } else {
        dred_enable_auto_hide_command_bar(pDred);
    }
}


void dred_set_text_editor_scale(dred_context* pDred, float scale)
{
    if (pDred == NULL) {
        return;
    }

    pDred->config.textEditorScale = dr_clamp(scale, 0.1f, 4.0f);

    // Every open text editors needs to be updated.
    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (drgui_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                dred_text_editor_set_text_scale(pControl, pDred->config.textEditorScale);
            }
        }
    }
}

float dred_get_text_editor_scale(dred_context* pDred)
{
    if (pDred == NULL) {
        return 0;
    }

    return pDred->config.textEditorScale;
}


dred_font* dred__load_system_font_ui(dred_context* pDred)
{
    dred_font_desc fontDesc;
    fontDesc.flags = 0;
    fontDesc.rotation = 0;

#ifdef _WIN32
    strcpy_s(fontDesc.family, sizeof(fontDesc.family), "Segoe UI");
    fontDesc.size = 12;
    fontDesc.weight = dred_gui_font_weight_normal;
    fontDesc.slant = dred_gui_font_slant_none;
#endif

#ifdef __linux__
    strcpy_s(fontDesc.family, sizeof(fontDesc.family), "sans");
    fontDesc.size = 13;
    fontDesc.weight = dred_gui_font_weight_normal;
    fontDesc.slant = dred_gui_font_slant_none;

    #if 1
    GSettings* settings = g_settings_new("org.mate.interface");
    if (settings != NULL) {
        char* fontName = g_settings_get_string(settings, "font-name");
        if (fontName != NULL) {
            PangoFontDescription* pPangoDesc = pango_font_description_from_string(fontName);
            if (pPangoDesc != NULL) {
                strcpy_s(fontDesc.family, sizeof(fontDesc.family), pango_font_description_get_family(pPangoDesc));

                gint size = pango_font_description_get_size(pPangoDesc);
                if (size > 0) {
                    if (pango_font_description_get_size_is_absolute(pPangoDesc)) {
                        fontDesc.size = size;
                    } else {
                        fontDesc.size = (unsigned int)(size/PANGO_SCALE * (96.0/72.0));
                    }
                }

                fontDesc.slant = dred_font_slant_from_pango(pango_font_description_get_style(pPangoDesc));
                fontDesc.weight = dred_font_weight_from_pango(pango_font_description_get_weight(pPangoDesc));

                pango_font_description_free(pPangoDesc);
            }
        }

        g_object_unref(settings);
    }
    #endif
#endif

    return dred_font_library_create_font(&pDred->fontLibrary, fontDesc.family, fontDesc.size, fontDesc.weight, fontDesc.slant, fontDesc.rotation, fontDesc.flags);
}

dred_font* dred__load_system_font_mono(dred_context* pDred)
{
    dred_font_desc fontDesc;
    fontDesc.flags = 0;
    fontDesc.rotation = 0;

#ifdef _WIN32
    strcpy_s(fontDesc.family, sizeof(fontDesc.family), "Consolas");
    fontDesc.size = 13;
    fontDesc.weight = dred_gui_font_weight_normal;
    fontDesc.slant = dred_gui_font_slant_none;

    // Fall back to Courier New by default for XP.
    OSVERSIONINFOA version;
    ZeroMemory(&version, sizeof(version));
    version.dwOSVersionInfoSize = sizeof(version);
    if (GetVersionExA(&version) && version.dwMajorVersion < 6) {
        strcpy_s(fontDesc.family, sizeof(fontDesc.family), "Courier New");
    }
#endif

#ifdef DRED_GTK
    strcpy_s(fontDesc.family, sizeof(fontDesc.family), "monospace");
    fontDesc.size = 13;
    fontDesc.weight = dred_gui_font_weight_normal;
    fontDesc.slant = dred_gui_font_slant_none;

    #if 0
    FcPattern* basepat = FcNameParse((const FcChar8*)"monospace");
    if (basepat != NULL) {
        FcConfigSubstitute(NULL, basepat, FcMatchPattern);
        FcDefaultSubstitute(basepat);

        FcResult result = FcResultNoMatch;
        FcPattern* fontpat = FcFontMatch(NULL, basepat, &result);
        if (fontpat != NULL && result == FcResultMatch) {
            FcPatternPrint(fontpat);

            FcChar8* family;
            FcPatternGetString(fontpat, FC_FAMILY, 0, &family);
            strcpy_s(fontDesc.family, sizeof(fontDesc.family), (const char*)family);
        }

        FcPatternDestroy(fontpat);
        FcPatternDestroy(basepat);
    }
    #endif

    #if 1
    GSettings* settings = g_settings_new("org.mate.interface");
    if (settings != NULL) {
        char* fontName = g_settings_get_string(settings, "monospace-font-name");
        if (fontName != NULL) {
            PangoFontDescription* pPangoDesc = pango_font_description_from_string(fontName);
            if (pPangoDesc != NULL) {
                strcpy_s(fontDesc.family, sizeof(fontDesc.family), pango_font_description_get_family(pPangoDesc));

                gint size = pango_font_description_get_size(pPangoDesc);
                if (size > 0) {
                    if (pango_font_description_get_size_is_absolute(pPangoDesc)) {
                        fontDesc.size = size;
                    } else {
                        fontDesc.size = (unsigned int)(size/PANGO_SCALE * (96.0/72.0));
                    }
                }

                fontDesc.slant = dred_font_slant_from_pango(pango_font_description_get_style(pPangoDesc));
                fontDesc.weight = dred_font_weight_from_pango(pango_font_description_get_weight(pPangoDesc));

                pango_font_description_free(pPangoDesc);
            }
        }

        g_object_unref(settings);
    }
    #endif
#endif

    return dred_font_library_create_font(&pDred->fontLibrary, fontDesc.family, fontDesc.size, fontDesc.weight, fontDesc.slant, fontDesc.rotation, fontDesc.flags);
}

dred_font* dred_parse_and_load_font(dred_context* pDred, const char* value)
{
    // Check for pre-defined fonts first.
    if (strcmp(value, "system-font-ui") == 0) {
        return dred__load_system_font_ui(pDred);
    }
    if (strcmp(value, "system-font-mono") == 0) {
        return dred__load_system_font_mono(pDred);
    }


    // The format of the font string is <family> <size> <weight> <slant>. The weight and slant are optional and default to normal weight and
    // no slant.
    dred_font_desc fontDesc;
    fontDesc.flags = 0;
    fontDesc.rotation = 0;
    fontDesc.weight = dred_gui_font_weight_normal;
    fontDesc.slant = dred_gui_font_slant_none;

    // Family.
    value = dr_next_token(value, fontDesc.family, sizeof(fontDesc.family));
    if (value == NULL) {
        return pDred->config.pUIFont;
    }

    // Size.
    char token[256];
    value = dr_next_token(value, token, sizeof(token));
    if (value == NULL) {
        return pDred->config.pUIFont;
    }

    int size = atoi(token);
    if (size < 0) {
        size = -size;
    }

    fontDesc.size = size;

    // Weight.
    value = dr_next_token(value, token, sizeof(token));
    if (value != NULL) {
        fontDesc.weight = dred_parse_font_weight(token);

        // Slant.
        value = dr_next_token(value, token, sizeof(token));
        if (value != NULL) {
            fontDesc.slant = dred_parse_font_slant(token);
        }
    }


    return dred_font_library_create_font(&pDred->fontLibrary, fontDesc.family, fontDesc.size, fontDesc.weight, fontDesc.slant, fontDesc.rotation, fontDesc.flags);
}


const char* dred_get_language_by_file_path(dred_context* pDred, const char* filePath)
{
    (void)pDred;

    // TODO: Use a map for this to make this easier to maintain. Also consider auto-generating default mappings.

    if (drpath_extension_equal(filePath, "c") || drpath_extension_equal(filePath, "h")) {
        return "c";
    }

    return "";
}


void dred_on_tab_activated(dred_context* pDred, dred_tab* pTab, dred_tab* pOldActiveTab)
{
    (void)pOldActiveTab;

    if (pDred == NULL) {
        return;
    }

    if (dred_tab_get_tabgroup(pTab) == dred_get_focused_tabgroup(pDred)) {
        dred_control* pControl = dred_tab_get_control(pTab);
        if (pControl == NULL) {
            return;
        }

        dred__update_window_title(pDred);

        if (drgui_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
            dred_window_set_menu(pDred->pMainWindow, pDred->menuLibrary.pMenu_TextEditor);
        }
        else {
            dred_window_set_menu(pDred->pMainWindow, pDred->menuLibrary.pMenu_Default);
        }

        dred_update_info_bar(pDred, pControl);
    }
}

void dred_on_tab_deactivated(dred_context* pDred, dred_tab* pTab, dred_tab* pNewActiveTab)
{
    if (pDred == NULL) {
        return;
    }

    if (pNewActiveTab == NULL && dred_tab_get_tabgroup(pTab) == dred_get_focused_tabgroup(pDred)) {
        dred_window_set_menu(pDred->pMainWindow, pDred->menuLibrary.pMenu_Default);
        dred_window_set_title(pDred->pMainWindow, "dred");
        dred_update_info_bar(pDred, NULL);
    }
}

void dred_on_accelerator(dred_context* pDred, dred_window* pWindow, size_t acceleratorIndex)
{
    (void)pWindow;

    dred_accelerator accelerator = pDred->shortcutTable.acceleratorTable.pAccelerators[acceleratorIndex];

    // The accelerator should be tied to a shortcut. We need to find that shortcut and execute it's command.
    for (size_t iShortcut = 0; iShortcut < pDred->shortcutTable.count; ++iShortcut) {
        dred_shortcut shortcut = pDred->shortcutTable.pShortcuts[iShortcut];
        if (pDred->queuedAccelerator.key == 0) {
            if (dred_accelerator_equal(shortcut.accelerators[0], accelerator)) {
                if (shortcut.accelerators[1].key == 0) {
                    const char* cmd = dred_shortcut_table_get_command_string_by_index(&pDred->shortcutTable, iShortcut);
                    if (cmd == NULL) {
                        return;
                    }

                    dred_exec(pDred, cmd, NULL);
                } else {
                    // There is a shortcut that uses this accelerator as it's first one. Thus, it needs to be queued.
                    pDred->queuedAccelerator = accelerator;
                }

                return;
            }
        } else {
            // An accelerator is queued.
            if (dred_shortcut_equal(shortcut, dred_shortcut_create(pDred->queuedAccelerator, accelerator))) {
                const char* cmd = dred_shortcut_table_get_command_string_by_index(&pDred->shortcutTable, iShortcut);
                if (cmd == NULL) {
                    pDred->queuedAccelerator = dred_accelerator_none();
                    return;
                }

                dred_exec(pDred, cmd, NULL);
            }
        }
    }

    // Make sure any queued accelerator is cleared.
    pDred->queuedAccelerator = dred_accelerator_none();
}

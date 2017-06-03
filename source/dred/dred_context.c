// Copyright (C) 2017 David Reid. See included LICENSE file.

float dred__get_cmd_bar_height(dred_context* pDred)
{
    if (pDred == NULL || !dred_control_is_visible(DRED_CONTROL(pDred->pCmdBar))) {
        return 0;
    }

    return dred_control_get_height(DRED_CONTROL(pDred->pCmdBar));
}

void dred__update_main_tab_group_container_layout(dred_context* pDred, dred_tabgroup_container* pContainer, float parentWidth, float parentHeight)
{
    if (pContainer == NULL) {
        return;
    }

    dred_control_set_size(DRED_CONTROL(pContainer), parentWidth, parentHeight - dred__get_cmd_bar_height(pDred));
    dred_tabgroup_refresh_styling(pDred->pMainTabGroup);
}

void dred__update_cmdbar_layout(dred_context* pDred, dred_cmdbar* pCmdBar, float parentWidth, float parentHeight)
{
    (void)pDred;

    if (pCmdBar == NULL) {
        return;
    }

    dred_control_set_size(DRED_CONTROL(pCmdBar), parentWidth, dred_control_get_height(DRED_CONTROL(pCmdBar)));
    dred_control_set_relative_position(DRED_CONTROL(pCmdBar), 0, parentHeight - dred__get_cmd_bar_height(pDred));

    if (pDred->pCmdBarPopup != NULL) {
        dred_window_set_size(pDred->pCmdBarPopup->pWindow, (int)parentWidth, 300);
        dred_cmdbar_popup_refresh_styling(pDred->pCmdBarPopup);
    }
}

void dred__update_background_layout(dred_context* pDred, dtk_control* pBackgroundControl, float parentWidth, float parentHeight)
{
    if (pBackgroundControl == NULL) {
        return;
    }

    dtk_control_set_size(pBackgroundControl, (dtk_uint32)parentWidth, (dtk_uint32)(parentHeight - dred__get_cmd_bar_height(pDred)));
}

void dred__update_main_window_layout(dred_window* pWindow, float windowWidth, float windowHeight)
{
    dred_context* pDred = pWindow->pDred;
    assert(pDred != NULL);

    dred__update_main_tab_group_container_layout(pDred, pDred->pMainTabGroupContainer, windowWidth, windowHeight);
    dred__update_cmdbar_layout(pDred, pDred->pCmdBar, windowWidth, windowHeight);
    dred__update_background_layout(pDred, &pDred->backgroundControl, windowWidth, windowHeight);
}


dr_bool32 dred_parse_cmdline__startup_files(const char* key, const char* value, void* pUserData)
{
    dred_context* pDred = (dred_context*)pUserData;
    assert(pDred != NULL);

    if (key == NULL) {
        dred_open_file(pDred, value);
        return DR_TRUE;
    }

    return DR_TRUE;
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
            if (dred_control_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_EDITOR)) {
                const char* filePath = dred_editor_get_file_path(DRED_EDITOR(pFocusedControl));
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

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pEditor));
    assert(pDred != NULL);

    if (dred_get_focused_tab(pDred) == pTab) {
        dred__update_window_title(pDred);
    }
}

void dred__on_editor_modified(dred_editor* pEditor)
{
    dred_tab* pTab = dred_find_control_tab(DRED_CONTROL(pEditor));
    if (pTab == NULL) {
        return;
    }

    // We need to show a "*" at the end of the tab's text to indicate that it's modified.
    dred__refresh_editor_tab_text(pEditor, pTab);
}

void dred__on_editor_unmodified(dred_editor* pEditor)
{
    dred_tab* pTab = dred_find_control_tab(DRED_CONTROL(pEditor));
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
    (void)posX;
    (void)posY;

    assert(pWindow != NULL);

    if (pWindow->pDred->pCmdBarPopup != NULL) {
        if (dtk_control_is_visible(DTK_CONTROL(&pWindow->pDred->pCmdBarPopup->pWindow->windowDTK))) {
            dred_cmdbar_popup_refresh_position(pWindow->pDred->pCmdBarPopup);
        }
    }

    int configWindowPosX;
    int configWindowPosY;
    dred_window_get_position(pWindow, &configWindowPosX, &configWindowPosY);

    pWindow->pDred->config.windowPosX = configWindowPosX;
    pWindow->pDred->config.windowPosY = configWindowPosY;
}

void dred_window_cb__on_main_window_size(dred_control* pControl, float width, float height)
{
    (void)height;

    dred_window* pWindow = dred_get_control_window(pControl);
    if (pWindow == NULL) {
        return;
    }

    dred__update_main_window_layout(pWindow, width, height);

    if (pWindow->pDred->pCmdBarPopup != NULL) {
        if (dtk_control_is_visible(DTK_CONTROL(&pWindow->pDred->pCmdBarPopup->pWindow->windowDTK))) {
            dred_cmdbar_popup_refresh_position(pWindow->pDred->pCmdBarPopup);
        }
    }


    // The config needs to be updated so that it's settings are serialized.
    unsigned int windowWidth;
    unsigned int windowHeight;
    dred_window_get_size(pWindow, &windowWidth, &windowHeight);

    pWindow->pDred->config.windowWidth = windowWidth;
    pWindow->pDred->config.windowHeight = windowHeight;
    pWindow->pDred->config.windowMaximized = dred_window_is_maximized(pWindow);
}


dred_file dred__open_log_file(dred_context* pDred)
{
    char logFilePath[DRED_MAX_PATH];
    if (!dred_get_log_path(pDred, logFilePath, sizeof(logFilePath))) {
        return NULL;
    }

    // Make sure the folder exists.
    char logFolderPath[DRED_MAX_PATH];
    if (dred_get_log_folder_path(pDred, logFolderPath, sizeof(logFolderPath))) {
        dr_mkdir_recursive(logFolderPath);
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


void dred_create_config_file_if_not_exists(dred_context* pDred, const char* fileName, const char* configString)
{
    char configFolderPath[DRED_MAX_PATH];
    if (!dred_get_config_folder_path(pDred, configFolderPath, sizeof(configFolderPath))) {
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


dtk_thread_result DTK_THREADCALL dred_ipc_message_proc(void* pData)
{
    dred_context* pDred = (dred_context*)pData;
    assert(pDred != NULL);

#if 0
    while (pDred->pipeIPC != NULL) {
        if (drpipe_connect(pDred->pipeIPC) != dripc_result_success) {
            return 0;
        }
    }
#endif

    while (!pDred->isClosing) {
        char pipeName[256];
        if (!dred_ipc_get_pipe_name(pipeName, sizeof(pipeName))) {
            break;
        }

        drpipe server;
        if (drpipe_open_named_server(pipeName, DR_IPC_READ, &server) != dripc_result_success) {
            return 0;
        }

        // We may have connected a temporary client during the shutdown procedure in order to return from the call above. Here is where we close.
        if (pDred->isClosing) {
            drpipe_close(server);
            return 0;
        }


        void* pMsgData;
        dred_ipc_message_header header;
        while (dred_ipc_read_message(server, &header, &pMsgData)) {
            dr_bool32 foundTerminator = DR_FALSE;
            switch (header.message)
            {
                case DRED_IPC_MESSAGE_TERMINATOR:
                {
                    foundTerminator = DR_TRUE;
                } break;

                default:
                {
                    dred_window_send_ipc_message_event(pDred->pMainWindow, header.message, pMsgData, header.size);
                } break;
            }

            free(pMsgData);

            if (foundTerminator) {
                break;
            }
        }

        drpipe_close(server);
    }

    return 0;
}


static dtk_bool32 dred_background_control_event_handler(dtk_event* pEvent)
{
    dred_context* pDred = (dred_context*)pEvent->pTK->pUserData;
    assert(pDred != NULL);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dtk_surface_draw_rect(pEvent->paint.pSurface, dtk_control_get_local_rect(pEvent->pControl), pDred->config.tabgroupBGColor);
        } break;
    }

    return dtk_control_default_event_handler(pEvent);
}

static dtk_bool32 dred_dtk_global_event_proc(dtk_event* pEvent)
{
    dred_context* pDred = (dred_context*)pEvent->pTK->pUserData;
    assert(pDred != NULL);

    switch (pEvent->type)
    {
        case DTK_EVENT_APPLICATION_FONT:
        {
            switch (pEvent->applicationFont.type)
            {
                case dtk_application_font_type_monospace:
                {
                    pEvent->applicationFont.pFont = &pDred->config.pSystemFontMono->fontDTK;
                } break;
                
                case dtk_application_font_type_ui:
                default:
                {
                    pEvent->applicationFont.pFont = &pDred->config.pUIFont->fontDTK;
                } break;
            }
        } return DTK_FALSE; // Don't break here! Return false to ensure the default event handler is not run.

        case DTK_EVENT_APPLICATION_SCALE:
        {
            pEvent->applicationScale.scale = pDred->config.uiScale;
        } return DTK_FALSE; // Don't break here! Return false to ensure the default event handler is not run.

        case DTK_EVENT_ACCELERATOR:
        {
            dred_on_accelerator((dred_context*)pEvent->pTK->pUserData, dtk_accelerator_init(pEvent->accelerator.key, pEvent->accelerator.modifiers, pEvent->accelerator.id));
        } break;

        // Currently, DTK does not reliably pass the window that owns the menu that triggered the DTK_EVENT_MENU event on
        // all platforms (looking at you GTK!). Therefore, we need to handle these here in the global event handler rather
        // than in the window's event handler.
        case DTK_EVENT_MENU:
        {
            // If the menu has an explicit command, use that. If not, check for the shortcut and use _it's_ command instead.
            const char* command = dred_menu_item_table_get_command(&pDred->menuItemTable, pEvent->menu.itemID);
            if (dtk_string_is_null_or_empty(command)) {
                const char* shortcut = dred_menu_item_table_get_shortcut(&pDred->menuItemTable, pEvent->menu.itemID);
                if (!dtk_string_is_null_or_empty(shortcut)) {
                    command = dred_shortcut_table_get_command_string_by_name(&pDred->shortcutTable, shortcut);
                }
            }

            if (!dtk_string_is_null_or_empty(command)) {
                dred_exec(pDred, command, NULL);
            }
        } break;

        case DTK_EVENT_CUSTOM:
        {
            switch (pEvent->custom.id)
            {
                case DRED_EVENT_IPC_ACTIVATE:
                case DRED_EVENT_IPC_OPEN:
                {
                    dred_on_ipc_message(pDred, pEvent->custom.id, pEvent->custom.pData);
                } break;
                default: break;
            }
        } break;

        default: break;
    }

    return dtk_default_event_handler(pEvent);
}

static dtk_bool32 dred_main_window_event_handler(dtk_event* pEvent)
{
    dtk_window* pWindow = DTK_WINDOW(pEvent->pControl);
    dred_context* pDred = (dred_context*)pEvent->pTK->pUserData;

    switch (pEvent->type)
    {
        case DTK_EVENT_DPI_CHANGED:
        {
            dtk_window_set_absolute_position(pWindow, pEvent->dpiChanged.suggestedPosX, pEvent->dpiChanged.suggestedPosY);
            dtk_window_set_size(pWindow, pEvent->dpiChanged.suggestedWidth, pEvent->dpiChanged.suggestedHeight);

            dred_refresh_layout(pDred);

            dtk_control_scheduled_redraw(DTK_CONTROL(pWindow), dtk_control_get_local_rect(DTK_CONTROL(pWindow)));
        } break;

        case DTK_EVENT_REFRESH_LAYOUT:
        {
            dred_cmdbar_refresh_styling(&pDred->cmdBar);
            dred_update_main_window_layout(pDred);

            // Any open editors need to have their layouts updated.
            for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
                for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
                    dtk_control_refresh_layout(DTK_CONTROL(dred_tab_get_control(pTab)));
                }
            }
        } break;

        default: break;
    }

    return dred_dtk_window_event_handler(pEvent);
}


dr_bool32 dred_init(dred_context* pDred, dr_cmdline cmdline, dred_package_library* pPackageLibrary)
{
    int windowPosX;
    int windowPosY;
    unsigned int windowWidth;
    unsigned int windowHeight;
    dr_bool32 showWindowMaximized;

    if (pDred == NULL) return DR_FALSE;
    dtk_zero_object(pDred);

    // Initialize the toolkit first.
    if (dtk_init(&pDred->tk, dred_dtk_global_event_proc, pDred) != DTK_SUCCESS) {
        return DTK_FALSE;
    }

    dred_platform_init(&pDred->tk); // <-- This is only temporary while DTK is being integrated.


    // The string pool is initialized with data from the pre-build tool. It contains strings for stock shortcuts, menus, etc.
    dred_string_pool_init(&pDred->stringPool, (const char*)g_InitialStringPoolData, sizeof(g_InitialStringPoolData));


    pDred->pGUI = &pDred->gui;

    pDred->cmdline = cmdline;
    pDred->pPackageLibrary = pPackageLibrary;

#ifdef DRED_PORTABLE
    pDred->isPortable = DTK_TRUE
#endif
    if (dr_cmdline_key_exists(&cmdline, "portable")) {
        pDred->isPortable = DTK_TRUE;
    }
    if (dr_cmdline_key_exists(&cmdline, "no-portable")) {
        pDred->isPortable = DTK_FALSE;
    }


    // Make sure the user's config directory exists.
    char configFolderPath[DRED_MAX_PATH];
    dred_get_config_folder_path(pDred, configFolderPath, sizeof(configFolderPath));
    dr_mkdir_recursive(configFolderPath);


    // Open the log file first to ensure we're able to log as soon as possible.
    pDred->logFile = dred__open_log_file(pDred);


    // The GUI.
    if (!dred_gui_init_dtk(pDred->pGUI, pDred)) {
        goto on_error;
    }

    // The GUI needs to be linked to the window system.
    dred_platform_bind_gui(pDred->pGUI);
    dred_platform_bind_logging(pDred);


    // The font library. This needs to be initialized before loading any fonts and configs.
    if (!dred_font_library_init(&pDred->fontLibrary, pDred)) {
        goto on_error;
    }

    // The image library. This needs to be initialized before loading any images and configs.
    if (!dred_image_library_init(&pDred->imageLibrary, pDred)) {
        goto on_error;
    }


    // Shortcut table.
    if (!dred_shortcut_table_init(pDred, &pDred->shortcutTable, DRED_STOCK_SHORTCUT_COUNT)) {
        goto on_error;
    }

    dred_init_stock_shortcuts__autogenerated(pDred);

    // Before loading configs we want to make sure any stock themes and settings are present.
    dred_create_config_file_if_not_exists(pDred, ".dred", "");
    dred_create_config_file_if_not_exists(pDred, "dark.dredtheme", g_StockTheme_Dark);
    dred_create_config_file_if_not_exists(pDred, "light.dredtheme", g_StockTheme_Light);


    // Config
    //
    // The config is loaded in 4 stages. The first initializes it to it's default values, the second loads the .dredprivate file from the main
    // user directory, the third reads the .dred file from the main user directory, and the4th loads the .dred file sitting in the working directory.
    dred_config_init(&pDred->config, pDred);

    char configPath[DRED_MAX_PATH];
    if (dred_get_config_path(pDred, configPath, sizeof(configPath))) {
        char configPathPrivate[DRED_MAX_PATH];
        strcpy_s(configPathPrivate, sizeof(configPathPrivate), configPath);
        strcat_s(configPathPrivate, sizeof(configPathPrivate), "private");
        dred_load_config(pDred, configPathPrivate);
        dred_load_config(pDred, configPath);
    } else {
        dred_warning(pDred, "Failed to load .dred config file from user directory. The most likely cause of this is that the path is too long.");
    }

    dred_load_config(pDred, ".dred");


    // Stock menus should be initialized after the shortcut table and configs because it will need access to the initial shortcut bindings
    // and recent files.
    dred_init_stock_menus__autogenerated(pDred);

    // The menu item table is what's used to bind menu items to commands and shortcuts. This table will be used for looking up the command
    // to execute when a menu item is pressed.
    if (dred_menu_item_table_init(pDred, &pDred->menuItemTable) != DRED_SUCCESS) {
        goto on_error;
    }

    dred_refresh_recent_files_menu(pDred);
    dred_refresh_favourite_files_menu(pDred);



    // The main window.
    pDred->pMainWindow = dred_window_create(pDred, dred_main_window_event_handler);
    if (pDred->pMainWindow == NULL) {
        dred_error(pDred, "Failed to create main window.");
        goto on_error;
    }

    pDred->pMainWindow->onClose = dred_window_cb__on_main_window_close;
    pDred->pMainWindow->onMove = dred_window_cb__on_main_window_move;
    dred_control_set_on_size(pDred->pMainWindow->pRootGUIControl, dred_window_cb__on_main_window_size);


    if (dtk_control_init(&pDred->tk, DTK_CONTROL(pDred->pMainWindow->pRootGUIControl), DTK_CONTROL_TYPE_EMPTY, dred_background_control_event_handler, &pDred->backgroundControl) != DTK_SUCCESS) {
        dred_error(pDred, "Failed to create background control.\n");
        goto on_error;
    }

    dtk_control_hide(&pDred->backgroundControl);


    // The main tab group container.
    pDred->pMainTabGroupContainer = &pDred->mainTabGroupContainer;
    if (!dred_tabgroup_container_init(pDred->pMainTabGroupContainer, pDred, pDred->pMainWindow->pRootGUIControl)) {
        dred_error(pDred, "Failed to create main tab group container.\n");
        goto on_error;
    }

    pDred->pMainTabGroup = &pDred->mainTabGroup;
    if (!dred_tabgroup_init(pDred->pMainTabGroup, pDred, DRED_CONTROL(pDred->pMainTabGroupContainer))) {
        dred_error(pDred, "Failed to create main tab group.\n");
        goto on_error;
    }


    // The command bar. Ensure this is given a valid initial size.
    pDred->pCmdBar = &pDred->cmdBar;
    if (!dred_cmdbar_init(pDred->pCmdBar, pDred, pDred->pMainWindow->pRootGUIControl)) {
        dred_error(pDred, "Failed to create command bar.\n");
        goto on_error;
    }

    if (pDred->config.autoHideCmdBar) {
        dred_control_hide(DRED_CONTROL(pDred->pCmdBar));
    }

    pDred->pCmdBarPopup = dred_cmdbar_popup_create(pDred);



    // Show the window last to ensure child GUI elements have been initialized and in a valid state. This should be done before
    // opening the files passed on the command line, however, because the window needs to be shown in order for it to receive
    // keyboard focus.
    windowPosX = pDred->config.windowPosX;
    windowPosY = pDred->config.windowPosY;
    windowWidth =  (unsigned int)pDred->config.windowWidth;
    windowHeight = (unsigned int)pDred->config.windowHeight;
    showWindowMaximized = pDred->config.windowMaximized;

    dred_window_set_title(pDred->pMainWindow, "dred");
    dred_window_set_menu(pDred->pMainWindow, &pDred->menus.nothingopen);
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
        dtk_window_move_into_view(&pDred->pMainWindow->windowDTK);  // <-- This just makes sure the window is in view in case the config has an erroneous position setting.
    }

    // We only want to use the default window position on first run.
    pDred->config.useDefaultWindowPos = DR_FALSE;




    // Load initial files from the command line.
    dr_parse_cmdline(&pDred->cmdline, dred_parse_cmdline__startup_files, pDred);

    // If there were no files passed on the command line, start with an empty text file. We can know this by simply finding the
    // focused editor. If it's null, nothing is open.
    if (dred_get_focused_editor(pDred) == NULL) {
        dred_open_new_text_file(pDred);
    }


    // This is a little hack to ensure the maximized state is saved correctly.
    pDred->config.windowMaximized = showWindowMaximized;


    // Create the IPC server pipe last to ensure the context is in a valid when messages are received.
    if (!dr_cmdline_key_exists(&cmdline, "noipc")) {
        if (dtk_thread_create(&pDred->threadIPC, dred_ipc_message_proc, pDred)) {
        }
    }





    pDred->isInitialized = DR_TRUE;
    return DR_TRUE;

on_error:
    dred_uninit(pDred);
    return DR_FALSE;
}

void dred_uninit(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    pDred->isClosing = DR_TRUE;


    // Make sure any lingering tabs are forcefully closed. This should be done at a higher level so that the user
    // can be prompted to save any unsaved work or whatnot, but I'm keeping this here for sanity.
    dred_close_all_tabs(pDred);


    // The IPC thread may be waiting for a client connection. To break from the loop we'll need to create a temporary
    // client in order to break from it.
    char pipeName[256];
    if (dred_ipc_get_pipe_name(pipeName, sizeof(pipeName))) {
        drpipe tempClientPipe;
        drpipe_open_named_client(pipeName, DR_IPC_WRITE, &tempClientPipe);
        dtk_thread_wait(&pDred->threadIPC);
        drpipe_close(tempClientPipe);
    }


    if (pDred->pAboutDialog) {
        dred_about_dialog_delete(pDred->pAboutDialog);
    }


    if (pDred->pCmdBar) {
        dred_cmdbar_uninit(pDred->pCmdBar);
    }

    if (pDred->pMainTabGroup) {
        dred_tabgroup_uninit(pDred->pMainTabGroup);
    }

    if (pDred->pMainTabGroupContainer) {
        dred_tabgroup_container_uninit(pDred->pMainTabGroupContainer);
    }

    dred_menu_item_table_uninit(&pDred->menuItemTable);

    if (pDred->pMainWindow) {
        dred_window_delete(pDred->pMainWindow);
    }

    dred_config_uninit(&pDred->config);
    dred_shortcut_table_uninit(&pDred->shortcutTable);

    dred_image_library_uninit(&pDred->imageLibrary);
    dred_font_library_uninit(&pDred->fontLibrary);

    if (pDred->pGUI) {
        dred_gui_uninit(pDred->pGUI);
    }

    if (pDred->logFile) {
        dred_file_close(pDred->logFile);
    }

    dtk_uninit(&pDred->tk);
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

    // This will return DR_FALSE if the user hits the cancel button.
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


dr_bool32 dred_load_config(dred_context* pDred, const char* configFilePath)
{
    if (pDred == NULL || configFilePath == NULL) {
        return DR_FALSE;
    }

    return dred_config_load_file(&pDred->config, configFilePath, dred_config__on_error, pDred);
}


dr_bool32 dred_exec(dred_context* pDred, const char* cmd, dred_command* pLastCmdOut)
{
    if (pDred == NULL || cmd == NULL) {
        return DR_FALSE;
    }

    dred_command_separator cmdSeparator = dred_command_separator_none;

    char subcmd[4096];
    while ((cmd = dred_next_command_string(cmd, subcmd, sizeof(subcmd), &cmdSeparator)) != NULL) {
        const char* value;
        dred_command command;
        if (dred_find_command(subcmd, &command, &value)) {
            if (pLastCmdOut) *pLastCmdOut = command;

            dr_bool32 result = command.proc(pDred, value);
            if (result) {
                if (cmdSeparator == dred_command_separator_or) {
                    return DR_TRUE;
                }
            } else {
                if (cmdSeparator == dred_command_separator_and || cmdSeparator == dred_command_separator_none) {
                    return DR_FALSE;
                }
            }
        }
    }

    return DR_TRUE;
}

dr_bool32 dred_bind_shortcut(dred_context* pDred, dtk_uint32 id, const char* name, const char* cmdStr, dtk_uint32 acceleratorCount, dtk_accelerator* pAccelerators)
{
    if (pDred == NULL) {
        return DR_FALSE;
    }

    if (!dred_shortcut_table_bind(&pDred->shortcutTable, id, name, cmdStr, acceleratorCount, pAccelerators)) {
        return DR_FALSE;
    }

    return DR_TRUE;
}


void dred_save_dredprivate(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }


    char dredprivatePath[DRED_MAX_PATH];
    dred_get_config_folder_path(pDred, dredprivatePath, sizeof(dredprivatePath));

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

        const char* shortcutName = dred_string_pool_cstr(&pDred->stringPool, pDred->shortcutTable.pShortcuts[i].nameOffset);
        const char* shortcutCmd = dred_string_pool_cstr(&pDred->stringPool, pDred->shortcutTable.pShortcuts[i].cmdOffset);

        char bindingStr[4096];
        if (snprintf(bindingStr, sizeof(bindingStr), "bind \"%s\" \"%s\" \"%s\"\n", shortcutName, shortcutStr, shortcutCmd) < 0) {
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

    // Favourite files.
    for (size_t i = 0; i < pDred->config.favouriteFileCount; ++i) {
        dred_file_write_string(file, "favourite-file \"");
        dred_file_write_string(file, pDred->config.favouriteFiles[i]);
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

    dred_gui_capture_keyboard(pControl);
}

void dred_release_keyboard(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_gui_release_keyboard(pDred->pGUI);
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

    if (!dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
        return NULL;
    }

    return DRED_EDITOR(pControl);
}

dtk_control* dred_get_element_with_keyboard_capture(dred_context* pDred)
{
    if (pDred == NULL) {
        return NULL;
    }

    dtk_control* pControl = dtk_get_control_with_keyboard_capture(&pDred->tk);
    if (pControl == NULL) {
        pControl = pDred->pMainWindow->windowDTK.pLastDescendantWithKeyboardFocus;
    }

    return pControl;
}


dr_bool32 dred__save_editor(dred_editor* pEditor, const char* newFilePath, dred_tab* pTab)
{
    if (!dred_editor_save(pEditor, newFilePath)) {
        return DR_FALSE;
    }

    dred__refresh_editor_tab_text(pEditor, pTab);
    return DR_TRUE;
}


#define DRED_FOREACH_PACKAGE(pDred, pPackage) \
    size_t iPackage = 0; \
    dred_package* pPackage = NULL; \
    while ((pPackage = dred_package_library_get_package(pDred->pPackageLibrary, iPackage++)) != NULL)


const char* dred_get_editor_type_by_path(dred_context* pDred, const char* filePath)
{
    if (filePath == NULL) return NULL;

    // Check for known extensions first as a performance optimization. If that fails we'll want to open either open the file and inspect it,
    // or look through extensions.
    if (drpath_extension_equal(filePath, "txt")) {
        return DRED_CONTROL_TYPE_TEXT_EDITOR;
    }

    // We need to look at packages and determine which one, if any, is able to open the file. If we can't find any, we fall back
    // to the text or hex editor.
    DRED_FOREACH_PACKAGE(pDred, pPackage) {
        if (pPackage->cbs.editor.getEditorTypeByPath) {
            const char* type = pPackage->cbs.editor.getEditorTypeByPath(pPackage, pDred, filePath);
            if (type != NULL) {
                return type;
            }
        }
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
            if (pControl != NULL && dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
                if (drpath_equal(dred_editor_get_file_path(DRED_EDITOR(pControl)), filePathAbsoluteClean)) {
                    return pTab;
                }
            }
        }
    }

    return NULL;
}

dr_bool32 dred_open_file(dred_context* pDred, const char* filePath)
{
    return dred_open_file_by_type(pDred, filePath, dred_get_editor_type_by_path(pDred, filePath));
}

dr_bool32 dred_open_file_by_type(dred_context* pDred, const char* filePath, const char* editorType)
{
    char filePathAbsolute[DRED_MAX_PATH];
    if (filePath != NULL && filePath[0] != '\0') {
        if (!dred_to_absolute_path(filePath, filePathAbsolute, sizeof(filePathAbsolute))) {
            dred_errorf(pDred, "File path is too long %s\n", filePath);
            return DR_FALSE;
        }
    } else {
        filePathAbsolute[0] = '\0';
    }

    // If the file is already open, activate it's tab.
    if (filePath != NULL) {
        dred_tab* pExistingTab = dred_find_editor_tab_by_absolute_path(pDred, filePathAbsolute);
        if (pExistingTab != NULL) {
            dred_tabgroup_activate_tab(dred_tab_get_tabgroup(pExistingTab), pExistingTab);
            return DR_TRUE;
        }
    }


    // Before creating the editor we'll want to identify the tab group to attach it to.
    dred_tabgroup* pTabGroup = dred_get_focused_tabgroup(pDred);
    if (pTabGroup == NULL) {
        return DR_FALSE;   // TODO: This means there is no tab group so one needs to be created.
    }

    //dred_control_begin_dirty(DRED_CONTROL(pTabGroup));
    {
        dred_editor* pEditor = dred_create_editor_by_type(pDred, pTabGroup, editorType, filePathAbsolute);
        if (pEditor == NULL) {
            //dred_control_end_dirty(DRED_CONTROL(pTabGroup));
            return DR_FALSE;
        }

        dred_editor_set_on_modified(pEditor, dred__on_editor_modified);
        dred_editor_set_on_unmodified(pEditor, dred__on_editor_unmodified);

        // We have the editor, so now we need to create a tab an associate the new editor with it.
        dred_tab* pTab = dred_tabgroup_prepend_tab(pTabGroup, NULL, DRED_CONTROL(pEditor));
        if (pTab == NULL) {
            dred_delete_editor_by_type(pEditor);
            //dred_control_end_dirty(DRED_CONTROL(pTabGroup));
            return DR_FALSE;
        }
        dred__refresh_editor_tab_text(pEditor, pTab);
        dred_tabgroup_activate_tab(pTabGroup, pTab);


        // If there is only one other tab, and it's an new, unmodified file, close it.
        if (pTab->pNextTab != NULL && pTab->pNextTab->pNextTab == NULL) {
            dred_editor* pOtherEditor = DRED_EDITOR(dred_tab_get_control(pTab->pNextTab));
            if (dred_control_is_of_type(DRED_CONTROL(pOtherEditor), DRED_CONTROL_TYPE_EDITOR) && !dred_editor_is_modified(pOtherEditor)) {
                const char* pOtherEditorFile = dred_editor_get_file_path(pOtherEditor);
                if (pOtherEditorFile == NULL || pOtherEditorFile[0] == '\0') {
                    // It's a new unmodified file. Close it.
                    dred_close_tab(pDred, pTab->pNextTab);
                }
            }
        }
    }
    //dred_control_end_dirty(DRED_CONTROL(pTabGroup));


    // The file should be added to the recent file list. This is done by adding it to the config and then refreshing the menu.
    if (filePathAbsolute[0] != '\0') {
        dred_config_push_recent_file(&pDred->config, filePathAbsolute);
        dred_refresh_recent_files_menu(pDred);
    }


    // Make sure the tab group is visible.
    dtk_control_hide(&pDred->backgroundControl);
    dtk_control_show(DTK_CONTROL(pTabGroup)->pParent);

    return DR_TRUE;
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

    dred_editor* pEditor = DRED_EDITOR(dred_tab_get_control(pTab));

    // Delete the tab.
    dred_tabgroup_delete_tab(dred_tab_get_tabgroup(pTab), pTab);

    // Delete the control.
    if (dred_control_is_of_type(DRED_CONTROL(pEditor), DRED_CONTROL_TYPE_EDITOR)) {
        dred_delete_editor_by_type(pEditor);
    }

    // If after closing the tab there are no other active tabs, activate the command bar.
    if (dred_get_focused_tab(pDred) == NULL) {
        dtk_control_hide(DTK_CONTROL(pDred->pMainTabGroupContainer));
        dtk_control_show(&pDred->backgroundControl);
        dred_focus_command_bar(pDred);
    }
}

void dred_close_tab_with_confirmation(dred_context* pDred, dred_tab* pTab)
{
    dred_editor* pEditor = DRED_EDITOR(dred_tab_get_control(pTab));
    if (pEditor == NULL) {
        dred_close_tab(pDred, pTab);
        return;
    }

    if (!dred_control_is_of_type(DRED_CONTROL(pEditor), DRED_CONTROL_TYPE_EDITOR)) {
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
        if (result == DTK_DIALOG_RESULT_YES) {
            if (!dred__save_editor(pEditor, NULL, pTab)) {
                char newFilePath[DRED_MAX_PATH];
                if (!dred_show_save_file_dialog(pDred, filePath, newFilePath, sizeof(newFilePath))) {
                    return;
                }
                dred_editor_save(pEditor, newFilePath);
            }
            dred_close_tab(pDred, pTab);
        } else if (result == DTK_DIALOG_RESULT_NO) {
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

dr_bool32 dred_close_all_tabs_with_confirmation(dred_context* pDred)
{
    if (pDred == NULL) {
        return DR_FALSE;
    }

    // If there's any modified files we need to show a dialog box.
    if (dred_are_any_open_files_modified(pDred)) {
        unsigned int result = dred_show_yesnocancel_dialog(pDred, "You have unsaved changes. Save changes?", "Save changes?");
        if (result == DTK_DIALOG_RESULT_YES) {
            if (!dred_save_all_open_files_with_saveas(pDred)) {
                return DR_TRUE;
            }
        } else if (result == DTK_DIALOG_RESULT_CANCEL) {
            return DR_FALSE;
        }
    }

    dred_close_all_tabs(pDred);
    return DR_TRUE;
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


dr_bool32 dred_save_focused_file(dred_context* pDred, const char* newFilePath)
{
    if (pDred == NULL) {
        return DR_FALSE;
    }

    dred_tab* pFocusedTab = dred_get_focused_tab(pDred);
    if (pFocusedTab == NULL) {
        return DR_FALSE;
    }

    dred_control* pFocusedControl = dred_tab_get_control(pFocusedTab);
    if (pFocusedControl == NULL) {
        return DR_FALSE;
    }

    // Editor.
    if (dred_control_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_EDITOR)) {
        return dred__save_editor(DRED_EDITOR(pFocusedControl), newFilePath, pFocusedTab);
    }

    // Output.
    //
    // TODO: Add support for saving the output window.


    // Add the file to the recent file's list.
    char absoluteFilePath[DRED_MAX_PATH];
    dred_to_absolute_path(newFilePath, absoluteFilePath, sizeof(absoluteFilePath));
    if (absoluteFilePath[0] != '\0') {
        dred_config_push_recent_file(&pDred->config, absoluteFilePath);
        dred_refresh_recent_files_menu(pDred);
    }

    return DR_FALSE;
}

dr_bool32 dred_save_focused_file_as(dred_context* pDred)
{
    if (pDred == NULL) {
        return DR_FALSE;
    }

    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    char newFilePath[DRED_MAX_PATH];
    if (!dred_show_save_file_dialog(pDred, dred_editor_get_file_path(pFocusedEditor), newFilePath, sizeof(newFilePath))) {
        return DR_FALSE;
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
            if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
                dred__save_editor(DRED_EDITOR(pControl), NULL, pTab);
            }
        }
    }
}

dr_bool32 dred_save_all_open_files_with_saveas(dred_context* pDred)
{
    if (pDred == NULL) {
        return DR_FALSE;
    }

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
                if (!dred__save_editor(DRED_EDITOR(pControl), NULL, pTab)) {
                    char newFileName[DRED_MAX_PATH];
                    if (!dred_show_save_file_dialog(pDred, dred_editor_get_file_path(DRED_EDITOR(pControl)), newFileName, sizeof(newFileName))) {
                        return DR_FALSE;
                    }

                    if (!dred_editor_save(DRED_EDITOR(pControl), newFileName)) {
                        return DR_FALSE;
                    }
                }
            }
        }
    }

    return DR_TRUE;
}


dr_bool32 dred_create_and_open_file(dred_context* pDred, const char* newFilePath)
{
    if (pDred == NULL) {
        return DR_FALSE;
    }

    if (newFilePath == NULL || newFilePath[0] == '\0') {
        return dred_open_new_text_file(pDred);
    } else {
        if (dr_file_exists(newFilePath)) {
            dred_errorf(pDred, "File already exists: %s", newFilePath);
            return DR_FALSE;
        }

        if (!dr_create_empty_file(newFilePath, DR_TRUE)) {
            dred_errorf(pDred, "Failed to create file: %s", newFilePath);
            return DR_FALSE;
        }

        return dred_open_file(pDred, newFilePath);
    }
}

dr_bool32 dred_open_new_text_file(dred_context* pDred)
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

    // Check for special built-in editors first.
    dred_editor* pEditor = NULL;
    if (pEditor == NULL && dred_is_control_type_of_type(editorType, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        pEditor = DRED_EDITOR(dred_text_editor_create(pDred, DTK_CONTROL(pTabGroup), sizeX, sizeY, filePathAbsolute));
    }
    if (pEditor == NULL && dred_is_control_type_of_type(editorType, DRED_CONTROL_TYPE_SETTINGS_EDITOR)) {
        pEditor = DRED_EDITOR(dred_settings_editor_create(pDred, DTK_CONTROL(pTabGroup), filePathAbsolute));
    }

    // Try loading from external packages if it's an unknown extension.
    if (pEditor == NULL) {
        DRED_FOREACH_PACKAGE(pDred, pPackage) {
            if (pPackage->cbs.editor.createEditor) {
                pEditor = pPackage->cbs.editor.createEditor(pPackage, pDred, DRED_CONTROL(pTabGroup), sizeX, sizeY, filePathAbsolute, editorType);
                if (pEditor != NULL) {
                    break;
                }
            }
        }
    }

    // Fall back to a text editor if it's an unknown extension.
    if (pEditor == NULL) {
        pEditor = DRED_EDITOR(dred_text_editor_create(pDred, DTK_CONTROL(pTabGroup), sizeX, sizeY, filePathAbsolute));
    }

    return pEditor;
}

void dred_delete_editor_by_type(dred_editor* pEditor)
{
    if (dred_control_is_of_type(DRED_CONTROL(pEditor), DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        dred_text_editor_delete(DRED_TEXT_EDITOR(pEditor));
        return;
    }
    if (dred_control_is_of_type(DRED_CONTROL(pEditor), DRED_CONTROL_TYPE_SETTINGS_EDITOR)) {
        dred_settings_editor_delete(DRED_SETTINGS_EDITOR(pEditor));
        return;
    }

    // If we get here it means it's not a known core editor, so check packages.
    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pEditor));
    DRED_FOREACH_PACKAGE(pDred, pPackage) {
        if (pPackage->cbs.editor.deleteEditor && pPackage->cbs.editor.deleteEditor(pPackage, pDred, pEditor)) {
            return;
        }
    }
}


dr_bool32 dred_are_any_open_files_modified(dred_context* pDred)
{
    if (pDred == NULL) {
        return DR_FALSE;
    }

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (pControl != NULL) {
                if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR) && dred_editor_is_modified(DRED_EDITOR(pControl))) {
                    return DR_TRUE;
                }
            }
        }
    }

    return DR_FALSE;
}


dr_bool32 dred_reload_focused_file(dred_context* pDred)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    return dred_editor_reload(pFocusedEditor);
}

dr_bool32 dred_check_if_focused_file_is_dirty_and_reload(dred_context* pDred)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
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
    ofn.hwndOwner = (HWND)pDred->pMainWindow->windowDTK.win32.hWnd;
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
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Open", GTK_WINDOW(pDred->pMainWindow->windowDTK.gtk.pWidget), GTK_FILE_CHOOSER_ACTION_OPEN,
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

dr_bool32 dred_show_save_file_dialog(dred_context* pDred, const char* currentFilePath, char* absolutePathOut, size_t absolutePathOutSize)
{
    if (pDred == NULL || absolutePathOut == NULL || absolutePathOutSize == 0) {
        return DR_FALSE;
    }

    absolutePathOut[0] = '\0';  // For safety, and also required for Win32.

#ifdef _WIN32
    if (currentFilePath != NULL) {
        strcpy_s(absolutePathOut, absolutePathOutSize, drpath_file_name(currentFilePath));
    }

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = (HWND)pDred->pMainWindow->windowDTK.win32.hWnd;
    ofn.lpstrFile = absolutePathOut;
    ofn.nMaxFile = (DWORD)absolutePathOutSize;
    ofn.lpstrFilter = "All\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetSaveFileNameA(&ofn)) {
        return DR_FALSE;
    }

    return DR_TRUE;
#else
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Save As", GTK_WINDOW(pDred->pMainWindow->windowDTK.gtk.pWidget), GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Save",   GTK_RESPONSE_ACCEPT,
        "_Cancel", GTK_RESPONSE_CANCEL, NULL);
    if (dialog == NULL) {
        return DR_FALSE;
    }

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    if (currentFilePath != NULL && currentFilePath[0] != '\0') {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), currentFilePath);
    }


    dr_bool32 result = DR_FALSE;
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

dtk_dialog_result dred_show_yesnocancel_dialog(dred_context* pDred, const char* message, const char* title)
{
    if (pDred == NULL) return 0;
    return dtk_message_box(DTK_WINDOW(pDred->pMainWindow), message, title, DTK_DIALOG_BUTTONS_YESNOCANCEL);

#if 0
    // TODO: Move this to the platform layer.
#ifdef _WIN32
    int result = MessageBoxA((HWND)pDred->pMainWindow->windowDTK.win32.hWnd, message, title, MB_YESNOCANCEL);
    switch (result)
    {
        case IDCANCEL: return DRED_MESSAGE_BOX_CANCEL;
        case IDYES:    return DRED_MESSAGE_BOX_YES;
        case IDNO:     return DRED_MESSAGE_BOX_NO;

        default: break;
    }

    return 0;
#else
    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(pDred->pMainWindow->windowDTK.gtk.pWidget), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s", message);
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
#endif
}

dr_bool32 dred_show_font_picker_dialog(dred_context* pDred, dtk_window* pOwnerWindow, const dred_font_desc* pDefaultFontDesc, dred_font_desc* pDescOut)
{
    if (pDred == NULL || pDescOut == NULL) {
        return DR_FALSE;
    }

    if (pOwnerWindow == NULL) {
        pOwnerWindow = DTK_WINDOW(pDred->pMainWindow);
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
        case dtk_font_weight_medium:      lf.lfWeight = FW_MEDIUM;     break;
        case dtk_font_weight_thin:        lf.lfWeight = FW_THIN;       break;
        case dtk_font_weight_extra_light: lf.lfWeight = FW_EXTRALIGHT; break;
        case dtk_font_weight_light:       lf.lfWeight = FW_LIGHT;      break;
        case dtk_font_weight_semi_bold:   lf.lfWeight = FW_SEMIBOLD;   break;
        case dtk_font_weight_bold:        lf.lfWeight = FW_BOLD;       break;
        case dtk_font_weight_extra_bold:  lf.lfWeight = FW_EXTRABOLD;  break;
        case dtk_font_weight_heavy:       lf.lfWeight = FW_HEAVY;      break;
        default: break;
        }

        if (pDefaultFontDesc->slant == dtk_font_slant_italic || pDefaultFontDesc->slant == dtk_font_slant_oblique) {
            lf.lfItalic = TRUE;
        }
    }

    CHOOSEFONTA cf;
    ZeroMemory(&cf, sizeof(cf));
    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = (HWND)pOwnerWindow->win32.hWnd;
    cf.hDC = GetDC((HWND)pOwnerWindow->win32.hWnd);
    cf.lpLogFont = &lf;
    cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_BOTH;

    if (!ChooseFontA(&cf)) {
        return DR_FALSE;
    }

    strcpy_s(pDescOut->family, sizeof(pDescOut->family), lf.lfFaceName);

    if (lf.lfHeight < 0) {
        pDescOut->size = (float)-lf.lfHeight;
    } else {
        pDescOut->size = (float)lf.lfHeight;
    }

    pDescOut->weight = dtk_font_weight_default;
    switch (lf.lfWeight)
    {
    case FW_MEDIUM:     pDescOut->weight = dtk_font_weight_medium;      break;
    case FW_THIN:       pDescOut->weight = dtk_font_weight_thin;        break;
    case FW_EXTRALIGHT: pDescOut->weight = dtk_font_weight_extra_light; break;
    case FW_LIGHT:      pDescOut->weight = dtk_font_weight_light;       break;
    case FW_SEMIBOLD:   pDescOut->weight = dtk_font_weight_semi_bold;   break;
    case FW_BOLD:       pDescOut->weight = dtk_font_weight_bold;        break;
    case FW_EXTRABOLD:  pDescOut->weight = dtk_font_weight_extra_bold;  break;
    case FW_HEAVY:      pDescOut->weight = dtk_font_weight_heavy;       break;
    default: break;
    }

    pDescOut->slant = dtk_font_slant_none;
    if (lf.lfItalic) {
        pDescOut->slant = dtk_font_slant_italic;
    }

    pDescOut->flags = 0;

    return DR_TRUE;
#endif

#ifdef DRED_GTK
    (void)pDefaultFontDesc;

    GtkWidget* dialog = gtk_font_chooser_dialog_new(NULL, GTK_WINDOW(pOwnerWindow->gtk.pWidget));
    if (dialog == NULL) {
        return DR_FALSE;
    }

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    gchar* pangoFontStr = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dialog));
    if (pangoFontStr == NULL) {
        gtk_widget_destroy(dialog);
        return DR_FALSE;
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

dtk_dialog_result dred_show_color_picker_dialog(dred_context* pDred, dtk_window* pOwnerWindow, dtk_color initialColor, dtk_color* pColorOut)
{
    if (pDred == NULL || pColorOut == NULL) {
        return 0;
    }

    if (pOwnerWindow == NULL) {
        pOwnerWindow = DTK_WINDOW(pDred->pMainWindow);
    }

    return dtk_show_color_picker_dialog(&pDred->tk, pOwnerWindow, initialColor, pColorOut);
}


typedef struct
{
    dred_context* pDred;
    drte_engine textEngine;
    drte_view* pTextView;
    dtk_surface paintSurface;
    dtk_font font;
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

    if (dtk_font_init(&pPrintData->pDred->tk, pFont->desc.family, pFont->desc.size*pPrintData->scaleY, pFont->desc.weight, pFont->desc.slant, 0, &pPrintData->font) != DTK_SUCCESS) {
        return;
    }

    dtk_font_metrics fontMetrics;
    dtk_font_get_metrics(&pPrintData->font, 1, &fontMetrics);

    drte_engine_register_style_token(&pPrintData->textEngine, (drte_style_token)&pPrintData->font, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));
    drte_engine_set_default_style(&pPrintData->textEngine, (drte_style_token)&pPrintData->font);


    // Should probably move this to somewhere more appropriate.
    drte_view_set_size(pPrintData->pTextView, pPrintData->pageSizeX, pPrintData->pageSizeY);
}

void dred__uninit_print_font(dred_print_data* pPrintData)
{
    dtk_font_uninit(&pPrintData->font);
}

void dred__on_paint_rect_for_printing(drte_engine* pTextEngine, drte_view* pView, drte_style_token styleToken, drte_rect rect, void* pPaintData)
{
    (void)pTextEngine;
    (void)pView;
    (void)styleToken;
    (void)rect;
    (void)pPaintData;
}

void dred__on_paint_text_for_printing(drte_engine* pTextEngine, drte_view* pView, drte_style_token styleTokenFG, drte_style_token styleTokenBG, const char* text, size_t textLength, float posX, float posY, void* pPaintData)
{
    (void)pTextEngine;
    (void)pView;
    (void)styleTokenFG;
    (void)styleTokenBG;

    dred_print_data* pPrintData = (dred_print_data*)pPaintData;
    assert(pPrintData != NULL);

    // Skip the line if it's partially visible - it'll be drawn on the next page.
    if (posY + drte_engine_get_line_height(pTextEngine) > drte_view_get_size_y(pView)) {
        return;
    }

    dtk_surface_draw_text(&pPrintData->paintSurface, &pPrintData->font, 1, text, (int)textLength, (dtk_int32)posX, (dtk_int32)posY, dtk_rgb(0, 0, 0), dtk_rgba(0, 0, 0, 0));
}

void dred__on_measure_string_for_printing(drte_engine* pTextEngine, drte_style_token styleToken, float scale, const char* text, size_t textLength, dtk_int32* pWidthOut, dtk_int32* pHeightOut)
{
    (void)pTextEngine;
    dtk_font_measure_string((dtk_font*)styleToken, scale, text, textLength, pWidthOut, pHeightOut);
}

void dred__on_get_cursor_position_from_point_for_printing(drte_engine* pTextEngine, drte_style_token styleToken, float scale, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut)
{
    (void)pTextEngine;
    dtk_font_get_text_cursor_position_from_point((dtk_font*)styleToken, scale, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosXOut, pCharacterIndexOut);
}

void dred__on_get_cursor_position_from_char_for_printing(drte_engine* pTextEngine, drte_style_token styleToken, float scale, const char* text, size_t characterIndex, float* pTextCursorPosXOut)
{
    (void)pTextEngine;
    dtk_font_get_text_cursor_position_from_char((dtk_font*)styleToken, scale, text, characterIndex, pTextCursorPosXOut);
}

void dred__print_page(dred_print_data* pPrintData, size_t iPage)
{
    // Scroll to the page.
    drte_view_set_inner_offset_y(pPrintData->pTextView, -(iPage * drte_engine_get_line_height(&pPrintData->textEngine) * drte_view_get_line_count_per_page(pPrintData->pTextView)));

    // Paint.
    drte_view_paint(pPrintData->pTextView, drte_make_rect(0, 0, drte_view_get_size_x(pPrintData->pTextView), drte_view_get_size_y(pPrintData->pTextView)), pPrintData);
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

    if (dtk_surface_init_transient_cairo(&pPrintData->pDred->tk, gtk_print_context_get_cairo_context(context), pPrintData->pageSizeX, pPrintData->pageSizeY, &pPrintData->paintSurface) != DTK_SUCCESS) {
        return;
    }

    dred__init_print_font(pPrintData);

    gtk_print_operation_set_n_pages(pPrint, drte_view_get_page_count(pPrintData->pTextView));
}

void dred_gtk__on_end_print(GtkPrintOperation *operation, GtkPrintContext *context, gpointer user_data)
{
    (void)operation;
    (void)context;

    dred_print_data* pPrintData = (dred_print_data*)user_data;
    assert(pPrintData != NULL);

    dred__uninit_print_font(pPrintData);
    dtk_surface_uninit(&pPrintData->paintSurface);
}

void dred_gtk__on_draw_page(GtkPrintOperation *pPrint, GtkPrintContext *context, gint page_nr, gpointer user_data)
{
    (void)pPrint;
    (void)context;

    dred__print_page((dred_print_data*)user_data, page_nr);
}
#endif

dr_bool32 dred_show_print_dialog(dred_context* pDred, dtk_window* pOwnerWindow, dred_print_info* pInfoOut)
{
    if (pDred == NULL || pInfoOut == NULL) {
        return DR_FALSE;
    }

    if (pOwnerWindow == NULL) {
        pOwnerWindow = DTK_WINDOW(pDred->pMainWindow);
    }

    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    // Just return DR_FALSE if the focused editor does not support printing.
    if (!dred_control_is_of_type(DRED_CONTROL(pFocusedEditor), DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        return DR_FALSE;   // Focused editor does not support printing.
    }


    dred_print_data printData;
    printData.pDred = pDred;


    // When printing a text editor we need to use a different text engine for layout because the dimensions are different
    // and we need to force word wrap.
    if (!drte_engine_init(&printData.textEngine, &printData)) {
        return DR_FALSE;
    }

    printData.pTextView = drte_view_create(&printData.textEngine);

    // Engine settings.
    drte_view_enable_word_wrap(printData.pTextView);
    drte_engine_set_on_paint_text(&printData.textEngine, dred__on_paint_text_for_printing);
    drte_engine_set_on_paint_rect(&printData.textEngine, dred__on_paint_rect_for_printing);
    printData.textEngine.onMeasureString = dred__on_measure_string_for_printing;
    printData.textEngine.onGetCursorPositionFromPoint = dred__on_get_cursor_position_from_point_for_printing;
    printData.textEngine.onGetCursorPositionFromChar = dred__on_get_cursor_position_from_char_for_printing;

    // Set the text.
    size_t textLength = dred_text_editor_get_text(DRED_TEXT_EDITOR(pFocusedEditor), NULL, 0);
    char* text = (char*)malloc(textLength + 1);
    if (text == NULL) {
        drte_engine_uninit(&printData.textEngine);
        return DR_FALSE;
    }

    dred_text_editor_get_text(DRED_TEXT_EDITOR(pFocusedEditor), text, textLength+1);
    drte_engine_set_text(&printData.textEngine, text);



#ifdef DRED_WIN32
    PRINTDLGA pd;
    ZeroMemory(&pd, sizeof(pd));
    pd.lStructSize = sizeof(pd);
    pd.hwndOwner = (HWND)pOwnerWindow->win32.hWnd;
    pd.Flags = PD_RETURNDC;
    if (!PrintDlgA(&pd)) {
        return DR_FALSE;
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

    if (dtk_surface_init_transient_HDC(&pDred->tk, hPrintDC, physicalWidth, physicalHeight, &printData.paintSurface) != DTK_SUCCESS) {
        return DR_FALSE;
    }

    dred__init_print_font(&printData);

    size_t pageCount = drte_view_get_page_count(printData.pTextView);


    DOCINFOA di;
    ZeroMemory(&di, sizeof(di));
    di.cbSize = sizeof(di);
    di.lpszDocName = dred_editor_get_file_path(dred_get_focused_editor(pDred));
    if (StartDocA(hPrintDC, &di) <= 0) {
        return DR_FALSE;
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
    dtk_surface_uninit(&printData.paintSurface);

    DeleteObject(hPrintDC);


    PDEVMODEA pDevMode = (PDEVMODEA)GlobalLock(pd.hDevMode);
    LPDEVNAMES pDevNames = (LPDEVNAMES)GlobalLock(pd.hDevNames); (void)pDevNames;

    strcpy_s(pInfoOut->printerName, sizeof(pInfoOut->printerName), (char*)pDevMode->dmDeviceName);
    pInfoOut->firstPage = pd.nFromPage;
    pInfoOut->lastPage  = pd.nToPage;
    pInfoOut->copies    = pd.nCopies;

    if (pd.hDevMode != NULL) GlobalFree(pd.hDevMode);
    if (pd.hDevNames != NULL) GlobalFree(pd.hDevNames);
    return DR_TRUE;
#endif

#ifdef DRED_GTK
    GtkPrintOperation* pPrint = gtk_print_operation_new();
    if (pPrint == NULL) {
        return DR_FALSE;
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

    GtkPrintOperationResult printResult = gtk_print_operation_run(pPrint, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW(pOwnerWindow->gtk.pWidget), NULL);
    if (printResult != GTK_PRINT_OPERATION_RESULT_APPLY) {
        g_object_unref(pPrint);
        g_object_unref(pSettings);
        return DR_FALSE;
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
    return DR_TRUE;
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

void dred_refresh_layout(dred_context* pDred)
{
    if (pDred == NULL) return;
    dtk_control_refresh_layout(DTK_CONTROL(pDred->pMainWindow));

    dred_settings_dialog_refresh_styling(pDred->pSettingsDialog);
    dred_about_dialog_refresh_layout(pDred->pAboutDialog);
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

    dred_capture_keyboard(pDred, DRED_CONTROL(pDred->pCmdBar));
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

        dred_capture_keyboard(pDred, DRED_CONTROL(pFocusedEditor));
    }
}


void dred_update_info_bar(dred_context* pDred, dred_control* pControl)
{
    if (pDred == NULL) {
        return;
    }

    if (dred_get_focused_editor(pDred) != DRED_EDITOR(pControl)) {
        return;
    }

    dred_cmdbar_update_info_bar(pDred->pCmdBar, pControl);
}


void dred_show_menu_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_set_config_variable(pDred, "show-menu-bar", "true");
}

void dred_hide_menu_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_set_config_variable(pDred, "show-menu-bar", "false");
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

    dred_set_config_variable(pDred, "show-tab-bar", "true");
}

void dred_hide_tabbars(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_set_config_variable(pDred, "show-tab-bar", "false");
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

    dred_set_config_variable(pDred, "texteditor-show-line-numbers", "true");
}

void dred_hide_line_numbers(dred_context* pDred)
{
    if (pDred == NULL || !pDred->config.textEditorShowLineNumbers) {
        return;
    }

    dred_set_config_variable(pDred, "texteditor-show-line-numbers", "false");
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

    dred_set_config_variable(pDred, "texteditor-enable-word-wrap", "true");
}

void dred_disable_word_wrap(dred_context* pDred)
{
    if (pDred == NULL || !pDred->config.textEditorEnableWordWrap) {
        return;
    }

    dred_set_config_variable(pDred, "texteditor-enable-word-wrap", "false");
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

    dred_control_show(DRED_CONTROL(pDred->pCmdBar));
    dred_update_main_window_layout(pDred);
}

void dred_hide_command_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_control_hide(DRED_CONTROL(pDred->pCmdBar));
    dred_update_main_window_layout(pDred);
}

void dred_enable_auto_hide_command_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_set_config_variable(pDred, "auto-hide-cmd-bar", "true");
}

void dred_disable_auto_hide_command_bar(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_set_config_variable(pDred, "auto-hide-cmd-bar", "false");
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
            if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                dred_text_editor_set_text_scale(DRED_TEXT_EDITOR(pControl), pDred->config.textEditorScale);
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


void dred_set_config_variable(dred_context* pDred, const char* name, const char* value)
{
    if (pDred == NULL || name == NULL) return;
    if (value == NULL) value = "";

    dred_config_set(&pDred->config, name, value);
}


dred_font* dred__load_system_font_ui(dred_context* pDred)
{
    dred_font_desc fontDesc;
    fontDesc.flags = 0;

#ifdef _WIN32
    strcpy_s(fontDesc.family, sizeof(fontDesc.family), "Segoe UI");
    fontDesc.size = 12;
    fontDesc.weight = dtk_font_weight_normal;
    fontDesc.slant = dtk_font_slant_none;
#else
    strcpy_s(fontDesc.family, sizeof(fontDesc.family), "sans");
    fontDesc.size = 13;
    fontDesc.weight = dtk_font_weight_normal;
    fontDesc.slant = dtk_font_slant_none;

    #if 1
    GSettings* settings = g_settings_new("org.gnome.desktop.interface");
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

    return dred_font_library_create_font(&pDred->fontLibrary, fontDesc.family, fontDesc.size, fontDesc.weight, fontDesc.slant, fontDesc.flags);
}

dred_font* dred__load_system_font_mono(dred_context* pDred)
{
    dred_font_desc fontDesc;
    fontDesc.flags = 0;

#ifdef _WIN32
    strcpy_s(fontDesc.family, sizeof(fontDesc.family), "Consolas");
    fontDesc.size = 13;
    fontDesc.weight = dtk_font_weight_normal;
    fontDesc.slant = dtk_font_slant_none;

    // Fall back to Courier New by default for XP.
    OSVERSIONINFOEXA version;
    ZeroMemory(&version, sizeof(version));
    version.dwOSVersionInfoSize = sizeof(version);
    version.dwMajorVersion = 5;
    if (VerifyVersionInfoA(&version, VER_MAJORVERSION, VerSetConditionMask(0, VER_MAJORVERSION, VER_LESS_EQUAL))) {
        strcpy_s(fontDesc.family, sizeof(fontDesc.family), "Courier New");
    }
#endif

#ifdef DRED_GTK
    strcpy_s(fontDesc.family, sizeof(fontDesc.family), "monospace");
    fontDesc.size = 13;
    fontDesc.weight = dtk_font_weight_normal;
    fontDesc.slant = dtk_font_slant_none;

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
    GSettings* settings = g_settings_new("org.gnome.desktop.interface");
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

    return dred_font_library_create_font(&pDred->fontLibrary, fontDesc.family, fontDesc.size, fontDesc.weight, fontDesc.slant, fontDesc.flags);
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
    fontDesc.weight = dtk_font_weight_normal;
    fontDesc.slant = dtk_font_slant_none;

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

    float size = (float)atof(token);
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


    return dred_font_library_create_font(&pDred->fontLibrary, fontDesc.family, fontDesc.size, fontDesc.weight, fontDesc.slant, fontDesc.flags);
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


dr_bool32 dred_add_favourite(dred_context* pDred, const char* absolutePath)
{
    if (pDred == NULL || absolutePath == NULL) {
        return DR_FALSE;
    }

    dred_config_push_favourite_file(&pDred->config, absolutePath);
    dred_refresh_favourite_files_menu(pDred);

    return DR_TRUE;
}

dr_bool32 dred_remove_favourite(dred_context* pDred, const char* absolutePath)
{
    if (pDred == NULL || absolutePath == NULL) {
        return DR_FALSE;
    }

    dred_config_remove_favourite_file(&pDred->config, absolutePath);
    dred_refresh_favourite_files_menu(pDred);

    return DR_TRUE;
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

        if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
            dred_window_set_menu(pDred->pMainWindow, &pDred->menus.text);
        } else {
            dred_window_set_menu(pDred->pMainWindow, &pDred->menus.nothingopen);
        }

        if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
            dred_editor* pEditor = DRED_EDITOR(pControl);
            if (pEditor->filePathAbsolute[0] == '\0') {
                dtk_menu_disable_item(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_ADDFAVOURITE);
                dtk_menu_disable_item(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_REMOVEFAVOURITE);
            } else {
                dtk_menu_enable_item(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_ADDFAVOURITE);
                dtk_menu_enable_item(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_REMOVEFAVOURITE);
            }
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
        dred_window_set_menu(pDred->pMainWindow, &pDred->menus.nothingopen);
        dred_window_set_title(pDred->pMainWindow, "dred");
        dred_update_info_bar(pDred, NULL);
    }
}

void dred_on_accelerator(dred_context* pDred, dtk_accelerator accelerator)
{
    // The accelerator should be tied to a shortcut. We need to find that shortcut and execute it's command.
    for (size_t iShortcut = 0; iShortcut < pDred->shortcutTable.count; ++iShortcut) {
        dred_shortcut shortcut = pDred->shortcutTable.pShortcuts[iShortcut];
        if (pDred->queuedAccelerator.key == 0) {
            if (dtk_accelerator_equal(shortcut.accelerators[0], accelerator)) {
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
                    pDred->queuedAccelerator = dtk_accelerator_none();
                    return;
                }

                dred_exec(pDred, cmd, NULL);
            }
        }
    }

    // Make sure any queued accelerator is cleared.
    pDred->queuedAccelerator = dtk_accelerator_none();
}

void dred_on_ipc_message(dred_context* pDred, unsigned int messageID, const void* pMessageData)
{
    switch (messageID)
    {
        case DRED_IPC_MESSAGE_ACTIVATE:
        {
            dred_window_bring_to_top(pDred->pMainWindow);
        } break;

        case DRED_IPC_MESSAGE_OPEN:
        {
            dred_open_file(pDred, (const char*)pMessageData);
        } break;

        default:
        {
            dred_warningf(pDred, "Received unknown IPC message: %d\n", messageID);
        } break;
    }
}


dred_context* dred_get_context_from_control(dtk_control* pControl)
{
    if (pControl == NULL) return NULL;
    return (dred_context*)pControl->pTK->pUserData;
}

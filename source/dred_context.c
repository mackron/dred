
void dred__update_main_tab_group_container_layout(dred_context* pDred, dred_tabgroup_container* pContainer, float parentWidth, float parentHeight)
{
    assert(pContainer != NULL);

    dred_control_set_size(pContainer, parentWidth, parentHeight - dred_control_get_height(pDred->pCmdBar));
}

void dred__update_cmdbar_layout(dred_context* pDred, dred_cmdbar* pCmdBar, float parentWidth, float parentHeight)
{
    assert(pCmdBar != NULL);

    dred_control_set_size(pCmdBar, parentWidth, dred_control_get_height(pCmdBar));
    dred_control_set_relative_position(pCmdBar, 0, parentHeight - dred_control_get_height(pCmdBar));
}

void dred__update_main_window_layout(dred_window* pWindow, float windowWidth, float windowHeight)
{
    dred_context* pDred = pWindow->pDred;
    assert(pDred != NULL);

    dred__update_main_tab_group_container_layout(pDred, pDred->pMainTabgroupContainer, windowWidth, windowHeight);
    dred__update_cmdbar_layout(pDred, pDred->pCmdBar, windowWidth, windowHeight);
}


void dred__refresh_editor_tab_text(dred_editor* pEditor, dred_tab* pTab)
{
    assert(pEditor != NULL);
    assert(pTab != NULL);

    char tabText[256];
    const char* filename = drpath_file_name(dred_editor_get_file_path(pEditor));
    const char* modified = "";
    const char* readonly = "";

    if (dred_editor_is_modified(pEditor)) {
        modified = "*";
    }
    if (dred_editor_is_read_only(pEditor)) {
        readonly = " [Read Only]";
    }

    snprintf(tabText, sizeof(tabText), "%s%s%s", filename, modified, readonly);
    dred_tab_set_text(pTab, tabText);
}

void dred__on_editor_modified(dred_editor* pEditor)
{
    dred_context* pDred = dred_control_get_context(pEditor);
    assert(pDred != NULL);

    dred_tab* pTab = dred_find_control_tab(pEditor);
    if (pTab == NULL) {
        return;
    }

    // We need to show a "*" at the end of the tab's text to indicate that it's modified.
    dred__refresh_editor_tab_text(pEditor, pTab);
}

void dred__on_editor_unmodified(dred_editor* pEditor)
{
    dred_context* pDred = dred_control_get_context(pEditor);
    assert(pDred != NULL);

    dred_tab* pTab = dred_find_control_tab(pEditor);
    if (pTab == NULL) {
        return;
    }

    // We need to hide the "*" at the end of the tab's text to indicate that it's unmodified.
    dred__refresh_editor_tab_text(pEditor, pTab);
}


void dred_window_cb__on_main_window_close(dred_window* pWindow)
{
    dred_platform_post_quit_message(0);
}

void dred_window_cb__on_main_window_size(drgui_element* pElement, float width, float height)
{
    (void)height;

    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow == NULL) {
        return;
    }

    dred__update_main_window_layout(pWindow, width, height);
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
    dred_context* pDred = (dred_context*)pUserData;
    assert(pDred != NULL);

    dred_warningf(pDred, "%s[%d] : %s", configPath, line, message);
}


bool dred_init(dred_context* pDred, dr_cmdline cmdline)
{
    // TODO: USE dred_error() AND FAMILY FOR PRINTING CRITICAL ERRORS INSTEAD OF printf()

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
        goto on_error;
    }


    // The GUI.
    pDred->pGUI = drgui_create_context_dr_2d(pDred->pDrawingContext);
    if (pDred->pGUI == NULL) {
        goto on_error;
    }

    // The GUI needs to be linked to the window system.
    dred_platform_bind_gui(pDred->pGUI);


    // The font library. This needs to be initialized before loading any fonts and configs.
    if (!dred_font_library_init(&pDred->fontLibrary, pDred)) {
        goto on_error;
    }

    // The default GUI font. This is based on the platform.
    // TODO: Put this in the platform layer.
    pDred->pGUIFont = dred_font_library_create_font(&pDred->fontLibrary, "Segoe UI", 12, drgui_font_weight_normal, drgui_font_slant_none, 0, 0);


    // Accelerator table needs to be initialized before the config, because the config can specify bindings.
    if (!dred_accelerator_table_init(&pDred->acceleratorTable)) {
        goto on_error;
    }

    dred_accelerator_table_bind(&pDred->acceleratorTable, 'A', DRED_KEY_STATE_CTRL_DOWN, "select-all");
    dred_accelerator_table_bind(&pDred->acceleratorTable, 'S', DRED_KEY_STATE_CTRL_DOWN, "save");



    // Config
    //
    // The config is loaded in 3 stages. The first initializes it to it's default values, the second loads the .dred file from the main
    // user directory and the 3rd loads the .dred file sitting in the working directory.
    dred_config_init(&pDred->config, pDred);

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
        goto on_error;
    }

    dred_window_set_size(pDred->pMainWindow, 1280, 720);

    pDred->pMainWindow->onClose = dred_window_cb__on_main_window_close;
    drgui_set_on_size(pDred->pMainWindow->pRootGUIElement, dred_window_cb__on_main_window_size);

    // Show the window as soon as possible to give it the illusion of loading quickly.
    dred_window_show(pDred->pMainWindow);

    // Ensure the accelerators are bound. This needs to be done after loading the initial configs.
    dred_window_bind_accelerators(pDred->pMainWindow, &pDred->acceleratorTable);



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

    drgui_capture_keyboard(pDred->pCmdBar);



    // TESTING
    //pDred->pEditor0 = dred_text_editor_create(pDred, NULL);
    //pDred->pEditor0Tab = dred_tabgroup_append_tab(pDred->pMainTabGroup, "Test Editor 0", pDred->pEditor0);
    //pDred->pEditor1 = dred_text_editor_create(pDred, NULL);
    //pDred->pEditor1Tab = dred_tabgroup_append_tab(pDred->pMainTabGroup, "Test Editor 1", pDred->pEditor1);

    //dred_open_file(pDred, ".dred");
    //dred_open_file(pDred, ".desktop");


    // Update the layout of the main window to ensure it's in the correct initial state.
    dred__update_main_window_layout(pDred->pMainWindow, 1280, 720);

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
    dred_accelerator_table_uninit(&pDred->acceleratorTable);
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

    return pControl;
}



const char* dred_get_editor_type_by_path(const char* filePath)
{
    if (drpath_extension_equal(filePath, "txt")) {
        return DRED_CONTROL_TYPE_TEXT_EDITOR;
    }

    return NULL;
}

dred_tab* dred_find_editor_tab_by_absolute_path(dred_context* pDred, const char* filePathAbsolute)
{
    // TODO: Iterate over every tab group.

    // For now there is only a single tab group...
    dred_tabgroup* pTabGroup = dred_get_focused_tabgroup(pDred);
    if (pTabGroup == NULL) {
        return NULL;
    }

    for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
        dred_control* pControl = dred_tab_get_control(pTab);
        if (pControl != NULL && dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
            if (drpath_equal(dred_editor_get_file_path(pControl), filePathAbsolute)) {
                return pTab;
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
    if (!dred_to_absolute_path(filePath, filePathAbsolute, sizeof(filePathAbsolute))) {
        dred_errorf(pDred, "File path is too long %s\n", filePath);
        return false;
    }

    // If the file is already open, activate it's tab.
    dred_tab* pExistingTab = dred_find_editor_tab_by_absolute_path(pDred, filePathAbsolute);
    if (pExistingTab != NULL) {
        dred_tabgroup_activate_tab(dred_tab_get_tabgroup(pExistingTab), pExistingTab);
        return false;
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
    dred_tab* pTab = dred_tabgroup_prepend_tab(pTabGroup, drpath_file_name(filePath), pEditor);
    if (pTab == NULL) {
        dred_delete_editor_by_type(pEditor);
        return false;
    }

    dred_tabgroup_activate_tab(pTabGroup, pTab);

    return true;
}

void dred_close_focused_file(dred_context* pDred)
{
    dred_close_tab(pDred, dred_get_focused_tab(pDred));
}

void dred_close_tab(dred_context* pDred, dred_tab* pTab)
{
    if (pDred == NULL || pTab == NULL) {
        return;
    }

    dred_editor* pEditor = dred_tab_get_control(pTab);

    // Delete the tab.
    dred_tabgroup_delete_tab(dred_tab_get_tabgroup(pTab), pTab);

    // Delete the editor.
    dred_delete_editor_by_type(pEditor);
}

void dred_close_all_tabs(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    // TODO: Do this for every tab group.

    dred_tabgroup* pTabGroup = dred_get_focused_tabgroup(pDred);
    if (pTabGroup == NULL) {
        return;
    }

    while (dred_tabgroup_first_tab(pTabGroup) != NULL) {
        dred_close_tab(pDred, dred_tabgroup_first_tab(pTabGroup));
    }
}


dred_tab* dred_find_control_tab(dred_control* pControl)
{
    dred_context* pDred = dred_control_get_context(pControl);
    if (pDred == NULL) {
        return NULL;
    }

    // TODO: Search every tab group.

    dred_tabgroup* pTabGroup = dred_get_focused_tabgroup(pDred);
    if (pTabGroup == NULL) {
        return NULL;
    }

    for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
        if (dred_tab_get_control(pTab) == pControl) {
            return pTab;
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
    if (dred_control_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_EDITOR)) {
        if (!dred_editor_save(pFocusedControl, newFilePath)) {
            return false;
        }

        dred__refresh_editor_tab_text(pFocusedControl, pFocusedTab);
        return true;
    }

    // Output.
    //
    // TODO: Add support for saving the output window.

    return false;
}

bool dred_save_focused_file_as(dred_context* pDred)
{
    if (pDred == NULL) {
        return false;
    }

    char newFilePath[DRED_MAX_PATH];
    if (!dred_show_save_file_dialog(pDred, newFilePath, sizeof(newFilePath))) {
        return false;
    }

    return dred_save_focused_file(pDred, newFilePath);
}


dred_editor* dred_create_editor_by_type(dred_context* pDred, dred_tabgroup* pTabGroup, const char* editorType, const char* filePathAbsolute)
{
    if (pDred == NULL) {
        return NULL;
    }

    // Note that the parent is always set to the tab group. This should be set at creation time to ensure it is
    // initialized properly. In particular, if we don't do this, the cursor will not be set to the correct value
    // because the sub-editor won't be attached to a window at creation time.

    dred_editor* pEditor = NULL;
    if (pEditor == NULL && dred_is_control_type_of_type(editorType, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        pEditor = dred_text_editor_create(pDred, pTabGroup, filePathAbsolute);
    }
    //if (pEditor == NULL && dred_is_control_type_of_type(editorType, DRED_CONTROL_TYPE_IMAGE_EDITOR)) {
    //    pEditor = dred_image_editor_create(pDred, pTabGroup, filePathAbsolute)
    //}

    // Fall back to a text editor if it's an unknown extension.
    if (pEditor == NULL) {
        pEditor = dred_text_editor_create(pDred, pTabGroup, filePathAbsolute);
    }

    return pEditor;
}

void dred_delete_editor_by_type(dred_editor* pEditor)
{
    if (dred_control_is_of_type(pEditor, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        dred_text_editor_delete(pEditor);
        return;
    }
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
#endif
}

bool dred_show_save_file_dialog(dred_context* pDred, char* absolutePathOut, size_t absolutePathOutSize)
{
    if (pDred == NULL || absolutePathOut == NULL || absolutePathOutSize == 0) {
        return false;
    }

#ifdef _WIN32
    absolutePathOut[0] = '\0';

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = pDred->pMainWindow->hWnd;
    ofn.lpstrFile = absolutePathOut;
    ofn.nMaxFile = absolutePathOutSize;
    //ofn.lpstrFilter = "All\0*.*\0";
    //ofn.nFilterIndex = 1;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetSaveFileNameA(&ofn)) {
        return false;
    }

    return absolutePathOut;
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
#endif
}


void dred_on_accelerator(dred_context* pDred, dred_window* pWindow, size_t acceleratorIndex)
{
    printf("Accelerator: %d\n", (int)acceleratorIndex);
}
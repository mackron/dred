
void dred__update_main_tab_group_container_layout(dred_context* pDred, dred_tabgroup_container* pContainer, float parentWidth, float parentHeight)
{
    if (pContainer == NULL) {
        return;
    }

    dred_control_set_size(pContainer, parentWidth, parentHeight - dred_control_get_height(pDred->pCmdBar));
}

void dred__update_cmdbar_layout(dred_context* pDred, dred_cmdbar* pCmdBar, float parentWidth, float parentHeight)
{
    (void)pDred;

    if (pCmdBar == NULL) {
        return;
    }

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
            if (dred_control_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_EDITOR)) {
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
    (void)pConfig;

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


    // Grab the system DPI scaling early so it can be used to correctly size GUI elements at initialization time.
    int baseDPI;
    int systemDPI;
    dred_get_base_dpi(&baseDPI, NULL);
    dred_get_system_dpi(&systemDPI, NULL);

    pDred->dpiScale = (float)systemDPI / (float)baseDPI;
    pDred->uiScale = pDred->dpiScale;
    pDred->textEditorScale = 1;


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
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_UNDO,       dred_shortcut_create_single(dred_accelerator_create('Z', DRED_KEY_STATE_CTRL_DOWN)), "undo");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_REDO,       dred_shortcut_create_single(dred_accelerator_create('Y', DRED_KEY_STATE_CTRL_DOWN)), "redo");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_CUT,        dred_shortcut_create_single(dred_accelerator_create('X', DRED_KEY_STATE_CTRL_DOWN)), "cut");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_COPY,       dred_shortcut_create_single(dred_accelerator_create('C', DRED_KEY_STATE_CTRL_DOWN)), "copy");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_PASTE,      dred_shortcut_create_single(dred_accelerator_create('V', DRED_KEY_STATE_CTRL_DOWN)), "paste");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_SELECT_ALL, dred_shortcut_create_single(dred_accelerator_create('A', DRED_KEY_STATE_CTRL_DOWN)), "select-all");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_GOTO,       dred_shortcut_create_single(dred_accelerator_create('G', DRED_KEY_STATE_CTRL_DOWN)), "cmdbar goto ");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_FIND,       dred_shortcut_create_single(dred_accelerator_create('F', DRED_KEY_STATE_CTRL_DOWN)), "cmdbar find-next ");
    dred_bind_shortcut(pDred, DRED_SHORTCUT_NAME_REPLACE,    dred_shortcut_create_single(dred_accelerator_create('F', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), "cmdbar replace-all ");


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

    // The UI scale will be known only after loading the configs.
    pDred->uiScale = pDred->dpiScale * pDred->config.uiScale;



    // The menu library. This must be initialized after the shortcut table and configs because it will need access to the initial shortcut bindings.
    if (!dred_menu_library_init(&pDred->menuLibrary, pDred)) {
        goto on_error;
    }



    // The main window.
    pDred->pMainWindow = dred_window_create(pDred);
    if (pDred->pMainWindow == NULL) {
        printf("Failed to create main window.");
        goto on_error;
    }

    pDred->pMainWindow->onClose = dred_window_cb__on_main_window_close;
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




    // Show the window last to ensure child GUI elements have been initialized and in a valid state. This should be done before
    // opening the files passed on the command line, however, because the window needs to be shown in order for it to receive
    // keyboard focus.
    dred_window_set_title(pDred->pMainWindow, "dred");
    dred_window_set_menu(pDred->pMainWindow, pDred->menuLibrary.pMenu_Default);
    dred_window_set_size(pDred->pMainWindow, (unsigned int)(1280*pDred->dpiScale), (unsigned int)(720*pDred->dpiScale));
    dred_window_show(pDred->pMainWindow);


    // Load initial files from the command line.
    dr_parse_cmdline(&pDred->cmdline, dred_parse_cmdline__startup_files, pDred);

    // If there were no files passed on the command line, start with an empty text file. We can know this by simply finding the
    // focused editor. If it's null, nothing is open.
    if (dred_get_focused_editor(pDred) == NULL) {
        dred_open_new_text_file(pDred);
    }

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

    // If there's any modified files we need to show a dialog box.
    if (dred_are_any_open_files_modified(pDred)) {
        unsigned int result = dred_show_yesnocancel_dialog(pDred, "You have unsaved changes. Save changes?", "Save changes?");
        if (result == DRED_MESSAGE_BOX_YES) {
            if (!dred_save_all_open_files_with_saveas(pDred)) {
                return;
            }
        } else if (result == DRED_MESSAGE_BOX_CANCEL) {
            return; // Cancel. Don't quit, just return.
        }
    }

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

    if (!dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
        return NULL;
    }

    return pControl;
}

drgui_element* dred_get_element_with_keyboard_capture(dred_context* pDred)
{
    if (pDred == NULL) {
        return NULL;
    }

    drgui_element* pElement = drgui_get_element_with_keyboard_capture(pDred->pGUI);
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
    if (drpath_extension_equal(filePath, "txt")) {
        return DRED_CONTROL_TYPE_TEXT_EDITOR;
    }

    return NULL;
}

dred_tab* dred_find_editor_tab_by_absolute_path(dred_context* pDred, const char* filePathAbsolute)
{
    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (pControl != NULL && dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
                if (drpath_equal(dred_editor_get_file_path(pControl), filePathAbsolute)) {
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
    if (dred_control_is_of_type(pEditor, DRED_CONTROL_TYPE_EDITOR)) {
        dred_delete_editor_by_type(pEditor);
    }
}

void dred_close_tab_with_confirmation(dred_context* pDred, dred_tab* pTab)
{
    dred_editor* pEditor = dred_tab_get_control(pTab);
    if (pEditor == NULL) {
        dred_close_tab(pDred, pTab);
        return;
    }

    if (!dred_control_is_of_type(pEditor, DRED_CONTROL_TYPE_EDITOR)) {
        dred_close_tab(pDred, pTab);
        return;
    }

    if (dred_editor_is_modified(pEditor)) {
        char msg[4096];

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
    if (dred_control_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_EDITOR)) {
        return dred__save_editor(pFocusedControl, newFilePath, pFocusedTab);
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
            if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
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
            if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
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


bool dred_are_any_open_files_modified(dred_context* pDred)
{
    if (pDred == NULL) {
        return false;
    }

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (pControl != NULL) {
                if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR) && dred_editor_is_modified(pControl)) {
                    return true;
                }
            }
        }
    }

    return false;
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
    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(pDred->pMainWindow->pGTKWindow), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, message);
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

void dred_show_about_dialog(dred_context* pDred)
{
    if (pDred == NULL) {
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

    dred_cmdbar_update_info_bar(pDred->pCmdBar, pControl);
}


void dred_show_line_numbers(dred_context* pDred)
{
    if (pDred == NULL || pDred->isShowingLineNumbers) {
        return;
    }

    pDred->isShowingLineNumbers = true;

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                dred_text_editor_show_line_numbers(pControl);
            }
        }
    }
}

void dred_hide_line_numbers(dred_context* pDred)
{
    if (pDred == NULL || !pDred->isShowingLineNumbers) {
        return;
    }

    pDred->isShowingLineNumbers = false;

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
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

    if (pDred->isShowingLineNumbers) {
        dred_hide_line_numbers(pDred);
    } else {
        dred_show_line_numbers(pDred);
    }
}


void dred_set_text_editor_scale(dred_context* pDred, float scale)
{
    if (pDred == NULL) {
        return;
    }

    pDred->textEditorScale = dr_clamp(scale, 0.1f, 4.0f);

    // Every open text editors needs to be updated.
    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_tabgroup_next_tabgroup(pTabGroup)) {
        for (dred_tab* pTab = dred_tabgroup_first_tab(pTabGroup); pTab != NULL; pTab = dred_tabgroup_next_tab(pTabGroup, pTab)) {
            dred_control* pControl = dred_tab_get_control(pTab);
            if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                dred_text_editor_set_text_scale(pControl, pDred->textEditorScale);
            }
        }
    }
}

float dred_get_text_editor_scale(dred_context* pDred)
{
    if (pDred == NULL) {
        return 0;
    }

    return pDred->textEditorScale;
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

        if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_EDITOR)) {
            if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                dred_window_set_menu(pDred->pMainWindow, pDred->menuLibrary.pMenu_TextEditor);
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
        dred_window_set_menu(pDred->pMainWindow, pDred->menuLibrary.pMenu_Default);
        dred_window_set_title(pDred->pMainWindow, "dred");
        dred_update_info_bar(pDred, NULL);
    }
}

void dred_on_accelerator(dred_context* pDred, dred_window* pWindow, size_t acceleratorIndex)
{
    (void)pWindow;

    // The accelerator should be tied to a shortcut. We need to find that shortcut and execute it's command.
    size_t shortcutIndex;
    if (!dred_shortcut_table_find(&pDred->shortcutTable, dred_shortcut_create_single(pDred->shortcutTable.acceleratorTable.pAccelerators[acceleratorIndex]), &shortcutIndex)) {
        return;
    }

    const char* cmd = dred_shortcut_table_get_command_string_by_index(&pDred->shortcutTable, shortcutIndex);
    if (cmd == NULL) {
        return;
    }

    dred_exec(pDred, cmd);
}

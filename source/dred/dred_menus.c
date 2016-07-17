
bool dred_menu_library_init(dred_menu_library* pLibrary, dred_context* pDred)
{
    if (pLibrary == NULL || pDred == NULL) {
        return false;
    }

    memset(pLibrary, 0, sizeof(*pLibrary));
    pLibrary->pDred = pDred;


    // Recent files
    pLibrary->pMenu_RecentFiles = dred_menu_create(pDred, dred_menu_type_popup);

    // Themes.
    pLibrary->pMenu_Themes = dred_menu_create(pDred, dred_menu_type_popup);

    // The default menu.
    pLibrary->pMenu_Default = dred_menu_create(pDred, dred_menu_type_menubar);
    if (pLibrary->pMenu_Default == NULL) {
        return false;
    }

    dred_menu* pFileMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu_Default, "&New", DRED_MENU_ITEM_ID_FILE_NEW, DRED_SHORTCUT_NAME_NEW);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu_Default, "&Open...", DRED_MENU_ITEM_ID_FILE_OPEN, DRED_SHORTCUT_NAME_OPEN);
    pLibrary->pOpenRecentItem_Default = dred_menu_item_create_and_append(pFileMenu_Default, "Open Recent", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pLibrary->pMenu_RecentFiles);
    dred_menu_item_create_and_append_separator(pFileMenu_Default);
    dred_menu_item_create_and_append(pFileMenu_Default, "E&xit", DRED_MENU_ITEM_ID_FILE_EXIT, "exit", dred_shortcut_create_single(dred_accelerator_create(DRGUI_F4, DRED_KEY_STATE_ALT_DOWN)), NULL);

    dred_menu* pViewMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append(pViewMenu_Default, "Toggle Tab Bars", DRED_MENU_ITEM_ID_VIEW_TAB_BARS, "toggle-tab-bar", dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append(pViewMenu_Default, "Toggle Command Bar Auto-Hide", DRED_MENU_ITEM_ID_VIEW_CMD_BAR, "toggle-auto-hide-cmdbar", dred_shortcut_none(), NULL);

    dred_menu* pSettingsMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    pLibrary->pThemesItem_Default = dred_menu_item_create_and_append(pSettingsMenu_Default, "Themes", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pLibrary->pMenu_Themes);
    dred_menu_item_create_and_append_separator(pSettingsMenu_Default);
    dred_menu_item_create_and_append(pSettingsMenu_Default, "&Settings...", DRED_MENU_ITEM_ID_SETTINGS_SETTINGS, "settings", dred_shortcut_none(), NULL);

    dred_menu* pHelpMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    //dred_menu_item_create_and_append(pHelpMenu_Default, "View &Help (Opens Web Browser)...", DRED_MENU_ITEM_ID_HELP_HELP, "help", dred_shortcut_create_single(dred_accelerator_create(DRGUI_F1, DRED_KEY_STATE_CTRL_DOWN), NULL);
    //dred_menu_item_create_and_append_separator(pHelpMenu_Default);
    dred_menu_item_create_and_append(pHelpMenu_Default, "&About...", DRED_MENU_ITEM_ID_HELP_ABOUT, "about", dred_shortcut_none(), NULL);

    dred_menu_item_create_and_append(pLibrary->pMenu_Default, "&File", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pFileMenu_Default);
    dred_menu_item_create_and_append(pLibrary->pMenu_Default, "&View", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pViewMenu_Default);
    dred_menu_item_create_and_append(pLibrary->pMenu_Default, "&Settings", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pSettingsMenu_Default);
    dred_menu_item_create_and_append(pLibrary->pMenu_Default, "&Help", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pHelpMenu_Default);




    // The text editor menu.
    pLibrary->pMenu_TextEditor = dred_menu_create(pDred, dred_menu_type_menubar);
    if (pLibrary->pMenu_TextEditor == NULL) {
        return false;
    }

    dred_menu* pFileMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&New", DRED_MENU_ITEM_ID_FILE_NEW, DRED_SHORTCUT_NAME_NEW);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&Open...", DRED_MENU_ITEM_ID_FILE_OPEN, DRED_SHORTCUT_NAME_OPEN);
    pLibrary->pOpenRecentItem_TextEditor = dred_menu_item_create_and_append(pFileMenu, "Open Recent", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pLibrary->pMenu_RecentFiles);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&Save", DRED_MENU_ITEM_ID_FILE_SAVE, DRED_SHORTCUT_NAME_SAVE);
    dred_menu_item_create_and_append(pFileMenu, "Save &As...", DRED_MENU_ITEM_ID_FILE_SAVE_AS,  "save-as",  dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "Save A&ll", DRED_MENU_ITEM_ID_FILE_SAVE_ALL,  DRED_SHORTCUT_NAME_SAVE_ALL);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&Close", DRED_MENU_ITEM_ID_FILE_CLOSE, DRED_SHORTCUT_NAME_CLOSE);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "Clos&e All", DRED_MENU_ITEM_ID_FILE_CLOSE_ALL, DRED_SHORTCUT_NAME_CLOSE_ALL);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "Print...", DRED_MENU_ITEM_ID_FILE_PRINT, DRED_SHORTCUT_NAME_PRINT);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append(pFileMenu, "E&xit", DRED_MENU_ITEM_ID_FILE_EXIT, "exit", dred_shortcut_create_single(dred_accelerator_create(DRGUI_F4, DRED_KEY_STATE_ALT_DOWN)), NULL);

    dred_menu* pEditMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Undo", DRED_MENU_ITEM_ID_EDIT_UNDO, DRED_SHORTCUT_NAME_UNDO);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Redo", DRED_MENU_ITEM_ID_EDIT_REDO, DRED_SHORTCUT_NAME_REDO);
    dred_menu_item_create_and_append_separator(pEditMenu);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "Cu&t", DRED_MENU_ITEM_ID_EDIT_CUT, DRED_SHORTCUT_NAME_CUT);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Copy", DRED_MENU_ITEM_ID_EDIT_COPY, DRED_SHORTCUT_NAME_COPY);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Paste", DRED_MENU_ITEM_ID_EDIT_PASTE, DRED_SHORTCUT_NAME_PASTE);
    dred_menu_item_create_and_append(pEditMenu, "&Delete", DRED_MENU_ITEM_ID_EDIT_DELETE, "delete", dred_shortcut_create_single(dred_accelerator_create(DRGUI_DELETE, 0)), NULL);
    dred_menu_item_create_and_append_separator(pEditMenu);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "Select &All", DRED_MENU_ITEM_ID_EDIT_SELECT_ALL, DRED_SHORTCUT_NAME_SELECT_ALL);

    dred_menu* pViewMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append(pViewMenu, "Toggle Tab Bars", DRED_MENU_ITEM_ID_VIEW_TAB_BARS, "toggle-tab-bar", dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append(pViewMenu, "Toggle Command Bar Auto-Hide", DRED_MENU_ITEM_ID_VIEW_CMD_BAR, "toggle-auto-hide-cmdbar", dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append_separator(pViewMenu);
    dred_menu_item_create_and_append(pViewMenu, "Toggle &Line Numbers", DRED_MENU_ITEM_ID_VIEW_LINE_NUMBERS, "toggle-line-numbers", dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append(pViewMenu, "Reset &Zoom", DRED_MENU_ITEM_ID_VIEW_RESET_ZOOM, "zoom 1", dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append_separator(pViewMenu);
    dred_menu_item_create_and_append(pViewMenu, "Toggle Word &Wrap", DRED_MENU_ITEM_ID_VIEW_WORD_WRAP, "toggle-word-wrap", dred_shortcut_none(), NULL);

    dred_menu* pFindMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append_with_shortcut(pFindMenu, "&Find...", DRED_MENU_ITEM_ID_FIND_FIND, DRED_SHORTCUT_NAME_FIND);
    dred_menu_item_create_and_append_with_shortcut(pFindMenu, "Find and &Replace...", DRED_MENU_ITEM_ID_FIND_REPLACE, DRED_SHORTCUT_NAME_REPLACE);
    dred_menu_item_create_and_append_separator(pFindMenu);
    dred_menu_item_create_and_append_with_shortcut(pFindMenu, "&Go To...", DRED_MENU_ITEM_ID_FIND_GOTO, DRED_SHORTCUT_NAME_GOTO);

    dred_menu* pSettingsMenu = dred_menu_create(pDred, dred_menu_type_popup);
    pLibrary->pThemesItem_Default = dred_menu_item_create_and_append(pSettingsMenu, "Themes", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pLibrary->pMenu_Themes);
    dred_menu_item_create_and_append_separator(pSettingsMenu);
    dred_menu_item_create_and_append(pSettingsMenu, "&Settings...", DRED_MENU_ITEM_ID_SETTINGS_SETTINGS, "settings", dred_shortcut_none(), NULL);

    dred_menu* pHelpMenu = dred_menu_create(pDred, dred_menu_type_popup);
    //dred_menu_item_create_and_append(pHelpMenu, "View &Help (Opens Web Browser)...", DRED_MENU_ITEM_ID_HELP_HELP, "help", dred_shortcut_create_single(dred_accelerator_create(DRGUI_F1, DRED_KEY_STATE_CTRL_DOWN), NULL);
    //dred_menu_item_create_and_append_separator(pHelpMenu);
    dred_menu_item_create_and_append(pHelpMenu, "&About...", DRED_MENU_ITEM_ID_HELP_ABOUT, "about", dred_shortcut_none(), NULL);

    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&File", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pFileMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&Edit", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pEditMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&View", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pViewMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "F&ind", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pFindMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&Settings", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pSettingsMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&Help", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pHelpMenu);


    // The text editor right-click popup menu.
    pLibrary->pPopupMenu_TextEditor = dred_menu_create(pDred, dred_menu_type_popup);
    if (pLibrary->pPopupMenu_TextEditor == NULL) {
        return false;
    }

    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Undo", DRED_MENU_ITEM_ID_EDIT_UNDO, DRED_SHORTCUT_NAME_UNDO);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Redo", DRED_MENU_ITEM_ID_EDIT_REDO, DRED_SHORTCUT_NAME_REDO);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_TextEditor);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "Cu&t", DRED_MENU_ITEM_ID_EDIT_CUT, DRED_SHORTCUT_NAME_CUT);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Copy", DRED_MENU_ITEM_ID_EDIT_COPY, DRED_SHORTCUT_NAME_COPY);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Paste", DRED_MENU_ITEM_ID_EDIT_PASTE, DRED_SHORTCUT_NAME_PASTE);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_TextEditor, "&Delete", DRED_MENU_ITEM_ID_EDIT_DELETE, "delete", dred_shortcut_create_single(dred_accelerator_create(DRGUI_DELETE, 0)), NULL);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_TextEditor);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "Select &All", DRED_MENU_ITEM_ID_EDIT_SELECT_ALL, DRED_SHORTCUT_NAME_SELECT_ALL);


    pLibrary->pPopupMenu_Tab = dred_menu_create(pDred, dred_menu_type_popup);
    if (pLibrary->pPopupMenu_Tab == NULL) {
        return false;
    }

    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "&Save", DRED_MENU_ITEM_ID_FILE_SAVE, DRED_SHORTCUT_NAME_SAVE);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_Tab, "Save &As...", DRED_MENU_ITEM_ID_FILE_SAVE_AS,  "save-as",  dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "Save A&ll", DRED_MENU_ITEM_ID_FILE_SAVE_ALL,  DRED_SHORTCUT_NAME_SAVE_ALL);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_Tab);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "&Close", DRED_MENU_ITEM_ID_FILE_CLOSE, DRED_SHORTCUT_NAME_CLOSE);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "Clos&e All", DRED_MENU_ITEM_ID_FILE_CLOSE_ALL, DRED_SHORTCUT_NAME_CLOSE_ALL);



    dred_menu_library_update_recent_files_menu(pLibrary);
    dred_menu_library_update_themes_menu(pLibrary);

    return true;
}

void dred_menu_library_uninit(dred_menu_library* pLibrary)
{
    if (pLibrary == NULL) {
        return;
    }

    // TODO: Clean up sub-menus. Consider making menus delete their menu items to keep things simple to manage.


    dred_menu_delete(pLibrary->pMenu_TextEditor);
    dred_menu_delete(pLibrary->pMenu_Default);
}

void dred_menu_library_update_recent_files_menu(dred_menu_library* pLibrary)
{
    if (pLibrary == NULL || pLibrary->pMenu_RecentFiles == NULL) {
        return;
    }

    pLibrary->recentFileCount = 0;
    dred_menu_delete_all_items(pLibrary->pMenu_RecentFiles);

    for (size_t iRecentFile = 0; iRecentFile < pLibrary->pDred->config.recentFileCount; ++iRecentFile) {
        char cmdStr[DRED_MAX_PATH];
        if (snprintf(cmdStr, sizeof(cmdStr), "open \"%s\"", pLibrary->pDred->config.recentFiles[iRecentFile]) < 0) {
            continue;   // Failed to make the command string. Probably because the buffer is too small.
        }

        dred_menu_item_create_and_append(pLibrary->pMenu_RecentFiles, pLibrary->pDred->config.recentFiles[iRecentFile], (uint16_t)(DRED_MENU_ITEM_ID_RECENT_FILE_0 + iRecentFile), cmdStr, dred_shortcut_none(), NULL);
        pLibrary->recentFileCount += 1;
    }


    // Enable or disable the relevant menu items depending on how many recent files we have.
    if (pLibrary->recentFileCount > 0) {
        dred_menu_item_enable(pLibrary->pOpenRecentItem_Default);
        dred_menu_item_enable(pLibrary->pOpenRecentItem_TextEditor);
    } else {
        dred_menu_item_disable(pLibrary->pOpenRecentItem_Default);
        dred_menu_item_disable(pLibrary->pOpenRecentItem_TextEditor);
    }
}

void dred_menu_library_update_themes_menu(dred_menu_library* pLibrary)
{
    if (pLibrary == NULL || pLibrary->pMenu_Themes == NULL) {
        return;
    }

    dred_menu_delete_all_items(pLibrary->pMenu_Themes);

    dred_menu_item_create_and_append(pLibrary->pMenu_Themes, "Dark (Default)", DRED_MENU_ITEM_ID_THEME_0 + 0, "load-config dark.dredtheme",  dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append(pLibrary->pMenu_Themes, "Light",          DRED_MENU_ITEM_ID_THEME_0 + 1, "load-config light.dredtheme", dred_shortcut_none(), NULL);
}
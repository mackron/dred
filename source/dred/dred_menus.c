// Copyright (C) 2016 David Reid. See included LICENSE file.

dr_bool32 dred_menu_library_init(dred_menu_library* pLibrary, dred_context* pDred)
{
    if (pLibrary == NULL || pDred == NULL) {
        return DR_FALSE;
    }

    memset(pLibrary, 0, sizeof(*pLibrary));
    pLibrary->pDred = pDred;


    // Recent files
    pLibrary->pMenu_RecentFiles = dred_menu_create(pDred, dred_menu_type_popup);

    // Favourite files.
    pLibrary->pMenu_FavouriteFiles = dred_menu_create(pDred, dred_menu_type_popup);

    // Themes.
    pLibrary->pMenu_Themes = dred_menu_create(pDred, dred_menu_type_popup);

    // The default menu.
    pLibrary->pMenu_Default = dred_menu_create(pDred, dred_menu_type_menubar);
    if (pLibrary->pMenu_Default == NULL) {
        return DR_FALSE;
    }

    dred_menu* pFileMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu_Default, "&New", DRED_MENU_ITEM_ID_FILE_NEW, DRED_SHORTCUT_NAME_NEW, 0);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu_Default, "&Open...", DRED_MENU_ITEM_ID_FILE_OPEN, DRED_SHORTCUT_NAME_OPEN, 0);
    pLibrary->pOpenRecentItem_Default = dred_menu_item_create_and_append(pFileMenu_Default, "Open Recent", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_RecentFiles);
    pLibrary->pOpenFavouriteItem_Default = dred_menu_item_create_and_append(pFileMenu_Default, "Open Favourite", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_FavouriteFiles);
    dred_menu_item_create_and_append_separator(pFileMenu_Default);
    dred_menu_item_create_and_append(pFileMenu_Default, "E&xit", DRED_MENU_ITEM_ID_FILE_EXIT, "exit", dred_shortcut_create_single(dred_accelerator_create(DRED_GUI_F4, DRED_KEY_STATE_ALT_DOWN)), 0, NULL);

    dred_menu* pViewMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append(pViewMenu_Default, "Toggle Tab Bars", DRED_MENU_ITEM_ID_VIEW_TAB_BARS, "toggle-tab-bar", dred_shortcut_none(), 0, NULL);
    dred_menu_item_create_and_append(pViewMenu_Default, "Toggle Command Bar Auto-Hide", DRED_MENU_ITEM_ID_VIEW_CMD_BAR, "toggle-auto-hide-cmdbar", dred_shortcut_none(), 0, NULL);

    dred_menu* pSettingsMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    pLibrary->pThemesItem_Default = dred_menu_item_create_and_append(pSettingsMenu_Default, "Themes", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_Themes);
    dred_menu_item_create_and_append_separator(pSettingsMenu_Default);
    dred_menu_item_create_and_append(pSettingsMenu_Default, "&Settings...", DRED_MENU_ITEM_ID_SETTINGS_SETTINGS, "settings", dred_shortcut_none(), 0, NULL);

    dred_menu* pHelpMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    //dred_menu_item_create_and_append(pHelpMenu_Default, "View &Help (Opens Web Browser)...", DRED_MENU_ITEM_ID_HELP_HELP, "help", dred_shortcut_create_single(dred_accelerator_create(DRED_GUI_F1, DRED_KEY_STATE_CTRL_DOWN), NULL);
    //dred_menu_item_create_and_append_separator(pHelpMenu_Default);
    dred_menu_item_create_and_append(pHelpMenu_Default, "&About...", DRED_MENU_ITEM_ID_HELP_ABOUT, "about", dred_shortcut_none(), 0, NULL);

    dred_menu_item_create_and_append(pLibrary->pMenu_Default, "&File", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pFileMenu_Default);
    dred_menu_item_create_and_append(pLibrary->pMenu_Default, "&View", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pViewMenu_Default);
    dred_menu_item_create_and_append(pLibrary->pMenu_Default, "&Settings", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pSettingsMenu_Default);
    dred_menu_item_create_and_append(pLibrary->pMenu_Default, "&Help", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pHelpMenu_Default);




    // The text editor menu.
    pLibrary->pMenu_TextEditor = dred_menu_create(pDred, dred_menu_type_menubar);
    if (pLibrary->pMenu_TextEditor == NULL) {
        return DR_FALSE;
    }

    dred_menu* pFileMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&New", DRED_MENU_ITEM_ID_FILE_NEW, DRED_SHORTCUT_NAME_NEW, 0);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&Open...", DRED_MENU_ITEM_ID_FILE_OPEN, DRED_SHORTCUT_NAME_OPEN, 0);
    pLibrary->pOpenRecentItem_TextEditor = dred_menu_item_create_and_append(pFileMenu, "Open Recent", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_RecentFiles);
    pLibrary->pOpenFavouriteItem_TextEditor = dred_menu_item_create_and_append(pFileMenu, "Open Favourite", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_FavouriteFiles);
    dred_menu_item_create_and_append_separator(pFileMenu);
    pLibrary->pMenuItem_AddToFavourites = dred_menu_item_create_and_append(pFileMenu, "Add to Favourites", DRED_MENU_ITEM_ID_ADD_TO_FAVOURITES, "add-favourite", dred_shortcut_none(), 0, NULL);
    pLibrary->pMenuItem_RemoveFromFavourites = dred_menu_item_create_and_append(pFileMenu, "Remove from Favourites", DRED_MENU_ITEM_ID_REMOVE_FROM_FAVOURITES, "remove-favourite", dred_shortcut_none(), 0, NULL);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&Save", DRED_MENU_ITEM_ID_FILE_SAVE, DRED_SHORTCUT_NAME_SAVE, 0);
    dred_menu_item_create_and_append(pFileMenu, "Save &As...", DRED_MENU_ITEM_ID_FILE_SAVE_AS,  "save-as",  dred_shortcut_none(), 0, NULL);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "Save A&ll", DRED_MENU_ITEM_ID_FILE_SAVE_ALL,  DRED_SHORTCUT_NAME_SAVE_ALL, 0);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&Close", DRED_MENU_ITEM_ID_FILE_CLOSE, DRED_SHORTCUT_NAME_CLOSE, 0);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "Clos&e All", DRED_MENU_ITEM_ID_FILE_CLOSE_ALL, DRED_SHORTCUT_NAME_CLOSE_ALL, 0);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "Print...", DRED_MENU_ITEM_ID_FILE_PRINT, DRED_SHORTCUT_NAME_PRINT, 0);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append(pFileMenu, "E&xit", DRED_MENU_ITEM_ID_FILE_EXIT, "exit", dred_shortcut_create_single(dred_accelerator_create(DRED_GUI_F4, DRED_KEY_STATE_ALT_DOWN)), 0, NULL);

    dred_menu* pEditMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Undo", DRED_MENU_ITEM_ID_EDIT_UNDO, DRED_SHORTCUT_NAME_UNDO, 0);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Redo", DRED_MENU_ITEM_ID_EDIT_REDO, DRED_SHORTCUT_NAME_REDO, 0);
    dred_menu_item_create_and_append_separator(pEditMenu);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "Cu&t", DRED_MENU_ITEM_ID_EDIT_CUT, DRED_SHORTCUT_NAME_CUT, 0);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Copy", DRED_MENU_ITEM_ID_EDIT_COPY, DRED_SHORTCUT_NAME_COPY, 0);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Paste", DRED_MENU_ITEM_ID_EDIT_PASTE, DRED_SHORTCUT_NAME_PASTE, 0);
    dred_menu_item_create_and_append(pEditMenu, "&Delete", DRED_MENU_ITEM_ID_EDIT_DELETE, "delete", dred_shortcut_create_single(dred_accelerator_create(DRED_GUI_DELETE, 0)), 0, NULL);
    dred_menu_item_create_and_append_separator(pEditMenu);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "Select &All", DRED_MENU_ITEM_ID_EDIT_SELECT_ALL, DRED_SHORTCUT_NAME_SELECT_ALL, 0);

    dred_menu* pViewMenu = dred_menu_create(pDred, dred_menu_type_popup);
    pLibrary->pMenuItem_ToggleTabBars = dred_menu_item_create_and_append(pViewMenu, "Show Tab Bars", DRED_MENU_ITEM_ID_VIEW_TAB_BARS, "toggle-tab-bar", dred_shortcut_none(),DRED_MENU_ITEM_CHECK, NULL);
    dred_menu_item_set_checked(pLibrary->pMenuItem_ToggleTabBars, pDred->config.showTabBar);
    pLibrary->pMenuItem_ToggleCmdBarAutoHide = dred_menu_item_create_and_append(pViewMenu, "Auto-Hide Command Bar", DRED_MENU_ITEM_ID_VIEW_CMD_BAR, "toggle-auto-hide-cmdbar", dred_shortcut_none(), DRED_MENU_ITEM_CHECK, NULL);
    dred_menu_item_set_checked(pLibrary->pMenuItem_ToggleCmdBarAutoHide, pDred->config.autoHideCmdBar);
    dred_menu_item_create_and_append_separator(pViewMenu);
    pLibrary->pMenuItem_ToggleLineNumbers = dred_menu_item_create_and_append(pViewMenu, "Show &Line Numbers", DRED_MENU_ITEM_ID_VIEW_LINE_NUMBERS, "toggle-line-numbers", dred_shortcut_none(), DRED_MENU_ITEM_CHECK, NULL);
    dred_menu_item_set_checked(pLibrary->pMenuItem_ToggleLineNumbers, pDred->config.textEditorShowLineNumbers);
    dred_menu_item_create_and_append(pViewMenu, "Reset &Zoom", DRED_MENU_ITEM_ID_VIEW_RESET_ZOOM, "zoom 1", dred_shortcut_none(), 0, NULL);
    dred_menu_item_create_and_append_separator(pViewMenu);
    pLibrary->pMenuItem_ToggleWordWrap = dred_menu_item_create_and_append(pViewMenu, "Word &Wrap", DRED_MENU_ITEM_ID_VIEW_WORD_WRAP, "toggle-word-wrap", dred_shortcut_none(), DRED_MENU_ITEM_CHECK, NULL);
    dred_menu_item_set_checked(pLibrary->pMenuItem_ToggleWordWrap, pDred->config.textEditorEnableWordWrap);

    dred_menu* pFindMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append_with_shortcut(pFindMenu, "&Find...", DRED_MENU_ITEM_ID_FIND_FIND, DRED_SHORTCUT_NAME_FIND, 0);
    dred_menu_item_create_and_append_with_shortcut(pFindMenu, "Find and &Replace...", DRED_MENU_ITEM_ID_FIND_REPLACE, DRED_SHORTCUT_NAME_REPLACE, 0);
    dred_menu_item_create_and_append_separator(pFindMenu);
    dred_menu_item_create_and_append_with_shortcut(pFindMenu, "&Go To...", DRED_MENU_ITEM_ID_FIND_GOTO, DRED_SHORTCUT_NAME_GOTO, 0);

    dred_menu* pSettingsMenu = dred_menu_create(pDred, dred_menu_type_popup);
    pLibrary->pThemesItem_Default = dred_menu_item_create_and_append(pSettingsMenu, "Themes", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_Themes);
    dred_menu_item_create_and_append_separator(pSettingsMenu);
    dred_menu_item_create_and_append(pSettingsMenu, "&Settings...", DRED_MENU_ITEM_ID_SETTINGS_SETTINGS, "settings", dred_shortcut_none(), 0, NULL);

    dred_menu* pHelpMenu = dred_menu_create(pDred, dred_menu_type_popup);
    //dred_menu_item_create_and_append(pHelpMenu, "View &Help (Opens Web Browser)...", DRED_MENU_ITEM_ID_HELP_HELP, "help", dred_shortcut_create_single(dred_accelerator_create(DRED_GUI_F1, DRED_KEY_STATE_CTRL_DOWN), NULL);
    //dred_menu_item_create_and_append_separator(pHelpMenu);
    dred_menu_item_create_and_append(pHelpMenu, "&About...", DRED_MENU_ITEM_ID_HELP_ABOUT, "about", dred_shortcut_none(), 0, NULL);

    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&File", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pFileMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&Edit", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pEditMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&View", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pViewMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "F&ind", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pFindMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&Settings", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pSettingsMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&Help", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pHelpMenu);


    // The text editor right-click popup menu.
    pLibrary->pPopupMenu_TextEditor = dred_menu_create(pDred, dred_menu_type_popup);
    if (pLibrary->pPopupMenu_TextEditor == NULL) {
        return DR_FALSE;
    }

    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Undo", DRED_MENU_ITEM_ID_EDIT_UNDO, DRED_SHORTCUT_NAME_UNDO, 0);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Redo", DRED_MENU_ITEM_ID_EDIT_REDO, DRED_SHORTCUT_NAME_REDO, 0);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_TextEditor);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "Cu&t", DRED_MENU_ITEM_ID_EDIT_CUT, DRED_SHORTCUT_NAME_CUT, 0);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Copy", DRED_MENU_ITEM_ID_EDIT_COPY, DRED_SHORTCUT_NAME_COPY, 0);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Paste", DRED_MENU_ITEM_ID_EDIT_PASTE, DRED_SHORTCUT_NAME_PASTE, 0);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_TextEditor, "&Delete", DRED_MENU_ITEM_ID_EDIT_DELETE, "delete", dred_shortcut_create_single(dred_accelerator_create(DRED_GUI_DELETE, 0)), 0, NULL);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_TextEditor);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "Select &All", DRED_MENU_ITEM_ID_EDIT_SELECT_ALL, DRED_SHORTCUT_NAME_SELECT_ALL, 0);


    pLibrary->pPopupMenu_Tab = dred_menu_create(pDred, dred_menu_type_popup);
    if (pLibrary->pPopupMenu_Tab == NULL) {
        return DR_FALSE;
    }

    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "&Save", DRED_MENU_ITEM_ID_FILE_SAVE, DRED_SHORTCUT_NAME_SAVE, 0);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_Tab, "Save &As...",       DRED_MENU_ITEM_ID_FILE_SAVE_AS, "save-as", dred_shortcut_none(), 0, NULL);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "Save A&ll", DRED_MENU_ITEM_ID_FILE_SAVE_ALL,  DRED_SHORTCUT_NAME_SAVE_ALL, 0);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_Tab);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "&Close", DRED_MENU_ITEM_ID_FILE_CLOSE, DRED_SHORTCUT_NAME_CLOSE, 0);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "Clos&e All", DRED_MENU_ITEM_ID_FILE_CLOSE_ALL, DRED_SHORTCUT_NAME_CLOSE_ALL, 0);
    //dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_Tab);
    //pLibrary->pMenuItem_AddToFavourites = dred_menu_item_create_and_append(pLibrary->pPopupMenu_Tab, "Add to Favourites", DRED_MENU_ITEM_ID_ADD_TO_FAVOURITES, "add-favourite", dred_shortcut_none(), 0, NULL);
    //pLibrary->pMenuItem_RemoveFromFavourites = dred_menu_item_create_and_append(pLibrary->pPopupMenu_Tab, "Remove from Favourites", DRED_MENU_ITEM_ID_REMOVE_FROM_FAVOURITES, "remove-favourite", dred_shortcut_none(), 0, NULL);



    dred_menu_library_update_recent_files_menu(pLibrary);
    dred_menu_library_update_favourite_files_menu(pLibrary);
    dred_menu_library_update_themes_menu(pLibrary);

    return DR_TRUE;
}

void dred_menu_library_uninit(dred_menu_library* pLibrary)
{
    if (pLibrary == NULL) {
        return;
    }

    dred_menu_delete_all_items(pLibrary->pMenu_RecentFiles);
    dred_menu_delete_all_items(pLibrary->pMenu_FavouriteFiles);

    //dred_menu_delete(pLibrary->pMenu_RecentFiles);
    dred_menu_delete(pLibrary->pPopupMenu_TextEditor);
    dred_menu_delete(pLibrary->pPopupMenu_Tab);
    dred_menu_delete(pLibrary->pMenu_TextEditor);
    //dred_menu_delete(pLibrary->pMenu_Default);
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

        dred_menu_item_create_and_append(pLibrary->pMenu_RecentFiles, pLibrary->pDred->config.recentFiles[iRecentFile], (uint16_t)(DRED_MENU_ITEM_ID_RECENT_FILE_0 + iRecentFile), cmdStr, dred_shortcut_none(), 0, NULL);
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

void dred_menu_library_update_favourite_files_menu(dred_menu_library* pLibrary)
{
    if (pLibrary == NULL || pLibrary->pMenu_FavouriteFiles == NULL) {
        return;
    }

    pLibrary->favouriteFileCount = 0;
    dred_menu_delete_all_items(pLibrary->pMenu_FavouriteFiles);

    for (size_t iFavouriteFile = 0; iFavouriteFile < pLibrary->pDred->config.favouriteFileCount; ++iFavouriteFile) {
        char cmdStr[DRED_MAX_PATH];
        if (snprintf(cmdStr, sizeof(cmdStr), "open \"%s\"", pLibrary->pDred->config.favouriteFiles[iFavouriteFile]) < 0) {
            continue;   // Failed to make the command string. Probably because the buffer is too small.
        }

        dred_menu_item_create_and_append(pLibrary->pMenu_FavouriteFiles, pLibrary->pDred->config.favouriteFiles[iFavouriteFile], (uint16_t)(DRED_MENU_ITEM_ID_FAVOURITE_FILE_0 + iFavouriteFile), cmdStr, dred_shortcut_none(), 0, NULL);
        pLibrary->favouriteFileCount += 1;
    }


    // Enable or disable the relevant menu items depending on how many recent files we have.
    if (pLibrary->favouriteFileCount > 0) {
        dred_menu_item_enable(pLibrary->pOpenFavouriteItem_Default);
        dred_menu_item_enable(pLibrary->pOpenFavouriteItem_TextEditor);
    } else {
        dred_menu_item_disable(pLibrary->pOpenFavouriteItem_Default);
        dred_menu_item_disable(pLibrary->pOpenFavouriteItem_TextEditor);
    }
}

void dred_menu_library_update_themes_menu(dred_menu_library* pLibrary)
{
    if (pLibrary == NULL || pLibrary->pMenu_Themes == NULL) {
        return;
    }

    dred_menu_delete_all_items(pLibrary->pMenu_Themes);

    dred_menu_item_create_and_append(pLibrary->pMenu_Themes, "Dark (Default)", DRED_MENU_ITEM_ID_THEME_0 + 0, "load-config dark.dredtheme",  dred_shortcut_none(), 0, NULL);
    dred_menu_item_create_and_append(pLibrary->pMenu_Themes, "Light",          DRED_MENU_ITEM_ID_THEME_0 + 1, "load-config light.dredtheme", dred_shortcut_none(), 0, NULL);
}
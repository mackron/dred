// Copyright (C) 2016 David Reid. See included LICENSE file.

#if 0
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
    dred_menu_item_create_and_append_with_shortcut(pFileMenu_Default, "&New", DRED_MENU_ITEM_ID_FILE_NEW, DRED_SHORTCUT_NAME_FILE_NEW, 0);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu_Default, "&Open...", DRED_MENU_ITEM_ID_FILE_OPEN, DRED_SHORTCUT_NAME_FILE_OPEN, 0);
    pLibrary->pOpenRecentItem_Default = dred_menu_item_create_and_append(pFileMenu_Default, "Open Recent", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_RecentFiles);
    pLibrary->pOpenFavouriteItem_Default = dred_menu_item_create_and_append(pFileMenu_Default, "Open Favourite", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_FavouriteFiles);
    dred_menu_item_create_and_append_separator(pFileMenu_Default);
    dred_menu_item_create_and_append(pFileMenu_Default, "E&xit", DRED_MENU_ITEM_ID_FILE_EXIT, "exit", dred_shortcut_create_single(dtk_accelerator_init(DRED_GUI_F4, DRED_KEY_STATE_ALT_DOWN, 0)), 0, NULL);

    dred_menu* pViewMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append(pViewMenu_Default, "Toggle Tab Bars", DRED_MENU_ITEM_ID_VIEW_TAB_BARS, "toggle-tab-bar", dred_shortcut_none(), 0, NULL);
    dred_menu_item_create_and_append(pViewMenu_Default, "Toggle Command Bar Auto-Hide", DRED_MENU_ITEM_ID_VIEW_CMD_BAR, "toggle-auto-hide-cmdbar", dred_shortcut_none(), 0, NULL);

    dred_menu* pSettingsMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    pLibrary->pThemesItem_Default = dred_menu_item_create_and_append(pSettingsMenu_Default, "Themes", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_Themes);
    dred_menu_item_create_and_append_separator(pSettingsMenu_Default);
    dred_menu_item_create_and_append(pSettingsMenu_Default, "&Settings...", DRED_MENU_ITEM_ID_SETTINGS_SETTINGS, "settings", dred_shortcut_none(), 0, NULL);

    dred_menu* pHelpMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    //dred_menu_item_create_and_append(pHelpMenu_Default, "View &Help (Opens Web Browser)...", DRED_MENU_ITEM_ID_HELP_HELP, "help", dred_shortcut_create_single(dtk_accelerator_init(DRED_GUI_F1, DRED_KEY_STATE_CTRL_DOWN), NULL);
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
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&New", DRED_MENU_ITEM_ID_FILE_NEW, DRED_SHORTCUT_NAME_FILE_NEW, 0);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&Open...", DRED_MENU_ITEM_ID_FILE_OPEN, DRED_SHORTCUT_NAME_FILE_OPEN, 0);
    pLibrary->pOpenRecentItem_TextEditor = dred_menu_item_create_and_append(pFileMenu, "Open Recent", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_RecentFiles);
    pLibrary->pOpenFavouriteItem_TextEditor = dred_menu_item_create_and_append(pFileMenu, "Open Favourite", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_FavouriteFiles);
    dred_menu_item_create_and_append_separator(pFileMenu);
    pLibrary->pMenuItem_AddToFavourites = dred_menu_item_create_and_append(pFileMenu, "Add to Favourites", DRED_MENU_ITEM_ID_ADD_TO_FAVOURITES, "add-favourite", dred_shortcut_none(), 0, NULL);
    pLibrary->pMenuItem_RemoveFromFavourites = dred_menu_item_create_and_append(pFileMenu, "Remove from Favourites", DRED_MENU_ITEM_ID_REMOVE_FROM_FAVOURITES, "remove-favourite", dred_shortcut_none(), 0, NULL);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&Save", DRED_MENU_ITEM_ID_FILE_SAVE, DRED_SHORTCUT_NAME_FILE_SAVE, 0);
    dred_menu_item_create_and_append(pFileMenu, "Save &As...", DRED_MENU_ITEM_ID_FILE_SAVE_AS,  "save-as",  dred_shortcut_none(), 0, NULL);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "Save A&ll", DRED_MENU_ITEM_ID_FILE_SAVE_ALL,  DRED_SHORTCUT_NAME_FILE_SAVE_ALL, 0);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&Close", DRED_MENU_ITEM_ID_FILE_CLOSE, DRED_SHORTCUT_NAME_FILE_CLOSE, 0);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "Clos&e All", DRED_MENU_ITEM_ID_FILE_CLOSE_ALL, DRED_SHORTCUT_NAME_FILE_CLOSE_ALL, 0);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append_with_shortcut(pFileMenu, "Print...", DRED_MENU_ITEM_ID_FILE_PRINT, DRED_SHORTCUT_NAME_FILE_PRINT, 0);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append(pFileMenu, "E&xit", DRED_MENU_ITEM_ID_FILE_EXIT, "exit", dred_shortcut_create_single(dtk_accelerator_init(DRED_GUI_F4, DRED_KEY_STATE_ALT_DOWN, 0)), 0, NULL);

    dred_menu* pEditMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Undo", DRED_MENU_ITEM_ID_EDIT_UNDO, DRED_SHORTCUT_NAME_EDIT_UNDO, 0);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Redo", DRED_MENU_ITEM_ID_EDIT_REDO, DRED_SHORTCUT_NAME_EDIT_REDO, 0);
    dred_menu_item_create_and_append_separator(pEditMenu);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "Cu&t", DRED_MENU_ITEM_ID_EDIT_CUT, DRED_SHORTCUT_NAME_EDIT_CUT, 0);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Copy", DRED_MENU_ITEM_ID_EDIT_COPY, DRED_SHORTCUT_NAME_EDIT_COPY, 0);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "&Paste", DRED_MENU_ITEM_ID_EDIT_PASTE, DRED_SHORTCUT_NAME_EDIT_PASTE, 0);
    dred_menu_item_create_and_append(pEditMenu, "&Delete", DRED_MENU_ITEM_ID_EDIT_DELETE, "delete", dred_shortcut_create_single(dtk_accelerator_init(DRED_GUI_DELETE, 0, 0)), 0, NULL);
    dred_menu_item_create_and_append_separator(pEditMenu);
    dred_menu_item_create_and_append_with_shortcut(pEditMenu, "Select &All", DRED_MENU_ITEM_ID_EDIT_SELECT_ALL, DRED_SHORTCUT_NAME_EDIT_SELECT_ALL, 0);

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
    dred_menu_item_create_and_append_with_shortcut(pFindMenu, "&Find...", DRED_MENU_ITEM_ID_FIND_FIND, DRED_SHORTCUT_NAME_FIND_FIND, 0);
    dred_menu_item_create_and_append_with_shortcut(pFindMenu, "Find and &Replace...", DRED_MENU_ITEM_ID_FIND_REPLACE, DRED_SHORTCUT_NAME_FIND_REPLACE, 0);
    dred_menu_item_create_and_append_separator(pFindMenu);
    dred_menu_item_create_and_append_with_shortcut(pFindMenu, "&Go To...", DRED_MENU_ITEM_ID_FIND_GOTO, DRED_SHORTCUT_NAME_FIND_GOTO, 0);

    dred_menu* pSettingsMenu = dred_menu_create(pDred, dred_menu_type_popup);
    pLibrary->pThemesItem_Default = dred_menu_item_create_and_append(pSettingsMenu, "Themes", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), 0, pLibrary->pMenu_Themes);
    dred_menu_item_create_and_append_separator(pSettingsMenu);
    dred_menu_item_create_and_append(pSettingsMenu, "&Settings...", DRED_MENU_ITEM_ID_SETTINGS_SETTINGS, "settings", dred_shortcut_none(), 0, NULL);

    dred_menu* pHelpMenu = dred_menu_create(pDred, dred_menu_type_popup);
    //dred_menu_item_create_and_append(pHelpMenu, "View &Help (Opens Web Browser)...", DRED_MENU_ITEM_ID_HELP_HELP, "help", dred_shortcut_create_single(dtk_accelerator_init(DRED_GUI_F1, DRED_KEY_STATE_CTRL_DOWN), NULL);
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

    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Undo", DRED_MENU_ITEM_ID_EDIT_UNDO, DRED_SHORTCUT_NAME_EDIT_UNDO, 0);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Redo", DRED_MENU_ITEM_ID_EDIT_REDO, DRED_SHORTCUT_NAME_EDIT_REDO, 0);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_TextEditor);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "Cu&t", DRED_MENU_ITEM_ID_EDIT_CUT, DRED_SHORTCUT_NAME_EDIT_CUT, 0);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Copy", DRED_MENU_ITEM_ID_EDIT_COPY, DRED_SHORTCUT_NAME_EDIT_COPY, 0);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "&Paste", DRED_MENU_ITEM_ID_EDIT_PASTE, DRED_SHORTCUT_NAME_EDIT_PASTE, 0);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_TextEditor, "&Delete", DRED_MENU_ITEM_ID_EDIT_DELETE, "delete", dred_shortcut_create_single(dtk_accelerator_init(DRED_GUI_DELETE, 0, 0)), 0, NULL);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_TextEditor);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_TextEditor, "Select &All", DRED_MENU_ITEM_ID_EDIT_SELECT_ALL, DRED_SHORTCUT_NAME_EDIT_SELECT_ALL, 0);


    pLibrary->pPopupMenu_Tab = dred_menu_create(pDred, dred_menu_type_popup);
    if (pLibrary->pPopupMenu_Tab == NULL) {
        return DR_FALSE;
    }

    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "&Save", DRED_MENU_ITEM_ID_FILE_SAVE, DRED_SHORTCUT_NAME_FILE_SAVE, 0);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_Tab, "Save &As...",       DRED_MENU_ITEM_ID_FILE_SAVE_AS, "save-as", dred_shortcut_none(), 0, NULL);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "Save A&ll", DRED_MENU_ITEM_ID_FILE_SAVE_ALL,  DRED_SHORTCUT_NAME_FILE_SAVE_ALL, 0);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_Tab);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "&Close", DRED_MENU_ITEM_ID_FILE_CLOSE, DRED_SHORTCUT_NAME_FILE_CLOSE, 0);
    dred_menu_item_create_and_append_with_shortcut(pLibrary->pPopupMenu_Tab, "Clos&e All", DRED_MENU_ITEM_ID_FILE_CLOSE_ALL, DRED_SHORTCUT_NAME_FILE_CLOSE_ALL, 0);
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
#endif


void dred_refresh_recent_files_menu(dred_context* pDred)
{
    // To refresh the list, we just overwrite any existing items, and then remove the excess, if any.
    dtk_uint32 newCount = pDred->config.recentFileCount;
    dtk_uint32 oldCount;
    dtk_menu_get_item_count(&pDred->menus.recentFiles, &oldCount);

    // Replace existing items.
    for (dtk_uint32 iItem = 0; iItem < newCount && iItem < oldCount; ++iItem) {
        char cmdStr[DRED_MAX_PATH];
        if (snprintf(cmdStr, sizeof(cmdStr), "open \"%s\"", pDred->config.recentFiles[iItem]) < 0) {
            continue;   // Failed to make the command string. Probably because the buffer is too small.
        }

        dtk_uint32 itemID = DRED_MENU_ITEM_ID_RECENT_FILE_0 + iItem;

        dtk_menu_set_item_text(&pDred->menus.recentFiles, dtk_menu_find_item_by_id(&pDred->menus.recentFiles, itemID), pDred->config.recentFiles[iItem]);
        dred_menu_item_table_bind(&pDred->menuItemTable, itemID, cmdStr, NULL);
    }

    // Either insert or remove extra items.
    if (newCount > oldCount) {
        // Insert new items.
        for (dtk_uint32 iItem = oldCount; iItem < newCount; ++iItem) {
            char cmdStr[DRED_MAX_PATH];
            if (snprintf(cmdStr, sizeof(cmdStr), "open \"%s\"", pDred->config.recentFiles[iItem]) < 0) {
                continue;   // Failed to make the command string. Probably because the buffer is too small.
            }

            dtk_uint32 itemID = DRED_MENU_ITEM_ID_RECENT_FILE_0 + iItem;

            // Create the menu item.
            dtk_menu_item_info info;
            memset(&info, 0, sizeof(info));
            info.id = itemID;
            info.text = pDred->config.recentFiles[iItem];
            dtk_menu_append_item(&pDred->menus.recentFiles, &info);

            // Bind the menu item to a command.
            dred_menu_item_table_bind(&pDred->menuItemTable, itemID, cmdStr, NULL);
        }
    } else {
        // Remove excess.
        for (dtk_uint32 iItem = newCount; iItem < oldCount; ++iItem) {
            dtk_menu_remove_item(&pDred->menus.recentFiles, iItem);
            dred_menu_item_table_unbind(&pDred->menuItemTable, DRED_MENU_ITEM_ID_RECENT_FILE_0 + iItem);
        }
    }

    // Enable or disable the relevant menu items depending on how many recent files we have.
    if (pDred->config.recentFileCount > 0) {
        dtk_menu_enable_item(&pDred->menus.textFile, dtk_menu_find_item_by_id(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_OPEN_RECENT));
        dtk_menu_enable_item(&pDred->menus.nothingopenFile, dtk_menu_find_item_by_id(&pDred->menus.nothingopenFile, DRED_MENU_ITEM_ID_NOTHINGOPEN_FILE_OPEN_RECENT));
    } else {
        dtk_menu_disable_item(&pDred->menus.textFile, dtk_menu_find_item_by_id(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_OPEN_RECENT));
        dtk_menu_disable_item(&pDred->menus.nothingopenFile, dtk_menu_find_item_by_id(&pDred->menus.nothingopenFile, DRED_MENU_ITEM_ID_NOTHINGOPEN_FILE_OPEN_RECENT));
    }
}

void dred_refresh_favourite_files_menu(dred_context* pDred)
{
    // To refresh the list, we just overwrite any existing items, and then remove the excess, if any.
    dtk_uint32 newCount = pDred->config.favouriteFileCount;
    dtk_uint32 oldCount;
    dtk_menu_get_item_count(&pDred->menus.favouriteFiles, &oldCount);

    // Replace existing items.
    for (dtk_uint32 iItem = 0; iItem < newCount && iItem < oldCount; ++iItem) {
        char cmdStr[DRED_MAX_PATH];
        if (snprintf(cmdStr, sizeof(cmdStr), "open \"%s\"", pDred->config.favouriteFiles[iItem]) < 0) {
            continue;   // Failed to make the command string. Probably because the buffer is too small.
        }

        dtk_uint32 itemID = DRED_MENU_ITEM_ID_FAVOURITE_FILE_0 + iItem;

        dtk_menu_set_item_text(&pDred->menus.recentFiles, dtk_menu_find_item_by_id(&pDred->menus.recentFiles, itemID), pDred->config.recentFiles[iItem]);
        dred_menu_item_table_bind(&pDred->menuItemTable, itemID, cmdStr, NULL);
    }

    // Either insert or remove extra items.
    if (newCount > oldCount) {
        // Insert new items.
        for (dtk_uint32 iItem = oldCount; iItem < newCount; ++iItem) {
            char cmdStr[DRED_MAX_PATH];
            if (snprintf(cmdStr, sizeof(cmdStr), "open \"%s\"", pDred->config.favouriteFiles[iItem]) < 0) {
                continue;   // Failed to make the command string. Probably because the buffer is too small.
            }

            dtk_uint32 itemID = DRED_MENU_ITEM_ID_FAVOURITE_FILE_0 + iItem;

            // Create the menu item.
            dtk_menu_item_info info;
            memset(&info, 0, sizeof(info));
            info.id = itemID;
            info.text = pDred->config.favouriteFiles[iItem];
            dtk_menu_append_item(&pDred->menus.favouriteFiles, &info);

            // Bind the menu item to a command.
            dred_menu_item_table_bind(&pDred->menuItemTable, itemID, cmdStr, NULL);
        }
    } else {
        // Remove excess.
        for (dtk_uint32 iItem = newCount; iItem < oldCount; ++iItem) {
            dtk_menu_remove_item(&pDred->menus.favouriteFiles, iItem);
            dred_menu_item_table_unbind(&pDred->menuItemTable, DRED_MENU_ITEM_ID_FAVOURITE_FILE_0 + iItem);
        }
    }

    // Enable or disable the relevant menu items depending on how many recent files we have.
    if (pDred->config.favouriteFileCount > 0) {
        dtk_menu_enable_item(&pDred->menus.textFile, dtk_menu_find_item_by_id(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_OPEN_FAVOURITE));
        dtk_menu_enable_item(&pDred->menus.nothingopenFile, dtk_menu_find_item_by_id(&pDred->menus.nothingopenFile, DRED_MENU_ITEM_ID_NOTHINGOPEN_FILE_OPEN_FAVOURITE));
    } else {
        dtk_menu_disable_item(&pDred->menus.textFile, dtk_menu_find_item_by_id(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_OPEN_FAVOURITE));
        dtk_menu_disable_item(&pDred->menus.nothingopenFile, dtk_menu_find_item_by_id(&pDred->menus.nothingopenFile, DRED_MENU_ITEM_ID_NOTHINGOPEN_FILE_OPEN_FAVOURITE));
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

dr_bool32 dred_is_stock_menu_item_id(dtk_uint32 id)
{
    if (id < DRED_STOCK_MENU_ITEM_ID_COUNT) {
        return DR_TRUE;
    }

    return DR_FALSE;
}

dred_result dred_menu_item_table_init(dred_context* pDred, dred_menu_item_table* pTable)
{
    if (pTable == NULL) return DRED_INVALID_ARGS;
    dtk_zero_object(pTable);

    if (pDred == NULL) return DRED_INVALID_ARGS;
    pTable->pDred = pDred;

    // The capacity should always be set to _at least_ the stock menu item count. The reason for this is that we use an optimized
    // constant-time lookup for stock menu items.
    pTable->capacity = DRED_STOCK_MENU_ITEM_COUNT;  
    if (pTable->capacity != 0) {
        pTable->pItems = (dred_menu_item_data*)malloc(pTable->capacity * sizeof(*pTable->pItems));
        if (pTable->pItems == NULL) {
            return DRED_OUT_OF_MEMORY;
        }
    }

    // Stock menu items need to be bound. This is an optimized binding with the assumption that the internal buffer has already
    // been allocated with enough space.
    dred_menu_item_table__bind_stock_items__autogenerated(pDred, pTable);
    pTable->count = DRED_STOCK_MENU_ITEM_COUNT;

    return DTK_SUCCESS;
}

dred_result dred_menu_item_table_uninit(dred_menu_item_table* pTable)
{
    if (pTable == NULL) return DRED_INVALID_ARGS;

    free(pTable->pItems);
    return DRED_SUCCESS;
}

dred_result dred_menu_item_table_bind(dred_menu_item_table* pTable, dtk_uint32 id, const char* commandStr, const char* shortcutName)
{
    if (pTable == NULL) return DRED_INVALID_ARGS;

    size_t index;
    if (!dred_menu_item_table_find(pTable, id, &index)) {
        // The item does not exist. Append.
        if (pTable->count == pTable->capacity) {
            size_t newCapacity = (pTable->capacity == 0) ? 64 : (pTable->capacity * 2);
            dred_menu_item_data* pNewItems = (dred_menu_item_data*)realloc(pTable->pItems, newCapacity * sizeof(*pNewItems));
            if (pNewItems == NULL) {
                return DRED_OUT_OF_MEMORY;
            }

            pTable->pItems = pNewItems;
            pTable->capacity = newCapacity;
        }

        assert(pTable->count < pTable->capacity);

        index = pTable->count;
        pTable->pItems[index].id = id;
        pTable->count += 1;
    }

    pTable->pItems[index].commandStrOffset = dred_string_pool_find_or_add(&pTable->pDred->stringPool, commandStr);
    pTable->pItems[index].shortcutStrOffset = dred_string_pool_find_or_add(&pTable->pDred->stringPool, shortcutName);

    return DRED_SUCCESS;
}

dred_result dred_menu_item_table_unbind(dred_menu_item_table* pTable, dtk_uint32 id)
{
    // We don't _actually_ unbind stock menu items because we want to use an optimized constant-time lookup for those ones
    // since we know that their range and that they're contiguous. We just return successful and pretend it was unbound.
    if (dred_is_stock_menu_item_id(id)) {
        pTable->pItems[id].commandStrOffset = 0;
        pTable->pItems[id].shortcutStrOffset = 0;
        return DRED_SUCCESS;
    }

    size_t index;
    if (dred_menu_item_table_find(pTable, id, &index)) {
        assert(index >= DRED_STOCK_MENU_ITEM_ID_COUNT);
        
        for (size_t i = index; i < pTable->count-1; ++i) {
            pTable->pItems[i] = pTable->pItems[i+1];
        }
        pTable->count -= 1;
    }

    return DRED_ERROR;  // Not found.
}

const char* dred_menu_item_table_get_command(dred_menu_item_table* pTable, dtk_uint32 id)
{
    size_t index;
    if (!dred_menu_item_table_find(pTable, id, &index)) {
        return NULL;
    }

    return dred_string_pool_cstr(&pTable->pDred->stringPool, pTable->pItems[index].commandStrOffset);
}

const char* dred_menu_item_table_get_shortcut(dred_menu_item_table* pTable, dtk_uint32 id)
{
    size_t index;
    if (!dred_menu_item_table_find(pTable, id, &index)) {
        return NULL;
    }

    return dred_string_pool_cstr(&pTable->pDred->stringPool, pTable->pItems[index].shortcutStrOffset);
}

dr_bool32 dred_menu_item_table_find(dred_menu_item_table* pTable, dtk_uint32 id, size_t* pIndex)
{
    if (pIndex) *pIndex = (size_t)-1;
    if (pTable == NULL) return DR_FALSE;

    if (dred_is_stock_menu_item_id(id)) {
        if (pIndex) *pIndex = id;
        return DR_TRUE;
    }

    for (size_t i = DRED_STOCK_MENU_ITEM_COUNT; i < pTable->count; ++i) {
        if (pTable->pItems[i].id == id) {
            if (pIndex) *pIndex = i;
            return DR_TRUE;
        }
    }

    return DR_FALSE;
}
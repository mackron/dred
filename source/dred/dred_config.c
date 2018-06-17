// Copyright (C) 2018 David Reid. See included LICENSE file.

dtk_bool32 dred_config_init(dred_config* pConfig, dred_context* pDred)
{
    if (pConfig == NULL || pDred == NULL) {
        return DTK_FALSE;
    }

    if (!dred_alias_map_init(&pConfig->aliasMap)) {
        return DTK_FALSE;
    }

    pConfig->pDred = pDred;
    pConfig->pSystemFontUI = dred_parse_and_load_font(pDred, "system-font-ui");
    pConfig->pSystemFontMono = dred_parse_and_load_font(pDred, "system-font-mono");

    dred_config_init_variables__autogenerated(pConfig);
    return DTK_TRUE;
}

void dred_config_uninit(dred_config* pConfig)
{
    if (pConfig == NULL) {
        return;
    }

    for (size_t i = 0; i < pConfig->recentFileCount; ++i) {
        dtk_free_string(pConfig->recentFiles[i]);
    }

    for (size_t i = 0; i < pConfig->favouriteFileCount; ++i) {
        dtk_free_string(pConfig->favouriteFiles[i]);
    }

    for (size_t i = 0; i < pConfig->recentCommandsCount; ++i) {
        dtk_free_string(pConfig->recentCommands[i]);
    }

    dred_config_uninit_variables__autogenerated(pConfig);
    dred_alias_map_uninit(&pConfig->aliasMap);
}


typedef struct
{
    dred_config* pConfig;
    const char* filePath;
    dred_config_on_error_proc onError;
    void* pUserData;
    dred_file file;
} dred_config_load_file__data;

size_t dred_config_load_file__on_read(void* pUserData, void* pDataOut, size_t bytesToRead)
{
    dred_config_load_file__data* pData = (dred_config_load_file__data*)pUserData;
    assert(pData != NULL);
    assert(pData->file != NULL);

    size_t bytesRead;
    if (!dred_file_read(pData->file, pDataOut, bytesToRead, &bytesRead)) {
        return 0;
    }

    return bytesRead;
}

void dred_config_load_file__on_pair(void* pUserData, const char* key, const char* value)
{
    dred_config_load_file__data* pData = (dred_config_load_file__data*)pUserData;
    assert(pData != NULL);

    if (strcmp(key, "include") == 0) {
        char fileAbsolutePath[DRED_MAX_PATH];
        if (!dred_to_absolute_path(pData->filePath, fileAbsolutePath, sizeof(fileAbsolutePath))) {
            return;
        }
        if (dtk_path_remove_file_name_in_place(fileAbsolutePath) == 0) {
            return;
        }

        char includeRelativePath[DRED_MAX_PATH];
        if (dtk_next_token(value, includeRelativePath, sizeof(includeRelativePath)) == NULL) {
            return;
        }

        char includeAbsolutePath[DRED_MAX_PATH];
        if (dtk_path_append_and_clean(includeAbsolutePath, sizeof(includeAbsolutePath), fileAbsolutePath, includeRelativePath) == 0) {
            return;
        }

        dred_load_config(pData->pConfig->pDred, includeAbsolutePath);
        return;
    }

    if (strcmp(key, "exec") == 0) {
        dred_exec(pData->pConfig->pDred, dtk_first_non_whitespace(value), NULL);
        return;
    }

    if (strcmp(key, "bind") == 0) {
        dred_command__bind(pData->pConfig->pDred, value);
        return;
    }

    if (strcmp(key, "recent-file") == 0) {
        char filePath[DRED_MAX_PATH];
        if (dtk_next_token(value, filePath, sizeof(filePath)) != NULL) {
            if (dtk_file_exists(filePath)) {
                dred_config_push_recent_file(pData->pConfig, filePath);
            }
        }

        return;
    }

    if (strcmp(key, "favourite-file") == 0) {
        char filePath[DRED_MAX_PATH];
        if (dtk_next_token(value, filePath, sizeof(filePath)) != NULL) {
            dred_config_push_favourite_file(pData->pConfig, filePath);
        }

        return;
    }

    if (strcmp(key, "recent-cmd") == 0) {
        char cmd[DRED_MAX_PATH];
        if (dtk_next_token(value, cmd, sizeof(cmd)) != NULL) {
            dred_config_push_recent_cmd(pData->pConfig, cmd);
        }

        return;
    }

    if (strcmp(key, "alias") == 0) {
        char aliasName[256];
        value = dtk_next_token(value, aliasName, sizeof(aliasName));
        if (value != NULL) {
            char aliasValue[DRED_MAX_PATH];
            if (dtk_next_token(value, aliasValue, sizeof(aliasValue)) != NULL) {
                dred_alias_map_add(&pData->pConfig->aliasMap, aliasName, aliasValue);
            }
        }

        return;
    }


    dred_config_set__autogenerated(pData->pConfig, key, value);
}

void dred_config_load_file__on_error(void* pUserData, const char* message, unsigned int line)
{
    dred_config_load_file__data* pData = (dred_config_load_file__data*)pUserData;
    assert(pData != NULL);

    if (pData->onError) {
        pData->onError(pData->pConfig, pData->filePath, message, line, pData->pUserData);
    }
}

dtk_bool32 dred_config_load_file(dred_config* pConfig, const char* filePath, dred_config_on_error_proc onError, void* pUserData)
{
    if (pConfig == NULL) {
        return DTK_FALSE;
    }

    dred_file file = dred_file_open(filePath, DRED_FILE_OPEN_MODE_READ);
    if (file == NULL) {
        return DTK_FALSE;
    }

    // Loading a config file can result in a lot of style changes, especially when loading theme files. To speed this up we
    // need to batch painting operations.
    //if (pConfig->pDred->pMainWindow != NULL) {
    //    dred_control_begin_dirty(pConfig->pDred->pMainWindow->pRootGUIControl);
    //}

    dred_config_load_file__data data;
    data.pConfig = pConfig;
    data.filePath = filePath;
    data.onError = onError;
    data.pUserData = pUserData;
    data.file = file;
    dtk_parse_key_value_pairs(dred_config_load_file__on_read, dred_config_load_file__on_pair, dred_config_load_file__on_error, &data);

    //if (pConfig->pDred->pMainWindow != NULL) {
    //    dred_control_end_dirty(pConfig->pDred->pMainWindow->pRootGUIControl);
    //}

    dred_file_close(file);
    return DTK_TRUE;
}

void dred_config_set(dred_config* pConfig, const char* name, const char* value)
{
    if (pConfig == NULL) {
        return;
    }

    dred_config_set__autogenerated(pConfig, name, value);
}

void dred_config_set_default(dred_config * pConfig, const char * name)
{
    if (pConfig == NULL) {
        return;
    }

    dred_config_set_default__autogenerated(pConfig, name);
}

void dred_config_push_recent_file(dred_config* pConfig, const char* fileAbsolutePath)
{
    if (pConfig == NULL) {
        return;
    }

    // If the path already exists, just move it to the end.
    for (size_t i = 0; i < pConfig->recentFileCount; ++i) {
        if (dtk_path_equal(fileAbsolutePath, pConfig->recentFiles[i])) {
            char* existingPath = pConfig->recentFiles[i];
            for (size_t j = i; j > 0; --j) {
                pConfig->recentFiles[j] = pConfig->recentFiles[j-1];
            }

            pConfig->recentFiles[0] = existingPath;
            return;
        }
    }


    // We need to drop the oldest file if we've run out of room.
    if (pConfig->recentFileCount == DRED_MAX_RECENT_FILES) {
        dtk_free_string(pConfig->recentFiles[pConfig->recentFileCount-1]);
        pConfig->recentFileCount -= 1;
    }

    assert(pConfig->recentFileCount < DRED_MAX_RECENT_FILES);

    // The most recent file is at position 0. Existing items need to be moved up a slot.
    for (size_t i = pConfig->recentFileCount; i > 0; --i) {
        pConfig->recentFiles[i] = pConfig->recentFiles[i-1];
    }

    // The path needs to be cleaned for aesthetic just to make it look nicer in the Recent Files menu.
    char filePathClean[DRED_MAX_PATH];
    dtk_path_clean(filePathClean, sizeof(filePathClean), fileAbsolutePath);

    pConfig->recentFiles[0] = dtk_make_string(filePathClean);
    pConfig->recentFileCount += 1;
}

void dred_config_clear_recent_files(dred_config* pConfig)
{
    pConfig->recentFileCount = 0;
}

void dred_config_push_favourite_file(dred_config* pConfig, const char* fileAbsolutePath)
{
    if (pConfig == NULL) {
        return;
    }

    // If the path already exists, just move it to the end.
    for (size_t i = 0; i < pConfig->favouriteFileCount; ++i) {
        if (dtk_path_equal(fileAbsolutePath, pConfig->favouriteFiles[i])) {
            char* existingPath = pConfig->favouriteFiles[i];
            for (size_t j = i; j < pConfig->favouriteFileCount-1; ++j) {
                pConfig->favouriteFiles[j] = pConfig->favouriteFiles[j+1];
            }

            pConfig->favouriteFiles[pConfig->favouriteFileCount-1] = existingPath;
            return;
        }
    }

    // Make room if there's none available.
    if (pConfig->favouriteFileCount == pConfig->favouriteFileBufferSize) {
        size_t newBufferSize = (pConfig->favouriteFileBufferSize == 0) ? 1 : pConfig->favouriteFileBufferSize*2;
        char** pNewBuffer = (char**)realloc(pConfig->favouriteFiles, sizeof(*pConfig->favouriteFiles) * newBufferSize);
        if (pNewBuffer == NULL) {
            return; // Probably out of memory.
        }

        pConfig->favouriteFiles = pNewBuffer;
        pConfig->favouriteFileBufferSize = newBufferSize;
    }

    // The path needs to be cleaned for aesthetic just to make it look nicer in the Recent Files menu.
    char filePathClean[DRED_MAX_PATH];
    dtk_path_clean(filePathClean, sizeof(filePathClean), fileAbsolutePath);

    pConfig->favouriteFiles[pConfig->favouriteFileCount] = dtk_make_string(filePathClean);
    pConfig->favouriteFileCount += 1;
}

void dred_config_remove_favourite_file_by_index(dred_config* pConfig, size_t index)
{
    if (pConfig == NULL || pConfig->favouriteFileCount == 0 || index >= pConfig->favouriteFileCount) return;

    // Just move everything down a slot.
    for (size_t i = index; i < pConfig->favouriteFileCount-1; ++i) {
        pConfig->favouriteFiles[i] = pConfig->favouriteFiles[i+1];
    }

    pConfig->favouriteFileCount -= 1;
}

void dred_config_remove_favourite_file(dred_config* pConfig, const char* fileAbsolutePath)
{
    if (pConfig == NULL || fileAbsolutePath == NULL) return;

    for (size_t i = 0; i < pConfig->favouriteFileCount; ++i) {
        if (strcmp(pConfig->favouriteFiles[i], fileAbsolutePath) == 0) {
            dred_config_remove_favourite_file_by_index(pConfig, i);
            return;
        }
    }
}

void dred_config_push_recent_cmd(dred_config* pConfig, const char* cmd)
{
    // We need to drop the oldest command if we've run out of room.
    if (pConfig->recentCommandsCount == DRED_MAX_RECENT_FILES) {
        dtk_free_string(pConfig->recentCommands[pConfig->recentCommandsCount-1]);
        pConfig->recentCommandsCount -= 1;
    }

    assert(pConfig->recentCommandsCount < DRED_MAX_RECENT_FILES);

    // The most recent file is at position 0. Existing items need to be moved up a slot.
    for (size_t i = pConfig->recentCommandsCount; i > 0; --i) {
        pConfig->recentCommands[i] = pConfig->recentCommands[i-1];
    }

    pConfig->recentCommands[0] = dtk_make_string(cmd);
    pConfig->recentCommandsCount += 1;
}



// Set handlers.

void dred_config_on_set__ui_scale(dred_context* pDred)
{
    dred_refresh_layout(pDred);
}

void dred_config_on_set__ui_font(dred_context* pDred)
{
    // Everything that uses the UI font needs to be updated.
    dred_cmdbar_refresh_styling(&pDred->cmdBar);
    dred_settings_dialog_refresh_styling(pDred->pSettingsDialog);

    // The UI font may have resulted in the main window's layout becoming invalid.
    dred_update_main_window_layout(pDred);
}

void dred_config_on_set__show_tab_bar(dred_context* pDred)
{
    if (pDred->config.showTabBar) {
        for (dtk_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
            dtk_tabgroup_show_tabbar(pTabGroup);
        }
    } else {
        for (dtk_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
            dtk_tabgroup_hide_tabbar(pTabGroup);
        }
    }

    dtk_menu_set_item_checked_by_id(&pDred->menus.textView, DRED_MENU_ITEM_ID_TEXT_VIEW_TABBARS, pDred->config.showTabBar);
    dtk_menu_set_item_checked_by_id(&pDred->menus.nothingopenView, DRED_MENU_ITEM_ID_NOTHINGOPEN_VIEW_TABBARS, pDred->config.showTabBar);
}

void dred_config_on_set__show_menu_bar(dred_context* pDred)
{
    if (pDred->config.showMenuBar) {
        dred_show_main_menu(pDred);
    } else {
        dred_hide_main_menu(pDred);
    }
}

void dred_config_on_set__auto_hide_cmd_bar(dred_context* pDred)
{
    if (pDred->config.autoHideCmdBar) {
        if (!dred_cmdbar_has_keyboard_focus(&pDred->cmdBar)) {
            dred_hide_command_bar(pDred);
        }
    } else {
        dred_show_command_bar(pDred);
    }

    dtk_menu_set_item_checked_by_id(&pDred->menus.textView, DRED_MENU_ITEM_ID_TEXT_VIEW_CMDBAR, pDred->config.autoHideCmdBar);
    dtk_menu_set_item_checked_by_id(&pDred->menus.nothingopenView, DRED_MENU_ITEM_ID_NOTHINGOPEN_VIEW_CMDBAR, pDred->config.autoHideCmdBar);
}

void dred_config_on_set__enable_auto_reload(dred_context* pDred)
{
    (void)pDred;
}


void dred_config_on_set__cmdbar_bg_color(dred_context* pDred)
{
    dred_cmdbar_refresh_styling(&pDred->cmdBar);
}

void dred_config_on_set__cmdbar_bg_color_active(dred_context* pDred)
{
    dred_cmdbar_refresh_styling(&pDred->cmdBar);
}

void dred_config_on_set__cmdbar_tb_font(dred_context* pDred)
{
    dred_cmdbar_refresh_styling(&pDred->cmdBar);
    dred_update_main_window_layout(pDred);
}

void dred_config_on_set__cmdbar_text_color(dred_context* pDred)
{
    dred_cmdbar_refresh_styling(&pDred->cmdBar);
}

void dred_config_on_set__cmdbar_text_color_active(dred_context* pDred)
{
    dred_cmdbar_refresh_styling(&pDred->cmdBar);
}

void dred_config_on_set__cmdbar_padding_horz(dred_context* pDred)
{
    dred_cmdbar_refresh_styling(&pDred->cmdBar);
    dred_update_main_window_layout(pDred);
}

void dred_config_on_set__cmdbar_padding_vert(dred_context* pDred)
{
    dred_cmdbar_refresh_styling(&pDred->cmdBar);
    dred_update_main_window_layout(pDred);
}

void dred_config_on_set__cmdbar_popup_bg_color_active(dred_context* pDred)
{
    dred_cmdbar_popup_refresh_styling(&pDred->cmdbarPopup);
}

void dred_config_on_set__cmdbar_popup_font(dred_context* pDred)
{
    dred_cmdbar_popup_refresh_styling(&pDred->cmdbarPopup);
}

void dred_config_on_set__cmdbar_popup_border_width(dred_context* pDred)
{
    dred_cmdbar_popup_refresh_styling(&pDred->cmdbarPopup);
}

void dred_config_on_set__cmdbar_popup_padding(dred_context* pDred)
{
    dred_cmdbar_popup_refresh_styling(&pDred->cmdbarPopup);
}


void dred_config_on_set__tabgroup_generic_refresh(dred_context* pDred)
{
    for (dtk_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
        dred_refresh_styling_tabgroup(pDred, pTabGroup);
    }
}


void dred_config_on_set__texteditor_generic_refresh(dred_context* pDred)
{
    for (dtk_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
        for (dtk_uint32 iTab = 0; iTab < dtk_tabgroup_get_tab_count(pTabGroup); ++iTab) {
            dtk_control* pPage = dtk_tabgroup_get_tab_page(pTabGroup, iTab);
            if (pPage != NULL && pPage->type == DTK_CONTROL_TYPE_DRED) {
                dred_control* pDredControl = DRED_CONTROL(pPage);
                if (dred_control_is_of_type(pDredControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                    dred_text_editor_refresh_styling(DRED_TEXT_EDITOR(pDredControl));
                }
            }
        }
    }

    dtk_menu_set_item_checked_by_id(&pDred->menus.textView, DRED_MENU_ITEM_ID_TEXT_VIEW_LINENUMBERS, pDred->config.textEditorShowLineNumbers);
}

void dred_config_on_set__texteditor_word_wrap(dred_context* pDred)
{
    for (dtk_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
        for (dtk_uint32 iTab = 0; iTab < dtk_tabgroup_get_tab_count(pTabGroup); ++iTab) {
            dtk_control* pPage = dtk_tabgroup_get_tab_page(pTabGroup, iTab);
            if (pPage != NULL && pPage->type == DTK_CONTROL_TYPE_DRED) {
                dred_control* pDredControl = DRED_CONTROL(pPage);
                if (dred_control_is_of_type(pDredControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                    if (pDred->config.textEditorEnableWordWrap) {
                        dred_text_editor_enable_word_wrap(DRED_TEXT_EDITOR(pDredControl));
                    } else {
                        dred_text_editor_disable_word_wrap(DRED_TEXT_EDITOR(pDredControl));
                    }
                }
            }
        }
    }

    dtk_menu_set_item_checked_by_id(&pDred->menus.textView, DRED_MENU_ITEM_ID_TEXT_VIEW_WORDWRAP, pDred->config.textEditorEnableWordWrap);
}

void dred_config_on_set__texteditor_drag_and_drop(dred_context* pDred)
{
    for (dtk_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
        for (dtk_uint32 iTab = 0; iTab < dtk_tabgroup_get_tab_count(pTabGroup); ++iTab) {
            dtk_control* pPage = dtk_tabgroup_get_tab_page(pTabGroup, iTab);
            if (pPage != NULL && pPage->type == DTK_CONTROL_TYPE_DRED) {
                dred_control* pDredControl = DRED_CONTROL(pPage);
                if (dred_control_is_of_type(pDredControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                    if (pDred->config.textEditorEnableDragAndDrop) {
                        dred_text_editor_enable_drag_and_drop(DRED_TEXT_EDITOR(pDredControl));
                    } else {
                        dred_text_editor_disable_drag_and_drop(DRED_TEXT_EDITOR(pDredControl));
                    }
                }
            }
        }
    }
}


void dred_config_on_set__cpp_syntax_color(dred_context* pDred)
{
    for (dtk_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
        for (dtk_uint32 iTab = 0; iTab < dtk_tabgroup_get_tab_count(pTabGroup); ++iTab) {
            dtk_control* pPage = dtk_tabgroup_get_tab_page(pTabGroup, iTab);
            if (pPage != NULL && pPage->type == DTK_CONTROL_TYPE_DRED) {
                dred_control* pDredControl = DRED_CONTROL(pPage);
                if (dred_control_is_of_type(pDredControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
                    dred_text_editor_refresh_styling(DRED_TEXT_EDITOR(pDredControl));
                }
            }
        }
    }
}


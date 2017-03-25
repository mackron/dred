// Copyright (C) 2016 David Reid. See included LICENSE file.

static dr_bool32 dred__preprocess_system_command(dred_context* pDred, const char* cmd, char* cmdOut, size_t cmdOutSize)
{
    // Currently, the only symbols we're expanding is the enescaped "%" character, which is expanded to the relative
    // path of the currently focused file.
    dred_editor* pEditor = dred_get_focused_editor(pDred);
    if (pEditor == NULL) {
        return DR_FALSE;
    }

    const char* absolutePath = dred_editor_get_file_path(pEditor);
    if (absolutePath == NULL) {
        return DR_FALSE;
    }

    char currentDir[DRED_MAX_PATH];
    dr_get_current_directory(currentDir, sizeof(currentDir));

    char relativePath[DRED_MAX_PATH];
    if (drpath_is_absolute(absolutePath)) {
        if (!drpath_to_relative(absolutePath, dr_get_current_directory(currentDir, sizeof(currentDir)), relativePath, sizeof(relativePath))) {
            return DR_FALSE;
        }
    } else {
        if (strcpy_s(relativePath, sizeof(relativePath), absolutePath) != 0) {
            return DR_FALSE;
        }
    }

    size_t relativePathLength = strlen(relativePath);


    char prevC = '\0';
    while (cmdOutSize > 0 && *cmd != '\0') {
        if (*cmd == '%' && prevC != '\\') {
            if (strncpy_s(cmdOut, cmdOutSize, relativePath, relativePathLength) != 0) {
                return DR_FALSE;
            }

            cmdOut += relativePathLength;
            cmdOutSize -= relativePathLength;
        } else {
            *cmdOut = *cmd;
            cmdOut += 1;
            cmdOutSize -= 1;
        }

        prevC = *cmd;
        cmd += 1;
    }

    *cmdOut = '\0';
    return DR_TRUE;
}

dr_bool32 dred_command__system_command(dred_context* pDred, const char* value)
{
    char valueExpanded[4096];
    if (dred__preprocess_system_command(pDred, value, valueExpanded, sizeof(valueExpanded))) {
        return system(valueExpanded) == 0;
    } else {
        return system(value) == 0;
    }
}

dr_bool32 dred_command__cmdbar(dred_context* pDred, const char* value)
{
    dred_focus_command_bar_and_set_text(pDred, value);
    return DR_TRUE;
}

dr_bool32 dred_command__bind(dred_context* pDred, const char* value)
{
    char shortcutName[256];
    dred_shortcut shortcut;
    char commandStr[4096];
    if (dred_parse_bind_command(value, shortcutName, sizeof(shortcutName), &shortcut, commandStr, sizeof(commandStr))) {
        size_t existingIndex;
        if (dred_shortcut_table_find_by_name(&pDred->shortcutTable, shortcutName, &existingIndex)) {
            dred_shortcut_table_replace_by_index(&pDred->shortcutTable, existingIndex, shortcutName, commandStr, shortcut.acceleratorCount, shortcut.accelerators);
        }

        return DR_TRUE;
    }

    return DR_FALSE;
}

dr_bool32 dred_command__load_config(dred_context* pDred, const char* value)
{
    char path[DRED_MAX_PATH];
    if (!dr_next_token(value, path, sizeof(path))) {
        return DR_FALSE;
    }

    if (drpath_is_relative(path) && !dr_file_exists(path)) {
        // If the path is relative and the file does not exist relative to the current directory, try making it relative to
        // the user config directory.
        char configPath[DRED_MAX_PATH];
        if (dred_get_config_folder_path(configPath, sizeof(configPath))) {
            char pathAbsolute[DRED_MAX_PATH];
            if (drpath_append_and_clean(pathAbsolute, sizeof(pathAbsolute), configPath, path)) {
                return dred_load_config(pDred, pathAbsolute);
            }
        }
    } else {
        return dred_load_config(pDred, path);
    }

    return DR_FALSE;
}

dr_bool32 dred_command__set(dred_context* pDred, const char* value)
{
    char name[256];
    value = dr_next_token(value, name, sizeof(name));
    if (value == NULL) {
        return DR_FALSE;
    }

    dred_config_set(&pDred->config, name, dr_ltrim(value));
    return DR_TRUE;
}

dr_bool32 dred_command__set_default(dred_context* pDred, const char* value)
{
    char name[256];
    value = dr_next_token(value, name, sizeof(name));
    if (value == NULL) {
        return DR_FALSE;
    }

    dred_config_set_default(&pDred->config, name);
    return DR_TRUE;
}


dr_bool32 dred_command__show_menu_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_menu_bar(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__hide_menu_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_hide_menu_bar(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__toggle_menu_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_toggle_menu_bar(pDred);
    return DR_TRUE;
}


dr_bool32 dred_command__show_tab_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_tabbars(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__hide_tab_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_hide_tabbars(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__toggle_tab_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_toggle_tabbars(pDred);
    return DR_TRUE;
}


dr_bool32 dred_command__enable_auto_hide_cmdbar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_enable_auto_hide_command_bar(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__disable_auto_hide_cmdbar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_disable_auto_hide_command_bar(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__toggle_auto_hide_cmdbar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_toggle_auto_hide_command_bar(pDred);
    return DR_TRUE;
}


dr_bool32 dred_command__next_tab(dred_context* pDred, const char* value)
{
    (void)value;
    dred_tabgroup_activate_next_tab(dred_get_focused_tabgroup(pDred));
    return DR_TRUE;
}

dr_bool32 dred_command__prev_tab(dred_context* pDred, const char* value)
{
    (void)value;
    dred_tabgroup_activate_prev_tab(dred_get_focused_tabgroup(pDred));
    return DR_TRUE;
}


dr_bool32 dred_command__cd(dred_context* pDred, const char* value)
{
    (void)pDred;
    return dr_set_current_directory(value);
}



dr_bool32 dred_command__new(dred_context* pDred, const char* value)
{
    dred_create_and_open_file(pDred, value);
    return DR_TRUE;
}

dr_bool32 dred_command__open(dred_context* pDred, const char* value)
{
    char fileName[DRED_MAX_PATH];
    if (dr_next_token(value, fileName, sizeof(fileName)) == NULL) {
        dred_show_open_file_dialog(pDred);
    } else {
        if (!dred_open_file(pDred, fileName)) {
            dred_cmdbar_set_message(pDred->pCmdBar, "Failed to open file.");
            return DR_FALSE;
        }
    }

    return DR_TRUE;
}

dr_bool32 dred_command__save(dred_context* pDred, const char* value)
{
    if (!dred_save_focused_file(pDred, value)) {
        return dred_save_focused_file_as(pDred);
    }

    return DR_TRUE;
}

dr_bool32 dred_command__save_all(dred_context* pDred, const char* value)
{
    (void)value;
    dred_save_all_open_files(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__save_as(dred_context* pDred, const char* value)
{
    (void)value;
    dred_save_focused_file_as(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__close(dred_context* pDred, const char* value)
{
    (void)value;
    dred_close_focused_file_with_confirmation(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__close_all(dred_context* pDred, const char* value)
{
    (void)value;
    dred_close_all_tabs_with_confirmation(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__exit(dred_context* pDred, const char* value)
{
    (void)value;
    dred_close(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__help(dred_context* pDred, const char* value)
{
    (void)value;
    (void)pDred;
    // TODO: Implement me.

    return DR_TRUE;
}

dr_bool32 dred_command__about(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_about_dialog(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__settings(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_settings_dialog(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__print(dred_context* pDred, const char* value)
{
    (void)value;

    dred_print_info printInfo;
    return dred_show_print_dialog(pDred, NULL, &printInfo);
}

dr_bool32 dred_command__reload(dred_context* pDred, const char* value)
{
    (void)value;
    dred_check_if_focused_file_is_dirty_and_reload(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__add_favourite(dred_context* pDred, const char* value)
{
    const char* absolutePath = value;
    if (absolutePath == NULL || absolutePath[0] == '\0') {
        dred_editor* pEditor = dred_get_focused_editor(pDred);
        if (pEditor != NULL) {
            absolutePath = dred_editor_get_file_path(pEditor);
        }
    }

    if (absolutePath == NULL || absolutePath[0] == '\0') {
        return DR_FALSE;
    }

    // TODO: If absolutePath is relative, make it absolute based on the current directory.

    return dred_add_favourite(pDred, absolutePath);
}

dr_bool32 dred_command__remove_favourite(dred_context* pDred, const char* value)
{
    const char* absolutePath = value;
    if (absolutePath == NULL || absolutePath[0] == '\0') {
        dred_editor* pEditor = dred_get_focused_editor(pDred);
        if (pEditor != NULL) {
            absolutePath = dred_editor_get_file_path(pEditor);
        }
    }

    if (absolutePath == NULL || absolutePath[0] == '\0') {
        return DR_FALSE;
    }

    // TODO: If absolutePath is relative, make it absolute based on the current directory.

    return dred_remove_favourite(pDred, absolutePath);
}


dr_bool32 dred_command__undo(dred_context* pDred, const char* value)
{
    (void)value;

    dred_control* pFocusedControl = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedControl == NULL) {
        return DR_TRUE;
    }

    if (dred_control_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_TEXTVIEW)) {
        dred_textview_undo(DRED_TEXTVIEW(pFocusedControl));
        return DR_TRUE;
    }

    return DR_FALSE;
}

dr_bool32 dred_command__redo(dred_context* pDred, const char* value)
{
    (void)value;

    dred_control* pFocusedControl = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedControl == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_TEXTVIEW)) {
        dred_textview_redo(DRED_TEXTVIEW(pFocusedControl));
        return DR_TRUE;
    }

    return DR_FALSE;
}

dr_bool32 dred_command__cut(dred_context* pDred, const char* value)
{
    dred_command__copy(pDred, value);
    dred_command__delete(pDred, value);
    return DR_TRUE;
}

dr_bool32 dred_command__copy(dred_context* pDred, const char* value)
{
    (void)value;

    dred_control* pFocusedControl = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedControl == NULL) {
        return DR_TRUE;
    }

    if (dred_control_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_TEXTVIEW)) {
        size_t selectedTextLength = dred_textview_get_selected_text(DRED_TEXTVIEW(pFocusedControl), NULL, 0);
        char* selectedText = (char*)malloc(selectedTextLength + 1);
        if (selectedText == NULL) {
            return DR_FALSE;
        }

        selectedTextLength = dred_textview_get_selected_text(DRED_TEXTVIEW(pFocusedControl), selectedText, selectedTextLength + 1);
        dtk_clipboard_set_text(&pDred->tk, selectedText, selectedTextLength);
        free(selectedText);

        return DR_TRUE;
    }

    return DR_FALSE;
}

dr_bool32 dred_command__paste(dred_context* pDred, const char* value)
{
    (void)value;

    dred_control* pFocusedControl = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedControl == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_TEXTVIEW)) {
        char* clipboardText = dtk_clipboard_get_text(&pDred->tk);
        if (clipboardText == NULL) {
            return DR_FALSE;
        }

        dr_bool32 wasTextChanged = DR_FALSE;
        dred_textview_prepare_undo_point(DRED_TEXTVIEW(pFocusedControl));
        {
            wasTextChanged = dred_textview_delete_selected_text_no_undo(DRED_TEXTVIEW(pFocusedControl)) || wasTextChanged;
            wasTextChanged = dred_textview_insert_text_at_cursors_no_undo(DRED_TEXTVIEW(pFocusedControl), clipboardText) || wasTextChanged;
        }
        if (wasTextChanged) { dred_textview_commit_undo_point(DRED_TEXTVIEW(pFocusedControl)); }

        dtk_clipboard_free_text(&pDred->tk, clipboardText);
        return DR_TRUE;
    }

    return DR_FALSE;
}

dr_bool32 dred_command__delete(dred_context* pDred, const char* value)
{
    (void)value;

    dred_control* pFocusedControl = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedControl == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_TEXTVIEW)) {
        dred_textview_do_delete(DRED_TEXTVIEW(pFocusedControl), 0);
        return DR_TRUE;
    }

    return DR_FALSE;
}

dr_bool32 dred_command__select_all(dred_context* pDred, const char* value)
{
    (void)value;

    dred_control* pFocusedControl = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedControl == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(pFocusedControl, DRED_CONTROL_TYPE_TEXTVIEW)) {
        dred_textview_select_all(DRED_TEXTVIEW(pFocusedControl));
        return DR_TRUE;
    }

    return DR_FALSE;
}


dr_bool32 dred_command__goto(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(DRED_CONTROL(pFocusedEditor), DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char param[256];
        if (dr_next_token(value, param, sizeof(param)) != NULL) {
            // If the last character is a %, we use a ratio based goto.
            if (param[strlen(param) - 1] == '%') {
                param[strlen(param) - 1] = '\0';
                dred_text_editor_goto_ratio(DRED_TEXT_EDITOR(pFocusedEditor), (unsigned int)abs(atoi(param)));
            } else {
                dred_text_editor_goto_line(DRED_TEXT_EDITOR(pFocusedEditor), (unsigned int)abs(atoi(param)));
            }

            return DR_TRUE;
        }
    }

    return DR_FALSE;
}

dr_bool32 dred_command__find(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(DRED_CONTROL(pFocusedEditor), DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char query[1024];
        if (dr_next_token(value, query, sizeof(query)) != NULL) {
            dred_text_editor_deselect_all_in_focused_view(DRED_TEXT_EDITOR(pFocusedEditor));
            if (!dred_text_editor_find_and_select_next(DRED_TEXT_EDITOR(pFocusedEditor), query)) {
                dred_cmdbar_set_message(pDred->pCmdBar, "No results found.");
                return DR_FALSE;
            }

            return DR_TRUE;
        }
    }

    return DR_FALSE;
}

dr_bool32 dred_command__replace(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(DRED_CONTROL(pFocusedEditor), DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char query[1024];
        value = dr_next_token(value, query, sizeof(query));
        if (value != NULL) {
            char replacement[1024];
            value = dr_next_token(value, replacement, sizeof(replacement));
            if (value != NULL) {
                if (!dred_text_editor_find_and_replace_next(DRED_TEXT_EDITOR(pFocusedEditor), query, replacement)) {
                    dred_cmdbar_set_message(pDred->pCmdBar, "No results found.");
                    return DR_FALSE;
                }

                return DR_TRUE;
            }
        }
    }

    return DR_FALSE;
}

dr_bool32 dred_command__replace_all(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(DRED_CONTROL(pFocusedEditor), DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char query[1024];
        value = dr_next_token(value, query, sizeof(query));
        if (value != NULL) {
            char replacement[1024];
            value = dr_next_token(value, replacement, sizeof(replacement));
            if (value != NULL) {
                if (!dred_text_editor_find_and_replace_all(DRED_TEXT_EDITOR(pFocusedEditor), query, replacement)) {
                    dred_cmdbar_set_message(pDred->pCmdBar, "No results found.");
                    return DR_FALSE;
                }

                return DR_TRUE;
            }
        }
    }

    return DR_FALSE;
}

dr_bool32 dred_command__show_line_numbers(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_line_numbers(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__hide_line_numbers(dred_context* pDred, const char* value)
{
    (void)value;
    dred_hide_line_numbers(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__toggle_line_numbers(dred_context* pDred, const char* value)
{
    (void)value;
    dred_toggle_line_numbers(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__toggle_word_wrap(dred_context* pDred, const char* value)
{
    (void)value;
    dred_toggle_word_wrap(pDred);
    return DR_TRUE;
}

dr_bool32 dred_command__zoom(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(DRED_CONTROL(pFocusedEditor), DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        dred_set_text_editor_scale(pDred, (float)atof(value));
    }

    return DR_TRUE;
}

dr_bool32 dred_command__unindent(dred_context* pDred, const char* value)
{
    (void)value;

    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(DRED_CONTROL(pFocusedEditor), DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        dred_text_editor_unindent_selected_blocks(DRED_TEXT_EDITOR(pFocusedEditor));
    }

    return DR_TRUE;
}

dr_bool32 dred_command__insert_date(dred_context* pDred, const char* value)
{
    (void)value;

    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(DRED_CONTROL(pFocusedEditor), DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char dateStr[256];
        dr_datetime_short(dr_now(), dateStr, sizeof(dateStr));

        dred_text_editor_insert_text_at_cursors(DRED_TEXT_EDITOR(pFocusedEditor), dateStr);
    }

    return DR_TRUE;
}

dr_bool32 dred_command__export2cstring(dred_context* pDred, const char* value)
{
    (void)value;

    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    if (dred_control_is_of_type(DRED_CONTROL(pFocusedEditor), DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char* pOriginalText = NULL;

        size_t selectedTextSize = dred_text_editor_get_selected_text(DRED_TEXT_EDITOR(pFocusedEditor), NULL, 0);
        if (selectedTextSize > 0) {
            pOriginalText = (char*)malloc(selectedTextSize + 1);
            if (pOriginalText != NULL) {
                selectedTextSize = dred_text_editor_get_selected_text(DRED_TEXT_EDITOR(pFocusedEditor), pOriginalText, selectedTextSize + 1);
                if (selectedTextSize == 0) {
                    free(pOriginalText);
                    pOriginalText = NULL;
                }
            }
        } else {
            // Nothing is selected - do the entire file.
            size_t textSize = dred_text_editor_get_text(DRED_TEXT_EDITOR(pFocusedEditor), NULL, 0);
            if (textSize > 0) {
                pOriginalText = (char*)malloc(textSize + 1);
                if (pOriginalText != NULL) {
                    textSize = dred_text_editor_get_text(DRED_TEXT_EDITOR(pFocusedEditor), pOriginalText, textSize + 1);
                    if (textSize == 0) {
                        free(pOriginalText);
                        pOriginalText = NULL;
                    }
                }
            } else {
                pOriginalText = (char*)calloc(1, 1);  // <-- Empty string. A single byte allocated on the heap. Nice. (Just doing it like this to simplify the clean-up logic below.)
            }
        }

        if (pOriginalText != NULL) {
            if (dred_open_new_text_file(pDred)) {
                dred_editor* pNewEditor = dred_get_focused_editor(pDred);
                if (pNewEditor != NULL) {
                    char* cstring = dred_codegen_buffer_to_c_string((const unsigned char*)pOriginalText, strlen(pOriginalText), NULL);
                    if (cstring != NULL) {
                        dred_text_editor_set_text(DRED_TEXT_EDITOR(pNewEditor), cstring);
                    }

                    free(cstring);
                }
            }

            free(pOriginalText);
            return DR_TRUE;
        }
    }

    // If we get here it means the focused editor does not support this command. May want to route this to packages, though...
    return DR_FALSE;
}


char* dred__get_selection_or_word_under_cursor_of_text_editor(dred_text_editor* pTextEditor)
{
    // Selection first.
    size_t selectionSize = dred_text_editor_get_selected_text(pTextEditor, NULL, 0);
    if (selectionSize > 0) {
        char* selectionText = (char*)malloc(selectionSize + 1);
        if (selectionText == NULL) {
            return NULL;    // Out of memory.
        }

        dred_text_editor_get_selected_text(pTextEditor, selectionText, selectionSize + 1);
        return selectionText;
    }

    // Fall back to the word.
    size_t wordBeg;
    size_t wordEnd;
    if (dred_text_editor_get_word_under_cursor(pTextEditor, dred_text_editor_get_last_cursor(pTextEditor), &wordBeg, &wordEnd)) {
        size_t wordLen = wordEnd - wordBeg;
        char* wordText = (char*)malloc(wordLen + 1);
        if (wordText == NULL) {
            return NULL;    // Out of memory.
        }

        dred_text_editor_get_subtext(pTextEditor, wordBeg, wordEnd, wordText, wordLen + 1);
        return wordText;
    }

    return NULL;
}

// This is a temporary function until a more flexible solution is thought up. All it does is sets the command with a pre-filled variable that is selected.
dr_bool32 dred__set_cmdbar_command_with_current_selection(dred_context* pDred, const char* command)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return DR_FALSE;
    }

    char* fullCommandText = NULL;
    if (dred_control_is_of_type(DRED_CONTROL(pFocusedEditor), DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char* prefilledText = dred__get_selection_or_word_under_cursor_of_text_editor(DRED_TEXT_EDITOR(pFocusedEditor));
        if (prefilledText != NULL) {
            fullCommandText = dtk_make_stringf("%s %s", command, prefilledText);
            free(prefilledText);
        } else {
            fullCommandText = dtk_make_stringf("%s ", command);
        }
    } else {
        fullCommandText = dtk_make_stringf("%s ", command);
    }

    dred_focus_command_bar_and_set_text(pDred, fullCommandText);

    size_t selectionBeg = strlen(command) + 1;  // +1 for the space between the command and the value.
    size_t selectionEnd = strlen(fullCommandText);
    dred_cmdbar_select_text(pDred->pCmdBar, selectionBeg, selectionEnd);

    dtk_free_string(fullCommandText);

    return DR_TRUE;
}

dr_bool32 dred_command__cmdbar_find_prefilled(dred_context* pDred, const char* value)
{
    (void)value;
    return dred__set_cmdbar_command_with_current_selection(pDred, "find");
}

dr_bool32 dred_command__cmdbar_replace_prefilled(dred_context* pDred, const char* value)
{
    (void)value;
    return dred__set_cmdbar_command_with_current_selection(pDred, "replace");
}

dr_bool32 dred_command__cmdbar_replace_all_prefilled(dred_context* pDred, const char* value)
{
    (void)value;
    return dred__set_cmdbar_command_with_current_selection(pDred, "replace-all");
}



dr_bool32 dred_find_command(const char* cmdStr, dred_command* pCommandOut, const char** pValueOut)
{
    if (cmdStr == NULL || pCommandOut == NULL || pValueOut == NULL) {
        return DR_FALSE;
    }

    // Trim whitespace.
    cmdStr = dr_first_non_whitespace(cmdStr);

    // Special case for "!".
    if (cmdStr[0] == '!') {
        *pCommandOut = g_Commands[0];
        *pValueOut   = dr_first_non_whitespace(cmdStr + 1);
        return DR_TRUE;
    }


    // For every other command the value will come after the first whitespace.
    char func[256];
    cmdStr = dr_next_token(cmdStr, func, sizeof(func));
    if (cmdStr == NULL) {
        return DR_FALSE;
    }

    size_t index = dred_find_command_index(func);
    if (index == (size_t)-1) {
        return DR_FALSE;
    }

    *pCommandOut = g_Commands[index];
    *pValueOut = dr_first_non_whitespace(cmdStr);
    return DR_TRUE;
}

size_t dred_find_command_index(const char* cmdFunc)
{
    // The command names are stored in a single pool of memory.
    if (cmdFunc[0] == '!') {
        return 0;
    }

    for (size_t i = 0; i < DRED_COMMAND_COUNT; ++i) {
        if (strcmp(cmdFunc, g_CommandNames[i]) == 0) {
            return i;
        }
    }

    return (size_t)-1;
}

int dred_find_commands_starting_with_qsort_comp(const void* a, const void* b)
{
    size_t indexA = *(size_t*)a;
    size_t indexB = *(size_t*)b;
    return strcmp(g_CommandNames[indexA], g_CommandNames[indexB]);
}

size_t dred_find_commands_starting_with(size_t* pIndexOut, size_t sizeOut, const char* cmdFunc)
{
    if (sizeOut == 0 && pIndexOut != NULL) return 0;

    size_t count = 0;
    for (size_t i = 0; i < DRED_COMMAND_COUNT; ++i) {
        if (dred_string_is_null_or_empty(cmdFunc) || strncmp(g_CommandNames[i], cmdFunc, strlen(cmdFunc)) == 0) {
            if (count < sizeOut) {
                pIndexOut[count] = i;
            }

            count += 1;
        }
    }

    if (pIndexOut != NULL) {
        qsort(pIndexOut, count, sizeof(size_t), dred_find_commands_starting_with_qsort_comp);
    }

    return count;
}

const char* dred_next_command_string(const char* cmdStr, char* cmdOut, size_t cmdOutSize, dred_command_separator* pSeparatorOut)
{
    if (cmdStr == NULL || cmdStr[0] == '\0' || cmdOut == NULL || cmdOutSize == 0) {
        return NULL;
    }

    size_t length = 0;
    while (cmdStr[length] != '\0') {
        if (cmdStr[length] == ';') {
            if (pSeparatorOut) *pSeparatorOut = dred_command_separator_semicolon;
            strncpy_s(cmdOut, cmdOutSize, cmdStr, length);
            return cmdStr + length + 1; // +1 to skip past the ';'
        }
        if (cmdStr[length] == '&' && cmdStr[length+1] == '&') {
            if (pSeparatorOut) *pSeparatorOut = dred_command_separator_and;
            strncpy_s(cmdOut, cmdOutSize, cmdStr, length);
            return cmdStr + length + 2; // +2 to skip past the '&&'
        }
        if (cmdStr[length] == '|' && cmdStr[length+1] == '|') {
            if (pSeparatorOut) *pSeparatorOut = dred_command_separator_or;
            strncpy_s(cmdOut, cmdOutSize, cmdStr, length);
            return cmdStr + length + 2; // +2 to skip past the '||'
        }

        length += 1;
    }

    if (length > 0) {
        if (pSeparatorOut) *pSeparatorOut = dred_command_separator_none;
        strcpy_s(cmdOut, cmdOutSize, cmdStr);
        return cmdOut + length;
    }

    cmdOut[0] = '\0';
    return NULL;
}

dr_bool32 dred_parse_bind_command(const char* value, char* nameOut, size_t nameOutSize, dred_shortcut* pShortcutOut, char* pCmdOut, size_t cmdOutSize)
{
    if (value == NULL) {
        return DR_FALSE;
    }

    value = dr_next_token(value, nameOut, (unsigned int)nameOutSize);
    if (value != NULL) {
        char shortcutStr[256];
        value = dr_next_token(value, shortcutStr, sizeof(shortcutStr));
        if (value != NULL) {
            if (pShortcutOut) *pShortcutOut = dred_shortcut_parse(shortcutStr);

            const char* cmd = dr_first_non_whitespace(value);
            if (cmd == NULL) {
                return DR_FALSE;
            }

            if (cmd[0] == '\"') {
                return dr_next_token(cmd, pCmdOut, (unsigned int)cmdOutSize) != NULL;    // <-- This will trim the double-quotes.
            } else {
                return strcpy_s(pCmdOut, cmdOutSize, cmd) == 0;
            }
        }
    }

    return DR_FALSE;
}

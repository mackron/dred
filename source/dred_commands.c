
static bool dred__preprocess_system_command(dred_context* pDred, const char* cmd, char* cmdOut, size_t cmdOutSize)
{
    // Currently, the only symbols we're expanding is the enescaped "%" character, which is expanded to the relative
    // path of the currently focused file.
    dred_editor* pEditor = dred_get_focused_editor(pDred);
    if (pEditor == NULL) {
        return false;
    }

    const char* absolutePath = dred_editor_get_file_path(pEditor);
    if (absolutePath == NULL) {
        return false;
    }

    char currentDir[DRED_MAX_PATH];
    dr_get_current_directory(currentDir, sizeof(currentDir));

    char relativePath[DRED_MAX_PATH];
    if (drpath_is_absolute(absolutePath)) {
        if (!drpath_to_relative(absolutePath, dr_get_current_directory(currentDir, sizeof(currentDir)), relativePath, sizeof(relativePath))) {
            return false;
        }
    } else {
        if (strcpy_s(relativePath, sizeof(relativePath), absolutePath) != 0) {
            return false;
        }
    }

    size_t relativePathLength = strlen(relativePath);


    char prevC = '\0';
    while (cmdOutSize > 0 && *cmd != '\0') {
        if (*cmd == '%' && prevC != '\\') {
            if (strncpy_s(cmdOut, cmdOutSize, relativePath, relativePathLength) != 0) {
                return false;
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
    return true;
}

bool dred_command__system_command(dred_context* pDred, const char* value)
{
    char valueExpanded[4096];
    if (dred__preprocess_system_command(pDred, value, valueExpanded, sizeof(valueExpanded))) {
        return system(valueExpanded) == 0;
    } else {
        return system(value) == 0;
    }
}

bool dred_command__cmdbar(dred_context* pDred, const char* value)
{
    dred_focus_command_bar_and_set_text(pDred, value);
    return true;
}

bool dred_command__bind(dred_context* pDred, const char* value)
{
    char shortcutName[256];
    dred_shortcut shortcut;
    char commandStr[4096];
    if (dred_parse_bind_command(value, shortcutName, sizeof(shortcutName), &shortcut, commandStr, sizeof(commandStr))) {
        return dred_bind_shortcut(pDred, shortcutName, shortcut, commandStr);
    }

    return false;
}

bool dred_command__load_config(dred_context* pDred, const char* value)
{
    char path[DRED_MAX_PATH];
    if (!dr_next_token(value, path, sizeof(path))) {
        return false;
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

    return false;
}

bool dred_command__set(dred_context* pDred, const char* value)
{
    char name[256];
    value = dr_next_token(value, name, sizeof(name));
    if (value == NULL) {
        return false;
    }

    dred_config_set(&pDred->config, name, dr_ltrim(value));
    return true;
}

bool dred_command__set_default(dred_context* pDred, const char* value)
{
    char name[256];
    value = dr_next_token(value, name, sizeof(name));
    if (value == NULL) {
        return false;
    }

    dred_config_set_default(&pDred->config, name);
    return true;
}


bool dred_command__show_menu_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_menu_bar(pDred);
    return true;
}

bool dred_command__hide_menu_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_hide_menu_bar(pDred);
    return true;
}

bool dred_command__toggle_menu_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_toggle_menu_bar(pDred);
    return true;
}


bool dred_command__show_tab_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_tabbars(pDred);
    return true;
}

bool dred_command__hide_tab_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_hide_tabbars(pDred);
    return true;
}

bool dred_command__toggle_tab_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_toggle_tabbars(pDred);
    return true;
}


bool dred_command__enable_auto_hide_cmdbar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_enable_auto_hide_command_bar(pDred);
    return true;
}

bool dred_command__disable_auto_hide_cmdbar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_disable_auto_hide_command_bar(pDred);
    return true;
}

bool dred_command__toggle_auto_hide_cmdbar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_toggle_auto_hide_command_bar(pDred);
    return true;
}


bool dred_command__next_tab(dred_context* pDred, const char* value)
{
    (void)value;
    dred_tabgroup_activate_next_tab(dred_get_focused_tabgroup(pDred));
    return true;
}

bool dred_command__prev_tab(dred_context* pDred, const char* value)
{
    (void)value;
    dred_tabgroup_activate_prev_tab(dred_get_focused_tabgroup(pDred));
    return true;
}


bool dred_command__cd(dred_context* pDred, const char* value)
{
    (void)pDred;
    return dr_set_current_directory(value);
}



bool dred_command__new(dred_context* pDred, const char* value)
{
    dred_create_and_open_file(pDred, value);
    return true;
}

bool dred_command__open(dred_context* pDred, const char* value)
{
    char fileName[DRED_MAX_PATH];
    if (dr_next_token(value, fileName, sizeof(fileName)) == NULL) {
        dred_show_open_file_dialog(pDred);
    } else {
        if (!dred_open_file(pDred, fileName)) {
            dred_cmdbar_set_message(pDred->pCmdBar, "Failed to open file.");
            return false;
        }
    }

    return true;
}

bool dred_command__save(dred_context* pDred, const char* value)
{
    if (!dred_save_focused_file(pDred, value)) {
        return dred_save_focused_file_as(pDred);
    }

    return true;
}

bool dred_command__save_all(dred_context* pDred, const char* value)
{
    (void)value;
    dred_save_all_open_files(pDred);
    return true;
}

bool dred_command__save_as(dred_context* pDred, const char* value)
{
    (void)value;
    dred_save_focused_file_as(pDred);
    return true;
}

bool dred_command__close(dred_context* pDred, const char* value)
{
    (void)value;
    dred_close_focused_file_with_confirmation(pDred);
    return true;
}

bool dred_command__close_all(dred_context* pDred, const char* value)
{
    (void)value;
    dred_close_all_tabs_with_confirmation(pDred);
    return true;
}

bool dred_command__exit(dred_context* pDred, const char* value)
{
    (void)value;
    dred_close(pDred);
    return true;
}

bool dred_command__help(dred_context* pDred, const char* value)
{
    (void)value;
    (void)pDred;
    // TODO: Implement me.

    return true;
}

bool dred_command__about(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_about_dialog(pDred);
    return true;
}

bool dred_command__settings(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_settings_dialog(pDred);
    return true;
}

bool dred_command__reload(dred_context* pDred, const char* value)
{
    (void)value;
    dred_check_if_focused_file_is_dirty_and_reload(pDred);
    return true;
}


bool dred_command__undo(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return true;
    }

    if (pFocusedElement != NULL && drgui_is_of_type(pFocusedElement, DRED_CONTROL_TYPE_TEXTBOX)) {
        dred_textbox_undo(pFocusedElement);
        return true;
    }

    return false;
}

bool dred_command__redo(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return false;
    }

    if (pFocusedElement != NULL && drgui_is_of_type(pFocusedElement, DRED_CONTROL_TYPE_TEXTBOX)) {
        dred_textbox_redo(pFocusedElement);
        return true;
    }

    return false;
}

bool dred_command__cut(dred_context* pDred, const char* value)
{
    dred_command__copy(pDred, value);
    dred_command__delete(pDred, value);
    return true;
}

bool dred_command__copy(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return true;
    }

    if (pFocusedElement != NULL && drgui_is_of_type(pFocusedElement, DRED_CONTROL_TYPE_TEXTBOX)) {
        size_t selectedTextLength = dred_textbox_get_selected_text(pFocusedElement, NULL, 0);
        char* selectedText = (char*)malloc(selectedTextLength + 1);
        if (selectedText == NULL) {
            return false;
        }

        selectedTextLength = dred_textbox_get_selected_text(pFocusedElement, selectedText, selectedTextLength + 1);
        dred_clipboard_set_text(selectedText, selectedTextLength);
        return true;
    }

    return false;
}

bool dred_command__paste(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return false;
    }

    if (pFocusedElement != NULL && drgui_is_of_type(pFocusedElement, DRED_CONTROL_TYPE_TEXTBOX)) {
        char* clipboardText = dred_clipboard_get_text();
        if (clipboardText == NULL) {
            return false;
        }

        dred_textbox_delete_selected_text(pFocusedElement);
        dred_textbox_insert_text_at_cursor(pFocusedElement, clipboardText);

        dred_clipboard_free_text(clipboardText);
        return true;
    }

    return false;
}

bool dred_command__delete(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return false;
    }

    if (pFocusedElement != NULL && drgui_is_of_type(pFocusedElement, DRED_CONTROL_TYPE_TEXTBOX)) {
        dred_textbox_delete_selected_text(pFocusedElement);
        return true;
    }

    return false;
}

bool dred_command__select_all(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return false;
    }

    if (pFocusedElement != NULL && drgui_is_of_type(pFocusedElement, DRED_CONTROL_TYPE_TEXTBOX)) {
        dred_textbox_select_all(pFocusedElement);
        return true;
    }

    return false;
}


bool dred_command__goto(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return false;
    }

    if (dred_control_is_of_type(pFocusedEditor, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char param[256];
        if (dr_next_token(value, param, sizeof(param)) != NULL) {
            // If the last character is a %, we use a ratio based goto.
            if (param[strlen(param) - 1] == '%') {
                param[strlen(param) - 1] = '\0';
                dred_text_editor_goto_ratio(pFocusedEditor, (unsigned int)abs(atoi(param)));
            } else {
                dred_text_editor_goto_line(pFocusedEditor, (unsigned int)abs(atoi(param)));
            }

            return true;
        }
    }

    return false;
}

bool dred_command__find(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return false;
    }

    if (dred_control_is_of_type(pFocusedEditor, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char query[1024];
        if (dr_next_token(value, query, sizeof(query)) != NULL) {
            if (!dred_text_editor_find_and_select_next(pFocusedEditor, query)) {
                dred_cmdbar_set_message(pDred->pCmdBar, "No results found.");
                return false;
            }

            return true;
        }
    }

    return false;
}

bool dred_command__replace(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return false;
    }

    if (dred_control_is_of_type(pFocusedEditor, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char query[1024];
        value = dr_next_token(value, query, sizeof(query));
        if (value != NULL) {
            char replacement[1024];
            value = dr_next_token(value, replacement, sizeof(replacement));
            if (value != NULL) {
                if (!dred_text_editor_find_and_replace_next(pFocusedEditor, query, replacement)) {
                    dred_cmdbar_set_message(pDred->pCmdBar, "No results found.");
                    return false;
                }

                return true;
            }
        }
    }

    return false;
}

bool dred_command__replace_all(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return false;
    }

    if (dred_control_is_of_type(pFocusedEditor, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char query[1024];
        value = dr_next_token(value, query, sizeof(query));
        if (value != NULL) {
            char replacement[1024];
            value = dr_next_token(value, replacement, sizeof(replacement));
            if (value != NULL) {
                if (!dred_text_editor_find_and_replace_all(pFocusedEditor, query, replacement)) {
                    dred_cmdbar_set_message(pDred->pCmdBar, "No results found.");
                    return false;
                }

                return true;
            }
        }
    }

    return false;
}

bool dred_command__show_line_numbers(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_line_numbers(pDred);
    return true;
}

bool dred_command__hide_line_numbers(dred_context* pDred, const char* value)
{
    (void)value;
    dred_hide_line_numbers(pDred);
    return true;
}

bool dred_command__toggle_line_numbers(dred_context* pDred, const char* value)
{
    (void)value;
    dred_toggle_line_numbers(pDred);
    return true;
}

bool dred_command__zoom(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return false;
    }

    if (dred_control_is_of_type(pFocusedEditor, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        dred_set_text_editor_scale(pDred, (float)atof(value));
    }

    return true;
}

bool dred_command__unindent(dred_context* pDred, const char* value)
{
    (void)value;

    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return false;
    }

    if (dred_control_is_of_type(pFocusedEditor, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        dred_text_editor_unindent_selected_blocks(pFocusedEditor);
    }

    return true;
}





bool dred_find_command(const char* cmdStr, dred_command* pCommandOut, const char** pValueOut)
{
    if (cmdStr == NULL || pCommandOut == NULL || pValueOut == NULL) {
        return false;
    }

    // Trim whitespace.
    cmdStr = dr_first_non_whitespace(cmdStr);

    // Special case for "!".
    if (cmdStr[0] == '!') {
        *pCommandOut = g_Commands[0];
        *pValueOut   = dr_first_non_whitespace(cmdStr + 1);
        return true;
    }


    // For every other command the value will come after the first whitespace.
    char func[256];
    cmdStr = dr_next_token(cmdStr, func, sizeof(func));
    if (cmdStr == NULL) {
        return false;
    }

    size_t index = dred_find_command_index(func);
    if (index == (size_t)-1) {
        return false;
    }

    *pCommandOut = g_Commands[index];
    *pValueOut = dr_first_non_whitespace(cmdStr);
    return true;
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
            return cmdStr + length + 2; // +1 to skip past the '&&'
        }
        if (cmdStr[length] == '|' && cmdStr[length+1] == '|') {
            if (pSeparatorOut) *pSeparatorOut = dred_command_separator_or;
            strncpy_s(cmdOut, cmdOutSize, cmdStr, length);
            return cmdStr + length + 2; // +1 to skip past the '||'
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

bool dred_parse_bind_command(const char* value, char* nameOut, size_t nameOutSize, dred_shortcut* pShortcutOut, char* pCmdOut, size_t cmdOutSize)
{
    if (value == NULL) {
        return false;
    }

    value = dr_next_token(value, nameOut, nameOutSize);
    if (value != NULL) {
        char shortcutStr[256];
        value = dr_next_token(value, shortcutStr, sizeof(shortcutStr));
        if (value != NULL) {
            if (pShortcutOut) *pShortcutOut = dred_shortcut_parse(shortcutStr);

            const char* cmd = dr_first_non_whitespace(value);
            if (cmd == NULL) {
                return false;
            }

            if (cmd[0] == '\"') {
                return dr_next_token(cmd, pCmdOut, cmdOutSize) != NULL;    // <-- This will trim the double-quotes.
            } else {
                return strcpy_s(pCmdOut, cmdOutSize, cmd) == 0;
            }
        }
    }

    return false;
}

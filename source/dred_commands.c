
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

void dred_command__system_command(dred_context* pDred, const char* value)
{
    char valueExpanded[4096];
    if (dred__preprocess_system_command(pDred, value, valueExpanded, sizeof(valueExpanded))) {
        system(valueExpanded);
    } else {
        system(value);
    }
}

void dred_command__cmdbar(dred_context* pDred, const char* value)
{
    dred_focus_command_bar_and_set_text(pDred, value);
}

void dred_command__bind(dred_context* pDred, const char* value)
{
    char shortcutName[256];
    dred_shortcut shortcut;
    const char* commandStr;
    if (dred_parse_bind_command(value, shortcutName, sizeof(shortcutName), &shortcut, &commandStr)) {
        dred_bind_shortcut(pDred, shortcutName, shortcut, commandStr);
    }
}


void dred_command__show_menu_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_window_show_menu(pDred->pMainWindow);
}

void dred_command__hide_menu_bar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_window_hide_menu(pDred->pMainWindow);
}

void dred_command__toggle_menu_bar(dred_context* pDred, const char* value)
{
    (void)value;

    if (dred_window_is_showing_menu(pDred->pMainWindow)) {
        dred_window_hide_menu(pDred->pMainWindow);
    } else {
        dred_window_show_menu(pDred->pMainWindow);
    }
}


void dred_command__show_tabbars(dred_context* pDred, const char* value)
{
    (void)value;

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
        dred_tabgroup_show_tabbar(pTabGroup);
    }
}

void dred_command__hide_tabbars(dred_context* pDred, const char* value)
{
    (void)value;

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
        dred_tabgroup_hide_tabbar(pTabGroup);
    }
}

void dred_command__toggle_tabbars(dred_context* pDred, const char* value)
{
    (void)value;

    for (dred_tabgroup* pTabGroup = dred_first_tabgroup(pDred); pTabGroup != NULL; pTabGroup = dred_next_tabgroup(pDred, pTabGroup)) {
        if (dred_tabgroup_is_showing_tabbar(pTabGroup)) {
            dred_tabgroup_hide_tabbar(pTabGroup);
        } else {
            dred_tabgroup_show_tabbar(pTabGroup);
        }
    }
}


void dred_command__enable_auto_hide_cmdbar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_enable_auto_hide_command_bar(pDred);
}

void dred_command__disable_auto_hide_cmdbar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_disable_auto_hide_command_bar(pDred);
}

void dred_command__toggle_auto_hide_cmdbar(dred_context* pDred, const char* value)
{
    (void)value;
    dred_toggle_auto_hide_command_bar(pDred);
}


void dred_command__next_tab(dred_context* pDred, const char* value)
{
    (void)value;
    dred_tabgroup_activate_next_tab(dred_get_focused_tabgroup(pDred));
}

void dred_command__prev_tab(dred_context* pDred, const char* value)
{
    (void)value;
    dred_tabgroup_activate_prev_tab(dred_get_focused_tabgroup(pDred));
}



void dred_command__new(dred_context* pDred, const char* value)
{
    dred_create_and_open_file(pDred, value);
}

void dred_command__open(dred_context* pDred, const char* value)
{
    char fileName[DRED_MAX_PATH];
    if (dr_next_token(value, fileName, sizeof(fileName)) == NULL) {
        dred_show_open_file_dialog(pDred);
    } else {
        if (!dred_open_file(pDred, fileName)) {
            dred_cmdbar_set_message(pDred->pCmdBar, "Failed to open file.");
        }
    }
}

void dred_command__save(dred_context* pDred, const char* value)
{
    if (!dred_save_focused_file(pDred, value)) {
        dred_save_focused_file_as(pDred);
    }
}

void dred_command__save_all(dred_context* pDred, const char* value)
{
    (void)value;
    dred_save_all_open_files(pDred);
}

void dred_command__save_as(dred_context* pDred, const char* value)
{
    (void)value;
    dred_save_focused_file_as(pDred);
}

void dred_command__close(dred_context* pDred, const char* value)
{
    (void)value;
    dred_close_focused_file_with_confirmation(pDred);
}

void dred_command__close_all(dred_context* pDred, const char* value)
{
    (void)value;
    dred_close_all_tabs_with_confirmation(pDred);
}

void dred_command__exit(dred_context* pDred, const char* value)
{
    (void)value;
    dred_close(pDred);
}

void dred_command__help(dred_context* pDred, const char* value)
{
    (void)value;
    (void)pDred;
    // TODO: Implement me.
}

void dred_command__about(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_about_dialog(pDred);
}


void dred_command__undo(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return;
    }

    // If the parent of the element is a DRED_CONTROL_TYPE_TEXTBOX then it means a dred textbox is focused and we can do an undo.
    drgui_element* pTextBox = drgui_get_parent(pFocusedElement);
    if (pTextBox != NULL && drgui_is_of_type(pTextBox, DRED_CONTROL_TYPE_TEXTBOX)) {
        dred_textbox_undo(pTextBox);
    }
}

void dred_command__redo(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return;
    }

    drgui_element* pTextBox = drgui_get_parent(pFocusedElement);
    if (pTextBox != NULL && drgui_is_of_type(pTextBox, DRED_CONTROL_TYPE_TEXTBOX)) {
        dred_textbox_redo(pTextBox);
    }
}

void dred_command__cut(dred_context* pDred, const char* value)
{
    dred_command__copy(pDred, value);
    dred_command__delete(pDred, value);
}

void dred_command__copy(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return;
    }

    drgui_element* pTextBox = drgui_get_parent(pFocusedElement);
    if (pTextBox != NULL && drgui_is_of_type(pTextBox, DRED_CONTROL_TYPE_TEXTBOX)) {
        size_t selectedTextLength = dred_textbox_get_selected_text(pTextBox, NULL, 0);
        char* selectedText = (char*)malloc(selectedTextLength + 1);
        if (selectedText == NULL) {
            return;
        }

        selectedTextLength = dred_textbox_get_selected_text(pTextBox, selectedText, selectedTextLength + 1);
        dred_clipboard_set_text(selectedText, selectedTextLength);
    }
}

void dred_command__paste(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return;
    }

    drgui_element* pTextBox = drgui_get_parent(pFocusedElement);
    if (pTextBox != NULL && drgui_is_of_type(pTextBox, DRED_CONTROL_TYPE_TEXTBOX)) {
        char* clipboardText = dred_clipboard_get_text();
        if (clipboardText == NULL) {
            return;
        }

        dred_textbox_delete_selected_text(pTextBox);
        dred_textbox_insert_text_at_cursor(pTextBox, clipboardText);

        dred_clipboard_free_text(clipboardText);
    }
}

void dred_command__delete(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return;
    }

    drgui_element* pTextBox = drgui_get_parent(pFocusedElement);
    if (pTextBox != NULL && drgui_is_of_type(pTextBox, DRED_CONTROL_TYPE_TEXTBOX)) {
        dred_textbox_delete_selected_text(pTextBox);
    }
}

void dred_command__select_all(dred_context* pDred, const char* value)
{
    (void)value;

    drgui_element* pFocusedElement = dred_get_element_with_keyboard_capture(pDred);
    if (pFocusedElement == NULL) {
        return;
    }

    // If the parent of the element is a DRED_CONTROL_TYPE_TEXTBOX then it means a dred textbox is focused and we can do an undo.
    drgui_element* pTextBox = drgui_get_parent(pFocusedElement);
    if (pTextBox != NULL && drgui_is_of_type(pTextBox, DRED_CONTROL_TYPE_TEXTBOX)) {
        dred_textbox_select_all(pTextBox);
    }
}


void dred_command__goto(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return;
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
        }
    }
}

void dred_command__find(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return;
    }

    if (dred_control_is_of_type(pFocusedEditor, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        char query[1024];
        if (dr_next_token(value, query, sizeof(query)) != NULL) {
            if (!dred_text_editor_find_and_select_next(pFocusedEditor, query)) {
                dred_cmdbar_set_message(pDred->pCmdBar, "No results found.");
            }
        }
    }
}

void dred_command__replace(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return;
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
                }
            }
        }
    }
}

void dred_command__replace_all(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return;
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
                }
            }
        }
    }
}

void dred_command__show_line_numbers(dred_context* pDred, const char* value)
{
    (void)value;
    dred_show_line_numbers(pDred);
}

void dred_command__hide_line_numbers(dred_context* pDred, const char* value)
{
    (void)value;
    dred_hide_line_numbers(pDred);
}

void dred_command__toggle_line_numbers(dred_context* pDred, const char* value)
{
    (void)value;
    dred_toggle_line_numbers(pDred);
}

void dred_command__zoom(dred_context* pDred, const char* value)
{
    dred_editor* pFocusedEditor = dred_get_focused_editor(pDred);
    if (pFocusedEditor == NULL) {
        return;
    }

    if (dred_control_is_of_type(pFocusedEditor, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
        dred_set_text_editor_scale(pDred, (float)atof(value));
    }
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

bool dred_parse_bind_command(const char* value, char* nameOut, size_t nameOutSize, dred_shortcut* pShortcutOut, const char** pCmdOut)
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
            if (pCmdOut) *pCmdOut = dr_first_non_whitespace(value);
            return true;
        }
    }

    return false;
}

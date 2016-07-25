// Copyright (C) 2016 David Reid. See included LICENSE file.

dred_textbox* dred_text_editor__get_textbox(dred_text_editor* pTextEditor)
{
    if (pTextEditor == NULL) {
        return NULL;
    }

    return pTextEditor->pTextBox;
}

void dred_text_editor__register_style(dred_text_editor* pTextEditor, dred_text_style* pStyle)
{
    dred_gui_font_metrics fontMetrics;
    dred_gui_get_font_metrics(pStyle->pFont, &fontMetrics);

    drte_font_metrics drteFontMetrics = drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth);
    drte_engine_register_style_token(dred_textbox_get_engine(dred_text_editor__get_textbox(pTextEditor)), (drte_style_token)pStyle, drteFontMetrics);
}


void dred_text_editor__on_size(dred_control* pControl, float newWidth, float newHeight)
{
    dred_text_editor* pTextEditor = DRED_TEXT_EDITOR(pControl);
    assert(pTextEditor != NULL);

    dred_textbox* pTextBox = dred_text_editor__get_textbox(pTextEditor);
    if (pTextBox == NULL) {
        return;
    }

    // The text box should take up the entire area of the editor.
    dred_control_set_size(DRED_CONTROL(pTextBox), newWidth, newHeight);
}

void dred_text_editor__on_capture_keyboard(dred_control* pControl, dred_control* pPrevCapturedControl)
{
    (void)pPrevCapturedControl;

    dred_text_editor* pTextEditor = DRED_TEXT_EDITOR(pControl);
    assert(pTextEditor != NULL);
    
    // When a text editor receives keyboard focus it should be routed down to the text box control.
    dred_textbox* pTextBox = dred_text_editor__get_textbox(pTextEditor);
    if (pTextBox == NULL) {
        return;
    }

    dred_capture_keyboard(dred_control_get_context(pControl), DRED_CONTROL(pTextBox));
}

void dred_text_editor_textbox__on_key_down(dred_control* pControl, dred_key key, int stateFlags)
{
    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    if (key == DRED_GUI_ESCAPE) {
        dred_focus_command_bar(dred_control_get_context(pControl));
    } else {
        dred_textbox_on_key_down(DRED_CONTROL(pTextBox), key, stateFlags);
    }
}

void dred_text_editor_textbox__on_mouse_wheel(dred_control* pControl, int delta, int mousePosX, int mousePosY, int stateFlags)
{
    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    dred_text_editor* pTextEditor = DRED_TEXT_EDITOR(dred_control_get_parent(pControl));
    if (pTextEditor == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pControl);
    assert(pDred != NULL);

    if (stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) {
        // When setting the scale, we actually do it application-wide, not just local to the current text editor.
        float oldTextScale = dred_get_text_editor_scale(pDred);
        float newTextScale;
        if (delta > 0) {
            newTextScale = oldTextScale * (1.0f + ( delta * 0.1f));
        } else {
            newTextScale = oldTextScale / (1.0f + (-delta * 0.1f));
        }

        // Always make sure the 100% scale is selectable.
        if ((newTextScale < 1 && oldTextScale > 1) || (newTextScale > 1 && oldTextScale < 1)) {
            newTextScale = 1;
        }


        dred_set_text_editor_scale(pDred, newTextScale);
    } else {
        dred_textbox_on_mouse_wheel(DRED_CONTROL(pTextBox), delta, mousePosX, mousePosY, stateFlags);
    }
}

void dred_text_editor_textbox__on_mouse_button_up(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    dred_text_editor* pTextEditor = DRED_TEXT_EDITOR(dred_control_get_parent(pControl));
    if (pTextEditor == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pControl);
    assert(pDred != NULL);

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_RIGHT) {
        dred_control_show_popup_menu(pControl, pDred->menuLibrary.pPopupMenu_TextEditor, mousePosX, mousePosY);
    } else {
        dred_textbox_on_mouse_button_up(DRED_CONTROL(pTextBox), mouseButton, mousePosX, mousePosY, stateFlags);
    }
}

void dred_text_editor_textbox__on_cursor_move(dred_textbox* pTextBox)
{
    dred_text_editor* pTextEditor = DRED_TEXT_EDITOR(dred_control_get_parent(DRED_CONTROL(pTextBox)));
    assert(pTextEditor != NULL);

    dred_update_info_bar(dred_control_get_context(DRED_CONTROL(pTextEditor)), DRED_CONTROL(pTextEditor));
}

void dred_text_editor_textbox__on_capture_keyboard(dred_control* pControl, dred_control* pPrevCapturedControl)
{
    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    dred_text_editor* pTextEditor = DRED_TEXT_EDITOR(dred_control_get_parent(pControl));
    if (pTextEditor == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pControl);
    assert(pDred != NULL);

    if (pDred->config.enableAutoReload) {
        dred_editor_check_if_dirty_and_reload(DRED_EDITOR(pTextEditor));
    }

    // Fall through to the text boxes normal capture_keyboard event handler...
    dred_textbox_on_capture_keyboard(DRED_CONTROL(pTextBox), pPrevCapturedControl);
}


void dred_text_editor_textbox__on_undo_point_changed(dred_textbox* pTextBox, unsigned int iUndoPoint)
{
    dred_text_editor* pTextEditor = DRED_TEXT_EDITOR(dred_control_get_parent(DRED_CONTROL(pTextBox)));
    if (pTextEditor == NULL) {
        return;
    }

    if (iUndoPoint == pTextEditor->iBaseUndoPoint) {
        dred_editor_unmark_as_modified(DRED_EDITOR(pTextEditor));
    } else {
        dred_editor_mark_as_modified(DRED_EDITOR(pTextEditor));
    }
}

bool dred_text_editor__on_save(dred_editor* pEditor, dred_file file, const char* filePath)
{
    dred_text_editor* pTextEditor = DRED_TEXT_EDITOR(pEditor);
    assert(pTextEditor != NULL);

    dred_textbox* pTextBox = dred_text_editor__get_textbox(pTextEditor);
    if (pTextBox == NULL) {
        return false;
    }

    size_t textLength = dred_textbox_get_text(pTextBox, NULL, 0);
    char* text = malloc(textLength + 1);
    if (text == NULL) {
        return false;
    }
    dred_textbox_get_text(pTextBox, text, textLength+1);

    bool result = dred_file_write_string(file, text);
    free(text);

    // After saving we need to update the base undo point and unmark the file as modified.
    if (result) {
        pTextEditor->iBaseUndoPoint = dred_textbox_get_undo_points_remaining_count(pTextBox);

        // Syntax highlighting needs to be updated based on the file extension.
        dred_text_editor_set_highlighter(pTextEditor, dred_get_language_by_file_path(dred_control_get_context(DRED_CONTROL(pTextEditor)), filePath));
    }

    return result;
}

bool dred_text_editor__on_reload(dred_editor* pEditor)
{
    dred_text_editor* pTextEditor = DRED_TEXT_EDITOR(pEditor);
    assert(pTextEditor != NULL);

    dred_textbox* pTextBox = dred_text_editor__get_textbox(pTextEditor);
    if (pTextBox == NULL) {
        return false;
    }

    char* pFileData = dr_open_and_read_text_file(dred_editor_get_file_path(DRED_EDITOR(pTextEditor)), NULL);
    if (pFileData == NULL) {
        return false;
    }

    dred_textbox_set_text(pTextEditor->pTextBox, pFileData);
    dr_free_file_data(pFileData);

    // After reloading we need to update the base undo point and unmark the file as modified.
    pTextEditor->iBaseUndoPoint = dred_textbox_get_undo_points_remaining_count(pTextBox);
    dred_editor_unmark_as_modified(DRED_EDITOR(pTextEditor));

    return true;
}

dred_text_editor* dred_text_editor_create(dred_context* pDred, dred_control* pParent, float sizeX, float sizeY, const char* filePathAbsolute)
{
    dred_text_editor* pTextEditor = (dred_text_editor*)calloc(1, sizeof(*pTextEditor));
    if (pTextEditor == NULL) {
        return NULL;
    }

    if (!dred_editor_init(DRED_EDITOR(pTextEditor), pDred, pParent, DRED_CONTROL_TYPE_TEXT_EDITOR, sizeX, sizeY, filePathAbsolute)) {
        free(pTextEditor);
        return NULL;
    }


    pTextEditor->pTextBox = &pTextEditor->textBox;
    if (!dred_textbox_init(pTextEditor->pTextBox, pDred, DRED_CONTROL(pTextEditor))) {
        dred_editor_uninit(DRED_EDITOR(pTextEditor));
        free(pTextEditor);
        return NULL;
    }

    dred_control_set_size(DRED_CONTROL(pTextEditor->pTextBox), sizeX, sizeY);

    pTextEditor->textScale = 1;
    dred_text_editor_set_highlighter(pTextEditor, dred_get_language_by_file_path(pDred, filePathAbsolute));

    if (filePathAbsolute != NULL && filePathAbsolute[0] != '\0') {
        char* pFileData = dr_open_and_read_text_file(filePathAbsolute, NULL);
        if (pFileData == NULL) {
            dred_textbox_uninit(pTextEditor->pTextBox);
            dred_editor_uninit(DRED_EDITOR(pTextEditor));
            free(pTextEditor);
            return NULL;
        }

        dred_textbox_set_text(pTextEditor->pTextBox, pFileData);
        dred_textbox_clear_undo_stack(pTextEditor->pTextBox);
        dr_free_file_data(pFileData);
    }


    // Events.
    dred_control_set_on_size(DRED_CONTROL(pTextEditor), dred_text_editor__on_size);
    dred_control_set_on_capture_keyboard(DRED_CONTROL(pTextEditor), dred_text_editor__on_capture_keyboard);
    dred_editor_set_on_save(DRED_EDITOR(pTextEditor), dred_text_editor__on_save);
    dred_editor_set_on_reload(DRED_EDITOR(pTextEditor), dred_text_editor__on_reload);
    dred_control_set_on_mouse_button_up(DRED_CONTROL(pTextEditor->pTextBox), dred_text_editor_textbox__on_mouse_button_up);
    dred_control_set_on_mouse_wheel(DRED_CONTROL(pTextEditor->pTextBox), dred_text_editor_textbox__on_mouse_wheel);
    dred_control_set_on_key_down(DRED_CONTROL(pTextEditor->pTextBox), dred_text_editor_textbox__on_key_down);
    dred_control_set_on_capture_keyboard(DRED_CONTROL(pTextEditor->pTextBox), dred_text_editor_textbox__on_capture_keyboard);
    dred_textbox_set_on_cursor_move(pTextEditor->pTextBox, dred_text_editor_textbox__on_cursor_move);
    dred_textbox_set_on_undo_point_changed(pTextEditor->pTextBox, dred_text_editor_textbox__on_undo_point_changed);

    // Initialize the styling.
    dred_text_editor_refresh_styling(pTextEditor);

    // Word wrap.
    if (pDred->config.textEditorEnableWordWrap) {
        dred_text_editor_enable_word_wrap(pTextEditor);
    }
    
    
    return pTextEditor;
}

void dred_text_editor_delete(dred_text_editor* pTextEditor)
{
    if (pTextEditor == NULL) {
        return;
    }

    dred_textbox_uninit(pTextEditor->pTextBox);

    dred_editor_uninit(DRED_EDITOR(pTextEditor));
    free(pTextEditor);
}


size_t dred_text_editor_get_text(dred_text_editor* pTextEditor, char* pTextOut, size_t textOutSize)
{
    if (pTextEditor == NULL) {
        return 0;
    }

    return dred_textbox_get_text(pTextEditor->pTextBox, pTextOut, textOutSize);
}


void dred_text_editor_enable_word_wrap(dred_text_editor* pTextEditor)
{
    if (pTextEditor == NULL) {
        return;
    }

    dred_textbox_enable_word_wrap(pTextEditor->pTextBox);
}

void dred_text_editor_disable_word_wrap(dred_text_editor* pTextEditor)
{
    if (pTextEditor == NULL) {
        return;
    }

    dred_textbox_disable_word_wrap(pTextEditor->pTextBox);
}

bool dred_text_editor_is_word_wrap_enabled(dred_text_editor* pTextEditor)
{
    if (pTextEditor == NULL) {
        return false;
    }

    return dred_textbox_is_word_wrap_enabled(pTextEditor->pTextBox);
}


bool dred_text_editor_insert_text_at_cursors(dred_text_editor* pTextEditor, const char* text)
{
    if (pTextEditor == NULL) {
        return false;
    }

    return dred_textbox_insert_text_at_cursors(pTextEditor->pTextBox, text);
}


void dred_text_editor_refresh_styling(dred_text_editor* pTextEditor)
{
    assert(pTextEditor != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pTextEditor));
    assert(pDred != NULL);

    dred_control_begin_dirty(DRED_CONTROL(pTextEditor));
    {
        // Highlighting.
        pTextEditor->highlighter.styles.common.comment.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->config.textEditorScale);
        pTextEditor->highlighter.styles.common.comment.bgColor = pDred->config.textEditorBGColor;
        pTextEditor->highlighter.styles.common.comment.fgColor = pDred->config.cppCommentTextColor;
        dred_text_editor__register_style(pTextEditor, &pTextEditor->highlighter.styles.common.comment);

        pTextEditor->highlighter.styles.common.string.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->config.textEditorScale);
        pTextEditor->highlighter.styles.common.string.bgColor = pDred->config.textEditorBGColor;
        pTextEditor->highlighter.styles.common.string.fgColor = pDred->config.cppStringTextColor;
        dred_text_editor__register_style(pTextEditor, &pTextEditor->highlighter.styles.common.string);
        
        pTextEditor->highlighter.styles.common.keyword.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->config.textEditorScale);
        pTextEditor->highlighter.styles.common.keyword.bgColor = pDred->config.textEditorBGColor;
        pTextEditor->highlighter.styles.common.keyword.fgColor = pDred->config.cppKeywordTextColor;
        dred_text_editor__register_style(pTextEditor, &pTextEditor->highlighter.styles.common.keyword);


        dred_textbox_set_font(pTextEditor->pTextBox, dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale));    // TODO: <-- This font needs to be unacquired.
        dred_textbox_set_text_color(pTextEditor->pTextBox, pDred->config.textEditorTextColor);
        dred_textbox_set_cursor_color(pTextEditor->pTextBox, pDred->config.textEditorCursorColor);
        dred_textbox_set_background_color(pTextEditor->pTextBox, pDred->config.textEditorBGColor);
        dred_textbox_set_selection_background_color(pTextEditor->pTextBox, pDred->config.textEditorSelectionBGColor);
        dred_textbox_set_active_line_background_color(pTextEditor->pTextBox, pDred->config.textEditorActiveLineColor);
        dred_textbox_set_padding(pTextEditor->pTextBox, 0);
        dred_textbox_set_line_numbers_color(pTextEditor->pTextBox, pDred->config.textEditorLineNumbersColor);
        dred_textbox_set_line_numbers_background_color(pTextEditor->pTextBox, pDred->config.textEditorLineNumbersBGColor);
        dred_textbox_set_line_numbers_padding(pTextEditor->pTextBox, pDred->config.textEditorLineNumbersPadding);
    
        dred_textbox_set_scrollbar_track_color(pTextEditor->pTextBox, pDred->config.textEditorSBTrackColor);
        dred_textbox_set_scrollbar_thumb_color(pTextEditor->pTextBox, pDred->config.textEditorSBThumbColor);
        dred_textbox_set_scrollbar_thumb_color_hovered(pTextEditor->pTextBox, pDred->config.textEditorSBThumbColorHovered);
        dred_textbox_set_scrollbar_thumb_color_pressed(pTextEditor->pTextBox, pDred->config.textEditorSBThumbColorPressed);
        dred_textbox_set_scrollbar_size(pTextEditor->pTextBox, pDred->config.textEditorSBSize * pDred->uiScale);
        if (pDred->config.textEditorShowScrollbarHorz) {
            dred_textbox_enable_horizontal_scrollbar(pTextEditor->pTextBox);
        } else {
            dred_textbox_disable_horizontal_scrollbar(pTextEditor->pTextBox);
        }
        if (pDred->config.textEditorShowScrollbarVert) {
            dred_textbox_enable_vertical_scrollbar(pTextEditor->pTextBox);
        } else {
            dred_textbox_disable_vertical_scrollbar(pTextEditor->pTextBox);
        }
        if (pDred->config.textEditorEnableExcessScrolling) {
            dred_textbox_enable_excess_scrolling(pTextEditor->pTextBox);
        } else {
            dred_textbox_disable_excess_scrolling(pTextEditor->pTextBox);
        }


        dred_textbox_set_tab_size_in_spaces(pTextEditor->pTextBox, pDred->config.textEditorTabSizeInSpaces);
        if (pDred->config.textEditorTabsToSpacesEnabled) {
            dred_textbox_enable_tabs_to_spaces(pTextEditor->pTextBox);
        } else {
            dred_textbox_disable_tabs_to_spaces(pTextEditor->pTextBox);
        }

        if (pDred->config.textEditorShowLineNumbers) {
            dred_text_editor_show_line_numbers(pTextEditor);
        } else {
            dred_text_editor_hide_line_numbers(pTextEditor);
        }

        dred_text_editor_set_text_scale(pTextEditor, pDred->config.textEditorScale);
    }
    dred_control_end_dirty(DRED_CONTROL(pTextEditor));
}

void dred_text_editor_set_highlighter(dred_text_editor* pTextEditor, const char* lang)
{
    if (pTextEditor == NULL) {
        return;
    }

    drte_engine* pEngine = dred_textbox_get_engine(dred_text_editor__get_textbox(pTextEditor));
    assert(pEngine != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pTextEditor));
    assert(pDred != NULL);

    if (lang == NULL) {
        drte_engine_set_highlighter(pEngine, NULL, NULL);
    } else {
        // Unfortunately highlighting is not quite ready for prime time.
        drte_engine_set_highlighter(pEngine, NULL, NULL);
        (void)pDred;
        (void)pTextEditor;
#if 0
        if (strcmp(lang, "c") == 0) {
            dred_highlighter_init(&pTextEditor->highlighter, pDred, dred_textbox_get_engine(pTextEditor->pTextBox), g_KeywordsC, sizeof(g_KeywordsC) / sizeof(g_KeywordsC[0]));
            drte_engine_set_highlighter(dred_textbox_get_engine(pTextEditor->pTextBox), pTextEditor->highlighter.onNextHighlight, &pTextEditor->highlighter);
        }
#endif
    }
}


void dred_text_editor_set_font(dred_text_editor* pTextEditor, dred_font* pFont)
{
    if (pTextEditor == NULL) {
        return;
    }

    dred_textbox_set_font(pTextEditor->pTextBox, dred_font_acquire_subfont(pFont, pFont->pDred->uiScale));
}


void dred_text_editor_show_line_numbers(dred_text_editor* pTextEditor)
{
    if (pTextEditor == NULL) {
        return;
    }

    dred_textbox_show_line_numbers(pTextEditor->pTextBox);
}

void dred_text_editor_hide_line_numbers(dred_text_editor* pTextEditor)
{
    if (pTextEditor == NULL) {
        return;
    }

    dred_textbox_hide_line_numbers(pTextEditor->pTextBox);
}


size_t dred_text_editor_get_cursor_line(dred_text_editor* pTextEditor)
{
    if (pTextEditor == NULL) {
        return 0;
    }

    return dred_textbox_get_cursor_line(pTextEditor->pTextBox);
}

size_t dred_text_editor_get_cursor_column(dred_text_editor* pTextEditor)
{
    if (pTextEditor == NULL) {
        return 0;
    }

    return dred_textbox_get_cursor_column(pTextEditor->pTextBox);
}


void dred_text_editor_goto_ratio(dred_text_editor* pTextEditor, size_t ratio)
{
    if (pTextEditor == NULL) {
        return;
    }

    if (ratio > 100) {
        ratio = 100;
    }

    dred_text_editor_goto_line(pTextEditor, (size_t)(roundf(dred_textbox_get_line_count(pTextEditor->pTextBox) * (ratio/100.0f))));
}

void dred_text_editor_goto_line(dred_text_editor* pTextEditor, size_t lineNumber)
{
    if (pTextEditor == NULL) {
        return;
    }

    if (lineNumber == 0) {
        lineNumber = 1;
    }
    if (lineNumber > dred_textbox_get_line_count(pTextEditor->pTextBox)) {
        lineNumber = dred_textbox_get_line_count(pTextEditor->pTextBox);
    }

    dred_textbox_deselect_all(pTextEditor->pTextBox);
    dred_textbox_move_cursor_to_start_of_line_by_index(pTextEditor->pTextBox, lineNumber - 1);
}


bool dred_text_editor_find_and_select_next(dred_text_editor* pTextEditor, const char* text)
{
    if (pTextEditor == NULL) {
        return false;
    }

    return dred_textbox_find_and_select_next(pTextEditor->pTextBox, text);
}

bool dred_text_editor_find_and_replace_next(dred_text_editor* pTextEditor, const char* text, const char* replacement)
{
    if (pTextEditor == NULL) {
        return false;
    }

    return dred_textbox_find_and_replace_next(pTextEditor->pTextBox, text, replacement);
}

bool dred_text_editor_find_and_replace_all(dred_text_editor* pTextEditor, const char* text, const char* replacement)
{
    if (pTextEditor == NULL) {
        return false;
    }

    return dred_textbox_find_and_replace_all(pTextEditor->pTextBox, text, replacement);
}


void dred_text_editor_set_text_scale(dred_text_editor* pTextEditor, float textScale)
{
    if (pTextEditor == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pTextEditor));
    assert(pDred != NULL);

    pTextEditor->textScale = dr_clamp(textScale, 0.1f, 4.0f);
    dred_textbox_set_line_numbers_width(pTextEditor->pTextBox, (48 + pDred->config.textEditorLineNumbersPadding) * pDred->uiScale * pTextEditor->textScale);
    dred_textbox_set_line_numbers_padding(pTextEditor->pTextBox, pDred->config.textEditorLineNumbersPadding * pDred->uiScale * pTextEditor->textScale);
    dred_textbox_set_font(pTextEditor->pTextBox, dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale * pTextEditor->textScale));    // TODO: <-- This font needs to be unacquired.
    dred_textbox_set_cursor_width(pTextEditor->pTextBox, pDred->config.textEditorCursorWidth * pDred->uiScale * pTextEditor->textScale);
}

void dred_text_editor_unindent_selected_blocks(dred_text_editor* pTextEditor)
{
    if (pTextEditor == NULL) {
        return;
    }

    dred_textbox_unindent_selected_blocks(pTextEditor->pTextBox);
}
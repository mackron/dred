
typedef struct
{
    dred_textbox* pTextBox;
    unsigned int iBaseUndoPoint;    // Used to determine whether or no the file has been modified.
    float textScale;

    dred_highlighter highlighter;
} dred_text_editor_data;



dred_textbox* dred_text_editor__get_textbox(dred_text_editor* pTextEditor)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return NULL;
    }

    return data->pTextBox;
}

void dred_text_editor__register_style(dred_text_editor* pTextEditor, dred_text_style* pStyle)
{
    drgui_font_metrics fontMetrics;
    drgui_get_font_metrics(pStyle->pFont, &fontMetrics);

    drte_font_metrics drteFontMetrics = drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth);
    drte_engine_register_style_token(dred_textbox_get_engine(dred_text_editor__get_textbox(pTextEditor)), (drte_style_token)pStyle, drteFontMetrics);
}


void dred_text_editor__on_size(dred_text_editor* pTextEditor, float newWidth, float newHeight)
{
    dred_textbox* pTextBox = dred_text_editor__get_textbox(pTextEditor);
    if (pTextBox == NULL) {
        return;
    }

    // The text box should take up the entire area of the editor.
    dred_control_set_size(pTextBox, newWidth, newHeight);
}

void dred_text_editor__on_capture_keyboard(dred_text_editor* pTextEditor, drgui_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;
    
    // When a text editor receives keyboard focus it should be routed down to the text box control.
    dred_textbox* pTextBox = dred_text_editor__get_textbox(pTextEditor);
    if (pTextBox == NULL) {
        return;
    }

    dred_capture_keyboard(dred_control_get_context(pTextBox), pTextBox);
}

void dred_text_editor_textbox__on_key_down(dred_textbox* pTextBox, drgui_key key, int stateFlags)
{
    if (key == DRGUI_ESCAPE) {
        dred_focus_command_bar(dred_control_get_context(pTextBox));
    } else {
        dred_textbox_on_key_down(pTextBox, key, stateFlags);
    }
}

void dred_text_editor_textbox__on_mouse_wheel(dred_textbox* pTextBox, int delta, int mousePosX, int mousePosY, int stateFlags)
{
    dred_text_editor* pTextEditor = dred_control_get_parent(pTextBox);
    if (pTextEditor == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pTextEditor);
    assert(pDred != NULL);

    if (stateFlags & DRGUI_KEY_STATE_CTRL_DOWN) {
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
        dred_textbox_on_mouse_wheel(pTextBox, delta, mousePosX, mousePosY, stateFlags);
    }
}

void dred_text_editor_textbox__on_mouse_button_up(dred_textbox* pTextBox, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    dred_text_editor* pTextEditor = dred_control_get_parent(pTextBox);
    if (pTextEditor == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pTextEditor);
    assert(pDred != NULL);

    if (mouseButton == DRGUI_MOUSE_BUTTON_RIGHT) {
        dred_control_show_popup_menu(pTextBox, pDred->menuLibrary.pPopupMenu_TextEditor, mousePosX, mousePosY);
    } else {
        dred_textbox_on_mouse_button_up(pTextBox, mouseButton, mousePosX, mousePosY, stateFlags);
    }
}

void dred_text_editor_textbox__on_cursor_move(dred_textbox* pTextBox)
{
    dred_text_editor* pTextEditor = dred_control_get_parent(pTextBox);
    if (pTextEditor == NULL) {
        return;
    }

    dred_update_info_bar(dred_control_get_context(pTextEditor), pTextEditor);
}

void dred_text_editor_textbox__on_undo_point_changed(dred_textbox* pTextBox, unsigned int iUndoPoint)
{
    dred_text_editor* pTextEditor = dred_control_get_parent(pTextBox);
    if (pTextEditor == NULL) {
        return;
    }

    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    assert(data != NULL);

    if (iUndoPoint == data->iBaseUndoPoint) {
        dred_editor_unmark_as_modified(pTextEditor);
    } else {
        dred_editor_mark_as_modified(pTextEditor);
    }
}

bool dred_text_editor__on_save(dred_text_editor* pTextEditor, dred_file file, const char* filePath)
{
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
        dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
        assert(data != NULL);
        data->iBaseUndoPoint = dred_textbox_get_undo_points_remaining_count(pTextBox);

        // Syntax highlighting needs to be updated based on the file extension.
        dred_text_editor_set_highlighter(pTextEditor, dred_get_language_by_file_path(dred_control_get_context(pTextEditor), filePath));
    }

    return result;
}

bool dred_text_editor__on_reload(dred_text_editor* pTextEditor)
{
    dred_textbox* pTextBox = dred_text_editor__get_textbox(pTextEditor);
    if (pTextBox == NULL) {
        return false;
    }

    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    assert(data != NULL);

    char* pFileData = dr_open_and_read_text_file(dred_editor_get_file_path(pTextEditor), NULL);
    if (pFileData == NULL) {
        return false;
    }

    dred_textbox_set_text(data->pTextBox, pFileData);
    dr_free_file_data(pFileData);

    // After reloading we need to update the base undo point and unmark the file as modified.
    data->iBaseUndoPoint = dred_textbox_get_undo_points_remaining_count(pTextBox);
    dred_editor_unmark_as_modified(pTextEditor);

    return true;
}

dred_text_editor* dred_text_editor_create(dred_context* pDred, dred_control* pParent, const char* filePathAbsolute)
{
    dred_text_editor* pTextEditor = dred_editor_create(pDred, pParent, DRED_CONTROL_TYPE_TEXT_EDITOR, filePathAbsolute, sizeof(dred_text_editor_data));
    if (pTextEditor == NULL) {
        return NULL;
    }

    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    assert(data != NULL);
    
    data->pTextBox = dred_textbox_create(pDred, pTextEditor);
    if (data->pTextBox == NULL) {
        dred_editor_delete(pTextEditor);
        return NULL;
    }

    data->textScale = 1;

    dred_text_editor_set_highlighter(pTextEditor, dred_get_language_by_file_path(pDred, filePathAbsolute));


    if (filePathAbsolute != NULL && filePathAbsolute[0] != '\0') {
        char* pFileData = dr_open_and_read_text_file(filePathAbsolute, NULL);
        if (pFileData == NULL) {
            dred_textbox_delete(data->pTextBox);
            dred_editor_delete(pTextEditor);
            return NULL;
        }

        dred_textbox_set_text(data->pTextBox, pFileData);
        dred_textbox_clear_undo_stack(data->pTextBox);
        dr_free_file_data(pFileData);
    }


    // Events.
    dred_control_set_on_size(pTextEditor, dred_text_editor__on_size);
    dred_control_set_on_capture_keyboard(pTextEditor, dred_text_editor__on_capture_keyboard);
    dred_editor_set_on_save(pTextEditor, dred_text_editor__on_save);
    dred_editor_set_on_reload(pTextEditor, dred_text_editor__on_reload);
    dred_control_set_on_mouse_button_up(data->pTextBox, dred_text_editor_textbox__on_mouse_button_up);
    dred_control_set_on_mouse_wheel(data->pTextBox, dred_text_editor_textbox__on_mouse_wheel);
    dred_control_set_on_key_down(data->pTextBox, dred_text_editor_textbox__on_key_down);
    dred_textbox_set_on_cursor_move(data->pTextBox, dred_text_editor_textbox__on_cursor_move);
    dred_textbox_set_on_undo_point_changed(data->pTextBox, dred_text_editor_textbox__on_undo_point_changed);

    // Initialize the styling.
    dred_text_editor_refresh_styling(pTextEditor);
    
    return pTextEditor;
}

void dred_text_editor_delete(dred_text_editor* pTextEditor)
{
    if (pTextEditor == NULL) {
        return;
    }

    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data != NULL) {
        dred_textbox_delete(data->pTextBox);
    }
    
    dred_editor_delete(pTextEditor);
}


void dred_text_editor_refresh_styling(dred_text_editor* pTextEditor)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    assert(data != NULL);

    dred_context* pDred = dred_control_get_context(pTextEditor);
    assert(pDred != NULL);

    drgui_begin_dirty(pTextEditor);
    {
        // Highlighting.
        data->highlighter.styles.common.comment.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->config.textEditorScale);
        data->highlighter.styles.common.comment.bgColor = pDred->config.textEditorBGColor;
        data->highlighter.styles.common.comment.fgColor = pDred->config.cppCommentTextColor;
        dred_text_editor__register_style(pTextEditor, &data->highlighter.styles.common.comment);

        data->highlighter.styles.common.string.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->config.textEditorScale);
        data->highlighter.styles.common.string.bgColor = pDred->config.textEditorBGColor;
        data->highlighter.styles.common.string.fgColor = pDred->config.cppStringTextColor;
        dred_text_editor__register_style(pTextEditor, &data->highlighter.styles.common.string);
        
        data->highlighter.styles.common.keyword.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->config.textEditorScale);
        data->highlighter.styles.common.keyword.bgColor = pDred->config.textEditorBGColor;
        data->highlighter.styles.common.keyword.fgColor = pDred->config.cppKeywordTextColor;
        dred_text_editor__register_style(pTextEditor, &data->highlighter.styles.common.keyword);


        dred_textbox_set_font(data->pTextBox, dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale));    // TODO: <-- This font needs to be unacquired.
        dred_textbox_set_text_color(data->pTextBox, pDred->config.textEditorTextColor);
        dred_textbox_set_cursor_color(data->pTextBox, pDred->config.textEditorCursorColor);
        dred_textbox_set_background_color(data->pTextBox, pDred->config.textEditorBGColor);
        dred_textbox_set_selection_background_color(data->pTextBox, pDred->config.textEditorSelectionBGColor);
        dred_textbox_set_active_line_background_color(data->pTextBox, pDred->config.textEditorActiveLineColor);
        dred_textbox_set_padding(data->pTextBox, 0);
        dred_textbox_set_line_numbers_color(data->pTextBox, pDred->config.textEditorLineNumbersColor);
        dred_textbox_set_line_numbers_background_color(data->pTextBox, pDred->config.textEditorLineNumbersBGColor);
        dred_textbox_set_line_numbers_padding(data->pTextBox, pDred->config.textEditorLineNumbersPadding);
    
        dred_textbox_set_scrollbar_track_color(data->pTextBox, pDred->config.textEditorSBTrackColor);
        dred_textbox_set_scrollbar_thumb_color(data->pTextBox, pDred->config.textEditorSBThumbColor);
        dred_textbox_set_scrollbar_thumb_color_hovered(data->pTextBox, pDred->config.textEditorSBThumbColorHovered);
        dred_textbox_set_scrollbar_thumb_color_pressed(data->pTextBox, pDred->config.textEditorSBThumbColorPressed);
        dred_textbox_set_scrollbar_size(data->pTextBox, pDred->config.textEditorSBSize * pDred->uiScale);
        if (pDred->config.textEditorShowScrollbarHorz) {
            dred_textbox_enable_horizontal_scrollbar(data->pTextBox);
        } else {
            dred_textbox_disable_horizontal_scrollbar(data->pTextBox);
        }
        if (pDred->config.textEditorShowScrollbarVert) {
            dred_textbox_enable_vertical_scrollbar(data->pTextBox);
        } else {
            dred_textbox_disable_vertical_scrollbar(data->pTextBox);
        }
        if (pDred->config.textEditorEnableExcessScrolling) {
            dred_textbox_enable_excess_scrolling(data->pTextBox);
        } else {
            dred_textbox_disable_excess_scrolling(data->pTextBox);
        }


        dred_textbox_set_tab_size_in_spaces(data->pTextBox, pDred->config.textEditorTabSizeInSpaces);
        if (pDred->config.textEditorTabsToSpacesEnabled) {
            dred_textbox_enable_tabs_to_spaces(data->pTextBox);
        } else {
            dred_textbox_disable_tabs_to_spaces(data->pTextBox);
        }

        if (pDred->config.textEditorShowLineNumbers) {
            dred_text_editor_show_line_numbers(pTextEditor);
        } else {
            dred_text_editor_hide_line_numbers(pTextEditor);
        }

        dred_text_editor_set_text_scale(pTextEditor, pDred->config.textEditorScale);
    }
    drgui_end_dirty(pTextEditor);
}

void dred_text_editor_set_highlighter(dred_text_editor* pTextEditor, const char* lang)
{
    if (pTextEditor == NULL) {
        return;
    }

    drte_engine* pEngine = dred_textbox_get_engine(dred_text_editor__get_textbox(pTextEditor));
    assert(pEngine != NULL);

    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    assert(data != NULL);

    dred_context* pDred = dred_control_get_context(pTextEditor);
    assert(pDred != NULL);

    if (lang == NULL) {
        drte_engine_set_highlighter(pEngine, NULL, NULL);
    } else {
        // Unfortunately highlighting is not quite ready for prime time.
        drte_engine_set_highlighter(pEngine, NULL, NULL);
        (void)pDred;
        (void)data;
#if 0
        if (strcmp(lang, "c") == 0) {
            dred_highlighter_init(&data->highlighter, pDred, dred_textbox_get_engine(data->pTextBox), g_KeywordsC, sizeof(g_KeywordsC) / sizeof(g_KeywordsC[0]));
            drte_engine_set_highlighter(dred_textbox_get_engine(data->pTextBox), data->highlighter.onNextHighlight, &data->highlighter);
        }
#endif
    }
}


void dred_text_editor_set_font(dred_text_editor* pTextEditor, dred_font* pFont)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return;
    }

    dred_textbox_set_font(data->pTextBox, dred_font_acquire_subfont(pFont, pFont->pDred->uiScale));
}


void dred_text_editor_show_line_numbers(dred_text_editor* pTextEditor)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return;
    }

    dred_textbox_show_line_numbers(data->pTextBox);
}

void dred_text_editor_hide_line_numbers(dred_text_editor* pTextEditor)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return;
    }

    dred_textbox_hide_line_numbers(data->pTextBox);
}


size_t dred_text_editor_get_cursor_line(dred_text_editor* pTextEditor)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return 0;
    }

    return dred_textbox_get_cursor_line(data->pTextBox);
}

size_t dred_text_editor_get_cursor_column(dred_text_editor* pTextEditor)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return 0;
    }

    return dred_textbox_get_cursor_column(data->pTextBox);
}


void dred_text_editor_goto_ratio(dred_text_editor* pTextEditor, size_t ratio)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return;
    }

    if (ratio > 100) {
        ratio = 100;
    }

    dred_text_editor_goto_line(pTextEditor, (size_t)(roundf(dred_textbox_get_line_count(data->pTextBox) * (ratio/100.0f))));
}

void dred_text_editor_goto_line(dred_text_editor* pTextEditor, size_t lineNumber)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return;
    }

    if (lineNumber == 0) {
        lineNumber = 1;
    }
    if (lineNumber > dred_textbox_get_line_count(data->pTextBox)) {
        lineNumber = dred_textbox_get_line_count(data->pTextBox);
    }

    dred_textbox_deselect_all(data->pTextBox);
    dred_textbox_move_cursor_to_start_of_line_by_index(data->pTextBox, lineNumber - 1);
}


bool dred_text_editor_find_and_select_next(dred_text_editor* pTextEditor, const char* text)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return false;
    }

    return dred_textbox_find_and_select_next(data->pTextBox, text);
}

bool dred_text_editor_find_and_replace_next(dred_text_editor* pTextEditor, const char* text, const char* replacement)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return false;
    }

    return dred_textbox_find_and_replace_next(data->pTextBox, text, replacement);
}

bool dred_text_editor_find_and_replace_all(dred_text_editor* pTextEditor, const char* text, const char* replacement)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return false;
    }

    return dred_textbox_find_and_replace_all(data->pTextBox, text, replacement);
}


void dred_text_editor_set_text_scale(dred_text_editor* pTextEditor, float textScale)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pTextEditor);
    assert(pDred != NULL);

    data->textScale = dr_clamp(textScale, 0.1f, 4.0f);
    dred_textbox_set_line_numbers_width(data->pTextBox, (48 + pDred->config.textEditorLineNumbersPadding) * pDred->uiScale * data->textScale);
    dred_textbox_set_line_numbers_padding(data->pTextBox, pDred->config.textEditorLineNumbersPadding * pDred->uiScale * data->textScale);
    dred_textbox_set_font(data->pTextBox, dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale * data->textScale));    // TODO: <-- This font needs to be unacquired.
    dred_textbox_set_cursor_width(data->pTextBox, pDred->config.textEditorCursorWidth * pDred->uiScale * data->textScale);
}
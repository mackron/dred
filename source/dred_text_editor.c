
typedef struct
{
    dred_textbox* pTextBox;
    unsigned int iBaseUndoPoint;    // Used to determine whether or no the file has been modified.
    double textScale;
} dred_text_editor_data;

dred_textbox* dred_text_editor__get_textbox(dred_text_editor* pTextEditor)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return NULL;
    }

    return data->pTextBox;
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

bool dred_text_editor__on_save(dred_text_editor* pTextEditor, dred_file file)
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
    }

    return result;
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

    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    assert(data != NULL);

    dred_context* pDred = dred_control_get_context(pTextEditor);
    assert(pDred != NULL);

    if (stateFlags & DRGUI_KEY_STATE_CTRL_DOWN) {
        // When setting the scale, we actually do it application-wide, not just local to the current text editor.
        double oldTextScale = dred_get_text_editor_scale(pDred);
        double newTextScale;
        if (delta > 0) {
            newTextScale = oldTextScale * (1.0 + ( delta * 0.1));
        } else {
            newTextScale = oldTextScale / (1.0 + (-delta * 0.1));
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

    dred_textbox_set_vertical_align(data->pTextBox, drgui_text_engine_alignment_top);
    dred_textbox_set_font(data->pTextBox, dred_font_acquire_subfont(pDred->config.pTextEditorFont, (float)pDred->uiScale));    // TODO: <-- This font needs to be unacquired.
    dred_textbox_set_background_color(data->pTextBox, pDred->config.textEditorBGColor);
    dred_textbox_set_active_line_background_color(data->pTextBox, pDred->config.textEditorActiveLineColor);

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
    dred_control_set_on_mouse_wheel(data->pTextBox, dred_text_editor_textbox__on_mouse_wheel);
    dred_control_set_on_key_down(data->pTextBox, dred_text_editor_textbox__on_key_down);
    dred_textbox_set_on_undo_point_changed(data->pTextBox, dred_text_editor_textbox__on_undo_point_changed);

    if (pDred->isShowingLineNumbers) {
        dred_text_editor_show_line_numbers(pTextEditor);
    }

    dred_text_editor_set_text_scale(pTextEditor, pDred->textEditorScale);
    
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


void dred_text_editor_set_text_scale(dred_text_editor* pTextEditor, double textScale)
{
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pTextEditor);
    assert(pDred != NULL);

    data->textScale = dr_clamp(textScale, 0.1, 4.0);
    dred_textbox_set_line_numbers_width(data->pTextBox, (float)(64 * pDred->uiScale * data->textScale));
    dred_textbox_set_font(data->pTextBox, dred_font_acquire_subfont(pDred->config.pTextEditorFont, (float)(pDred->uiScale * data->textScale)));    // TODO: <-- This font needs to be unacquired.
}

typedef struct
{
    dred_textbox* pTextBox;
    unsigned int iBaseUndoPoint;    // Used to determine whether or no the file has been modified.
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
    dred_text_editor_data* data = (dred_text_editor_data*)dred_editor_get_extra_data(pTextEditor);
    assert(data != NULL);
    data->iBaseUndoPoint = dred_textbox_get_undo_points_remaining_count(pTextBox);
    dred_editor_unmark_as_modified(pTextEditor);

    return result;
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

    dred_textbox_set_vertical_align(data->pTextBox, drgui_text_engine_alignment_top);
    dred_textbox_set_font(data->pTextBox, dred_font_acquire_subfont(pDred->config.pTextEditorFont, 1));

    char* pFileData = dr_open_and_read_text_file(filePathAbsolute, NULL);
    if (pFileData == NULL) {
        dred_textbox_delete(data->pTextBox);
        dred_editor_delete(pTextEditor);
        return NULL;
    }

    dred_textbox_set_text(data->pTextBox, pFileData);
    dr_free_file_data(pFileData);

    // Events.
    dred_control_set_on_size(pTextEditor, dred_text_editor__on_size);
    dred_editor_set_on_save(pTextEditor, dred_text_editor__on_save);
    dred_textbox_set_on_undo_point_changed(data->pTextBox, dred_text_editor_textbox__on_undo_point_changed);
    
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
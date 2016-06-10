
typedef struct
{
    dred_textbox* pTextBox;
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


    // Events.
    dred_control_set_on_size(pTextEditor, dred_text_editor__on_size);

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
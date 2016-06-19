
typedef struct
{
    dred_textbox2* pTextBox;
} dred_settings_editor_data;

dred_textbox2* dred_settings_editor__get_textbox(dred_text_editor* pTextEditor)
{
    dred_settings_editor_data* data = (dred_settings_editor_data*)dred_editor_get_extra_data(pTextEditor);
    if (data == NULL) {
        return NULL;
    }

    return data->pTextBox;
}

void dred_settings_editor__on_size(dred_settings_editor* pSettingsEditor, float newWidth, float newHeight)
{
    dred_textbox2* pTextBox = dred_settings_editor__get_textbox(pSettingsEditor);
    if (pTextBox == NULL) {
        return;
    }

    // The text box should take up the entire area of the editor.
    dred_control_set_size(pTextBox, newWidth, newHeight);
}

void dred_settings_editor__on_capture_keyboard(dred_settings_editor* pSettingsEditor, drgui_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;
    
    // When a text editor receives keyboard focus it should be routed down to the text box control.
    dred_textbox2* pTextBox = dred_settings_editor__get_textbox(pSettingsEditor);
    if (pTextBox == NULL) {
        return;
    }

    dred_capture_keyboard(dred_control_get_context(pTextBox), pTextBox);
}

dred_settings_editor* dred_settings_editor_create(dred_context* pDred, dred_control* pParent, const char* filePathAbsolute)
{
    dred_settings_editor* pSettingsEditor = dred_editor_create(pDred, pParent, DRED_CONTROL_TYPE_SETTINGS_EDITOR, filePathAbsolute, sizeof(dred_settings_editor_data));
    if (pSettingsEditor == NULL) {
        return NULL;
    }

    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    pData->pTextBox = dred_textbox2_create(pDred, pSettingsEditor);


    // Events.
    dred_control_set_on_size(pSettingsEditor, dred_settings_editor__on_size);
    dred_control_set_on_capture_keyboard(pSettingsEditor, dred_settings_editor__on_capture_keyboard);


    return pSettingsEditor;
}

void dred_settings_editor_delete(dred_settings_editor* pSettingsEditor)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    if (pData != NULL) {
        dred_textbox2_delete(pData->pTextBox);
    }

    dred_editor_delete(pSettingsEditor);
}
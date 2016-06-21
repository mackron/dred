
typedef struct
{
    dred_control* pFontButton;
} dred_settings_editor_data;

void dred_settings_editor__on_size(dred_settings_editor* pSettingsEditor, float newWidth, float newHeight)
{
    (void)pSettingsEditor;
    (void)newWidth;
    (void)newHeight;

    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    (void)pData;
}

void dred_settings_editor__on_capture_keyboard(dred_settings_editor* pSettingsEditor, drgui_element* pPrevCapturedElement)
{
    (void)pSettingsEditor;
    (void)pPrevCapturedElement;
}

void dred_settings__btn_choose_font__on_pressed(dred_button* pButton)
{
    dred_window* pWindow = dred_get_element_window(pButton);
    if (pWindow == NULL) {
        return; // Should never hit this, but leaving here for sanity.
    }

    dred_context* pDred = pWindow->pDred;

    dred_font_desc fontDesc;
    if (!dred_show_font_picker_dialog(pDred, pWindow, &pDred->config.pTextEditorFont->desc, &fontDesc)) {
        return;
    }

    char fontDescStr[256];
    dred_font_desc_to_string(&fontDesc, fontDescStr, sizeof(fontDescStr));
    
    dred_config_set(&pDred->config, "texteditor-font", fontDescStr);
}

dred_settings_editor* dred_settings_editor_create(dred_context* pDred, dred_control* pParent, const char* filePathAbsolute)
{
    dred_settings_editor* pSettingsEditor = dred_editor_create(pDred, pParent, DRED_CONTROL_TYPE_SETTINGS_EDITOR, filePathAbsolute, sizeof(dred_settings_editor_data));
    if (pSettingsEditor == NULL) {
        return NULL;
    }

    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    pData->pFontButton = dred_button_create(pDred, pSettingsEditor, "Choose Font...");
    if (pData->pFontButton == NULL) {
        dred_editor_delete(pSettingsEditor);
        return NULL;
    }

    dred_button_set_on_pressed(pData->pFontButton, dred_settings__btn_choose_font__on_pressed);
    dred_control_set_relative_position(pData->pFontButton, 8, 8);
    dred_control_set_size(pData->pFontButton, 128, 32);






    // Events.
    dred_control_set_on_size(pSettingsEditor, dred_settings_editor__on_size);
    dred_control_set_on_capture_keyboard(pSettingsEditor, dred_settings_editor__on_capture_keyboard);


    return pSettingsEditor;
}

void dred_settings_editor_delete(dred_settings_editor* pSettingsEditor)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    if (pData != NULL) {
    }

    dred_editor_delete(pSettingsEditor);
}


void dred_settings_editor_refresh_styling(dred_settings_editor* pSettingsEditor)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    if (pData == NULL) {
        return;
    }


}
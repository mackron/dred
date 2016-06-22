
typedef struct
{
    dred_control* pCloseButton;
    dred_control* pFontButton;
    dred_control* pTextColorButton;
    dred_control* pBGColorButton;
    dred_control* pLineColorButton;

    dred_checkbox* pCBTabsToSpaces;
    dred_checkbox* pCBShowTabBar;
} dred_settings_editor_data;

void dred_settings_editor__on_size(dred_settings_editor* pSettingsEditor, float newWidth, float newHeight)
{
    (void)pSettingsEditor;
    (void)newWidth;
    (void)newHeight;

    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    dred_context* pDred = dred_control_get_context(pSettingsEditor);

    dred_control_set_relative_position(pData->pCloseButton,
        dred_control_get_width(pSettingsEditor) - dred_control_get_width(pData->pCloseButton) - 8*pDred->uiScale,
        dred_control_get_height(pSettingsEditor) - dred_control_get_height(pData->pCloseButton) - 8*pDred->uiScale);
}

void dred_settings_editor__on_capture_keyboard(dred_settings_editor* pSettingsEditor, drgui_element* pPrevCapturedElement)
{
    (void)pSettingsEditor;
    (void)pPrevCapturedElement;
}

void dred_settings__btn_close__on_pressed(dred_button* pButton)
{
    dred_settings_dialog_hide(dred_control_get_context(pButton)->pSettingsDialog);
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

    pData->pCloseButton = dred_button_create(pDred, pSettingsEditor, "Close");
    if (pData->pCloseButton == NULL) {
        dred_editor_delete(pSettingsEditor);
        return NULL;
    }

    dred_button_set_on_pressed(pData->pCloseButton, dred_settings__btn_close__on_pressed);
    dred_button_set_padding(pData->pCloseButton, 32*pDred->uiScale, 6*pDred->uiScale);
    dred_control_set_relative_position(pData->pCloseButton,
        dred_control_get_width(pSettingsEditor) - dred_control_get_width(pData->pCloseButton) - 8*pDred->uiScale,
        dred_control_get_height(pSettingsEditor) - dred_control_get_height(pData->pCloseButton) - 8*pDred->uiScale);



    pData->pFontButton = dred_button_create(pDred, pSettingsEditor, "Choose Font...");
    if (pData->pFontButton == NULL) {
        dred_editor_delete(pSettingsEditor);
        return NULL;
    }

    dred_button_set_on_pressed(pData->pFontButton, dred_settings__btn_choose_font__on_pressed);
    dred_button_set_padding(pData->pFontButton, 16*pDred->uiScale, 6*pDred->uiScale);
    dred_control_set_relative_position(pData->pFontButton, 256*pDred->uiScale, 8*pDred->uiScale);


    pData->pTextColorButton = dred_colorbutton_create(pDred, pSettingsEditor, "Text color", pDred->config.textEditorTextColor);
    if (pData->pTextColorButton == NULL) {
        dred_editor_delete(pSettingsEditor);
        return NULL;
    }

    dred_colorbutton_set_bind_to_config_var(pData->pTextColorButton, "texteditor-text-color");
    dred_colorbutton_set_padding(pData->pTextColorButton, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pTextColorButton, 256*pDred->uiScale, (8+32)*pDred->uiScale);



    pData->pBGColorButton = dred_colorbutton_create(pDred, pSettingsEditor, "Background color", pDred->config.textEditorBGColor);
    if (pData->pBGColorButton == NULL) {
        dred_editor_delete(pSettingsEditor);
        return NULL;
    }

    dred_colorbutton_set_bind_to_config_var(pData->pBGColorButton, "texteditor-bg-color");
    dred_colorbutton_set_padding(pData->pBGColorButton, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pBGColorButton, 256*pDred->uiScale, (8+64)*pDred->uiScale);


    pData->pLineColorButton = dred_colorbutton_create(pDred, pSettingsEditor, "Active line color", pDred->config.textEditorActiveLineColor);
    if (pData->pLineColorButton == NULL) {
        dred_editor_delete(pSettingsEditor);
        return NULL;
    }

    dred_colorbutton_set_bind_to_config_var(pData->pLineColorButton, "texteditor-active-line-color");
    dred_colorbutton_set_padding(pData->pLineColorButton, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pLineColorButton, 256*pDred->uiScale, (8+96)*pDred->uiScale);


    pData->pCBTabsToSpaces = dred_checkbox_create(pDred, pSettingsEditor, "Convert tabs to spaces", pDred->config.textEditorTabsToSpacesEnabled);
    dred_checkbox_set_bind_to_config_var(pData->pCBTabsToSpaces, "texteditor-enable-tabs-to-spaces");
    dred_checkbox_set_padding(pData->pCBTabsToSpaces, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pCBTabsToSpaces, 256, 200);

    pData->pCBShowTabBar = dred_checkbox_create(pDred, pSettingsEditor, "Show Tab Bar", pDred->config.showTabBar);
    dred_checkbox_set_bind_to_config_var(pData->pCBShowTabBar, "show-tab-bar");
    dred_checkbox_set_padding(pData->pCBShowTabBar, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pCBShowTabBar, 256, 232);


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

void dred_settings_dialog__on_window_close(dred_window* pWindow)
{
    // Just hide the window. The main context will delete it for real at close time.
    dred_window_hide(pWindow, 0);
}

dred_settings_dialog* dred_settings_dialog_create(dred_context* pDred)
{
    if (pDred == NULL) {
        return NULL;
    }

    dred_settings_dialog* pDialog = (dred_settings_dialog*)calloc(1, sizeof(*pDialog));
    if (pDialog == NULL) {
        return NULL;
    }

    pDialog->pWindow = dred_window_create_dialog(pDred->pMainWindow, "Settings", (unsigned int)(720*pDred->dpiScale), (unsigned int)(480*pDred->dpiScale));
    if (pDialog->pWindow == NULL) {
        free(pDialog);
        return NULL;
    }
    
    pDialog->pWindow->pUserData = pDialog;
    pDialog->pWindow->onClose = dred_settings_dialog__on_window_close;
    pDialog->pWindow->pRootGUIElement->onSize = drgui_on_size_fit_children_to_parent;


    pDialog->pSettingsEditor = dred_settings_editor_create(pDred, pDialog->pWindow->pRootGUIElement, NULL);
    if (pDialog->pSettingsEditor == NULL) {
        dred_window_delete(pDialog->pWindow);
        free(pDialog);
    }

    unsigned int windowSizeX;
    unsigned int windowSizeY;
    dred_window_get_client_size(pDialog->pWindow, &windowSizeX, &windowSizeY);
    dred_control_set_size(pDialog->pSettingsEditor, (float)windowSizeX, (float)windowSizeY);


    return pDialog;
}

void dred_settings_dialog_delete(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_window_delete(pDialog->pWindow);
    free(pDialog);
}


void dred_settings_dialog_show(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_window_move_to_center(pDialog->pWindow);
    dred_window_show(pDialog->pWindow);
}

void dred_settings_dialog_hide(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_window_hide(pDialog->pWindow, 0);
}

void dred_settings_dialog_refresh_styling(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_settings_editor_refresh_styling(pDialog->pSettingsEditor);
}
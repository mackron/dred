// Copyright (C) 2016 David Reid. See included LICENSE file.

void dred_settings_dialog__on_window_close(dred_window* pWindow)
{
    // Just hide the window. The main context will delete it for real at close time.
    dred_settings_dialog* pDialog = (dred_settings_dialog*)pWindow->pUserData;
    dred_settings_dialog_hide(pDialog);
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

    pDialog->pWindow = dred_window_create_dialog(pDred->pMainWindow, "Settings", (unsigned int)(720*pDred->uiScale), (unsigned int)(480*pDred->uiScale));
    if (pDialog->pWindow == NULL) {
        free(pDialog);
        return NULL;
    }
    
    pDialog->pWindow->pUserData = pDialog;
    pDialog->pWindow->onClose = dred_settings_dialog__on_window_close;
    pDialog->pWindow->pRootGUIControl->onSize = dred_control_on_size_fit_children_to_parent;


    pDialog->pSettingsEditor = dred_settings_editor_create(pDred, pDialog->pWindow->pRootGUIControl, NULL);
    if (pDialog->pSettingsEditor == NULL) {
        dred_window_delete(pDialog->pWindow);
        free(pDialog);
		return NULL;
    }

    unsigned int windowSizeX;
    unsigned int windowSizeY;
    dred_window_get_client_size(pDialog->pWindow, &windowSizeX, &windowSizeY);
    dred_control_set_size(DRED_CONTROL(pDialog->pSettingsEditor), (float)windowSizeX, (float)windowSizeY);


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
    pDialog->isShowing = true;
}

void dred_settings_dialog_hide(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_window_hide(pDialog->pWindow, 0);
    pDialog->isShowing = false;
}

bool dred_settings_dialog_is_showing(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return false;
    }

    return pDialog->isShowing;
}

void dred_settings_dialog_refresh_styling(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_settings_editor_refresh_styling(pDialog->pSettingsEditor);
}
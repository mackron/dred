// Copyright (C) 2017 David Reid. See included LICENSE file.

#define DRED_SETTINGS_DIALOG_BASE_SIZE_X    720
#define DRED_SETTINGS_DIALOG_BASE_SIZE_Y    480

static dtk_bool32 dred_settings_dialog_event_handler(dtk_event* pEvent)
{
    dred_settings_dialog* pDialog = (dred_settings_dialog*)pEvent->pControl;

    switch (pEvent->type)
    {
        case DTK_EVENT_DPI_CHANGED:
        {
            dtk_control_refresh_layout(pEvent->pControl);
        } break;

        case DTK_EVENT_REFRESH_LAYOUT:
        {
            dred_settings_dialog_refresh_styling(pDialog);
        } break;

        case DTK_EVENT_CLOSE:
        {
            // Just hide the window. The main context will delete it for real at close time.
            dred_settings_dialog_hide(pDialog);
        } return DTK_FALSE;

        case DTK_EVENT_SIZE:
        {
            dtk_control_set_size(DTK_CONTROL(pDialog->pSettingsEditor), pEvent->size.width, pEvent->size.height);
        } break;
    }

    return dred_dtk_window_event_handler(pEvent);
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

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(&pDred->mainWindow));

    dtk_uint32 sizeX = (unsigned int)(DRED_SETTINGS_DIALOG_BASE_SIZE_X*uiScale);
    dtk_uint32 sizeY = (unsigned int)(DRED_SETTINGS_DIALOG_BASE_SIZE_Y*uiScale);
    dtk_result result = dtk_window_init(&pDred->tk, dred_settings_dialog_event_handler, DTK_CONTROL(&pDred->mainWindow), dtk_window_type_dialog, "Settings", sizeX, sizeY, &pDialog->window);
    if (result != DTK_SUCCESS) {
        free(pDialog);
        return NULL;
    }

    pDialog->pSettingsEditor = dred_settings_editor_create(pDred, DTK_CONTROL(pDialog), NULL);
    if (pDialog->pSettingsEditor == NULL) {
        dtk_window_uninit(DTK_WINDOW(pDialog));
        free(pDialog);
		return NULL;
    }

    dtk_int32 clientSizeX;
    dtk_int32 clientSizeY;
    dtk_window_get_client_size(DTK_WINDOW(pDialog), &clientSizeX, &clientSizeY);
    dtk_control_set_size(DTK_CONTROL(pDialog->pSettingsEditor), clientSizeX, clientSizeY);


    return pDialog;
}

void dred_settings_dialog_delete(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_settings_editor_delete(pDialog->pSettingsEditor);

    dtk_window_uninit(DTK_WINDOW(pDialog));
    free(pDialog);
}


void dred_settings_dialog_show(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dtk_window_move_to_center(DTK_WINDOW(pDialog));
    dtk_window_show(DTK_WINDOW(pDialog), DTK_SHOW_NORMAL);
}

void dred_settings_dialog_hide(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dtk_window_hide(DTK_WINDOW(pDialog));
}

dr_bool32 dred_settings_dialog_is_showing(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return DR_FALSE;
    }

    return dtk_control_is_visible(DTK_CONTROL(pDialog));
}

void dred_settings_dialog_refresh_styling(dred_settings_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pDialog));
    dtk_uint32 windowWidth  = (dtk_uint32)(DRED_SETTINGS_DIALOG_BASE_SIZE_X*uiScale);
    dtk_uint32 windowHeight = (dtk_uint32)(DRED_SETTINGS_DIALOG_BASE_SIZE_Y*uiScale);
    dtk_window_set_size(DTK_WINDOW(pDialog), windowWidth, windowHeight);
    dtk_window_move_to_center(DTK_WINDOW(pDialog));

    dred_settings_editor_refresh_styling(pDialog->pSettingsEditor);
}
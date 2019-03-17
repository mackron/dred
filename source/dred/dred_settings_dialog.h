// Copyright (C) 2019 David Reid. See included LICENSE file.

typedef struct
{
    dtk_window window;
    dred_settings_editor* pSettingsEditor;
} dred_settings_dialog;

dtk_result dred_settings_dialog_init(dred_context* pDred, dred_settings_dialog* pDialog);
dtk_result dred_settings_dialog_uninit(dred_settings_dialog* pDialog);

void dred_settings_dialog_show(dred_settings_dialog* pDialog);
void dred_settings_dialog_hide(dred_settings_dialog* pDialog);
dtk_bool32 dred_settings_dialog_is_showing(dred_settings_dialog* pDialog);

void dred_settings_dialog_refresh_styling(dred_settings_dialog* pDialog);
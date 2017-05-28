// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    dtk_window window;
    dred_settings_editor* pSettingsEditor;
} dred_settings_dialog;

dred_settings_dialog* dred_settings_dialog_create(dred_context* pDred);
void dred_settings_dialog_delete(dred_settings_dialog* pDialog);

void dred_settings_dialog_show(dred_settings_dialog* pDialog);
void dred_settings_dialog_hide(dred_settings_dialog* pDialog);
dr_bool32 dred_settings_dialog_is_showing(dred_settings_dialog* pDialog);

void dred_settings_dialog_refresh_styling(dred_settings_dialog* pDialog);
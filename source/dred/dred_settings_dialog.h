// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef struct
{
    dred_window* pWindow;
    dred_settings_editor* pSettingsEditor;
    drBool32 isShowing;
} dred_settings_dialog;

dred_settings_dialog* dred_settings_dialog_create(dred_context* pDred);
void dred_settings_dialog_delete(dred_settings_dialog* pDialog);

void dred_settings_dialog_show(dred_settings_dialog* pDialog);
void dred_settings_dialog_hide(dred_settings_dialog* pDialog);
drBool32 dred_settings_dialog_is_showing(dred_settings_dialog* pDialog);

void dred_settings_dialog_refresh_styling(dred_settings_dialog* pDialog);
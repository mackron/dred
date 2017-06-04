// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    dtk_window window;
    dred_gui_image* pLogo;
    dtk_button closeButton;
} dred_about_dialog;

dtk_result dred_about_dialog_init(dred_context* pDred, dred_about_dialog* pDialog);
dtk_result dred_about_dialog_uninit(dred_about_dialog* pDialog);

void dred_about_dialog_show(dred_about_dialog* pDialog);
void dred_about_dialog_hide(dred_about_dialog* pDialog);
dr_bool32 dred_about_dialog_is_showing(dred_about_dialog* pDialog);
void dred_about_dialog_refresh_inner_layout(dred_about_dialog* pDialog);
void dred_about_dialog_refresh_layout(dred_about_dialog* pDialog);

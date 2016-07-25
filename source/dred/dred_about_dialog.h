// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef struct
{
    dred_window* pWindow;
    dred_gui_image* pLogo;
    dred_button* pCloseButton;
    bool isShowing;
} dred_about_dialog;

dred_about_dialog* dred_about_dialog_create(dred_context* pDred);
void dred_about_dialog_delete(dred_about_dialog* pDialog);

void dred_about_dialog_show(dred_about_dialog* pDialog);
void dred_about_dialog_hide(dred_about_dialog* pDialog);
bool dred_about_dialog_is_showing(dred_about_dialog* pDialog);
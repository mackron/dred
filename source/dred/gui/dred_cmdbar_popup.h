// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef struct
{
    dred_context* pDred;
    dred_window* pWindow;
} dred_cmdbar_popup;

dred_cmdbar_popup* dred_cmdbar_popup_create(dred_context* pDred);
void dred_cmdbar_popup_delete(dred_cmdbar_popup* pCmdBarPopup);

void dred_cmdbar_popup_show(dred_cmdbar_popup* pCmdBarPopup);
void dred_cmdbar_popup_hide(dred_cmdbar_popup* pCmdBarPopup);
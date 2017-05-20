// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    dred_context* pDred;
    dred_window* pWindow;
    size_t* pCommandIndices;
    size_t commandIndexCount;
    size_t commandIndexCapacity;
    dred_gui_font* pFont;
    dr_bool32 showOnlyFirstCommand : 1;

    dred_cmdbox_cmdlist cmdlist;
} dred_cmdbar_popup;

dred_cmdbar_popup* dred_cmdbar_popup_create(dred_context* pDred);
void dred_cmdbar_popup_delete(dred_cmdbar_popup* pCmdBarPopup);

void dred_cmdbar_popup_show(dred_cmdbar_popup* pCmdBarPopup);
void dred_cmdbar_popup_hide(dred_cmdbar_popup* pCmdBarPopup);
void dred_cmdbar_popup_refresh_position(dred_cmdbar_popup* pCmdBarPopup);
void dred_cmdbar_popup_refresh_styling(dred_cmdbar_popup* pCmdBarPopup);
void dred_cmdbar_popup_refresh_autocomplete(dred_cmdbar_popup* pCmdBarPopup, const char* runningText);
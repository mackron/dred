// Copyright (C) 2019 David Reid. See included LICENSE file.

#define DRED_CMDBAR_POPUP(p) ((dred_cmdbar_popup*)(p))
typedef struct
{
    dtk_window window;
    size_t* pCommandIndices;
    size_t commandIndexCount;
    size_t commandIndexCapacity;
    dtk_font* pFont;
    dtk_bool32 showOnlyFirstCommand : 1;

    dred_cmdbox_cmdlist cmdlist;
} dred_cmdbar_popup;

dtk_result dred_cmdbar_popup_init(dred_context* pDred, dred_cmdbar_popup* pCmdBarPopup);
dtk_result dred_cmdbar_popup_uninit(dred_cmdbar_popup* pCmdBarPopup);

void dred_cmdbar_popup_show(dred_cmdbar_popup* pCmdBarPopup);
void dred_cmdbar_popup_hide(dred_cmdbar_popup* pCmdBarPopup);
void dred_cmdbar_popup_refresh_position(dred_cmdbar_popup* pCmdBarPopup);
void dred_cmdbar_popup_refresh_styling(dred_cmdbar_popup* pCmdBarPopup);
void dred_cmdbar_popup_refresh_autocomplete(dred_cmdbar_popup* pCmdBarPopup, const char* runningText);
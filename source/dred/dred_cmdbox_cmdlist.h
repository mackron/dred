// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    dtk_control control;
    dred_context* pDred;
    dtk_scrollbar scrollbar;   // Always created, but dynamically shown and hidden depending on the list.
    char* runningText;          // Used to determine which commands should be listed.
    dtk_uint32 selectedItemIndex;

    dtk_uint32* pCommandIndices;
    dtk_uint32 commandIndexCount;
    dtk_uint32 commandIndexCapacity;
    dtk_bool32 showOnlyFirstCommand : 1;
} dred_cmdbox_cmdlist;

dred_result dred_cmdbox_cmdlist_init(dred_context* pDred, dtk_control* pParent, dred_cmdbox_cmdlist* pCmdList);
dred_result dred_cmdbox_cmdlist_uninit(dred_cmdbox_cmdlist* pCmdList);
dred_result dred_cmdbox_cmdlist_update_list(dred_cmdbox_cmdlist* pCmdList, const char* runningText);
dred_result dred_cmdbox_cmdlist_highlight_next_item(dred_cmdbox_cmdlist* pCmdList);
dred_result dred_cmdbox_cmdlist_highlight_prev_item(dred_cmdbox_cmdlist* pCmdList);
const char* dred_cmdbox_cmdlist_get_highlighted_command_name(dred_cmdbox_cmdlist* pCmdList);
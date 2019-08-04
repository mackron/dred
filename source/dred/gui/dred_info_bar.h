// Copyright (C) 2019 David Reid. See included LICENSE file.

// The info bar contains information about a specific type of control. It is contextual depending on the type.

typedef struct dred_info_bar dred_info_bar;
#define DRED_INFO_BAR(a) ((dred_info_bar*)(a))

struct dred_info_bar
{
    dtk_control control;

    dtk_font* pFont;
    int type;
    char lineStr[32];
    char colStr[32];
    char zoomStr[32];
};

// dred_info_bar_create()
dtk_bool32 dred_info_bar_init(dred_context* pDred, dtk_control* pParent, dred_info_bar* pInfoBar);

// dred_info_bar_delete()
void dred_info_bar_uninit(dred_info_bar* pInfoBar);

// Updates the info bar based on the given control.
void dred_info_bar_update(dred_info_bar* pInfoBar, dtk_control* pControl);

// Refreshes the styling of the info bar.
void dred_info_bar_refresh_styling(dred_info_bar* pInfoBar);
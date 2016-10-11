// Copyright (C) 2016 David Reid. See included LICENSE file.

// The info bar contains information about a specific type of control. It is contextual depending on the type.

#define DRED_CONTROL_TYPE_INFO_BAR   "dred.infobar"

typedef struct dred_info_bar dred_info_bar;
#define DRED_INFO_BAR(a) ((dred_info_bar*)(a))

struct dred_info_bar
{
    // The base control.
    dred_control control;

    dred_gui_font* pFont;
    int type;
    char lineStr[32];
    char colStr[32];
    char zoomStr[32];
};

// dred_info_bar_create()
dr_bool32 dred_info_bar_init(dred_info_bar* pInfoBar, dred_context* pDred, dred_control* pParent);

// dred_info_bar_delete()
void dred_info_bar_uninit(dred_info_bar* pInfoBar);

// Updates the info bar based on the given control.
void dred_info_bar_update(dred_info_bar* pInfoBar, dred_control* pControl);

// Refreshes the styling of the info bar.
void dred_info_bar_refresh_styling(dred_info_bar* pInfoBar);
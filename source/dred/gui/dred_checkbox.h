// Copyright (C) 2016 David Reid. See included LICENSE file.

// The checkbox control is a box with a check mark and a label. The size of the box is based on the size of the font.

#define DRED_CONTROL_TYPE_CHECKBOX   "dred.common.checkbox"

typedef struct dred_checkbox dred_checkbox;
#define DRED_CHECKBOX(a) ((dred_checkbox*)(a))

typedef void (* dred_checkbox_on_checked_changed_proc)(dred_checkbox* pCheckbox);

struct dred_checkbox
{
    // The base control.
    dred_control control;

    char text[64];
    dred_font* pFont;
    dred_gui_font* pSubFont;
    dred_color textColor;
    dred_color bgColor;
    dred_color boxBGColor;
    dred_color boxBGColorHovered;
    dred_color boxBGColorPressed;
    dred_color boxBorderColor;
    dred_color checkColor;
    float borderWidth;
    float padding;
    dr_bool32 isMouseOver;
    dr_bool32 isAutoSizeEnabled;
    dr_bool32 isChecked;
    char varBinding[128];
    dred_checkbox_on_checked_changed_proc onCheckChanged;
};

dr_bool32 dred_checkbox_init(dred_checkbox* pCheckbox, dred_context* pDred, dred_control* pParent, const char* text, dr_bool32 checked);
void dred_checkbox_uninit(dred_checkbox* pCheckbox);

void dred_checkbox_set_text(dred_checkbox* pCheckbox, const char* text);
void dred_checkbox_enable_auto_size(dred_checkbox* pCheckbox);
void dred_checkbox_disable_auto_size(dred_checkbox* pCheckbox);

void dred_checkbox_set_font(dred_checkbox* pCheckbox, dred_font* pFont);
void dred_checkbox_set_background_color(dred_checkbox* pCheckbox, dred_color color);
void dred_checkbox_set_border_color(dred_checkbox* pCheckbox, dred_color color);
void dred_checkbox_set_border_width(dred_checkbox* pCheckbox, float width);
void dred_checkbox_set_padding(dred_checkbox* pCheckbox, float padding);

void dred_checkbox_check(dred_checkbox* pCheckbox);
void dred_checkbox_uncheck(dred_checkbox* pCheckbox);
void dred_checkbox_toggle(dred_checkbox* pCheckbox);
void dred_checkbox_set_checked(dred_checkbox* pCheckbox, dr_bool32 checked, dr_bool32 blockEvent);
dr_bool32 dred_is_checked(dred_checkbox* pCheckbox);

void dred_checkbox_set_bind_to_config_var(dred_checkbox* pCheckbox, const char* varName);
void dred_checkbox_set_on_checked_changed(dred_checkbox* pCheckbox, dred_checkbox_on_checked_changed_proc proc);
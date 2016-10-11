// Copyright (C) 2016 David Reid. See included LICENSE file.

// The checkbox control is a box with a check mark and a label. The size of the box is based on the size of the font.

#define DRED_CONTROL_TYPE_COLOR_BUTTON   "dred.common.colorbutton"

typedef struct dred_colorbutton dred_colorbutton;
#define DRED_COLOR_BUTTON(a) ((dred_colorbutton*)(a))

typedef void (* dred_colorbutton_on_color_changed_proc)(dred_colorbutton* pButton, dred_color color);

struct dred_colorbutton
{
    // The base control.
    dred_control control;

    char text[64];
    dred_font* pFont;
    dred_gui_font* pSubFont;
    dred_color textColor;
    dred_color bgColor;
    dred_color bgColorHovered;
    dred_color bgColorPressed;
    dred_color boxBorderColor;
    dred_color color;
    float borderWidth;
    float padding;
    dr_bool32 isMouseOver;
    dr_bool32 isAutoSizeEnabled;
    char varBinding[128];
    dred_colorbutton_on_color_changed_proc onColorChanged;
};


dr_bool32 dred_colorbutton_init(dred_colorbutton* pButton, dred_context* pDred, dred_control* pParent, const char* text, dred_color color);
void dred_colorbutton_uninit(dred_colorbutton* pButton);

void dred_colorbutton_set_text(dred_colorbutton* pButton, const char* text);
void dred_colorbutton_enable_auto_size(dred_colorbutton* pButton);
void dred_colorbutton_disable_auto_size(dred_colorbutton* pButton);

void dred_colorbutton_set_font(dred_colorbutton* pButton, dred_font* pFont);
void dred_colorbutton_set_background_color(dred_colorbutton* pButton, dred_color color);
void dred_colorbutton_set_border_color(dred_colorbutton* pButton, dred_color color);
void dred_colorbutton_set_border_width(dred_colorbutton* pButton, float width);
void dred_colorbutton_set_padding(dred_colorbutton* pButton, float padding);

void dred_colorbutton_set_color(dred_colorbutton* pButton, dred_color color, dr_bool32 blockEvent);
dred_color dred_colorbutton_get_color(dred_colorbutton* pbutton);

void dred_colorbutton_set_bind_to_config_var(dred_colorbutton* pButton, const char* varName);
void dred_colorbutton_set_on_color_changed(dred_colorbutton* pButton, dred_colorbutton_on_color_changed_proc proc);
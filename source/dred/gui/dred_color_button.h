// Copyright (C) 2017 David Reid. See included LICENSE file.

// The checkbox control is a box with a check mark and a label. The size of the box is based on the size of the font.

#define DRED_CONTROL_TYPE_COLOR_BUTTON   "dred.common.colorbutton"

typedef struct dtk_colorbutton dtk_colorbutton;
#define DRED_COLOR_BUTTON(a) ((dtk_colorbutton*)(a))

typedef void (* dtk_colorbutton_on_color_changed_proc)(dtk_colorbutton* pButton, dtk_color color);

struct dtk_colorbutton
{
    // The base control.
    dred_control control;

    char text[64];
    dred_font* pFont;
    dred_gui_font* pSubFont;
    dtk_color textColor;
    dtk_color bgColor;
    dtk_color bgColorHovered;
    dtk_color bgColorPressed;
    dtk_color boxBorderColor;
    dtk_color color;
    float borderWidth;
    float padding;
    dr_bool32 isMouseOver;
    dr_bool32 isAutoSizeEnabled;
    char varBinding[128];
    dtk_colorbutton_on_color_changed_proc onColorChanged;
};


dr_bool32 dtk_colorbutton_init(dtk_colorbutton* pButton, dred_context* pDred, dred_control* pParent, const char* text, dtk_color color);
void dtk_colorbutton_uninit(dtk_colorbutton* pButton);

void dtk_colorbutton_set_text(dtk_colorbutton* pButton, const char* text);
void dtk_colorbutton_enable_auto_size(dtk_colorbutton* pButton);
void dtk_colorbutton_disable_auto_size(dtk_colorbutton* pButton);

void dtk_colorbutton_set_font(dtk_colorbutton* pButton, dred_font* pFont);
void dtk_colorbutton_set_background_color(dtk_colorbutton* pButton, dtk_color color);
void dtk_colorbutton_set_border_color(dtk_colorbutton* pButton, dtk_color color);
void dtk_colorbutton_set_border_width(dtk_colorbutton* pButton, float width);
void dtk_colorbutton_set_padding(dtk_colorbutton* pButton, float padding);

void dtk_colorbutton_set_color(dtk_colorbutton* pButton, dtk_color color, dr_bool32 blockEvent);
dtk_color dtk_colorbutton_get_color(dtk_colorbutton* pbutton);

void dtk_colorbutton_set_bind_to_config_var(dtk_colorbutton* pButton, const char* varName);
void dtk_colorbutton_set_on_color_changed(dtk_colorbutton* pButton, dtk_colorbutton_on_color_changed_proc proc);

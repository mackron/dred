// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_BUTTON   "dred.common.button"

typedef struct dred_button dred_button;
#define DRED_BUTTON(a) ((dred_button*)(a))

typedef void (* dred_button_on_pressed_proc)(dred_button* pButton);

struct dred_button
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
    dred_color borderColor;
    float borderWidth;
    float paddingHorz;
    float paddingVert;
    bool isMouseOver;
    bool isAutoSizeEnabled;
    dred_button_on_pressed_proc onPressed;
};

bool dred_button_init(dred_button* pButton, dred_context* pDred, dred_control* pParent, const char* text);
void dred_button_uninit(dred_button* pButton);

void dred_button_set_text(dred_button* pButton, const char* text);
void dred_button_enable_auto_size(dred_button* pButton);
void dred_button_disable_auto_size(dred_button* pButton);

void dred_button_set_font(dred_button* pButton, dred_font* pFont);
void dred_button_set_background_color(dred_button* pButton, dred_color color);
void dred_button_set_border_color(dred_button* pButton, dred_color color);
void dred_button_set_border_width(dred_button* pButton, float width);
void dred_button_set_padding(dred_button* pButton, float paddingHorz, float paddingVert);

void dred_button_set_on_pressed(dred_button* pButton, dred_button_on_pressed_proc proc);
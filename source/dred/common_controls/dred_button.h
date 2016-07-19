// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_BUTTON   "dred.common.button"

typedef dred_control dred_button;
typedef void (* dred_button_on_pressed_proc)(dred_button* pButton);

dred_button* dred_button_create(dred_context* pDred, dred_control* pParent, const char* text);
void dred_button_delete(dred_button* pButton);

void dred_button_set_text(dred_button* pButton, const char* text);
void dred_button_enable_auto_size(dred_button* pButton);
void dred_button_disable_auto_size(dred_button* pButton);

void dred_button_set_font(dred_button* pButton, dred_font* pFont);
void dred_button_set_background_color(dred_button* pButton, drgui_color color);
void dred_button_set_border_color(dred_button* pButton, drgui_color color);
void dred_button_set_border_width(dred_button* pButton, float width);
void dred_button_set_padding(dred_button* pButton, float paddingHorz, float paddingVert);

void dred_button_set_on_pressed(dred_button* pButton, dred_button_on_pressed_proc proc);
// Copyright (C) 2019 David Reid. See included LICENSE file.

typedef struct dtk_button dtk_button;
typedef void (* dtk_button_on_pressed_proc)(dtk_button* pButton);

#define DTK_BUTTON(a) ((dtk_button*)(a))
struct dtk_button
{
    dtk_control control;
    char text[64];
    dtk_font* pFont;
    dtk_color textColor;
    dtk_color bgColor;
    dtk_color bgColorHovered;
    dtk_color bgColorPressed;
    dtk_color borderColor;
    dtk_int32 borderWidth;
    dtk_int32 paddingHorz;
    dtk_int32 paddingVert;
    dtk_bool32 isMouseOver       : 1;
    dtk_bool32 isAutoSizeEnabled : 1;
    dtk_button_on_pressed_proc onPressed;
};

dtk_result dtk_button_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, const char* text, dtk_button* pButton);
void dtk_button_uninit(dtk_button* pButton);
dtk_bool32 dtk_button_default_event_handler(dtk_event* pEvent);

void dtk_button_set_text(dtk_button* pButton, const char* text);

// When auto-sizing is enabled, you can trigger a resize manually with dtk_control_refresh_layout().
void dtk_button_enable_auto_size(dtk_button* pButton);
void dtk_button_disable_auto_size(dtk_button* pButton);

void dtk_button_set_font(dtk_button* pButton, dtk_font* pFont);
void dtk_button_set_background_color(dtk_button* pButton, dtk_color color);
void dtk_button_set_border_color(dtk_button* pButton, dtk_color color);
void dtk_button_set_border_width(dtk_button* pButton, dtk_int32 width);
void dtk_button_set_padding(dtk_button* pButton, dtk_int32 paddingHorz, dtk_int32 paddingVert);

void dtk_button_set_on_pressed(dtk_button* pButton, dtk_button_on_pressed_proc proc);
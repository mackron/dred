// Copyright (C) 2019 David Reid. See included LICENSE file.

typedef struct dtk_color_button dtk_color_button;
typedef void (* dtk_color_button_on_color_changed_proc)(dtk_color_button* pButton, dtk_color color);

#define DTK_COLOR_BUTTON(a) ((dtk_color_button*)(a))
struct dtk_color_button
{
    dtk_control control;
    char text[64];
    dtk_font* pFont;
    dtk_color textColor;
    dtk_color bgColor;
    dtk_color bgColorHovered;
    dtk_color bgColorPressed;
    dtk_color boxBorderColor;
    dtk_color color;
    dtk_int32 borderWidth;
    dtk_int32 padding;
    dtk_bool32 isMouseOver;
    dtk_bool32 isAutoSizeEnabled;
    dtk_color_button_on_color_changed_proc onColorChanged;
};

dtk_result dtk_color_button_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, const char* text, dtk_color color, dtk_color_button* pButton);
dtk_result dtk_color_button_uninit(dtk_color_button* pColorButton);
dtk_bool32 dtk_color_button_default_event_handler(dtk_event* pEvent);

void dtk_color_button_set_text(dtk_color_button* pButton, const char* text);
void dtk_color_button_enable_auto_size(dtk_color_button* pButton);
void dtk_color_button_disable_auto_size(dtk_color_button* pButton);
void dtk_color_button_auto_size(dtk_color_button* pButton);

void dtk_color_button_set_font(dtk_color_button* pButton, dtk_font* pFont);
void dtk_color_button_set_background_color(dtk_color_button* pButton, dtk_color color);
void dtk_color_button_set_border_color(dtk_color_button* pButton, dtk_color color);
void dtk_color_button_set_border_width(dtk_color_button* pButton, dtk_int32 width);
void dtk_color_button_set_padding(dtk_color_button* pButton, dtk_int32 padding);

void dtk_color_button_set_color(dtk_color_button* pButton, dtk_color color, dtk_bool32 blockEvent);
dtk_color dtk_color_button_get_color(dtk_color_button* pbutton);

void dtk_color_button_set_on_color_changed(dtk_color_button* pButton, dtk_color_button_on_color_changed_proc proc);
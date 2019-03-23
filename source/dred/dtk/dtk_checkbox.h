// Copyright (C) 2019 David Reid. See included LICENSE file.

#define DTK_CHECKBOX(a) ((dtk_checkbox*)(a))
struct dtk_checkbox
{
    dtk_control control;
    char text[64];
    dtk_font* pFont;
    dtk_color textColor;
    dtk_color bgColor;
    dtk_color boxBGColor;
    dtk_color boxBGColorHovered;
    dtk_color boxBGColorPressed;
    dtk_color boxBorderColor;
    dtk_color checkColor;
    dtk_int32 borderWidth;
    dtk_int32 padding;
    dtk_bool32 isMouseOver       : 1;
    dtk_bool32 isAutoSizeEnabled : 1;
    dtk_bool32 isChecked         : 1;
};

dtk_result dtk_checkbox_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, const char* text, dtk_bool32 checked, dtk_checkbox* pCheckbox);
dtk_result dtk_checkbox_uninit(dtk_checkbox* pCheckbox);
dtk_bool32 dtk_checkbox_default_event_handler(dtk_event* pEvent);

void dtk_checkbox_set_text(dtk_checkbox* pCheckbox, const char* text);
void dtk_checkbox_enable_auto_size(dtk_checkbox* pCheckbox);
void dtk_checkbox_disable_auto_size(dtk_checkbox* pCheckbox);
void dtk_checkbox_auto_size(dtk_checkbox* pCheckbox);

void dtk_checkbox_set_font(dtk_checkbox* pCheckbox, dtk_font* pFont);
void dtk_checkbox_set_background_color(dtk_checkbox* pCheckbox, dtk_color color);
void dtk_checkbox_set_border_color(dtk_checkbox* pCheckbox, dtk_color color);
void dtk_checkbox_set_border_width(dtk_checkbox* pCheckbox, dtk_int32 width);
void dtk_checkbox_set_padding(dtk_checkbox* pCheckbox, dtk_int32 padding);

void dtk_checkbox_check(dtk_checkbox* pCheckbox);
void dtk_checkbox_uncheck(dtk_checkbox* pCheckbox);
void dtk_checkbox_toggle(dtk_checkbox* pCheckbox);
void dtk_checkbox_set_checked(dtk_checkbox* pCheckbox, dtk_bool32 checked, dtk_bool32 blockEvent);
dtk_bool32 dtk_checkbox_is_checked(dtk_checkbox* pCheckbox);
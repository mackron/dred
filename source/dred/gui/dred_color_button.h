// Copyright (C) 2019 David Reid. See included LICENSE file.

typedef struct dred_color_button dred_color_button;

#define DRED_COLOR_BUTTON(a) ((dred_color_button*)(a))
struct dred_color_button
{
    dtk_color_button buttonDTK;
    char varBinding[128];
};

dtk_result dred_color_button_init(dred_context* pDred, dtk_control* pParent, const char* text, dtk_color color, dred_color_button* pButton);
dtk_result dred_color_button_uninit(dred_color_button* pButton);
void dred_color_button_set_bind_to_config_var(dred_color_button* pButton, const char* varName);

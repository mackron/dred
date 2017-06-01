// Copyright (C) 2017 David Reid. See included LICENSE file.

// The checkbox control is a box with a check mark and a label. The size of the box is based on the size of the font.
typedef struct dred_checkbox dred_checkbox;

#define DRED_CHECKBOX(a) ((dred_checkbox*)(a))
struct dred_checkbox
{
    dtk_checkbox checkboxDTK;
    char varBinding[128];
};

dtk_result dred_checkbox_init(dred_context* pDred, dtk_control* pParent, const char* text, dtk_bool32 checked, dred_checkbox* pCheckbox);
dtk_result dred_checkbox_uninit(dred_checkbox* pCheckbox);

void dred_checkbox_set_bind_to_config_var(dred_checkbox* pCheckbox, const char* varName);
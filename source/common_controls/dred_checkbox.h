// The checkbox control is a box with a check mark and a label. The size of the box is based on the size of the font.

#define DRED_CONTROL_TYPE_CHECKBOX   "dred.common.checkbox"

typedef dred_control dred_checkbox;
typedef void (* dred_checkbox_on_checked_changed_proc)(dred_checkbox* pCheckbox);

dred_checkbox* dred_checkbox_create(dred_context* pDred, dred_control* pParent, const char* text, bool checked);
void dred_checkbox_delete(dred_checkbox* pCheckbox);

void dred_checkbox_set_text(dred_checkbox* pCheckbox, const char* text);
void dred_checkbox_enable_auto_size(dred_checkbox* pCheckbox);
void dred_checkbox_disable_auto_size(dred_checkbox* pCheckbox);

void dred_checkbox_set_font(dred_checkbox* pCheckbox, dred_font* pFont);
void dred_checkbox_set_background_color(dred_checkbox* pCheckbox, drgui_color color);
void dred_checkbox_set_border_color(dred_checkbox* pCheckbox, drgui_color color);
void dred_checkbox_set_border_width(dred_checkbox* pCheckbox, float width);
void dred_checkbox_set_padding(dred_checkbox* pCheckbox, float padding);

void dred_checkbox_check(dred_checkbox* pCheckbox);
void dred_checkbox_uncheck(dred_checkbox* pCheckbox);
void dred_checkbox_toggle(dred_checkbox* pCheckbox);
void dred_checkbox_set_checked(dred_checkbox* pCheckbox, bool checked, bool blockEvent);
bool dred_is_checked(dred_checkbox* pCheckbox);

void dred_checkbox_set_bind_to_config_var(dred_checkbox* pCheckbox, const char* varName);
void dred_checkbox_set_on_checked_changed(dred_checkbox* pCheckbox, dred_checkbox_on_checked_changed_proc proc);
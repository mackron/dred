// The checkbox control is a box with a check mark and a label. The size of the box is based on the size of the font.

#define DRED_CONTROL_TYPE_COLOR_BUTTON   "dred.common.colorbutton"

typedef dred_control dred_colorbutton;
typedef void (* dred_colorbutton_on_color_changed_proc)(dred_colorbutton* pButton, drgui_color color);

dred_colorbutton* dred_colorbutton_create(dred_context* pDred, dred_control* pParent, const char* text, drgui_color color);
void dred_colorbutton_delete(dred_colorbutton* pButton);

void dred_colorbutton_set_text(dred_colorbutton* pButton, const char* text);
void dred_colorbutton_enable_auto_size(dred_colorbutton* pButton);
void dred_colorbutton_disable_auto_size(dred_colorbutton* pButton);

void dred_colorbutton_set_font(dred_colorbutton* pButton, dred_font* pFont);
void dred_colorbutton_set_background_color(dred_colorbutton* pButton, drgui_color color);
void dred_colorbutton_set_border_color(dred_colorbutton* pButton, drgui_color color);
void dred_colorbutton_set_border_width(dred_colorbutton* pButton, float width);
void dred_colorbutton_set_padding(dred_colorbutton* pButton, float padding);

void dred_colorbutton_set_color(dred_colorbutton* pButton, drgui_color color, bool blockEvent);
drgui_color dred_colorbutton_get_color(dred_colorbutton* pbutton);

void dred_colorbutton_set_bind_to_config_var(dred_colorbutton* pButton, const char* varName);
void dred_colorbutton_set_on_color_changed(dred_colorbutton* pButton, dred_colorbutton_on_color_changed_proc proc);
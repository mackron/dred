
#define DRED_CONTROL_TYPE_TEXTBOX2  "dred.textbox2"

typedef dred_control dred_textbox2;

dred_textbox2* dred_textbox2_create(dred_context* pDred, dred_control* pParent);
void dred_textbox2_delete(dred_textbox2* pTextBox);

void dred_textbox2_set_text(dred_textbox2* pTextBox, const char* text);
// A dred_textbox is a dred_control that has a child drgui_textbox object.

#define DRED_CONTROL_TYPE_TEXTBOX  "dred.textbox"

typedef dred_control dred_textbox;

// dred_textbox_create()
dred_textbox* dred_textbox_create(dred_context* pDred, dred_control* pParent);

// dred_textbox_delete()
void dred_textbox_delete(dred_textbox* pTextBox);


// Sets the text of the text box.
void dred_textbox_set_text(dred_textbox* pTextBox, const char* text);

// Retrieves the text of the text box.
size_t dred_textbox_get_text(dred_textbox* pTextBox, char* textOut, size_t textOutSize);


// Function for explicitly handling the on_key_down event.
void dred_textbox_on_key_down(dred_textbox* pTextBox, drgui_key key, int stateFlags);

// Function for explicitly handling the on_key_up event.
void dred_textbox_on_key_up(dred_textbox* pTextBox, drgui_key key, int stateFlags);

// Function for explicitly handling the on_printable_key_down event.
void dred_textbox_on_printable_key_down(dred_textbox* pTextBox, uint32_t utf32, int stateFlags);
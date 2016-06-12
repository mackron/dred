
#define DRED_CONTROL_TYPE_TEXT_EDITOR  "dred.editor.text"

typedef dred_editor dred_text_editor;


// dred_text_editor_create()
dred_text_editor* dred_text_editor_create(dred_context* pDred, dred_control* pParent, const char* filePathAbsolute);

// dred_text_editor_delete()
void dred_text_editor_delete(dred_text_editor* pTextEditor);


// dred_text_editor_goto_ratio()
void dred_text_editor_goto_ratio(dred_text_editor* pTextEditor, unsigned int ratio);

// dred_text_editor_goto_line()
void dred_text_editor_goto_line(dred_text_editor* pTextEditor, unsigned int lineNumber);
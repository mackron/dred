
#define DRED_CONTROL_TYPE_EDITOR  "dred.editor"

typedef dred_control dred_editor;


// dred_editor_create()
dred_editor* dred_editor_create(dred_context* pDred, dred_control* pParent, const char* type, const char* filePathAbsolute, size_t extraDataSize);

// dred_editor_delete()
void dred_editor_delete(dred_editor* pEditor);


// dred_editor_get_data()
void* dred_editor_get_extra_data(dred_editor* pEditor);

// dred_editor_get_data_size()
size_t dred_editor_get_extra_data_size(dred_editor* pEditor);


// Retrieves the absolute path of the file the editor is tied to.
//
// Returns an empty string if the editor is not tied to a file.
const char* dred_editor_get_file_path(dred_editor* pEditor);
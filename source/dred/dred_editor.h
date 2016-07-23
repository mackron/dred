// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_EDITOR  "dred.editor"

typedef dred_control dred_editor;

typedef bool (* dred_editor_on_save_proc)(dred_editor* pEditor, dred_file file, const char* filePath);
typedef bool (* dred_editor_on_reload_proc)(dred_editor* pEditor);
typedef void (* dred_editor_on_modified_proc)(dred_editor* pEditor);
typedef void (* dred_editor_on_unmodified_proc)(dred_editor* pEditor);


// dred_editor_create()
dred_editor* dred_editor_create(dred_context* pDred, dred_control* pParent, const char* type, float sizeX, float sizeY, const char* filePathAbsolute, size_t extraDataSize);

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

// Sets the file path of the given editor.
bool dred_editor_set_file_path(dred_editor* pEditor, const char* newFilePath);


// Saves the given editor to the given file.
//
// This will change the file association to the new file.
bool dred_editor_save(dred_editor* pEditor, const char* newFilePath);

// Reloads the given editor.
bool dred_editor_reload(dred_editor* pEditor);

// Checks if the file tied to the given editor is dirty and reloads it if so.
bool dred_editor_check_if_dirty_and_reload(dred_editor* pEditor);


// Marks the editor as modified.
void dred_editor_mark_as_modified(dred_editor* pEditor);

// Unmarks the editor as modified.
void dred_editor_unmark_as_modified(dred_editor* pEditor);

// Determines whether or not the editor is marked as modified.
bool dred_editor_is_modified(dred_editor* pEditor);


// Updates the last modified time of the file.
void dred_editor_update_file_last_modified_time(dred_editor* pEditor);

// Determines if the editor is read-only.
bool dred_editor_is_read_only(dred_editor* pEditor);


// Events
void dred_editor_set_on_save(dred_editor* pEditor, dred_editor_on_save_proc proc);
void dred_editor_set_on_reload(dred_editor* pEditor, dred_editor_on_reload_proc proc);
void dred_editor_set_on_modified(dred_editor* pEditor, dred_editor_on_modified_proc proc);
void dred_editor_set_on_unmodified(dred_editor* pEditor, dred_editor_on_unmodified_proc proc);
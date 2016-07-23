// Copyright (C) 2016 David Reid. See included LICENSE file.

#include "dred_editor.h"

typedef struct
{
    char filePathAbsolute[DRED_MAX_PATH];
    uint64_t fileLastModifiedTime;
    dred_editor_on_save_proc onSave;
    dred_editor_on_reload_proc onReload;
    dred_editor_on_modified_proc onModified;
    dred_editor_on_unmodified_proc onUnmodified;
    bool isModified;
    bool isReadOnly;
} dred_editor_data;

dred_editor* dred_editor_create(dred_context* pDred, dred_control* pParent, const char* type, float sizeX, float sizeY, const char* filePathAbsolute, size_t extraDataSize)
{
    if (!dred_is_control_type_of_type(type, DRED_CONTROL_TYPE_EDITOR)) {
        dred_errorf(pDred, "[DEVELOPER ERROR] Attempting to create an editor that is not of an editor type (%s).", type);
        return NULL;
    }

    dred_control* pEditor = dred_control_create(pDred, pParent, type, sizeof(dred_editor_data) + extraDataSize);
    if (pEditor == NULL) {
        return NULL;
    }

    drgui_set_size(pEditor, sizeX, sizeY);

    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    assert(data != NULL);

    memset(data, 0, dred_control_get_extra_data_size(pEditor));

    if (filePathAbsolute != NULL && filePathAbsolute[0] != '\0') {
        if (drpath_clean(filePathAbsolute, data->filePathAbsolute, sizeof(data->filePathAbsolute)) == 0) {
            dred_control_delete(pEditor);
            dred_errorf(pDred, "File path is too long: %s\n", filePathAbsolute);
            return NULL;
        }

        dred_editor_update_file_last_modified_time(pEditor);
    }

    data->isReadOnly = dr_is_file_read_only(filePathAbsolute);

    return pEditor;
}

void dred_editor_delete(dred_editor* pEditor)
{
    dred_control_delete(pEditor);
}


void* dred_editor_get_extra_data(dred_editor* pEditor)
{
    return (void*)((uint8_t*)dred_control_get_extra_data(pEditor) + sizeof(dred_editor_data));
}

size_t dred_editor_get_extra_data_size(dred_editor* pEditor)
{
    return dred_control_get_extra_data_size(pEditor) - sizeof(dred_editor_data);
}


const char* dred_editor_get_file_path(dred_editor* pEditor)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return "";
    }

    return data->filePathAbsolute;
}

bool dred_editor_set_file_path(dred_editor* pEditor, const char* newFilePath)
{
    if (pEditor == NULL || newFilePath == NULL) {
        return false;
    }

    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return false;
    }

    if (drpath_is_relative(newFilePath)) {
        char basePath[DRED_MAX_PATH];
        if (!dr_get_current_directory(basePath, sizeof(basePath))) {
            return false;
        }

        return drpath_append_and_clean(data->filePathAbsolute, sizeof(data->filePathAbsolute), basePath, newFilePath) > 0;
    } else {
        return strcpy_s(data->filePathAbsolute, sizeof(data->filePathAbsolute), newFilePath) == 0;
    }
}


bool dred_editor_save(dred_editor* pEditor, const char* newFilePath)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return false;
    }

    if (data->onSave == NULL) {
        return false;
    }

    if (data->isReadOnly && (newFilePath == NULL || newFilePath[0] == '\0')) {
        dred_errorf(dred_control_get_gui(pEditor), "File is read only.");
        return false;
    }

    const char* actualFilePath = newFilePath;
    if (actualFilePath == NULL || actualFilePath[0] == '\0') {
        actualFilePath = dred_editor_get_file_path(pEditor);
    }

    if (actualFilePath == NULL || actualFilePath[0] == '\0') {
        return false;
    }

    // Saving a file happens in two steps. The first step writes to a temporary file. The second step replaces the old
    // file with the new file. The reason for this process is to prevent data loss in the event that an error occurs
    // while in the middle of saving.
    char tempFilePath[DRED_MAX_PATH];
    if (!drpath_copy_and_append_extension(tempFilePath, sizeof(tempFilePath), actualFilePath, "dredtmp")) {
        return false;
    }

    dred_file file = dred_file_open(tempFilePath, DRED_FILE_OPEN_MODE_WRITE);
    if (file == NULL) {
        return false;
    }

    if (!data->onSave(pEditor, file, actualFilePath)) {
        dred_file_close(file);
        return false;
    }

    dred_file_close(file);


    // At this point the temporary file has been saved, so now we just need to overwrite the old one.
    if (!dr_move_file(tempFilePath, actualFilePath)) {
        dr_delete_file(tempFilePath);
        return false;
    }

    dred_editor_unmark_as_modified(pEditor);
    dred_editor_update_file_last_modified_time(pEditor);
    data->isReadOnly = dr_is_file_read_only(actualFilePath);

    if (newFilePath != NULL && newFilePath[0] != '\0') {
        return dred_editor_set_file_path(pEditor, newFilePath);
    } else {
        return true;
    }
}

bool dred_editor_reload(dred_editor* pEditor)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return false;
    }

    if (data->onReload == NULL) {
        return false;
    }

    const char* fileName = dred_editor_get_file_path(pEditor);
    if (fileName == NULL || fileName[0] == '\0') {
        return false;
    }

    if (!data->onReload(pEditor)) {
        return false;
    }

    dred_editor_update_file_last_modified_time(pEditor);
    return true;
}

bool dred_editor_check_if_dirty_and_reload(dred_editor* pEditor)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return false;
    }

    if (data->fileLastModifiedTime >= dr_get_file_modified_time(dred_editor_get_file_path(pEditor))) {
        return false;   // Not modified.
    }

    // It's dirty. Try reloading.
    return dred_editor_reload(pEditor);
}


void dred_editor_mark_as_modified(dred_editor* pEditor)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return;
    }

    if (data->isModified) {
        return;
    }

    data->isModified = true;
    if (data->onModified) {
        data->onModified(pEditor);
    }
}

void dred_editor_unmark_as_modified(dred_editor* pEditor)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return;
    }

    if (!data->isModified) {
        return;
    }

    data->isModified = false;
    if (data->onUnmodified) {
        data->onUnmodified(pEditor);
    }
}

bool dred_editor_is_modified(dred_editor* pEditor)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return false;
    }

    return data->isModified;
}


void dred_editor_update_file_last_modified_time(dred_editor* pEditor)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return;
    }

    data->fileLastModifiedTime = dr_get_file_modified_time(dred_editor_get_file_path(pEditor));
}

bool dred_editor_is_read_only(dred_editor* pEditor)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return false;
    }

    return data->isReadOnly;
}



void dred_editor_set_on_save(dred_editor* pEditor, dred_editor_on_save_proc proc)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return;
    }

    data->onSave = proc;
}

void dred_editor_set_on_reload(dred_editor* pEditor, dred_editor_on_reload_proc proc)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return;
    }

    data->onReload = proc;
}

void dred_editor_set_on_modified(dred_editor* pEditor, dred_editor_on_modified_proc proc)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return;
    }

    data->onModified = proc;
}

void dred_editor_set_on_unmodified(dred_editor* pEditor, dred_editor_on_unmodified_proc proc)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return;
    }

    data->onUnmodified = proc;
}
// Copyright (C) 2016 David Reid. See included LICENSE file.

bool dred_editor_init(dred_editor* pEditor, dred_context* pDred, dred_control* pParent, const char* type, float sizeX, float sizeY, const char* filePathAbsolute)
{
    if (!dred_is_control_type_of_type(type, DRED_CONTROL_TYPE_EDITOR)) {
        dred_errorf(pDred, "[DEVELOPER ERROR] Attempting to create an editor that is not of an editor type (%s).", type);
        return false;
    }

    memset(pEditor, 0, sizeof(*pEditor));

    if (!dred_control_init(DRED_CONTROL(pEditor), pDred, pParent, type)) {
        free(pEditor);
        return false;
    }

    dred_control_set_size(DRED_CONTROL(pEditor), sizeX, sizeY);
    

    if (filePathAbsolute != NULL && filePathAbsolute[0] != '\0') {
        if (drpath_clean(filePathAbsolute, pEditor->filePathAbsolute, sizeof(pEditor->filePathAbsolute)) == 0) {
            dred_control_uninit(DRED_CONTROL(pEditor));
            dred_errorf(pDred, "File path is too long: %s\n", filePathAbsolute);
            return false;
        }

        dred_editor_update_file_last_modified_time(pEditor);
    }

    pEditor->isReadOnly = dr_is_file_read_only(filePathAbsolute);

    return true;
}

void dred_editor_uninit(dred_editor* pEditor)
{
    dred_control_uninit(DRED_CONTROL(pEditor));
}


void* dred_editor_get_extra_data(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return NULL;
    }

    return pEditor->pExtraData;
}

size_t dred_editor_get_extra_data_size(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return 0;
    }

    return pEditor->extraDataSize;
}


const char* dred_editor_get_file_path(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return "";
    }

    return pEditor->filePathAbsolute;
}

bool dred_editor_set_file_path(dred_editor* pEditor, const char* newFilePath)
{
    if (pEditor == NULL || newFilePath == NULL) {
        return false;
    }

    if (pEditor == NULL) {
        return false;
    }

    if (drpath_is_relative(newFilePath)) {
        char basePath[DRED_MAX_PATH];
        if (!dr_get_current_directory(basePath, sizeof(basePath))) {
            return false;
        }

        return drpath_append_and_clean(pEditor->filePathAbsolute, sizeof(pEditor->filePathAbsolute), basePath, newFilePath) > 0;
    } else {
        return strcpy_s(pEditor->filePathAbsolute, sizeof(pEditor->filePathAbsolute), newFilePath) == 0;
    }
}


bool dred_editor_save(dred_editor* pEditor, const char* newFilePath)
{
    if (pEditor == NULL) {
        return false;
    }

    if (pEditor->onSave == NULL) {
        return false;
    }

    if (pEditor->isReadOnly && (newFilePath == NULL || newFilePath[0] == '\0')) {
        dred_errorf(dred_control_get_context(DRED_CONTROL(pEditor)), "File is read only.");
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
    // file with the new file. The reason for this process is to prevent pEditor loss in the event that an error occurs
    // while in the middle of saving.
    char tempFilePath[DRED_MAX_PATH];
    if (!drpath_copy_and_append_extension(tempFilePath, sizeof(tempFilePath), actualFilePath, "dredtmp")) {
        return false;
    }

    dred_file file = dred_file_open(tempFilePath, DRED_FILE_OPEN_MODE_WRITE);
    if (file == NULL) {
        return false;
    }

    if (!pEditor->onSave(pEditor, file, actualFilePath)) {
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
    pEditor->isReadOnly = dr_is_file_read_only(actualFilePath);

    if (newFilePath != NULL && newFilePath[0] != '\0') {
        return dred_editor_set_file_path(pEditor, newFilePath);
    } else {
        return true;
    }
}

bool dred_editor_reload(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return false;
    }

    if (pEditor->onReload == NULL) {
        return false;
    }

    const char* fileName = dred_editor_get_file_path(pEditor);
    if (fileName == NULL || fileName[0] == '\0') {
        return false;
    }

    if (!pEditor->onReload(pEditor)) {
        return false;
    }

    dred_editor_update_file_last_modified_time(pEditor);
    return true;
}

bool dred_editor_check_if_dirty_and_reload(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return false;
    }

    if (pEditor->fileLastModifiedTime >= dr_get_file_modified_time(dred_editor_get_file_path(pEditor))) {
        return false;   // Not modified.
    }

    // It's dirty. Try reloading.
    return dred_editor_reload(pEditor);
}


void dred_editor_mark_as_modified(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return;
    }

    if (pEditor->isModified) {
        return;
    }

    pEditor->isModified = true;
    if (pEditor->onModified) {
        pEditor->onModified(pEditor);
    }
}

void dred_editor_unmark_as_modified(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return;
    }

    if (!pEditor->isModified) {
        return;
    }

    pEditor->isModified = false;
    if (pEditor->onUnmodified) {
        pEditor->onUnmodified(pEditor);
    }
}

bool dred_editor_is_modified(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return false;
    }

    return pEditor->isModified;
}


void dred_editor_update_file_last_modified_time(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return;
    }

    pEditor->fileLastModifiedTime = dr_get_file_modified_time(dred_editor_get_file_path(pEditor));
}

bool dred_editor_is_read_only(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return false;
    }

    return pEditor->isReadOnly;
}



void dred_editor_set_on_save(dred_editor* pEditor, dred_editor_on_save_proc proc)
{
    if (pEditor == NULL) {
        return;
    }

    pEditor->onSave = proc;
}

void dred_editor_set_on_reload(dred_editor* pEditor, dred_editor_on_reload_proc proc)
{
    if (pEditor == NULL) {
        return;
    }

    pEditor->onReload = proc;
}

void dred_editor_set_on_modified(dred_editor* pEditor, dred_editor_on_modified_proc proc)
{
    if (pEditor == NULL) {
        return;
    }

    pEditor->onModified = proc;
}

void dred_editor_set_on_unmodified(dred_editor* pEditor, dred_editor_on_unmodified_proc proc)
{
    if (pEditor == NULL) {
        return;
    }

    pEditor->onUnmodified = proc;
}
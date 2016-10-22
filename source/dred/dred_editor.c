// Copyright (C) 2016 David Reid. See included LICENSE file.

dr_bool32 dred_editor_init(dred_editor* pEditor, dred_context* pDred, dred_control* pParent, const char* type, float sizeX, float sizeY, const char* filePathAbsolute)
{
    if (!dred_is_control_type_of_type(type, DRED_CONTROL_TYPE_EDITOR)) {
        dred_errorf(pDred, "[DEVELOPER ERROR] Attempting to create an editor that is not of an editor type (%s).", type);
        return DR_FALSE;
    }

    memset(pEditor, 0, sizeof(*pEditor));

    if (!dred_control_init(DRED_CONTROL(pEditor), pDred, pParent, type)) {
        free(pEditor);
        return DR_FALSE;
    }

    dred_control_set_size(DRED_CONTROL(pEditor), sizeX, sizeY);
    

    if (filePathAbsolute != NULL && filePathAbsolute[0] != '\0') {
        if (drpath_clean(filePathAbsolute, pEditor->filePathAbsolute, sizeof(pEditor->filePathAbsolute)) == 0) {
            dred_control_uninit(DRED_CONTROL(pEditor));
            dred_errorf(pDred, "File path is too long: %s\n", filePathAbsolute);
            return DR_FALSE;
        }

        dred_editor_update_file_last_modified_time(pEditor);
    }

    pEditor->isReadOnly = dr_is_file_read_only(filePathAbsolute);

    return DR_TRUE;
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

dr_bool32 dred_editor_set_file_path(dred_editor* pEditor, const char* newFilePath)
{
    if (pEditor == NULL || newFilePath == NULL) {
        return DR_FALSE;
    }

    if (pEditor == NULL) {
        return DR_FALSE;
    }

    if (drpath_is_relative(newFilePath)) {
        char basePath[DRED_MAX_PATH];
        if (!dr_get_current_directory(basePath, sizeof(basePath))) {
            return DR_FALSE;
        }

        return drpath_append_and_clean(pEditor->filePathAbsolute, sizeof(pEditor->filePathAbsolute), basePath, newFilePath) > 0;
    } else {
        return strcpy_s(pEditor->filePathAbsolute, sizeof(pEditor->filePathAbsolute), newFilePath) == 0;
    }
}


dr_bool32 dred_editor_save(dred_editor* pEditor, const char* newFilePath)
{
    if (pEditor == NULL) {
        return DR_FALSE;
    }

    if (pEditor->onSave == NULL) {
        return DR_FALSE;
    }

    if (pEditor->isReadOnly && (newFilePath == NULL || newFilePath[0] == '\0')) {
        dred_errorf(dred_control_get_context(DRED_CONTROL(pEditor)), "File is read only.");
        return DR_FALSE;
    }

    const char* actualFilePath = newFilePath;
    if (actualFilePath == NULL || actualFilePath[0] == '\0') {
        actualFilePath = dred_editor_get_file_path(pEditor);
    }

    if (actualFilePath == NULL || actualFilePath[0] == '\0') {
        return DR_FALSE;
    }

    // Saving is done in 3 stages:
    //   1) A copy of the original file is created called <original_file>.dredtmp.
    //   2) The contents of the original file is replaced with the new contents.
    //   3) The copy of the original file is deleted.
    //
    // The rationale for this system is to try and prevent data loss in the event that an error occurs while in the
    // middle of saving.
    dr_bool32 haveTempFile = DR_FALSE;
    char tempFilePath[DRED_MAX_PATH];

    const char* originalFilePath = dred_editor_get_file_path(pEditor);
    if (!dred_string_is_null_or_empty(originalFilePath)) {
        if (drpath_copy_and_append_extension(tempFilePath, sizeof(tempFilePath), originalFilePath, "dredtmp")) {
            if (dr_copy_file(originalFilePath, tempFilePath, DR_TRUE)) {
                haveTempFile = DR_TRUE;
            }
        }
    }

    dr_bool32 wasSaved = DR_FALSE;
    dred_file file = dred_file_open(actualFilePath, DRED_FILE_OPEN_MODE_WRITE);
    if (file != NULL) {
        wasSaved = pEditor->onSave(pEditor, file, actualFilePath);
        dred_file_close(file);
    }

    // Everything should be saved so just delete the temporary one.
    if (haveTempFile) {
        dr_delete_file(tempFilePath);
    }

    if (!wasSaved) {
        return DR_FALSE;
    }
    

    dred_editor_unmark_as_modified(pEditor);
    dred_editor_update_file_last_modified_time(pEditor);
    pEditor->isReadOnly = dr_is_file_read_only(actualFilePath);

    if (newFilePath != NULL && newFilePath[0] != '\0') {
        return dred_editor_set_file_path(pEditor, newFilePath);
    } else {
        return DR_TRUE;
    }
}

dr_bool32 dred_editor_reload(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return DR_FALSE;
    }

    if (pEditor->onReload == NULL) {
        return DR_FALSE;
    }

    const char* fileName = dred_editor_get_file_path(pEditor);
    if (fileName == NULL || fileName[0] == '\0') {
        return DR_FALSE;
    }

    if (!pEditor->onReload(pEditor)) {
        return DR_FALSE;
    }

    dred_editor_update_file_last_modified_time(pEditor);
    return DR_TRUE;
}

dr_bool32 dred_editor_check_if_dirty_and_reload(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return DR_FALSE;
    }

    if (pEditor->fileLastModifiedTime >= dr_get_file_modified_time(dred_editor_get_file_path(pEditor))) {
        return DR_FALSE;   // Not modified.
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

    pEditor->isModified = DR_TRUE;
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

    pEditor->isModified = DR_FALSE;
    if (pEditor->onUnmodified) {
        pEditor->onUnmodified(pEditor);
    }
}

dr_bool32 dred_editor_is_modified(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return DR_FALSE;
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

dr_bool32 dred_editor_is_read_only(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return DR_FALSE;
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
// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_bool32 dred_editor_init(dred_editor* pEditor, dred_context* pDred, dtk_control* pParent, const char* type, dtk_event_proc onEvent, dtk_int32 sizeX, dtk_int32 sizeY, const char* filePathAbsolute)
{
    if (!dred_is_control_type_of_type(type, DRED_CONTROL_TYPE_EDITOR)) {
        dred_errorf(pDred, "[DEVELOPER ERROR] Attempting to create an editor that is not of an editor type (%s).", type);
        return DTK_FALSE;
    }

    memset(pEditor, 0, sizeof(*pEditor));

    if (!dred_control_init(DRED_CONTROL(pEditor), pDred, NULL, pParent, type, onEvent)) {
        free(pEditor);
        return DTK_FALSE;
    }

    dred_control_set_size(DRED_CONTROL(pEditor), sizeX, sizeY);
    

    if (filePathAbsolute != NULL && filePathAbsolute[0] != '\0') {
        if (dtk_path_clean(pEditor->filePathAbsolute, sizeof(pEditor->filePathAbsolute), filePathAbsolute) == 0) {
            dred_control_uninit(DRED_CONTROL(pEditor));
            dred_errorf(pDred, "File path is too long: %s\n", filePathAbsolute);
            return DTK_FALSE;
        }

        dred_editor_update_file_last_modified_time(pEditor);
    }

    pEditor->isReadOnly = dtk_is_file_read_only(filePathAbsolute);

    return DTK_TRUE;
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

dtk_bool32 dred_editor_set_file_path(dred_editor* pEditor, const char* newFilePath)
{
    if (pEditor == NULL || newFilePath == NULL) {
        return DTK_FALSE;
    }

    if (dtk_path_is_relative(newFilePath)) {
        char* pCurrentDir = dtk_get_current_directory();
        if (pCurrentDir == NULL) {
            return DTK_FALSE;
        }

        if (dtk_path_append_and_clean(pEditor->filePathAbsolute, sizeof(pEditor->filePathAbsolute), pCurrentDir, newFilePath) == 0) {
            dtk_free(pCurrentDir);
            return DTK_FALSE;
        }

        dtk_free(pCurrentDir);
        return DTK_TRUE;
    } else {
        return strcpy_s(pEditor->filePathAbsolute, sizeof(pEditor->filePathAbsolute), newFilePath) == 0;
    }
}


dtk_bool32 dred_editor_save(dred_editor* pEditor, const char* newFilePath)
{
    if (pEditor == NULL) {
        return DTK_FALSE;
    }

    if (pEditor->onSave == NULL) {
        return DTK_FALSE;
    }

    if (pEditor->isReadOnly && (newFilePath == NULL || newFilePath[0] == '\0')) {
        dred_errorf(dred_control_get_context(DRED_CONTROL(pEditor)), "File is read only.");
        return DTK_FALSE;
    }

    const char* actualFilePath = newFilePath;
    if (actualFilePath == NULL || actualFilePath[0] == '\0') {
        actualFilePath = dred_editor_get_file_path(pEditor);
    }

    if (actualFilePath == NULL || actualFilePath[0] == '\0') {
        return DTK_FALSE;
    }

    // Saving is done in 3 stages:
    //   1) A copy of the original file is created called <original_file>.dredtmp.
    //   2) The contents of the original file is replaced with the new contents.
    //   3) The copy of the original file is deleted.
    //
    // The rationale for this system is to try and prevent data loss in the event that an error occurs while in the
    // middle of saving.
    dtk_bool32 haveTempFile = DTK_FALSE;
    char tempFilePath[DRED_MAX_PATH];

    const char* originalFilePath = dred_editor_get_file_path(pEditor);
    if (!dred_string_is_null_or_empty(originalFilePath)) {
        if (dtk_path_append_extension(tempFilePath, sizeof(tempFilePath), originalFilePath, "dredtmp")) {
            if (dtk_copy_file(originalFilePath, tempFilePath, DTK_TRUE) == DTK_SUCCESS) {
                haveTempFile = DTK_TRUE;
            }
        }
    }

    dtk_bool32 wasSaved = DTK_FALSE;
    FILE* pFile;
    dtk_result result = dtk_fopen(&pFile, actualFilePath, dtk_fopenmode(DTK_OPEN_MODE_WRITE));
    if (result == DTK_SUCCESS) {
        wasSaved = pEditor->onSave(pEditor, pFile, actualFilePath);
        dtk_fclose(pFile);
    }

    // Everything should be saved so just delete the temporary one.
    if (haveTempFile) {
        dtk_delete_file(tempFilePath);
    }

    if (!wasSaved) {
        return DTK_FALSE;
    }
    

    dred_editor_unmark_as_modified(pEditor);
    dred_editor_update_file_last_modified_time(pEditor);
    pEditor->isReadOnly = dtk_is_file_read_only(actualFilePath);

    if (newFilePath != NULL && newFilePath[0] != '\0') {
        return dred_editor_set_file_path(pEditor, newFilePath);
    } else {
        return DTK_TRUE;
    }
}

dtk_bool32 dred_editor_reload(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return DTK_FALSE;
    }

    if (pEditor->onReload == NULL) {
        return DTK_FALSE;
    }

    const char* fileName = dred_editor_get_file_path(pEditor);
    if (fileName == NULL || fileName[0] == '\0') {
        return DTK_FALSE;
    }

    if (!pEditor->onReload(pEditor)) {
        return DTK_FALSE;
    }

    dred_editor_update_file_last_modified_time(pEditor);
    return DTK_TRUE;
}

dtk_bool32 dred_editor_check_if_dirty_and_reload(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return DTK_FALSE;
    }

    if (pEditor->fileLastModifiedTime >= dtk_get_file_modified_time(dred_editor_get_file_path(pEditor))) {
        return DTK_FALSE;   // Not modified.
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

    pEditor->isModified = DTK_TRUE;
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

    pEditor->isModified = DTK_FALSE;
    if (pEditor->onUnmodified) {
        pEditor->onUnmodified(pEditor);
    }
}

dtk_bool32 dred_editor_is_modified(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return DTK_FALSE;
    }

    return pEditor->isModified;
}


void dred_editor_update_file_last_modified_time(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return;
    }

    pEditor->fileLastModifiedTime = dtk_get_file_modified_time(dred_editor_get_file_path(pEditor));
}

dtk_bool32 dred_editor_is_read_only(dred_editor* pEditor)
{
    if (pEditor == NULL) {
        return DTK_FALSE;
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
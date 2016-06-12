
typedef struct
{
    char filePathAbsolute[DRED_MAX_PATH];
    dred_editor_on_save_proc onSave;
} dred_editor_data;

dred_editor* dred_editor_create(dred_context* pDred, dred_control* pParent, const char* type, const char* filePathAbsolute, size_t extraDataSize)
{
    if (!dred_is_control_type_of_type(type, DRED_CONTROL_TYPE_EDITOR)) {
        dred_errorf(pDred, "[DEVELOPER ERROR] Attempting to create an editor that is not of an editor type (%s).", type);
        return NULL;
    }

    dred_control* pEditor = dred_control_create(pDred, pParent, type, sizeof(dred_editor_data) + extraDataSize);
    if (pEditor == NULL) {
        return NULL;
    }

    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    assert(data != NULL);

    memset(data, 0, dred_control_get_extra_data_size(pEditor));

    if (strcpy_s(data->filePathAbsolute, sizeof(data->filePathAbsolute), filePathAbsolute) != 0) {
        dred_control_delete(pEditor);
        dred_errorf(pDred, "File path is too long: %s\n", filePathAbsolute);
        return NULL;
    }

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

    const char* actualFilePath = newFilePath;
    if (actualFilePath == NULL || actualFilePath[0] == '\0') {
        actualFilePath = dred_editor_get_file_path(pEditor);
    }

    dred_file* pFile = dred_file_open(actualFilePath, DRED_FILE_OPEN_MODE_WRITE);
    if (pFile == NULL) {
        return false;
    }

    if (!data->onSave(pEditor, pFile)) {
        dred_file_close(pFile);
        return false;
    }

    dred_file_close(pFile);


    if (newFilePath != NULL && newFilePath[0] != '\0') {
        return dred_editor_set_file_path(pEditor, newFilePath);
    } else {
        return true;
    }
}


void dred_editor_set_on_save(dred_editor* pEditor, dred_editor_on_save_proc proc)
{
    dred_editor_data* data = (dred_editor_data*)dred_control_get_extra_data(pEditor);
    if (data == NULL) {
        return;
    }

    data->onSave = proc;
}
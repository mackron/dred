
typedef struct
{
    int unused;
} dred_editor_data;

dred_editor* dred_editor_create(dred_context* pDred, dred_control* pParent, const char* type, size_t extraDataSize)
{
    if (!dred_is_control_type_of_type(type, DRED_CONTROL_TYPE_EDITOR)) {
        dred_errorf(pDred, "[DEVELOPER ERROR] Attempting to create an editor that is not of an editor type (%s).", type);
        return NULL;
    }

    dred_control* pEditor = dred_control_create(pDred, pParent, type, sizeof(dred_editor_data) + extraDataSize);
    if (pEditor == NULL) {
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
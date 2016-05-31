

bool dred_config_init_default(dred_config* pConfig)
{
    if (pConfig == NULL) {
        return false;
    }

    pConfig->uiScaleX = 1;
    pConfig->uiScaleY = 1;

    return true;
}

void dred_config_uninit(dred_config* pConfig)
{
    if (pConfig == NULL) {
        return;
    }

    // Free any dynamically allocated data.
}


typedef struct
{
    dred_config* pConfig;
    const char* filePath;
    dred_config_on_error_proc onError;
    void* pUserData;
    dred_file file;
} dred_config_load_file__data;

size_t dred_config_load_file__on_read(void* pUserData, void* pDataOut, size_t bytesToRead)
{
    dred_config_load_file__data* pData = (dred_config_load_file__data*)pUserData;
    assert(pData != NULL);
    assert(pData->file != NULL);

    size_t bytesRead;
    if (!dred_file_read(pData->file, pDataOut, bytesToRead, &bytesRead)) {
        return 0;
    }

    return bytesRead;
}

void dred_config_load_file__on_pair(void* pUserData, const char* key, const char* value)
{
    dred_config_load_file__data* pData = (dred_config_load_file__data*)pUserData;
    assert(pData != NULL);

    if (strcmp(key, "ui-scale-x") == 0) {
        pData->pConfig->uiScaleX = (float)atof(value);
        return;
    }
    if (strcmp(key, "ui-scale-y") == 0) {
        pData->pConfig->uiScaleY = (float)atof(value);
        return;
    }
}

void dred_config_load_file__on_error(void* pUserData, const char* message, unsigned int line)
{
    dred_config_load_file__data* pData = (dred_config_load_file__data*)pUserData;
    assert(pData != NULL);

    if (pData->onError) {
        pData->onError(pData->pConfig, pData->filePath, message, line, pData->pUserData);
    }
}

bool dred_config_load_file(dred_config* pConfig, const char* filePath, dred_config_on_error_proc onError, void* pUserData)
{
    dred_file file = dred_file_open(filePath, DRED_FILE_OPEN_MODE_READ);
    if (file == NULL) {
        return false;
    }

    dred_config_load_file__data data;
    data.pConfig = pConfig;
    data.filePath = filePath;
    data.onError = onError;
    data.pUserData = pUserData;
    data.file = file;
    dr_parse_key_value_pairs(dred_config_load_file__on_read, dred_config_load_file__on_pair, dred_config_load_file__on_error, &data);
    return true;
}
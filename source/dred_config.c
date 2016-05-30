

bool dred_config_init_default(dred_config* pConfig)
{
    if (pConfig == NULL) {
        return false;
    }

    pConfig->scaleX = 1;
    pConfig->scaleY = 1;

    return true;
}

void dred_config_uninit(dred_config* pConfig)
{
    if (pConfig == NULL) {
        return;
    }

    // Free any dynamically allocated data.
}


void dred_config_load_file__on_pair(void* pUserData, const char* key, const char* value)
{
    dred_config* pConfig = (dred_config*)pUserData;

    if (strcmp(key, "scale-x") == 0) {
        pConfig->scaleX = (float)atof(value);
        return;
    }
    if (strcmp(key, "scale-y") == 0) {
        pConfig->scaleY = (float)atof(value);
        return;
    }
}

bool dred_config_load_file(dred_config* pConfig, const char* filePath, dr_key_value_error_proc onError)
{
    dr_parse_key_value_pairs_from_file(filePath, dred_config_load_file__on_pair, onError, pConfig);
}
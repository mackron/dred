
typedef struct
{
    float scaleX;   // scale-x
    float scaleY;   // scale-y
} dred_config;

bool dred_config_init_default(dred_config* pConfig);
void dred_config_uninit(dred_config* pConfig);

// The pUserData argument of onError will be set to pConfig.
bool dred_config_load_file(dred_config* pConfig, const char* filePath, dr_key_value_error_proc onError);


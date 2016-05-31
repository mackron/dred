
typedef struct
{
    float uiScaleX;   // scale-x
    float uiScaleY;   // scale-y
} dred_config;

typedef void (* dred_config_on_error_proc)(dred_config* pConfig, const char* configPath, const char* message, unsigned int line, void* pUserData);

bool dred_config_init_default(dred_config* pConfig);
void dred_config_uninit(dred_config* pConfig);

// The pUserData argument of onError will be set to pConfig.
bool dred_config_load_file(dred_config* pConfig, const char* filePath, dred_config_on_error_proc onError, void* pUserData);


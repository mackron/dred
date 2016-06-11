
typedef struct
{
    float uiScaleX;     // scale-x
    float uiScaleY;     // scale-y
    float cmdbarHeight; // cmdbar-height

    dred_font* pTextEditorFont;
} dred_config;

typedef void (* dred_config_on_error_proc)(dred_config* pConfig, const char* configPath, const char* message, unsigned int line, void* pUserData);

bool dred_config_init_default(dred_config* pConfig, dred_context* pDred);
void dred_config_uninit(dred_config* pConfig, dred_context* pDred);

// The pUserData argument of onError will be set to pConfig.
bool dred_config_load_file(dred_config* pConfig, dred_context* pDred, const char* filePath, dred_config_on_error_proc onError, void* pUserData);


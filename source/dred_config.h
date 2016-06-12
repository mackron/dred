
typedef struct
{
    // The main context that owns this config. This is set to the context that initialized the config object.
    dred_context* pDred;

    float uiScaleX;     // scale-x
    float uiScaleY;     // scale-y
    dred_font* pUIFont;

    drgui_color cmdbarBGColor;
    dred_font* pCmdbarTBFont;

    dred_font* pTextEditorFont;
    drgui_color textEditorBGColor;
} dred_config;

typedef void (* dred_config_on_error_proc)(dred_config* pConfig, const char* configPath, const char* message, unsigned int line, void* pUserData);

bool dred_config_init(dred_config* pConfig, dred_context* pDred);
void dred_config_uninit(dred_config* pConfig);

// The pUserData argument of onError will be set to pConfig.
bool dred_config_load_file(dred_config* pConfig, const char* filePath, dred_config_on_error_proc onError, void* pUserData);


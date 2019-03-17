
#include "../dred/dred_build_config.h"

typedef struct
{
    command_var* pCommandVars;
    config_var* pConfigVars;
} dred_build_context;

char* dred_build__generate_website__on_resolve_value(drwg_context* pWebgen, const char* name, const char* args, void* pUserData)
{
    assert(pWebgen != NULL);

    dred_build_context* pBuildContext = (dred_build_context*)pUserData;
    assert(pBuildContext != NULL);

    char* pResultStr = NULL;
    if (strcmp(name, "dred-command-list") == 0) {
        int count = stb_sb_count(pBuildContext->pCommandVars);
        for (int i = 0; i < count; ++i) {
            drwg_config subconfig;
            if (!drwg_config_init(&subconfig, &pWebgen->config)) {
                return NULL;
            }

            drwg_config_set(&subconfig, "dred-command-name", pBuildContext->pCommandVars[i].name);

            char* pCommandStr = drwg_context_process_file_with_config(pWebgen, &subconfig, "_drwebgen/template-command.html");
            if (pCommandStr != NULL) {
                pResultStr = drwg_append_string(pResultStr, pCommandStr);
                drwg_free_string(pCommandStr);
            }

            drwg_config_uninit(&subconfig);
        }

        return pResultStr;
    }

    if (strcmp(name, "dred-config-var-list") == 0) {
        int count = stb_sb_count(pBuildContext->pConfigVars);
        for (int i = 0; i < count; ++i) {
            drwg_config subconfig;
            if (!drwg_config_init(&subconfig, &pWebgen->config)) {
                return NULL;
            }

            drwg_config_set(&subconfig, "dred-config-var-name", pBuildContext->pConfigVars[i].name);
            drwg_config_set(&subconfig, "dred-config-var-documentation", pBuildContext->pConfigVars[i].documentation);
            drwg_config_set(&subconfig, "dred-config-var-type", pBuildContext->pConfigVars[i].typeSrc);
            drwg_config_set(&subconfig, "dred-config-var-default-value", pBuildContext->pConfigVars[i].defaultValueSrc);

            char* pConfigVarStr = drwg_context_process_file_with_config(pWebgen, &subconfig, "_drwebgen/template-config-var.html");
            if (pConfigVarStr != NULL) {
                pResultStr = drwg_append_string(pResultStr, pConfigVarStr);
                drwg_free_string(pConfigVarStr);
            }

            drwg_config_uninit(&subconfig);
        }

        return pResultStr;
    }

    return NULL;
}

void dred_build__generate_website__on_free_value(drwg_context* pWebgen, char* valueReturnedByOnResolveValue, void* pUserData)
{
    (void)pWebgen;
    (void)pUserData;
    drwg_free_string(valueReturnedByOnResolveValue);
}

void dred_build__generate_website__on_error(drwg_context* pWebgen, const char* message, void* pUserData)
{
    (void)pWebgen;
    (void)pUserData;
    printf("%s\n", message);
}

dr_bool32 dred_build__generate_website(command_var* pCommandVars, config_var* pConfigVars)
{
    dred_build_context context;
    context.pCommandVars = pCommandVars;
    context.pConfigVars = pConfigVars;

    drwg_context webgen;
    if (!drwg_context_init(&webgen, "../../../source/website", "../../../build/website", dred_build__generate_website__on_error, &context)) {
        return DR_FALSE;
    }

    webgen.onResolveValue = dred_build__generate_website__on_resolve_value;
    webgen.onFreeValue = dred_build__generate_website__on_free_value;


    // Properties.
    drwg_context_set(&webgen, "dred-version", DRED_VERSION_STRING);
    drwg_context_set(&webgen, "download-link-win64", "https://github.com/dr-soft/dred/releases/download/" DRED_VERSION_STRING "/dred.exe");

    dr_bool32 result = drwg_context_generate(&webgen);

    drwg_context_uninit(&webgen);
    return result;
}
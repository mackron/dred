
#include "../dred/dred_build_config.h"

typedef struct
{
    command_var* pCommandVars;
    config_var* pConfigVars;
} dred_build_context;

dtk_string dred_build__generate_website__on_resolve_value(dtk_webgen_context* pWebgen, const char* name, const char* args, void* pUserData)
{
    (void)args;
    assert(pWebgen != NULL);

    dred_build_context* pBuildContext = (dred_build_context*)pUserData;
    assert(pBuildContext != NULL);

    dtk_string pResultStr = NULL;
    if (strcmp(name, "dred-command-list") == 0) {
        int count = stb_sb_count(pBuildContext->pCommandVars);
        for (int i = 0; i < count; ++i) {
            dtk_webgen_config subconfig;
            if (!dtk_webgen_config_init(&subconfig, &pWebgen->config)) {
                return NULL;
            }

            dtk_webgen_config_set(&subconfig, "dred-command-name", pBuildContext->pCommandVars[i].name);

            dtk_string pCommandStr = dtk_webgen_context_process_file_with_config(pWebgen, &subconfig, "_drwebgen/template-command.html");
            if (pCommandStr != NULL) {
                pResultStr = dtk_append_string(pResultStr, pCommandStr);
                dtk_free_string(pCommandStr);
            }

            dtk_webgen_config_uninit(&subconfig);
        }

        return pResultStr;
    }

    if (strcmp(name, "dred-config-var-list") == 0) {
        int count = stb_sb_count(pBuildContext->pConfigVars);
        for (int i = 0; i < count; ++i) {
            dtk_webgen_config subconfig;
            if (!dtk_webgen_config_init(&subconfig, &pWebgen->config)) {
                return NULL;
            }

            dtk_webgen_config_set(&subconfig, "dred-config-var-name", pBuildContext->pConfigVars[i].name);
            dtk_webgen_config_set(&subconfig, "dred-config-var-documentation", pBuildContext->pConfigVars[i].documentation);
            dtk_webgen_config_set(&subconfig, "dred-config-var-type", pBuildContext->pConfigVars[i].typeSrc);
            dtk_webgen_config_set(&subconfig, "dred-config-var-default-value", pBuildContext->pConfigVars[i].defaultValueSrc);

            dtk_string pConfigVarStr = dtk_webgen_context_process_file_with_config(pWebgen, &subconfig, "_drwebgen/template-config-var.html");
            if (pConfigVarStr != NULL) {
                pResultStr = dtk_append_string(pResultStr, pConfigVarStr);
                dtk_free_string(pConfigVarStr);
            }

            dtk_webgen_config_uninit(&subconfig);
        }

        return pResultStr;
    }

    return NULL;
}

void dred_build__generate_website__on_free_value(dtk_webgen_context* pWebgen, dtk_string valueReturnedByOnResolveValue, void* pUserData)
{
    (void)pWebgen;
    (void)pUserData;
    dtk_free_string(valueReturnedByOnResolveValue);
}

void dred_build__generate_website__on_error(dtk_webgen_context* pWebgen, const char* message, void* pUserData)
{
    (void)pWebgen;
    (void)pUserData;
    printf("%s\n", message);
}

dtk_bool32 dred_build__generate_website(command_var* pCommandVars, config_var* pConfigVars)
{
    dred_build_context context;
    context.pCommandVars = pCommandVars;
    context.pConfigVars = pConfigVars;

    dtk_webgen_context webgen;
    if (!dtk_webgen_context_init(&webgen, "../../../source/website", "../../../build/website", dred_build__generate_website__on_error, &context)) {
        return DTK_FALSE;
    }

    webgen.onResolveValue = dred_build__generate_website__on_resolve_value;
    webgen.onFreeValue = dred_build__generate_website__on_free_value;


    // Properties.
    dtk_webgen_context_set(&webgen, "dred-version", DRED_VERSION_STRING);
    dtk_webgen_context_set(&webgen, "download-link-win64", "https://github.com/dr-soft/dred/releases/download/" DRED_VERSION_STRING "/dred.exe");

    dtk_bool32 result = dtk_webgen_context_generate(&webgen);

    dtk_webgen_context_uninit(&webgen);
    return result;
}
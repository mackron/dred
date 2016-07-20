
#include "../dred/dred_build_config.h"

bool dred_build__generate_website()
{
    drwg_context webgen;
    if (!drwg_context_init(&webgen, "../../../source/website", "../../../build/website")) {
        return false;
    }

    // Properties.
    drwg_context_set(&webgen, "dred-version", DRED_VERSION_STRING);

    bool result = drwg_context_generate(&webgen);

    drwg_context_uninit(&webgen);
    return result;
}
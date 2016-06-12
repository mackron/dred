
drgui_font_weight dred_parse_font_weight(const char* weight)
{
    if (strcmp(weight, "medium") == 0 || strcmp(weight, "normal") == 0 || strcmp(weight, "default") == 0) {
        return drgui_font_weight_normal;
    }
    if (strcmp(weight, "thin") == 0) {
        return drgui_font_weight_thin;
    }
    if (strcmp(weight, "extra-light") == 0) {
        return drgui_font_weight_extra_light;
    }
    if (strcmp(weight, "light") == 0) {
        return drgui_font_weight_light;
    }
    if (strcmp(weight, "semi-bold") == 0) {
        return drgui_font_weight_semi_bold;
    }
    if (strcmp(weight, "bold") == 0) {
        return drgui_font_weight_bold;
    }
    if (strcmp(weight, "extra-bold") == 0) {
        return drgui_font_weight_extra_bold;
    }
    if (strcmp(weight, "heavy") == 0) {
        return drgui_font_weight_heavy;
    }

    return drgui_font_weight_normal;
}

drgui_font_slant dred_parse_font_slant(const char* slant)
{
    if (strcmp(slant, "none") == 0 || strcmp(slant, "default") == 0) {
        return drgui_font_slant_none;
    }
    if (strcmp(slant, "italic") == 0) {
        return drgui_font_slant_italic;
    }
    if (strcmp(slant, "oblique") == 0) {
        return drgui_font_slant_oblique;
    }

    return drgui_font_slant_none;
}

drgui_color dred_parse_color(const char* color)
{
    if (color != NULL) {
        color = dr_first_non_whitespace(color);
        if (color[0] == '0' && (color[1] == 'x' || color[1] == 'X')) {
            // HTML style. Support both #RRGGBB and #RGB format.
            color += 2;

            unsigned int hexvals[6];

            int len = 0;
            for (int i = 0; i < 6; ++i) {
                if (!dr_hex_char_to_uint(color[i], &hexvals[i])) {
                    break;
                }
                len += 1;
            }

            drgui_byte r = 0;
            drgui_byte g = 0;
            drgui_byte b = 0;

            if (len == 3) {
                // #RGB -> #RRGGBB
                hexvals[5] = hexvals[2]; hexvals[4] = hexvals[2];
                hexvals[3] = hexvals[1]; hexvals[2] = hexvals[1];
                hexvals[1] = hexvals[0]; hexvals[0] = hexvals[0];
            }

            r = (hexvals[0] << 4) | hexvals[1];
            g = (hexvals[2] << 4) | hexvals[3];
            b = (hexvals[4] << 4) | hexvals[5];

            return drgui_rgb(r, g, b);
        } else {
            // R G B style (0 .. 255 per component)
            drgui_byte r = 0;
            drgui_byte g = 0;
            drgui_byte b = 0;

            char c[4];
            color = dr_next_token(color, c, sizeof(c));
            if (color != NULL) {
                r = (drgui_byte)atoi(c);
            }

            color = dr_next_token(color, c, sizeof(c));
            if (color != NULL) {
                g = (drgui_byte)atoi(c);
            }

            color = dr_next_token(color, c, sizeof(c));
            if (color != NULL) {
                b = (drgui_byte)atoi(c);
            }

            return drgui_rgb(r, g, b);
        }
    }
    
    return drgui_rgb(0, 0, 0);
}

dred_font* dred_config__load_system_font_ui(dred_context* pDred)
{
    dred_font_desc fontDesc;
    fontDesc.flags = 0;
    fontDesc.rotation = 0;

#ifdef _WIN32
    strcpy_s(fontDesc.family, sizeof(fontDesc.family), "Segoe UI");
    fontDesc.size = 12;
    fontDesc.weight = drgui_font_weight_normal;
    fontDesc.slant = drgui_font_slant_none;
#endif

#ifdef __linux__
#error "Implement me. Try using fontconfig for this to avoid dependency on GTK."
#endif

    return dred_font_library_create_font(&pDred->fontLibrary, fontDesc.family, fontDesc.size, fontDesc.weight, fontDesc.slant, fontDesc.rotation, fontDesc.flags);
}

dred_font* dred_config__load_system_font_mono(dred_context* pDred)
{
    dred_font_desc fontDesc;
    fontDesc.flags = 0;
    fontDesc.rotation = 0;

#ifdef _WIN32
    strcpy_s(fontDesc.family, sizeof(fontDesc.family), "Consolas");
    fontDesc.size = 13;
    fontDesc.weight = drgui_font_weight_normal;
    fontDesc.slant = drgui_font_slant_none;
#endif

#ifdef __linux__
#error "Implement me. Can use fontconfig for this."
#endif

    return dred_font_library_create_font(&pDred->fontLibrary, fontDesc.family, fontDesc.size, fontDesc.weight, fontDesc.slant, fontDesc.rotation, fontDesc.flags);
}

dred_font* dred_config__parse_and_load_font(dred_context* pDred, const char* value)
{
    // Check for pre-defined fonts first.
    if (strcmp(value, "system-font-ui") == 0) {
        return dred_config__load_system_font_ui(pDred);
    }
    if (strcmp(value, "system-font-mono") == 0) {
        return dred_config__load_system_font_mono(pDred);
    }


    // The format of the font string is <family> <size> <weight> <slant>. The weight and slant are optional and default to normal weight and
    // no slant.
    dred_font_desc fontDesc;
    fontDesc.flags = 0;
    fontDesc.rotation = 0;
    fontDesc.weight = drgui_font_weight_normal;
    fontDesc.slant = drgui_font_slant_none;

    // Family.
    value = dr_next_token(value, fontDesc.family, sizeof(fontDesc.family));
    if (value == NULL) {
        return NULL;
    }

    // Size.
    char token[256];
    value = dr_next_token(value, token, sizeof(token));
    if (value == NULL) {
        return NULL;
    }

    int size = atoi(token);
    if (size < 0) {
        size = -size;
    }

    fontDesc.size = size;

    // Weight.
    value = dr_next_token(value, token, sizeof(token));
    if (value != NULL) {
        fontDesc.weight = dred_parse_font_weight(token);

        // Slant.
        value = dr_next_token(value, token, sizeof(token));
        if (value != NULL) {
            fontDesc.slant = dred_parse_font_slant(token);
        }
    }

    
    return dred_font_library_create_font(&pDred->fontLibrary, fontDesc.family, fontDesc.size, fontDesc.weight, fontDesc.slant, fontDesc.rotation, fontDesc.flags);
}

bool dred_config_init(dred_config* pConfig, dred_context* pDred)
{
    if (pConfig == NULL || pDred == NULL) {
        return false;
    }

    pConfig->pDred = pDred;

    pConfig->pUIFont = dred_config__load_system_font_ui(pDred);
    pConfig->uiScaleX = 1;
    pConfig->uiScaleY = 1;

    pConfig->pCmdbarTBFont = dred_config__load_system_font_mono(pDred);
    pConfig->cmdbarBGColor = drgui_rgb(64, 64, 64);

    pConfig->pTextEditorFont = dred_config__load_system_font_mono(pDred);
    pConfig->textEditorBGColor = drgui_rgb(48, 48, 48);
    pConfig->textEditorActiveLineColor = drgui_rgb(32, 32, 32);

    return true;
}

void dred_config_uninit(dred_config* pConfig)
{
    if (pConfig == NULL) {
        return;
    }

    // Free any dynamically allocated data.
    dred_font_library_delete_font(&pConfig->pDred->fontLibrary, pConfig->pTextEditorFont);
    dred_font_library_delete_font(&pConfig->pDred->fontLibrary, pConfig->pCmdbarTBFont);
    dred_font_library_delete_font(&pConfig->pDred->fontLibrary, pConfig->pUIFont);
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
    if (strcmp(key, "ui-font") == 0) {
        pData->pConfig->pUIFont = dred_config__parse_and_load_font(pData->pConfig->pDred, value);
        return;
    }


    if (strcmp(key, "cmdbar-tb-font") == 0) {
        pData->pConfig->pCmdbarTBFont = dred_config__parse_and_load_font(pData->pConfig->pDred, value);
        return;
    }
    if (strcmp(key, "cmdbar-bg-color") == 0) {
        pData->pConfig->cmdbarBGColor = dred_parse_color(value);
        return;
    }


    if (strcmp(key, "texteditor-font") == 0) {
        pData->pConfig->pTextEditorFont = dred_config__parse_and_load_font(pData->pConfig->pDred, value);
        return;
    }
    if (strcmp(key, "texteditor-bg-color") == 0) {
        pData->pConfig->textEditorBGColor = dred_parse_color(value);
        return;
    }
    if (strcmp(key, "texteditor-active-line-color") == 0) {
        pData->pConfig->textEditorActiveLineColor = dred_parse_color(value);
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
    if (pConfig == NULL) {
        return false;
    }

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

    dred_file_close(file);
    return true;
}
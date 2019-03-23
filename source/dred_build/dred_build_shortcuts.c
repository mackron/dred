
typedef struct
{
    char id[256];
    char key[256];
    char cmd[256];
} dred_build__shortcut;

typedef struct
{
    dred_build__shortcut* pShortcuts;   // <-- stretchy_buffer.
} dred_build__shortcuts_context;

void dred_build__process_json_shortcuts(dred_build__shortcuts_context* pContext, struct json_array_s* pJSONArray)
{
    for (struct json_array_element_s* pJSONArrayElement = pJSONArray->start; pJSONArrayElement != NULL; pJSONArrayElement = pJSONArrayElement->next) {
        // Every element in the array should be an object. If it's not we just skip it with a warning.
        struct json_value_s* pJSONArrayElementValue = pJSONArrayElement->value;
        if (pJSONArrayElementValue->type == json_type_object) {
            struct json_object_s* pJSONObject = (struct json_object_s*)pJSONArrayElementValue->payload;

            struct json_object_element_s* pJSONObjectElement_ID  = NULL;
            struct json_object_element_s* pJSONObjectElement_Key = NULL;
            struct json_object_element_s* pJSONObjectElement_Cmd = NULL;
            for (struct json_object_element_s* pJSONObjectElement = pJSONObject->start; pJSONObjectElement != NULL; pJSONObjectElement = pJSONObjectElement->next) {
                if (strcmp(pJSONObjectElement->name->string, "id") == 0) {
                    pJSONObjectElement_ID = pJSONObjectElement;
                } else if (strcmp(pJSONObjectElement->name->string, "key") == 0) {
                    pJSONObjectElement_Key = pJSONObjectElement;
                } else if (strcmp(pJSONObjectElement->name->string, "cmd") == 0) {
                    pJSONObjectElement_Cmd = pJSONObjectElement;
                }
            }

            // Every element is required for shortcuts.
            if (pJSONObjectElement_ID == NULL) {
                printf("[MENUS] WARNING: Found a shortcut with no \"id\" property. Skipping.");
                continue;
            }
            if (pJSONObjectElement_Key == NULL) {
                printf("[MENUS] WARNING: Found a shortcut with no \"key\" property. Skipping.");
                continue;
            }
            if (pJSONObjectElement_Cmd == NULL) {
                printf("[MENUS] WARNING: Found a shortcut with no \"cmd\" property. Skipping.");
                continue;
            }

            dred_build__shortcut shortcut;
            strcpy_s(shortcut.id,  sizeof(shortcut.id),  ((struct json_string_s*)pJSONObjectElement_ID->value->payload)->string);
            strcpy_s(shortcut.key, sizeof(shortcut.key), ((struct json_string_s*)pJSONObjectElement_Key->value->payload)->string);
            strcpy_s(shortcut.cmd, sizeof(shortcut.id),  ((struct json_string_s*)pJSONObjectElement_Cmd->value->payload)->string);
            stb_sb_push(pContext->pShortcuts, shortcut);
        } else {
            printf("[SHORTCUTS] WARNING: Expecting an object.\n");   // TODO: Make this more descriptive.
        }
    }
}

void dred_build__format_shortcut_id_macro(const char* id, char* strOut)
{
    // - Prefixed with DRED_SHORTCUT_ID_
    // - All letters are converted to upper case
    // - Periods and dashes are replaced with underscores

    char tail[256];
    char* tailRunning = tail;

    for (;;) {
        if (*id == '\0') {
            break;
        }

        if (*id == '.' || *id == '-') {
            *tailRunning = '_';
        } else {
            *tailRunning = (char)toupper(*id);
        }

        tailRunning += 1;
        id += 1;
    }

    *tailRunning = '\0';

    strcpy(strOut, "DRED_SHORTCUT_ID_");
    strcat(strOut, tail);
}

void dred_build__format_shortcut_name_macro(const char* id, char* strOut)
{
    // - Prefixed with DRED_SHORTCUT_NAME_
    // - All letters are converted to upper case
    // - Periods and dashes are replaced with underscores

    char tail[256];
    char* tailRunning = tail;

    for (;;) {
        if (*id == '\0') {
            break;
        }

        if (*id == '.' || *id == '-') {
            *tailRunning = '_';
        } else {
            *tailRunning = (char)toupper(*id);
        }

        tailRunning += 1;
        id += 1;
    }

    *tailRunning = '\0';

    strcpy(strOut, "DRED_SHORTCUT_NAME_");
    strcat(strOut, tail);
}


void dred_build__format_accelerator_init_string(char* acceleratorInitStr, size_t acceleratorInitStrOut, dtk_accelerator accelerator)
{
    if (accelerator.key == 0 && accelerator.modifiers == 0) {
        snprintf(acceleratorInitStr, acceleratorInitStrOut, "dtk_accelerator_init(0, 0, 0)");
    } else {
        char keyStr[128] = { '\0' };
        dtk_key_to_string(accelerator.key, keyStr, sizeof(keyStr));

        // If the return key string is a word (as in it represents a special key like backspace or escape) just convert it to the
        // DTK key code by prefixing with "DTK_KEY_" and converting to upper case.
        if (strlen(keyStr) > 1) {
            char temp[128];
            int i = 0;
            while (keyStr[i] != '\0') {
                temp[i] = (char)toupper(keyStr[i]);
                i += 1;
            }

            temp[i] = '\0';
            snprintf(keyStr, sizeof(keyStr), "DTK_KEY_%s", temp);
        }


        char modifiersStr[256] = { '\0' };
        if (accelerator.modifiers & DTK_MODIFIER_SHIFT) {
            if (modifiersStr[0] != '\0') {
                dtk_strcat_s(modifiersStr, sizeof(modifiersStr), "|");
            }
            dtk_strcat_s(modifiersStr, sizeof(modifiersStr), "DTK_MODIFIER_SHIFT");
        }
        if (accelerator.modifiers & DTK_MODIFIER_CTRL) {
            if (modifiersStr[0] != '\0') {
                dtk_strcat_s(modifiersStr, sizeof(modifiersStr), "|");
            }
            dtk_strcat_s(modifiersStr, sizeof(modifiersStr), "DTK_MODIFIER_CTRL");
        }
        if (accelerator.modifiers & DTK_MODIFIER_ALT) {
            if (modifiersStr[0] != '\0') {
                dtk_strcat_s(modifiersStr, sizeof(modifiersStr), "|");
            }
            dtk_strcat_s(modifiersStr, sizeof(modifiersStr), "DTK_MODIFIER_ALT");
        }

        if (modifiersStr[0] == '\0') {
            modifiersStr[0] = '0';
            modifiersStr[1] = '\0';
        }

        if (strlen(keyStr) == 1) {
            snprintf(acceleratorInitStr, acceleratorInitStrOut, "dtk_accelerator_init('%s', %s, 0)", keyStr, modifiersStr);
        } else {
            snprintf(acceleratorInitStr, acceleratorInitStrOut, "dtk_accelerator_init(%s, %s, 0)", keyStr, modifiersStr);
        }
        
    }
}

void dred_build__format_shortcut_init_string(char* shortcutInitStr, size_t shortcutInitStrSize, const char* keyStr)
{
    // The key string is just an accelerator string. We just pass that using DTK.
    dtk_accelerator accelerators[4];
    dtk_uint32 acceleratorCount = sizeof(accelerators) / sizeof(accelerators[0]);
    if (dtk_accelerator_parse_chord(keyStr, accelerators, &acceleratorCount) != DTK_SUCCESS) {
        printf("[SHORTCUTS] ERROR: Failed to parse key combination: %s", keyStr);
        return;
    }

    // We are not currently supporting chords with more than 2 accelerators.
    if (acceleratorCount > 2) {
        printf("[SHORTCUTS] WARNING: dred currently only supports 2 accelerators per chord, but %d were specified for chord: %s", acceleratorCount, keyStr);
        return;
    }

    char accelerator0Str[256];
    dred_build__format_accelerator_init_string(accelerator0Str, sizeof(accelerator0Str), accelerators[0]);

    char accelerator1Str[256];
    if (acceleratorCount == 1) {
        dred_build__format_accelerator_init_string(accelerator1Str, sizeof(accelerator1Str), dtk_accelerator_init(0, 0, 0));
    } else {
        dred_build__format_accelerator_init_string(accelerator1Str, sizeof(accelerator1Str), accelerators[1]);
    }

    snprintf(shortcutInitStr, shortcutInitStrSize, "dred_shortcut_create(%s, %s)", accelerator0Str, accelerator1Str);
}

void dred_build__generate_shortcuts(FILE* pFileOut, FILE* pFileOutH, dtk_string_pool* pStringPool)
{
    (void)pFileOut;
    (void)pFileOutH;
    (void)pStringPool;

    dred_build__shortcuts_context context;
    context.pShortcuts = NULL;

    size_t shortcutsFileSize;
    char* pShortcutsFileData;
    dtk_result result = dtk_open_and_read_text_file("../../../resources/gui/dred_shortcuts.json", &shortcutsFileSize, &pShortcutsFileData);
    if (result != DTK_SUCCESS) {
        printf("ERROR: Could not find dred_shortcuts.json\n");
        return;
    }

    struct json_parse_result_s resultJSON;
    struct json_value_s* pJSON = json_parse_ex(pShortcutsFileData, shortcutsFileSize, json_parse_flags_allow_c_style_comments, NULL, NULL, &resultJSON);
    if (pJSON == NULL) {
        printf("dred_shortcuts.json (%u): %s\n", (unsigned int)resultJSON.error_line_no, dred_build__json_error_to_string(resultJSON.error));
        return;
    }

    struct json_object_s* pJSONRoot = (struct json_object_s*)pJSON->payload;
    if (strcmp(pJSONRoot->start->name->string, "shortcuts") != 0) {
        printf("dred_shortcuts.json: Expecting root element of \"shortcuts\" but found \"%s\"\n", pJSONRoot->start->name->string);
        return;
    }

    dred_build__process_json_shortcuts(&context, (struct json_array_s*)pJSONRoot->start->value->payload);
    
    free(pJSON);


    // Header file.
    fwrite_string(pFileOutH, "\n\n");
    fprintf(pFileOutH, "#define DRED_STOCK_SHORTCUT_COUNT %u\n", stb_sb_count(context.pShortcuts)+1); // +1 to include NONE
    fwrite_string(pFileOutH, "#define DRED_SHORTCUT_ID_NONE 0\n");
    int nextID = 1;
    for (int i = 0; i < stb_sb_count(context.pShortcuts); ++i) {
        char idMacroStr[256];
        dred_build__format_shortcut_id_macro(context.pShortcuts[i].id, idMacroStr);
        fprintf(pFileOutH, "#define %s %d\n", idMacroStr, nextID);
        nextID += 1;
    }
    fwrite_string(pFileOutH, "\n");

    fwrite_string(pFileOutH, "#define DRED_SHORTCUT_NAME_NONE \"\"\n");
    for (int i = 0; i < stb_sb_count(context.pShortcuts); ++i) {
        char nameMacroStr[256];
        dred_build__format_shortcut_name_macro(context.pShortcuts[i].id, nameMacroStr);
        fprintf(pFileOutH, "#define %s \"%s\"\n", nameMacroStr, context.pShortcuts[i].id);
        nextID += 1;
    }


    // Source file.
    fwrite_string(pFileOut, "\n\n");
    fwrite_string(pFileOut, "void dred_init_stock_shortcuts__autogenerated(dred_context* pDred)\n{\n");
    {
        fwrite_string(pFileOut, "    dtk_accelerator accelerators[4];\n\n");

        for (int i = 0; i < stb_sb_count(context.pShortcuts); ++i) {
            dred_build__shortcut* pShortcut = &context.pShortcuts[i];

            dtk_accelerator accelerators[4];
            dtk_uint32 acceleratorCount = sizeof(accelerators) / sizeof(accelerators[0]);
            if (dtk_accelerator_parse_chord(pShortcut->key, accelerators, &acceleratorCount) != DTK_SUCCESS) {
                printf("[SHORTCUTS] ERROR: Failed to parse key combination: %s", pShortcut->key);
                return;
            }

            for (size_t iAccel = 0; iAccel < acceleratorCount; ++iAccel) {
                char accelInitStr[256];
                dred_build__format_accelerator_init_string(accelInitStr, sizeof(accelInitStr), accelerators[0]);
                fprintf(pFileOut, "    accelerators[%u] = %s;\n", (unsigned int)iAccel, accelInitStr);
            }


            char* cmdStr = pShortcut->cmd;
            
            char idStr[256];
            dred_build__format_shortcut_id_macro(context.pShortcuts[i].id, idStr);

            char nameStr[256];
            dred_build__format_shortcut_name_macro(context.pShortcuts[i].id, nameStr);

            fprintf(pFileOut, "    dred_bind_shortcut(pDred, %s, %s, \"%s\", %u, accelerators);", idStr, nameStr, cmdStr, acceleratorCount);

            if (i != stb_sb_count(context.pShortcuts)-1) {
                fprintf(pFileOut, "\n\n");
            }
        }
    }
    fwrite_string(pFileOut, "\n}\n\n");


    // String pool.
    for (int iItem = 0; iItem < stb_sb_count(context.pShortcuts); ++iItem) {
        const char* id = context.pShortcuts[iItem].id;
        if (id[0] != '\0' && !dtk_string_pool_find(pStringPool, id, NULL)) {
            dtk_string_pool_add(pStringPool, id, (size_t)-1);
        }

        const char* cmd = context.pShortcuts[iItem].cmd;
        if (cmd[0] != '\0' && !dtk_string_pool_find(pStringPool, cmd, NULL)) {
            dtk_string_pool_add(pStringPool, cmd, (size_t)-1);
        }
    }
}
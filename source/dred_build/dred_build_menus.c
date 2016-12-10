typedef struct
{
    char id[256];
    char type[256];
    char text[256];
    char shortcut[256]; // Can be empty.
    char cmd[256];      // Can be empty.
    char submenu[256];
} dred_build__menu_item;

typedef struct
{
    char id[256];
    char type[256];     // Can be "menubar", "popup" or empty in which case it defaults to "popup"
    int* pItemsIndices;  // stretchy_buffer
} dred_build__menu;

typedef struct
{
    dred_build__menu_item* pAllItems;
    dred_build__menu* pAllMenus;
} dred_build__menu_parse_context;

void dred_build__process_json_menus(dred_build__menu_parse_context* pContext, struct json_array_s* pJSONArray)
{
    // The format of the JSON file is simple. There is a global "menus" object with an array of sub-objects. These sub-objects
    // define each menu (both top-level and sub-menus).
    //
    // Each menu is listed in a flat hierarchy which means no complex hierarchy management (yay!). Each menu will have an
    // "items" element which is where each of the menu's items are defined. Each item can have a sub-menu, which is defined
    // by the "submenu" element. If an item's id is set to "separator" it will be treated as a separator.
    for (struct json_array_element_s* pJSONMenuArrayElement = pJSONArray->start; pJSONMenuArrayElement != NULL; pJSONMenuArrayElement = pJSONMenuArrayElement->next) {
        // Every element in the array should be an object. If it's not we just skip it with a warning.
        struct json_value_s* pJSONMenuArrayElementValue = pJSONMenuArrayElement->value;
        if (pJSONMenuArrayElementValue->type == json_type_object) {
            struct json_object_s* pJSONMenuObject = (struct json_object_s*)pJSONMenuArrayElementValue->payload;

            struct json_object_element_s* pJSONObjectElement_ID       = NULL;
            struct json_object_element_s* pJSONObjectElement_Type     = NULL;
            struct json_object_element_s* pJSONObjectElement_Items    = NULL;
            for (struct json_object_element_s* pJSONMenuObjectElement = pJSONMenuObject->start; pJSONMenuObjectElement != NULL; pJSONMenuObjectElement = pJSONMenuObjectElement->next) {
                if (strcmp(pJSONMenuObjectElement->name->string, "id") == 0) {
                    pJSONObjectElement_ID = pJSONMenuObjectElement;
                } else if (strcmp(pJSONMenuObjectElement->name->string, "type") == 0) {
                    pJSONObjectElement_Type = pJSONMenuObjectElement;
                } else if (strcmp(pJSONMenuObjectElement->name->string, "items") == 0) {
                    pJSONObjectElement_Items = pJSONMenuObjectElement;
                }
            }

            // Every menu needs an ID.
            if (pJSONObjectElement_ID == NULL || pJSONObjectElement_ID->value->type != json_type_string) {
                printf("[MENUS] WARNING: Found a menu with no \"id\" property. Skipping.");
                continue;   // No ID. Skip.
            }

            // Every menu needs a list of items.
            if (pJSONObjectElement_Items == NULL || pJSONObjectElement_Items->value->type != json_type_array) {
                printf("[MENUS] WARNING: Found a menu with no items. Skipping.");
                continue;   // No items. Skip.
            }

            dred_build__menu menu;
            memset(&menu, 0, sizeof(menu));
            strcpy_s(menu.id, sizeof(menu.id), ((struct json_string_s*)pJSONObjectElement_ID->value->payload)->string);

            if (pJSONObjectElement_Type != NULL && pJSONObjectElement_Type->value->type == json_type_string) {
                strcpy_s(menu.type, sizeof(menu.type), ((struct json_string_s*)pJSONObjectElement_Type->value->payload)->string);
            } else {
                strcpy_s(menu.type, sizeof(menu.type), "popup");
            }


            // Items.
            for (struct json_array_element_s* pJSONItemArrayElement = ((struct json_array_s*)pJSONObjectElement_Items->value->payload)->start; pJSONItemArrayElement != NULL; pJSONItemArrayElement = pJSONItemArrayElement->next) {
                struct json_value_s* pJSONItemArrayElementValue = pJSONItemArrayElement->value;
                if (pJSONItemArrayElementValue->type == json_type_object) {
                    struct json_object_s* pJSONItemObject = (struct json_object_s*)pJSONItemArrayElementValue->payload;

                    pJSONObjectElement_ID   = NULL;
                    pJSONObjectElement_Type = NULL;
                    struct json_object_element_s* pJSONObjectElement_Text     = NULL;
                    struct json_object_element_s* pJSONObjectElement_Shortcut = NULL;
                    struct json_object_element_s* pJSONObjectElement_Cmd      = NULL;
                    struct json_object_element_s* pJSONObjectElement_Submenu  = NULL;
                    for (struct json_object_element_s* pJSONItemObjectElement = pJSONItemObject->start; pJSONItemObjectElement != NULL; pJSONItemObjectElement = pJSONItemObjectElement->next) {
                        if (strcmp(pJSONItemObjectElement->name->string, "id") == 0) {
                            pJSONObjectElement_ID = pJSONItemObjectElement;
                        } else if (strcmp(pJSONItemObjectElement->name->string, "type") == 0) {
                            pJSONObjectElement_Type = pJSONItemObjectElement;
                        } else if (strcmp(pJSONItemObjectElement->name->string, "text") == 0) {
                            pJSONObjectElement_Text = pJSONItemObjectElement;
                        } else if (strcmp(pJSONItemObjectElement->name->string, "shortcut") == 0) {
                            pJSONObjectElement_Shortcut = pJSONItemObjectElement;
                        } else if (strcmp(pJSONItemObjectElement->name->string, "cmd") == 0) {
                            pJSONObjectElement_Cmd = pJSONItemObjectElement;
                        } else if (strcmp(pJSONItemObjectElement->name->string, "submenu") == 0) {
                            pJSONObjectElement_Submenu = pJSONItemObjectElement;
                        }
                    }

                    // Every menu item needs an ID.
                    if (pJSONObjectElement_ID == NULL || pJSONObjectElement_ID->value->type != json_type_string) {
                        printf("[MENUS] WARNING: Found a menu with no \"id\" property. Skipping.");
                        continue;   // No ID. Skip.
                    }

                    dred_build__menu_item item;
                    memset(&item, 0, sizeof(item));
                    strcpy_s(item.id, sizeof(item.id), ((struct json_string_s*)pJSONObjectElement_ID->value->payload)->string);

                    if (pJSONObjectElement_Type != NULL && pJSONObjectElement_Type->value->type == json_type_string) {
                        strcpy_s(item.type, sizeof(item.type), ((struct json_string_s*)pJSONObjectElement_Type->value->payload)->string);
                    } else {
                        if (strcmp(item.id, "separator") == 0) {
                            strcpy_s(item.type, sizeof(item.type), "separator");
                        } else {
                            strcpy_s(item.type, sizeof(item.type), "normal");
                        }
                    }

                    if (strcmp(item.type, "normal") != 0 &&
                        strcmp(item.type, "check") != 0 &&
                        strcmp(item.type, "separator") != 0) {
                        printf("[MENUS] WARNING: Found a menu with in invalid \"type\" property (\"%s\"). Skipping.", item.type);
                        continue;   // Invalid type. Skip.
                    }


                    if (pJSONObjectElement_Text != NULL && pJSONObjectElement_Text->value->type == json_type_string) {
                        strcpy_s(item.text, sizeof(item.text), ((struct json_string_s*)pJSONObjectElement_Text->value->payload)->string);
                    }
                    if (pJSONObjectElement_Shortcut != NULL && pJSONObjectElement_Shortcut->value->type == json_type_string) {
                        strcpy_s(item.shortcut, sizeof(item.shortcut), ((struct json_string_s*)pJSONObjectElement_Shortcut->value->payload)->string);
                    }
                    if (pJSONObjectElement_Cmd != NULL && pJSONObjectElement_Cmd->value->type == json_type_string) {
                        strcpy_s(item.cmd, sizeof(item.cmd), ((struct json_string_s*)pJSONObjectElement_Cmd->value->payload)->string);
                    }
                    if (pJSONObjectElement_Submenu != NULL && pJSONObjectElement_Submenu->value->type == json_type_string) {
                        strcpy_s(item.submenu, sizeof(item.submenu), ((struct json_string_s*)pJSONObjectElement_Submenu->value->payload)->string);
                    }

                    stb_sb_push(menu.pItemsIndices, stb_sb_count(pContext->pAllItems));
                    stb_sb_push(pContext->pAllItems, item);
                }
            }

            stb_sb_push(pContext->pAllMenus, menu);\
        } else {
            printf("[MENUS] WARNING: Expecting an object.\n");   // TODO: Make this more descriptive.
        }
    }
}

void dred_build__format_menu_name(const char* id, char* nameOut)
{
    dr_bool32 isFirstWord = DR_TRUE;
    dr_bool32 atStartOfWord = DR_TRUE;

    for (;;) {
        if (*id == '\0') {
            break;
        }

        if (*id == '.' || *id == '-') {
            isFirstWord = DR_FALSE;
            atStartOfWord = DR_TRUE;
            id += 1;
            continue;
        }

        if (atStartOfWord && !isFirstWord) {
            *nameOut = (char)toupper(*id);
        } else {
            *nameOut = *id;
        }

        nameOut += 1;
        id += 1;
        atStartOfWord = DR_FALSE;
    }

    *nameOut = '\0';
}

void dred_build__format_menu_item_id_macro(const char* id, char* strOut)
{
    // - Prefixed with DRED_MENU_ITEM_ID_
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

    strcpy(strOut, "DRED_MENU_ITEM_ID_");
    strcat(strOut, tail);
}

dr_bool32 dred_build__find_submenu_index_by_id(dred_build__menu_parse_context* pContext, const char* id, int* pIndexOut)
{
    for (int i = 0; i < stb_sb_count(pContext->pAllMenus); ++i) {
        if (strcmp(pContext->pAllMenus[i].id, id) == 0) {
            *pIndexOut = i;
            return DR_TRUE;
        }
    }

    return DR_FALSE;
}

void dred_build__sort_menus_by_init_order(dred_build__menu_parse_context* pContext, int* pMenuOrder)
{
    // TODO: Implement me.
    (void)pContext;
    (void)pMenuOrder;
}

void dred_build__generate_menus(FILE* pFileOut, FILE* pFileOutH, dred_string_pool* pStringPool)
{
    dred_build__menu_parse_context context;
    context.pAllItems = NULL;
    context.pAllMenus = NULL;

    size_t menusFileSize;
    char* menusFileData = dr_open_and_read_text_file("../../../resources/gui/dred_menus.json", &menusFileSize);
    if (menusFileData == NULL) {
        printf("ERROR: Could not find dred_menus.json\n");
        return;
    }

    struct json_parse_result_s resultJSON;
    struct json_value_s* pJSON = json_parse_ex(menusFileData, menusFileSize, json_parse_flags_allow_c_style_comments, NULL, NULL, &resultJSON);
    if (pJSON == NULL) {
        printf("dred_menus.json (%d): %s\n", resultJSON.error_line_no, dred_build__json_error_to_string(resultJSON.error));
        return;
    }

    struct json_object_s* pJSONRoot = (struct json_object_s*)pJSON->payload;
    if (strcmp(pJSONRoot->start->name->string, "menus") != 0) {
        printf("dred_menus.json: Expecting root element of \"menus\" but found \"%s\"\n", pJSONRoot->start->name->string);
        return;
    }

    dred_build__process_json_menus(&context, (struct json_array_s*)pJSONRoot->start->value->payload);
    
    free(pJSON);

    
    // Header file.
    fwrite_string(pFileOutH, "\n\n");
    fprintf(pFileOutH, "#define DRED_STOCK_MENU_ITEM_COUNT %u\n", stb_sb_count(context.pAllItems)+2);   // +2 to include NONE and SEPARATOR
    fwrite_string(pFileOutH, "#define DRED_MENU_ITEM_ID_NONE 0\n");
    fwrite_string(pFileOutH, "#define DRED_MENU_ITEM_ID_SEPARATOR 1\n");
    int nextID = 2;
    for (int i = 0; i < stb_sb_count(context.pAllItems); ++i) {
        if (strcmp(context.pAllItems[i].id, "separator") != 0) {
            char idMacroStr[256];
            dred_build__format_menu_item_id_macro(context.pAllItems[i].id, idMacroStr);
            fprintf(pFileOutH, "#define %s %d\n", idMacroStr, nextID);
            nextID += 1;
        }
    }

    fwrite_string(pFileOutH, "\n");
    fwrite_string(pFileOutH, "typedef struct\n{\n");
    {
        // The dred_stock_menus structure requires a dtk_menu object for each menu. The name is based on the id and
        // formatted as "theFileMenu". For example, a menu with an id of "file.open" will be named as "fileOpen". The
        // order these are declared does not matter.
        for (int i = 0; i < stb_sb_count(context.pAllMenus); ++i) {
            char formattedName[256];
            dred_build__format_menu_name(context.pAllMenus[i].id, formattedName);

            fprintf(pFileOutH, "    dtk_menu %s;\n", formattedName);
        }
    }
    fwrite_string(pFileOutH, "} dred_stock_menus;\n\n");


    // Source file.
    fwrite_string(pFileOut, "\n\n");
    fwrite_string(pFileOut, "void dred_init_stock_menus__autogenerated(dred_context* pDred)\n{\n");
    {
        fwrite_string(pFileOut, "    dtk_menu_item_info item;\n");

        // When initializing menus we need to ensure all sub-menus are done first. To do this we just create
        // a list of menu indices and sort them.
        int* pMenuOrder = (int*)malloc(sizeof(*pMenuOrder) * stb_sb_count(context.pAllMenus));
        if (pMenuOrder != NULL) {
            for (int i = 0; i < stb_sb_count(pMenuOrder); ++i) {
                pMenuOrder[i] = i;
            }
        }

        dred_build__sort_menus_by_init_order(&context, pMenuOrder);

        for (int iMenu = 0; iMenu < stb_sb_count(context.pAllMenus); ++iMenu) {
            dred_build__menu* pMenu = &context.pAllMenus[iMenu];
            
            char formattedName[256];
            dred_build__format_menu_name(pMenu->id, formattedName);

            fprintf(pFileOut, "\n");
            fprintf(pFileOut, "    dtk_menu_init(&pDred->tk, dtk_menu_type_%s, &pDred->menus.%s);\n    {", pMenu->type, formattedName);
            for (int iItem = 0; iItem < stb_sb_count(pMenu->pItemsIndices); ++iItem) {
                dred_build__menu_item* pItem = &context.pAllItems[pMenu->pItemsIndices[iItem]];

                char idMacroStr[256];
                dred_build__format_menu_item_id_macro(pItem->id, idMacroStr);

                char submenuFormattedName[256];
                dred_build__format_menu_name(pItem->submenu, submenuFormattedName);

                fprintf(pFileOut, "\n");
                fprintf(pFileOut, "        item.id = %s;\n", idMacroStr);
                fprintf(pFileOut, "        item.type = dtk_menu_item_type_%s;\n", pItem->type);
                fprintf(pFileOut, "        item.stateFlags = 0;\n");
                if (submenuFormattedName[0] == '\0') {
                    fprintf(pFileOut, "        item.pSubMenu = NULL;\n");
                } else {
                    fprintf(pFileOut, "        item.pSubMenu = &pDred->menus.%s;\n", submenuFormattedName);
                }
                fprintf(pFileOut, "        item.text = \"%s\";\n", pItem->text);
                fprintf(pFileOut, "        item.shortcut = NULL;\n");
                fprintf(pFileOut, "        dtk_menu_append_item(&pDred->menus.%s, &item);", formattedName);

                // Blank line between items just for readability during testing.
                if (iItem != stb_sb_count(pMenu->pItemsIndices)-1) {
                    fprintf(pFileOut, "\n");
                }
            }
            fprintf(pFileOut, "\n    }");

            if (iMenu != stb_sb_count(context.pAllMenus)-1) {
                fprintf(pFileOut, "\n");
            }
        }

        free(pMenuOrder);
    }
    fwrite_string(pFileOut, "\n}\n\n");


    // The strings for each menu item's command needs to be added to the main string pool. dred will use this as the storage
    // for the command strings for menu items.
    for (int iItem = 0; iItem < stb_sb_count(context.pAllItems); ++iItem) {
        const char* cmd = context.pAllItems[iItem].cmd;
        if (cmd[0] != '\0' && !dred_string_pool_find(pStringPool, cmd, NULL)) {
            dred_string_pool_add(pStringPool, cmd, (size_t)-1);
        }
    }
}
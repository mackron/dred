// Copyright (C) 2017 David Reid. See included LICENSE file.

void dred_refresh_recent_files_menu(dred_context* pDred)
{
    // To refresh the list, we just overwrite any existing items, and then remove the excess, if any.
    dtk_uint32 newCount = (dtk_uint32)pDred->config.recentFileCount;
    dtk_uint32 oldCount;
    dtk_menu_get_item_count(&pDred->menus.recentFiles, &oldCount);

    // Replace existing items.
    for (dtk_uint32 iItem = 0; iItem < newCount && iItem < oldCount; ++iItem) {
        char cmdStr[DRED_MAX_PATH];
        if (snprintf(cmdStr, sizeof(cmdStr), "open \"%s\"", pDred->config.recentFiles[iItem]) < 0) {
            continue;   // Failed to make the command string. Probably because the buffer is too small.
        }

        dtk_uint32 itemID = DRED_MENU_ITEM_ID_RECENT_FILE_0 + iItem;

        dtk_menu_set_item_text(&pDred->menus.recentFiles, dtk_menu_find_item_by_id(&pDred->menus.recentFiles, itemID), pDred->config.recentFiles[iItem]);
        dred_menu_item_table_bind(&pDred->menuItemTable, itemID, cmdStr, NULL);
    }

    // Either insert or remove extra items.
    if (newCount > oldCount) {
        // Insert new items.
        for (dtk_uint32 iItem = oldCount; iItem < newCount; ++iItem) {
            char cmdStr[DRED_MAX_PATH];
            if (snprintf(cmdStr, sizeof(cmdStr), "open \"%s\"", pDred->config.recentFiles[iItem]) < 0) {
                continue;   // Failed to make the command string. Probably because the buffer is too small.
            }

            dtk_uint32 itemID = DRED_MENU_ITEM_ID_RECENT_FILE_0 + iItem;

            // Create the menu item.
            dtk_menu_item_info info;
            memset(&info, 0, sizeof(info));
            info.id = itemID;
            info.text = pDred->config.recentFiles[iItem];
            dtk_menu_append_item(&pDred->menus.recentFiles, &info);

            // Bind the menu item to a command.
            dred_menu_item_table_bind(&pDred->menuItemTable, itemID, cmdStr, NULL);
        }
    } else {
        // Remove excess.
        dtk_uint32 iItemToRemove = newCount;
        for (dtk_uint32 iItem = newCount; iItem < oldCount; ++iItem) {
            dred_menu_item_table_unbind(&pDred->menuItemTable, DRED_MENU_ITEM_ID_RECENT_FILE_0 + iItem);
            dtk_menu_remove_item(&pDred->menus.recentFiles, iItemToRemove);
        }
    }

    // Enable or disable the relevant menu items depending on how many recent files we have.
    if (pDred->config.recentFileCount > 0) {
        dtk_menu_enable_item(&pDred->menus.textFile, dtk_menu_find_item_by_id(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_OPEN_RECENT));
        dtk_menu_enable_item(&pDred->menus.nothingopenFile, dtk_menu_find_item_by_id(&pDred->menus.nothingopenFile, DRED_MENU_ITEM_ID_NOTHINGOPEN_FILE_OPEN_RECENT));
    } else {
        dtk_menu_disable_item(&pDred->menus.textFile, dtk_menu_find_item_by_id(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_OPEN_RECENT));
        dtk_menu_disable_item(&pDred->menus.nothingopenFile, dtk_menu_find_item_by_id(&pDred->menus.nothingopenFile, DRED_MENU_ITEM_ID_NOTHINGOPEN_FILE_OPEN_RECENT));
    }
}

void dred_refresh_favourite_files_menu(dred_context* pDred)
{
    // To refresh the list, we just overwrite any existing items, and then remove the excess, if any.
    dtk_uint32 newCount = (dtk_uint32)pDred->config.favouriteFileCount;
    dtk_uint32 oldCount;
    dtk_menu_get_item_count(&pDred->menus.favouriteFiles, &oldCount);

    // Replace existing items.
    for (dtk_uint32 iItem = 0; iItem < newCount && iItem < oldCount; ++iItem) {
        char cmdStr[DRED_MAX_PATH];
        if (snprintf(cmdStr, sizeof(cmdStr), "open \"%s\"", pDred->config.favouriteFiles[iItem]) < 0) {
            continue;   // Failed to make the command string. Probably because the buffer is too small.
        }

        dtk_uint32 itemID = DRED_MENU_ITEM_ID_FAVOURITE_FILE_0 + iItem;

        dtk_menu_set_item_text(&pDred->menus.favouriteFiles, dtk_menu_find_item_by_id(&pDred->menus.favouriteFiles, itemID), pDred->config.favouriteFiles[iItem]);
        dred_menu_item_table_bind(&pDred->menuItemTable, itemID, cmdStr, NULL);
    }

    // Either insert or remove extra items.
    if (newCount > oldCount) {
        // Insert new items.
        for (dtk_uint32 iItem = oldCount; iItem < newCount; ++iItem) {
            char cmdStr[DRED_MAX_PATH];
            if (snprintf(cmdStr, sizeof(cmdStr), "open \"%s\"", pDred->config.favouriteFiles[iItem]) < 0) {
                continue;   // Failed to make the command string. Probably because the buffer is too small.
            }

            dtk_uint32 itemID = DRED_MENU_ITEM_ID_FAVOURITE_FILE_0 + iItem;

            // Create the menu item.
            dtk_menu_item_info info;
            memset(&info, 0, sizeof(info));
            info.id = itemID;
            info.text = pDred->config.favouriteFiles[iItem];
            dtk_menu_append_item(&pDred->menus.favouriteFiles, &info);

            // Bind the menu item to a command.
            dred_menu_item_table_bind(&pDred->menuItemTable, itemID, cmdStr, NULL);
        }
    } else {
        // Remove excess.
        dtk_uint32 iItemToRemove = newCount;
        for (dtk_uint32 iItem = newCount; iItem < oldCount; ++iItem) {
            dred_menu_item_table_unbind(&pDred->menuItemTable, DRED_MENU_ITEM_ID_FAVOURITE_FILE_0 + iItem);
            dtk_menu_remove_item(&pDred->menus.favouriteFiles, iItemToRemove);
        }
    }

    // Enable or disable the relevant menu items depending on how many recent files we have.
    if (pDred->config.favouriteFileCount > 0) {
        dtk_menu_enable_item(&pDred->menus.textFile, dtk_menu_find_item_by_id(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_OPEN_FAVOURITE));
        dtk_menu_enable_item(&pDred->menus.nothingopenFile, dtk_menu_find_item_by_id(&pDred->menus.nothingopenFile, DRED_MENU_ITEM_ID_NOTHINGOPEN_FILE_OPEN_FAVOURITE));
    } else {
        dtk_menu_disable_item(&pDred->menus.textFile, dtk_menu_find_item_by_id(&pDred->menus.textFile, DRED_MENU_ITEM_ID_TEXT_FILE_OPEN_FAVOURITE));
        dtk_menu_disable_item(&pDred->menus.nothingopenFile, dtk_menu_find_item_by_id(&pDred->menus.nothingopenFile, DRED_MENU_ITEM_ID_NOTHINGOPEN_FILE_OPEN_FAVOURITE));
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

dr_bool32 dred_is_stock_menu_item_id(dtk_uint32 id)
{
    if (id < DRED_STOCK_MENU_ITEM_COUNT) {
        return DR_TRUE;
    }

    return DR_FALSE;
}

dred_result dred_menu_item_table_init(dred_context* pDred, dred_menu_item_table* pTable)
{
    if (pTable == NULL) return DRED_INVALID_ARGS;
    dtk_zero_object(pTable);

    if (pDred == NULL) return DRED_INVALID_ARGS;
    pTable->pDred = pDred;

    // The capacity should always be set to _at least_ the stock menu item count. The reason for this is that we use an optimized
    // constant-time lookup for stock menu items.
    pTable->capacity = DRED_STOCK_MENU_ITEM_COUNT;  
    if (pTable->capacity != 0) {
        pTable->pItems = (dred_menu_item_data*)malloc(pTable->capacity * sizeof(*pTable->pItems));
        if (pTable->pItems == NULL) {
            return DRED_OUT_OF_MEMORY;
        }
    }

    // Stock menu items need to be bound. This is an optimized binding with the assumption that the internal buffer has already
    // been allocated with enough space.
    dred_menu_item_table__bind_stock_items__autogenerated(pDred, pTable);
    pTable->count = DRED_STOCK_MENU_ITEM_COUNT;

    return DTK_SUCCESS;
}

dred_result dred_menu_item_table_uninit(dred_menu_item_table* pTable)
{
    if (pTable == NULL) return DRED_INVALID_ARGS;

    free(pTable->pItems);
    return DRED_SUCCESS;
}

dred_result dred_menu_item_table_bind(dred_menu_item_table* pTable, dtk_uint32 id, const char* commandStr, const char* shortcutName)
{
    if (pTable == NULL) return DRED_INVALID_ARGS;

    size_t index;
    if (!dred_menu_item_table_find(pTable, id, &index)) {
        // The item does not exist. Append.
        if (pTable->count == pTable->capacity) {
            size_t newCapacity = (pTable->capacity == 0) ? 64 : (pTable->capacity * 2);
            dred_menu_item_data* pNewItems = (dred_menu_item_data*)realloc(pTable->pItems, newCapacity * sizeof(*pNewItems));
            if (pNewItems == NULL) {
                return DRED_OUT_OF_MEMORY;
            }

            pTable->pItems = pNewItems;
            pTable->capacity = newCapacity;
        }

        assert(pTable->count < pTable->capacity);

        index = pTable->count;
        pTable->pItems[index].id = id;
        pTable->count += 1;
    }

    pTable->pItems[index].commandStrOffset = dred_string_pool_find_or_add(&pTable->pDred->stringPool, commandStr);
    pTable->pItems[index].shortcutStrOffset = dred_string_pool_find_or_add(&pTable->pDred->stringPool, shortcutName);

    return DRED_SUCCESS;
}

dred_result dred_menu_item_table_unbind(dred_menu_item_table* pTable, dtk_uint32 id)
{
    // We don't _actually_ unbind stock menu items because we want to use an optimized constant-time lookup for those ones
    // since we know that their range and that they're contiguous. We just return successful and pretend it was unbound.
    if (dred_is_stock_menu_item_id(id)) {
        pTable->pItems[id].commandStrOffset = 0;
        pTable->pItems[id].shortcutStrOffset = 0;
        return DRED_SUCCESS;
    }

    size_t index;
    if (dred_menu_item_table_find(pTable, id, &index)) {
        assert(index >= DRED_STOCK_MENU_ITEM_COUNT);
        
        for (size_t i = index; i < pTable->count-1; ++i) {
            pTable->pItems[i] = pTable->pItems[i+1];
        }
        pTable->count -= 1;

        return DRED_SUCCESS;
    }

    return DRED_ERROR;  // Not found.
}

dred_result dred_menu_item_table_update_bindings_by_shortcut_name(dred_menu_item_table* pTable, const char* shortcutName, const char* commandStr)
{
    if (pTable == NULL) return DRED_INVALID_ARGS;

    for (size_t iItem = 0; iItem < pTable->count; ++iItem) {
        dred_menu_item_data* pItem = &pTable->pItems[iItem];
        if (strcmp(dred_string_pool_cstr(&pTable->pDred->stringPool, pItem->shortcutStrOffset), shortcutName) == 0) {
            pItem->commandStrOffset = dred_string_pool_find_or_add(&pTable->pDred->stringPool, commandStr);
        }
    }

    return DTK_SUCCESS;
}

const char* dred_menu_item_table_get_command(dred_menu_item_table* pTable, dtk_uint32 id)
{
    size_t index;
    if (!dred_menu_item_table_find(pTable, id, &index)) {
        return NULL;
    }

    return dred_string_pool_cstr(&pTable->pDred->stringPool, pTable->pItems[index].commandStrOffset);
}

const char* dred_menu_item_table_get_shortcut(dred_menu_item_table* pTable, dtk_uint32 id)
{
    size_t index;
    if (!dred_menu_item_table_find(pTable, id, &index)) {
        return NULL;
    }

    return dred_string_pool_cstr(&pTable->pDred->stringPool, pTable->pItems[index].shortcutStrOffset);
}

dr_bool32 dred_menu_item_table_find(dred_menu_item_table* pTable, dtk_uint32 id, size_t* pIndex)
{
    if (pIndex) *pIndex = (size_t)-1;
    if (pTable == NULL) return DR_FALSE;

    if (dred_is_stock_menu_item_id(id)) {
        if (pIndex) *pIndex = id;
        return DR_TRUE;
    }

    for (size_t i = DRED_STOCK_MENU_ITEM_COUNT; i < pTable->count; ++i) {
        if (pTable->pItems[i].id == id) {
            if (pIndex) *pIndex = i;
            return DR_TRUE;
        }
    }

    return DR_FALSE;
}
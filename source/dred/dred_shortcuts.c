// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_bool32 dred_shortcut_table_init(dred_context* pDred, dred_shortcut_table* pTable, size_t initialCapacity)
{
    if (pTable == NULL) return DTK_FALSE;
    memset(pTable, 0, sizeof(*pTable));
    
    pTable->pDred = pDred;
    pTable->count = 0;
    pTable->bufferSize = initialCapacity;

    if (initialCapacity > 0) {
        pTable->pShortcuts = (dred_shortcut*)realloc(pTable->pShortcuts, pTable->bufferSize * sizeof(*pTable->pShortcuts));
        if (pTable->pShortcuts == NULL) {
            return DTK_FALSE;
        }
    }

    //if (!dred_accelerator_table_init(&pTable->acceleratorTable)) {
    //    return DTK_FALSE;
    //}

    return DTK_TRUE;
}

void dred_shortcut_table_uninit(dred_shortcut_table* pTable)
{
    if (pTable == NULL) {
        return;
    }

    free(pTable->pShortcuts);
    pTable->count = 0;
    pTable->bufferSize = 0;

    //dred_accelerator_table_uninit(&pTable->acceleratorTable);
}

dtk_bool32 dred_shortcut_table_bind(dred_shortcut_table* pTable, dtk_uint32 id, const char* name, const char* cmdStr, dtk_uint32 acceleratorCount, dtk_accelerator* pAccelerators)
{
    if (pTable == NULL) {
        return DTK_FALSE;
    }

    if (name == NULL) {
        name = "";
    }


    // If an accelerator with the same key combination already exists, just replace the command.
    size_t existingIndex;
    if (dred_shortcut_table_find(pTable, id, &existingIndex)) {
        dred_shortcut_table_replace_by_index(pTable, existingIndex, name, cmdStr, acceleratorCount, pAccelerators);
        return DTK_TRUE;   // Already exists.
    }

    // If we get here it means the accelerator does not already exist and needs to be added.
    if (pTable->count == pTable->bufferSize) {
        size_t newBufferSize = (pTable->bufferSize == 0) ? 16 : (pTable->bufferSize * 2);
        dred_shortcut* pNewShortcuts = (dred_shortcut*)realloc(pTable->pShortcuts, newBufferSize * sizeof(*pNewShortcuts));
        if (pNewShortcuts == NULL) {
            return DTK_FALSE;
        }

        pTable->pShortcuts = pNewShortcuts;
        pTable->bufferSize = newBufferSize;
    }

    assert(pTable->count < pTable->bufferSize);

    // The accelerators of the shortcut need to be added to the table. If we don't do this, the platform layer will not be aware of it.
    //for (size_t i = 0; i < acceleratorCount; ++i) {
    //    if (!dred_accelerator_table_add(&pTable->acceleratorTable, pAccelerators[i])) {
    //        return DTK_FALSE;
    //    }
    //}

    // TODO: Need to improve the efficiency of this. The implementation of dtk_bind_accelerator() will perform a complete recreation
    //       of the internal accelerator table for each call. May want something like a begin/end pair or dtk_shortcut_table_update_internal_bindings().
    if (dtk_bind_accelerators(&pTable->pDred->tk, pAccelerators, acceleratorCount) != DTK_SUCCESS) {
        return DTK_FALSE;
    }

    memset(&pTable->pShortcuts[pTable->count], 0, sizeof(pTable->pShortcuts[pTable->count]));
    pTable->pShortcuts[pTable->count].id = id;
    pTable->pShortcuts[pTable->count].nameOffset = dred_string_pool_find_or_add(&pTable->pDred->stringPool, name);
    pTable->pShortcuts[pTable->count].cmdOffset = dred_string_pool_find_or_add(&pTable->pDred->stringPool, cmdStr);
    pTable->pShortcuts[pTable->count].acceleratorCount = acceleratorCount;
    memcpy(pTable->pShortcuts[pTable->count].accelerators, pAccelerators, acceleratorCount * sizeof(*pAccelerators));
    pTable->count += 1;

    return DTK_TRUE;
}

dtk_bool32 dred_shortcut_table_unbind(dred_shortcut_table* pTable, dtk_uint32 id)
{
    size_t index;
    if (!dred_shortcut_table_find(pTable, id, &index)) {
        return DTK_FALSE;
    }

    //dred_shortcut shortcut = pTable->pShortcuts[index];

    if (index+1 < pTable->count) {
        memmove(pTable->pShortcuts + index, pTable->pShortcuts + (index+1), sizeof(*pTable->pShortcuts) * (pTable->count - (index+1)));
    }

    pTable->count -= 1;


    // At this point the shortcut will be removed, but there may be a leftover accelerator in the accelerator table. We need
    // to check if those accelerators are now unused, and if so, remove them.
#if 0
    for (size_t i = 0; i < shortcut.acceleratorCount; ++i) {
        dtk_bool32 exists = DTK_FALSE;
        for (size_t j = 0; j < pTable->acceleratorTable.count; ++j) {
            if (dtk_accelerator_equal(shortcut.accelerators[i], pTable->acceleratorTable.pAccelerators[j])) {
                exists = DTK_TRUE;
                break;
            }
        }

        // Remove the accelerator from the table if it's unused.
        if (!exists) {
            dred_accelerator_table_remove(&pTable->acceleratorTable, shortcut.accelerators[i]);
        }
    }
#endif

    return DTK_TRUE;
}


dtk_bool32 dred_shortcut_table_find(dred_shortcut_table* pTable, dtk_uint32 id, size_t* pIndexOut)
{
    if (pIndexOut) *pIndexOut = 0;  // Safety.
    if (pTable == NULL) {
        return DTK_FALSE;
    }

    for (size_t i = 0; i < pTable->count; ++i) {
        if (pTable->pShortcuts[i].id == id) {
            if (pIndexOut) *pIndexOut = i;
            return DTK_TRUE;
        }
    }

    return DTK_FALSE;
}

dtk_bool32 dred_shortcut_table_find_by_name(dred_shortcut_table* pTable, const char* name, size_t* pIndexOut)
{
    if (pIndexOut) *pIndexOut = 0;  // Safety.
    if (pTable == NULL) {
        return DTK_FALSE;
    }

    for (size_t i = 0; i < pTable->count; ++i) {
        if (strcmp(name, dred_string_pool_cstr(&pTable->pDred->stringPool, pTable->pShortcuts[i].nameOffset)) == 0) {
            if (pIndexOut) *pIndexOut = i;
            return DTK_TRUE;
        }
    }

    return DTK_FALSE;
}

void dred_shortcut_table_replace_by_index(dred_shortcut_table* pTable, size_t shortcutIndex, const char* name, const char* cmdStr, dtk_uint32 acceleratorCount, dtk_accelerator* pAccelerators)
{
    if (pTable == NULL || shortcutIndex >= pTable->count || acceleratorCount > 4) {
        return;
    }

    pTable->pShortcuts[shortcutIndex].nameOffset = dred_string_pool_find_or_add(&pTable->pDred->stringPool, name);
    pTable->pShortcuts[shortcutIndex].cmdOffset = dred_string_pool_find_or_add(&pTable->pDred->stringPool, cmdStr);
    pTable->pShortcuts[shortcutIndex].acceleratorCount = acceleratorCount;
    memcpy(pTable->pShortcuts[shortcutIndex].accelerators, pAccelerators, acceleratorCount * sizeof(*pAccelerators));
}


dtk_bool32 dred_shortcut_table_get_shortcut_by_index(dred_shortcut_table* pTable, size_t shortcutIndex, dred_shortcut* pShortcutOut)
{
    if (pTable == NULL || shortcutIndex > pTable->count) {
        return DTK_FALSE;
    }

    if (pShortcutOut) *pShortcutOut = pTable->pShortcuts[shortcutIndex];
    return DTK_TRUE;
}

const char* dred_shortcut_table_get_command_string_by_index(dred_shortcut_table* pTable, size_t shortcutIndex)
{
    if (pTable == NULL || shortcutIndex >= pTable->count) {
        return NULL;
    }

    return pTable->pDred->stringPool.pData + pTable->pShortcuts[shortcutIndex].cmdOffset;
}

const char* dred_shortcut_table_get_command_string_by_name(dred_shortcut_table* pTable, const char* name)
{
    size_t index;
    if (!dred_shortcut_table_find_by_name(pTable, name, &index)) {
        return NULL;
    }

    return dred_shortcut_table_get_command_string_by_index(pTable, index);
}



dred_shortcut dred_shortcut_create(dtk_accelerator accel0, dtk_accelerator accel1)
{
    if (accel0.key == 0 && accel1.key != 0) {
        accel0 = accel1;
    }

    dred_shortcut shortcut;
    memset(&shortcut, 0, sizeof(shortcut));
    shortcut.accelerators[0] = accel0;
    shortcut.accelerators[1] = accel1;

    if (shortcut.accelerators[0].key != 0) shortcut.acceleratorCount += 1;
    if (shortcut.accelerators[1].key != 0) shortcut.acceleratorCount += 1;

    return shortcut;
}

dred_shortcut dred_shortcut_create_single(dtk_accelerator accel)
{
    return dred_shortcut_create(accel, dtk_accelerator_none());
}

dred_shortcut dred_shortcut_none()
{
    return dred_shortcut_create(dtk_accelerator_none(), dtk_accelerator_none());
}

dred_shortcut dred_shortcut_parse(const char* shortcutStr)
{
    if (shortcutStr == NULL) {
        return dred_shortcut_none();
    }


    dred_shortcut shortcut = dred_shortcut_none();
    int iNextAccel = 0;

    // A shortcut string is just 1-4 accelerator strings separated by a comma.
    const char* nextBeg = shortcutStr;
    const char* nextEnd = nextBeg;
    for (;;)
    {
        if (nextEnd[0] == ',' || nextEnd[0] == '\0') {
            char nextAccelStr[256];
            if (strncpy_s(nextAccelStr, sizeof(nextAccelStr), nextBeg, (nextEnd - nextBeg)) != 0) {
                break;
            }

            dtk_accelerator_parse(nextAccelStr, &shortcut.accelerators[iNextAccel++]);


            if (nextEnd[0] == '\0' || iNextAccel == 4) {
                break;
            } else {
                nextBeg = nextEnd + 1;
                nextEnd = nextBeg;
                continue;
            }
        }

        nextEnd += 1;
    }

    shortcut.acceleratorCount = (dtk_uint32)iNextAccel;
    return shortcut;
}

dtk_bool32 dred_shortcut_equal(dred_shortcut a, dred_shortcut b)
{
    if (a.acceleratorCount == b.acceleratorCount) {
        for (dtk_uint32 i = 0; i < a.acceleratorCount; ++i) {
            if (!dtk_accelerator_equal(a.accelerators[i], b.accelerators[i])) {
                return DTK_FALSE;
            }
        }
    } else {
        return DTK_FALSE;
    }

    return DTK_TRUE;
}

size_t dred_shortcut_to_string(dred_shortcut shortcut, char* strOut, size_t strOutSize)
{
    if (shortcut.accelerators[0].key == 0) {
        if (strOut != NULL && strOutSize > 0) {
            strOut[0] = '\0';
        }

        return 0;
    }

    if (strOut == NULL) {
        size_t len = shortcut.acceleratorCount-1;   // <-- One character for each comma.
        for (dtk_uint32 i = 0; i < shortcut.acceleratorCount; ++i) {
            char accelStr[256];
            len += dtk_accelerator_to_string(shortcut.accelerators[i], accelStr, sizeof(accelStr));
        }

        return len;
    } else {
        if (strOutSize > 0) {
            strOut[0] = '\0';
        }

        for (dtk_uint32 i = 0; i < shortcut.acceleratorCount; ++i) {
            char accelStr[256];
            dtk_accelerator_to_string(shortcut.accelerators[0], accelStr, sizeof(accelStr));
            dtk_strcat_s(strOut, strOutSize, accelStr);

            if (i != shortcut.acceleratorCount-1) {
                dtk_strcat_s(strOut, strOutSize, ",");
            }
        }

        return strlen(strOut);
    }
}
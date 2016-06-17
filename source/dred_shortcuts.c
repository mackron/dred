
bool dred_shortcut_table_init(dred_shortcut_table* pTable)
{
    if (pTable == NULL) {
        return false;
    }

    pTable->pShortcuts = NULL;
    pTable->ppCmdStrings = NULL;
    pTable->count = 0;
    pTable->bufferSize = 0;

    if (!dred_accelerator_table_init(&pTable->acceleratorTable)) {
        return false;
    }

    return true;
}

void dred_shortcut_table_uninit(dred_shortcut_table* pTable)
{
    if (pTable == NULL) {
        return;
    }

    for (size_t i = 0; i < pTable->count; ++i) {
        gb_free_string(pTable->ppCmdStrings[i]);
    }
    free(pTable->ppCmdStrings);

    free(pTable->pShortcuts);
    pTable->count = 0;
    pTable->bufferSize = 0;

    dred_accelerator_table_uninit(&pTable->acceleratorTable);
}

bool dred_shortcut_table_bind(dred_shortcut_table* pTable, dred_shortcut shortcut, const char* cmdStr)
{
    if (pTable == NULL) {
        return false;
    }

    // If an accelerator with the same key combination already exists, just replace the command.
    size_t existingIndex;
    if (dred_shortcut_table_find(pTable, shortcut, &existingIndex)) {
        dred_shortcut_table_replace(pTable, existingIndex, cmdStr);
    }

    // If we get here it means the accelerator does not already exist and needs to be added.
    if (pTable->count == pTable->bufferSize)
    {
        size_t newBufferSize = (pTable->bufferSize == 0) ? 16 : (pTable->bufferSize * 2);
        dred_shortcut* pNewShortcuts = (dred_shortcut*)realloc(pTable->pShortcuts, newBufferSize * sizeof(*pNewShortcuts));
        if (pNewShortcuts == NULL) {
            return false;
        }

        char** ppNewCmdStrings = (char**)realloc(pTable->ppCmdStrings, newBufferSize * sizeof(*ppNewCmdStrings));
        if (ppNewCmdStrings == NULL) {
            return false;
        }

        pTable->pShortcuts = pNewShortcuts;
        pTable->ppCmdStrings = ppNewCmdStrings;
        pTable->bufferSize = newBufferSize;
    }

    assert(pTable->count < pTable->bufferSize);

    // The accelerators of the shortcut need to be added to the table. If we don't do this, the platform layer will not be aware of it.
    if (shortcut.accelerators[0].key != 0 && !dred_accelerator_table_add(&pTable->acceleratorTable, shortcut.accelerators[0])) {
        return false;
    }
    if (shortcut.accelerators[1].key != 0 && !dred_accelerator_table_add(&pTable->acceleratorTable, shortcut.accelerators[1])) {
        return false;
    }


    pTable->pShortcuts[pTable->count] = shortcut;
    pTable->ppCmdStrings[pTable->count] = gb_make_string(cmdStr);
    pTable->count += 1;

    return true;
}

bool dred_shortcut_table_unbind(dred_shortcut_table* pTable, dred_shortcut shortcut)
{
    size_t index;
    if (!dred_shortcut_table_find(pTable, shortcut, &index)) {
        return false;
    }

    if (index+1 < pTable->count) {
        memmove(pTable->pShortcuts + index, pTable->pShortcuts + (index+1), sizeof(*pTable->pShortcuts) * (pTable->count - (index+1)));

        gb_free_string(pTable->ppCmdStrings[index]);
        memmove(pTable->ppCmdStrings + index, pTable->ppCmdStrings + (index+1), sizeof(*pTable->ppCmdStrings) * (pTable->count - (index+1)));

        // TODO: Delete name.
    }

    pTable->count -= 1;


    // At this point the shortcut will be removed, but there may be a leftover accelerator in the accelerator table. We need
    // to check if those accelerators are now unused, and if so, remove them.
    for (int i = 0; i < 2; ++i) {
        int refcount = 0;
        for (size_t j = 0; j < pTable->acceleratorTable.count; ++j) {
            if (dred_accelerator_equal(shortcut.accelerators[i], pTable->acceleratorTable.pAccelerators[j])) {
                refcount += 1;
                continue;
            }
        }

        assert(refcount == 0);

        // The accelerator is unused. Remove it from the table.
        dred_accelerator_table_remove(&pTable->acceleratorTable, shortcut.accelerators[i]);
    }

    return true;
}


bool dred_shortcut_table_find(dred_shortcut_table* pTable, dred_shortcut shortcut, size_t* pIndexOut)
{
    if (pIndexOut) *pIndexOut = 0;  // Safety.
    if (pTable == NULL) {
        return false;
    }

    for (size_t i = 0; i < pTable->count; ++i) {
        if (dred_shortcut_equal(pTable->pShortcuts[i], shortcut)) {
            if (pIndexOut) *pIndexOut = i;
            return true;
        }
    }

    return false;
}

void dred_shortcut_table_replace(dred_shortcut_table* pTable, size_t shortcutIndex, const char* cmdStr)
{
    if (pTable == NULL || shortcutIndex >= pTable->count) {
        return;
    }

    char* pNewCmdStr = gb_make_string(cmdStr);
    if (pNewCmdStr == NULL) {
        return;
    }

    gb_free_string(pTable->ppCmdStrings[shortcutIndex]);
    pTable->ppCmdStrings[shortcutIndex] = pNewCmdStr;
}

const char* dred_shortcut_table_get_command_string_by_index(dred_shortcut_table* pTable, size_t shortcutIndex)
{
    if (pTable == NULL || shortcutIndex >= pTable->count) {
        return NULL;
    }

    return pTable->ppCmdStrings[shortcutIndex];
}



dred_shortcut dred_shortcut_create(dred_accelerator accel0, dred_accelerator accel1)
{
    if (accel0.key == 0 && accel1.key != 0) {
        accel0 = accel1;
    }

    dred_shortcut shortcut;
    shortcut.accelerators[0] = accel0;
    shortcut.accelerators[1] = accel1;

    return shortcut;
}

dred_shortcut dred_shortcut_create_single(dred_accelerator accel)
{
    return dred_shortcut_create(accel, dred_accelerator_none());
}

dred_shortcut dred_shortcut_none()
{
    return dred_shortcut_create(dred_accelerator_none(), dred_accelerator_none());
}

bool dred_shortcut_equal(dred_shortcut a, dred_shortcut b)
{
    return dred_accelerator_equal(a.accelerators[0], b.accelerators[0]) &&
           dred_accelerator_equal(a.accelerators[1], b.accelerators[1]);
}

size_t dred_shortcut_to_string(dred_shortcut shortcut, char* strOut, size_t strOutSize)
{
    if (shortcut.accelerators[0].key == 0) {
        strcpy_s(strOut, strOutSize, "");
        return 0;
    }

    char accelStr0[256];
    dred_accelerator_to_string(shortcut.accelerators[0], accelStr0, sizeof(accelStr0));

    int length = 0;
    if (shortcut.accelerators[1].key != 0) {
        char accelStr1[256];
        dred_accelerator_to_string(shortcut.accelerators[1], accelStr1, sizeof(accelStr1));

        length = snprintf(strOut, strOutSize, "%s,%s", accelStr0, accelStr1);
    } else {
        length = snprintf(strOut, strOutSize, "%s", accelStr0);
    }

    if (length < 0) {
        return 0;
    }

    return length;
}


bool dred_accelerator_table_init(dred_accelerator_table* pTable)
{
    if (pTable == NULL) {
        return false;
    }

    pTable->pAccelerators = NULL;
    pTable->count = 0;
    pTable->bufferSize = 0;

    return true;
}

void dred_accelerator_table_uninit(dred_accelerator_table* pTable)
{
    if (pTable == NULL) {
        return;
    }

    free(pTable->pAccelerators);
    pTable->count = 0;
    pTable->bufferSize = 0;
}


bool dred_accelerator_table_bind(dred_accelerator_table* pTable, drgui_key key, uint32_t modifiers, const char* cmdStr)
{
    if (pTable == NULL) {
        return false;
    }

    // If an accelerator with the same key combination already exists, just replace the command.
    size_t existingIndex;
    if (dred_accelerator_table_find(pTable, key, modifiers, &existingIndex)) {
        dred_accelerator_table_replace(pTable, existingIndex, cmdStr);
    }

    // If we get here it means the accelerator does not already exist and needs to be added.
    if (pTable->count == pTable->bufferSize)
    {
        size_t newBufferSize = (pTable->bufferSize == 0) ? 16 : (pTable->bufferSize * 2);
        dred_accelerator* pNewAccelerators = (dred_accelerator*)realloc(pTable->pAccelerators, newBufferSize * sizeof(*pNewAccelerators));
        if (pNewAccelerators == NULL) {
            return false;
        }

        pTable->pAccelerators = pNewAccelerators;
        pTable->bufferSize = newBufferSize;
    }

    assert(pTable->count < pTable->bufferSize);

    pTable->pAccelerators[pTable->count].key = key;
    pTable->pAccelerators[pTable->count].modifiers = modifiers;
    pTable->count += 1;

    return true;
}


bool dred_accelerator_table_find(dred_accelerator_table* pTable, drgui_key key, uint32_t modifiers, size_t* pIndexOut)
{
    // TODO: Implement Me.
    (void)pTable;
    (void)key;
    (void)modifiers;
    (void)pIndexOut;

    return false;
}

void dred_accelerator_table_replace(dred_accelerator_table* pTable, size_t acceleratorIndex, const char* cmdStr)
{
    // TODO: Implement Me.
    (void)pTable;
    (void)acceleratorIndex;
    (void)cmdStr;
}
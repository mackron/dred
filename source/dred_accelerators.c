

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


bool dred_accelerator_table_add(dred_accelerator_table* pTable, dred_accelerator accelerator)
{
    if (pTable == NULL) {
        return false;
    }

    // If an accelerator with the same key combination already exists, just replace the command.
    size_t existingIndex;
    if (dred_accelerator_table_find(pTable, accelerator, &existingIndex)) {
        return true;    // Already exists.
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

    pTable->pAccelerators[pTable->count] = accelerator;
    pTable->count += 1;

    return true;
}

bool dred_accelerator_table_remove(dred_accelerator_table* pTable, dred_accelerator accelerator)
{
    size_t index;
    if (!dred_accelerator_table_find(pTable, accelerator, &index)) {
        return false;
    }

    if (index+1 < pTable->count) {
        memmove(pTable->pAccelerators + index, pTable->pAccelerators + (index+1), sizeof(*pTable->pAccelerators) * (pTable->count - (index+1)));
    }

    pTable->count -= 1;
    return true;
}


bool dred_accelerator_table_find(dred_accelerator_table* pTable, dred_accelerator accelerator, size_t* pIndexOut)
{
    if (pIndexOut) *pIndexOut = 0;  // Safety.
    if (pTable == NULL) {
        return false;
    }

    for (size_t i = 0; i < pTable->count; ++i) {
        if (pTable->pAccelerators[i].key == accelerator.key && pTable->pAccelerators[i].modifiers == accelerator.modifiers) {
            if (pIndexOut) *pIndexOut = i;
            return true;
        }
    }

    return false;
}


dred_accelerator dred_accelerator_none()
{
    dred_accelerator result;
    result.key = 0;
    result.modifiers = 0;

    return result;
}

dred_accelerator dred_accelerator_create(drgui_key key, uint32_t modifiers)
{
    dred_accelerator result;
    result.key = key;
    result.modifiers = modifiers;

    return result;
}

bool dred_accelerator_equal(dred_accelerator a, dred_accelerator b)
{
    return a.key == b.key && a.modifiers == b.modifiers;
}

size_t dred_accelerator_to_string(dred_accelerator accelerator, char* strOut, size_t strOutSize)
{
    if (strOut == NULL || strOutSize == 0) {
        return 0;
    }

    strOut[0] = '\0';

    size_t characterLength = 0;
    char characterStr[16] = {0};
    if (accelerator.modifiers != 0) {
        characterStr[characterLength++] = '+';
    }
    characterLength += drgui_key_to_string(accelerator.key, characterStr + characterLength, sizeof(characterStr) - characterLength);


    size_t modifiersLength = 0;
    char modifiersStr[256] = {0};
    if (accelerator.modifiers & DRED_KEY_STATE_CTRL_DOWN) {
        if (modifiersStr[0] != '\0') {
            modifiersStr[modifiersLength++] = '+';
        }
        if (strcpy_s(modifiersStr + modifiersLength, sizeof(modifiersStr) - modifiersLength - 1, "Ctrl") != 0) {
            return 0;
        }
        modifiersLength += strlen("Ctrl");
    }

    if (accelerator.modifiers & DRED_KEY_STATE_SHIFT_DOWN) {
        if (modifiersStr[0] != '\0') {
            modifiersStr[modifiersLength++] = '+';
        }
        if (strcpy_s(modifiersStr + modifiersLength, sizeof(modifiersStr) - modifiersLength - 1, "Shift") != 0) {
            return 0;
        }
        modifiersLength += strlen("Shift");
    }

    if (accelerator.modifiers & DRED_KEY_STATE_ALT_DOWN) {
        if (modifiersStr[0] != '\0') {
            modifiersStr[modifiersLength++] = '+';
        }
        if (strcpy_s(modifiersStr + modifiersLength, sizeof(modifiersStr) - modifiersLength - 1, "Alt") != 0) {
            return 0;
        }
        modifiersLength += strlen("Alt");
    }

    
    snprintf(strOut, strOutSize, "%s%s", modifiersStr, characterStr);
    return modifiersLength + characterLength;
}


bool dred_accelerator_table_init(dred_accelerator_table* pTable)
{
    if (pTable == NULL) {
        return false;
    }

    pTable->pAccelerators = NULL;
    pTable->ppCmdStrings = NULL;
    pTable->count = 0;
    pTable->bufferSize = 0;

    return true;
}

void dred_accelerator_table_uninit(dred_accelerator_table* pTable)
{
    if (pTable == NULL) {
        return;
    }

    for (size_t i = 0; i < pTable->count; ++i) {
        gb_free_string(pTable->ppCmdStrings[i]);
    }
    free(pTable->ppCmdStrings);

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

        char** ppNewCmdStrings = (char**)realloc(pTable->ppCmdStrings, newBufferSize * sizeof(*ppNewCmdStrings));
        if (ppNewCmdStrings == NULL) {
            return false;
        }

        pTable->pAccelerators = pNewAccelerators;
        pTable->ppCmdStrings = ppNewCmdStrings;
        pTable->bufferSize = newBufferSize;
    }

    assert(pTable->count < pTable->bufferSize);

    pTable->pAccelerators[pTable->count].key = key;
    pTable->pAccelerators[pTable->count].modifiers = modifiers;
    pTable->ppCmdStrings[pTable->count] = gb_make_string(cmdStr);
    pTable->count += 1;

    return true;
}


bool dred_accelerator_table_find(dred_accelerator_table* pTable, drgui_key key, uint32_t modifiers, size_t* pIndexOut)
{
    if (pIndexOut) *pIndexOut = 0;  // Safety.
    if (pTable == NULL) {
        return false;
    }

    for (size_t i = 0; i < pTable->count; ++i) {
        if (pTable->pAccelerators[i].key == key && pTable->pAccelerators[i].modifiers == modifiers) {
            if (pIndexOut) *pIndexOut = i;
            return true;
        }
    }

    return false;
}

void dred_accelerator_table_replace(dred_accelerator_table* pTable, size_t acceleratorIndex, const char* cmdStr)
{
    if (pTable == NULL || acceleratorIndex >= pTable->count) {
        return;
    }

    char* pNewCmdStr = gb_make_string(cmdStr);
    if (pNewCmdStr == NULL) {
        return;
    }

    gb_free_string(pTable->ppCmdStrings[acceleratorIndex]);
    pTable->ppCmdStrings[acceleratorIndex] = pNewCmdStr;
}

const char* dred_accelerator_table_get_command_string_by_index(dred_accelerator_table* pTable, size_t acceleratorIndex)
{
    if (pTable == NULL || acceleratorIndex >= pTable->count) {
        return NULL;
    }

    return pTable->ppCmdStrings[acceleratorIndex];
}


size_t dred_accelerator_to_string(dred_accelerator accelerator, char* strOut, size_t strOutSize)
{
    if (strOut == NULL || strOutSize == 0) {
        return 0;
    }

    strOut[0] = '\0';

    size_t characterLength = 0;
    char characterStr[16];
    if (accelerator.modifiers != 0) {
        characterStr[characterLength++] = '+';
    }
    // TODO: dr_utf32_to_utf8_ch()
    characterStr[characterLength++] = (char)accelerator.key;
    characterStr[characterLength] = '\0';


    size_t modifiersLength = 0;
    char modifiersStr[256] = {'\0'};
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
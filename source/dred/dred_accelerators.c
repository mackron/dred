// Copyright (C) 2016 David Reid. See included LICENSE file.


dr_bool32 dred_accelerator_table_init(dred_accelerator_table* pTable)
{
    if (pTable == NULL) {
        return DR_FALSE;
    }

    pTable->pAccelerators = NULL;
    pTable->count = 0;
    pTable->bufferSize = 0;

    return DR_TRUE;
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


dr_bool32 dred_accelerator_table_add(dred_accelerator_table* pTable, dtk_accelerator accelerator)
{
    if (pTable == NULL) {
        return DR_FALSE;
    }

    // If an accelerator with the same key combination already exists, just replace the command.
    size_t existingIndex;
    if (dred_accelerator_table_find(pTable, accelerator, &existingIndex)) {
        return DR_TRUE;    // Already exists.
    }

    // If we get here it means the accelerator does not already exist and needs to be added.
    if (pTable->count == pTable->bufferSize)
    {
        size_t newBufferSize = (pTable->bufferSize == 0) ? 16 : (pTable->bufferSize * 2);
        dtk_accelerator* pNewAccelerators = (dtk_accelerator*)realloc(pTable->pAccelerators, newBufferSize * sizeof(*pNewAccelerators));
        if (pNewAccelerators == NULL) {
            return DR_FALSE;
        }

        pTable->pAccelerators = pNewAccelerators;
        pTable->bufferSize = newBufferSize;
    }

    assert(pTable->count < pTable->bufferSize);

    pTable->pAccelerators[pTable->count] = accelerator;
    pTable->count += 1;

    return DR_TRUE;
}

dr_bool32 dred_accelerator_table_remove(dred_accelerator_table* pTable, dtk_accelerator accelerator)
{
    size_t index;
    if (!dred_accelerator_table_find(pTable, accelerator, &index)) {
        return DR_FALSE;
    }

    if (index+1 < pTable->count) {
        memmove(pTable->pAccelerators + index, pTable->pAccelerators + (index+1), sizeof(*pTable->pAccelerators) * (pTable->count - (index+1)));
    }

    pTable->count -= 1;
    return DR_TRUE;
}


dr_bool32 dred_accelerator_table_find(dred_accelerator_table* pTable, dtk_accelerator accelerator, size_t* pIndexOut)
{
    if (pIndexOut) *pIndexOut = 0;  // Safety.
    if (pTable == NULL) {
        return DR_FALSE;
    }

    for (size_t i = 0; i < pTable->count; ++i) {
        if (pTable->pAccelerators[i].key == accelerator.key && pTable->pAccelerators[i].modifiers == accelerator.modifiers) {
            if (pIndexOut) *pIndexOut = i;
            return DR_TRUE;
        }
    }

    return DR_FALSE;
}


#if 0
dred_accelerator dred_accelerator_none()
{
    dred_accelerator result;
    result.key = 0;
    result.modifiers = 0;

    return result;
}

dred_accelerator dred_accelerator_create(dred_key key, uint32_t modifiers)
{
    dred_accelerator result;
    result.key = key;
    result.modifiers = modifiers;

    return result;
}

const char* dred_accelerator__next_token(const char* tokens, char* tokenOut, unsigned int tokenOutSize)
{
    if (tokens == NULL) {
        return NULL;
    }

    // Skip past leading whitespace.
    while (tokens[0] != '\0' && !(tokens[0] != '+' && tokens[0] != ' ' && tokens[0] != '\t' && tokens[0] != '\n' && tokens[0] != '\v' && tokens[0] != '\f' && tokens[0] != '\r')) {
        tokens += 1;
    }

    if (tokens[0] == '\0') {
        return NULL;
    }


    const char* strBeg = tokens;
    const char* strEnd = strBeg;

    // Loop until the first "+" symbol or whitespace character.
    while (strEnd[0] != '\0' && (strEnd[0] != '+' && strEnd[0] != ' ' && strEnd[0] != '\t' && strEnd[0] != '\n' && strEnd[0] != '\v' && strEnd[0] != '\f' && strEnd[0] != '\r')) {
        strEnd += 1;
    }

    // If the output buffer is large enough to hold the token, copy the token into it. When we copy the token we need to
    // ensure we don't include the escape character.
    //assert(strEnd >= strBeg);

    while (tokenOutSize > 1 && strBeg < strEnd) {
        *tokenOut++ = *strBeg++;
        tokenOutSize -= 1;
    }

    // Null-terminate.
    if (tokenOutSize > 0) {
        *tokenOut = '\0';
    }


    return strEnd;
}

dred_accelerator dred_accelerator_parse(const char* accelStr)
{
    dred_accelerator accelerator = dred_accelerator_none();

    const char* prevAccelStr = accelStr;

    char token[256];
    while ((accelStr = dred_accelerator__next_token(accelStr, token, sizeof(token))) != NULL) {
        if (_stricmp(token, "ctrl") == 0) {
            accelerator.modifiers |= DRED_GUI_KEY_STATE_CTRL_DOWN;
            prevAccelStr = accelStr;
            continue;
        }
        if (_stricmp(token, "alt") == 0) {
            accelerator.modifiers |= DRED_GUI_KEY_STATE_ALT_DOWN;
            prevAccelStr = accelStr;
            continue;
        }
        if (_stricmp(token, "shift") == 0) {
            accelerator.modifiers |= DRED_GUI_KEY_STATE_SHIFT_DOWN;
            prevAccelStr = accelStr;
            continue;
        }

        // If we get here, assume it's a key. The key should always come last.
        if (prevAccelStr[0] == '+') {
            prevAccelStr += 1;
        }

        accelerator.key = dred_key_parse(prevAccelStr);

        // Shortcuts should use the capitalized version of the key. If we don't do this it won't work on Win32.
        if (accelerator.key >= 32 && accelerator.key <= 126) {
            accelerator.key = toupper(accelerator.key);
        }

        break;
    }

    return accelerator;
}

dr_bool32 dred_accelerator_equal(dred_accelerator a, dred_accelerator b)
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
    if (accelerator.key == '\t') {
        characterStr[characterLength++] = 'T';
        characterStr[characterLength++] = 'A';
        characterStr[characterLength++] = 'B';
    } else {
        characterLength += dred_key_to_string(accelerator.key, characterStr + characterLength, sizeof(characterStr) - characterLength);
    }
    


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
#endif

// Copyright (C) 2016 David Reid. See included LICENSE file.

const char* dtk_accelerator__next_token(const char* tokens, char* tokenOut, unsigned int tokenOutSize)
{
    if (tokens == NULL) return NULL;

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

dtk_result dtk_accelerator_parse(const char* accelStr, dtk_accelerator* pAccelerator)
{
    if (pAccelerator == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pAccelerator);

    if (dtk_string_is_null_or_empty(accelStr)) return DTK_INVALID_ARGS;

    dtk_accelerator accelerator = dtk_accelerator_init(0, 0, 0);

    const char* prevAccelStr = accelStr;

    char token[256];
    while ((accelStr = dtk_accelerator__next_token(accelStr, token, sizeof(token))) != NULL) {
        if (dtk_stricmp(token, "ctrl") == 0) {
            accelerator.modifiers |= DTK_MODIFIER_CTRL;
            prevAccelStr = accelStr;
            continue;
        }
        if (dtk_stricmp(token, "alt") == 0) {
            accelerator.modifiers |= DTK_MODIFIER_ALT;
            prevAccelStr = accelStr;
            continue;
        }
        if (dtk_stricmp(token, "shift") == 0) {
            accelerator.modifiers |= DTK_MODIFIER_SHIFT;
            prevAccelStr = accelStr;
            continue;
        }

        // If we get here, assume it's a key. The key should always come last.
        if (prevAccelStr[0] == '+') {
            prevAccelStr += 1;
        }

        accelerator.key = dtk_key_parse(prevAccelStr);

        // Shortcuts should use the capitalized version of the key. If we don't do this it won't work on Win32.
        if (accelerator.key >= 32 && accelerator.key <= 126) {
            accelerator.key = toupper(accelerator.key);
        }

        break;
    }

    *pAccelerator = accelerator;
    return DTK_SUCCESS;
}

dtk_result dtk_accelerator_parse_chord(const char* accelStr, dtk_accelerator* pAccelerators, dtk_uint32* pCount)
{
    if (accelStr == NULL || pCount == NULL) return DTK_INVALID_ARGS;

    if (pAccelerators != NULL && pCount != NULL) {
        dtk_zero_memory(pAccelerators, sizeof(*pAccelerators) * (*pCount));
    }

    dtk_uint32 iNextAccel = 0;
    const char* nextBeg = accelStr;
    const char* nextEnd = nextBeg;
    for (;;)
    {
        if (nextEnd[0] == ',' || nextEnd[0] == '\0') {
            char nextAccelStr[256];
            if (dtk_strncpy_s(nextAccelStr, sizeof(nextAccelStr), nextBeg, (nextEnd - nextBeg)) != 0) {
                break;
            }

            dtk_accelerator accelerator;
            if (dtk_accelerator_parse(nextAccelStr, &accelerator) == DTK_SUCCESS) {
                if (pCount && *pCount > iNextAccel) {
                    pAccelerators[iNextAccel] = accelerator;
                }

                iNextAccel += 1;
            }

            if (nextEnd[0] == '\0' || iNextAccel == 1) {
                break;
            } else {
                nextBeg = nextEnd + 1;
                nextEnd = nextBeg;
                continue;
            }
        }

        nextEnd += 1;
    }

    if (pCount) *pCount = iNextAccel;
    return DTK_SUCCESS;
}

size_t dtk_accelerator_to_string(dtk_accelerator accelerator, char* strOut, size_t strOutSize)
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
        characterLength += dtk_key_to_string(accelerator.key, characterStr + characterLength, sizeof(characterStr) - characterLength);
    }
    


    size_t modifiersLength = 0;
    char modifiersStr[256] = {0};
    if (accelerator.modifiers & DTK_MODIFIER_CTRL) {
        if (modifiersStr[0] != '\0') {
            modifiersStr[modifiersLength++] = '+';
        }
        if (dtk_strcpy_s(modifiersStr + modifiersLength, sizeof(modifiersStr) - modifiersLength - 1, "Ctrl") != 0) {
            return 0;
        }
        modifiersLength += strlen("Ctrl");
    }

    if (accelerator.modifiers & DTK_MODIFIER_SHIFT) {
        if (modifiersStr[0] != '\0') {
            modifiersStr[modifiersLength++] = '+';
        }
        if (dtk_strcpy_s(modifiersStr + modifiersLength, sizeof(modifiersStr) - modifiersLength - 1, "Shift") != 0) {
            return 0;
        }
        modifiersLength += strlen("Shift");
    }

    if (accelerator.modifiers & DTK_MODIFIER_ALT) {
        if (modifiersStr[0] != '\0') {
            modifiersStr[modifiersLength++] = '+';
        }
        if (dtk_strcpy_s(modifiersStr + modifiersLength, sizeof(modifiersStr) - modifiersLength - 1, "Alt") != 0) {
            return 0;
        }
        modifiersLength += strlen("Alt");
    }

    
    dtk_strncpy_s(strOut, strOutSize, modifiersStr, _TRUNCATE);
    dtk_strncat_s(strOut, strOutSize, characterStr, _TRUNCATE);

    return modifiersLength + characterLength;
}
// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_bool32 dtk_hex_char_to_uint(char ascii, unsigned int* out)
{
    if (ascii >= '0' && ascii <= '9') {
        if (out) *out = ascii - '0';
        return DTK_TRUE;
    }

    if (ascii >= 'A' && ascii <= 'F') {
        if (out) *out = 10 + (ascii - 'A');
        return DTK_TRUE;
    }

    if (ascii >= 'a' && ascii <= 'f') {
        if (out) *out = 10 + (ascii - 'a');
        return DTK_TRUE;
    }

    if (out) *out = 0;
    return DTK_FALSE;
}

const char* dtk_next_token(const char* tokens, char* tokenOut, size_t tokenOutSize)
{
    if (tokenOut) tokenOut[0] = '\0';

    if (tokens == NULL) {
        return NULL;
    }

    // Skip past leading whitespace.
    while (tokens[0] != '\0' && !(tokens[0] != ' ' && tokens[0] != '\t' && tokens[0] != '\n' && tokens[0] != '\v' && tokens[0] != '\f' && tokens[0] != '\r')) {
        tokens += 1;
    }

    if (tokens[0] == '\0') {
        return NULL;
    }


    const char* strBeg = tokens;
    const char* strEnd = strBeg;

    if (strEnd[0] == '\"')
    {
        // It's double-quoted - loop until the next unescaped quote character.

        // Skip past the first double-quote character.
        strBeg += 1;
        strEnd += 1;

        // Keep looping until the next unescaped double-quote character.
        char prevChar = '\0';
        while (strEnd[0] != '\0' && (strEnd[0] != '\"' || prevChar == '\\'))
        {
            prevChar = strEnd[0];
            strEnd += 1;
        }
    }
    else
    {
        // It's not double-quoted - just loop until the first whitespace.
        while (strEnd[0] != '\0' && (strEnd[0] != ' ' && strEnd[0] != '\t' && strEnd[0] != '\n' && strEnd[0] != '\v' && strEnd[0] != '\f' && strEnd[0] != '\r')) {
            strEnd += 1;
        }
    }


    // If the output buffer is large enough to hold the token, copy the token into it. When we copy the token we need to
    // ensure we don't include the escape character.
    //assert(strEnd >= strBeg);

    while (tokenOutSize > 1 && strBeg < strEnd)
    {
        if (strBeg[0] == '\\' && strBeg[1] == '\"' && strBeg < strEnd) {
            strBeg += 1;
        }

        *tokenOut++ = *strBeg++;
        tokenOutSize -= 1;
    }

    // Null-terminate.
    if (tokenOutSize > 0) {
        *tokenOut = '\0';
    }


    // Skip past the double-quote character before returning.
    if (strEnd[0] == '\"') {
        strEnd += 1;
    }

    return strEnd;
}


void dtk_parse_key_value_pairs(dtk_key_value_read_proc onRead, dtk_key_value_pair_proc onPair, dtk_key_value_error_proc onError, void* pUserData)
{
    if (onRead == NULL) {
        return;
    }

    char pChunk[4096];
    size_t chunkSize = 0;

    unsigned int currentLine = 1;

    dtk_bool32 moveToNextLineBeforeProcessing = DTK_FALSE;
    dtk_bool32 skipWhitespaceBeforeProcessing = DTK_FALSE;

    // Just keep looping. We'll break from this loop when we have run out of data.
    for (;;)
    {
        // Start the iteration by reading as much data as we can.
        chunkSize = onRead(pUserData, pChunk, sizeof(pChunk));
        if (chunkSize == 0) {
            // No more data available.
            return;
        }

        char* pChunkEnd = pChunk + chunkSize;
        char* pC = pChunk;  // Chunk pointer. This is as the chunk is processed.

        if (moveToNextLineBeforeProcessing)
        {
            move_to_next_line:
            while (pC < pChunkEnd && pC[0] != '\n') {
                pC += 1;
            }

            if (pC == pChunkEnd) {
                // Ran out of data. Load the next chunk and keep going.
                moveToNextLineBeforeProcessing = DTK_TRUE;
                continue;
            }

            pC += 1;     // pC[0] == '\n' - skip past the new line character.
            currentLine += 1;
            moveToNextLineBeforeProcessing = DTK_FALSE;
        }

        if (skipWhitespaceBeforeProcessing)
        {
            while (pC < pChunkEnd && (pC[0] == ' ' || pC[0] == '\t' || pC[0] == '\r')) {
                pC += 1;
            }

            if (pC == pChunkEnd) {
                // Ran out of data.
                skipWhitespaceBeforeProcessing = DTK_TRUE;
                continue;
            }

            skipWhitespaceBeforeProcessing = DTK_FALSE;
        }


        // We loop character by character. When we run out of data, we start again.
        while (pC < pChunkEnd)
        {
            //// Key ////

            // Skip whitespace.
            while (pC < pChunkEnd && (pC[0] == ' ' || pC[0] == '\t' || pC[0] == '\r')) {
                pC += 1;
            }

            if (pC == pChunkEnd) {
                // Ran out of data.
                skipWhitespaceBeforeProcessing = DTK_TRUE;
                continue;
            }

            if (pC[0] == '\n') {
                // Found the end of the line.
                pC += 1;
                currentLine += 1;
                continue;
            }

            if (pC[0] == '#') {
                // Found a comment. Move to the end of the line and continue.
                goto move_to_next_line;
            }

            char* pK = pC;
            while (pC < pChunkEnd && pC[0] != ' ' && pC[0] != '\t' && pC[0] != '\r' && pC[0] != '\n' && pC[0] != '#') {
                pC += 1;
            }

            if (pC == pChunkEnd)
            {
                // Ran out of data. We need to move what we have of the key to the start of the chunk buffer, and then read more data.
                if (chunkSize == sizeof(pChunk))
                {
                    size_t lineSizeSoFar = pC - pK;
                    memmove(pChunk, pK, lineSizeSoFar);

                    chunkSize = lineSizeSoFar + onRead(pUserData, pChunk + lineSizeSoFar, sizeof(pChunk) - lineSizeSoFar);
                    pChunkEnd = pChunk + chunkSize;

                    pK = pChunk;
                    pC = pChunk + lineSizeSoFar;
                    while (pC < pChunkEnd && pC[0] != ' ' && pC[0] != '\t' && pC[0] != '\r' && pC[0] != '\n' && pC[0] != '#') {
                        pC += 1;
                    }
                }

                if (pC == pChunkEnd) {
                    if (chunkSize == sizeof(pChunk)) {
                        if (onError) {
                            onError(pUserData, "Line is too long. A single line cannot exceed 4KB.", currentLine);
                        }

                        goto move_to_next_line;
                    } else {
                        // No more data. Just treat this one as a value-less key and return.
                        if (onPair) {
                            pC[0] = '\0';
                            onPair(pUserData, pK, NULL);
                        }

                        return;
                    }
                }
            }

            char* pKEnd = pC;

            //// Value ////

            // Skip whitespace.
            while (pC < pChunkEnd && (pC[0] == ' ' || pC[0] == '\t' || pC[0] == '\r')) {
                pC += 1;
            }

            if (pC == pChunkEnd)
            {
                // Ran out of data. We need to move what we have of the key to the start of the chunk buffer, and then read more data.
                if (chunkSize == sizeof(pChunk))
                {
                    size_t lineSizeSoFar = pC - pK;
                    memmove(pChunk, pK, lineSizeSoFar);

                    chunkSize = lineSizeSoFar + onRead(pUserData, pChunk + lineSizeSoFar, sizeof(pChunk) - lineSizeSoFar);
                    pChunkEnd = pChunk + chunkSize;

                    pKEnd = pChunk + (pKEnd - pK);
                    pK = pChunk;
                    pC = pChunk + lineSizeSoFar;
                    while (pC < pChunkEnd && (pC[0] == ' ' || pC[0] == '\t' || pC[0] == '\r')) {
                        pC += 1;
                    }
                }

                if (pC == pChunkEnd) {
                    if (chunkSize == sizeof(pChunk)) {
                        if (onError) {
                            onError(pUserData, "Line is too long. A single line cannot exceed 4KB.", currentLine);
                        }

                        goto move_to_next_line;
                    } else {
                        // No more data. Just treat this one as a value-less key and return.
                        if (onPair) {
                            pKEnd[0] = '\0';
                            onPair(pUserData, pK, NULL);
                        }

                        return;
                    }
                }
            }

            if (pC[0] == '\n') {
                // Found the end of the line. Treat it as a value-less key.
                pKEnd[0] = '\0';
                if (onPair) {
                    onPair(pUserData, pK, NULL);
                }

                pC += 1;
                currentLine += 1;
                continue;
            }

            if (pC[0] == '#') {
                // Found a comment. Treat is as a value-less key and move to the end of the line.
                pKEnd[0] = '\0';
                if (onPair) {
                    onPair(pUserData, pK, NULL);
                }

                goto move_to_next_line;
            }

            char* pV = pC;

            // Find the last non-whitespace character.
            char* pVEnd = pC;
            while (pC < pChunkEnd && pC[0] != '\n' && pC[0] != '#') {
                if (pC[0] != ' ' && pC[0] != '\t' && pC[0] != '\r') {
                    pVEnd = pC;
                }

                pC += 1;
            }

            if (pC == pChunkEnd)
            {
                // Ran out of data. We need to move what we have of the key to the start of the chunk buffer, and then read more data.
                if (chunkSize == sizeof(pChunk))
                {
                    size_t lineSizeSoFar = pC - pK;
                    memmove(pChunk, pK, lineSizeSoFar);

                    chunkSize = lineSizeSoFar + onRead(pUserData, pChunk + lineSizeSoFar, sizeof(pChunk) - lineSizeSoFar);
                    pChunkEnd = pChunk + chunkSize;

                    pVEnd = pChunk + (pVEnd - pK);
                    pKEnd = pChunk + (pKEnd - pK);
                    pV = pChunk + (pV - pK);
                    pK = pChunk;
                    pC = pChunk + lineSizeSoFar;
                    while (pC < pChunkEnd && pC[0] != '\n' && pC[0] != '#') {
                        if (pC[0] != ' ' && pC[0] != '\t' && pC[0] != '\r') {
                            pVEnd = pC;
                        }

                        pC += 1;
                    }
                }

                if (pC == pChunkEnd) {
                    if (chunkSize == sizeof(pChunk)) {
                        if (onError) {
                            onError(pUserData, "Line is too long. A single line cannot exceed 4KB.", currentLine);
                        }

                        goto move_to_next_line;
                    }
                }
            }


            // Before null-terminating the value we first need to determine how we'll proceed after posting onPair.
            dtk_bool32 wasOnNL = pVEnd[1] == '\n';

            pKEnd[0] = '\0';
            pVEnd[1] = '\0';
            if (onPair) {
                onPair(pUserData, pK, pV);
            }

            if (wasOnNL)
            {
                // Was sitting on a new-line character.
                pC += 1;
                currentLine += 1;
                continue;
            }
            else
            {
                // Was sitting on a comment - just to the next line.
                goto move_to_next_line;
            }
        }
    }
}


typedef struct
{
    FILE* pFile;
    dtk_key_value_pair_proc onPair;
    dtk_key_value_error_proc onError;
    void* pOriginalUserData;
} dtk_parse_key_value_pairs_from_file_data;

size_t dtk_parse_key_value_pairs_from_file__on_read(void* pUserData, void* pDataOut, size_t bytesToRead)
{
    dtk_parse_key_value_pairs_from_file_data* pData = (dtk_parse_key_value_pairs_from_file_data*)pUserData;
    assert(pData != NULL);

    return fread(pDataOut, 1, bytesToRead, pData->pFile);
}

void dtk_parse_key_value_pairs_from_file__on_pair(void* pUserData, const char* key, const char* value)
{
    dtk_parse_key_value_pairs_from_file_data* pData = (dtk_parse_key_value_pairs_from_file_data*)pUserData;
    assert(pData != NULL);

    pData->onPair(pData->pOriginalUserData, key, value);
}

void dtk_parse_key_value_pairs_from_file__on_error(void* pUserData, const char* message, unsigned int line)
{
    dtk_parse_key_value_pairs_from_file_data* pData = (dtk_parse_key_value_pairs_from_file_data*)pUserData;
    assert(pData != NULL);

    pData->onError(pData->pOriginalUserData, message, line);
}

dtk_bool32 dtk_parse_key_value_pairs_from_file(const char* filePath, dtk_key_value_pair_proc onPair, dtk_key_value_error_proc onError, void* pUserData)
{
    dtk_parse_key_value_pairs_from_file_data data;
    if (dtk_fopen(filePath, "rb", &data.pFile) != DTK_SUCCESS) {
        if (onError) onError(pUserData, "Could not open file.", 0);
        return DTK_FALSE;
    }

    data.onPair = onPair;
    data.onError = onError;
    data.pOriginalUserData = pUserData;
    dtk_parse_key_value_pairs(dtk_parse_key_value_pairs_from_file__on_read, dtk_parse_key_value_pairs_from_file__on_pair, dtk_parse_key_value_pairs_from_file__on_error, &data);

    fclose(data.pFile);
    return DTK_TRUE;
}




dtk_string dtk_malloc_string(size_t sizeInBytesIncludingNullTerminator)
{
    if (sizeInBytesIncludingNullTerminator == 0) {
        return NULL;
    }

    return (dtk_string)dtk_calloc(sizeInBytesIncludingNullTerminator, 1);   // Use calloc() to ensure it's null terminated.
}

dtk_string dtk_make_string(const char* str)
{
    if (str == NULL) return NULL;
    
    size_t len = strlen(str);
    char* newStr = (char*)dtk_malloc(len+1);
    if (newStr == NULL) {
        return NULL;    // Out of memory.
    }

    dtk_strcpy_s(newStr, len+1, str);
    return newStr;
}

dtk_string dtk_make_stringv(const char* format, va_list args)
{
    if (format == NULL) format = "";

    va_list args2;
    va_copy(args2, args);

#if defined(_MSC_VER)
    int len = _vscprintf(format, args2);
#else
    int len = vsnprintf(NULL, 0, format, args2);
#endif

    va_end(args2);
    if (len < 0) {
        return NULL;
    }


    char* str = (char*)dtk_malloc(len+1);
    if (str == NULL) {
        return NULL;
    }

#if defined(_MSC_VER)
    len = vsprintf_s(str, len+1, format, args);
#else
    len = vsnprintf(str, len+1, format, args);
#endif

    return str;
}

dtk_string dtk_make_stringf(const char* format, ...)
{
    if (format == NULL) format = "";

    va_list args;
    va_start(args, format);

    char* str = dtk_make_stringv(format, args);

    va_end(args);
    return str;
}

dtk_string dtk_make_string_from_substring(const char* str, size_t strLen)
{
    if (str == NULL) return NULL;
    
    char* newStr = (char*)dtk_malloc(strLen+1);
    if (newStr == NULL) {
        return NULL;    // Out of memory.
    }

    dtk_strncpy_s(newStr, strLen+1, str, strLen);
    return newStr;
}

dtk_string dtk_set_string(dtk_string str, const char* newStr)
{
    if (newStr == NULL) newStr = "";

    if (str == NULL) {
        return dtk_make_string(newStr);
    } else {
        // If there's enough room for the new string don't bother reallocating.
        size_t oldStrCap = dtk_string_capacity(str);
        size_t newStrLen = strlen(newStr);

        if (oldStrCap < newStrLen) {
            str = (dtk_string)dtk_realloc(str, newStrLen + 1);  // +1 for null terminator.
            if (str == NULL) {
                return NULL;    // Out of memory.
            }
        }

        memcpy(str, newStr, newStrLen+1);   // +1 to include the null terminator.
        return str;
    }
}

dtk_string dtk_append_string(dtk_string lstr, const char* rstr)
{
    if (rstr == NULL) {
        rstr = "";
    }

    if (lstr == NULL) {
        return dtk_make_string(rstr);
    }

    size_t lstrLen = dtk_string_length(lstr);
    size_t rstrLen = strlen(rstr);
    char* str = (char*)dtk_realloc(lstr, lstrLen + rstrLen + 1);
    if (str == NULL) {
        return NULL;
    }

    memcpy(str + lstrLen, rstr, rstrLen);
    str[lstrLen + rstrLen] = '\0';

    return str;
}

dtk_string dtk_append_stringv(dtk_string lstr, const char* format, va_list args)
{
    dtk_string rstr = dtk_make_stringv(format, args);
    if (rstr == NULL) {
        return NULL;    // Probably out of memory.
    }

    char* str = dtk_append_string(lstr, rstr);

    dtk_free_string(rstr);
    return str;
}

dtk_string dtk_append_stringf(dtk_string lstr, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    char* str = dtk_append_stringv(lstr, format, args);

    va_end(args);
    return str;
}

dtk_string dtk_append_substring(dtk_string lstr, const char* rstr, size_t rstrLen)
{
    if (rstr == NULL) {
        rstr = "";
    }

    if (lstr == NULL) {
        return dtk_make_string_from_substring(rstr, rstrLen);
    }

    size_t lstrLen = dtk_string_length(lstr);
    char* str = (char*)dtk_realloc(lstr, lstrLen + rstrLen + 1);
    if (str == NULL) {
        return NULL;
    }

    dtk_strncat_s(str, lstrLen + rstrLen + 1, rstr, rstrLen);
    str[lstrLen + rstrLen] = '\0';

    return str;
}

size_t dtk_string_length(dtk_string str)
{
    return strlen(str);
}

size_t dtk_string_capacity(dtk_string str)
{
    // Currently we're not doing anything fancy with the memory management of strings, but this API is used right now
    // so that future optimizations are easily enabled.
    return dtk_string_length(str);
}

void dtk_free_string(dtk_string str)
{
    dtk_free(str);
}
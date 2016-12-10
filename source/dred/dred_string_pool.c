// Copyright (C) 2016 David Reid. See included LICENSE file.

// Optimization Ideas
// ==================
// - Consider embedding the length of each string for faster searching and comparisons.
// - Consider aligning each string to a 32 or 64 bit boundary for fast SIMD-ified comparisons.

#define DRED_STRING_POOL_CHUNK_SIZE     256

dred_result dred_string_pool_init(dred_string_pool* pPool, const char* pInitialData, size_t initialDataSize)
{
    if (pPool == NULL) return DRED_INVALID_ARGS;
    dtk_zero_object(pPool);

    if (pInitialData != NULL) {
        if (initialDataSize == (size_t)-1) {
            initialDataSize = strlen(pInitialData)+1;
        }

        pPool->capacity = dr_round_up(initialDataSize+1, DRED_STRING_POOL_CHUNK_SIZE);    // <-- +1 because we always need the first byte to be a null terminator.
    } else {
        pPool->capacity = DRED_STRING_POOL_CHUNK_SIZE;
    }

    pPool->pData = (char*)malloc(pPool->capacity);
    if (pPool->pData == NULL) {
        return DRED_OUT_OF_MEMORY;
    }

    // Initial size of 1 for the first entry which is just a null terminator.
    pPool->byteCount = 1;
    pPool->pData[0] = '\0';

    if (pInitialData != NULL) {
        memcpy(pPool->pData+1, pInitialData, initialDataSize);
    }

    return DRED_SUCCESS;
}

dred_result dred_string_pool_uninit(dred_string_pool* pPool)
{
    if (pPool == NULL) return DRED_INVALID_ARGS;
    free(pPool->pData);
}


size_t dred_string_pool_add(dred_string_pool* pPool, const char* str, size_t strLen)
{
    if (pPool == NULL || str == NULL) return 0;

    if (strLen == (size_t)-1) {
        strLen = strlen(str);
    }

    size_t offset = pPool->byteCount;
    if (offset + strLen+1 > pPool->capacity) {
        size_t newCapacity = dr_round_up(pPool->capacity + strLen+1, DRED_STRING_POOL_CHUNK_SIZE);
        char* pNewData = (char*)realloc(pPool->pData, newCapacity);
        if (pNewData == NULL) {
            return DRED_OUT_OF_MEMORY;
        }

        pPool->pData = pNewData;
        pPool->capacity = newCapacity;
    }

    assert(offset + strLen+1 <= pPool->capacity);

    // When copying the input string into the pool, keep in mind that the input string may not have been null terminated.
    memcpy(pPool->pData+offset, str, strLen);
    pPool->pData[offset+strLen] = '\0';
    pPool->byteCount += strLen+1;

    return offset;
}

size_t dred_string_pool_find(dred_string_pool* pPool, const char* str)
{
    if (pPool == NULL || dtk_string_is_null_or_empty(str)) {
        return 0;
    }

    // NOTE: This can be optimized. See optimization notes at the top of this file.

    size_t strLen = strlen(str);

    for (size_t i = 0; i < pPool->byteCount; /* DO NOTHING */) {
        char* pNextStr = pPool->pData + i;
        size_t nextStrLen = strlen(pNextStr);
        if (strLen == nextStrLen) {
            if (strcmp(pNextStr, str) == 0) {
                return i;   // Found it.
            }
        }

        i += nextStrLen+1;
    }

    return 0;   // Didn't find it.
}
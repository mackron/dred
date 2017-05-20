// Copyright (C) 2017 David Reid. See included LICENSE file.

// Optimization Ideas
// ==================
// - Consider embedding the length of each string for faster searching and comparisons.
// - Consider aligning each string to a 32 or 64 bit boundary for fast SIMD-ified comparisons.

#define DRED_STRING_POOL_CHUNK_SIZE     256

dr_bool32 dred_string_pool_init(dred_string_pool* pPool, const char* pInitialData, size_t initialDataSize)
{
    if (pPool == NULL) return DR_FALSE;
    memset(pPool, 0, sizeof(*pPool));

    if (pInitialData != NULL) {
        pPool->capacity = dr_round_up(initialDataSize, DRED_STRING_POOL_CHUNK_SIZE);
    } else {
        pPool->capacity = DRED_STRING_POOL_CHUNK_SIZE;
    }

    pPool->pData = (char*)malloc(pPool->capacity);
    if (pPool->pData == NULL) {
        return DR_FALSE;
    }

    if (pInitialData != NULL) {
        pPool->byteCount = initialDataSize;
        memcpy(pPool->pData, pInitialData, initialDataSize);
    } else {
        pPool->byteCount = 1;
        pPool->pData[0] = '\0';
    }

    return DR_TRUE;
}

dr_bool32 dred_string_pool_uninit(dred_string_pool* pPool)
{
    if (pPool == NULL) return DR_FALSE;

    free(pPool->pData);
    return DR_TRUE;
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
            return 0;
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

dr_bool32 dred_string_pool_find(dred_string_pool* pPool, const char* str, size_t* pOffset)
{
    if (pOffset) *pOffset = 0;
    if (pPool == NULL || str == NULL || str[0] == '\0') {
        return DR_FALSE;
    }

    // NOTE: This can be optimized. See optimization notes at the top of this file.

    size_t strLen = strlen(str);

    for (size_t i = 0; i < pPool->byteCount; /* DO NOTHING */) {
        char* pNextStr = pPool->pData + i;
        size_t nextStrLen = strlen(pNextStr);
        if (strLen == nextStrLen) {
            if (strcmp(pNextStr, str) == 0) {
                if (pOffset) *pOffset = i;
                return DR_TRUE; // Found it.
            }
        }

        i += nextStrLen+1;
    }

    return DR_FALSE;   // Didn't find it.
}


size_t dred_string_pool_find_or_add(dred_string_pool* pPool, const char* str)
{
    size_t offset = 0;
    if (str != NULL && str[0] != '\0' && !dred_string_pool_find(pPool, str, &offset)) {
        offset = dred_string_pool_add(pPool, str, (size_t)-1);
    }

    return offset;
}

const char* dred_string_pool_cstr(dred_string_pool* pPool, size_t offset)
{
    if (pPool == NULL || offset >= pPool->byteCount) return NULL;
    return pPool->pData + offset;
}

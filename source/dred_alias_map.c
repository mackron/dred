
bool dred_alias_map_init(dred_alias_map* pMap)
{
    if (pMap == NULL) {
        return false;
    }

    memset(pMap, 0, sizeof(*pMap));
    return true;
}

void dred_alias_map_uninit(dred_alias_map* pMap)
{
    if (pMap == NULL) {
        return;
    }

    for (size_t i = 0; i < pMap->count; ++i) {
        gb_free_string(pMap->keys[i]);
        gb_free_string(pMap->values[i]);
    }

    free(pMap->keys);
    free(pMap->values);
}

void dred_alias_map_add(dred_alias_map* pMap, const char* key, const char* value)
{
    if (pMap == NULL) {
        return;
    }
    
    // Remove the existing alias first.
    dred_alias_map_remove(pMap, key);


    if (pMap->count == pMap->bufferSize) {
        size_t newBufferSize = (pMap->count == 0) ? 16 : pMap->bufferSize * 2;

        char** newKeys = (char**)realloc(pMap->keys, newBufferSize * sizeof(*newKeys));
        if (newKeys == NULL) {
            return;
        }

        char** newValues = (char**)realloc(pMap->values, newBufferSize * sizeof(*newValues));
        if (newValues == NULL) {
            return;
        }

        pMap->bufferSize = newBufferSize;
        pMap->keys = newKeys;
        pMap->values = newValues;
    }

    assert(pMap->count < pMap->bufferSize);

    pMap->keys[pMap->count] = gb_make_string(key);
    pMap->values[pMap->count] = gb_make_string(value);
    pMap->count += 1;
}

void dred_alias_map_remove(dred_alias_map* pMap, const char* key)
{
    size_t i;
    if (!dred_alias_map_find(pMap, key, &i)) {
        return;
    }

    dred_alias_map_remove_by_index(pMap, i);
}

void dred_alias_map_remove_by_index(dred_alias_map* pMap, size_t index)
{
    if (pMap == NULL || index >= pMap->count) {
        return;
    }

    gb_free_string(pMap->keys[index]);
    gb_free_string(pMap->values[index]);
    
    if (pMap->count > 1) {
        for (size_t i = index; i < pMap->count-1; ++i) {
            pMap->keys[i] = pMap->keys[i+1];
            pMap->values[i] = pMap->values[i+1];
        }
    }

    pMap->count -= 1;
}


const char* dred_alias_map_get_value(dred_alias_map* pMap, const char* key)
{
    size_t i;
    if (!dred_alias_map_find(pMap, key, &i)) {
        return NULL;
    }

    return dred_alias_map_get_value_by_index(pMap, i);
}

const char* dred_alias_map_get_value_by_index(dred_alias_map* pMap, size_t index)
{
    if (pMap == NULL || index >= pMap->count) {
        return NULL;
    }

    return pMap->values[index];
}


bool dred_alias_map_find(dred_alias_map* pMap, const char* key, size_t* pIndexOut)
{
    if (pMap == NULL || key == NULL) {
        return false;
    }

    // Linear for now.
    for (size_t i = 0; i < pMap->count; ++i) {
        if (strcmp(pMap->keys[i], key) == 0) {
            if (pIndexOut) *pIndexOut = i;
            return true;
        }
    }

    return false;
}

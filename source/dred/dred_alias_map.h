// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    size_t count;
    size_t bufferSize;
    char** keys;
    char** values;
} dred_alias_map;

dtk_bool32 dred_alias_map_init(dred_alias_map* pMap);
void dred_alias_map_uninit(dred_alias_map* pMap);

void dred_alias_map_add(dred_alias_map* pMap, const char* key, const char* value);

void dred_alias_map_remove(dred_alias_map* pMap, const char* key);
void dred_alias_map_remove_by_index(dred_alias_map* pMap, size_t index);

const char* dred_alias_map_get_value(dred_alias_map* pMap, const char* key);
const char* dred_alias_map_get_value_by_index(dred_alias_map* pMap, size_t index);

dtk_bool32 dred_alias_map_find(dred_alias_map* pMap, const char* key, size_t* pIndexOut);

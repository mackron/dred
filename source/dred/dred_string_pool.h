// Copyright (C) 2016 David Reid. See included LICENSE file.

// NOTES:
// - The first string in the pool is _always_ an empty string. Rationale: It allows an offset of 0 to be used
//   for error conditions while also representing an empty string.
// - This data structure is best used when the number of strings are relatively small, constant and long-
//   living. Adding and searching for strings runs in linear time so it becomes less suitable as it becomes
//   larger and the frequency of your search queries increase.

typedef struct
{
    size_t capacity;
    size_t byteCount;
    char* pData;
} dred_string_pool;

// Initializes a string pool.
//
// Note that <initialDataSize> _must_ include null terminators.
dred_result dred_string_pool_init(dred_string_pool* pPool, const char* pInitialData, size_t initialDataSize);

// Uninitializes a string pool.
dred_result dred_string_pool_uninit(dred_string_pool* pPool);

// Adds a string to the pool, regardless of whether or not the string already exists.
//
// Use dred_string_pool_find() to look for an existing occurance of the string.
size_t dred_string_pool_add(dred_string_pool* pPool, const char* str, size_t strLen);

// Finds a string in the given pool.
//
// If the string does not exist, 0 is returned. This runs in linear time.
size_t dred_string_pool_find(dred_string_pool* pPool, const char* str);
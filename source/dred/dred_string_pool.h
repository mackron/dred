// Copyright (C) 2017 David Reid. See included LICENSE file.

// The idea of a string pool is to simplify memory management of strings and to support the ability to
// re-use those strings. When referencing a string in a string pool, it is done so with an offset
// rather than an actual pointer. The reason for this is that the memory location of the internal memory
// allocation can change, whereas the offset of each string will remain constant. A string pool can expand,
// but will never shrink.
//
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
dtk_bool32 dred_string_pool_init(dred_string_pool* pPool, const char* pInitialData, size_t initialDataSize);

// Uninitializes a string pool.
dtk_bool32 dred_string_pool_uninit(dred_string_pool* pPool);

// Adds a string to the pool, regardless of whether or not the string already exists.
//
// Use dred_string_pool_find() to look for an existing occurance of the string.
size_t dred_string_pool_add(dred_string_pool* pPool, const char* str, size_t strLen);

// Finds a string in the given pool.
//
// Returns true if the string exists; false otherwise. If the string does not exist, 0 is returned in <pOffset>. This runs in linear time.
dtk_bool32 dred_string_pool_find(dred_string_pool* pPool, const char* str, size_t* pOffset);

// A helper for finding a string, and if not found, adding it.
//
// Returns the offset of the string. Returns 0 if an error occurs.
size_t dred_string_pool_find_or_add(dred_string_pool* pPool, const char* str);

// Retrieves a C-style string from the given string offset.
const char* dred_string_pool_cstr(dred_string_pool* pPool, size_t offset);
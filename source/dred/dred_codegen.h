// Copyright (C) 2017 David Reid. See included LICENSE file.

// Converts the given buffer to a C-style static const unsigned char array.
//
// Free the returned pointer with free().
char* dred_codegen_buffer_to_c_array(const unsigned char* buffer, size_t size, const char* variableName);

// Converts the given buffer to a C-style static const char* string.
//
// Free the returned pointer with free().
//
// If "variableName" is null, the variable declaration will be excluded.
char* dred_codegen_buffer_to_c_string(const unsigned char* buffer, size_t size, const char* variableName);
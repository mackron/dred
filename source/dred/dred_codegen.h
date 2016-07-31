// Copyright (C) 2016 David Reid. See included LICENSE file.

// Converts the given buffer to a C-style static const unsigned char array.
//
// Free the returned pointer with free().
char* dred_codegen_buffer_to_c_array(const unsigned char* buffer, unsigned int size, const char* variableName);
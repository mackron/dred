// Copyright (C) 2017 David Reid. See included LICENSE file.

// Retrieves the file data of the given file. Free the returned pointer with dtk_free().
dtk_result dtk_open_and_read_file(const char* filePath, size_t* pFileSizeOut, void** ppFileData);

// Retrieves the file data of the given file as a null terminated string. Free the returned pointer with dtk_free(). The
// returned file size is the length of the string not including the null terminator.
dtk_result dtk_open_and_read_text_file(const char* filePath, size_t* pFileSizeOut, char** ppFileData);

// Creates a new file with the given data.
dtk_result dtk_open_and_write_file(const char* filePath, const void* pData, size_t dataSize);

// Creates a new file with the given string.
dtk_result dtk_open_and_write_text_file(const char* filePath, const char* text);

// Checks if the given path is an existing directory.
dtk_bool32 dtk_is_directory(const char* path);


// Retrieves the current directory. Free the returned string with dtk_free(). Returns NULL on error.
char* dtk_get_current_directory();

// Sets the current directory.
dtk_result dtk_set_current_directory(const char* path);
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

// Determines whether or not the given file path is to a file.
//
// This will return DR_FALSE if the path points to a directory.
dtk_bool32 dtk_file_exists(const char* filePath);

// Determines whether or not the given file path points to a directory.
//
// This will return DR_FALSE if the path points to a file.
DTK_INLINE dtk_bool32 dtk_directory_exists(const char* directoryPath) { return dtk_is_directory(directoryPath); }

// Deletes the file at the given path.
//
// This uses remove() on POSIX platforms and DeleteFile() on Windows platforms.
dtk_result dtk_delete_file(const char* filePath);

// Cross-platform wrapper for creating a directory.
dtk_result dtk_mkdir(const char* directoryPath);

// Recursively creates a directory.
dtk_result dtk_mkdir_recursive(const char* directoryPath);


// Callback function for file iteration.
typedef dtk_bool32 (* dtk_iterate_files_proc)(const char* filePath, void* pUserData);

// Iterates over every file and folder of the given directory.
dtk_bool32 dtk_iterate_files(const char* directory, dtk_bool32 recursive, dtk_iterate_files_proc proc, void* pUserData);


// Retrieves the current directory. Free the returned string with dtk_free(). Returns NULL on error.
char* dtk_get_current_directory();

// Sets the current directory.
dtk_result dtk_set_current_directory(const char* path);
// Copyright (C) 2019 David Reid. See included LICENSE file.

// Wrapper API for fopen() for cleanly compiling against supported compilers.
dtk_result dtk_fopen(const char* filePath, const char* openMode, FILE** ppFile);

// Helper for creating an empty file.
dtk_result dtk_create_empty_file(const char* fileName, dtk_bool32 failIfExists);

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
// This will return DTK_FALSE if the path points to a directory.
dtk_bool32 dtk_file_exists(const char* filePath);

// Determines whether or not the given file path points to a directory.
//
// This will return DTK_FALSE if the path points to a file.
DTK_INLINE dtk_bool32 dtk_directory_exists(const char* directoryPath) { return dtk_is_directory(directoryPath); }

// Moves a file.
//
// This uses rename() on POSIX platforms and MoveFileEx(oldPath, newPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH) on windows platforms.
dtk_result dtk_move_file(const char* oldPath, const char* newPath);

// Copies a file.
dtk_result dtk_copy_file(const char* srcPath, const char* dstPath, dtk_bool32 failIfExists);

// Determines if the given file is read only.
dtk_bool32 dtk_is_file_read_only(const char* filePath);

// Retrieves the last modified time of the file at the given path.
dtk_uint64 dtk_get_file_modified_time(const char* filePath);

// Deletes the file at the given path.
//
// This uses remove() on POSIX platforms and DeleteFile() on Windows platforms.
dtk_result dtk_delete_file(const char* filePath);

// Cross-platform wrapper for creating a directory.
dtk_result dtk_mkdir(const char* directoryPath);

// Recursively creates a directory.
dtk_result dtk_mkdir_recursive(const char* directoryPath);

// Recursively creates the directory containing the specified file.
//
// This is a helper API that strips off the file name from the specified path and calls dtk_mkdir_recursive() from that base path. The
// specified path must include the name of the file.
dtk_result dtk_mkdir_for_file(const char* filePath);


// Callback function for file iteration.
typedef dtk_bool32 (* dtk_iterate_files_proc)(const char* filePath, void* pUserData);

// Iterates over every file and folder of the given directory.
dtk_bool32 dtk_iterate_files(const char* directory, dtk_bool32 recursive, dtk_iterate_files_proc proc, void* pUserData);


// Retrieves the current directory. Free the returned string with dtk_free(). Returns NULL on error.
char* dtk_get_current_directory();

// Sets the current directory.
dtk_result dtk_set_current_directory(const char* path);

// Converts a relative path to absolute based on the current directory.
//
// Free the returned string with dtk_free(). Returns NULL on error.
char* dtk_make_absolute_path_from_current_directory(const char* pRelativePath);

// Converts an absolute path to relative based on the current directory.
//
// Free the returned string with dtk_free(). Returns NULL on error.
char* dtk_make_relative_path_from_current_directory(const char* pAbsolutePath);
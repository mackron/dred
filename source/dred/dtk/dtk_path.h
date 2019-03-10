// Copyright (C) 2018 David Reid. See included LICENSE file.

// Structure representing a section of a path.
typedef struct
{
    size_t offset;
    size_t length;
} dtk_path_segment;

// Structure used for iterating over a path.
typedef struct
{
    const char* path;
    dtk_path_segment segment;
} dtk_path_iterator;

// Compares a section of two strings for equality.
//
// s0Path [in] The first path.
// s0     [in] The segment of the first path to compare.
// s1Path [in] The second path.
// s1     [in] The segment of the second path to compare.
//
// Returns true if the strings are equal; false otherwise.
dtk_bool32 dtk_path_segments_equal(const char* s0Path, const dtk_path_segment s0, const char* s1Path, const dtk_path_segment s1);

// Creates an iterator for iterating over each segment in a path.
//
// path [in] The path whose segments are being iterated.
//
// Returns true if at least one segment is found; false if it's an empty path.
dtk_bool32 dtk_path_first(const char* path, dtk_path_iterator* i);

// Creates an iterator beginning at the last segment.
dtk_bool32 dtk_path_last(const char* path, dtk_path_iterator* i);

// Goes to the next segment in the path as per the given iterator.
//
// i [in] A pointer to the iterator to increment.
//
// Returns true if the iterator contains a valid value. Use this to determine when to terminate iteration.
dtk_bool32 dtk_path_next(dtk_path_iterator* i);

// Goes to the previous segment in the path.
//
// i [in] A pointer to the iterator to decrement.
//
// Returns true if the iterator contains a valid value. Use this to determine when to terminate iteration.
dtk_bool32 dtk_path_prev(dtk_path_iterator* i);

// Determines if the given iterator is at the end.
//
// i [in] The iterator to check.
dtk_bool32 dtk_path_at_end(dtk_path_iterator i);

// Determines if the given iterator is at the start.
//
// i [in] The iterator to check.
dtk_bool32 dtk_path_at_start(dtk_path_iterator i);

// Compares the string values of two iterators for equality.
//
// i0 [in] The first iterator to compare.
// i1 [in] The second iterator to compare.
//
// Returns true if the strings are equal; false otherwise.
dtk_bool32 dtk_path_iterators_equal(const dtk_path_iterator i0, const dtk_path_iterator i1);


// Determines whether or not the given iterator refers to the root segment of a path.
dtk_bool32 dtk_path_is_root(const char* path);
dtk_bool32 dtk_path_is_root_segment(const char* path, const dtk_path_segment segment);

// Determines whether or not the given iterator refers to a Unix style root directory ("/")
dtk_bool32 dtk_path_is_unix_style_root(const char* path);
dtk_bool32 dtk_path_is_unix_style_root_segment(const char* path, const dtk_path_segment segment);

// Determines whether or not the given iterator refers to a Windows style root directory.
dtk_bool32 dtk_path_is_win32_style_root(const char* path);
dtk_bool32 dtk_path_is_win32_style_root_segment(const char* path, const dtk_path_segment segment);


// Converts the slashes in the given path to forward slashes.
//
// path [in] The path whose slashes are being converted.
void dtk_path_to_forward_slashes(char* path);

// Converts the slashes in the given path to back slashes.
//
// path [in] The path whose slashes are being converted.
void dtk_path_to_backslashes(char* path);


// Determines whether or not the given path is a decendant of another.
//
// descendantAbsolutePath [in] The absolute path of the descendant.
// parentAbsolutePath     [in] The absolute path of the parent.
//
// As an example, "C:/My/Folder" is a descendant of "C:/".
//
// If either path contains "." or "..", clean it with dtk_path_clean() before calling this.
dtk_bool32 dtk_path_is_descendant(const char* descendantAbsolutePath, const char* parentAbsolutePath);

// Determines whether or not the given path is a direct child of another.
//
// childAbsolutePath [in] The absolute of the child.
// parentAbsolutePath [in] The absolute path of the parent.
//
// As an example, "C:/My/Folder" is NOT a child of "C:/" - it is a descendant. Alternatively, "C:/My" IS a child of "C:/".
//
// If either path contains "." or "..", clean it with dtk_path_clean() before calling this.
dtk_bool32 dtk_path_is_child(const char* childAbsolutePath, const char* parentAbsolutePath);


// Finds the file name portion of the path.
//
// path [in] The path to search.
//
// Returns a pointer to the beginning of the string containing the file name. If this is non-null, it will be an offset of "path".
//
// A path with a trailing slash will return an empty string.
//
// The return value is just an offset of "path".
const char* dtk_path_file_name(const char* path);

// Finds the file extension of the given file path.
//
// path [in] The path to search.
//
// Returns a pointer to the beginning of the string containing the file's extension. This is an offset of "path".
//
// A path with a trailing slash will return an empty string.
//
// The return value is just an offset of "path".
//
// On a path such as "filename.ext1.ext2" the returned string will be "ext2".
const char* dtk_path_extension(const char* path);

// Determines whether or not the specified path has an extension.
dtk_bool32 dtk_path_has_extension(const char* path);


// Checks whether or not the two paths are equal.
//
// path1 [in] The first path.
// path2 [in] The second path.
//
// Returns true if the paths are equal; false otherwise.
//
// This is case-sensitive.
//
// This is more than just a string comparison. Rather, this splits the path and compares each segment. The path "C:/My/Folder" is considered
// equal to to "C:\\My\\Folder".
dtk_bool32 dtk_path_equal(const char* path1, const char* path2);

// Checks if the extension of the given path is equal to the given extension.
//
// This is not case sensitive. If you want this to be case sensitive, just do "strcmp(dtk_path_extension(path), extension)".
dtk_bool32 dtk_path_extension_equal(const char* path, const char* extension);


// Determines whether or not the given path is relative.
//
// path [in] The path to check.
dtk_bool32 dtk_path_is_relative(const char* path);

// Determines whether or not the given path is absolute.
//
// path [in] The path to check.
dtk_bool32 dtk_path_is_absolute(const char* path);


// Retrieves the base path from the given path, not including the trailing slash.
//
// pathOut     [out] The destination buffer.
// pathOutSize [in]  The size of the buffer pointed to by "pathOut", in bytes.
// path        [in]  The input path.
//
// Returns the length of the returned string, _including_ the null terminator; 0 on error.
//
// As an example, when "path" is "C:/MyFolder/MyFile", the output will be "C:/MyFolder". Note that there is no trailing slash.
//
// If "path" is something like "/MyFolder", the return value will be an empty string.
size_t dtk_path_base_path(char* pathOut, size_t pathOutSize, const char* path);

// Retrieves the file name portion of the given path, without the extension.
//
// pathOut     [out] The destination buffer.
// pathOutSize [in]  The size of the buffer pointed to by "pathOut", in bytes.
// path        [in]  The input path.
size_t dtk_path_file_name_without_extension(char* pathOut, size_t pathOutSize, const char* path);


// Appends two paths together, and copies them to a separate buffer.
//
// pathOut     [out] The destination buffer.
// pathOutSize [in]  The size of the buffer pointed to by "pathOut", in bytes.
// base        [in]  The base directory.
// other       [in]  The relative path to append to "base".
//
// Returns the length of the resulting string including the null terminator. Returns 0 on error.
//
// This assumes both paths are well formed and "other" is a relative path.
//
// "pathOut" and "base" are allowed to be the same pointer, in which case the other path is appended in-place.
size_t dtk_path_append(char* pathOut, size_t pathOutSize, const char* base, const char* other);

// Appends a base path and an iterator together, and copyies them to a separate buffer.
//
// pathOut     [out] The destination buffer.
// pathOutSize [in]  The size of the buffer pointed to by "dst", in bytes.
// base        [in]  The base directory.
// i           [in]  The iterator to append.
//
// Returns the length of the resulting string including the null terminator. Returns 0 on error.
//
// This assumes both paths are well formed and "i" is a valid iterator.
//
// "pathOut" and "base" are allowed to be the same pointer, in which case the other path is appended in-place.
size_t dtk_path_append_iterator(char* pathOut, size_t pathOutSize, const char* base, dtk_path_iterator i);

// Appends an extension to the given base path and copies them to a separate buffer.
//
// pathOut     [out] The destination buffer.
// pathOutSize [in]  The size of the buffer pointed to by "dst", in bytes.
// base        [in]  The base directory.
// extension   [in]  The relative path to append to "base".
//
// Returns the length of the resulting string including the null terminator. Returns 0 on error.
//
// "pathOut" and "base" are allowed to be the same pointer, in which case the other path is appended in-place.
size_t dtk_path_append_extension(char* pathOut, size_t pathOutSize, const char* base, const char* extension);


// Cleans the path and resolves the ".." and "." segments.
//
// pathOut     [out] A pointer to the buffer that will receive the path.
// pathOutSize [in]  The size of the buffer pointed to by pathOut, in bytes.
// path        [in]  The path to clean.
//
// Returns the number of bytes written to the output buffer, including the null terminator.
//
// The output path will never be longer than the input path.
//
// The output buffer should never overlap with the input path.
//
// As an example, the path "my/messy/../path" will result in "my/path"
//
// The path "my/messy/../../../path" (note how there are too many ".." segments) will return "path" (the extra ".." segments will be dropped.)
//
// If an error occurs, such as an invalid input path, 0 will be returned.
size_t dtk_path_clean(char* pathOut, size_t pathOutSize, const char* path);

// Appends one path to the other and then cleans it.
size_t dtk_path_append_and_clean(char* pathOut, size_t pathOutSize, const char* base, const char* other);


// Creates a copy of the given string and removes the extension.
size_t dtk_path_remove_extension(char* pathOut, size_t pathOutSize, const char* path);

// Creates a copy of the given string and removes the extension.
size_t dtk_path_remove_file_name(char* pathOut, size_t pathOutSize, const char* path);
size_t dtk_path_remove_file_name_in_place(char* path);


// Converts an absolute path to a relative path.
//
// Returns the length of the resulting string plus the null terminator. Returns on error.
//
// This will normalize every slash to forward slashes.
size_t dtk_path_to_relative(char* pathOut, size_t pathOutSize, const char* absolutePathToMakeRelative, const char* absolutePathToMakeRelativeTo);

// Converts a relative path to an absolute path based on a base path.
//
// Returns the length of the resulting string plus the null terminator. Returns on error.
//
// This is equivalent to an append followed by a clean. Slashes will be normalized to forward slashes.
size_t dtk_path_to_absolute(char* pathOut, size_t pathOutSize, const char* relativePathToMakeAbsolute, const char* basePath);



///////////////////////////////////////////////////////////////////////////////
//
// High Level APIs
//
///////////////////////////////////////////////////////////////////////////////

// Same as dtk_path_file_name_without_extension() except returns the result as a new dtk_string.
//
// Free the returned string with dtk_free_string().
dtk_string dtk_path_file_name_without_extension_str(const char* path);
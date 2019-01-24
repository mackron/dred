// Copyright (C) 2018 David Reid. See included LICENSE file.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Win32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
dtk_bool32 dtk_is_directory__win32(const char* path)
{
    dtk_assert(path != NULL);

    DWORD attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

dtk_bool32 dtk_file_exists__win32(const char* filePath)
{
    dtk_assert(filePath != NULL);

    DWORD attributes = GetFileAttributesA(filePath);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

char* dtk_get_current_directory__win32()
{
    DWORD len = GetCurrentDirectoryA(0, NULL);
    if (len == 0) {
        return NULL;
    }

    char* pDir = (char*)dtk_malloc(len);
    if (pDir == NULL) {
        return NULL;    // Out of memory.
    }

    len = GetCurrentDirectoryA(len, pDir);
    if (len == 0) {
        dtk_free(pDir);
        return NULL;
    }

    return pDir;
}

dtk_result dtk_set_current_directory__win32(const char* path)
{
    if (SetCurrentDirectoryA(path) == 0) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// POSIX
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_POSIX
dtk_bool32 dtk_is_directory__posix(const char* path)
{
    dtk_assert(path != NULL);

    struct stat info;
    if (stat(path, &info) != 0) {
        return DTK_FALSE;   // Likely the folder doesn't exist.
    }

    return (info.st_mode & S_IFDIR) != 0;
}

dtk_bool32 dtk_file_exists__posix(const char* filePath)
{
    dtk_assert(filePath != NULL);

    struct stat info;
    if (stat(filePath, &info) != 0) {
        return DTK_FALSE;   // Likely the folder doesn't exist.
    }

    return (info.st_mode & S_IFDIR) == 0;
}

char* dtk_get_current_directory__posix()
{
    char* pDirTemp = getcwd(NULL, 0);
    if (pDirTemp == NULL) {
        return NULL;
    }

    // Unfortunately getcwd() explicitly uses malloc() for the allocation, however we require the caller
    // to free with dtk_free(). To ensure correctness, we need to allocate it again :(
    size_t len = strlen(pDirTemp);
    char* pDir = (char*)dtk_malloc(len + 1);
    if (pDir == NULL) {
        return NULL;    // Out of memory.
    }

    dtk_strcpy(pDir, pDirTemp);
    free(pDirTemp);

    return pDir;
}

dtk_result dtk_set_current_directory__posix(const char* path)
{
    if (chdir(path) != 0) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}
#endif


dtk_result dtk_fopen(const char* filePath, const char* openMode, FILE** ppFile)
{
    if (filePath == NULL || openMode == NULL || ppFile == NULL) {
        return DTK_INVALID_ARGS;
    }

#if _MSC_VER
    if (fopen_s(ppFile, filePath, openMode) != 0) {
        return DTK_FAILED_TO_OPEN_FILE;
    }
#else
    FILE* pFile = fopen(filePath, openMode);
    if (pFile == NULL) {
        return DTK_FAILED_TO_OPEN_FILE;
    }

    *ppFile = pFile;
#endif

    return DTK_SUCCESS;
}

dtk_result dtk_create_empty_file(const char* fileName, dtk_bool32 failIfExists)
{
    if (fileName == NULL) {
        return DTK_INVALID_ARGS;
    }

#ifdef _WIN32
    DWORD dwCreationDisposition;
    if (failIfExists) {
        dwCreationDisposition = CREATE_NEW;
    } else {
        dwCreationDisposition = CREATE_ALWAYS;
    }

    HANDLE hFile = CreateFileA(fileName, FILE_GENERIC_WRITE, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return dtk_win32_error_to_result(GetLastError());
    }

    CloseHandle(hFile);
    return DTK_SUCCESS;
#else
    int flags = O_WRONLY | O_CREAT;
    if (failIfExists) {
        flags |= O_EXCL;
    } else {
        flags |= O_TRUNC;
    }
    int fd = open(fileName, flags, 0666);
    if (fd == -1) {
        return dtk_errno_to_result(errno);
    }

    close(fd);
    return DTK_SUCCESS;
#endif
}

static dtk_result dtk_open_and_read_file_with_extra_data(const char* filePath, size_t* pFileSizeOut, void** ppFileData, size_t extraBytes)
{
    // Safety.
    if (pFileSizeOut) {
        *pFileSizeOut = 0;
    }
    if (ppFileData) {
        *ppFileData = NULL;
    }

    if (filePath == NULL) {
        return DTK_INVALID_ARGS;
    }

    // TODO: Use 64-bit versions of the FILE APIs.

    FILE* pFile;
    dtk_result result = dtk_fopen(filePath, "rb", &pFile);
    if (result != DTK_SUCCESS) {
        return DTK_FAILED_TO_OPEN_FILE;
    }

    fseek(pFile, 0, SEEK_END);
    dtk_uint64 fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    if (fileSize + extraBytes > SIZE_MAX) {
        fclose(pFile);
        return DTK_FILE_TOO_BIG;
    }

    void* pFileData = dtk_malloc((size_t)fileSize + extraBytes);    // <-- Safe cast due to the check above.
    if (pFileData == NULL) {
        fclose(pFile);
        return DTK_OUT_OF_MEMORY;
    }

    size_t bytesRead = fread(pFileData, 1, (size_t)fileSize, pFile);
    if (bytesRead != fileSize) {
        dtk_free(pFileData);
        fclose(pFile);
        return DTK_FAILED_TO_READ_FILE;
    }

    fclose(pFile);

    if (pFileSizeOut) {
        *pFileSizeOut = (size_t)fileSize;
    }

    if (ppFileData) {
        *ppFileData = pFileData;
    } else {
        dtk_free(pFileData);
    }

    return DTK_SUCCESS;
}

dtk_result dtk_open_and_read_file(const char* filePath, size_t* pFileSizeOut, void** ppFileData)
{
    return dtk_open_and_read_file_with_extra_data(filePath, pFileSizeOut, ppFileData, 0);
}

dtk_result dtk_open_and_read_text_file(const char* filePath, size_t* pFileSizeOut, char** ppFileData)
{
    size_t fileSize;
    char* pFileData;
    dtk_result result = dtk_open_and_read_file_with_extra_data(filePath, &fileSize, (void**)&pFileData, 1);
    if (result != DTK_SUCCESS) {
        return result;
    }

    pFileData[fileSize] = '\0';

    if (pFileSizeOut) {
        *pFileSizeOut = fileSize;
    }

    if (ppFileData) {
        *ppFileData = pFileData;
    } else {
        dtk_free(pFileData);
    }

    return DTK_SUCCESS;
}

dtk_result dtk_open_and_write_file(const char* filePath, const void* pData, size_t dataSize)
{
    if (filePath == NULL) {
        return DTK_INVALID_ARGS;
    }

    // TODO: Use 64-bit versions of the FILE APIs.

    FILE* pFile;
    dtk_result result = dtk_fopen(filePath, "wb", &pFile);
    if (result != DTK_SUCCESS) {
        return DTK_FAILED_TO_OPEN_FILE;
    }

    if (pData != NULL && dataSize > 0) {
        if (fwrite(pData, 1, dataSize, pFile) != dataSize) {
            fclose(pFile);
            return DTK_FAILED_TO_WRITE_FILE;
        }
    }

    fclose(pFile);
    return DTK_SUCCESS;
}

dtk_result dtk_open_and_write_text_file(const char* filePath, const char* text)
{
    if (text == NULL) {
        text = "";
    }

    return dtk_open_and_write_file(filePath, text, strlen(text));
}

dtk_bool32 dtk_is_directory(const char* path)
{
    if (dtk_string_is_null_or_empty(path)) {
        return DTK_FALSE;
    }

#ifdef DTK_WIN32
    return dtk_is_directory__win32(path);
#endif
#ifdef DTK_POSIX
    return dtk_is_directory__posix(path);
#endif
}

dtk_bool32 dtk_file_exists(const char* filePath)
{
    if (filePath == NULL) {
        return DTK_FALSE;
    }

#ifdef DTK_WIN32
    return dtk_file_exists__win32(filePath);
#endif
#ifdef DTK_POSIX
    return dtk_file_exists__posix(filePath);
#endif
}


dtk_result dtk_move_file(const char* oldPath, const char* newPath)
{
    if (oldPath == NULL || newPath == NULL) {
        return DTK_INVALID_ARGS;
    }

#if _WIN32
    if (MoveFileExA(oldPath, newPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH) == 0) {
        return dtk_win32_error_to_result(GetLastError());
    }

    return DTK_SUCCESS;
#else
    if (rename(oldPath, newPath) != 0) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
#endif
}

dtk_result dtk_copy_file(const char* srcPath, const char* dstPath, dtk_bool32 failIfExists)
{
    if (srcPath == NULL || dstPath == NULL) {
        return DTK_INVALID_ARGS;
    }

#if _WIN32
    if (CopyFileA(srcPath, dstPath, failIfExists) == 0) {
        return dtk_win32_error_to_result(GetLastError());
    }

    return DTK_SUCCESS;
#else
    int fdSrc = open(srcPath, O_RDONLY, 0666);
    if (fdSrc == -1) {
        return dtk_errno_to_result(errno);
    }

    int fdDst = open(dstPath, O_WRONLY | O_TRUNC | O_CREAT | ((failIfExists) ? O_EXCL : 0), 0666);
    if (fdDst == -1) {
        close(fdSrc);
        return dtk_errno_to_result(errno);
    }

    dtk_bool32 result = DTK_TRUE;
    struct stat info;
    if (fstat(fdSrc, &info) == 0) {
        char buffer[BUFSIZ];
        int bytesRead;
        while ((bytesRead = read(fdSrc, buffer, sizeof(buffer))) > 0) {
            if (write(fdDst, buffer, bytesRead) != bytesRead) {
                result = DTK_FALSE;
                break;
            }
        }
    } else {
        result = DTK_FALSE;
    }

    close(fdDst);
    close(fdSrc);

    // Permissions.
    chmod(dstPath, info.st_mode & 07777);

    if (result == DTK_FALSE) {
        return dtk_errno_to_result(errno);
    }

    return DTK_SUCCESS;
#endif
}

dtk_bool32 dtk_is_file_read_only(const char* filePath)
{
    if (filePath == NULL || filePath[0] == '\0') {
        return DTK_FALSE;
    }

#if _WIN32
    DWORD attributes = GetFileAttributesA(filePath);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_READONLY) != 0;
#else
    return access(filePath, W_OK) == -1;
#endif
}

dtk_uint64 dtk_get_file_modified_time(const char* filePath)
{
    if (filePath == NULL || filePath[0] == '\0') {
        return 0;
    }

#if _WIN32
    HANDLE hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }

    FILETIME fileTime;
    BOOL wasSuccessful = GetFileTime(hFile, NULL, NULL, &fileTime);
    CloseHandle(hFile);

    if (!wasSuccessful) {
        return 0;
    }

    ULARGE_INTEGER result;
    result.HighPart = fileTime.dwHighDateTime;
    result.LowPart = fileTime.dwLowDateTime;
    return result.QuadPart;
#else
    struct stat info;
    if (stat(filePath, &info) != 0) {
        return 0;
    }

    return info.st_mtime;
#endif
}


dtk_result dtk_delete_file(const char* filePath)
{
    if (filePath == NULL) {
        return DTK_INVALID_ARGS;
    }

#ifdef DTK_WIN32
    if (DeleteFileA(filePath) == 0) {
        return dtk_win32_error_to_result(GetLastError());
    }

    return DTK_SUCCESS;
#endif
#ifdef DTK_POSIX
    if (remove(filePath) != 0) {
        return dtk_errno_to_result(errno);
    }

    return DTK_SUCCESS;
#endif
}

dtk_result dtk_mkdir(const char* directoryPath)
{
    if (directoryPath == NULL) {
        return DTK_INVALID_ARGS;
    }

#ifdef DTK_WIN32
    if (CreateDirectoryA(directoryPath, NULL) == 0) {
        return dtk_win32_error_to_result(GetLastError());
    }

    return DTK_SUCCESS;
#endif
#ifdef DTK_POSIX
    if (mkdir(directoryPath, 0777) != 0) {
        return dtk_errno_to_result(errno);
    }

    return DTK_SUCCESS;
#endif
}

dtk_result dtk_mkdir_recursive(const char* directoryPath)
{
    if (directoryPath == NULL || directoryPath[0] == '\0') {
        return DTK_INVALID_ARGS;
    }

    // All we do is iterate over each segment in the path and check for the directory.
    dtk_string runningPathStr = NULL;

    dtk_path_iterator pathIterator;
    if (dtk_path_first(directoryPath, &pathIterator)) {
        for (;;) {
            if (runningPathStr != NULL) {
                runningPathStr = dtk_append_string(runningPathStr, "/");
            }
            runningPathStr = dtk_append_substring(runningPathStr, pathIterator.path + pathIterator.segment.offset, pathIterator.segment.length);

            if (runningPathStr != NULL && !dtk_path_is_root(runningPathStr) && !dtk_directory_exists(runningPathStr)) {
                dtk_result result = dtk_mkdir(runningPathStr);
                if (result != DTK_SUCCESS) {
                    dtk_free_string(runningPathStr);
                    return result;
                }
            }

            if (!dtk_path_next(&pathIterator)) {
                break;
            }
        }
    }
    
    dtk_free_string(runningPathStr);
    return DTK_SUCCESS;
}

dtk_result dtk_mkdir_for_file(const char* filePath)
{
    // This is done in two passes. We first get the length, dynamically allocate some memory, then do it again.
    size_t directoryLenPlusNullTerminator = dtk_path_base_path(NULL, 0, filePath);
    if (directoryLenPlusNullTerminator == 0) {
        return DTK_ERROR;   // Failed to get the length of the base path.
    }

    char* directoryPath = (char*)dtk_malloc(sizeof(*directoryPath)*directoryLenPlusNullTerminator);
    if (directoryPath == NULL) {
        return DTK_OUT_OF_MEMORY;
    }

    directoryLenPlusNullTerminator = dtk_path_base_path(directoryPath, directoryLenPlusNullTerminator, filePath);
    if (directoryLenPlusNullTerminator == 0) {
        dtk_free(directoryPath);
        return DTK_ERROR;   // Failed to get the base path. Should never happen since the input path is the same.
    }

    dtk_result result = dtk_mkdir_recursive(directoryPath);
    dtk_free(directoryPath);

    return result;
}

dtk_bool32 dtk_iterate_files(const char* directory, dtk_bool32 recursive, dtk_iterate_files_proc proc, void* pUserData)
{
#ifdef _WIN32
    char searchQuery[MAX_PATH];
    dtk_strcpy_s(searchQuery, sizeof(searchQuery), directory);

    unsigned int searchQueryLength = (unsigned int)strlen(searchQuery);
    if (searchQueryLength >= MAX_PATH - 3) {
        return DTK_FALSE;    // Path is too long.
    }

    searchQuery[searchQueryLength + 0] = '\\';
    searchQuery[searchQueryLength + 1] = '*';
    searchQuery[searchQueryLength + 2] = '\0';

    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(searchQuery, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return DTK_FALSE; // Failed to begin search.
    }

    do
    {
        // Skip past "." and ".." directories.
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0) {
            continue;
        }

        char filePath[MAX_PATH];
        dtk_strcpy_s(filePath, sizeof(filePath), directory);
        dtk_strcat_s(filePath, sizeof(filePath), "/");
        dtk_strcat_s(filePath, sizeof(filePath), ffd.cFileName);

        if (!proc(filePath, pUserData)) {
            return DTK_FALSE;
        }

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (recursive) {
                if (!dtk_iterate_files(filePath, recursive, proc, pUserData)) {
                    return DTK_FALSE;
                }
            }
        }

    } while (FindNextFileA(hFind, &ffd));

    FindClose(hFind);
#else
    DIR* dir = opendir(directory);
    if (dir == NULL) {
        return DTK_FALSE;
    }

    struct dirent* info = NULL;
    while ((info = readdir(dir)) != NULL)
    {
        // Skip past "." and ".." directories.
        if (strcmp(info->d_name, ".") == 0 || strcmp(info->d_name, "..") == 0) {
            continue;
        }

        char filePath[4096];
        dtk_strcpy_s(filePath, sizeof(filePath), directory);
        dtk_strcat_s(filePath, sizeof(filePath), "/");
        dtk_strcat_s(filePath, sizeof(filePath), info->d_name);

        struct stat fileinfo;
        if (stat(filePath, &fileinfo) != 0) {
            continue;
        }

        if (!proc(filePath, pUserData)) {
            return DTK_FALSE;
        }

        if (fileinfo.st_mode & S_IFDIR) {
            if (recursive) {
                if (!dtk_iterate_files(filePath, recursive, proc, pUserData)) {
                    return DTK_FALSE;
                }
            }
        }
    }

    closedir(dir);
#endif

    return DTK_TRUE;
}


char* dtk_get_current_directory()
{
#ifdef DTK_WIN32
    return dtk_get_current_directory__win32();
#endif
#ifdef DTK_POSIX
    return dtk_get_current_directory__posix();
#endif
}

dtk_result dtk_set_current_directory(const char* path)
{
#ifdef DTK_WIN32
    return dtk_set_current_directory__win32(path);
#endif
#ifdef DTK_POSIX
    return dtk_set_current_directory__posix(path);
#endif
}

char* dtk_make_absolute_path_from_current_directory(const char* pRelativePath)
{
    if (pRelativePath == NULL) {
        pRelativePath = "";
    }

    char* pCurrentDir = dtk_get_current_directory();
    if (pCurrentDir == NULL) {
        return NULL;
    }

    size_t len = dtk_path_to_absolute(NULL, 0, pRelativePath, pCurrentDir);
    if (len == 0) {
        dtk_free(pCurrentDir);
        return NULL;
    }

    char* pAbsolutePath = (char*)dtk_malloc(len);
    if (pAbsolutePath == NULL) {
        dtk_free(pCurrentDir);
        return NULL;
    }

    dtk_path_to_absolute(pAbsolutePath, len, pRelativePath, pCurrentDir);
    dtk_free(pCurrentDir);

    return pAbsolutePath;
}

char* dtk_make_relative_path_from_current_directory(const char* pAbsolutePath)
{
    if (pAbsolutePath == NULL) {
        pAbsolutePath = "";
    }

    char* pCurrentDir = dtk_get_current_directory();
    if (pCurrentDir == NULL) {
        return NULL;
    }

    size_t len = dtk_path_to_relative(NULL, 0, pAbsolutePath, pCurrentDir);
    if (len == 0) {
        dtk_free(pCurrentDir);
        return NULL;
    }

    char* pRelativePath = (char*)dtk_malloc(len);
    if (pRelativePath == NULL) {
        dtk_free(pCurrentDir);
        return NULL;
    }

    dtk_path_to_relative(pRelativePath, len, pAbsolutePath, pCurrentDir);
    dtk_free(pCurrentDir);

    return pRelativePath;
}

// Copyright (C) 2017 David Reid. See included LICENSE file.

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
#endif


dtk_result dtk_fopen(const char* filePath, const char* openMode, FILE** ppFile)
{
    if (filePath == NULL || openMode == NULL || ppFile == NULL) return DTK_INVALID_ARGS;

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

static dtk_result dtk_open_and_read_file_with_extra_data(const char* filePath, size_t* pFileSizeOut, void** ppFileData, size_t extraBytes)
{
    // Safety.
    if (pFileSizeOut) *pFileSizeOut = 0;
    if (ppFileData) *ppFileData = NULL;

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
    dr_uint64 fileSize = ftell(pFile);
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
    char* pFileData;
    dtk_result result = dtk_open_and_read_file_with_extra_data(filePath, pFileSizeOut, &pFileData, 1);
    if (result != DTK_SUCCESS) {
        return result;
    }

    pFileData[*pFileSizeOut] = '\0';

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
    if (dtk_string_is_null_or_empty(path)) return DTK_FALSE;

#ifdef DTK_WIN32
    return dtk_is_directory__win32(path);
#endif
#ifdef DTK_POSIX
    return dtk_is_directory__posix(path);
#endif
}



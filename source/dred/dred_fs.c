// Copyright (C) 2017 David Reid. See included LICENSE file.

size_t dred_get_config_folder_path(dred_context* pDred, char* pathOut, size_t pathOutSize)
{
    if (pDred != NULL && !pDred->isPortable) {
        char basePath[32768];
        if (dtk_get_config_directory_path(basePath, sizeof(basePath)) == 0) {
            return 0;
        }

        return dtk_path_append(pathOut, pathOutSize, basePath, "dred");
    } else {
        char basePath[32768];
        if (dtk_get_executable_directory_path(basePath, sizeof(basePath)) == 0) {
            return 0;
        }

        return dtk_path_append(pathOut, pathOutSize, basePath, "config");
    }
}

size_t dred_get_config_path(dred_context* pDred, char* pathOut, size_t pathOutSize)
{
    char basePath[32768];
    if (dred_get_config_folder_path(pDred, basePath, sizeof(basePath)) == 0) {
        return 0;
    }

    return dtk_path_append(pathOut, pathOutSize, basePath, ".dred");
}

size_t dred_get_log_folder_path(dred_context* pDred, char* pathOut, size_t pathOutSize)
{
    if (pDred != NULL && !pDred->isPortable) {
        // On Windows we use the same directory that we use for configs. On *nix we'll use "~/.dred"
    #ifdef DTK_WIN32
        return dred_get_config_folder_path(pDred, pathOut, pathOutSize);
    #endif
    #ifdef DTK_POSIX
        const char* homedir = getenv("HOME");
        if (homedir == NULL) {
            homedir = getpwuid(getuid())->pw_dir;
            if (homedir == NULL) {
                return 0;
            }
        }

        dtk_assert(homedir != NULL);
        return dtk_path_append(pathOut, pathOutSize, homedir, ".dred");
    #endif
    } else {
        char basePath[32768];
        if (dtk_get_executable_directory_path(basePath, sizeof(basePath)) == 0) {
            return 0;
        }

        return dtk_path_append(pathOut, pathOutSize, basePath, "logs");
    }
}

size_t dred_get_log_path(dred_context* pDred, char* pathOut, size_t pathOutSize)
{
    char basePath[32768];
    if (dred_get_log_folder_path(pDred, basePath, sizeof(basePath)) == 0) {
        return 0;
    }

    return dtk_path_append(pathOut, pathOutSize, basePath, "dred.log");
}

size_t dred_get_packages_folder_path(char* pathOut, size_t pathOutSize)
{
    char basePath[32768];
    if (dtk_get_executable_directory_path(basePath, sizeof(basePath)) == 0) {
        return 0;
    }

    return dtk_path_append(pathOut, pathOutSize, basePath, "packages");
}



dred_file dred_file_open(const char* filePath, unsigned int openMode)
{
    if (filePath == NULL || openMode == 0) {
        return NULL;
    }

    const char* openModeSTD;
    if (openMode & DRED_FILE_OPEN_MODE_READ) {
        if (openMode & DRED_FILE_OPEN_MODE_WRITE) {
            openModeSTD = "w+b";
        } else {
            openModeSTD = "rb";
        }
    } else {
        if (openMode & DRED_FILE_OPEN_MODE_WRITE) {
            openModeSTD = "wb";
        } else {
            return NULL;    // Neither read nor write mode was specified.
        }
    }
    
    FILE* pFile;
#ifdef _MSC_VER
    if (fopen_s(&pFile, filePath, openModeSTD) != 0) {
        return NULL;
    }
#else
#ifdef DRED_WIN32
    pFile = fopen(filePath, openModeSTD);
    if (pFile == NULL) {
        return NULL;
    }
#else
    pFile = fopen64(filePath, openModeSTD);
    if (pFile == NULL) {
        return NULL;
    }
#endif
#endif

    return pFile;
}

void dred_file_close(dred_file file)
{
    fclose((FILE*)file);
}

dr_bool32 dred_file_read(dred_file file, void* pDataOut, size_t bytesToRead, size_t* pBytesRead)
{
    size_t bytesRead = fread(pDataOut, 1, bytesToRead, (FILE*)file);
    if (pBytesRead) *pBytesRead = bytesRead;

    if (bytesRead == 0 && bytesToRead != 0) {
        return DR_FALSE;
    }

    return DR_TRUE;
}

dr_bool32 dred_file_write(dred_file file, const void* pData, size_t bytesToWrite, size_t* pBytesWritten)
{
    size_t bytesWritten = fwrite(pData, 1, bytesToWrite, (FILE*)file);
    if (pBytesWritten) *pBytesWritten = bytesWritten;

    if (bytesWritten == 0 && bytesToWrite != 0) {
        return DR_FALSE;
    }

    return DR_TRUE;
}

dr_bool32 dred_file_seek(dred_file file, int64_t bytesToSeek, dred_seek_origin origin)
{
    int originSTD = SEEK_SET;
    if (origin == dred_seek_origin_current) {
        originSTD = SEEK_CUR;
    } else if (origin == dred_seek_origin_end) {
        originSTD = SEEK_END;
    }

#ifdef DRED_WIN32
    if (_fseeki64((FILE*)file, bytesToSeek, originSTD) == -1) {
        return DR_FALSE;
    }
#else
    if (fseeko((FILE*)file, bytesToSeek, originSTD) == -1) {
        return DR_FALSE;
    }
#endif

    return DR_TRUE;
}

uint64_t dred_file_tell(dred_file file)
{
    int64_t result;
#ifdef DRED_WIN32
    result = _ftelli64((FILE*)file);
#else
    result = ftello((FILE*)file);
#endif

    if (result == -1) {
        return 0;
    }

    return (uint64_t)result;
}

void dred_file_flush(dred_file file)
{
    fflush((FILE*)file);
}



//// High Level Helpers ////

dr_bool32 dred_file_write_string(dred_file file, const char* str)
{
    return dred_file_write(file, str, (unsigned int)strlen(str), NULL);
}

dr_bool32 dred_file_write_line(dred_file file, const char* str)
{
    return dred_file_write_string(file, str) && dred_file_write_string(file, "\n");
}


dr_bool32 dred_to_absolute_path(const char* relativePath, char* absolutePathOut, size_t absolutePathOutSize)
{
    if (dtk_path_is_absolute(relativePath)) {
        return strcpy_s(absolutePathOut, absolutePathOutSize, relativePath) == 0;
    }

    char* pCurrentDir = dtk_get_current_directory();
    if (pCurrentDir == NULL) {
        return DR_FALSE;
    }

    if (dtk_path_append(absolutePathOut, absolutePathOutSize, pCurrentDir, relativePath) == 0) {
        dtk_free(pCurrentDir);
        return DR_FALSE;
    }

    dtk_free(pCurrentDir);
    return DR_TRUE;
}
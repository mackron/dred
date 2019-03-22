// Copyright (C) 2019 David Reid. See included LICENSE file.

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

dtk_bool32 dred_to_absolute_path(const char* relativePath, char* absolutePathOut, size_t absolutePathOutSize)
{
    if (dtk_path_is_absolute(relativePath)) {
        return strcpy_s(absolutePathOut, absolutePathOutSize, relativePath) == 0;
    }

    char* pCurrentDir = dtk_get_current_directory();
    if (pCurrentDir == NULL) {
        return DTK_FALSE;
    }

    if (dtk_path_append(absolutePathOut, absolutePathOutSize, pCurrentDir, relativePath) == 0) {
        dtk_free(pCurrentDir);
        return DTK_FALSE;
    }

    dtk_free(pCurrentDir);
    return DTK_TRUE;
}
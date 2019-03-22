// Copyright (C) 2019 David Reid. See included LICENSE file.


typedef struct
{
    char name[256];
    char libraryName[256];
} dred_package_info;

void dred_load_package_info__on_pair(void* pUserData, const char* key, const char* value)
{
    if (pUserData == NULL || key == NULL || value == NULL) return;

    dred_package_info* pInfo = (dred_package_info*)pUserData;
    assert(pInfo != NULL);

    if (strcmp(key, "Name") == 0) {
        char valueWithoutQuotes[256];
        if (dtk_next_token(value, valueWithoutQuotes, sizeof(valueWithoutQuotes)) == NULL) {
            return;
        }

        strcpy_s(pInfo->name, sizeof(pInfo->name), valueWithoutQuotes);
        return;
    }

    if (strcmp(key, "SO") == 0) {
        char valueWithoutQuotes[256];
        if (dtk_next_token(value, valueWithoutQuotes, sizeof(valueWithoutQuotes)) == NULL) {
            return;
        }

        strcpy_s(pInfo->libraryName, sizeof(pInfo->libraryName), valueWithoutQuotes);
        return;
    }

    // If we get here it means it's an unknown property. Don't throw an error here - the package may want to use custom
    // properties for it's own internal configuration.
}

dtk_bool32 dred_load_package_info(const char* dredpackagePath, dred_package_info* pInfo)
{
    assert(dredpackagePath != NULL);
    assert(pInfo != NULL);

    memset(pInfo, 0, sizeof(*pInfo));

    if (!dtk_parse_key_value_pairs_from_file(dredpackagePath, dred_load_package_info__on_pair, NULL, pInfo)) {
        return DTK_FALSE;
    }

    if (pInfo->name[0] == '\0' || pInfo->libraryName[0] == '\0') {
        return DTK_FALSE;
    }

    return DTK_TRUE;
}

dtk_bool32 dred_construct_library_path(char* pathOut, size_t pathOutSize, const char* basePackageFolderPath, dred_package_info* pInfo)
{
    assert(pathOut != NULL);
    assert(pathOutSize > 0);
    assert(basePackageFolderPath != NULL);
    assert(pInfo != NULL);

    const char* platformSubPath = "bin/"
#ifdef DRED_WIN32
        "win32";
#else
        "unix";
#endif

    const char* libraryExt = "_"
#ifdef DRED_32BIT
        "x86."
#endif
#ifdef DRED_64BIT
        "x64."
#endif
#ifdef DRED_WIN32
        "dll";
#elif __linux__
        "so";
#elif __APPLE__
        "dylib";
#endif

    // Doubtful this will show up in profiling, but this can definitely be optimized.
    return
        dtk_path_append(pathOut, pathOutSize, basePackageFolderPath, platformSubPath) > 0 &&
        strcat_s(pathOut, pathOutSize, "/") == 0 &&
        strcat_s(pathOut, pathOutSize, pInfo->libraryName) == 0 &&
        strcat_s(pathOut, pathOutSize, libraryExt) == 0;
}

dred_package* dred_package_library_load_package(dred_package_library* pLibrary, const char* packageFolderPath)
{
    if (pLibrary == NULL || packageFolderPath == NULL) return DTK_FALSE;

    // Look for a .dredpackage file. If it doesn't exist, just skip it. Inside the .dredpackage file is information
    // about the package that we'll need in order to load it; in particular the name of the DLL/SO.
    char dredpackagePath[DRED_MAX_PATH];
    if (dtk_path_append(dredpackagePath, sizeof(dredpackagePath), packageFolderPath, ".dredpackage")) {
        dred_package_info info;
        if (dred_load_package_info(dredpackagePath, &info)) {
            char libraryPath[DRED_MAX_PATH];
            if (dred_construct_library_path(libraryPath, sizeof(libraryPath), packageFolderPath, &info)) {
                dtk_handle dl = dtk_dlopen(libraryPath);
                if (dl != NULL) {
                    dred_package_create_proc package_create = (dred_package_create_proc)dtk_dlsym(dl, "dred_package_create");
                    if (package_create != NULL) {
                        dred_package* pPackage = package_create();
                        if (pPackage != NULL) {
                            pPackage->_dl = dl;
                            return pPackage;
                        } else {
                            dtk_dlclose(libraryPath);  // Failed to create the package instance.
                        }
                    } else {
                        dtk_dlclose(libraryPath);  // Could not find the "dred_package_create()" function.
                    }
                }
            }
        }
    }

    return NULL;
}

void dred_package_library_unload_package(dred_package_library* pLibrary, dred_package* pPackage)
{
    if (pLibrary == NULL || pPackage == NULL) return;

    dtk_handle dl = pPackage->_dl;
    if (dl != NULL) {
        dred_package_delete_proc package_delete = (dred_package_delete_proc)dtk_dlsym(dl, "dred_package_delete");
        if (package_delete) {
            package_delete(pPackage);
        }

        dtk_dlclose(dl);
    }
}


dtk_bool32 dred_package_library_package_iterator_cb(const char* filePath, void* pUserData)
{
    dred_package_library* pLibrary = (dred_package_library*)pUserData;
    assert(pLibrary != NULL);

    dtk_bool32 isDirectory = dtk_is_directory(filePath);
    if (isDirectory) {
        dred_package* pPackage = dred_package_library_load_package(pLibrary, filePath);
        if (pPackage != NULL) {
            stb_sb_push(pLibrary->pPackages, pPackage);
        }
    }

    return DTK_TRUE;
}

dtk_bool32 dred_package_library_init(dred_package_library* pLibrary)
{
    if (pLibrary == NULL) return DTK_FALSE;

    memset(pLibrary, 0, sizeof(*pLibrary));

    // Packages will be relative to the executable in the "packages" directory. Each package will be in it's own
    // directory which will include a .dredpackage file with information about the package.
    char basePackageDir[DRED_MAX_PATH];
    if (dred_get_packages_folder_path(basePackageDir, sizeof(basePackageDir)) == 0) {
        return DTK_FALSE;
    }

    dtk_iterate_files(basePackageDir, DTK_FALSE, dred_package_library_package_iterator_cb, pLibrary);

    return DTK_TRUE;
}

void dred_package_library_uninit(dred_package_library* pLibrary)
{
    if (pLibrary == NULL) return;

    for (int iPackage = 0; iPackage < stb_sb_count(pLibrary->pPackages); ++iPackage) {
        dred_package_library_unload_package(pLibrary, pLibrary->pPackages[iPackage]);
    }

    stb_sb_free(pLibrary->pPackages);
}

size_t dred_package_library_get_package_count(dred_package_library* pLibrary)
{
    if (pLibrary == NULL) return 0;
    return (size_t)stb_sb_count(pLibrary->pPackages);
}

dred_package* dred_package_library_get_package(dred_package_library* pLibrary, size_t index)
{
    if (pLibrary == NULL || index >= dred_package_library_get_package_count(pLibrary)) return NULL;
    return pLibrary->pPackages[index];
}

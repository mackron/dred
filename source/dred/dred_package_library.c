// Copyright (C) 2016 David Reid. See included LICENSE file.


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
        if (dr_next_token(value, valueWithoutQuotes, sizeof(valueWithoutQuotes)) == NULL) {
            return;
        }

        strcpy_s(pInfo->name, sizeof(pInfo->name), valueWithoutQuotes);
        return;
    }

    if (strcmp(key, "SO") == 0) {
        char valueWithoutQuotes[256];
        if (dr_next_token(value, valueWithoutQuotes, sizeof(valueWithoutQuotes)) == NULL) {
            return;
        }

        strcpy_s(pInfo->libraryName, sizeof(pInfo->libraryName), valueWithoutQuotes);
        return;
    }

    // If we get here it means it's an unknown property. Don't throw an error here - the package may want to use custom
    // properties for it's own internal configuration.
}

dr_bool32 dred_load_package_info(const char* dredpackagePath, dred_package_info* pInfo)
{
    assert(dredpackagePath != NULL);
    assert(pInfo != NULL);

    memset(pInfo, 0, sizeof(*pInfo));

    if (!dr_parse_key_value_pairs_from_file(dredpackagePath, dred_load_package_info__on_pair, NULL, pInfo)) {
        return DR_FALSE;
    }

    if (pInfo->name[0] == '\0' || pInfo->libraryName[0] == '\0') {
        return DR_FALSE;
    }

    return DR_TRUE;
}

dr_bool32 dred_construct_library_path(char* pathOut, size_t pathOutSize, const char* basePackageFolderPath, dred_package_info* pInfo)
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
        drpath_copy_and_append(pathOut, pathOutSize, basePackageFolderPath, platformSubPath) && 
        drpath_append(pathOut, pathOutSize, pInfo->libraryName) &&
        strcat_s(pathOut, pathOutSize, libraryExt) == 0;
}

dred_package* dred_package_library_load_package(dred_package_library* pLibrary, const char* packageFolderPath)
{
    if (pLibrary == NULL || packageFolderPath == NULL) return DR_FALSE;

    // Look for a .dredpackage file. If it doesn't exist, just skip it. Inside the .dredpackage file is information
    // about the package that we'll need in order to load it; in particular the name of the DLL/SO.
    char dredpackagePath[DRED_MAX_PATH];
    if (drpath_copy_and_append(dredpackagePath, sizeof(dredpackagePath), packageFolderPath, ".dredpackage")) {
        dred_package_info info;
        if (dred_load_package_info(dredpackagePath, &info)) {
            char libraryPath[DRED_MAX_PATH];
            if (dred_construct_library_path(libraryPath, sizeof(libraryPath), packageFolderPath, &info)) {
                dred_dl dl = dred_dlopen(libraryPath);
                if (dl != NULL) {
                    dred_package_create_proc package_create = (dred_package_create_proc)dred_dlsym(dl, "dred_package_create");
                    if (package_create != NULL) {
                        dred_package* pPackage = package_create();
                        if (pPackage != NULL) {
                            pPackage->_dl = dl;
                            return pPackage;
                        } else {
                            dred_dlclose(libraryPath);  // Failed to create the package instance.
                        }
                    } else {
                        dred_dlclose(libraryPath);  // Could not find the "dred_package_create()" function.
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

    dred_dl dl = pPackage->_dl;
    if (dl != NULL) {
        dred_package_delete_proc package_delete = (dred_package_delete_proc)dred_dlsym(dl, "dred_package_delete");
        if (package_delete) {
            package_delete(pPackage);
        }

        dred_dlclose(dl);
    }
}


dr_bool32 dred_package_library_package_iterator_cb(const char* filePath, void* pUserData)
{
    dred_package_library* pLibrary = (dred_package_library*)pUserData;
    assert(pLibrary != NULL);

    dr_bool32 isDirectory = dr_is_directory(filePath);
    if (isDirectory) {
        dred_package* pPackage = dred_package_library_load_package(pLibrary, filePath);
        if (pPackage != NULL) {
            stb_sb_push(pLibrary->pPackages, pPackage);
        }
    }

    return DR_TRUE;
}

dr_bool32 dred_package_library_init(dred_package_library* pLibrary)
{
    if (pLibrary == NULL) return DR_FALSE;

    memset(pLibrary, 0, sizeof(*pLibrary));

    // Packages will be relative to the executable in the "packages" directory. Each package will be in it's own
    // directory which will include a .dredpackage file with information about the package.
    char basePackageDir[DRED_MAX_PATH];
    if (!dred_get_packages_folder_path(basePackageDir, sizeof(basePackageDir))) {
        return DR_FALSE;
    }

    dr_iterate_files(basePackageDir, DR_FALSE, dred_package_library_package_iterator_cb, pLibrary);

    return DR_TRUE;
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

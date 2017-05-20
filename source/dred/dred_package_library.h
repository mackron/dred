// Copyright (C) 2017 David Reid. See included LICENSE file.

struct dred_package_library
{
    dred_package** pPackages;   // <-- This is an stb_stretchy_buffer object.
};

// Initializes the package library by loading every active package.
dr_bool32 dred_package_library_init(dred_package_library* pLibrary);

// Uninitializes the package library.
void dred_package_library_uninit(dred_package_library* pLibrary);

// Retrieves the number of loaded packages.
size_t dred_package_library_get_package_count(dred_package_library* pLibrary);

// Retrieves a pointer to the package at the given index.
dred_package* dred_package_library_get_package(dred_package_library* pLibrary, size_t index);
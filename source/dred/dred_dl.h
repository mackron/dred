// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef void* dred_dl;
typedef void (* dred_proc)();

// Opens a dynamic library.
dred_dl dred_dlopen(const char* filename);

// Closes a dynamic library.
void dred_dlclose(dred_dl handle);

// Retrieves a pointer to the function with the given name. Returns null if the function does not
// exist in the library.
dred_proc dred_dlsym(dred_dl handle, const char* symbol);
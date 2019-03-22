// Copyright (C) 2019 David Reid. See included LICENSE file.

// Opens a dynamic library.
dtk_handle dtk_dlopen(const char* filename);

// Closes a dynamic library.
void dtk_dlclose(dtk_handle handle);

// Retrieves a pointer to the function with the given name. Returns null if the function does not
// exist in the library.
dtk_proc dtk_dlsym(dtk_handle handle, const char* symbol);
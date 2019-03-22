// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_handle dtk_dlopen(const char* filename)
{
#ifdef _WIN32
    return (dtk_handle)LoadLibraryA(filename);
#else
    return (dtk_handle)dlopen(filename, RTLD_NOW);
#endif
}

void dtk_dlclose(dtk_handle handle)
{
#ifdef _WIN32
    CloseHandle((HANDLE)handle);
#else
    dlclose((void*)handle);
#endif
}

dtk_proc dtk_dlsym(dtk_handle handle, const char* symbol)
{
#ifdef _WIN32
    return (dtk_proc)GetProcAddress((HMODULE)handle, symbol);
#else
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif
    return (dtk_proc)dlsym((void*)handle, symbol);
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
    #pragma GCC diagnostic pop
#endif
#endif
}
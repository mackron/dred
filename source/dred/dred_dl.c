// Copyright (C) 2018 David Reid. See included LICENSE file.

dred_dl dred_dlopen(const char* filename)
{
#ifdef _WIN32
    return (dred_dl)LoadLibraryA(filename);
#else
    return (dred_dl)dlopen(filename, RTLD_NOW);
#endif
}

void dred_dlclose(dred_dl handle)
{
#ifdef _WIN32
    CloseHandle((HANDLE)handle);
#else
    dlclose((void*)handle);
#endif
}

dred_proc dred_dlsym(dred_dl handle, const char* symbol)
{
#ifdef _WIN32
    return (dred_proc)GetProcAddress((HMODULE)handle, symbol);
#else
    return (dred_proc)dlsym((void*)handle, symbol);
#endif
}
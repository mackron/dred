// Copyright (C) 2019 David Reid. See included LICENSE file.

// dred_get_config_folder_path()
size_t dred_get_config_folder_path(dred_context* pDred, char* pathOut, size_t pathOutSize);

// dred_get_config_path()
size_t dred_get_config_path(dred_context* pDred, char* pathOut, size_t pathOutSize);

// dred_get_log_folder_path()
size_t dred_get_log_folder_path(dred_context* pDred, char* pathOut, size_t pathOutSize);

// dred_get_log_path()
size_t dred_get_log_path(dred_context* pDred, char* pathOut, size_t pathOutSize);

// dred_get_packages_folder_path()
size_t dred_get_packages_folder_path(char* pathOut, size_t pathOutSize);

// Converts a relative path to absolute.
dtk_bool32 dred_to_absolute_path(const char* relativePath, char* absolutePathOut, size_t absolutePathOutSize);

// Copyright (C) 2016 David Reid. See included LICENSE file.

// dred_get_config_folder_path()
dr_bool32 dred_get_config_folder_path(dred_context* pDred, char* pathOut, size_t pathOutSize);

// dred_get_config_path()
dr_bool32 dred_get_config_path(dred_context* pDred, char* pathOut, size_t pathOutSize);

// dred_get_log_folder_path()
dr_bool32 dred_get_log_folder_path(dred_context* pDred, char* pathOut, size_t pathOutSize);

// dred_get_log_path()
dr_bool32 dred_get_log_path(dred_context* pDred, char* pathOut, size_t pathOutSize);

// dred_get_packages_folder_path()
dr_bool32 dred_get_packages_folder_path(char* pathOut, size_t pathOutSize);


//// Basic File IO Wrapper ////
typedef void* dred_file;

#define DRED_FILE_OPEN_MODE_READ    1
#define DRED_FILE_OPEN_MODE_WRITE   2

typedef enum
{
    dred_seek_origin_start,
    dred_seek_origin_current,
    dred_seek_origin_end,
} dred_seek_origin;

// dred_file_open()
dred_file dred_file_open(const char* filePath, unsigned int openMode);

// dred_file_close()
void dred_file_close(dred_file file);

// dred_file_read()
dr_bool32 dred_file_read(dred_file file, void* pDataOut, size_t bytesToRead, size_t* pBytesRead);

// dred_file_write()
dr_bool32 dred_file_write(dred_file file, const void* pData, size_t bytesToWrite, size_t* pBytesWritten);

// dred_file_seek()
dr_bool32 dred_file_seek(dred_file file, int64_t bytesToSeek, dred_seek_origin origin);

// dred_file_tell()
uint64_t dred_file_tell(dred_file file);

// dred_file_flush()
void dred_file_flush(dred_file file);



//// High Level Helpers ////

// dred_file_write_string()
dr_bool32 dred_file_write_string(dred_file file, const char* str);

// dred_file_write_line()
dr_bool32 dred_file_write_line(dred_file file, const char* str);


// Converts a relative path to absolute.
dr_bool32 dred_to_absolute_path(const char* relativePath, char* absolutePathOut, size_t absolutePathOutSize);

// Copyright (C) 2018 David Reid. See included LICENSE file.

typedef enum
{
	dtk_ipc_handle_type_pipe,
	dtk_ipc_handle_type_shared_memory	
} dtk_ipc_handle_type;
  
struct dtk_ipc_handle_impl
{
	dtk_ipc_handle_type type;
	
	union
	{
		struct
		{
#ifdef DTK_WIN32
			HANDLE win32Handle;
#endif
#ifdef DTK_POSIX
			int fd;
#endif
		} pipe;
	
		struct
		{
#ifdef DTK_WIN32
			HANDLE win32Handle;
#endif
#ifdef DTK_POSIX
			int fd;
            size_t mappedDataSize;
#endif
			void* pMappedData;
		} shared_memory;
	} data;

#ifdef DTK_POSIX
    // For unix builds we often need to allocate a variable amount of data for storing the names of named objects.
    char name[1];
#endif
};

#ifdef DTK_WIN32
dtk_result dtk_ipc_alloc_handle__win32(dtk_ipc_handle_type type, dtk_ipc_handle* pHandle)
{
    if (pHandle == NULL) return DTK_INVALID_ARGS;

    dtk_ipc_handle handle = (dtk_ipc_handle)dtk_calloc(1, sizeof(*handle));
    if (handle == NULL) {
        return DTK_OUT_OF_MEMORY;
    }

    handle->type = type;

    *pHandle = handle;
    return DTK_SUCCESS;
}

void dtk_ipc_free_handle__win32(dtk_ipc_handle handle)
{
    dtk_free(handle);
}
#endif

#ifdef DTK_POSIX
dtk_result dtk_ipc_alloc_handle__unix(dtk_ipc_handle_type type, size_t extraDataSize, dtk_ipc_handle* pHandle)
{
    if (pHandle == NULL) return DTK_INVALID_ARGS;

    dtk_ipc_handle handle = (dtk_ipc_handle)dtk_calloc(1, sizeof(*handle) + extraDataSize);
    if (handle == NULL) {
        return DTK_OUT_OF_MEMORY;
    }

    handle->type = type;

    *pHandle = handle;
    return DTK_SUCCESS;
}

void dtk_ipc_free_handle__unix(dtk_ipc_handle handle)
{
    dtk_free(handle);
}
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Win32 Implementation
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32

#define DTK_WIN32_PIPE_NAME_HEAD            "\\\\.\\pipe\\"
#define DTK_WIN32_PIPE_BUFFER_SIZE          512
#define DTK_WIN32_HANDLE_TO_PIPE(handle)    ((dtk_pipe)handle)
#define DTK_IPC_PIPE_TO_WIN32_HANDLE(pipe)  ((HANDLE)pipe)

dtk_result dtk_pipe_open_named_server__win32(const char* name, unsigned int options, dtk_pipe* pPipeOut)
{
    char nameWin32[256] = DTK_WIN32_PIPE_NAME_HEAD;
    if (dtk_strcat_s(nameWin32, sizeof(nameWin32), name) != 0) {
        return DTK_NAME_TOO_LONG;
    }

    DWORD dwOpenMode = FILE_FLAG_FIRST_PIPE_INSTANCE;
    if (options & DTK_IPC_READ) {
        if (options & DTK_IPC_WRITE) {
            dwOpenMode |= PIPE_ACCESS_DUPLEX;
        } else {
            dwOpenMode |= PIPE_ACCESS_INBOUND;
        }
    } else {
        if (options & DTK_IPC_WRITE) {
            dwOpenMode |= PIPE_ACCESS_OUTBOUND;
        } else {
            return DTK_INVALID_ARGS;   // Neither read nor write mode was specified.
        }
    }

    HANDLE hPipeWin32 = CreateNamedPipeA(nameWin32, dwOpenMode, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, DTK_WIN32_PIPE_BUFFER_SIZE, DTK_WIN32_PIPE_BUFFER_SIZE, NMPWAIT_USE_DEFAULT_WAIT, NULL);
    if (hPipeWin32 == INVALID_HANDLE_VALUE) {
        return dtk_win32_error_to_result(GetLastError());
    }


    // Wait for a client to connect...
    if (!ConnectNamedPipe(hPipeWin32, NULL)) {
        CloseHandle(hPipeWin32);
        return dtk_win32_error_to_result(GetLastError());
    }


    *pPipeOut = DTK_WIN32_HANDLE_TO_PIPE(hPipeWin32);
    return DTK_SUCCESS;
}

dtk_result dtk_pipe_open_named_client__win32(const char* name, unsigned int options, dtk_pipe* pPipeOut)
{
    char nameWin32[256] = DTK_WIN32_PIPE_NAME_HEAD;
    if (dtk_strcat_s(nameWin32, sizeof(nameWin32), name) != 0) {
        return DTK_NAME_TOO_LONG;
    }

    DWORD dwDesiredAccess = 0;
    if (options & DTK_IPC_READ) {
        dwDesiredAccess |= GENERIC_READ;
    }
    if (options & DTK_IPC_WRITE) {
        dwDesiredAccess |= GENERIC_WRITE;
    }

    if (dwDesiredAccess == 0) {
        return DTK_INVALID_ARGS;   // Neither read nor write mode was specified.
    }

    // The pipe might be busy, so just keep trying.
    for (;;) {
        HANDLE hPipeWin32 = CreateFileA(nameWin32, dwDesiredAccess, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipeWin32 == INVALID_HANDLE_VALUE) {
            DWORD dwError = GetLastError();
            if (dwError != ERROR_PIPE_BUSY) {
                return dtk_win32_error_to_result(dwError);
            }
        } else {
            *pPipeOut = DTK_WIN32_HANDLE_TO_PIPE(hPipeWin32);
            break;
        }
    }

    return DTK_SUCCESS;
}

dtk_result dtk_pipe_open_anonymous__win32(dtk_pipe* pPipeRead, dtk_pipe* pPipeWrite)
{
    HANDLE hPipeReadWin32;
    HANDLE hPipeWriteWin32;
    if (!CreatePipe(&hPipeReadWin32, &hPipeWriteWin32, NULL, DTK_WIN32_PIPE_BUFFER_SIZE)) {
        return dtk_win32_error_to_result(GetLastError());
    }

    *pPipeRead = DTK_WIN32_HANDLE_TO_PIPE(hPipeReadWin32);
    *pPipeWrite = DTK_WIN32_HANDLE_TO_PIPE(hPipeWriteWin32);
    return DTK_SUCCESS;
}

void dtk_pipe_close__win32(dtk_pipe pipe)
{
    CloseHandle(DTK_IPC_PIPE_TO_WIN32_HANDLE(pipe));
}


dtk_result dtk_pipe_connect__win32(dtk_pipe pipe)
{
    if (!ConnectNamedPipe(pipe, NULL)) {
        return dtk_win32_error_to_result(GetLastError());
    }

    return DTK_SUCCESS;
}


dtk_result dtk_pipe_read__win32(dtk_pipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead)
{
    HANDLE hPipe = DTK_IPC_PIPE_TO_WIN32_HANDLE(pipe);

    DWORD dwBytesRead;
    if (!ReadFile(hPipe, pDataOut, (DWORD)bytesToRead, &dwBytesRead, NULL)) {
        return dtk_win32_error_to_result(GetLastError());
    }

    if (pBytesRead) *pBytesRead = dwBytesRead;
    return DTK_SUCCESS;
}

dtk_result dtk_pipe_write__win32(dtk_pipe pipe, const void* pData, size_t bytesToWrite, size_t* pBytesWritten)
{
    HANDLE hPipe = DTK_IPC_PIPE_TO_WIN32_HANDLE(pipe);

    DWORD dwBytesWritten;
    if (!WriteFile(hPipe, pData, (DWORD)bytesToWrite, &dwBytesWritten, NULL)) {
        return dtk_win32_error_to_result(GetLastError());
    }

    if (pBytesWritten) *pBytesWritten = dwBytesWritten;
    return DTK_SUCCESS;
}

size_t dtk_pipe_get_translated_name__win32(const char* name, char* nameOut, size_t nameOutSize)
{
    if (nameOut != NULL && nameOutSize == 0) {
        return 0;
    }

    char nameWin32[256] = DTK_WIN32_PIPE_NAME_HEAD;
    if (dtk_strcat_s(nameWin32, sizeof(nameWin32), name) != 0) {
        return 0;
    }

    if (nameOut != NULL) {
        dtk_strcpy_s(nameOut, nameOutSize, nameWin32);
    }

    return strlen(nameWin32);
}
#endif  // Win32


///////////////////////////////////////////////////////////////////////////////
//
// Unix Implementation
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_POSIX

#ifndef DTK_POSIX_PIPE_NAME_HEAD
#define DTK_POSIX_PIPE_NAME_HEAD  "/tmp/"
#endif

#define DTK_POSIX_SERVER          (1 << 31)
#define DTK_POSIX_CLIENT          (1 << 30)

typedef struct
{
    int fd;
    unsigned int options;
    char name[1];
} dtk_pipe_unix;

static dtk_result dtk_result_from_unix_error(int error)
{
    switch (error)
    {
    default: return DTK_ERROR;
    }
}

static int dtk_ipc_options_to_fd_open_flags(unsigned int options)
{
    int flags = 0;
    if (options & DTK_IPC_READ) {
        if (options & DTK_IPC_WRITE) {
            flags |= O_RDWR;
        } else {
            flags |= O_RDONLY;
        }
    } else {
        if (options & DTK_IPC_WRITE) {
            flags |= O_WRONLY;
        } else {
            return DTK_INVALID_ARGS;   // Neither read nor write mode was specified.
        }
    }

    return flags;
}

dtk_result dtk_pipe_open_named_server__unix(const char* name, unsigned int options, dtk_pipe* pPipeOut)
{
    char nameUnix[512];
    if (dtk_pipe_get_translated_name(name, nameUnix, sizeof(nameUnix)) == 0) {
        return DTK_NAME_TOO_LONG;
    }

    if (mkfifo(nameUnix, 0666) == -1) {
        return dtk_result_from_unix_error(errno);
    }


    dtk_pipe_unix* pPipeUnix = (dtk_pipe_unix*)malloc(sizeof(*pPipeUnix) + strlen(nameUnix)+1);     // +1 for null terminator.
    if (pPipeUnix == NULL) {
        return DTK_ERROR;
    }

    pPipeUnix->options = options | DTK_POSIX_SERVER;
    strcpy(pPipeUnix->name, nameUnix);


    // Wait for a client to connect...
    pPipeUnix->fd = open(pPipeUnix->name, dtk_ipc_options_to_fd_open_flags(pPipeUnix->options));
    if (pPipeUnix->fd == -1) {
        return dtk_result_from_unix_error(errno);
    }


    *pPipeOut = (dtk_pipe)pPipeUnix;
    return DTK_SUCCESS;
}

dtk_result dtk_pipe_open_named_client__unix(const char* name, unsigned int options, dtk_pipe* pPipeOut)
{
    char nameUnix[512];
    if (dtk_pipe_get_translated_name(name, nameUnix, sizeof(nameUnix)) == 0) {
        return DTK_NAME_TOO_LONG;
    }

    dtk_pipe_unix* pPipeUnix = (dtk_pipe_unix*)malloc(sizeof(*pPipeUnix) + strlen(nameUnix)+1);     // +1 for null terminator.
    if (pPipeUnix == NULL) {
        return DTK_ERROR;
    }

    pPipeUnix->options = options | DTK_POSIX_CLIENT;
    strcpy(pPipeUnix->name, nameUnix);

    pPipeUnix->fd = open(nameUnix, dtk_ipc_options_to_fd_open_flags(options));
    if (pPipeUnix->fd == -1) {
        free(pPipeUnix);
        return dtk_result_from_unix_error(errno);
    }


    *pPipeOut = (dtk_pipe)pPipeUnix;
    return DTK_SUCCESS;
}

dtk_result dtk_pipe_open_anonymous__unix(dtk_pipe* pPipeRead, dtk_pipe* pPipeWrite)
{
    dtk_pipe_unix* pPipeReadUnix = (dtk_pipe_unix*)calloc(1, sizeof(*pPipeReadUnix) + 1);
    if (pPipeReadUnix == NULL) {
        return DTK_ERROR;
    }

    dtk_pipe_unix* pPipeWriteUnix = (dtk_pipe_unix*)calloc(1, sizeof(*pPipeWriteUnix) + 1);
    if (pPipeWriteUnix == NULL) {
        free(pPipeReadUnix);
        return DTK_ERROR;
    }

    int pipeFDs[2];
    if (pipe(pipeFDs) == -1) {
        free(pPipeWriteUnix);
        free(pPipeReadUnix);
        return dtk_result_from_unix_error(errno);
    }

    pPipeReadUnix->fd  = pipeFDs[0];
    pPipeWriteUnix->fd = pipeFDs[1];

    *pPipeRead = pPipeReadUnix;
    *pPipeWrite = pPipeWriteUnix;

    return DTK_SUCCESS;
}

void dtk_pipe_close__unix(dtk_pipe pipe)
{
    dtk_pipe_unix* pPipeUnix = (dtk_pipe_unix*)pipe;

    if (pPipeUnix->fd != -1) {
        close(pPipeUnix->fd);
    }

    if (pPipeUnix->options & DTK_POSIX_SERVER) {
        unlink(pPipeUnix->name);
    }
}


dtk_result dtk_pipe_read__unix(dtk_pipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead)
{
    dtk_pipe_unix* pPipeUnix = (dtk_pipe_unix*)pipe;

    ssize_t bytesRead = read(pPipeUnix->fd, pDataOut, bytesToRead);
    if (bytesRead == -1) {
        return dtk_result_from_unix_error(errno);
    }

    if (pBytesRead) *pBytesRead = (size_t)bytesRead;
    return DTK_SUCCESS;
}

dtk_result dtk_pipe_write__unix(dtk_pipe pipe, const void* pData, size_t bytesToWrite, size_t* pBytesWritten)
{
    dtk_pipe_unix* pPipeUnix = (dtk_pipe_unix*)pipe;

    ssize_t bytesWritten = write(pPipeUnix->fd, pData, bytesToWrite);
    if (bytesWritten == -1) {
        return dtk_result_from_unix_error(errno);
    }

    if (pBytesWritten) *pBytesWritten = (size_t)bytesWritten;
    return DTK_SUCCESS;
}

size_t dtk_pipe_get_translated_name__unix(const char* name, char* nameOut, size_t nameOutSize)
{
    if (nameOut != NULL && nameOutSize == 0) {
        return 0;
    }

    char nameUnix[512];
    if (dtk_strcpy_s(nameUnix, sizeof(nameUnix), DTK_POSIX_PIPE_NAME_HEAD) != 0) {
        return 0;
    }
    if (dtk_strcat_s(nameUnix, sizeof(nameUnix), name) != 0) {
        return 0;
    }

    if (dtk_strcpy_s(nameOut, nameOutSize, nameUnix) != 0) {
        return 0;
    }

    return strlen(nameUnix);
}
#endif  // Unix

dtk_result dtk_pipe_open_named_server(const char* name, unsigned int options, dtk_pipe* pPipeOut)
{
    if (name == NULL || options == 0 || pPipeOut == NULL) {
        return DTK_INVALID_ARGS;
    }

    *pPipeOut = NULL;


#ifdef DTK_WIN32
    return dtk_pipe_open_named_server__win32(name, options, pPipeOut);
#endif

#ifdef DTK_POSIX
    return dtk_pipe_open_named_server__unix(name, options, pPipeOut);
#endif
}

dtk_result dtk_pipe_open_named_client(const char* name, unsigned int options, dtk_pipe* pPipeOut)
{
    if (name == NULL || options == 0 || pPipeOut == NULL) {
        return DTK_INVALID_ARGS;
    }

    *pPipeOut = NULL;


#ifdef DTK_WIN32
    return dtk_pipe_open_named_client__win32(name, options, pPipeOut);
#endif

#ifdef DTK_POSIX
    return dtk_pipe_open_named_client__unix(name, options, pPipeOut);
#endif
}

dtk_result dtk_pipe_open_anonymous(dtk_pipe* pPipeRead, dtk_pipe* pPipeWrite)
{
    if (pPipeRead == NULL || pPipeWrite == NULL) {
        return DTK_INVALID_ARGS;
    }

    *pPipeRead = NULL;
    *pPipeWrite = NULL;


#ifdef DTK_WIN32
    return dtk_pipe_open_anonymous__win32(pPipeRead, pPipeWrite);
#endif

#ifdef DTK_POSIX
    return dtk_pipe_open_anonymous__unix(pPipeRead, pPipeWrite);
#endif
}

void dtk_pipe_close(dtk_pipe pipe)
{
    if (pipe == NULL) {
        return;
    }

#ifdef DTK_WIN32
    dtk_pipe_close__win32(pipe);
#endif

#ifdef DTK_POSIX
    dtk_pipe_close__unix(pipe);
#endif
}


dtk_result dtk_pipe_read(dtk_pipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead)
{
    if (pBytesRead) *pBytesRead = 0;

    if (pipe == NULL || pDataOut == NULL) {
        return DTK_INVALID_ARGS;
    }

    // Currently, reading is restricted to 2^31 bytes.
    if (bytesToRead > 0x7FFFFFFF) {
        return DTK_INVALID_ARGS;
    }


#ifdef DTK_WIN32
    return dtk_pipe_read__win32(pipe, pDataOut, bytesToRead, pBytesRead);
#endif

#ifdef DTK_POSIX
    return dtk_pipe_read__unix(pipe, pDataOut, bytesToRead, pBytesRead);
#endif
}

dtk_result dtk_pipe_read_exact(dtk_pipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead)
{
    if (pBytesRead) *pBytesRead = 0;

    while (bytesToRead > 0) {
        size_t bytesRead;
        dtk_result result = dtk_pipe_read(pipe, pDataOut, (bytesToRead <= 0x7FFFFFFF) ? bytesToRead : 0x7FFFFFFF, &bytesRead);
        if (result != DTK_SUCCESS) {
            return result;
        }

        pDataOut = (void*)((char*)pDataOut + bytesRead);

        bytesToRead -= bytesRead;
        if (pBytesRead) *pBytesRead += bytesRead;
    }

    return DTK_SUCCESS;
}


dtk_result dtk_pipe_write(dtk_pipe pipe, const void* pData, size_t bytesToWrite, size_t* pBytesWritten)
{
    if (pBytesWritten) *pBytesWritten = 0;

    if (pipe == NULL || pData == NULL) {
        return DTK_INVALID_ARGS;
    }

    // Currently, writing is restricted to 2^31 bytes.
    if (bytesToWrite > 0x7FFFFFFF) {
        return DTK_INVALID_ARGS;
    }


#ifdef DTK_WIN32
    return dtk_pipe_write__win32(pipe, pData, bytesToWrite, pBytesWritten);
#endif

#ifdef DTK_POSIX
    return dtk_pipe_write__unix(pipe, pData, bytesToWrite, pBytesWritten);
#endif
}


size_t dtk_pipe_get_translated_name(const char* name, char* nameOut, size_t nameOutSize)
{
    if (name == NULL) {
        return 0;
    }

#ifdef DTK_WIN32
    return dtk_pipe_get_translated_name__win32(name, nameOut, nameOutSize);
#endif

#ifdef DTK_POSIX
    return dtk_pipe_get_translated_name__unix(name, nameOut, nameOutSize);
#endif
}


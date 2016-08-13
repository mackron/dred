// Interprocess communication. Public Domain. See "unlicense" statement at the end of this file.
// dr_ipc - v0.1a - 04/08/2016
//
// David Reid - mackron@gmail.com

// USAGE
//
// dr_ipc is a single-file library. To use it, do something like the following in one .c file.
//   #define DR_IPC_IMPLEMENTATION
//   #include "dr_ipc.h"
//
// You can then #include this file in other parts of the program as you would with any other header file.
//
//
// --- Pipes ---
//
// For named pipes, dr_ipc uses the notion of a server and client. The server is the one who initially creates the
// pipe and the client is the one who connects after the fact. To create ther server-side end of a named pipe, do
// something like the following:
//
//   drpipe serverPipe;
//   dripc_result result = drpipe_open_named_server("my_pipe_name", DR_IPC_READ | DR_IPC_WRITE, &serverPipe);
//   if (result != dripc_success) {
//       return -1;
//   }
//
// The above function will block until a client is connected on the other end of the pipe so you will likely want to
// do this on a separate thread. Connecting on the client side is very similar:
//
//   drpipe clientPipe;
//   dripc_result result = drpipe_open_named_client("my_pipe_name", DR_IPC_READ | DR_IPC_WRITE, &clientPipe);
//   if (result != dripc_success) {
//       return -1;
//   }
//
// To read and write data, use drpipe_read() and drpipe_write() respectively. These functions are both blocking. You
// can also use drpipe_read_exact() to continuously read bytes until exactly the number of bytes requested have been
// read.
//
// Internally, for all platforms, the name of the pipe is translated to a platform-specific name. To get this name,
// use the drpipe_get_translated_name() API. On *nix platforms the pipe will be named as "/tmp/{your pipe name}" by
// default. This can be changed by #define-ing DR_IPC_UNIX_PIPE_NAME_HEAD before #include-ing this file.
//
// An anonymous pipe can be created with the drpipe_open_anonymous() API.
// 
//
//
//
// QUICK NOTES
// - Currently, only pipes have been implemented. Sockets will be coming soon.
// - Non-blocking pipes are not supported.

#ifndef dr_ipc_h
#define dr_ipc_h

#include <stddef.h> // For size_t

#ifdef __cplusplus
extern "C" {
#endif

// Each primitive type in dr_ipc is opaque because otherwise it would require exposing system headers like windows.h
// to the public section of this file.
typedef void* drpipe;

#define DR_IPC_READ     0x01
#define DR_IPC_WRITE    0x02

#define DR_IPC_INFINITE 0xFFFFFFFF

typedef enum
{
    dripc_result_success = 0,
    dripc_result_unknown_error,
    dripc_result_invalid_args,
    dripc_result_name_too_long,
    dripc_result_access_denied,
    dripc_result_timeout
} dripc_result;

// Opens a server-side pipe.
//
// This will block until a client is connected. Calling drpipe_close() will _not_ force it to return early.
//
// On *nix platforms the pipe will be named as "/tmp/{name}" by default, but it can be changed by #define-ing DR_IPC_UNIX_PIPE_NAME_HEAD
// before including the implementation of dr_ipc.h.
dripc_result drpipe_open_named_server(const char* name, unsigned int options, drpipe* pPipeOut);

// Opens the client-side end of a named pipe.
//
// If the server-side end of the pipe does not exist, this will fail.
dripc_result drpipe_open_named_client(const char* name, unsigned int options, drpipe* pPipeOut);

// Opens an anonymous pipe.
dripc_result drpipe_open_anonymous(drpipe* pPipeRead, drpipe* pPipeWrite);

// Closes a pipe opened with drpipe_open_named_server(), drpipe_open_named_client() or drpipe_open_anonymous().
void drpipe_close(drpipe pipe);


// Reads data from a pipe.
//
// This is a blocking call, and may not return the exact number of bytes requested. In addition, it is not guaranteed that
// writes from one end of the pipe is atomic.
dripc_result drpipe_read(drpipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead);

// Reads data from a pipe and does not return until either an error occurs or exactly the number of requested bytes have been read.
//
// This is a blocking call.
dripc_result drpipe_read_exact(drpipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead);


// Writes data to a pipe.
//
// This blocks until the data has been written.
dripc_result drpipe_write(drpipe pipe, const void* pData, size_t bytesToWrite, size_t* pBytesWritten);


// Internally, dr_ipc needs to translate the name of a pipe to a platform-specific name. This function returns that internal name.
//
// Returns the length of the name. If nameOut is NULL the return value is the required size, not including the null terminator.
size_t drpipe_get_translated_name(const char* name, char* nameOut, size_t nameOutSize);

#ifdef __cplusplus
}
#endif
#endif  // dr_ipc


///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DR_IPC_IMPLEMENTATION
#include <string.h> // For strlen() and memset()

// Platform Detection
#ifdef _WIN32
#define DR_IPC_WIN32
#include <windows.h>

#define dripc__zero_memory ZeroMemory
#else
#define DR_IPC_UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h> // For malloc/free.

#define dripc__zero_memory(dst, size) memset((dst), 0, (size))
#endif

#ifdef _MSC_VER
#define dripc__strcpy_s strcpy_s
#define dripc__strcat_s strcat_s
#else
int dripc__strcpy_s(char* dst, size_t dstSizeInBytes, const char* src)
{
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    size_t i;
    for (i = 0; i < dstSizeInBytes && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (i < dstSizeInBytes) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return ERANGE;
}

int dripc__strcat_s(char* dst, size_t dstSizeInBytes, const char* src)
{
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    char* dstorig = dst;

    while (dstSizeInBytes > 0 && dst[0] != '\0') {
        dst += 1;
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes == 0) {
        return EINVAL;  // Unterminated.
    }


    while (dstSizeInBytes > 0 && src[0] != '\0') {
        *dst++ = *src++;
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes > 0) {
        dst[0] = '\0';
    } else {
        dstorig[0] = '\0';
        return ERANGE;
    }

    return 0;
}
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Win32 Implementation
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DR_IPC_WIN32

#define DR_IPC_WIN32_PIPE_NAME_HEAD         "\\\\.\\pipe\\"
#define DR_IPC_WIN32_PIPE_BUFFER_SIZE       512
#define DR_IPC_WIN32_HANDLE_TO_PIPE(handle) ((drpipe)handle)
#define DR_IPC_PIPE_TO_WIN32_HANDLE(pipe)   ((HANDLE)pipe)

static dripc_result dripc_result_from_win32_error(DWORD dwError)
{
    switch (dwError)
    {
    case ERROR_INVALID_PARAMETER: return dripc_result_invalid_args;
    case ERROR_ACCESS_DENIED:     return dripc_result_access_denied;
    case ERROR_SEM_TIMEOUT:       return dripc_result_timeout;
    default:                      return dripc_result_unknown_error;
    }
}

dripc_result drpipe_open_named_server__win32(const char* name, unsigned int options, drpipe* pPipeOut)
{
    char nameWin32[256] = DR_IPC_WIN32_PIPE_NAME_HEAD;
    if (dripc__strcat_s(nameWin32, sizeof(nameWin32), name) != 0) {
        return dripc_result_name_too_long;
    }

    DWORD dwOpenMode = FILE_FLAG_FIRST_PIPE_INSTANCE;
    if (options & DR_IPC_READ) {
        if (options & DR_IPC_WRITE) {
            dwOpenMode |= PIPE_ACCESS_DUPLEX;
        } else {
            dwOpenMode |= PIPE_ACCESS_INBOUND;
        }
    } else {
        if (options & DR_IPC_WRITE) {
            dwOpenMode |= PIPE_ACCESS_OUTBOUND;
        } else {
            return dripc_result_invalid_args;   // Neither read nor write mode was specified.
        }
    }

    HANDLE hPipeWin32 = CreateNamedPipeA(nameWin32, dwOpenMode, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, DR_IPC_WIN32_PIPE_BUFFER_SIZE, DR_IPC_WIN32_PIPE_BUFFER_SIZE, NMPWAIT_USE_DEFAULT_WAIT, NULL);
    if (hPipeWin32 == INVALID_HANDLE_VALUE) {
        return dripc_result_from_win32_error(GetLastError());
    }


    // Wait for a client to connect...
    if (!ConnectNamedPipe(hPipeWin32, NULL)) {
        CloseHandle(hPipeWin32);
        return dripc_result_from_win32_error(GetLastError());
    }


    *pPipeOut = DR_IPC_WIN32_HANDLE_TO_PIPE(hPipeWin32);
    return dripc_result_success;
}

dripc_result drpipe_open_named_client__win32(const char* name, unsigned int options, drpipe* pPipeOut)
{
    char nameWin32[256] = DR_IPC_WIN32_PIPE_NAME_HEAD;
    if (dripc__strcat_s(nameWin32, sizeof(nameWin32), name) != 0) {
        return dripc_result_name_too_long;
    }

    DWORD dwDesiredAccess = 0;
    if (options & DR_IPC_READ) {
        dwDesiredAccess |= GENERIC_READ;
    }
    if (options & DR_IPC_WRITE) {
        dwDesiredAccess |= GENERIC_WRITE;
    }

    if (dwDesiredAccess == 0) {
        return dripc_result_invalid_args;   // Neither read nor write mode was specified.
    }

    // The pipe might be busy, so just keep trying.
    for (;;) {
        HANDLE hPipeWin32 = CreateFileA(nameWin32, dwDesiredAccess, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipeWin32 == INVALID_HANDLE_VALUE) {
            DWORD dwError = GetLastError();
            if (dwError != ERROR_PIPE_BUSY) {
                return dripc_result_from_win32_error(dwError);
            }
        } else {
            *pPipeOut = DR_IPC_WIN32_HANDLE_TO_PIPE(hPipeWin32);
            break;
        }
    }

    return dripc_result_success;
}

dripc_result drpipe_open_anonymous__win32(drpipe* pPipeRead, drpipe* pPipeWrite)
{
    HANDLE hPipeReadWin32;
    HANDLE hPipeWriteWin32;
    if (!CreatePipe(&hPipeReadWin32, &hPipeWriteWin32, NULL, DR_IPC_WIN32_PIPE_BUFFER_SIZE)) {
        return dripc_result_from_win32_error(GetLastError());
    }

    *pPipeRead = DR_IPC_WIN32_HANDLE_TO_PIPE(hPipeReadWin32);
    *pPipeWrite = DR_IPC_WIN32_HANDLE_TO_PIPE(hPipeWriteWin32);
    return dripc_result_success;
}

void drpipe_close__win32(drpipe pipe)
{
    CloseHandle(DR_IPC_PIPE_TO_WIN32_HANDLE(pipe));
}


dripc_result drpipe_connect__win32(drpipe pipe)
{
    if (!ConnectNamedPipe(pipe, NULL)) {
        return dripc_result_from_win32_error(GetLastError());
    }

    return dripc_result_success;
}


dripc_result drpipe_read__win32(drpipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead)
{
    HANDLE hPipe = DR_IPC_PIPE_TO_WIN32_HANDLE(pipe);

    DWORD dwBytesRead;
    if (!ReadFile(hPipe, pDataOut, (DWORD)bytesToRead, &dwBytesRead, NULL)) {
        return dripc_result_from_win32_error(GetLastError());
    }

    *pBytesRead = dwBytesRead;
    return dripc_result_success;
}

dripc_result drpipe_write__win32(drpipe pipe, const void* pData, size_t bytesToWrite, size_t* pBytesWritten)
{
    HANDLE hPipe = DR_IPC_PIPE_TO_WIN32_HANDLE(pipe);

    DWORD dwBytesWritten;
    if (!WriteFile(hPipe, pData, (DWORD)bytesToWrite, &dwBytesWritten, NULL)) {
        return dripc_result_from_win32_error(GetLastError());
    }

    *pBytesWritten = dwBytesWritten;
    return dripc_result_success;
}

size_t drpipe_get_translated_name__win32(const char* name, char* nameOut, size_t nameOutSize)
{
    if (nameOut != NULL && nameOutSize == 0) {
        return 0;
    }

    char nameWin32[256] = DR_IPC_WIN32_PIPE_NAME_HEAD;
    if (dripc__strcat_s(nameWin32, sizeof(nameWin32), name) != 0) {
        return dripc_result_name_too_long;
    }

    if (nameOut != NULL) {
        dripc__strcpy_s(nameOut, nameOutSize, nameWin32);
    }

    return strlen(nameWin32);
}
#endif  // Win32


///////////////////////////////////////////////////////////////////////////////
//
// Unix Implementation
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DR_IPC_UNIX

#ifndef DR_IPC_UNIX_PIPE_NAME_HEAD
#define DR_IPC_UNIX_PIPE_NAME_HEAD  "/tmp/"
#endif

#define DR_IPC_UNIX_SERVER          (1 << 31)
#define DR_IPC_UNIX_CLIENT          (1 << 30)

typedef struct
{
    int fd;
    unsigned int options;
    char name[1];
} drpipe_unix;

static dripc_result dripc_result_from_unix_error(int error)
{
    switch (error)
    {
    default: return dripc_result_unknown_error;
    }
}

static int dripc_options_to_fd_open_flags(unsigned int options)
{
    int flags = 0;
    if (options & DR_IPC_READ) {
        if (options & DR_IPC_WRITE) {
            flags |= O_RDWR;
        } else {
            flags |= O_RDONLY;
        }
    } else {
        if (options & DR_IPC_WRITE) {
            flags |= O_WRONLY;
        } else {
            return dripc_result_invalid_args;   // Neither read nor write mode was specified.
        }
    }

    return flags;
}

dripc_result drpipe_open_named_server__unix(const char* name, unsigned int options, drpipe* pPipeOut)
{
    char nameUnix[512];
    if (drpipe_get_translated_name(name, nameUnix, sizeof(nameUnix)) == 0) {
        return dripc_result_name_too_long;
    }

    if (mkfifo(nameUnix, 0666) == -1) {
        return dripc_result_from_unix_error(errno);
    }


    drpipe_unix* pPipeUnix = (drpipe_unix*)malloc(sizeof(*pPipeUnix) + strlen(nameUnix)+1);     // +1 for null terminator.
    if (pPipeUnix == NULL) {
        return dripc_result_unknown_error;
    }

    pPipeUnix->options = options | DR_IPC_UNIX_SERVER;
    strcpy(pPipeUnix->name, nameUnix);


    // Wait for a client to connect...
    pPipeUnix->fd = open(pPipeUnix->name, dripc_options_to_fd_open_flags(pPipeUnix->options));
    if (pPipeUnix->fd == -1) {
        return dripc_result_from_unix_error(errno);
    }


    *pPipeOut = (drpipe)pPipeUnix;
    return dripc_result_success;
}

dripc_result drpipe_open_named_client__unix(const char* name, unsigned int options, drpipe* pPipeOut)
{
    char nameUnix[512];
    if (drpipe_get_translated_name(name, nameUnix, sizeof(nameUnix)) == 0) {
        return dripc_result_name_too_long;
    }

    drpipe_unix* pPipeUnix = (drpipe_unix*)malloc(sizeof(*pPipeUnix) + strlen(nameUnix)+1);     // +1 for null terminator.
    if (pPipeUnix == NULL) {
        return dripc_result_unknown_error;
    }

    pPipeUnix->options = options | DR_IPC_UNIX_CLIENT;
    strcpy(pPipeUnix->name, nameUnix);

    pPipeUnix->fd = open(nameUnix, dripc_options_to_fd_open_flags(options));
    if (pPipeUnix->fd == -1) {
        free(pPipeUnix);
        return dripc_result_from_unix_error(errno);
    }


    *pPipeOut = (drpipe)pPipeUnix;
    return dripc_result_success;
}

dripc_result drpipe_open_anonymous__unix(drpipe* pPipeRead, drpipe* pPipeWrite)
{
    drpipe_unix* pPipeReadUnix = (drpipe_unix*)calloc(1, sizeof(*pPipeReadUnix) + 1);
    if (pPipeReadUnix == NULL) {
        return dripc_result_unknown_error;
    }

    drpipe_unix* pPipeWriteUnix = (drpipe_unix*)calloc(1, sizeof(*pPipeWriteUnix) + 1);
    if (pPipeWriteUnix == NULL) {
        free(pPipeReadUnix);
        return dripc_result_unknown_error;
    }

    int pipeFDs[2];
    if (pipe(pipeFDs) == -1) {
        free(pPipeWriteUnix);
        free(pPipeReadUnix);
        return dripc_result_from_unix_error(errno);
    }

    pPipeReadUnix->fd  = pipeFDs[0];
    pPipeWriteUnix->fd = pipeFDs[1];

    *pPipeRead = pPipeReadUnix;
    *pPipeWrite = pPipeWriteUnix;

    return dripc_result_success;
}

void drpipe_close__unix(drpipe pipe)
{
    drpipe_unix* pPipeUnix = (drpipe_unix*)pipe;

    if (pPipeUnix->fd != -1) {
        close(pPipeUnix->fd);
    }

    if (pPipeUnix->options & DR_IPC_UNIX_SERVER) {
        unlink(pPipeUnix->name);
    }
}


dripc_result drpipe_read__unix(drpipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead)
{
    drpipe_unix* pPipeUnix = (drpipe_unix*)pipe;

    ssize_t bytesRead = read(pPipeUnix->fd, pDataOut, bytesToRead);
    if (bytesRead == -1) {
        return dripc_result_from_unix_error(errno);
    }

    *pBytesRead = (size_t)bytesRead;
    return dripc_result_success;
}

dripc_result drpipe_write__unix(drpipe pipe, const void* pData, size_t bytesToWrite, size_t* pBytesWritten)
{
    drpipe_unix* pPipeUnix = (drpipe_unix*)pipe;

    ssize_t bytesWritten = write(pPipeUnix->fd, pData, bytesToWrite);
    if (bytesWritten == -1) {
        return dripc_result_from_unix_error(errno);
    }

    *pBytesWritten = (size_t)bytesWritten;
    return dripc_result_success;
}

size_t drpipe_get_translated_name__unix(const char* name, char* nameOut, size_t nameOutSize)
{
    if (nameOut != NULL && nameOutSize == 0) {
        return 0;
    }

    char nameUnix[512];
    if (dripc__strcpy_s(nameUnix, sizeof(nameUnix), DR_IPC_UNIX_PIPE_NAME_HEAD) != 0) {
        return 0;
    }
    if (dripc__strcat_s(nameUnix, sizeof(nameUnix), name) != 0) {
        return 0;
    }

    if (dripc__strcpy_s(nameOut, nameOutSize, nameUnix) != 0) {
        return 0;
    }

    return strlen(nameUnix);
}
#endif  // Unix

dripc_result drpipe_open_named_server(const char* name, unsigned int options, drpipe* pPipeOut)
{
    if (name == NULL || options == 0 || pPipeOut == NULL) {
        return dripc_result_invalid_args;
    }

    *pPipeOut = NULL;


#ifdef DR_IPC_WIN32
    return drpipe_open_named_server__win32(name, options, pPipeOut);
#endif

#ifdef DR_IPC_UNIX
    return drpipe_open_named_server__unix(name, options, pPipeOut);
#endif
}

dripc_result drpipe_open_named_client(const char* name, unsigned int options, drpipe* pPipeOut)
{
    if (name == NULL || options == 0 || pPipeOut == NULL) {
        return dripc_result_invalid_args;
    }

    *pPipeOut = NULL;


#ifdef DR_IPC_WIN32
    return drpipe_open_named_client__win32(name, options, pPipeOut);
#endif

#ifdef DR_IPC_UNIX
    return drpipe_open_named_client__unix(name, options, pPipeOut);
#endif
}

dripc_result drpipe_open_anonymous(drpipe* pPipeRead, drpipe* pPipeWrite)
{
    if (pPipeRead == NULL || pPipeWrite == NULL) {
        return dripc_result_invalid_args;
    }

    *pPipeRead = NULL;
    *pPipeWrite = NULL;


#ifdef DR_IPC_WIN32
    return drpipe_open_anonymous__win32(pPipeRead, pPipeWrite);
#endif

#ifdef DR_IPC_UNIX
    return drpipe_open_anonymous__unix(pPipeRead, pPipeWrite);
#endif
}

void drpipe_close(drpipe pipe)
{
    if (pipe == NULL) {
        return;
    }

#ifdef DR_IPC_WIN32
    drpipe_close__win32(pipe);
#endif

#ifdef DR_IPC_UNIX
    drpipe_close__unix(pipe);
#endif
}


dripc_result drpipe_read(drpipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead)
{
    if (pBytesRead) *pBytesRead = 0;

    if (pipe == NULL || pDataOut == NULL) {
        return dripc_result_invalid_args;
    }

    // Currently, reading is restricted to 2^31 bytes.
    if (bytesToRead > 0x7FFFFFFF) {
        return dripc_result_invalid_args;
    }


#ifdef DR_IPC_WIN32
    return drpipe_read__win32(pipe, pDataOut, bytesToRead, pBytesRead);
#endif

#ifdef DR_IPC_UNIX
    return drpipe_read__unix(pipe, pDataOut, bytesToRead, pBytesRead);
#endif
}

dripc_result drpipe_read_exact(drpipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead)
{
    if (pBytesRead) *pBytesRead = 0;

    while (bytesToRead > 0) {
        size_t bytesRead;
        dripc_result result = drpipe_read(pipe, pDataOut, (bytesToRead <= 0x7FFFFFFF) ? bytesToRead : 0x7FFFFFFF, &bytesRead);
        if (result != dripc_result_success) {
            return result;
        }

        pDataOut = (void*)((char*)pDataOut + bytesRead);

        bytesToRead -= bytesRead;
        if (pBytesRead) *pBytesRead += bytesRead;
    }

    return dripc_result_success;
}


dripc_result drpipe_write(drpipe pipe, const void* pData, size_t bytesToWrite, size_t* pBytesWritten)
{
    if (pBytesWritten) *pBytesWritten = 0;

    if (pipe == NULL || pData == NULL) {
        return dripc_result_invalid_args;
    }

    // Currently, writing is restricted to 2^31 bytes.
    if (bytesToWrite > 0x7FFFFFFF) {
        return dripc_result_invalid_args;
    }


#ifdef DR_IPC_WIN32
    return drpipe_write__win32(pipe, pData, bytesToWrite, pBytesWritten);
#endif

#ifdef DR_IPC_UNIX
    return drpipe_write__unix(pipe, pData, bytesToWrite, pBytesWritten);
#endif
}


size_t drpipe_get_translated_name(const char* name, char* nameOut, size_t nameOutSize)
{
    if (name == NULL) {
        return 0;
    }

#ifdef DR_IPC_WIN32
    return drpipe_get_translated_name__win32(name, nameOut, nameOutSize);
#endif

#ifdef DR_IPC_UNIX
    return drpipe_get_translated_name__unix(name, nameOut, nameOutSize);
#endif
}

#endif  // DR_IPC_IMPLEMENTATION


// REVISION HISTORY
//
// v0.1a - 04/08/2016
//   - Compilation fixes for *nix builds.
//
// v0.1 - 31/07/2016
//   - Initial versioned release.


/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

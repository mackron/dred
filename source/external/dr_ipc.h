// Interprocess communication. Public Domain. See "unlicense" statement at the end of this file.
// dr_ipc - v0.3a - 2016-10-11
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
// On Linux you will need to link to the following:
//   -lrt (shared memory)
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
//   if (result != dripc_result_success) {
//       return -1;
//   }
//
// The above function will block until a client is connected on the other end of the pipe so you will likely want to
// do this on a separate thread. Connecting on the client side is very similar:
//
//   drpipe clientPipe;
//   dripc_result result = drpipe_open_named_client("my_pipe_name", DR_IPC_READ | DR_IPC_WRITE, &clientPipe);
//   if (result != dripc_result_success) {
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
// --- Shared Memory ---
//
// Note for Win32: Shared memory requires Administrator rights for it to work. You will want to embed a manifest as
// detailed here: https://msdn.microsoft.com/en-us/library/bb756929.aspx.
//
// Note for Linux: You will need to link to "rt" to use shared memory.
//
// To use shared memory, an application must first allocate it with dripc_create_shared_memory(). Afterwards, other
// applications can connect to this memory with dripc_open_shared_memory().
//
// Secondary applications should disconnect from the memory with dripc_close_shared_memory(). The memory can be freed
// completely with dripc_delete_shared_memory().
//
// The aforementioned APIs return a handle representing the memory. A pointer to the underlying memory can be retrieved
// by mapping the handle with dripc_map_shared_memory(). Unmap the memory with dripc_unmap_shared_memory(). It is an
// error to map memory if it's already mapped.
//
// Example Server Code:
//
//   dripc_handle hMem;
//   dripc_result result = dripc_create_shared_memory("MyGlobalMemory", sharedMemorySizeInBytes, &hMem);
//   if (result != dripc_result_success) {
//       return -1;
//   }
//
//   void* pSharedData;
//   result = dripc_map_shared_memory(hMem, &pSharedData);
//   if (result != dripc_result_success) {
//       return -1;
//   }
//
//   memcpy(pSharedData, pMySharedData, mySharedDataSizeInBytes);   // Use the mapped pointer like any other.
//
//   ...
//
//   dripc_unmap_shared_memory(hMem);
//   dripc_delete_shared_memory(hMem);
//
//
// Example Client Code:
//
//   dripc_handle hMem;
//   dripc_result result = dripc_open_shared_memory("MyGlobalMemory", &hMem);
//   if (result != dripc_result_success) {
//       return -1;
//   }
//
//   void* pSharedData;
//   result = dripc_map_shared_memory(hMem, &pSharedData);
//   if (result != dripc_result_success) {
//       return -1;
//   }
//
//   // Do something with pSharedData...
//
//   ...
//
//   dripc_unmap_shared_memory(hMem);
//   dripc_close_shared_memory(hMem);
//
//
// OPTIONS
// #define these options before including this file.
//
// #define DR_IPC_NO_SHARED_MEMORY
//   Disables shared memory.
//
//
// QUICK NOTES
// - Currently, only pipes have been implemented. Sockets will be coming soon.
// - Non-blocking pipes are not supported.

#ifndef dr_ipc_h
#define dr_ipc_h

#include <stddef.h> // For size_t

#ifndef DR_SIZED_TYPES_DEFINED
#define DR_SIZED_TYPES_DEFINED
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef   signed char    dr_int8;
typedef unsigned char    dr_uint8;
typedef   signed short   dr_int16;
typedef unsigned short   dr_uint16;
typedef   signed int     dr_int32;
typedef unsigned int     dr_uint32;
typedef   signed __int64 dr_int64;
typedef unsigned __int64 dr_uint64;
#else
#include <stdint.h>
typedef int8_t           dr_int8;
typedef uint8_t          dr_uint8;
typedef int16_t          dr_int16;
typedef uint16_t         dr_uint16;
typedef int32_t          dr_int32;
typedef uint32_t         dr_uint32;
typedef int64_t          dr_int64;
typedef uint64_t         dr_uint64;
#endif
typedef int8_t           dr_bool8;
typedef int32_t          dr_bool32;
#define DR_TRUE          1
#define DR_FALSE         0
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Each primitive type in dr_ipc is opaque because otherwise it would require exposing system headers like windows.h
// to the public section of this file.
typedef void* drpipe;
typedef struct dripc_handle_impl dripc_handle_impl;
typedef dripc_handle_impl *dripc_handle;

#define DR_IPC_READ     0x01
#define DR_IPC_WRITE    0x02

#define DR_IPC_INFINITE 0xFFFFFFFF

typedef enum
{
    dripc_result_success = 0,
    dripc_result_unknown_error,
    dripc_result_invalid_args,
    dripc_result_out_of_memory,
    dripc_result_name_too_long,
    dripc_result_access_denied,
    dripc_result_timeout,
    dripc_result_memory_already_mapped
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



///////////////////////////////////////////////////////////////////////////////
//
// Shared Memory
//
///////////////////////////////////////////////////////////////////////////////

// Allocates a block of shared memory and returns a handle representing it.
//
// This should be used by the process that initially allocates the shared memory. Use dripc_open_shared_memory() to
// allow a secondary process to gain access to the memory.
dripc_result dripc_create_shared_memory(const char* name, size_t sizeInBytes, dripc_handle* pMemory);

// Connects the running process to a block of shared memory that was previously allocated by another instance.
dripc_result dripc_open_shared_memory(const char* name, dripc_handle* pMemory);

// Destroys a shared memory object. The opposite of dripc_create_shared_memory().
void dripc_delete_shared_memory(dripc_handle memory);

// Closes the connection to the shared memory. The opposite of dripc_open_shared_memory(). This does not
// deallocate the memory.
void dripc_close_shared_memory(dripc_handle memory);

// Returns a usable pointer to the shared memory.
//
// It is an error to map shared memory if it's already mapped.
dripc_result dripc_map_shared_memory(dripc_handle memory, void** ppDataOut);

// Unmaps the shared memory. 
void dripc_unmap_shared_memory(dripc_handle memory);

// Determines whether or not the given memory object is currently mapped.
dr_bool32 dripc_is_shared_memory_mapped(dripc_handle memory);


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
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h> // For malloc/free.

#define dripc__zero_memory(dst, size) memset((dst), 0, (size))
#endif

typedef enum
{
	dripc_handle_type_pipe,
	dripc_handle_type_shared_memory	
} dripc_handle_type;
  
struct dripc_handle_impl
{
	dripc_handle_type type;
	
	union
	{
		struct
		{
#ifdef DR_IPC_WIN32
			HANDLE win32Handle;
#endif
#ifdef DR_IPC_UNIX
			int fd;
#endif
		} pipe;
	
		struct
		{
#ifdef DR_IPC_WIN32
			HANDLE win32Handle;
#endif
#ifdef DR_IPC_UNIX
			int fd;
            size_t mappedDataSize;
#endif
			void* pMappedData;
		} shared_memory;
	} data;

#ifdef DR_IPC_UNIX
    // For unix builds we often need to allocate a variable amount of data for storing the names of named objects.
    char name[1];
#endif
};

#ifdef DR_IPC_WIN32
dripc_result dripc_alloc_handle__win32(dripc_handle_type type, dripc_handle* pHandle)
{
    if (pHandle == NULL) return dripc_result_invalid_args;

    dripc_handle handle = (dripc_handle)calloc(1, sizeof(*handle));
    if (handle == NULL) {
        return dripc_result_out_of_memory;
    }

    handle->type = type;

    *pHandle = handle;
    return dripc_result_success;
}

void dripc_free_handle__win32(dripc_handle handle)
{
    free(handle);
}
#endif

#ifdef DR_IPC_UNIX
dripc_result dripc_alloc_handle__unix(dripc_handle_type type, size_t extraDataSize, dripc_handle* pHandle)
{
    if (pHandle == NULL) return dripc_result_invalid_args;

    dripc_handle handle = (dripc_handle)calloc(1, sizeof(*handle) + extraDataSize);
    if (handle == NULL) {
        return dripc_result_out_of_memory;
    }

    handle->type = type;

    *pHandle = handle;
    return dripc_result_success;
}

void dripc_free_handle__unix(dripc_handle handle)
{
    free(handle);
}
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

    if (pBytesRead) *pBytesRead = dwBytesRead;
    return dripc_result_success;
}

dripc_result drpipe_write__win32(drpipe pipe, const void* pData, size_t bytesToWrite, size_t* pBytesWritten)
{
    HANDLE hPipe = DR_IPC_PIPE_TO_WIN32_HANDLE(pipe);

    DWORD dwBytesWritten;
    if (!WriteFile(hPipe, pData, (DWORD)bytesToWrite, &dwBytesWritten, NULL)) {
        return dripc_result_from_win32_error(GetLastError());
    }

    if (pBytesWritten) *pBytesWritten = dwBytesWritten;
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

    if (pBytesRead) *pBytesRead = (size_t)bytesRead;
    return dripc_result_success;
}

dripc_result drpipe_write__unix(drpipe pipe, const void* pData, size_t bytesToWrite, size_t* pBytesWritten)
{
    drpipe_unix* pPipeUnix = (drpipe_unix*)pipe;

    ssize_t bytesWritten = write(pPipeUnix->fd, pData, bytesToWrite);
    if (bytesWritten == -1) {
        return dripc_result_from_unix_error(errno);
    }

    if (pBytesWritten) *pBytesWritten = (size_t)bytesWritten;
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





///////////////////////////////////////////////////////////////////////////////
//
// Shared Memory
//
///////////////////////////////////////////////////////////////////////////////
#ifndef DR_IPC_NO_SHARED_MEMORY
#ifdef DR_IPC_WIN32
char* dripc_alloc_shared_memory_name__win32(const char* name)
{
    const char prefix[] = "Global\\";
    size_t nameLen = strlen(prefix) + strlen(name);

    char* nameOut = (char*)malloc(nameLen+1);   // +1 for null terminator.
    if (nameOut == NULL) {
        return NULL;
    }

    dripc__strcpy_s(nameOut, nameLen+1, prefix);
    dripc__strcat_s(nameOut, nameLen+1, name);

    return nameOut;
}

dripc_result dripc_create_shared_memory__win32(const char* name, size_t sizeInBytes, dripc_handle* pMemory)
{
    dripc_handle memory;
    dripc_result result = dripc_alloc_handle__win32(dripc_handle_type_shared_memory, &memory);
    if (result != dripc_result_success) {
        return result;
    }

#if SIZE_MAX == ~0ULL
    DWORD sizeHi = (sizeInBytes >> 32) & 0xFFFFFFFF;
    DWORD sizeLo = sizeInBytes & 0xFFFFFFFF;
#else
    DWORD sizeHi = 0;
    DWORD sizeLo = sizeInBytes & 0xFFFFFFFF;
#endif

    char* nameWin32 = dripc_alloc_shared_memory_name__win32(name);
    if (nameWin32 == NULL) {
        dripc_free_handle__win32(memory);
        return dripc_result_out_of_memory;
    }

    memory->data.shared_memory.win32Handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, sizeHi, sizeLo, nameWin32);
    if (memory->data.shared_memory.win32Handle == NULL) {
        free(nameWin32);
        dripc_free_handle__win32(memory);
        return dripc_result_from_win32_error(GetLastError());
    }

    free(nameWin32);

    *pMemory = memory;
    return dripc_result_success;
}

dripc_result dripc_open_shared_memory__win32(const char* name, dripc_handle* pMemory)
{
    dripc_handle memory;
    dripc_result result = dripc_alloc_handle__win32(dripc_handle_type_shared_memory, &memory);
    if (result != dripc_result_success) {
        return result;
    }

    char* nameWin32 = dripc_alloc_shared_memory_name__win32(name);
    if (nameWin32 == NULL) {
        dripc_free_handle__win32(memory);
        return dripc_result_out_of_memory;
    }

    memory->data.shared_memory.win32Handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, nameWin32);
    if (memory->data.shared_memory.win32Handle == NULL) {
        free(nameWin32);
        dripc_free_handle__win32(memory);
        return dripc_result_from_win32_error(GetLastError());
    }

    free(nameWin32);

    *pMemory = memory;
    return dripc_result_success;
}

void dripc_delete_shared_memory__win32(dripc_handle memory)
{
    CloseHandle(memory->data.shared_memory.win32Handle);
    dripc_free_handle__win32(memory);
}

void dripc_close_shared_memory__win32(dripc_handle memory)
{
    CloseHandle(memory->data.shared_memory.win32Handle);
    dripc_free_handle__win32(memory);
}

dripc_result dripc_map_shared_memory__win32(dripc_handle memory, void** ppDataOut)
{
    memory->data.shared_memory.pMappedData = MapViewOfFile(memory->data.shared_memory.win32Handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (memory->data.shared_memory.pMappedData == NULL) {
        return dripc_result_from_win32_error(GetLastError());
    }

    *ppDataOut = memory->data.shared_memory.pMappedData;
    return dripc_result_success;
}

void dripc_unmap_shared_memory__win32(dripc_handle memory)
{
    UnmapViewOfFile(memory->data.shared_memory.pMappedData);
    memory->data.shared_memory.pMappedData = NULL;
}

dr_bool32 dripc_is_shared_memory_mapped__win32(dripc_handle memory)
{
    return memory->data.shared_memory.pMappedData != NULL;
}
#endif

#ifdef DR_IPC_UNIX
dripc_result dripc_alloc_handle__unix__shared_memory(const char* name, int oflag, dripc_handle* pMemory)
{
    size_t nameLen = strlen(name) + 1;  // +1 because we need to prepend a forward slash.

    dripc_handle memory;
    dripc_result result = dripc_alloc_handle__unix(dripc_handle_type_shared_memory, nameLen+1, &memory);    // +1 for null terminator.
    if (result != dripc_result_success) {
        return result;
    }

    dripc__strcpy_s(memory->name, nameLen+1, "/");
    dripc__strcat_s(memory->name, nameLen+1, name);

    memory->data.shared_memory.fd = shm_open(memory->name, oflag, 0666);
    if (memory->data.shared_memory.fd == -1) {
        dripc_free_handle__unix(memory);
        return dripc_result_from_unix_error(errno);
    }

    *pMemory = memory;
    return dripc_result_success;
}

dripc_result dripc_create_shared_memory__unix(const char* name, size_t sizeInBytes, dripc_handle* pMemory)
{
    dripc_handle memory;
    dripc_result result = dripc_alloc_handle__unix__shared_memory(name, O_RDWR | O_CREAT, &memory);    // +1 for null terminator.
    if (result != dripc_result_success) {
        return result;
    }

    if (ftruncate(memory->data.shared_memory.fd, sizeInBytes) == -1) {
        shm_unlink(memory->name);
        dripc_free_handle__unix(memory);
        return dripc_result_from_unix_error(errno);
    }

    *pMemory = memory;
    return dripc_result_success;
}

dripc_result dripc_open_shared_memory__unix(const char* name, dripc_handle* pMemory)
{
    dripc_handle memory;
    dripc_result result = dripc_alloc_handle__unix__shared_memory(name, O_RDWR, &memory);    // +1 for null terminator.
    if (result != dripc_result_success) {
        return result;
    }

    *pMemory = memory;
    return dripc_result_success;
}

void dripc_delete_shared_memory__unix(dripc_handle memory)
{
    close(memory->data.shared_memory.fd);
    shm_unlink(memory->name);
    dripc_free_handle__unix(memory);
}

void dripc_close_shared_memory__unix(dripc_handle memory)
{
    close(memory->data.shared_memory.fd);
    dripc_free_handle__unix(memory);
}

dripc_result dripc_map_shared_memory__unix(dripc_handle memory, void** ppDataOut)
{
    // We always map the whole buffer, so we'll need to grab it's size first.
    struct stat sb;
    if (fstat(memory->data.shared_memory.fd, &sb) == -1) {
        return dripc_result_from_unix_error(errno);
    }

    memory->data.shared_memory.mappedDataSize = sb.st_size;
    memory->data.shared_memory.pMappedData = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, memory->data.shared_memory.fd, 0);
    if (memory->data.shared_memory.pMappedData == MAP_FAILED) {
        return dripc_result_from_unix_error(errno);
    }

    *ppDataOut = memory->data.shared_memory.pMappedData;
    return dripc_result_success;
}

void dripc_unmap_shared_memory__unix(dripc_handle memory)
{
    munmap(memory->data.shared_memory.pMappedData, memory->data.shared_memory.mappedDataSize);
    memory->data.shared_memory.pMappedData = NULL;
    memory->data.shared_memory.mappedDataSize = 0;
}

dr_bool32 dripc_is_shared_memory_mapped__unix(dripc_handle memory)
{
    return memory->data.shared_memory.pMappedData != NULL;
}
#endif

dripc_result dripc_create_shared_memory(const char* name, size_t sizeInBytes, dripc_handle* pMemory)
{
    if (pMemory == NULL) return dripc_result_invalid_args;

#ifdef DR_IPC_WIN32
    return dripc_create_shared_memory__win32(name, sizeInBytes, pMemory);
#endif

#ifdef DR_IPC_UNIX
    return dripc_create_shared_memory__unix(name, sizeInBytes, pMemory);
#endif
}

dripc_result dripc_open_shared_memory(const char* name, dripc_handle* pMemory)
{
    if (pMemory == NULL) return dripc_result_invalid_args;

#ifdef DR_IPC_WIN32
    return dripc_open_shared_memory__win32(name, pMemory);
#endif

#ifdef DR_IPC_UNIX
    return dripc_open_shared_memory__unix(name, pMemory);
#endif
}

void dripc_delete_shared_memory(dripc_handle memory)
{
    if (memory == NULL || memory->type != dripc_handle_type_shared_memory) return;

#ifdef DR_IPC_WIN32
    dripc_delete_shared_memory__win32(memory);
#endif

#ifdef DR_IPC_UNIX
    dripc_delete_shared_memory__unix(memory);
#endif
}

void dripc_close_shared_memory(dripc_handle memory)
{
    if (memory == NULL || memory->type != dripc_handle_type_shared_memory) return;

#ifdef DR_IPC_WIN32
    dripc_close_shared_memory__win32(memory);
#endif

#ifdef DR_IPC_UNIX
    dripc_close_shared_memory__unix(memory);
#endif
}

dripc_result dripc_map_shared_memory(dripc_handle memory, void** ppDataOut)
{
    if (memory == NULL || memory->type != dripc_handle_type_shared_memory || ppDataOut == NULL) return dripc_result_invalid_args;

    if (dripc_is_shared_memory_mapped(memory)) {
        return dripc_result_memory_already_mapped;
    }

#ifdef DR_IPC_WIN32
    return dripc_map_shared_memory__win32(memory, ppDataOut);
#endif

#ifdef DR_IPC_UNIX
    return dripc_map_shared_memory__unix(memory, ppDataOut);
#endif
}

void dripc_unmap_shared_memory(dripc_handle memory)
{
    if (memory == NULL || memory->type != dripc_handle_type_shared_memory) return;

    if (!dripc_is_shared_memory_mapped(memory)) {
        return; // It's not mapped.
    }

#ifdef DR_IPC_WIN32
    dripc_unmap_shared_memory__win32(memory);
#endif

#ifdef DR_IPC_UNIX
    dripc_unmap_shared_memory__unix(memory);
#endif
}

dr_bool32 dripc_is_shared_memory_mapped(dripc_handle memory)
{
    if (memory == NULL || memory->type != dripc_handle_type_shared_memory) return DR_FALSE;

#ifdef DR_IPC_WIN32
    return dripc_is_shared_memory_mapped__win32(memory);
#endif

#ifdef DR_IPC_UNIX
    return dripc_is_shared_memory_mapped__unix(memory);
#endif
}
#endif  // DR_IPC_NO_SHARED_MEMORY
#endif  // DR_IPC_IMPLEMENTATION


// REVISION HISTORY
//
// v0.3a - 2016-10-11
//   - Rename drBool32 to dr_bool32 for styling consistency.
//
// v0.3 - 2016-09-29
//   - API/ABI CHANGE. Use dr_bool32 instead of the built-in bool type. Rationale is for ABI consistency between C and C++ builds.
//
// v0.2 - 2016-09-20
//   - Add support for shared memory.
//   - Use ISO 8601 format for dates (YYYY-MM-DD).
//
// v0.1a - 2016-08-04
//   - Compilation fixes for *nix builds.
//
// v0.1 - 2016-07-31
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

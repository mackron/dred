// Copyright (C) 2017 David Reid. See included LICENSE file.

// Keeping these types opaque because otherwise it would require exposing system headers like windows.h
// to the public section of this file.
typedef void* dtk_pipe;
typedef struct dtk_ipc_handle_impl dtk_ipc_handle_impl;
typedef dtk_ipc_handle_impl *dtk_ipc_handle;

#define DTK_IPC_READ     0x01
#define DTK_IPC_WRITE    0x02

#define DTK_IPC_INFINITE 0xFFFFFFFF

// Opens a server-side pipe.
//
// This will block until a client is connected. Calling dtk_pipe_close() will _not_ force it to return early.
//
// On *nix platforms the pipe will be named as "/tmp/{name}" by default, but it can be changed by #define-ing DTK_IPC_PIPE_TO_WIN32_HANDLE.
dtk_result dtk_pipe_open_named_server(const char* name, unsigned int options, dtk_pipe* pPipeOut);

// Opens the client-side end of a named pipe.
//
// If the server-side end of the pipe does not exist, this will fail.
dtk_result dtk_pipe_open_named_client(const char* name, unsigned int options, dtk_pipe* pPipeOut);

// Opens an anonymous pipe.
dtk_result dtk_pipe_open_anonymous(dtk_pipe* pPipeRead, dtk_pipe* pPipeWrite);

// Closes a pipe opened with dtk_pipe_open_named_server(), dtk_pipe_open_named_client() or dtk_pipe_open_anonymous().
void dtk_pipe_close(dtk_pipe pipe);


// Reads data from a pipe.
//
// This is a blocking call, and may not return the exact number of bytes requested. In addition, it is not guaranteed that
// writes from one end of the pipe is atomic.
dtk_result dtk_pipe_read(dtk_pipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead);

// Reads data from a pipe and does not return until either an error occurs or exactly the number of requested bytes have been read.
//
// This is a blocking call.
dtk_result dtk_pipe_read_exact(dtk_pipe pipe, void* pDataOut, size_t bytesToRead, size_t* pBytesRead);


// Writes data to a pipe.
//
// This blocks until the data has been written.
dtk_result dtk_pipe_write(dtk_pipe pipe, const void* pData, size_t bytesToWrite, size_t* pBytesWritten);


// Internally, DTK needs to translate the name of a pipe to a platform-specific name. This function returns that internal name.
//
// Returns the length of the name. If nameOut is NULL the return value is the required size, not including the null terminator.
size_t dtk_pipe_get_translated_name(const char* name, char* nameOut, size_t nameOutSize);

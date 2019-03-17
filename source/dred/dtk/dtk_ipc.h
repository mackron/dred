// Copyright (C) 2019 David Reid. See included LICENSE file.

// USAGE
//
// On Linux you will need to link to the following:
//   -lrt (shared memory)
//
//
// --- Pipes ---
//
// For named pipes, DTK's IPC uses the notion of a server and client. The server is the one who initially creates the
// pipe and the client is the one who connects after the fact. To create ther server-side end of a named pipe, do
// something like the following:
//
//   dtk_pipe serverPipe;
//   dtk_result result = dtk_pipe_open_named_server("my_pipe_name", DTK_IPC_READ | DTK_IPC_WRITE, &serverPipe);
//   if (result != DTK_SUCCESS) {
//       return -1;
//   }
//
// The above function will block until a client is connected on the other end of the pipe so you will likely want to
// do this on a separate thread. Connecting on the client side is very similar:
//
//   dtk_pipe clientPipe;
//   dtk_result result = dtk_pipe_open_named_client("my_pipe_name", DTK_IPC_READ | DTK_IPC_WRITE, &clientPipe);
//   if (result != DTK_SUCCESS) {
//       return -1;
//   }
//
// To read and write data, use dtk_pipe_read() and dtk_pipe_write() respectively. These functions are both blocking. You
// can also use dtk_pipe_read_exact() to continuously read bytes until exactly the number of bytes requested have been
// read.
//
// Internally, for all platforms, the name of the pipe is translated to a platform-specific name. To get this name,
// use the dtk_pipe_get_translated_name() API. On *nix platforms the pipe will be named as "/tmp/{your pipe name}" by
// default. This can be changed by #define-ing DTK_IPC_UNIX_PIPE_NAME_HEAD before #include-ing this file.
//
// An anonymous pipe can be created with the dtk_pipe_open_anonymous() API.
// 
//
// --- Shared Memory ---
//
// Note for Win32: Shared memory requires Administrator rights for it to work. You will want to embed a manifest as
// detailed here: https://msdn.microsoft.com/en-us/library/bb756929.aspx.
//
// Note for Linux: You will need to link to "rt" to use shared memory.
//
// To use shared memory, an application must first allocate it with dtk_create_shared_memory(). Afterwards, other
// applications can connect to this memory with dtk_open_shared_memory().
//
// Secondary applications should disconnect from the memory with dtk_close_shared_memory(). The memory can be freed
// completely with dtk_delete_shared_memory().
//
// The aforementioned APIs return a handle representing the memory. A pointer to the underlying memory can be retrieved
// by mapping the handle with dtk_map_shared_memory(). Unmap the memory with dtk_unmap_shared_memory(). It is an
// error to map memory if it's already mapped.
//
// Example Server Code:
//
//   dtk_ipc_handle hMem;
//   dtk_result result = dtk_create_shared_memory("MyGlobalMemory", sharedMemorySizeInBytes, &hMem);
//   if (result != dtk_result_success) {
//       return -1;
//   }
//
//   void* pSharedData;
//   result = dtk_map_shared_memory(hMem, &pSharedData);
//   if (result != dtk_result_success) {
//       return -1;
//   }
//
//   memcpy(pSharedData, pMySharedData, mySharedDataSizeInBytes);   // Use the mapped pointer like any other.
//
//   ...
//
//   dtk_unmap_shared_memory(hMem);
//   dtk_delete_shared_memory(hMem);
//
//
// Example Client Code:
//
//   dtk_ipc_handle hMem;
//   dtk_result result = dtk_open_shared_memory("MyGlobalMemory", &hMem);
//   if (result != dtk_result_success) {
//       return -1;
//   }
//
//   void* pSharedData;
//   result = dtk_map_shared_memory(hMem, &pSharedData);
//   if (result != dtk_result_success) {
//       return -1;
//   }
//
//   // Do something with pSharedData...
//
//   ...
//
//   dtk_unmap_shared_memory(hMem);
//   dtk_close_shared_memory(hMem);
//
//
// OPTIONS
// #define these options before including this file.
//
// #define DTK_IPC_NO_SHARED_MEMORY
//   Disables shared memory.
//
//
// QUICK NOTES
// - Currently, only pipes have been implemented.
// - Non-blocking pipes are not supported.


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


///////////////////////////////////////////////////////////////////////////////
//
// Shared Memory
//
///////////////////////////////////////////////////////////////////////////////

// Allocates a block of shared memory and returns a handle representing it.
//
// This should be used by the process that initially allocates the shared memory. Use dtk_open_shared_memory() to
// allow a secondary process to gain access to the memory.
dtk_result dtk_create_shared_memory(const char* name, size_t sizeInBytes, dtk_ipc_handle* pMemory);

// Connects the running process to a block of shared memory that was previously allocated by another instance.
dtk_result dtk_open_shared_memory(const char* name, dtk_ipc_handle* pMemory);

// Destroys a shared memory object. The opposite of dtk_create_shared_memory().
void dtk_delete_shared_memory(dtk_ipc_handle memory);

// Closes the connection to the shared memory. The opposite of dtk_open_shared_memory(). This does not
// deallocate the memory.
void dtk_close_shared_memory(dtk_ipc_handle memory);

// Returns a usable pointer to the shared memory.
//
// It is an error to map shared memory if it's already mapped.
dtk_result dtk_map_shared_memory(dtk_ipc_handle memory, void** ppDataOut);

// Unmaps the shared memory. 
void dtk_unmap_shared_memory(dtk_ipc_handle memory);

// Determines whether or not the given memory object is currently mapped.
dtk_bool32 dtk_is_shared_memory_mapped(dtk_ipc_handle memory);

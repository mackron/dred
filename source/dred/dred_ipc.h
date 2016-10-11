// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_IPC_MESSAGE_TERMINATOR 0
#define DRED_IPC_MESSAGE_ACTIVATE   1
#define DRED_IPC_MESSAGE_OPEN       2

#define DRED_IPC_MAGIC_NUMBER       0x2F8A572D

#pragma pack(4)
typedef struct
{
    uint32_t magic;
    uint32_t message;
    uint32_t size;  // <-- The size of the data not including this header.
} dred_ipc_message_header;
#pragma pack()

dr_bool32 dred_ipc_post_message(drpipe clientPipe, uint32_t message, const void* pData, size_t dataSize);

// Free the returned data with free().
//
// If the size of the data is 0, the returned pointer will be null. Do not use the returned pointer for
// error checking. Use the return value for error checking.
dr_bool32 dred_ipc_read_message(drpipe serverPipe, dred_ipc_message_header* pHeaderOut, void** ppDataOut);


// Retrieves the name to use for the IPC pipe. This will be namespaced based on the username of the current user.
dr_bool32 dred_ipc_get_pipe_name(char* nameOut, size_t nameOutSize);

// Retrieves the name to use for the lock file for IPC. This is namespaced based on the username of the current user.
dr_bool32 dred_ipc_get_lock_name(char* nameOut, size_t nameOutSize);

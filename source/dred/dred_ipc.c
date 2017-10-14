// Copyright (C) 2017 David Reid. See included LICENSE file.

dr_bool32 dred_ipc_post_message(dtk_pipe clientPipe, uint32_t message, const void* pData, size_t dataSize)
{
    size_t messageSize = sizeof(dred_ipc_message_header) + dataSize;
    uint8_t* pMessageData = (uint8_t*)malloc(messageSize);
    if (pMessageData == NULL) {
        return DR_FALSE;
    }

    dred_ipc_message_header header;
    header.magic = DRED_IPC_MAGIC_NUMBER;
    header.message = message;
    header.size = (uint32_t)dataSize;

    memcpy(pMessageData, &header, sizeof(header));
    
    if (dataSize > 0) {
        memcpy(pMessageData + sizeof(header), pData, dataSize);
    }

    size_t bytesWritten;
    dtk_result result = dtk_pipe_write(clientPipe, pMessageData, messageSize, &bytesWritten);

    free(pMessageData);
    return result == DTK_SUCCESS && bytesWritten == messageSize;
}

dr_bool32 dred_ipc_read_message(dtk_pipe serverPipe, dred_ipc_message_header* pHeaderOut, void** ppDataOut)
{
    if (ppDataOut == NULL) return DR_FALSE;
    *ppDataOut = NULL;

    if (pHeaderOut == NULL) {
        return DR_FALSE;
    }


    size_t bytesRead;
    dtk_result result = dtk_pipe_read_exact(serverPipe, pHeaderOut, sizeof(*pHeaderOut), &bytesRead);
    if (result != DTK_SUCCESS) {
        return DR_FALSE;
    }

    if (pHeaderOut->magic != DRED_IPC_MAGIC_NUMBER) {
        return DR_FALSE;
    }

    if (pHeaderOut->size == 0) {
        return DR_TRUE;
    }

    void* pData = malloc(pHeaderOut->size);
    if (pData == NULL) {
        return DR_FALSE;
    }

    *ppDataOut = pData;

    result = dtk_pipe_read_exact(serverPipe, pData, pHeaderOut->size, &bytesRead);
    if (result != DTK_SUCCESS) {
        free(pData);
        return DR_FALSE;
    }

    return DR_TRUE;
}


dr_bool32 dred_ipc_get_pipe_name(char* nameOut, size_t nameOutSize)
{
    if (nameOut == NULL || nameOutSize == 0) {
        return DR_FALSE;
    }

    if (dr_get_username(nameOut, nameOutSize) == 0) {
        return DR_FALSE;
    }

    if (strcat_s(nameOut, nameOutSize, ".") != 0) {
        return DR_FALSE;
    }
    if (strcat_s(nameOut, nameOutSize, "dred.pipe") != 0) {
        return DR_FALSE;
    }

    return DR_TRUE;
}

dr_bool32 dred_ipc_get_lock_name(char* nameOut, size_t nameOutSize)
{
    if (nameOut == NULL || nameOutSize == 0) {
        return DR_FALSE;
    }

#ifdef _WIN32
    nameOut[0] = '\0';
    return DR_TRUE;
#else
    if (strcpy_s(nameOut, nameOutSize, "/tmp/") != 0) {
        return DR_FALSE;
    }

    char username[512];
    if (dr_get_username(username, sizeof(username)) == 0) {
        return DR_FALSE;
    }

    if (strcat_s(nameOut, nameOutSize, username) != 0) {
        return DR_FALSE;
    }
    if (strcat_s(nameOut, nameOutSize, ".") != 0) {
        return DR_FALSE;
    }
    if (strcat_s(nameOut, nameOutSize, "dred.lock") != 0) {
        return DR_FALSE;
    }

    return DR_TRUE;
#endif
}
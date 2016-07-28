// Copyright (C) 2016 David Reid. See included LICENSE file.

bool dred_ipc_post_message(drpipe clientPipe, uint32_t message, const void* pData, size_t dataSize)
{
    size_t messageSize = sizeof(dred_ipc_message_header) + dataSize;
    uint8_t* pMessageData = (uint8_t*)malloc(messageSize);
    if (pMessageData == NULL) {
        return false;
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
    dripc_result result = drpipe_write(clientPipe, pMessageData, messageSize, &bytesWritten);

    free(pMessageData);
    return result == dripc_result_success && bytesWritten == messageSize;
}

bool dred_ipc_read_message(drpipe serverPipe, dred_ipc_message_header* pHeaderOut, void** ppDataOut)
{
    if (ppDataOut == NULL) return false;
    *ppDataOut = NULL;

    if (pHeaderOut == NULL) {
        return false;
    }


    size_t bytesRead;
    dripc_result result = drpipe_read_exact(serverPipe, pHeaderOut, sizeof(*pHeaderOut), &bytesRead);
    if (result != dripc_result_success) {
        return false;
    }

    if (pHeaderOut->magic != DRED_IPC_MAGIC_NUMBER) {
        return false;
    }

    if (pHeaderOut->size == 0) {
        return true;
    }

    void* pData = malloc(pHeaderOut->size);
    if (pData == NULL) {
        return false;
    }

    *ppDataOut = pData;

    result = drpipe_read_exact(serverPipe, pData, pHeaderOut->size, &bytesRead);
    if (result != dripc_result_success) {
        free(pData);
        return false;
    }

    return true;
}


bool dred_ipc_get_pipe_name(char* nameOut, size_t nameOutSize)
{
    if (nameOut == NULL || nameOutSize == 0) {
        return false;
    }

    if (dr_get_username(nameOut, nameOutSize) == 0) {
        return false;
    }

    if (strcat_s(nameOut, nameOutSize, ".") != 0) {
        return false;
    }
    if (strcat_s(nameOut, nameOutSize, "dred.pipe") != 0) {
        return false;
    }

    return true;
}

bool dred_ipc_get_lock_name(char* nameOut, size_t nameOutSize)
{
    if (nameOut == NULL || nameOutSize == 0) {
        return false;
    }

#ifdef _WIN32
    nameOut[0] = '\0';
    return true;
#else
    if (strcpy_s(nameOut, nameOutSize, "/tmp/") != 0) {
        return false;
    }

    char username[512];
    if (dr_get_username(username, sizeof(username)) == 0) {
        return false;
    }

    if (strcat_s(nameOut, nameOutSize, username) != 0) {
        return false;
    }
    if (strcat_s(nameOut, nameOutSize, ".") != 0) {
        return false;
    }
    if (strcat_s(nameOut, nameOutSize, "dred.lock") != 0) {
        return false;
    }

    return true;
#endif
}
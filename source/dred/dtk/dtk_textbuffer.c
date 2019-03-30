// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_result dtk_textbuffer_init(dtk_textbuffer* pTextBuffer)
{
    if (pTextBuffer == NULL) {
        return DTK_INVALID_ARGS;
    }

    dtk_zero_object(pTextBuffer);



    return DTK_SUCCESS;
}

void dtk_textbuffer_uninit(dtk_textbuffer* pTextBuffer)
{
    if (pTextBuffer == NULL) {
        return;
    }
}

dtk_result dtk_textbuffer_set_text_utf8(dtk_textbuffer* pTextBuffer, const char* pUTF8String, size_t stringLengthInBytes)
{
    if (pTextBuffer == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_textbuffer_update_utf8(pTextBuffer, 0, dtk_textbuffer_get_text_size(pTextBuffer), pUTF8String, stringLengthInBytes);
}

dtk_result dtk_textbuffer_get_text_utf8(dtk_textbuffer* pTextBuffer, char* pUTF8BufferOut, size_t bufferCapInBytes)
{
    if (pUTF8BufferOut == NULL || bufferCapInBytes == 0) {
        return DTK_INVALID_ARGS;
    }

    pUTF8BufferOut[0] = '\0';

    if (pTextBuffer == NULL) {
        return DTK_INVALID_ARGS;
    }

    if (bufferCapInBytes < dtk_textbuffer_get_text_size(pTextBuffer)) { /* +1 for null-terminator. */
        return DTK_INVALID_ARGS;
    }

    /* TODO: Implement me. */

    return DTK_SUCCESS;
}

size_t dtk_textbuffer_get_text_size(dtk_textbuffer* pTextBuffer)
{
    if (pTextBuffer == NULL) {
        return 0;
    }

    /* TODO: Implement me. */
    return 0;
}

dtk_result dtk_textbuffer_insert_utf8(dtk_textbuffer* pTextBuffer, size_t insertPosInBytes, const char* pUTF8String, size_t stringLengthInBytes)
{
    dtk_result result;

    if (pTextBuffer == NULL) {
        return DTK_INVALID_ARGS;
    }
    if (stringLengthInBytes == 0) {
        return DTK_INVALID_ARGS;    /* Trying to insert an empty string. */
    }
    if (stringLengthInBytes == (size_t)-1 && dtk_string_is_null_or_empty(pUTF8String)) {
        return DTK_INVALID_ARGS;
    }
    if (insertPosInBytes > dtk_textbuffer_get_text_size(pTextBuffer)) {
        return DTK_INVALID_ARGS;    /* Trying to insert past the end of the buffer. */
    }

    if (stringLengthInBytes == (size_t)-1) {
        stringLengthInBytes = strlen(pUTF8String);
    }

    result = dtk_textbuffer_begin_undo_point(pTextBuffer);
    if (result != DTK_SUCCESS) {
        return result;
    }

    /* TODO: Implement me. */

    result = dtk_textbuffer_commit_undo_point(pTextBuffer);
    if (result != DTK_SUCCESS) {
        dtk_textbuffer_rollback_undo_point(pTextBuffer);
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_textbuffer_delete_utf8(dtk_textbuffer* pTextBuffer, size_t deletePosBegInBytes, size_t deletePosEndInBytesPlus1)
{
    dtk_result result;

    if (pTextBuffer == NULL) {
        return DTK_INVALID_ARGS;
    }
    if (deletePosBegInBytes > deletePosEndInBytesPlus1) {
        return DTK_INVALID_ARGS;    /* The start position is past the end position. */
    }

    if (deletePosBegInBytes == deletePosEndInBytesPlus1) {
        return DTK_SUCCESS;         /* Nothing to delete. Pretend it's true. */
    }

    result = dtk_textbuffer_begin_undo_point(pTextBuffer);
    if (result != DTK_SUCCESS) {
        return result;
    }

    /* TODO: Implement me. */

    result = dtk_textbuffer_commit_undo_point(pTextBuffer);
    if (result != DTK_SUCCESS) {
        dtk_textbuffer_rollback_undo_point(pTextBuffer);
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_textbuffer_update_utf8(dtk_textbuffer* pTextBuffer, size_t updatePosBegInBytes, size_t updatePosEndInBytesPlus1, const char* pUTF8String, size_t stringLengthInBytes)
{
    dtk_result result;

    if (pTextBuffer == NULL) {
        return DTK_INVALID_ARGS;
    }
    if (updatePosBegInBytes > updatePosEndInBytesPlus1) {
        return DTK_INVALID_ARGS;
    }

    if (pUTF8String == NULL) {
        pUTF8String = "";
        stringLengthInBytes = 0;
    }

    if (stringLengthInBytes == (size_t)-1) {
        stringLengthInBytes = strlen(pUTF8String);
    }

    /* Do this as a delete and then insert. */
    result = dtk_textbuffer_begin_undo_point(pTextBuffer);
    if (result != DTK_SUCCESS) {
        return result;
    }

    result = dtk_textbuffer_delete_utf8(pTextBuffer, updatePosBegInBytes, updatePosEndInBytesPlus1);
    if (result != DTK_SUCCESS) {
        dtk_textbuffer_rollback_undo_point(pTextBuffer);
        return result;
    }

    result = dtk_textbuffer_insert_utf8(pTextBuffer, updatePosBegInBytes, pUTF8String, stringLengthInBytes);
    if (result != DTK_SUCCESS) {
        dtk_textbuffer_rollback_undo_point(pTextBuffer);
        return result;
    }

    result = dtk_textbuffer_commit_undo_point(pTextBuffer);
    if (result != DTK_SUCCESS) {
        dtk_textbuffer_rollback_undo_point(pTextBuffer);
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_textbuffer_begin_undo_point(dtk_textbuffer* pTextBuffer)
{
    if (pTextBuffer == NULL) {
        return DTK_INVALID_ARGS;
    }

    /* TODO: Implement me. */

    return DTK_SUCCESS;
}

dtk_result dtk_textbuffer_commit_undo_point(dtk_textbuffer* pTextBuffer)
{
    if (pTextBuffer == NULL) {
        return DTK_INVALID_ARGS;
    }

    /*
    TODO: If this is the last undo point in the current nesting then we need to post the changes to the views. Otherwise we just concatenate the
    changes to the undo point one level up.
    */

    return DTK_SUCCESS;
}

dtk_result dtk_textbuffer_rollback_undo_point(dtk_textbuffer* pTextBuffer)
{
    if (pTextBuffer == NULL) {
        return DTK_INVALID_ARGS;
    }

    /* TODO: Undo only the changes in the current nesting. Do not send change events to the views. */

    return DTK_SUCCESS;
}

dtk_result dtk_textbuffer_clear_undo_stack(dtk_textbuffer* pTextBuffer)
{
    if (pTextBuffer == NULL) {
        return DTK_INVALID_ARGS;
    }

    /* TODO: Implement me. Send an event to the views so they can do their own clean up of any custom undo/redo state. */

    return DTK_SUCCESS;
}

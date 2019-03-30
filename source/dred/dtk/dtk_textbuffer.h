// Copyright (C) 2019 David Reid. See included LICENSE file.

struct dtk_textbuffer
{
    size_t textLen; /* Not including null terminator. */
};

dtk_result dtk_textbuffer_init(dtk_textbuffer* pTextBuffer);
void dtk_textbuffer_uninit(dtk_textbuffer* pTextBuffer);
dtk_result dtk_textbuffer_set_text_utf8(dtk_textbuffer* pTextBuffer, const char* pUTF8String, size_t stringLengthInBytes);
dtk_result dtk_textbuffer_get_text_utf8(dtk_textbuffer* pTextBuffer, char* pUTF8BufferOut, size_t bufferCapInBytes);
size_t dtk_textbuffer_get_text_size(dtk_textbuffer* pTextBuffer); /* In bytes. Includes the null terminator. Returns 0 on error. */
dtk_result dtk_textbuffer_insert_utf8(dtk_textbuffer* pTextBuffer, size_t insertPosInBytes, const char* pUTF8String, size_t stringLengthInBytes);
dtk_result dtk_textbuffer_delete_utf8(dtk_textbuffer* pTextBuffer, size_t deletePosBegInBytes, size_t deletePosEndInBytesPlus1);
dtk_result dtk_textbuffer_update_utf8(dtk_textbuffer* pTextBuffer, size_t updatePosBegInBytes, size_t updatePosEndInBytesPlus1, const char* pUTF8String, size_t stringLengthInBytes);
dtk_result dtk_textbuffer_begin_undo_point(dtk_textbuffer* pTextBuffer);
dtk_result dtk_textbuffer_commit_undo_point(dtk_textbuffer* pTextBuffer);
dtk_result dtk_textbuffer_rollback_undo_point(dtk_textbuffer* pTextBuffer);
dtk_result dtk_textbuffer_clear_undo_stack(dtk_textbuffer* pTextBuffer);

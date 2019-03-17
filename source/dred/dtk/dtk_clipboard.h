// Copyright (C) 2019 David Reid. See included LICENSE file.

// Sets the text on the clipboard.
dtk_bool32 dtk_clipboard_set_text(dtk_context* pTK, const char* text, size_t textLength);

// Retrieves the text on the clipboard.
//
// The returned string must be freed with dtk_clipboard_free_text().
char* dtk_clipboard_get_text(dtk_context* pTK);

// Frees the text returned by dtk_clipboard_get_text().
void dtk_clipboard_free_text(dtk_context* pTK, char* text);
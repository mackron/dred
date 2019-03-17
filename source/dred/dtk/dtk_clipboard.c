// Copyright (C) 2019 David Reid. See included LICENSE file.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// WIN32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
dtk_bool32 dtk_clipboard_set_text__win32(dtk_context* pTK, const char* text, size_t textLength)
{
    (void)pTK;

    // We must ensure line endlings are normalized to \r\n. If we don't do this pasting won't work
    // correctly in things like Notepad.
    //
    // We allocate a buffer x2 the size of the original string to guarantee there will be enough room
    // for the extra \r character's we'll be adding.
    HGLOBAL hTextMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, ((textLength*2 + 1) * sizeof(char)));
    if (hTextMem == NULL) {
        return DTK_FALSE;
    }

    char* textRN = (char*)GlobalLock(hTextMem);
    if (textRN == NULL) {
        GlobalFree(hTextMem);
        return DTK_FALSE;
    }

    if (!OpenClipboard(NULL)) {
        GlobalFree(hTextMem);
        return DTK_FALSE;
    }

    if (!EmptyClipboard()) {
        GlobalFree(hTextMem);
        CloseClipboard();
        return DTK_FALSE;
    }

    while (*text != '\0' && textLength > 0) {
        if (text[0] == '\r' && textLength > 1 && text[1] == '\n') {
            *textRN++ = '\r';
            *textRN++ = '\n';

            text += 2;
            textLength -= 2;
        } else {
            if (*text == '\n') {
                *textRN++ = '\r';
            }

            *textRN++ = *text++;
            textLength -= 1;
        }
    }

    *textRN = '\0';

    GlobalUnlock(hTextMem);

    if (SetClipboardData(CF_TEXT, hTextMem) == NULL) {
        GlobalFree(hTextMem);
        CloseClipboard();
        return DTK_FALSE;
    }

    CloseClipboard();
    return DTK_TRUE;
}

char* dtk_clipboard_get_text__win32(dtk_context* pTK)
{
    (void)pTK;

    if (!IsClipboardFormatAvailable(CF_TEXT)) {
        return 0;
    }

    if (!OpenClipboard(NULL)) {
        return 0;
    }

    HGLOBAL hTextMem = GetClipboardData(CF_TEXT);
    if (hTextMem == NULL) {
        CloseClipboard();
        return 0;
    }

    char* textRN = (char*)GlobalLock(hTextMem);
    if (textRN == NULL) {
        CloseClipboard();
        return 0;
    }

    size_t textRNLength = strlen(textRN);
    char* result = (char*)dtk_malloc(textRNLength + 1);
    dtk_strcpy_s(result, textRNLength + 1, textRN);

    GlobalUnlock(hTextMem);
    CloseClipboard();

    return result;
}

void dtk_clipboard_free_text__win32(dtk_context* pTK, char* text)
{
    (void)pTK;
    dtk_free(text);
}
#endif  // DTK_WIN32


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// GTK
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_GTK
GtkClipboard* dtk_get_gtk_clipboard()
{
    return gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);
}

dtk_bool32 dtk_clipboard_set_text__gtk(dtk_context* pTK, const char* text, size_t textLength)
{
    (void)pTK;

    gtk_clipboard_set_text(dtk_get_gtk_clipboard(), text, textLength);
    return DTK_TRUE;
}

char* dtk_clipboard_get_text__gtk(dtk_context* pTK)
{
    (void)pTK;
    return gtk_clipboard_wait_for_text(dtk_get_gtk_clipboard());
}

void dtk_clipboard_free_text__gtk(dtk_context* pTK, char* text)
{
    (void)pTK;
    g_free(text);
}
#endif  // DTK_GTK

dtk_bool32 dtk_clipboard_set_text(dtk_context* pTK, const char* text, size_t textLength)
{
    if (pTK == NULL) return DTK_FALSE;

    if (text == NULL) {
        text = "";
        textLength = 0;
    }

    if (textLength == (size_t)-1) {
        textLength = strlen(text);
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    result = dtk_clipboard_set_text__win32(pTK, text, textLength);
#endif
#ifdef DTK_GTK
    result = dtk_clipboard_set_text__gtk(pTK, text, textLength);
#endif

    return result;
}

char* dtk_clipboard_get_text(dtk_context* pTK)
{
    if (pTK == NULL) return NULL;

#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        return dtk_clipboard_get_text__win32(pTK);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        return dtk_clipboard_get_text__gtk(pTK);
    }
#endif

    return NULL;    // No backend.
}

void dtk_clipboard_free_text(dtk_context* pTK, char* text)
{
    if (pTK == NULL || text == NULL) return;

#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        dtk_clipboard_free_text__win32(pTK, text);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        dtk_clipboard_free_text__gtk(pTK, text);
    }
#endif
}
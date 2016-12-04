// Copyright (C) 2016 David Reid. See included LICENSE file.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// WIN32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
dtk_bool32 dtk_clipboard_set_text__win32(const char* text, size_t textLength)
{
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

char* dtk_clipboard_get_text__win32()
{
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

void dtk_clipboard_free_text__win32(char* text)
{
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

dtk_bool32 dtk_clipboard_set_text__gtk(const char* text, size_t textLength)
{
    gtk_clipboard_set_text(dtk_get_gtk_clipboard(), text, textLength);
    return DTK_TRUE;
}

char* dtk_clipboard_get_text__gtk()
{
    return gtk_clipboard_wait_for_text(dtk_get_gtk_clipboard());
}

void dtk_clipboard_free_text__gtk(char* text)
{
    g_free(text);
}
#endif  // DTK_GTK

dtk_bool32 dtk_clipboard_set_text(const char* text, size_t textLength)
{
    if (text == NULL) {
        text = "";
        textLength = 0;
    }

    if (textLength == (size_t)-1) {
        textLength = strlen(text);
    }

#ifdef DTK_WIN32
    return dtk_clipboard_set_text__win32(text, textLength);
#endif
#ifdef DTK_GTK
    return dtk_clipboard_set_text__gtk(text, textLength);
#endif
}

char* dtk_clipboard_get_text()
{
#ifdef DTK_WIN32
    return dtk_clipboard_get_text__win32();
#endif
#ifdef DTK_GTK
    return dtk_clipboard_get_text__gtk();
#endif
}

void dtk_clipboard_free_text(char* text)
{
    if (text == NULL) return;

#ifdef DTK_WIN32
    dtk_clipboard_free_text__win32(text);
#endif
#ifdef DTK_GTK
    dtk_clipboard_free_text__gtk(text);
#endif
}
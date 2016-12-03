// Copyright (C) 2016 David Reid. See included LICENSE file.

static size_t dtk_strcpy_len(char* dst, size_t dstSize, const char* src)
{
    if (dtk_strcpy_s(dst, dstSize, src) == 0) {
        return strlen(dst);
    }

    return 0;
}

size_t dtk_key_to_string(dtk_key key, char* strOut, size_t strOutSize)
{
    if (strOut == NULL || strOutSize == 0) {
        return 0;
    }

    if (strOutSize == 1) {
        strOut[0] = '\0';
        return 0;
    }


    switch (key)
    {
    case DTK_KEY_BACKSPACE:   return dtk_strcpy_len(strOut, strOutSize, "Backspace");
    case DTK_KEY_SHIFT:       return dtk_strcpy_len(strOut, strOutSize, "Shift");
    case DTK_KEY_ESCAPE:      return dtk_strcpy_len(strOut, strOutSize, "Escape");
    case DTK_KEY_PAGE_UP:     return dtk_strcpy_len(strOut, strOutSize, "Page Up");
    case DTK_KEY_PAGE_DOWN:   return dtk_strcpy_len(strOut, strOutSize, "Page Down");
    case DTK_KEY_END:         return dtk_strcpy_len(strOut, strOutSize, "End");
    case DTK_KEY_HOME:        return dtk_strcpy_len(strOut, strOutSize, "Home");
    case DTK_KEY_ARROW_LEFT:  return dtk_strcpy_len(strOut, strOutSize, "Arrow Left");
    case DTK_KEY_ARROW_UP:    return dtk_strcpy_len(strOut, strOutSize, "Arrow Up");
    case DTK_KEY_ARROW_RIGHT: return dtk_strcpy_len(strOut, strOutSize, "Arrow Right");
    case DTK_KEY_ARROW_DOWN:  return dtk_strcpy_len(strOut, strOutSize, "Arrow Down");
    case DTK_KEY_DELETE:      return dtk_strcpy_len(strOut, strOutSize, "Delete");
    case DTK_KEY_F1:          return dtk_strcpy_len(strOut, strOutSize, "F1");
    case DTK_KEY_F2:          return dtk_strcpy_len(strOut, strOutSize, "F2");
    case DTK_KEY_F3:          return dtk_strcpy_len(strOut, strOutSize, "F3");
    case DTK_KEY_F4:          return dtk_strcpy_len(strOut, strOutSize, "F4");
    case DTK_KEY_F5:          return dtk_strcpy_len(strOut, strOutSize, "F5");
    case DTK_KEY_F6:          return dtk_strcpy_len(strOut, strOutSize, "F6");
    case DTK_KEY_F7:          return dtk_strcpy_len(strOut, strOutSize, "F7");
    case DTK_KEY_F8:          return dtk_strcpy_len(strOut, strOutSize, "F8");
    case DTK_KEY_F9:          return dtk_strcpy_len(strOut, strOutSize, "F9");
    case DTK_KEY_F10:         return dtk_strcpy_len(strOut, strOutSize, "F10");
    case DTK_KEY_F11:         return dtk_strcpy_len(strOut, strOutSize, "F11");
    case DTK_KEY_F12:         return dtk_strcpy_len(strOut, strOutSize, "F12");
    }

    if (key >= 32 && key <= 126) {
        strOut[0] = (char)key;
        strOut[1] = '\0';
        return 1;
    }

    // TODO: Non-ascii characters.
    return 0;
}

dtk_key dtk_key_parse(const char* str)
{
    if (str == NULL || str[0] == '\0') {
        return 0;
    }

    if (_stricmp(str, "backspace")   == 0) return DTK_KEY_BACKSPACE;
    if (_stricmp(str, "shift")       == 0) return DTK_KEY_SHIFT;
    if (_stricmp(str, "escape")      == 0) return DTK_KEY_ESCAPE;
    if (_stricmp(str, "page up")     == 0 || _stricmp(str, "pageup")     == 0) return DTK_KEY_PAGE_UP;
    if (_stricmp(str, "page down")   == 0 || _stricmp(str, "pagedown")   == 0) return DTK_KEY_PAGE_DOWN;
    if (_stricmp(str, "end")         == 0) return DTK_KEY_END;
    if (_stricmp(str, "home")        == 0) return DTK_KEY_HOME;
    if (_stricmp(str, "arrow left")  == 0 || _stricmp(str, "arrowleft")  == 0) return DTK_KEY_ARROW_LEFT;
    if (_stricmp(str, "arrow up")    == 0 || _stricmp(str, "arrowup")    == 0) return DTK_KEY_ARROW_UP;
    if (_stricmp(str, "arrow right") == 0 || _stricmp(str, "arrowright") == 0) return DTK_KEY_ARROW_RIGHT;
    if (_stricmp(str, "arrow down")  == 0 || _stricmp(str, "arrowdown")  == 0) return DTK_KEY_ARROW_DOWN;
    if (_stricmp(str, "delete")      == 0) return DTK_KEY_BACKSPACE;

    if (str[0] == 'F' || str[0] == 'f') {
        if (str[1] ==  '1') {
            if (str[2] == '\0') {
                return DTK_KEY_F1;
            } else {
                if (str[2] == '0' && str[3] == '\0') return DTK_KEY_F10;
                if (str[2] == '1' && str[3] == '\0') return DTK_KEY_F11;
                if (str[2] == '2' && str[3] == '\0') return DTK_KEY_F12;
            }
        }
        if (str[1] == '2' && str[2] == '\0') return DTK_KEY_F2;
        if (str[1] == '3' && str[2] == '\0') return DTK_KEY_F3;
        if (str[1] == '4' && str[2] == '\0') return DTK_KEY_F4;
        if (str[1] == '5' && str[2] == '\0') return DTK_KEY_F5;
        if (str[1] == '6' && str[2] == '\0') return DTK_KEY_F6;
        if (str[1] == '7' && str[2] == '\0') return DTK_KEY_F7;
        if (str[1] == '8' && str[2] == '\0') return DTK_KEY_F8;
        if (str[1] == '9' && str[2] == '\0') return DTK_KEY_F9;
    }


    // ASCII characters.
    if (str[0] >= 32 && str[0] <= 126 && str[1] == '\0') {
        return str[0];
    }

    if (_stricmp(str, "tab") == 0) {
        return '\t';
    }


    // TODO: Non-ascii characters.
    return 0;
}
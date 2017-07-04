// Copyright (C) 2017 David Reid. See included LICENSE file.

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
    case DTK_KEY_TAB:         return dtk_strcpy_len(strOut, strOutSize, "Tab");
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

    if (key == '\t') {
        return dtk_strcpy_len(strOut, strOutSize, "Tab");
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

    if (dtk_stricmp(str, "backspace")   == 0) return DTK_KEY_BACKSPACE;
    if (dtk_stricmp(str, "shift")       == 0) return DTK_KEY_SHIFT;
    if (dtk_stricmp(str, "escape")      == 0) return DTK_KEY_ESCAPE;
    if (dtk_stricmp(str, "page up")     == 0 || dtk_stricmp(str, "pageup")     == 0) return DTK_KEY_PAGE_UP;
    if (dtk_stricmp(str, "page down")   == 0 || dtk_stricmp(str, "pagedown")   == 0) return DTK_KEY_PAGE_DOWN;
    if (dtk_stricmp(str, "end")         == 0) return DTK_KEY_END;
    if (dtk_stricmp(str, "home")        == 0) return DTK_KEY_HOME;
    if (dtk_stricmp(str, "arrow left")  == 0 || dtk_stricmp(str, "arrowleft")  == 0) return DTK_KEY_ARROW_LEFT;
    if (dtk_stricmp(str, "arrow up")    == 0 || dtk_stricmp(str, "arrowup")    == 0) return DTK_KEY_ARROW_UP;
    if (dtk_stricmp(str, "arrow right") == 0 || dtk_stricmp(str, "arrowright") == 0) return DTK_KEY_ARROW_RIGHT;
    if (dtk_stricmp(str, "arrow down")  == 0 || dtk_stricmp(str, "arrowdown")  == 0) return DTK_KEY_ARROW_DOWN;
    if (dtk_stricmp(str, "delete")      == 0) return DTK_KEY_DELETE;

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

    if (dtk_stricmp(str, "tab") == 0) {
        return '\t';
    }


    // TODO: Non-ascii characters.
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// WIN32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
dtk_key dtk_convert_key_from_win32(WPARAM wParam)
{
    switch (wParam)
    {
    case VK_BACK:   return DTK_KEY_BACKSPACE;
    case VK_TAB:    return DTK_KEY_TAB;
    case VK_RETURN: return DTK_KEY_RETURN;
    case VK_SHIFT:  return DTK_KEY_SHIFT;
    case VK_ESCAPE: return DTK_KEY_ESCAPE;
    case VK_PRIOR:  return DTK_KEY_PAGE_UP;
    case VK_NEXT:   return DTK_KEY_PAGE_DOWN;
    case VK_END:    return DTK_KEY_END;
    case VK_HOME:   return DTK_KEY_HOME;
    case VK_LEFT:   return DTK_KEY_ARROW_LEFT;
    case VK_UP:     return DTK_KEY_ARROW_UP;
    case VK_RIGHT:  return DTK_KEY_ARROW_RIGHT;
    case VK_DOWN:   return DTK_KEY_ARROW_DOWN;
    case VK_DELETE: return DTK_KEY_DELETE;
    case VK_F1:     return DTK_KEY_F1;
    case VK_F2:     return DTK_KEY_F2;
    case VK_F3:     return DTK_KEY_F3;
    case VK_F4:     return DTK_KEY_F4;
    case VK_F5:     return DTK_KEY_F5;
    case VK_F6:     return DTK_KEY_F6;
    case VK_F7:     return DTK_KEY_F7;
    case VK_F8:     return DTK_KEY_F8;
    case VK_F9:     return DTK_KEY_F9;
    case VK_F10:    return DTK_KEY_F10;
    case VK_F11:    return DTK_KEY_F11;
    case VK_F12:    return DTK_KEY_F12;
    case VK_SPACE:  return DTK_KEY_SPACE;

    default: break;
    }

    return (dtk_key)wParam;
}

WORD dtk_convert_key_to_win32(dtk_key key)
{
    switch (key)
    {
    case DTK_KEY_BACKSPACE:   return VK_BACK;
    case DTK_KEY_TAB:         return VK_TAB;
    case DTK_KEY_RETURN:      return VK_RETURN;
    case DTK_KEY_SHIFT:       return VK_SHIFT;
    case DTK_KEY_ESCAPE:      return VK_ESCAPE;
    case DTK_KEY_PAGE_UP:     return VK_PRIOR;
    case DTK_KEY_PAGE_DOWN:   return VK_NEXT;
    case DTK_KEY_END:         return VK_END;
    case DTK_KEY_HOME:        return VK_HOME;
    case DTK_KEY_ARROW_LEFT:  return VK_LEFT;
    case DTK_KEY_ARROW_UP:    return VK_UP;
    case DTK_KEY_ARROW_RIGHT: return VK_RIGHT;
    case DTK_KEY_ARROW_DOWN:  return VK_DOWN;
    case DTK_KEY_DELETE:      return VK_DELETE;
    case DTK_KEY_F1:          return VK_F1;
    case DTK_KEY_F2:          return VK_F2;
    case DTK_KEY_F3:          return VK_F3;
    case DTK_KEY_F4:          return VK_F4;
    case DTK_KEY_F5:          return VK_F5;
    case DTK_KEY_F6:          return VK_F6;
    case DTK_KEY_F7:          return VK_F7;
    case DTK_KEY_F8:          return VK_F8;
    case DTK_KEY_F9:          return VK_F9;
    case DTK_KEY_F10:         return VK_F10;
    case DTK_KEY_F11:         return VK_F11;
    case DTK_KEY_F12:         return VK_F12;
    case DTK_KEY_SPACE:       return VK_SPACE;

    default: break;
    }

    return (WORD)key;
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
dtk_key dtk_convert_key_from_gtk(guint keyval)
{
    switch (keyval)
    {
    case GDK_KEY_BackSpace:    return DTK_KEY_BACKSPACE;
    case GDK_KEY_Tab:          return DTK_KEY_TAB;
    case GDK_KEY_ISO_Left_Tab: return DTK_KEY_TAB;
    case GDK_KEY_KP_Tab:       return DTK_KEY_TAB;
    case GDK_KEY_Return:       return DTK_KEY_RETURN;
    case GDK_KEY_Shift_L:      return DTK_KEY_SHIFT;
    case GDK_KEY_Shift_R:      return DTK_KEY_SHIFT;
    case GDK_KEY_Escape:       return DTK_KEY_ESCAPE;
    case GDK_KEY_Page_Up:      return DTK_KEY_PAGE_UP;
    case GDK_KEY_Page_Down:    return DTK_KEY_PAGE_DOWN;
    case GDK_KEY_End:          return DTK_KEY_END;
    case GDK_KEY_Home:         return DTK_KEY_HOME;
    case GDK_KEY_Left:         return DTK_KEY_ARROW_LEFT;
    case GDK_KEY_Up:           return DTK_KEY_ARROW_UP;
    case GDK_KEY_Right:        return DTK_KEY_ARROW_RIGHT;
    case GDK_KEY_Down:         return DTK_KEY_ARROW_DOWN;
    case GDK_KEY_Delete:       return DTK_KEY_DELETE;
    case GDK_KEY_F1:           return DTK_KEY_F1;
    case GDK_KEY_F2:           return DTK_KEY_F2;
    case GDK_KEY_F3:           return DTK_KEY_F3;
    case GDK_KEY_F4:           return DTK_KEY_F4;
    case GDK_KEY_F5:           return DTK_KEY_F5;
    case GDK_KEY_F6:           return DTK_KEY_F6;
    case GDK_KEY_F7:           return DTK_KEY_F7;
    case GDK_KEY_F8:           return DTK_KEY_F8;
    case GDK_KEY_F9:           return DTK_KEY_F9;
    case GDK_KEY_F10:          return DTK_KEY_F10;
    case GDK_KEY_F11:          return DTK_KEY_F11;
    case GDK_KEY_F12:          return DTK_KEY_F12;
    case GDK_KEY_space:        return DTK_KEY_SPACE;

    default: break;
    }

    if (keyval == GDK_KEY_Tab) {
        return '\t';
    }

    return (dtk_key)keyval;
}

guint dtk_convert_key_to_gtk(dtk_key key)
{
    switch (key)
    {
    case DTK_KEY_BACKSPACE:   return GDK_KEY_BackSpace;
    case DTK_KEY_TAB:         return GDK_KEY_Tab;
    case DTK_KEY_RETURN:      return GDK_KEY_Return;
    case DTK_KEY_SHIFT:       return GDK_KEY_Shift_L;
    //case DTK_KEY_SHIFT:       return GDK_KEY_Shift_R;
    case DTK_KEY_ESCAPE:      return GDK_KEY_Escape;
    case DTK_KEY_PAGE_UP:     return GDK_KEY_Page_Up;
    case DTK_KEY_PAGE_DOWN:   return GDK_KEY_Page_Down;
    case DTK_KEY_END:         return GDK_KEY_End;
    case DTK_KEY_HOME:        return GDK_KEY_Begin;
    case DTK_KEY_ARROW_LEFT:  return GDK_KEY_Left;
    case DTK_KEY_ARROW_UP:    return GDK_KEY_Up;
    case DTK_KEY_ARROW_RIGHT: return GDK_KEY_Right;
    case DTK_KEY_ARROW_DOWN:  return GDK_KEY_Down;
    case DTK_KEY_DELETE:      return GDK_KEY_Delete;
    case DTK_KEY_F1:          return GDK_KEY_F1;
    case DTK_KEY_F2:          return GDK_KEY_F2;
    case DTK_KEY_F3:          return GDK_KEY_F3;
    case DTK_KEY_F4:          return GDK_KEY_F4;
    case DTK_KEY_F5:          return GDK_KEY_F5;
    case DTK_KEY_F6:          return GDK_KEY_F6;
    case DTK_KEY_F7:          return GDK_KEY_F7;
    case DTK_KEY_F8:          return GDK_KEY_F8;
    case DTK_KEY_F9:          return GDK_KEY_F9;
    case DTK_KEY_F10:         return GDK_KEY_F10;
    case DTK_KEY_F11:         return GDK_KEY_F11;
    case DTK_KEY_F12:         return GDK_KEY_F12;
    case DTK_KEY_SPACE:       return GDK_KEY_space;

    default: break;
    }

    if (key == '\t') {
        return GDK_KEY_Tab;
    }

    return (guint)key;
}
#endif  // DTK_GTK


dtk_uint32 dtk_get_mouse_button_modifier_flag(dtk_mouse_button button)
{
    if (button > DTK_MOUSE_BUTTON_5) {
        return 0;
    }

    dtk_uint32 table[] = {
        0,                                  // 0
        DTK_MODIFIER_MOUSE_BUTTON_LEFT,     // DTK_MOUSE_BUTTON_LEFT
        DTK_MODIFIER_MOUSE_BUTTON_RIGHT,    // DTK_MOUSE_BUTTON_RIGHT
        DTK_MODIFIER_MOUSE_BUTTON_MIDDLE,   // DTK_MOUSE_BUTTON_MIDDLE
        DTK_MODIFIER_MOUSE_BUTTON_4,        // DTK_MOUSE_BUTTON_4
        DTK_MODIFIER_MOUSE_BUTTON_5,        // DTK_MOUSE_BUTTON_5
    };

    return table[button];
}
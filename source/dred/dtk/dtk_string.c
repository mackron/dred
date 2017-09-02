// Copyright (C) 2017 David Reid. See included LICENSE file.

dtk_string dtk_make_string(const char* str)
{
    if (str == NULL) return NULL;
    
    size_t len = strlen(str);
    char* newStr = (char*)dtk_malloc(len+1);
    if (newStr == NULL) {
        return NULL;    // Out of memory.
    }

    dtk_strcpy_s(newStr, len+1, str);
    return newStr;
}

dtk_string dtk_make_stringv(const char* format, va_list args)
{
    if (format == NULL) format = "";

    va_list args2;
    va_copy(args2, args);

#if defined(_MSC_VER)
    int len = _vscprintf(format, args2);
#else
    int len = vsnprintf(NULL, 0, format, args2);
#endif

    va_end(args2);
    if (len < 0) {
        return NULL;
    }


    char* str = (char*)dtk_malloc(len+1);
    if (str == NULL) {
        return NULL;
    }

#if defined(_MSC_VER)
    len = vsprintf_s(str, len+1, format, args);
#else
    len = vsnprintf(str, len+1, format, args);
#endif

    return str;
}

dtk_string dtk_make_stringf(const char* format, ...)
{
    if (format == NULL) format = "";

    va_list args;
    va_start(args, format);

    char* str = dtk_make_stringv(format, args);

    va_end(args);
    return str;
}

dtk_string dtk_make_string_length(const char* str, size_t strLen)
{
    if (str == NULL) return NULL;
    
    char* newStr = (char*)dtk_malloc(strLen+1);
    if (newStr == NULL) {
        return NULL;    // Out of memory.
    }

    dtk_strncpy_s(newStr, strLen+1, str, strLen);
    return newStr;
}

dtk_string dtk_append_string(dtk_string lstr, const char* rstr)
{
    if (rstr == NULL) {
        rstr = "";
    }

    if (lstr == NULL) {
        return dtk_make_string(rstr);
    }

    size_t lstrLen = dtk_string_length(lstr);
    size_t rstrLen = strlen(rstr);
    char* str = (char*)dtk_realloc(lstr, lstrLen + rstrLen + 1);
    if (str == NULL) {
        return NULL;
    }

    memcpy(str + lstrLen, rstr, rstrLen);
    str[lstrLen + rstrLen] = '\0';

    return str;
}

dtk_string dtk_append_stringv(dtk_string lstr, const char* format, va_list args)
{
    dtk_string rstr = dtk_make_stringv(format, args);
    if (rstr == NULL) {
        return NULL;    // Probably out of memory.
    }

    char* str = dtk_append_string(lstr, rstr);

    dtk_free_string(rstr);
    return str;
}

dtk_string dtk_append_stringf(dtk_string lstr, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    char* str = dtk_append_stringv(lstr, format, args);

    va_end(args);
    return str;
}

dtk_string dtk_append_string_length(dtk_string lstr, const char* rstr, size_t rstrLen)
{
    if (rstr == NULL) {
        rstr = "";
    }

    if (lstr == NULL) {
        return dtk_make_string(rstr);
    }

    size_t lstrLen = dtk_string_length(lstr);
    char* str = (char*)dtk_realloc(lstr, lstrLen + rstrLen + 1);
    if (str == NULL) {
        return NULL;
    }

    dtk_strncat_s(str, lstrLen + rstrLen + 1, rstr, rstrLen);
    str[lstrLen + rstrLen] = '\0';

    return str;
}

size_t dtk_string_length(dtk_string str)
{
    return strlen(str);
}

void dtk_free_string(dtk_string str)
{
    dtk_free(str);
}
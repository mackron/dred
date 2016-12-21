// Copyright (C) 2016 David Reid. See included LICENSE file.

// Determines whether or not the given string is null or empty.
DTK_INLINE dtk_bool32 dtk_string_is_null_or_empty(const char* str)
{
    return str == NULL || str[0] == '\0';
}

#ifndef _MSC_VER
    #ifndef _TRUNCATE
    #define _TRUNCATE ((size_t)-1)
    #endif
#endif

DTK_INLINE int dtk_strcpy_s(char* dst, size_t dstSizeInBytes, const char* src)
{
#ifdef _MSC_VER
    return strcpy_s(dst, dstSizeInBytes, src);
#else
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    size_t i;
    for (i = 0; i < dstSizeInBytes && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (i < dstSizeInBytes) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return ERANGE;
#endif
}

DTK_INLINE int dtk_strncpy_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
{
#ifdef _MSC_VER
    return strncpy_s(dst, dstSizeInBytes, src, count);
#else
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return EINVAL;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    size_t maxcount = count;
    if (count == ((size_t)-1) || count >= dstSizeInBytes) {        // -1 = _TRUNCATE
        maxcount = dstSizeInBytes - 1;
    }

    size_t i;
    for (i = 0; i < maxcount && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (src[i] == '\0' || i == count || count == ((size_t)-1)) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return ERANGE;
#endif
}

DTK_INLINE int dtk_strcat_s(char* dst, size_t dstSizeInBytes, const char* src)
{
#ifdef _MSC_VER
    return strcat_s(dst, dstSizeInBytes, src);
#else
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    char* dstorig = dst;

    while (dstSizeInBytes > 0 && dst[0] != '\0') {
        dst += 1;
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes == 0) {
        return EINVAL;  // Unterminated.
    }


    while (dstSizeInBytes > 0 && src[0] != '\0') {
        *dst++ = *src++;
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes > 0) {
        dst[0] = '\0';
    } else {
        dstorig[0] = '\0';
        return ERANGE;
    }

    return 0;
#endif
}

DTK_INLINE int dtk_strncat_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
{
#ifdef _MSC_VER
    return strncat_s(dst, dstSizeInBytes, src, count);
#else
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        return EINVAL;
    }

    char* dstorig = dst;

    while (dstSizeInBytes > 0 && dst[0] != '\0') {
        dst += 1;
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes == 0) {
        return EINVAL;  // Unterminated.
    }


    if (count == ((size_t)-1)) {        // _TRUNCATE
        count = dstSizeInBytes - 1;
    }

    while (dstSizeInBytes > 0 && src[0] != '\0' && count > 0)
    {
        *dst++ = *src++;
        dstSizeInBytes -= 1;
        count -= 1;
    }

    if (dstSizeInBytes > 0) {
        dst[0] = '\0';
    } else {
        dstorig[0] = '\0';
        return ERANGE;
    }

    return 0;
#endif
}

DTK_INLINE int dtk_stricmp(const char* string1, const char* string2)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    return _stricmp(string1, string2);
#else
    return strcasecmp(string1, string2);
#endif
}


// Converts a UTF-16 character to UTF-32.
DTK_INLINE dtk_uint32 dtk_utf16_to_utf32_ch(dtk_uint16 utf16[2])
{
    if (utf16 == NULL) {
        return 0;
    }

    if (utf16[0] < 0xD800 || utf16[0] > 0xDFFF) {
        return utf16[0];
    } else {
        if ((utf16[0] & 0xFC00) == 0xD800 && (utf16[1] & 0xFC00) == 0xDC00) {
            return ((dtk_uint32)utf16[0] << 10) + utf16[1] - 0x35FDC00;
        } else {
            return 0;   // Invalid.
        }
    }
}

// Converts a UTF-16 surrogate pair to UTF-32.
DTK_INLINE dtk_uint32 dtk_utf16pair_to_utf32_ch(dtk_uint16 utf160, dtk_uint16 utf161)
{
    dtk_uint16 utf16[2];
    utf16[0] = utf160;
    utf16[1] = utf161;
    return dtk_utf16_to_utf32_ch(utf16);
}

// Converts a UTF-32 character to a UTF-16. Returns the number fo UTF-16 values making up the character.
DTK_INLINE dtk_uint32 dtk_utf32_to_utf16_ch(dtk_uint32 utf32, dtk_uint16 utf16[2])
{
    if (utf16 == NULL) {
        return 0;
    }

    if (utf32 < 0xD800 || (utf32 >= 0xE000 && utf32 <= 0xFFFF)) {
        utf16[0] = (dtk_uint16)utf32;
        utf16[1] = 0;
        return 1;
    } else {
        if (utf32 >= 0x10000 && utf32 <= 0x10FFFF) {
            utf16[0] = (dtk_uint16)(0xD7C0 + (dtk_uint16)(utf32 >> 10));
            utf16[1] = (dtk_uint16)(0xDC00 + (dtk_uint16)(utf32 & 0x3FF));
            return 2;
        } else {
            // Invalid.
            utf16[0] = 0;
            utf16[1] = 0;
            return 0;
        }
    }
}

// Converts a UTF-32 character to a UTF-8 character. Returns the number of bytes making up the UTF-8 character.
DTK_INLINE dtk_uint32 dtk_utf32_to_utf8_ch(dtk_uint32 utf32, char* utf8, size_t utf8Size)
{
    dtk_uint32 utf8ByteCount = 0;
    if (utf32 < 0x80) {
        utf8ByteCount = 1;
    } else if (utf32 < 0x800) {
        utf8ByteCount = 2;
    } else if (utf32 < 0x10000) {
        utf8ByteCount = 3;
    } else if (utf32 < 0x110000) {
        utf8ByteCount = 4;
    }

    if (utf8ByteCount > utf8Size) {
        if (utf8 != NULL && utf8Size > 0) {
            utf8[0] = '\0';
        }
        return 0;
    }

    utf8 += utf8ByteCount;
    if (utf8ByteCount < utf8Size) {
        utf8[0] = '\0'; // Null terminate.
    }

    const unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
    switch (utf8ByteCount)
    {
        case 4: *--utf8 = (char)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 3: *--utf8 = (char)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 2: *--utf8 = (char)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 1: *--utf8 = (char)(utf32 | firstByteMark[utf8ByteCount]);
        default: break;
    }

    return utf8ByteCount;
}


// Creates a formatted string. Free the string with dtk_free_string().
char* dtk_make_stringv(const char* format, va_list args);
char* dtk_make_stringf(const char* format, ...);

// Frees a string created by dtk_make_string*()
void dtk_free_string(char* str);
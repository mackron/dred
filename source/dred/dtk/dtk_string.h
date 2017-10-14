// Copyright (C) 2017 David Reid. See included LICENSE file.

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

DTK_INLINE char* dtk_strcpy(char* dst, const char* src)
{
    if (dst == NULL) return NULL;

    // If the source string is null, just pretend it's an empty string. I don't believe this is standard behaviour of strcpy(), but I prefer it.
    if (src == NULL) {
        src = "\0";
    }

#ifdef _MSC_VER
    while ((*dst++ = *src++));
    return dst;
#else
    return strcpy(dst, src);
#endif
}

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

DTK_INLINE int dtk_itoa_s(int value, char* dst, size_t dstSizeInBytes, int radix)
{
#ifdef _MSC_VER
    return _itoa_s(value, dst, dstSizeInBytes, radix);
#else
    if (dst == NULL || dstSizeInBytes == 0) {
        return EINVAL;
    }
    if (radix < 2 || radix > 36) {
        dst[0] = '\0';
        return EINVAL;
    }

    int sign = (value < 0 && radix == 10) ? -1 : 1;     // The negative sign is only used when the base is 10.

    unsigned int valueU;
    if (value < 0) {
        valueU = -value;
    } else {
        valueU = value;
    }

    char* dstEnd = dst;
    do
    {
        int remainder = valueU % radix;
        if (remainder > 9) {
            *dstEnd = (char)((remainder - 10) + 'a');
        } else {
            *dstEnd = (char)(remainder + '0');
        }

        dstEnd += 1;
        dstSizeInBytes -= 1;
        valueU /= radix;
    } while (dstSizeInBytes > 0 && valueU > 0);

    if (dstSizeInBytes == 0) {
        dst[0] = '\0';
        return EINVAL;  // Ran out of room in the output buffer.
    }

    if (sign < 0) {
        *dstEnd++ = '-';
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes == 0) {
        dst[0] = '\0';
        return EINVAL;  // Ran out of room in the output buffer.
    }

    *dstEnd = '\0';


    // At this point the string will be reversed.
    dstEnd -= 1;
    while (dst < dstEnd) {
        char temp = *dst;
        *dst = *dstEnd;
        *dstEnd = temp;

        dst += 1;
        dstEnd -= 1;
    }

    return 0;
#endif
}

DTK_INLINE size_t dtk_strcpy_len(char* dst, size_t dstSize, const char* src)
{
    if (dtk_strcpy_s(dst, dstSize, src) == 0) {
        return strlen(dst);
    }

    return 0;
}

DTK_INLINE int dtk_stricmp(const char* string1, const char* string2)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    return _stricmp(string1, string2);
#else
    return strcasecmp(string1, string2);
#endif
}


#ifndef _MSC_VER
DTK_INLINE int strcpy_s(char* dst, size_t dstSizeInBytes, const char* src)
{
    return dtk_strcpy_s(dst, dstSizeInBytes, src);
}

DTK_INLINE int strncpy_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
{
    return dtk_strncpy_s(dst, dstSizeInBytes, src, count);
}

DTK_INLINE int strcat_s(char* dst, size_t dstSizeInBytes, const char* src)
{
    return dtk_strcat_s(dst, dstSizeInBytes, src);
}

DTK_INLINE int strncat_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
{
    return dtk_strncat_s(dst, dstSizeInBytes, src, count);
}

#ifndef __MINGW32__
DTK_INLINE int _stricmp(const char* string1, const char* string2)
{
    return strcasecmp(string1, string2);
}
#endif

DTK_INLINE int _itoa_s(int value, char* dst, size_t dstSizeInBytes, int radix)
{
    return dtk_itoa_s(value, dst, dstSizeInBytes, radix);
}
#endif


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

DTK_INLINE dtk_bool32 dtk_is_whitespace(dtk_uint32 utf32)
{
    return utf32 == ' ' || utf32 == '\t' || utf32 == '\n' || utf32 == '\v' || utf32 == '\f' || utf32 == '\r';
}

DTK_INLINE const char* dtk_first_non_whitespace(const char* str)
{
    if (str == NULL) {
        return NULL;
    }

    while (str[0] != '\0' && !(str[0] != ' ' && str[0] != '\t' && str[0] != '\n' && str[0] != '\v' && str[0] != '\f' && str[0] != '\r')) {
        str += 1;
    }

    return str;
}

DTK_INLINE const char* dtk_first_whitespace(const char* str)
{
    if (str == NULL) {
        return NULL;
    }

    while (str[0] != '\0' && (str[0] != ' ' && str[0] != '\t' && str[0] != '\n' && str[0] != '\v' && str[0] != '\f' && str[0] != '\r')) {
        str += 1;
    }

    return str;
}

DTK_INLINE const char* dtk_ltrim(const char* str)
{
    if (str == NULL) {
        return NULL;
    }

    while (str[0] != '\0' && !(str[0] != ' ' && str[0] != '\t' && str[0] != '\n' && str[0] != '\v' && str[0] != '\f' && str[0] != '\r')) {
        str += 1;
    }

    return str;
}

DTK_INLINE const char* dtk_rtrim(const char* str)
{
    if (str == NULL) {
        return NULL;
    }

    const char* rstr = str;
    while (str[0] != '\0') {
        if (dtk_is_whitespace(str[0])) {
            str += 1;
            continue;
        }

        str += 1;
        rstr = str;
    }

    return rstr;
}

DTK_INLINE void dtk_trim(char* str)
{
    if (str == NULL) {
        return;
    }

    const char* lstr = dtk_ltrim(str);
    const char* rstr = dtk_rtrim(lstr);

    if (lstr > str) {
        memmove(str, lstr, rstr-lstr);
    }

    str[rstr-lstr] = '\0';
}


// Converts an ASCII hex character to it's integral equivalent. Returns false if it's not a valid hex character.
dtk_bool32 dtk_hex_char_to_uint(char ascii, unsigned int* out);


// Retrieves the first token in the given string.
//
// This function is suitable for doing a simple whitespace tokenization of a null-terminated string.
//
// The return value is a pointer to one character past the last character of the next token. You can use the return value to execute
// this function in a loop to parse an entire string.
//
// <tokenOut> can be null. If the buffer is too small to contain the entire token it will be set to an empty string. The original
// input string combined with the return value can be used to reliably find the token.
//
// This will handle double-quoted strings, so a string such as "My \"Complex String\"" contains two tokens: "My" and "\"Complex String\"".
//
// This function has no dependencies.
const char* dtk_next_token(const char* tokens, char* tokenOut, size_t tokenOutSize);

// Callbacks for dtk_parse_key_value_pairs().
typedef size_t (* dtk_key_value_read_proc) (void* pUserData, void* pDataOut, size_t bytesToRead);
typedef void   (* dtk_key_value_pair_proc) (void* pUserData, const char* key, const char* value);
typedef void   (* dtk_key_value_error_proc)(void* pUserData, const char* message, unsigned int line);

// Parses a series of simple Key/Value pairs.
//
// This function is suitable for parsing simple key/value config files.
//
// This function will never allocate memory on the heap. Because of this there is a minor restriction in the length of an individual
// key/value pair which is 4KB.
//
// Formatting rules are as follows:
//  - The basic syntax for a key/value pair is [key][whitespace][value]. Example: MyProperty 1234
//  - All key/value pairs must be declared on a single line, and a single line cannot contain more than a single key/value pair.
//  - Comments begin with the '#' character and continue until the end of the line.
//  - A key cannot contain spaces but are permitted in values.
//  - The value will have any leading and trailing whitespace trimmed.
//
// If an error occurs, that line will be skipped and processing will continue.
void dtk_parse_key_value_pairs(dtk_key_value_read_proc onRead, dtk_key_value_pair_proc onPair, dtk_key_value_error_proc onError, void* pUserData);

// This will only return DR_FALSE if the file fails to open. It will still return DR_TRUE even if there are syntax error or whatnot.
dtk_bool32 dtk_parse_key_value_pairs_from_file(const char* filePath, dtk_key_value_pair_proc onPair, dtk_key_value_error_proc onError, void* pUserData);


//// dtk_string ////

typedef char* dtk_string;

// Allocates the memory for a string, including the null terminator.
//
// Use this API if you want to allocate memory for the string, but you want to fill it with raw data yourself.
dtk_string dtk_malloc_string(size_t sizeInBytesIncludingNullTerminator);

// Creates a newly allocated string. Free the string with dtk_free_string().
dtk_string dtk_make_string(const char* str);

// Creates a formatted string. Free the string with dtk_free_string().
dtk_string dtk_make_stringv(const char* format, va_list args);
dtk_string dtk_make_stringf(const char* format, ...);

// Creates a newly allocated string, restricting it to a maximum length.
dtk_string dtk_make_string_length(const char* str, size_t strLen);

// Appends a string to another dtk_string.
//
// This free's "lstr". Use this API like so: "lstr = dtk_append_string(lstr, rstr)". It works the same way as realloc().
//
// Use dtk_make_stringf("%s%s", str1, str2) to append to C-style strings together. An optimized solution for this may be implemented in the future.
dtk_string dtk_append_string(dtk_string lstr, const char* rstr);

// Appends a formatted string to another dtk_string.
dtk_string dtk_append_stringv(dtk_string lstr, const char* format, va_list args);
dtk_string dtk_append_stringf(dtk_string lstr, const char* format, ...);

// Same as dtk_append_string(), except restricts it to a maximum number of characters and does not require the input
// string to be null terminated.
dtk_string dtk_append_string_length(dtk_string lstr, const char* rstr, size_t rstrLen);

// Retrieves the length of the given string.
size_t dtk_string_length(dtk_string str);


// Frees a string created by dtk_make_string*()
void dtk_free_string(dtk_string str);
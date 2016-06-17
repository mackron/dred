// Text layout and editing engine. Public Domain. See "unlicense" statement at the end of this file.
// dr_text_engine - v0.0 - unreleased
//
// David Reid - mackron@gmail.com

// REQUIREMENTS
//
// - Full UTF-8, UTF-16 (LE and BE) and UTF-32 support
//   - UTF-8 internally, high-level conversion routines at a higher level for UTF-16 and UTF-32.
// - Syntax highlighting
//   - Must be able to plug-in custom highlighters.
// - Lexing
//   - This will be used to identify words for doing word-based movement and finding the word under a point.
// - Efficient editing of huge files
//   - Reallocating and moving around a monolithic buffer is way too slow.
//   - Need a way to easily split the file into chunks.
//   - Need to easily pick out individual characters by their index.
//   - Need a very simple high-level API while still maintaining good performance characteristics.
//     - Have a dr_text_buffer() object?
//       - dr_text_buffer_insert_character(pTextBuffer, size_t index, uint32_t utf32);
//       - dr_text_buffer_remove_character(pTextBuffer, size_t index);
// - Rendering callbacks
//   - The text engine will not do the actual rendering, but instead notify the application.
// - Notifications for when a region have become invalid and needs redrawing
// - Line numbers
// - Underlines
// - Getting words under a specific point in the text region.
//   - Useful for right-click context menus and whatnot (go to definition, etc.)
// - Multi-line editing and selection
//   - Box selection, just like MSVC's ALT+LMB selection.
// - Carets
//   - Block caret for insert mode.
// - Overwrite mode
// - Inserting and removing text and any position, not just where the cursor is placed.
//   - All editing should be done like this.
//   - Will enable multi-line editing.

// PAST ISSUES
// - Selection mode became as issue where it would often get "stuck" due to incorrectly enabling/disabling selection mode. Might want
//   to look into a more robust solution that avoids this problem at a fundamental level. That state flags in mouse events helps a lot.

#ifndef dr_text_engine_h
#define dr_text_engine_h

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int unused;
} drte_buffer;

typedef struct
{
    drte_buffer buffer;
} drte_engine;


// Initializes a text engine.
//
// This should be the first function called for any given text engine. Use drte_engine_uninit() to uninitialize the engine.
bool drte_engine_init(drte_engine* pEngine);

// Unitializes a text engine.
void drte_engine_uninit(drte_engine* pEngine);



#ifdef __cplusplus
}
#endif
#endif //dr_text_engine_h


///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DR_TEXT_ENGINE_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>

bool drte_buffer_init(drte_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return false;
    }

    return true;
}

void drte_buffer_uninit(drte_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return;
    }
}




bool drte_engine_init(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    memset(pEngine, 0, sizeof(*pEngine));

    if (!drte_buffer_init(&pEngine->buffer)) {
        return false;
    }

    return true;
}

void drte_engine_uninit(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    drte_buffer_uninit(&pEngine->buffer);
}
#endif  //DR_TEXT_ENGINE_IMPLEMENTATION


/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

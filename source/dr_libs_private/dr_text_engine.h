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
// - Word wrap

// PAST ISSUES
// - Selection mode became as issue where it would often get "stuck" due to incorrectly enabling/disabling selection mode. Might want
//   to look into a more robust solution that avoids this problem at a fundamental level. That state flags in mouse events helps a lot.


// BRAINSTORMING
//
// - The text engine does not load fonts or care about colors. Instead it only cares about "styles". Different styles are identified by
//   a simple abstract integer, the meaning of which is determined by the application at a higher level.
// - When a text engine requires information about a style, such as the metrics of the font of a style, it will request it. The efficiency
//   of this is directly affected by the efficiency at which the host application delivers that data.


// USAGE
//
// dr_text_engine is a single-file library. To use it, do something like the following in one .c file.
//   #define DR_TEXT_ENGINE_IMPLEMENTATION
//   #include "dr_text_engine.h"
//
// You can then #include this file in other parts of the program as you would with any other header file.
//
//
// QUICK NOTES
// - The text engine is optimized for top/left alignment. Other alignments will run on a slightly slower, more generic path.


#ifndef dr_text_engine_h
#define dr_text_engine_h

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define DRTE_INLINE static __forceinline
#else
#define DRTE_INLINE static inline
#endif

#define DRTE_SHOW_CURSOR    (1 << 0)
#define DRTE_UNDO_ENABLED   (1 << 1)

typedef uintptr_t drte_style_id;
typedef struct drte_rect drte_rect;
typedef struct drte_font_metrics drte_font_metrics;
typedef struct drte_run drte_run;
typedef struct drte_page drte_page;
typedef struct drte_engine drte_engine;

typedef void (* drte_on_dirty_proc)(drte_engine* pEngine, drte_rect rect);
typedef void (* drte_on_paint_rect_proc)(drte_engine* pEngine, drte_rect rect, drte_style_id styleID, void* pUserData);

typedef enum
{
    drte_alignment_left,
    drte_alignment_top,
    drte_alignment_center,
    drte_alignment_right,
    drte_alignment_bottom
} drte_alignment;

struct drte_font_metrics
{
    int ascent;
    int descent;
    int lineHeight;
    int spaceWidth;
};


struct drte_rect
{
    float left;
    float top;
    float right;
    float bottom;
};

DRTE_INLINE drte_rect drte_make_rect(float left, float top, float right, float bottom)
{
    drte_rect rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;

    return rect;
}

DRTE_INLINE drte_rect drte_make_inside_out_rect()
{
    drte_rect rect;
    rect.left   =  FLT_MAX;
    rect.top    =  FLT_MAX;
    rect.right  = -FLT_MAX;
    rect.bottom = -FLT_MAX;

    return rect;
}

DRTE_INLINE drte_rect drte_rect_union(drte_rect rect0, drte_rect rect1)
{
    drte_rect result;
    result.left   = (rect0.left   < rect1.left)   ? rect0.left   : rect1.left;
    result.top    = (rect0.top    < rect1.top)    ? rect0.top    : rect1.top;
    result.right  = (rect0.right  > rect1.right)  ? rect0.right  : rect1.right;
    result.bottom = (rect0.bottom > rect1.bottom) ? rect0.bottom : rect1.bottom;

    return result;
}

DRTE_INLINE drte_rect drte_rect_clamp(drte_rect rect, drte_rect other)
{
    drte_rect result;
    result.left   = (rect.left   >= other.left)   ? rect.left   : other.left;
    result.top    = (rect.top    >= other.top)    ? rect.top    : other.top;
    result.right  = (rect.right  <= other.right)  ? rect.right  : other.right;
    result.bottom = (rect.bottom <= other.bottom) ? rect.bottom : other.bottom;

    return result;
}

DRTE_INLINE bool drte_rect_has_volume(drte_rect rect)
{
    return rect.right > rect.left && rect.bottom > rect.top;
}


struct drte_run
{
    // The index of the first character in the run (11 bits), and it's length (5 bits).
    uint16_t charPosAndLen;

    // The slot of the style to apply to this run.
    uint8_t styleSlot;

    // Padding for future use.
    uint8_t padding;
};

struct drte_page
{
    // A pointer to the runs making up the page.
    drte_run pRuns[64];

    // The number of runs in the page.
    uint8_t runCount;

    // The content of the page. This is is null terminated.
    char text[2048];
};

struct drte_engine
{
    // The buffer containing the raw text data. This does not store any undo/redo data.
    //drte_buffer buffer;

    // The size of the bounds.
    float boundsSizeX;
    float boundsSizeY;

    // The inner offset. This is how scrolling is implemented.
    float innerOffsetX;
    float innerOffsetY;

    // The horizontal alignment.
    drte_alignment alignmentHorz;
    drte_alignment alignmentVert;

    // Boolean flags.
    //   DRTE_SHOW_CURSOR
    //   DRTE_UNDO_ENABLED
    uint32_t flags;


    // The dirty counter. This is used to determine whether or not the text engine is having invalidation calls batched.
    unsigned int dirtyCounter;

    // The accumulated dirty rectangle. Only used when batching invalidations. Is initialized to an inside out rectangle
    // and accumulated with drte_rect_union(). At the end of each batch it is reset back to an inside out rectangle.
    drte_rect dirtyRect;


    // Styles.
    struct
    {
        drte_style_id background;
    } style;


    // The function to call when a region of the bounds has been marked as dirty. The implementation of this function will
    // typically want to redraw the engine with drte_engine_paint().
    drte_on_dirty_proc onDirty;

    // The function to call when a rectangle needs to be painted.
    drte_on_paint_rect_proc onPaintRect;


    // Application-defined data.
    void* pUserData;
};


// Initializes a text engine.
//
// This should be the first function called for any given text engine. Use drte_engine_uninit() to uninitialize the engine.
bool drte_engine_init(drte_engine* pEngine);

// Unitializes a text engine.
void drte_engine_uninit(drte_engine* pEngine);


// Sets the bounds of the text engine.
//
// This will mark the engine as dirty which will result in a redraw.
void drte_engine_set_bounds(drte_engine* pEngine, float boundsSizeX, float boundsSizeY);

// Helper for retrieving the local rectangle of the bounds.
//
// The returned rectangle will have it's left and top properties set to 0, and right and bottom set to the bounds width and height.
drte_rect drte_engine_get_bounds_rect(drte_engine* pEngine);


// Marks the beginning of a batched invalidation.
void drte_engine_begin_dirty(drte_engine* pEngine);

// Marks the end of a batched invalidation.
void drte_engine_end_dirty(drte_engine* pEngine);

// Marks a region of the text engine as dirty.
//
// Because marking a region as dirty can potentially result in an immediate redraw, consider batching these by wrapping them in
// drte_engine_begin_dirty() and drte_engine_end_dirty() calls.
void drte_engine_dirty(drte_engine* pEngine, drte_rect rect);

// Paints the text engine.
//
// The specified rectangle should be relative to the bounds.
void drte_engine_paint(drte_engine* pEngine, drte_rect rect, void* pUserData);


// Sets the text of the text engine.
void drte_engine_set_text(drte_engine* pEngine, const char* text);



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


///////////////////////////////////////////////////////////////////////////////
//
// General String Processing
//
///////////////////////////////////////////////////////////////////////////////

size_t drte_strlen(const char* str)
{
    return strlen(str);
}

int drte_strcpy(char* dst, size_t dstSizeInBytes, const char* src)
{
    // TODO: strcpy_s() is not available on Linux. Implement this manually in that case.
    return strcpy_s(dst, dstSizeInBytes, src);
}




///////////////////////////////////////////////////////////////////////////////
//
// drte_engine
//
///////////////////////////////////////////////////////////////////////////////

bool drte_engine_init(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    memset(pEngine, 0, sizeof(*pEngine));

    pEngine->alignmentHorz = drte_alignment_left;
    pEngine->alignmentVert = drte_alignment_top;
    pEngine->dirtyRect = drte_make_inside_out_rect();

    return true;
}

void drte_engine_uninit(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }
}


void drte_engine_set_bounds(drte_engine* pEngine, float boundsSizeX, float boundsSizeY)
{
    if (pEngine == NULL) {
        return;
    }

    drte_engine_begin_dirty(pEngine);
    {
        float rectXSize = boundsSizeX - pEngine->boundsSizeX;
        float rectYSize = boundsSizeY - pEngine->boundsSizeY;
    
        if (rectXSize > 0) {
            drte_engine_dirty(pEngine, drte_make_rect(pEngine->boundsSizeX, 0, boundsSizeX, boundsSizeY));
        }
        if (rectYSize > 0) {
            drte_engine_dirty(pEngine, drte_make_rect(0, pEngine->boundsSizeY, boundsSizeX, boundsSizeY));
        }

        pEngine->boundsSizeX = boundsSizeX;
        pEngine->boundsSizeY = boundsSizeY;

        // TODO: Alignment needs to be updated if it's not top/left.
    }
    drte_engine_end_dirty(pEngine);
}

drte_rect drte_engine_get_bounds_rect(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return drte_make_rect(0, 0, 0, 0);
    }

    return drte_make_rect(0, 0, pEngine->boundsSizeX, pEngine->boundsSizeY);
}

void drte_engine_begin_dirty(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->dirtyCounter += 1;
}

void drte_engine_end_dirty(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    assert(pEngine->dirtyCounter > 0);  // <-- If you've triggered this you have a bug. Fix it. Make sure every call to drte_engine_begin_dirty() is matched with exactly one call to drte_engine_end_dirty().

    pEngine->dirtyCounter -= 1;
    if (pEngine->dirtyCounter == 0) {
        drte_engine_dirty(pEngine, pEngine->dirtyRect);
    }
}

void drte_engine_dirty(drte_engine* pEngine, drte_rect rect)
{
    if (pEngine == NULL || !drte_rect_has_volume(rect)) {
        return;
    }

    if (pEngine->dirtyCounter > 0) {
        pEngine->dirtyRect = drte_rect_union(pEngine->dirtyRect, rect);
    } else {
        if (pEngine->onDirty) {
            pEngine->onDirty(pEngine, rect);
        }
    }
}

void drte_engine_paint(drte_engine* pEngine, drte_rect rect, void* pUserData)
{
    if (pEngine == NULL || pEngine->onPaintRect == NULL) {
        return;
    }

    // Make sure the rectangle is clamped to the bounds.
    rect = drte_rect_clamp(rect, drte_engine_get_bounds_rect(pEngine));

    for (drte_run* pRun = drte_engine_first_visible_run(pEngine, rect); pRun != NULL; pRun = drte_engine_next_visible_run(pEngine, rect, pRun)) {
        drte_engine_paint_run(pEngine, pRun);
    }

    // TEMP: Paint a rectangle over the entire bounds.
    (void)rect;
    pEngine->onPaintRect(pEngine, drte_make_rect(0, 0, pEngine->boundsSizeX, pEngine->boundsSizeY), pEngine->style.background, pUserData);
}


void drte_engine_set_text(drte_engine* pEngine, const char* text)
{
    if (pEngine == NULL) {
        return;
    }

    if (text == NULL) {
        text = "";
    }


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

// Text layout and editing engine. Public Domain. See "unlicense" statement at the end of this file.
// dr_text_engine - v0.0 - unreleased
//
// David Reid - mackron@gmail.com

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
// - The text engine is optimized for top/left alignment.


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

typedef struct drte_engine drte_engine;
typedef struct drte_view drte_view;
typedef uintptr_t drte_style_token;

typedef enum
{
    drte_alignment_left,
    drte_alignment_top,
    drte_alignment_center,
    drte_alignment_right,
    drte_alignment_bottom,
} drte_alignment;

typedef struct
{
    int16_t ascent;
    int16_t descent;
    int16_t lineHeight;
    int16_t spaceWidth;
} drte_font_metrics;

static drte_font_metrics drte_font_metrics_create(int ascent, int descent, int lineHeight, int spaceWidth)
{
    drte_font_metrics metrics;
    metrics.ascent = (int16_t)ascent;
    metrics.descent = (int16_t)descent;
    metrics.lineHeight = (int16_t)lineHeight;
    metrics.spaceWidth = (int16_t)spaceWidth;
    return metrics;
}

typedef struct
{
    drte_style_token styleToken;
    drte_font_metrics fontMetrics;
} drte_style;


typedef struct 
{
    float left;
    float top;
    float right;
    float bottom;
} drte_rect;


typedef void   (* drte_engine_on_measure_string_proc)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textLength, float* pWidthOut, float* pHeightOut);
typedef void   (* drte_engine_on_get_cursor_position_from_point_proc)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut);
typedef void   (* drte_engine_on_get_cursor_position_from_char_proc)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t characterIndex, float* pTextCursorPosXOut);
typedef bool   (* drte_engine_on_get_next_highlight_proc)(drte_engine* pEngine, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut, drte_style_token* pStyleTokenOut, void* pUserData);

typedef void   (* drte_engine_on_paint_text_proc)        (drte_engine* pEngine, drte_view* pView, drte_style_token styleTokenFG, drte_style_token styleTokenBG, const char* text, size_t textLength, float posX, float posY, void* pPaintData);
typedef void   (* drte_engine_on_paint_rect_proc)        (drte_engine* pEngine, drte_view* pView, drte_style_token styleToken, drte_rect rect, void* pPaintData);
typedef void   (* drte_engine_on_cursor_move_proc)       (drte_engine* pEngine, drte_view* pView, size_t iCursor);
typedef void   (* drte_engine_on_dirty_proc)             (drte_engine* pEngine, drte_view* pView, drte_rect rect);
typedef void   (* drte_engine_on_text_changed_proc)      (drte_engine* pEngine);
typedef void   (* drte_engine_on_undo_point_changed_proc)(drte_engine* pEngine, unsigned int iUndoPoint);
typedef size_t (* drte_engine_on_get_undo_state_proc)    (drte_engine* pEngine, void* pDataOut);
typedef void   (* drte_engine_on_apply_undo_state_proc)  (drte_engine* pEngine, size_t dataSize, const void* pData);
typedef void   (* drte_engine_on_undo_stack_trimmed_proc)(drte_engine* pEngine);

typedef struct
{
    // The index of the character the marker is positioned at. This will be to the left of the character.
    size_t iCharAbs;

    // The index of the line the cursor is positioned on.
    size_t iLine;

    // The absolute position on the x axis to place the marker when moving up and down lines. This is relative to
    // the left position of the line. This will be updated when the marker is moved left and right.
    float absoluteSickyPosX;

} drte_cursor;

typedef struct
{
    size_t iCharBeg;
    size_t iCharEnd;
} drte_region;

typedef struct
{
    // The index of the first character in the segment.
    size_t iCharBeg;

    // The index of the last character in the segment.
    size_t iCharEnd;

    // The slot of the style to apply to this segment.
    uint8_t styleSlot;

} drte_style_segment;


// Used internally for implementing the undo/redo stack.
typedef struct
{
    size_t stackPtr;
    size_t bufferSize;
    void* pBuffer;
} drte_stack_buffer;

typedef enum
{
	drte_undo_change_type_insert,
	drte_undo_change_type_delete
} drte_undo_change_type;

typedef struct
{
	drte_undo_change_type type;
	size_t iCharBeg;
	size_t iCharEnd;
	size_t textOffset;
} drte_undo_change;


// Used internally for caching lines. APIs for working on line caches are private.
typedef struct
{
	size_t* pLines;
	size_t bufferSize;
	size_t count;
} drte_line_cache;

struct drte_view
{
    // A pointer to the engine that owns this view.
    drte_engine* pEngine;

    // The size of the container of the view.
    float sizeX;
    float sizeY;

    // The inner offset of the view. This is used for doing scrolling.
    float innerOffsetX;
    float innerOffsetY;

    // Boolean flags.
    unsigned int flags;

    // The size of a tab in spaces.
    unsigned int tabSizeInSpaces;


    // The width of the text cursor.
    float cursorWidth;

    // The list of active cursors.
    drte_cursor* pCursors;

    // The number of active cursors.
    size_t cursorCount;


    // The list of selection regions.
    drte_region* pSelections;

    // The number of active selection regions. When this is 0, nothing is selected.
    size_t selectionCount;


    // Application defined data.
    void* pUserData;


    //
    // Internal
    //
    size_t _id;   // <-- This is used for identifying the view when restoring undo/redo state.
    drte_view* _pPrevView;
    drte_view* _pNextView;
    unsigned int _dirtyCounter;
    drte_rect _accumulatedDirtyRect;
    drte_line_cache _wrappedLines;
    drte_line_cache* pWrappedLines;     // Points to _wrappedLines if word wrap is enabled; points to pEngine->_unwrappedLines when word wrap is disabled.
};

struct drte_engine
{
    // The list of registered styles. There is a maximum of 256 styles.
    drte_style styles[256];

    // The number of registered styles.
    uint16_t styleCount;

    // The default style.
    uint8_t defaultStyleSlot;

    // The style to apply to selected text. Only the background color is used.
    uint8_t selectionStyleSlot;

    // The style to apply to active lines. Only the background color is used.
    uint8_t activeLineStyleSlot;

    // The style to apply to the cursor.
    uint8_t cursorStyleSlot;

    // The style to apply to line numbers.
    uint8_t lineNumbersStyleSlot;

    // The height of each line. This is set to the maximum line height of every registered style, or set explicitly if the
    // DRTE_USE_EXPLICIT_LINE_HEIGHT flag is set.
    float lineHeight;


    // Flags.
    //   DRTE_USE_EXPLICIT_LINE_HEIGHT
    uint8_t flags;


    // The function to call when a string needs to be measured.
    drte_engine_on_measure_string_proc onMeasureString;

    // The function to call when the position of the cursor needs to be retrieved based on a pixel position within that string.
    drte_engine_on_get_cursor_position_from_point_proc onGetCursorPositionFromPoint;

    // The function to call when the position of the cursor needs to be retrieved based on a character at a specific index.
    drte_engine_on_get_cursor_position_from_char_proc onGetCursorPositionFromChar;



    // This unusual construct is to make handling word wrapping easier. One line cache is used for storing information about raw,
    // unwrapped lines. The other is for storing information about wrapped lines. When word wrap is not being used, we optimize things
    // by having the wrapped line cache and unwrapped line cache point to the same underlying line cache to save on memory.

    // The line caches.
    drte_line_cache _unwrappedLines;
    drte_line_cache* pUnwrappedLines;   // Always points to _unwrappedLines. Exists only for consistency with pWrappedLines.



    // The function to call for handling syntax highlighting. See documentation for drte_engine_set_highlighter() for information
    // on how this function is used.
    drte_engine_on_get_next_highlight_proc onGetNextHighlight;

    // The user data to pass to each call to onGetNextHighlight.
    void* pHighlightUserData;


    /// The main text of the layout.
    char* text;

    /// The length of the text.
    size_t textLength;


    /// The function to call when the text engine needs to be redrawn.
    drte_engine_on_dirty_proc onDirty;

    /// The function to call when the content of the text engine changes.
    drte_engine_on_text_changed_proc onTextChanged;

    /// The function to call when the current undo point has changed.
    drte_engine_on_undo_point_changed_proc onUndoPointChanged;

    // The function to call when application-defined data needs to be saved and restored for undo/redo points.
    drte_engine_on_get_undo_state_proc onGetUndoState;

    // The function to call when application-defined data needs to be applied for undo/redo points.
    drte_engine_on_apply_undo_state_proc onApplyUndoState;

    // The function to call when the undo stack has been trimmed.
    drte_engine_on_undo_stack_trimmed_proc onUndoStackTrimmed;


    /// The blink rate in milliseconds of the cursor.
    unsigned int cursorBlinkRate;

    /// The amount of time in milliseconds to toggle the cursor's blink state.
    unsigned int timeToNextCursorBlink;

    /// Whether or not the cursor is showing based on it's blinking state.
    bool isCursorBlinkOn;


    /// The function to call when a text run needs to be painted.
    drte_engine_on_paint_text_proc onPaintText;

    /// The function to call when a rectangle needs to be painted.
    drte_engine_on_paint_rect_proc onPaintRect;

    /// The function to call when the cursor moves.
    drte_engine_on_cursor_move_proc onCursorMove;


    /// The number of items in the undo/redo stack.
    unsigned int undoStackCount;

    /// The index of the undo/redo state item we are currently sitting on.
    unsigned int iUndoState;


    // Whether or not there is a prepared undo state.
    bool hasPreparedUndoState;

    // The number of textual changes in the prepared undo state.
    size_t preparedUndoTextChangeCount;

    // The local offset of the textual changes in the prepared undo state.
    size_t preparedUndoTextChangesOffset;

    // The buffer containing the raw data of the prepared undo state.
    drte_stack_buffer preparedUndoState;

    // The main undo/redo buffer, implemented on a stack-based allocation scheme.
    drte_stack_buffer undoBuffer;

    // The offset in the main undo buffer of the first byte of the current undo point.
    size_t currentUndoDataOffset;
    size_t currentRedoDataOffset;


    // The ID to use for the next view. This is used for identifying views when restoring undo/redo state.
    size_t nextViewID;

    // A pointer to the first view. This is a linked list.
    drte_view* pRootView;


    // Application-defined data.
    void* pUserData;
};


/// Creates a new text engine object.
bool drte_engine_init(drte_engine* pEngine, void* pUserData);

/// Deletes the given text engine.
void drte_engine_uninit(drte_engine* pEngine);


// Retrieves a pointer to the first view.
//
// Use this with drte_engine_next_view() to iterate over each view.
DRTE_INLINE drte_view* drte_engine_first_view(drte_engine* pEngine) { if (pEngine == NULL) return NULL; return pEngine->pRootView; }

// Retrieves a pointer to the next view.
DRTE_INLINE drte_view* drte_engine_next_view(drte_engine* pEngine, drte_view* pView) { (void)pEngine; if (pView == NULL) return NULL; return pView->_pNextView; }
DRTE_INLINE drte_view* drte_engine_prev_view(drte_engine* pEngine, drte_view* pView) { (void)pEngine; if (pView == NULL) return NULL; return pView->_pPrevView; }

// Counts the number of views. This runs in linear time.
size_t drte_engine_get_view_count(drte_engine* pEngine);


// Registers a style token.
//
// There is a maximum of 255 style tokens that can be registered at any given time.
//
// The style token can be anything you would like. It's is an integer the size of a pointer, so it is possible to use a direct pointer for tokens.
//
// If the underlying style of the token changes, simply call this function again to force a refresh of the text engine.
bool drte_engine_register_style_token(drte_engine* pEngine, drte_style_token styleToken, drte_font_metrics fontMetrics);

// Sets the default style to use for text.
//
// This style is used for any text segments that have not had a style explicitly set. It is also used for drawing the background
// regions where there is no text.
//
// The given style must have been registered first with drte_engine_register_style_token().
void drte_engine_set_default_style(drte_engine* pEngine, drte_style_token styleToken);

// Sets the style to use for selected text.
//
// Only the background color is used for this. The text is drawn with the standard non-selected style.
void drte_engine_set_selection_style(drte_engine* pEngine, drte_style_token styleToken);

// Sets the style to use for active lines.
//
// Only the background color is used for this. The text is drawn with it's normal.
void drte_engine_set_active_line_style(drte_engine* pEngine, drte_style_token styleToken);

// Sets the style to use for the cursor.
void drte_engine_set_cursor_style(drte_engine* pEngine, drte_style_token styleToken);

// Sets the style to use for the line numbers.
void drte_engine_set_line_numbers_style(drte_engine* pEngine, drte_style_token styleToken);


// Registers a highlighter.
//
// When the text engine is being painted, it needs information about how to style the text. To do this the engine will refer back to the
// application through the use of a callback function. The engine will pass a character index to this callback and the callback will return
// the range of characters (if any) that come come after that character which require highlighting.
//
// TODO: This isn't clear - clarify.
void drte_engine_set_highlighter(drte_engine* pEngine, drte_engine_on_get_next_highlight_proc proc, void* pUserData);


// Explicitly sets the line height. Set this to 0 to use the line height based off the registered styles.
void drte_engine_set_line_height(drte_engine* pEngine, float lineHeight);

// Retrieves the line height.
float drte_engine_get_line_height(drte_engine* pEngine);


// Gets the character at the given index as a UTF-32 code point.
uint32_t drte_engine_get_utf32(drte_engine* pEngine, size_t characterIndex);


/// Sets the given text engine's text.
void drte_engine_set_text(drte_engine* pEngine, const char* text);

/// Retrieves the given text engine's text.
///
/// @return The length of the string, not including the null terminator.
///
/// @remarks
///     Call this function with <textOut> set to NULL to retieve the required size of <textOut>.
size_t drte_engine_get_text(drte_engine* pEngine, char* textOut, size_t textOutSize);


/// Sets the function to call when a region of the text engine needs to be redrawn.
void drte_engine_set_on_dirty(drte_engine* pEngine, drte_engine_on_dirty_proc proc);

/// Sets the function to call when the content of the given text engine has changed.
void drte_engine_set_on_text_changed(drte_engine* pEngine, drte_engine_on_text_changed_proc proc);

/// Sets the function to call when the content of the given text engine's current undo point has moved.
void drte_engine_set_on_undo_point_changed(drte_engine* pEngine, drte_engine_on_undo_point_changed_proc proc);

/// Sets the function to call when the cursor in the given text engine is mvoed.
void drte_engine_set_on_cursor_move(drte_engine* pEngine, drte_engine_on_cursor_move_proc proc);


/// Sets the blink rate of the cursor in milliseconds.
void drte_engine_set_cursor_blink_rate(drte_engine* pEngine, unsigned int blinkRateInMilliseconds);

/// Retrieves the blink rate of the cursor in milliseconds.
unsigned int drte_engine_get_cursor_blink_rate(drte_engine* pEngine);

// Resets the cursors blink state.
void drte_engine_reset_cursor_blinks(drte_engine* pEngine);



/// Inserts a character into the given text engine.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_insert_character(drte_engine* pEngine, size_t insertIndex, uint32_t utf32);

// Deletes the character at the given index. Returns true if the text was changed.
bool drte_engine_delete_character(drte_engine* pEngine, size_t iChar);

/// Inserts the given string at the given character index.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_insert_text(drte_engine* pEngine, const char* text, size_t insertIndex);

/// Deletes a range of text in the given text engine.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_delete_text(drte_engine* pEngine, size_t iFirstCh, size_t iLastChPlus1);


// Retrieves the start of the next word starting from the given character.
bool drte_engine_get_start_of_next_word_from_character(drte_engine* pEngine, size_t iChar, size_t* pWordBegOut);

// Retrieves the last character of the word beginning with a character which can be at any position within said word.
bool drte_engine_get_end_of_word_containing_character(drte_engine* pEngine, size_t iChar, size_t* pWordEndOut);

// Retrieves the word containing the given character.
bool drte_engine_get_word_containing_character(drte_engine* pEngine, size_t iChar, size_t* pWordBegOut, size_t* pWordEndOut);


/// Prepares the next undo/redo point.
///
/// @remarks
///     This captures the state that will be applied when the undo/redo point is undone.
bool drte_engine_prepare_undo_point(drte_engine* pEngine);

/// Creates a snapshot of the current state of the text engine and pushes it to the top of the undo/redo stack.
bool drte_engine_commit_undo_point(drte_engine* pEngine);

/// Performs an undo operation.
bool drte_engine_undo(drte_engine* pEngine);

/// Performs a redo operation.
bool drte_engine_redo(drte_engine* pEngine);

/// Retrieves the number of undo points remaining in the stack.
unsigned int drte_engine_get_undo_points_remaining_count(drte_engine* pEngine);

/// Retrieves the number of redo points remaining in the stack.
unsigned int drte_engine_get_redo_points_remaining_count(drte_engine* pEngine);

/// Clears the undo stack.
void drte_engine_clear_undo_stack(drte_engine* pEngine);


/// Sets the function to call when a run of text needs to be painted for the given text engine.
void drte_engine_set_on_paint_text(drte_engine* pEngine, drte_engine_on_paint_text_proc proc);

/// Sets the function to call when a quad needs to the be painted for the given text engine.
void drte_engine_set_on_paint_rect(drte_engine* pEngine, drte_engine_on_paint_rect_proc proc);


/// Steps the given text engine by the given number of milliseconds.
///
/// @remarks
///     This will trigger the on_dirty callback when the cursor switches it's blink states.
void drte_engine_step(drte_engine* pEngine, unsigned int milliseconds);




///////////////////////////////////////////////////////////////////////////////
//
// Views
//
///////////////////////////////////////////////////////////////////////////////

// Creates a view for the given text engine.
drte_view* drte_view_create(drte_engine* pEngine);

// Deletes the given view.
void drte_view_delete(drte_view* pView);


// Retrieves a pointer to the neighboring view in the linked list. Use this for iteration.
DRTE_INLINE drte_view* drte_view_next_view(drte_view* pView) { if (pView == NULL) return NULL; return pView->_pNextView; }
DRTE_INLINE drte_view* drte_view_prev_view(drte_view* pView) { if (pView == NULL) return NULL; return pView->_pPrevView; }


// Sets the size of the container of the view.
void drte_view_set_size(drte_view* pView, float sizeX, float sizeY);

// Retrieves the size of the view.
void drte_view_get_size(drte_view* pView, float* pSizeXOut, float* pSizeYOut);
DRTE_INLINE float drte_view_get_size_x(drte_view* pView) { if (pView == NULL) return 0; return pView->sizeX; }
DRTE_INLINE float drte_view_get_size_y(drte_view* pView) { if (pView == NULL) return 0; return pView->sizeY; }


// Sets the inner offset of the view.
void drte_view_set_inner_offset(drte_view* pView, float innerOffsetX, float innerOffsetY);
DRTE_INLINE void drte_view_set_inner_offset_x(drte_view* pView, float innerOffsetX) { if (pView == NULL) return; drte_view_set_inner_offset(pView, innerOffsetX, pView->innerOffsetY); }
DRTE_INLINE void drte_view_set_inner_offset_y(drte_view* pView, float innerOffsetY) { if (pView == NULL) return; drte_view_set_inner_offset(pView, pView->innerOffsetX, innerOffsetY); }

// Retrieves the inner offset of the view.
void drte_view_get_inner_offset(drte_view* pView, float* pInnerOffsetXOut, float* pInnerOffsetYOut);
DRTE_INLINE float drte_view_get_inner_offset_x(drte_view* pView) { if (pView == NULL) return 0; return pView->innerOffsetX; }
DRTE_INLINE float drte_view_get_inner_offset_y(drte_view* pView) { if (pView == NULL) return 0; return pView->innerOffsetY; }


// Sets the size of a tab in spaces.
void drte_view_set_tab_size(drte_view* pView, unsigned int sizeInSpaces);

// Retrieves the size of a tab in spaces.
DRTE_INLINE unsigned int drte_view_get_tab_size(drte_view* pView) { if (pView == NULL) return 0; return pView->tabSizeInSpaces; }


// Begins a batch dirty of the given view.
void drte_view_begin_dirty(drte_view* pView);

// Ends a batch dirty of the given view.
void drte_view_end_dirty(drte_view* pView);

// Marks a region of the given view as dirty and triggers a redraw.
void drte_view_dirty(drte_view* pView, drte_rect rect);

// Paints a region of the given view.
void drte_view_paint(drte_view* pView, drte_rect rect, void* pUserData);

// Paints the line numbers for the given view.
void drte_view_paint_line_numbers(drte_view* pView, float lineNumbersWidth, float lineNumbersHeight, drte_engine_on_paint_text_proc onPaintText, drte_engine_on_paint_rect_proc onPaintRect, void* pPaintData);


// Retrieves the local rectangle of the given view.
drte_rect drte_view_get_local_rect(drte_view* pView);


// Enables word wrap on the given view.
void drte_view_enable_word_wrap(drte_view* pView);

// Disables word wrap on the given view.
void drte_view_disable_word_wrap(drte_view* pView);

// Determines whether or not the given view has word wrap enabled.
bool drte_view_is_word_wrap_enabled(drte_view* pView);


// Retrieves the index of the line containing the character at the given index.
size_t drte_view_get_character_line(drte_view* pView, drte_line_cache* pLineCache, size_t characterIndex);

// Retrieves the position of the character at the given index, relative to the text rectangle.
//
// To make the position relative to the container simply add the inner offset to them.
void drte_view_get_character_position(drte_view* pView, drte_line_cache* pLineCache, size_t characterIndex, float* pPosXOut, float* pPosYOut);

// Retrieves the closest character to the given point relative to the text.
//
// The return value is whether or not the point is actually over a character. If false is returned, piCharOut and piLineOut will
// be set based on the input position being clamped to the text region.
bool drte_view_get_character_under_point_relative_to_text(drte_view* pView, drte_line_cache* pLineCache, float inputPosXRelativeToText, float inputPosYRelativeToText, size_t* piCharOut, size_t* piLineOut);

// Retrieves the closest character to the given point relative to the container.
//
// The return value is whether or not the point is actually over a character. If false is returned, piCharOut and piLineOut will
// be set based on the input position being clamped to the text region.
bool drte_view_get_character_under_point(drte_view* pView, drte_line_cache* pLineCache, float inputPosXRelativeToContainer, float inputPosYRelativeToContainer, size_t* piCharOut, size_t* piLineOut);

// Retrieves the indices of the visible lines.
void drte_view_get_visible_lines(drte_view* pView, size_t* pFirstLineOut, size_t* pLastLineOut);


// Retrieves the number of lines in the given text engine.
size_t drte_view_get_line_count(drte_view* pView);

// Gets the number of lines per page.
//
// This does not include partially visible lines. Use this for printing.
size_t drte_view_get_line_count_per_page(drte_view* pView);

// Retrieves the page count.
//
// Use this for printing.
size_t drte_view_get_page_count(drte_view* pView);

// Retrieves the number of lines that can fit on the visible region of the text engine.
//
// Use this for controlling the page size for scrollbars.
size_t drte_view_get_visible_line_count(drte_view* pView);

// Retrieves the width of the visible lines.
//
// Use this for implementing horizontal scrollbars.
float drte_view_get_visible_line_width(drte_view* pView);

// Measures a line.
void drte_view_measure_line(drte_view* pView, size_t iLine, float* pWidthOut, float* pHeightOut);

// Retrieves the position of the line at the given index on the y axis.
//
//  Use this for calculating the inner offset for scrolling on the y axis.
float drte_view_get_line_pos_y(drte_view* pView, size_t iLine);

// Finds the line under the given point on the y axis relative to the container.
size_t drte_view_get_line_at_pos_y(drte_view* pView, drte_line_cache* pLineCache, float posY);

// Retrieves the index of the first character of the line at the given index.
size_t drte_view_get_line_first_character(drte_view* pView, drte_line_cache* pLineCache, size_t iLine);

// Retrieves the index of the last character of the line at the given index.
size_t drte_view_get_line_last_character(drte_view* pView, drte_line_cache* pLineCache, size_t iLine);

// Retrieves the index of the first non-whitespace character of the line at the given index.
size_t drte_view_get_line_first_non_whitespace_character(drte_view* pView, drte_line_cache* pLineCache, size_t iLine);

// Retrieves teh index of the first and last character of the line at the given index.
void drte_view_get_line_character_range(drte_view* pView, drte_line_cache* pLineCache, size_t iLine, size_t* pCharStartOut, size_t* pCharEndOut);



// Sets the width of the text cursor.
void drte_view_set_cursor_width(drte_view* pView, float cursorWidth);

// Retrieves the width of the text cursor.
DRTE_INLINE float drte_view_get_cursor_width(drte_view* pView) { if (pView == NULL) return 0; return pView->cursorWidth; }

// Shows the cursor.
void drte_view_show_cursors(drte_view* pView);

// Hides the cursor.
void drte_view_hide_cursors(drte_view* pView);

// Determines whether or not the cursor is visible.
bool drte_view_is_showing_cursors(drte_view* pView);

// Retrieves the position of the cursor, relative to the container.
void drte_view_get_cursor_position(drte_view* pView, size_t cursorIndex, float* pPosXOut, float* pPosYOut);

// Retrieves the rectangle of the cursor at the given index.
drte_rect drte_view_get_cursor_rect(drte_view* pView, size_t cursorIndex);

// Inserts a new cursor. Returns the index of the new cursor.
size_t drte_view_insert_cursor(drte_view* pView, size_t iChar);
size_t drte_view_insert_cursor_at_character_and_line(drte_view* pView, size_t iChar, size_t iLine);

// Removes a cursor by it's index.
void drte_view_remove_cursor(drte_view* pView, size_t cursorIndex);

// Removes any overlapping cursors, leaving only one of the overlapping cursors remaining.
void drte_view_remove_overlapping_cursors(drte_view* pView);

// Retrieves the last cursor.
size_t drte_view_get_last_cursor(drte_view* pView);

/// Retrieves the index of the line the cursor is currently sitting on.
size_t drte_view_get_cursor_line(drte_view* pView, size_t cursorIndex);

/// Retrieves the index of the column the cursor is currently sitting on.
size_t drte_view_get_cursor_column(drte_view* pView, size_t cursorIndex);

/// Retrieves the index of the character the cursor is currently sitting on.
size_t drte_view_get_cursor_character(drte_view* pView, size_t cursorIndex);

/// Moves the cursor to the closest character based on the given input position.
bool drte_view_move_cursor_to_point(drte_view* pView, size_t cursorIndex, float posX, float posY);

/// Moves the cursor to the closest character based on the given input position, relative to the text (not the container).
bool drte_view_move_cursor_to_point_relative_to_text(drte_view* pView, size_t cursorIndex, float posXRelativeToText, float posYRelativeToText);

/// Moves the cursor of the given text engine to the left by one character.
bool drte_view_move_cursor_left(drte_view* pView, size_t cursorIndex);

/// Moves the cursor of the given text engine to the right by one character.
bool drte_view_move_cursor_right(drte_view* pView, size_t cursorIndex);

/// Moves the cursor of the given text engine up one line.
bool drte_view_move_cursor_up(drte_view* pView, size_t cursorIndex);

/// Moves the cursor of the given text engine down one line.
bool drte_view_move_cursor_down(drte_view* pView, size_t cursorIndex);

/// Moves the cursor up or down the given number of lines.
bool drte_view_move_cursor_y(drte_view* pView, size_t cursorIndex, int amount);

/// Moves the cursor of the given text engine to the end of the line.
bool drte_view_move_cursor_to_end_of_line(drte_view* pView, size_t cursorIndex);

/// Moves the cursor of the given text engine to the start of the line.
bool drte_view_move_cursor_to_start_of_line(drte_view* pView, size_t cursorIndex);

/// Moves the cursor of the given text engine to the end of the line at the given index.
bool drte_view_move_cursor_to_end_of_line_by_index(drte_view* pView, size_t cursorIndex, size_t iLine);

/// Moves the cursor of the given text engine to the start of the line at the given index.
bool drte_view_move_cursor_to_start_of_line_by_index(drte_view* pView, size_t cursorIndex, size_t iLine);

/// Moves the cursor to the end of the unwrapped line it is sitting on.
bool drte_view_move_cursor_to_end_of_unwrapped_line(drte_view* pView, size_t cursorIndex);

/// Moves the cursor to the start of the unwrapped line it is sitting on.
bool drte_view_move_cursor_to_start_of_unwrapped_line(drte_view* pView, size_t cursorIndex);

/// Determines whether or not the given cursor is at the end of a wrapped line.
bool drte_view_is_cursor_at_end_of_wrapped_line(drte_view* pView, size_t cursorIndex);

/// Determines whether or not the given cursor is at the start of a wrapped line.
bool drte_view_is_cursor_at_start_of_wrapped_line(drte_view* pView, size_t cursorIndex);

/// Moves the cursor of the given text engine to the end of the text.
bool drte_view_move_cursor_to_end_of_text(drte_view* pView, size_t cursorIndex);

/// Moves the cursor of the given text engine to the end of the text.
bool drte_view_move_cursor_to_start_of_text(drte_view* pView, size_t cursorIndex);

/// Moves the cursor to the start of the selected text.
void drte_view_move_cursor_to_start_of_selection(drte_view* pView, size_t cursorIndex);

/// Moves the cursor to the end of the selected text.
void drte_view_move_cursor_to_end_of_selection(drte_view* pView, size_t cursorIndex);

/// Moves the cursor to the given character index.
void drte_view_move_cursor_to_character(drte_view* pView, size_t cursorIndex, size_t characterIndex);
void drte_view_move_cursor_to_character_and_line(drte_view* pView, size_t cursorIndex, size_t iChar, size_t iLine);

// Moves the cursor to the end of the current word.
size_t drte_view_move_cursor_to_end_of_word(drte_view* pView, size_t cursorIndex);

// Moves the cursor to the start of the next word.
size_t drte_view_move_cursor_to_start_of_next_word(drte_view* pView, size_t cursorIndex);

// Moves the cursor to the start of the current word.
size_t drte_view_move_cursor_to_start_of_word(drte_view* pView, size_t cursorIndex);

// Retrieves the number of characters between the given character index and the next tab column.
size_t drte_view_get_spaces_to_next_column_from_character(drte_view* pView, size_t iChar);

/// Retrieves the number of characters between the cursor and the next tab column.
size_t drte_view_get_spaces_to_next_column_from_cursor(drte_view* pView, size_t cursorIndex);

/// Determines whether or not the cursor is sitting at the start of the selection.
bool drte_view_is_cursor_at_start_of_selection(drte_view* pView, size_t cursorIndex);

/// Determines whether or not the cursor is sitting at the end fo the selection.
bool drte_view_is_cursor_at_end_of_selection(drte_view* pView, size_t cursorIndex);

// Retrieves the word under the given character.
bool drte_view_get_word_under_cursor(drte_view* pView, size_t cursorIndex, size_t* pWordBegOut, size_t* pWordEndOut);

// Retrieves the word under the point relative to the container.
bool drte_view_get_word_under_point(drte_view* pView, float posX, float posY, size_t* pWordBegOut, size_t* pWordEndOut);



/// Determines whether or not anything is selected in the given text engine.
bool drte_view_is_anything_selected(drte_view* pView);

/// Deselects everything in the given text engine.
void drte_view_deselect_all(drte_view* pView);

/// Selects everything in the given text engine.
void drte_view_select_all(drte_view* pView);

/// Selects the given range of text.
void drte_view_select(drte_view* pView, size_t firstCharacter, size_t lastCharacter);

// Selects the word under a cursor.
void drte_view_select_word_under_cursor(drte_view* pView, size_t cursorIndex);

/// Retrieves a copy of the selected text.
///
/// @remarks
///     This returns the length of the selected text. Call this once with <textOut> set to NULL to calculate the required size of the
///     buffer.
///     @par
///     If the output buffer is not larger enough, the string will be truncated.
size_t drte_view_get_selected_text(drte_view* pView, char* textOut, size_t textOutLength);

/// Retrieves the index of the first line of the current selection.
size_t drte_view_get_selection_first_line(drte_view* pView, size_t iSelection);

/// Retrieves the index of the last line of the current selection.
size_t drte_view_get_selection_last_line(drte_view* pView, size_t iSelection);

/// Moves the selection anchor to the end of the given line.
void drte_view_move_selection_anchor_to_end_of_line(drte_view* pView, size_t iLine);

/// Moves the selection anchor to the start of the given line.
void drte_view_move_selection_anchor_to_start_of_line(drte_view* pView, size_t iLine);

/// Retrieves the line the selection anchor is sitting on.
size_t drte_view_get_selection_anchor_line(drte_view* pView);

// Begins a new selection region.
void drte_view_begin_selection(drte_view* pView, size_t iCharBeg);

// Cancels a selection by it's index.
void drte_view_cancel_selection(drte_view* pView, size_t iSelection);

// Cancels the most recent selection.
void drte_view_cancel_last_selection(drte_view* pView);

// Sets the anchor of the most recent selection region.
void drte_view_set_selection_anchor(drte_view* pView, size_t iCharBeg);

// Sets the end point of the most recent selection region.
void drte_view_set_selection_end_point(drte_view* pView, size_t iCharEnd);

// Retrieves the character range of the last selection, if any.
bool drte_view_get_last_selection(drte_view* pView, size_t* iCharBegOut, size_t* iCharEndOut);

// Retrieves the selection region under the given point, if any.
bool drte_view_get_selection_under_point(drte_view* pView, float posX, float posY, size_t* piSelectionOut);


/// Inserts a character at the position of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_view_insert_character_at_cursor(drte_view* pView, size_t cursorIndex, unsigned int character);
bool drte_view_insert_character_at_cursors(drte_view* pView, unsigned int character);

/// Inserts a character at the position of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_view_insert_text_at_cursor(drte_view* pView, size_t cursorIndex, const char* text);

/// Deletes the character to the left of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_view_delete_character_to_left_of_cursor(drte_view* pView, size_t cursorIndex);
bool drte_view_delete_character_to_left_of_cursors(drte_view* pView, bool leaveNewLines);

/// Deletes the character to the right of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_view_delete_character_to_right_of_cursor(drte_view* pView, size_t cursorIndex);
bool drte_view_delete_character_to_right_of_cursors(drte_view* pView, bool leaveNewLines);


/// Deletes the currently selected text.
///
/// @return True if the text within the text engine has changed.
bool drte_view_delete_selected_text(drte_view* pView);

// Deletes the text of a specific selection.
bool drte_view_delete_selection_text(drte_view* pView, size_t iSelectionToDelete);


/// Finds the given string starting from the cursor and then looping back.
bool drte_view_find_next(drte_view* pView, const char* text, size_t* pSelectionStartOut, size_t* pSelectionEndOut);

/// Finds the given string starting from the cursor, but does not loop back.
bool drte_view_find_next_no_loop(drte_view* pView, const char* text, size_t* pSelectionStartOut, size_t* pSelectionEndOut);


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

#ifndef DRTE_STACK_BUFFER_ALIGNMENT
#define DRTE_STACK_BUFFER_ALIGNMENT sizeof(size_t)
#endif

#ifndef DRTE_STACK_BUFFER_BLOCK_SIZE
#define DRTE_STACK_BUFFER_BLOCK_SIZE 4096
#endif

#ifndef DRTE_PAGE_LINE_COUNT
#define DRTE_PAGE_LINE_COUNT    256
#endif

#define DRTE_INVALID_STYLE_SLOT 255

// Flags for the drte_engine::flags and drte_view::flags properties.
#define DRTE_USE_EXPLICIT_LINE_HEIGHT   (1 << 0)
#define DRTE_WORD_WRAP_ENABLED          (1 << 1)
#define DRTE_SHOWING_CURSORS            (1 << 2)



// min
#define drte_min(a, b) (((a) < (b)) ? (a) : (b))
#define drte_round_up(x, multiple) ((((x) + ((multiple) - 1)) / (multiple)) * (multiple))

// Determines if the given character is whitespace.
bool drte_is_whitespace(uint32_t utf32)
{
    return utf32 == ' ' || utf32 == '\t' || utf32 == '\n' || utf32 == '\v' || utf32 == '\f' || utf32 == '\r';
}

// Helper for determining whether or not the given character is a symbol or whitespace.
bool drte_is_symbol_or_whitespace(uint32_t utf32)
{
    // Special case for underscores. This is a bit of a hack and should probably be moved out of here later.
    if (utf32 == '_') {
        return false;
    }

    return (utf32 < '0') || (utf32 >= ':' && utf32 < 'A') || (utf32 >= '[' && utf32 < 'a') || (utf32 > '{');
}

// Helper for swapping the characters in a region.
static drte_region drte_region_swap_characters(drte_region region)
{
    drte_region result;
    result.iCharBeg = region.iCharEnd;
    result.iCharEnd = region.iCharBeg;

    return result;
}

// Helper for normalizing a region which just means to swap the beginning and end characters if they are the wrong way around.
static drte_region drte_region_normalize(drte_region region)
{
    if (region.iCharBeg < region.iCharEnd) {
        return region;  // Already normalized.
    }

    return drte_region_swap_characters(region);
}

// strcpy_s()
static int drte__strcpy_s(char* dst, size_t dstSizeInBytes, const char* src)
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

// strncpy_s()
int drte__strncpy_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
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



//// Rect ////

DRTE_INLINE drte_rect drte_make_rect(float left, float top, float right, float bottom)
{
    drte_rect rect;
    rect.left   = left;
    rect.top    = top;
    rect.right  = right;
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

DRTE_INLINE bool drte_rect_has_volume(drte_rect rect)
{
    return rect.right > rect.left && rect.bottom > rect.top;
}

DRTE_INLINE drte_rect drte_rect_make_right_way_out(drte_rect rect)
{
    drte_rect result = rect;

    if (rect.right < rect.left) {
        result.left = rect.right;
        result.right = rect.left;
    }

    if (rect.bottom < rect.top) {
        result.top = rect.bottom;
        result.bottom = rect.top;
    }

    return result;
}



//// Stack Buffer ////
//
// A stack buffer is a simple FILO buffer where blocks memory of arbitrary sizes are pushed to the end and, likewise, freed from the end.
//
// Do not persistently store pointers returned by drte_stack_buffer_alloc(). Instead store offsets by interrogating the stackPtr property.
//
// Data can be popped from the internal buffer with drte_stack_buffer_free() or drte_stack_buffer_set_stack_ptr(). Note that these will
// ensure the stack pointer is aligned based on DRTE_STACK_BUFFER_ALIGNMENT, so you will want to call drte_stack_buffer_get_stack_ptr()
// after the fact if you need to access the stack pointer again later.
bool drte_stack_buffer_init(drte_stack_buffer* pStack)
{
    if (pStack == NULL) {
        return false;
    }

    memset(pStack, 0, sizeof(*pStack));
    return true;
}

void drte_stack_buffer_uninit(drte_stack_buffer* pStack)
{
    if (pStack == NULL) {
        return;
    }

    free(pStack->pBuffer);
}

size_t drte_stack_buffer_get_stack_ptr(drte_stack_buffer* pStack)
{
    if (pStack == NULL) {
        return 0;
    }

    return pStack->stackPtr;
}

size_t drte_stack_buffer_set_stack_ptr(drte_stack_buffer* pStack, size_t newStackPtr)
{
    if (pStack == NULL) {
        return 0;
    }

    assert(newStackPtr <= pStack->stackPtr);   // <-- If you trigger this it means you're trying to move the stack pointer beyond the valid area. You cannot use this function to grow the buffer.

    pStack->stackPtr = drte_round_up(newStackPtr, DRTE_STACK_BUFFER_ALIGNMENT);

    // Shrink the buffer if we're getting a bit too wasteful.
    size_t desiredBufferSize = drte_round_up(pStack->stackPtr, DRTE_STACK_BUFFER_BLOCK_SIZE);
    if (desiredBufferSize == 0) {
        desiredBufferSize = DRTE_STACK_BUFFER_BLOCK_SIZE;
    }

    if (desiredBufferSize < pStack->bufferSize) {
        void* pNewBuffer = realloc(pStack->pBuffer, desiredBufferSize);
        if (pNewBuffer != NULL) {
            pStack->pBuffer = pNewBuffer;
            pStack->bufferSize = desiredBufferSize;
        }
    }

    return pStack->stackPtr;
}

void* drte_stack_buffer_alloc(drte_stack_buffer* pStack, size_t sizeInBytes)
{
    if (pStack == NULL || sizeInBytes == 0) {
        return NULL;
    }

    // The allocated size is always aligned.
    sizeInBytes = drte_round_up(sizeInBytes, DRTE_STACK_BUFFER_ALIGNMENT);

    size_t newStackPtr = pStack->stackPtr + sizeInBytes;
    if (newStackPtr > pStack->bufferSize) {
        size_t newBufferSize = (pStack->bufferSize == 0) ? DRTE_STACK_BUFFER_BLOCK_SIZE : drte_round_up(newStackPtr, DRTE_STACK_BUFFER_BLOCK_SIZE);
        void* pNewBuffer = realloc(pStack->pBuffer, newBufferSize);
        if (pNewBuffer == NULL) {
            return NULL;
        }

        pStack->bufferSize = newBufferSize;
        pStack->pBuffer = pNewBuffer;
    }

    void* pResult = (void*)((uint8_t*)pStack->pBuffer + pStack->stackPtr);

    pStack->stackPtr = newStackPtr;
    return pResult;
}

void drte_stack_buffer_free(drte_stack_buffer* pStack, size_t sizeInBytes)
{
    if (pStack == NULL || sizeInBytes == 0) {
        return;
    }

    assert(sizeInBytes <= pStack->stackPtr);     // <-- If you trigger this is means you're trying to free too much.
    drte_stack_buffer_set_stack_ptr(pStack, pStack->stackPtr - sizeInBytes);
}

void* drte_stack_buffer_get_data_ptr(drte_stack_buffer* pStack, size_t offset)
{
    if (pStack == NULL || pStack->stackPtr < offset) {
        return NULL;
    }

    return (void*)((uint8_t*)pStack->pBuffer + offset);
}




// Performs a full refresh of the text engine, including refreshing line wrapping and repaining.
void drte_engine__refresh(drte_engine* pEngine);

/// Applies the given undo state.
void drte_engine__apply_undo_state(drte_engine* pEngine, const void* pUndoDataPtr);

/// Applies the given undo state as a redo operation.
void drte_engine__apply_redo_state(drte_engine* pEngine, const void* pUndoDataPtr);

/// Called when a cursor moves.
void drte_engine__on_cursor_move(drte_engine* pEngine, drte_view* pView, size_t cursorIndex);



static void drte_view__refresh_word_wrapping(drte_view* pView);
static float drte_view__get_tab_width_in_pixels(drte_view* pView);

void drte_view__update_cursor_sticky_position(drte_view* pView, drte_cursor* pCursor)
{
    if (pView == NULL || pCursor == NULL) {
        return;
    }

    // If the character is on a different line to the cursor, it means the cursor is pinned to the end of the previous line and the character
    // is the first character on the _next_ line. This will happen when word wrap is enabled. In this case things need to be treated a bit
    // differently to calculate the x position.
    float charPosX;
    float charPosY;
    if (pCursor->iLine != drte_view_get_character_line(pView, pView->pWrappedLines, pCursor->iCharAbs)) {
        drte_view_measure_line(pView, pCursor->iLine, &charPosX, NULL);
        charPosY = drte_view_get_line_pos_y(pView, pCursor->iLine);
    } else {
        drte_view_get_character_position(pView, pView->pWrappedLines, pCursor->iCharAbs, &charPosX, &charPosY);
    }

    pCursor->absoluteSickyPosX = charPosX;
}

size_t drte_engine__acquire_view_id(drte_engine* pEngine)
{
    assert(pEngine != NULL);
    return pEngine->nextViewID++;   // <-- Make this thread-safe?
}


// Finds a style slot index of the given style token. Returns DRTE_INVALID_STYLE_SLOT if it could not be found.
uint8_t drte_engine__get_style_slot(drte_engine* pEngine, drte_style_token styleToken)
{
    assert(pEngine != NULL);
    assert(pEngine->styleCount < 256);

    for (uint8_t iStyleSlot = 0; iStyleSlot < pEngine->styleCount; ++iStyleSlot) {
        if (pEngine->styles[iStyleSlot].styleToken == styleToken) {
            return iStyleSlot;
        }
    }

    return DRTE_INVALID_STYLE_SLOT;
}

// Gets the style token associated with the given slot.
drte_style_token drte_engine__get_style_token(drte_engine* pEngine, uint8_t styleSlot)
{
    assert(pEngine != NULL);
    return pEngine->styles[styleSlot].styleToken;
}

// Retrieves the next selection region starting from the given character, including the region the character is sitting in, if any.
static bool drte_view__get_next_selection_from_character(drte_view* pView, size_t iChar, drte_region* pSelectionOut)
{
    assert(pView != NULL);
    assert(pSelectionOut != NULL);

    // Selections can be in any order. Need to first check every single one to determine if any are on top of the character. If so we
    // just return the first one. Otherwise we fall through to the next loop which finds the closest selection to the character.
    bool foundSelectionAfterChar = false;
    drte_region closestSelection;
    closestSelection.iCharBeg = (size_t)-1;
    closestSelection.iCharEnd = (size_t)-1;

    for (size_t iSelection = 0; iSelection < pView->selectionCount; ++iSelection) {
        drte_region selection = drte_region_normalize(pView->pSelections[iSelection]);
        if (iChar >= selection.iCharBeg && iChar < selection.iCharEnd) {
            *pSelectionOut = selection;
            return true;
        }

        if (selection.iCharBeg > iChar) {
            foundSelectionAfterChar = true;
            if (closestSelection.iCharBeg > selection.iCharBeg) {
                closestSelection = selection;
            }
        }
    }


    if (foundSelectionAfterChar) {
        *pSelectionOut = closestSelection;
    }

    return foundSelectionAfterChar;
}



//// Line Cache ////

bool drte_line_cache_init(drte_line_cache* pLineCache)
{
    if (pLineCache == NULL) {
        return false;
    }

    // There's always at least one line.
    pLineCache->bufferSize = DRTE_PAGE_LINE_COUNT;
    pLineCache->pLines = (size_t*)calloc(pLineCache->bufferSize, sizeof(*pLineCache->pLines));   // <-- calloc() is important here. It initializes the first line to 0.
    pLineCache->count = 1;

    return true;
}

void drte_line_cache_uninit(drte_line_cache* pLineCache)
{
    if (pLineCache == NULL) {
        return;
    }

    free(pLineCache->pLines);

    // It's important to clear everything to zero in case this is called multiple times after each other which is abolutely possible.
    pLineCache->pLines = NULL;
    pLineCache->bufferSize = 0;
    pLineCache->count = 0;
}

size_t drte_line_cache_get_line_count(drte_line_cache* pLineCache)
{
    if (pLineCache == NULL) {
        return 0;
    }

    return pLineCache->count;
}


size_t drte_line_cache_get_line_first_character(drte_line_cache* pLineCache, size_t iLine)
{
    if (pLineCache == NULL || iLine >= pLineCache->count) {
        return 0;
    }

    return pLineCache->pLines[iLine];
}

void drte_line_cache_set_line_first_character(drte_line_cache* pLineCache, size_t iLine, size_t iCharBeg)
{
    if (pLineCache == NULL || iLine >= pLineCache->count) {
        return;
    }

    pLineCache->pLines[iLine] = iCharBeg;
}

bool drte_line_cache_insert_lines(drte_line_cache* pLineCache, size_t insertLineIndex, size_t lineCount, size_t characterOffset)
{
    if (pLineCache == NULL || insertLineIndex > pLineCache->count) {
        return false;
    }

    size_t newLineCount = pLineCache->count + lineCount;

    if (newLineCount >= pLineCache->bufferSize) {
        size_t newLineBufferSize = (pLineCache->bufferSize == 0) ? DRTE_PAGE_LINE_COUNT : drte_round_up(newLineCount, DRTE_PAGE_LINE_COUNT);
        size_t* pNewLines = (size_t*)realloc(pLineCache->pLines, newLineBufferSize * sizeof(*pNewLines));
        if (pNewLines == NULL) {
            return false;   // Ran out of memory?
        }

        pLineCache->bufferSize = newLineBufferSize;
        pLineCache->pLines = pNewLines;
    }

    // All existing lines coming after the line the text was inserted at need to be moved down newLineCount slots. They also need to have their
    // first character index updated.
    for (size_t i = pLineCache->count; i > insertLineIndex; --i) {
        size_t iSrc = i-1;
        size_t iDst = iSrc + lineCount;
        pLineCache->pLines[iDst] = pLineCache->pLines[iSrc] + characterOffset;
    }

    pLineCache->count = newLineCount;
    return true;
}

bool drte_line_cache_append_line(drte_line_cache* pLineCache, size_t iLineCharBeg)
{
    size_t lineCount = drte_line_cache_get_line_count(pLineCache);
    if (!drte_line_cache_insert_lines(pLineCache, lineCount, 1, 0)) {
        return false;
    }

    drte_line_cache_set_line_first_character(pLineCache, lineCount, iLineCharBeg);
    return true;
}

bool drte_line_cache_remove_lines(drte_line_cache* pLineCache, size_t firstLineIndex, size_t lineCount, size_t characterOffset)
{
    if (pLineCache == NULL || firstLineIndex >= pLineCache->count) {
        return false;
    }

    if (pLineCache->count <= lineCount) {
        pLineCache->count = 1;
    } else {
        for (size_t i = firstLineIndex; i < (pLineCache->count - lineCount); ++i) {
            size_t iDst = i;
            size_t iSrc = i + lineCount;
            pLineCache->pLines[iDst] = pLineCache->pLines[iSrc] - characterOffset;
        }

        pLineCache->count -= lineCount;
    }

    return true;
}

bool drte_line_cache_offset_lines(drte_line_cache* pLineCache, size_t firstLineIndex, size_t characterOffset)
{
    if (pLineCache == NULL || firstLineIndex >= pLineCache->count) {
        return false;
    }

    for (size_t iLine = firstLineIndex; iLine < pLineCache->count; ++iLine) {
        pLineCache->pLines[iLine] += characterOffset;
    }

    return true;
}

bool drte_line_cache_offset_lines_negative(drte_line_cache* pLineCache, size_t firstLineIndex, size_t characterOffset)
{
    if (pLineCache == NULL || firstLineIndex >= pLineCache->count) {
        return false;
    }

    for (size_t iLine = firstLineIndex; iLine < pLineCache->count; ++iLine) {
        pLineCache->pLines[iLine] -= characterOffset;
    }

    return true;
}


size_t drte_line_cache_find_line_by_character__internal(drte_line_cache* pLineCache, size_t iChar, size_t iLineBeg, size_t iLineEnd)
{
    assert(pLineCache != NULL);

    if ((iLineBeg+1 == iLineEnd) || (iChar >= pLineCache->pLines[iLineBeg] && iChar < pLineCache->pLines[iLineBeg+1])) {
        return iLineBeg;    // It's on iLineBeg.
    }

    size_t iLineMid = iLineEnd - ((iLineEnd - iLineBeg)/2);
    if (iChar >= pLineCache->pLines[iLineMid]) {
        return drte_line_cache_find_line_by_character__internal(pLineCache, iChar, iLineMid, iLineEnd);
    } else {
        return drte_line_cache_find_line_by_character__internal(pLineCache, iChar, iLineBeg, iLineMid);
    }
}

size_t drte_line_cache_find_line_by_character(drte_line_cache* pLineCache, size_t iChar)
{
    if (pLineCache == NULL || pLineCache->count <= 1) {
        return 0;
    }

#if 1
    if (iChar >= pLineCache->pLines[pLineCache->count-1]) {
        return pLineCache->count-1;
    }

    return drte_line_cache_find_line_by_character__internal(pLineCache, iChar, 0, pLineCache->count-1); // <-- We've already checked the last line so start at the second-last line.
#endif

#if 0
    // Linear search. Simple, but slow.
    size_t lineIndex = 0;
    for (size_t iLine = 0; iLine < pLineCache->count; ++iLine) {
        if (pLineCache->pLines[iLine] > iChar) {
            break;
        }

        lineIndex = iLine;
    }

    return lineIndex;
#endif
}

void drte_line_cache_clear(drte_line_cache* pLineCache)
{
    if (pLineCache == NULL) {
        return;
    }

    pLineCache->count = 0;
}



// A drte_segment object is used for iterating over the segments of a chunk of text.
typedef struct
{
    drte_line_cache* pLineCache;
    size_t iLine;
    size_t iLineCharBeg;
    size_t iLineCharEnd;
    size_t iCursorLine;
    size_t iCharBeg;
    size_t iCharEnd;
    uint8_t fgStyleSlot;
    uint8_t bgStyleSlot;
    float posX;
    float width;
    bool isAtEnd;
    bool isAtEndOfLine;
} drte_segment;

float drte_engine__measure_segment(drte_view* pView, drte_segment* pSegment)
{
    assert(pView != NULL);
    assert(pSegment != NULL);

    drte_engine* pEngine = pView->pEngine;
    assert(pEngine != NULL);


    float segmentWidth = 0;
    if (pSegment->iCharEnd > pSegment->iCharBeg) {
        uint32_t c = drte_engine_get_utf32(pEngine, pSegment->iCharBeg);
        if (c == '\t') {
            // It was a tab segment.
            float tabWidth = drte_view__get_tab_width_in_pixels(pView);
            size_t tabCount = pSegment->iCharEnd - pSegment->iCharBeg;
            float nextTabPos = (float)((int)(pSegment->posX / tabWidth) + 1) * tabWidth;
            float distanceToNextTab = nextTabPos - pSegment->posX;
            segmentWidth = distanceToNextTab + ((tabCount-1) * tabWidth);
        } else if (pSegment->iCharBeg == pSegment->iLineCharEnd) {
            segmentWidth = 0;
        } else {
            // It's normal text. We need to refer to the backend for measuring.
            float unused;
            drte_style_token fgStyleToken = drte_engine__get_style_token(pEngine, pSegment->fgStyleSlot);
            if (pEngine->onMeasureString && fgStyleToken) {
                pEngine->onMeasureString(pEngine, fgStyleToken, pEngine->text + pSegment->iCharBeg, pSegment->iCharEnd - pSegment->iCharBeg, &segmentWidth, &unused);
            }
        }
    }

    return segmentWidth;
}

bool drte_engine__next_segment(drte_view* pView, drte_segment* pSegment)
{
    assert(pView != NULL);
    assert(pSegment != NULL);

    drte_engine* pEngine = pView->pEngine;
    assert(pEngine != NULL);

    // TODO: Handle UTF-8 properly.
    // TODO: There is LOTS of optimization opportunity in this function.

    // The next segment is clamped to a specific character with the following priorities, from highest priority to lowest
    // - The end of the text
    // - The end of the line
    // - Tab boundaries
    // - Selection boundaries
    // - Highlight boundaries
    //
    // If the next character begins in the middle of a segment, it will be clamped against said segment.


    if (pSegment->isAtEnd) {
        return false;
    }

    if (pSegment->isAtEndOfLine) {
        pSegment->iLine += 1;
        pSegment->posX = 0;
        pSegment->width = 0;
        pSegment->isAtEndOfLine = false;
        drte_view_get_line_character_range(pView, pSegment->pLineCache, pSegment->iLine, &pSegment->iLineCharBeg, &pSegment->iLineCharEnd);

        pSegment->iCharEnd = pSegment->iLineCharBeg;
    }

    uint8_t fgStyleSlot = pEngine->defaultStyleSlot;
    uint8_t bgStyleSlot = pEngine->defaultStyleSlot;

    // Find the end of the next segment, but don't modify the segment yet. The reason for this is that we need to measure the segment later.
    size_t iCharBeg = pSegment->iCharEnd;
    size_t iCharEnd = iCharBeg;

    if (pView->cursorCount > 0 && pSegment->iLine == pSegment->iCursorLine) {
        bgStyleSlot = pEngine->activeLineStyleSlot;
    }


    bool isAnythingSelected = false;
    bool isInSelection = false;
    drte_region selection;
    if (drte_view__get_next_selection_from_character(pView, iCharBeg, &selection)) {
        isInSelection = iCharBeg >= selection.iCharBeg && iCharBeg < selection.iCharEnd;
        isAnythingSelected = true;
    }

    if (isInSelection) {
        bgStyleSlot = pEngine->selectionStyleSlot;
    }



    // Highlight segment.
    drte_style_segment highlightSegment;
    drte_style_token highlightStyleToken;
    bool isInHighlightSegment = false;
    if (pEngine->onGetNextHighlight && pEngine->onGetNextHighlight(pEngine, iCharBeg, &highlightSegment.iCharBeg, &highlightSegment.iCharEnd, &highlightStyleToken, pEngine->pHighlightUserData)) {
        isInHighlightSegment = iCharBeg >= highlightSegment.iCharBeg && iCharBeg < highlightSegment.iCharEnd;
    } else {
        highlightSegment.iCharBeg = (size_t)-1;
        highlightSegment.iCharEnd = (size_t)-1;
        highlightStyleToken = drte_engine__get_style_token(pEngine, pEngine->defaultStyleSlot);
    }



    size_t iMaxChar = (size_t)-1;

    // Clamp to selection.
    if (isInSelection) {
        iMaxChar = selection.iCharEnd;
    } else if (isAnythingSelected) {
        if (selection.iCharBeg > iCharBeg) {
            iMaxChar = selection.iCharBeg;
        }
    }

    // Clamp to highlight segment.
    if (isInHighlightSegment) {
        fgStyleSlot = drte_engine__get_style_slot(pEngine, highlightStyleToken);
        iMaxChar = drte_min(iMaxChar, highlightSegment.iCharEnd);
    } else {
        iMaxChar = drte_min(iMaxChar, highlightSegment.iCharBeg);
    }



    char c = pEngine->text[iCharBeg];
    if (c == '\0') {
        pSegment->isAtEnd = true;
    } else {
        if (iCharBeg == pSegment->iLineCharEnd) {
            pSegment->isAtEndOfLine = true;
            iCharEnd += 1;
        } else {
            for (;;) {
                c = pEngine->text[iCharEnd];
                if (c == '\0' || iCharEnd == pSegment->iLineCharEnd) {
                    break;
                }

                if (c == '\t') {
                    if (pEngine->text[iCharBeg] != '\t') {
                        break;
                    } else {
                        // Group tabs into a single segment.
                        for (;;) {
                            c = pEngine->text[iCharEnd];
                            if (c == '\0' || iCharEnd == pSegment->iLineCharEnd || c != '\t') {
                                break;
                            }

                            if (iCharEnd == iMaxChar) {
                                break;
                            }

                            iCharEnd += 1;
                        }

                        break;
                    }
                }


                // Selection and styling segment clamp.
                if (iCharEnd == iMaxChar) {
                    break;
                }

                iCharEnd += 1;
            }
        }
    }

    if (iCharBeg == iCharEnd) {
        assert(pSegment->isAtEnd == true);
        iCharEnd += 1;
    }


    // We now have everything we need to construct the next segment iterator.
    pSegment->iCharBeg = iCharBeg;
    pSegment->iCharEnd = iCharEnd;
    pSegment->fgStyleSlot = fgStyleSlot;
    pSegment->bgStyleSlot = bgStyleSlot;
    pSegment->posX += pSegment->width;
    pSegment->width = drte_engine__measure_segment(pView, pSegment);

    return true;
}

bool drte_engine__next_segment_on_line(drte_view* pView, drte_segment* pSegment)
{
    assert(pView != NULL);
    assert(pSegment != NULL);

    if (pSegment->isAtEndOfLine) {
        return false;
    }

    return drte_engine__next_segment(pView, pSegment);
}

bool drte_engine__first_segment(drte_view* pView, drte_line_cache* pLineCache, size_t iChar, drte_segment* pSegment)
{
    if (pView == NULL || pSegment == NULL) {
        return false;
    }

    drte_engine* pEngine = pView->pEngine;
    if (pEngine == NULL || pEngine->textLength == 0) {
        return false;
    }


    pSegment->pLineCache = pLineCache;
    pSegment->iLine = drte_line_cache_find_line_by_character(pLineCache, iChar);
    pSegment->iCursorLine = drte_view_get_cursor_line(pView, pView->cursorCount-1);
    pSegment->iCharBeg = iChar;
    pSegment->iCharEnd = pSegment->iCharBeg;
    pSegment->fgStyleSlot = pEngine->defaultStyleSlot;
    pSegment->bgStyleSlot = pEngine->defaultStyleSlot;
    pSegment->posX = 0;
    pSegment->width = 0;
    pSegment->isAtEnd = false;
    pSegment->isAtEndOfLine = false;
    drte_view_get_line_character_range(pView, pSegment->pLineCache, pSegment->iLine, &pSegment->iLineCharBeg, &pSegment->iLineCharEnd);

    return drte_engine__next_segment(pView, pSegment);
}

bool drte_engine__first_segment_on_line(drte_view* pView, drte_line_cache* pLineCache, size_t lineIndex, size_t iChar, drte_segment* pSegment)
{
    if (pView == NULL || pSegment == NULL) {
        return false;
    }

    drte_engine* pEngine = pView->pEngine;
    if (pEngine == NULL || pEngine->textLength == 0) {
        return false;
    }


    pSegment->pLineCache = pLineCache;
    pSegment->iLine = lineIndex;
    pSegment->iCursorLine = drte_view_get_cursor_line(pView, pView->cursorCount-1);
    pSegment->iCharBeg = iChar;
    pSegment->iCharEnd = iChar;
    pSegment->fgStyleSlot = pEngine->defaultStyleSlot;
    pSegment->bgStyleSlot = pEngine->defaultStyleSlot;
    pSegment->posX = 0;
    pSegment->width = 0;
    pSegment->isAtEnd = false;
    pSegment->isAtEndOfLine = false;

    if (iChar == (size_t)-1) {
        pSegment->iCharBeg = drte_view_get_line_first_character(pView, pLineCache, lineIndex);
        pSegment->iCharEnd = pSegment->iCharBeg;
    }

    pSegment->iLineCharBeg = pSegment->iCharBeg;
    pSegment->iLineCharEnd = drte_view_get_line_last_character(pView, pLineCache, lineIndex);

    return drte_engine__next_segment(pView, pSegment);
}



// Word iteration.
typedef struct
{
    size_t iCharRangeBeg;
    size_t iCharRangeEnd;
    size_t iCharBeg;
    size_t iCharEnd;
    bool isAtEnd;
} drte_word_iterator;

bool drte_engine__next_word(drte_engine* pEngine, drte_word_iterator* pIterator)
{
    if (pEngine == NULL || pIterator == NULL) {
        return false;
    }

    if (pIterator->isAtEnd) {
        return false;
    }

    pIterator->iCharBeg = pIterator->iCharEnd;

    size_t iCharEnd;
    if (!drte_engine_get_end_of_word_containing_character(pEngine, pIterator->iCharBeg, &iCharEnd)) {
        pIterator->isAtEnd = true;
        return false;
    }

    // Always make sure the last character never goes beyond the end of the range.
    if (iCharEnd > pIterator->iCharRangeEnd) {
        iCharEnd = pIterator->iCharRangeEnd;
    }

    pIterator->iCharEnd = iCharEnd;

    // If we got an empty word we will fall back to the remaining section of the range as the last part.
    if (pIterator->iCharBeg == pIterator->iCharEnd) {
        pIterator->iCharEnd = pIterator->iCharRangeEnd;
    }


    if (pIterator->iCharBeg == pIterator->iCharEnd) {
        pIterator->isAtEnd = true;
        return false;
    }

    return true;
}

bool drte_engine__first_word(drte_engine* pEngine, size_t iCharRangeBeg, size_t iCharRangeEnd, drte_word_iterator* pIterator)
{
    if (pEngine == NULL || pEngine->textLength == 0 || pIterator == NULL) {
        return false;
    }

    if (iCharRangeBeg >= iCharRangeEnd) {
        return false;
    }

    if (iCharRangeBeg > pEngine->textLength) {
        return false;
    }

    // Clamp the end character to the last character in the text.
    if (iCharRangeEnd > pEngine->textLength) {
        iCharRangeEnd = pEngine->textLength;
    }


    pIterator->iCharRangeBeg = iCharRangeBeg;
    pIterator->iCharRangeEnd = iCharRangeEnd;
    pIterator->iCharBeg = iCharRangeBeg;
    pIterator->iCharEnd = iCharRangeBeg;
    pIterator->isAtEnd = false;

    return drte_engine__next_word(pEngine, pIterator);
}




void drte_engine__push_text_change_to_prepared_undo_state(drte_engine* pEngine, drte_undo_change_type type, size_t iCharBeg, size_t iCharEnd, const char* text)
{
    if (pEngine == NULL || text == NULL) {
        return;
    }

    size_t sizeInBytes =
        sizeof(type) +
        sizeof(size_t) +
        sizeof(size_t) +
        (iCharEnd - iCharBeg) + 1;  // +1 for null terminator.

    uint8_t* pData = (uint8_t*)drte_stack_buffer_alloc(&pEngine->preparedUndoState, sizeInBytes);
    if (pData == NULL) {
        return;
    }

    memcpy(pData, &type, sizeof(type));
    memcpy(pData + sizeof(type), &iCharBeg, sizeof(iCharBeg));
    memcpy(pData + sizeof(type) + sizeof(iCharBeg), &iCharEnd, sizeof(iCharEnd));
    memcpy(pData + sizeof(type) + sizeof(iCharBeg) + sizeof(iCharEnd), text, (iCharEnd - iCharBeg));
    *(pData + sizeof(type) + sizeof(iCharBeg) + sizeof(iCharEnd) + (iCharEnd - iCharBeg)) = '\0';

    *((size_t*)drte_stack_buffer_get_data_ptr(&pEngine->preparedUndoState, pEngine->preparedUndoTextChangesOffset)) += 1;
}


bool drte_engine_init(drte_engine* pEngine, void* pUserData)
{
    if (pEngine == NULL) {
        return false;
    }

    memset(pEngine, 0, sizeof(*pEngine));

    pEngine->defaultStyleSlot = DRTE_INVALID_STYLE_SLOT;
    pEngine->selectionStyleSlot = DRTE_INVALID_STYLE_SLOT;
    pEngine->activeLineStyleSlot = DRTE_INVALID_STYLE_SLOT;
    pEngine->cursorStyleSlot = DRTE_INVALID_STYLE_SLOT;
    pEngine->lineNumbersStyleSlot = DRTE_INVALID_STYLE_SLOT;


    // Note that _wrappedLines is intentionally left uninitialized because word wrap is disabled by default.
    drte_line_cache_init(&pEngine->_unwrappedLines);

    // Both line caches point to the same underlying structure to save on memory. This is only the case when word wrap is disabled, which is
    // the default behaviour.
    pEngine->pUnwrappedLines = &pEngine->_unwrappedLines;

    pEngine->cursorBlinkRate       = 500;
    pEngine->timeToNextCursorBlink = pEngine->cursorBlinkRate;
    pEngine->isCursorBlinkOn       = true;
    pEngine->pUserData             = pUserData;

    drte_stack_buffer_init(&pEngine->preparedUndoState);
    drte_stack_buffer_init(&pEngine->undoBuffer);


    // The temporary view.
    //pEngine->pView = drte_view_create(pEngine);

    return true;
}

void drte_engine_uninit(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    // All views need to be deleted.
    while (pEngine->pRootView != NULL) {
        drte_view_delete(pEngine->pRootView);
    }


    drte_engine_clear_undo_stack(pEngine);

    drte_stack_buffer_uninit(&pEngine->undoBuffer);
    drte_stack_buffer_uninit(&pEngine->preparedUndoState);

    drte_line_cache_uninit(&pEngine->_unwrappedLines);

    //free(pEngine->pView->pSelections);
    //free(pEngine->pView->pCursors);

    free(pEngine->text);
}


size_t drte_engine_get_view_count(drte_engine* pEngine)
{
    size_t count = 0;
    for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
        count += 1;
    }

    return count;
}


bool drte_engine_register_style_token(drte_engine* pEngine, drte_style_token styleToken, drte_font_metrics fontMetrics)
{
    if (pEngine == NULL) {
        return false;
    }

    // If the token already exists just refresh.
    uint8_t styleSlot = drte_engine__get_style_slot(pEngine, styleToken);
    if (styleSlot != DRTE_INVALID_STYLE_SLOT) {
        pEngine->styles[styleSlot].fontMetrics = fontMetrics;

        if (!(pEngine->flags & DRTE_USE_EXPLICIT_LINE_HEIGHT)) {
            pEngine->lineHeight = 0;
            for (uint16_t i = 0; i < pEngine->styleCount; ++i) {
                if (pEngine->lineHeight < fontMetrics.lineHeight) {
                    pEngine->lineHeight = fontMetrics.lineHeight;
                }
            }
        }

        drte_engine__refresh(pEngine);
        return true;
    }


    // If we get here it means the style has not previously been registered. We don't need to do any repainting or refreshing here
    // because the style will not actually be used by anything yet.
    if (pEngine->styleCount == 255) {
        return false;   // Too many styles. The 256'th slot (index 255) is used as the error indicator.
    }

    // Line heights need to be refreshed if it has not been set explicitly.
    if (pEngine->lineHeight < fontMetrics.lineHeight && (pEngine->flags & DRTE_USE_EXPLICIT_LINE_HEIGHT) == 0) {
        pEngine->lineHeight = fontMetrics.lineHeight;
    }

    pEngine->styles[pEngine->styleCount].styleToken = styleToken;
    pEngine->styles[pEngine->styleCount].fontMetrics = fontMetrics;

    pEngine->styleCount += 1;
    return true;
}

void drte_engine_set_default_style(drte_engine* pEngine, drte_style_token styleToken)
{
    if (pEngine == NULL) {
        return;
    }

    uint8_t styleSlot = drte_engine__get_style_slot(pEngine, styleToken);
    if (styleSlot == DRTE_INVALID_STYLE_SLOT) {
        return;
    }

    if (pEngine->defaultStyleSlot == styleSlot) {
        return; // Nothing has changed.
    }

    pEngine->defaultStyleSlot = styleSlot;
    drte_engine__refresh(pEngine);
}

void drte_engine_set_selection_style(drte_engine* pEngine, drte_style_token styleToken)
{
    if (pEngine == NULL) {
        return;
    }

    uint8_t styleSlot = drte_engine__get_style_slot(pEngine, styleToken);
    if (styleSlot == DRTE_INVALID_STYLE_SLOT) {
        return;
    }

    if (pEngine->selectionStyleSlot == styleSlot) {
        return; // Nothing has changed.
    }

    pEngine->selectionStyleSlot = styleSlot;


    for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
        if (drte_view_is_anything_selected(pView)) {
            drte_view__refresh_word_wrapping(pView);    // <-- This will repaint.
        }
    }
}

void drte_engine_set_active_line_style(drte_engine* pEngine, drte_style_token styleToken)
{
    if (pEngine == NULL) {
        return;
    }

    uint8_t styleSlot = drte_engine__get_style_slot(pEngine, styleToken);
    if (styleSlot == DRTE_INVALID_STYLE_SLOT) {
        return;
    }

    if (pEngine->activeLineStyleSlot == styleSlot) {
        return; // Nothing has changed.
    }

    pEngine->activeLineStyleSlot = styleSlot;
    drte_engine__refresh(pEngine);
}

void drte_engine_set_cursor_style(drte_engine* pEngine, drte_style_token styleToken)
{
    if (pEngine == NULL) {
        return;
    }

    uint8_t styleSlot = drte_engine__get_style_slot(pEngine, styleToken);
    if (styleSlot == DRTE_INVALID_STYLE_SLOT) {
        return;
    }

    if (pEngine->cursorStyleSlot == styleSlot) {
        return; // Nothing has changed.
    }

    pEngine->cursorStyleSlot = styleSlot;
    drte_engine__refresh(pEngine);
}

void drte_engine_set_line_numbers_style(drte_engine* pEngine, drte_style_token styleToken)
{
    if (pEngine == NULL) {
        return;
    }

    uint8_t styleSlot = drte_engine__get_style_slot(pEngine, styleToken);
    if (styleSlot == DRTE_INVALID_STYLE_SLOT) {
        return;
    }

    if (pEngine->lineNumbersStyleSlot == styleSlot) {
        return; // Nothing has changed.
    }

    pEngine->lineNumbersStyleSlot = styleSlot;
    drte_engine__refresh(pEngine);
}


void drte_engine_set_highlighter(drte_engine* pEngine, drte_engine_on_get_next_highlight_proc proc, void* pUserData)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->onGetNextHighlight = proc;
    pEngine->pHighlightUserData = pUserData;

    drte_engine__refresh(pEngine);
}


void drte_engine_set_line_height(drte_engine* pEngine, float lineHeight)
{
    if (pEngine == NULL) {
        return;
    }

    // Don't do anything if nothing is changing.
    if (lineHeight == 0 && (pEngine->flags & DRTE_USE_EXPLICIT_LINE_HEIGHT) == 0) {
        return;
    }

    pEngine->flags |= DRTE_USE_EXPLICIT_LINE_HEIGHT;
    pEngine->lineHeight = lineHeight;

    for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
        drte_view_dirty(pView, drte_view_get_local_rect(pView));
    }
}

float drte_engine_get_line_height(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return pEngine->lineHeight;
}



uint32_t drte_engine_get_utf32(drte_engine* pEngine, size_t characterIndex)
{
    if (pEngine == NULL) {
        return 0;
    }

    // TODO: Handle UTF-8 properly.
    return pEngine->text[characterIndex];
}


void drte_engine_set_text(drte_engine* pEngine, const char* text)
{
    if (pEngine == NULL) {
        return;
    }

    // Remove existing text first.
    if (pEngine->textLength > 0) {
        drte_engine_delete_text(pEngine, 0, pEngine->textLength);
    }

    // Insert new text.
    drte_engine_insert_text(pEngine, text, 0);
}

size_t drte_engine_get_text(drte_engine* pEngine, char* textOut, size_t textOutSize)
{
    if (pEngine == NULL) {
        return 0;
    }

    if (textOut == NULL) {
        return pEngine->textLength;
    }


    if (drte__strcpy_s(textOut, textOutSize, (pEngine->text != NULL) ? pEngine->text : "") == 0) {
        return pEngine->textLength;
    }

    return 0;   // Error with strcpy_s().
}


void drte_engine_set_on_dirty(drte_engine* pEngine, drte_engine_on_dirty_proc proc)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->onDirty = proc;
}

void drte_engine_set_on_text_changed(drte_engine* pEngine, drte_engine_on_text_changed_proc proc)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->onTextChanged = proc;
}

void drte_engine_set_on_undo_point_changed(drte_engine* pEngine, drte_engine_on_undo_point_changed_proc proc)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->onUndoPointChanged = proc;
}

void drte_engine_set_on_cursor_move(drte_engine* pEngine, drte_engine_on_cursor_move_proc proc)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->onCursorMove = proc;
}



void drte_engine_set_cursor_blink_rate(drte_engine* pEngine, unsigned int blinkRateInMilliseconds)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->cursorBlinkRate = blinkRateInMilliseconds;
}

unsigned int drte_engine_get_cursor_blink_rate(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return pEngine->cursorBlinkRate;
}

void drte_engine_reset_cursor_blinks(drte_engine* pEngine)
{
    if (pEngine == NULL) return;
    pEngine->timeToNextCursorBlink = pEngine->cursorBlinkRate;
    pEngine->isCursorBlinkOn = true;
}



bool drte_engine_insert_character(drte_engine* pEngine, size_t insertIndex, uint32_t utf32)
{
    // TODO: Do a proper UTF-32 -> UTF-8 conversion.
    char utf8[16];
    utf8[0] = (char)utf32;
    utf8[1] = '\0';

    return drte_engine_insert_text(pEngine, utf8, insertIndex);
}

bool drte_engine_delete_character(drte_engine* pEngine, size_t iChar)
{
    return drte_engine_delete_text(pEngine, iChar, iChar+1);
}

bool drte_engine_insert_text(drte_engine* pEngine, const char* text, size_t insertIndex)
{
    if (pEngine == NULL || text == NULL) {
        return false;
    }

    if (insertIndex > pEngine->textLength) {
        return false;
    }

    size_t newTextLength = strlen(text);
    if (newTextLength == 0) {
        return false;
    }

    // We need to get the index of the line that's being inserted so we can know how to update the internal line cache.
    size_t iLine = drte_line_cache_find_line_by_character(pEngine->pUnwrappedLines, insertIndex);


    // TODO: Add proper support for UTF-8.
    char* pOldText = pEngine->text;
    char* pNewText = (char*)malloc(pEngine->textLength + newTextLength + 1);   // +1 for the new character and +1 for the null terminator.

    if (insertIndex > 0) {
        memcpy(pNewText, pOldText, insertIndex);
    }


    size_t linesAddedCount = 0;

    char* dst = pNewText + insertIndex;
    const char* src = text;
    while (*src != '\0') {
        if (src[0] == '\n') {
            linesAddedCount += 1;
        } else if (src[0] == '\r' && src[1] == '\n') {
            linesAddedCount += 1;

            *dst++ = *src;
            src += 1;
        }

        *dst++ = *src;
        src += 1;
    }

    if (insertIndex < pEngine->textLength) {
        memcpy(pNewText + insertIndex + newTextLength, pOldText + insertIndex, pEngine->textLength - insertIndex);
    }

    pEngine->textLength += newTextLength;
    pEngine->text = pNewText;
    pNewText[pEngine->textLength] = '\0';

    free(pOldText);


    // Adjust lines.
    if (linesAddedCount > 0) {
        if (!drte_line_cache_insert_lines(pEngine->pUnwrappedLines, iLine+1, linesAddedCount, newTextLength)) {
            return false;
        }

        size_t iRunningChar;
        for (size_t i = iLine+1; i <= iLine + linesAddedCount; ++i) {
            iRunningChar = drte_line_cache_get_line_first_character(pEngine->pUnwrappedLines, i-1);
            while (pEngine->text[iRunningChar] != '\0') {
                if (pEngine->text[iRunningChar] == '\n') {
                    iRunningChar += 1;
                    break;
                } else if (pEngine->text[iRunningChar] == '\r' && pEngine->text[iRunningChar+1] == '\n') {
                    iRunningChar += 2;
                    break;
                }

                iRunningChar += 1;
            }

            drte_line_cache_set_line_first_character(pEngine->pUnwrappedLines, i, iRunningChar);
        }
    } else {
        // No new lines were added, but we still need to update the character positions of the line cache.
        drte_line_cache_offset_lines(pEngine->pUnwrappedLines, iLine+1, newTextLength);
    }



    // Add the change to the prepared state.
    if (pEngine->hasPreparedUndoState) {
        drte_engine__push_text_change_to_prepared_undo_state(pEngine, drte_undo_change_type_insert, insertIndex, insertIndex + newTextLength, text);
    }


    // Cursors and selections after this cursor need to be updated.
    for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
        for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
            if (pView->pCursors[iCursor].iCharAbs >= insertIndex) {
                drte_view_move_cursor_to_character(pView, iCursor, pView->pCursors[iCursor].iCharAbs + newTextLength);
            }
        }

        // As with cursors, selections need to be updated too.
        for (size_t iSelection = 0; iSelection < pView->selectionCount; ++iSelection) {
            drte_region selection = drte_region_normalize(pView->pSelections[iSelection]);
            if (selection.iCharBeg > insertIndex) {
                pView->pSelections[iSelection].iCharBeg += newTextLength;
                pView->pSelections[iSelection].iCharEnd += newTextLength;
            } else {
                if (selection.iCharEnd >= insertIndex) {
                    pView->pSelections[iSelection].iCharEnd += newTextLength;
                }
            }
        }
    }


    // Refresh the lines if line wrap is enabled.
    // TODO: Optimize this.
    for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
        if (drte_view_is_word_wrap_enabled(pView)) {
            drte_view__refresh_word_wrapping(pView);    // <-- This will repaint.
        } else {
            drte_view_dirty(pView, drte_view_get_local_rect(pView));
        }
    }


    if (pEngine->onTextChanged) {
        pEngine->onTextChanged(pEngine);
    }

    return true;
}

bool drte_engine_delete_text(drte_engine* pEngine, size_t iFirstCh, size_t iLastChPlus1)
{
    if (pEngine == NULL || iLastChPlus1 == iFirstCh) {
        return false;
    }

    if (iFirstCh > pEngine->textLength) {
        iFirstCh = pEngine->textLength;
    }

    if (iLastChPlus1 > pEngine->textLength) {
        iLastChPlus1 = pEngine->textLength;
    }


    if (iFirstCh > iLastChPlus1) {
        size_t temp = iFirstCh;
        iFirstCh = iLastChPlus1;
        iLastChPlus1 = temp;
    }


    // We need to get the index of the line that's being inserted so we can know how to update the internal line cache.
    size_t iLine = drte_line_cache_find_line_by_character(pEngine->pUnwrappedLines, iFirstCh);

    size_t linesRemovedCount = 0;
    for (size_t iChar = iFirstCh; iChar < iLastChPlus1; ++iChar) {
        if (pEngine->text[iChar] == '\n') {
            linesRemovedCount += 1;
        } else if (pEngine->text[iChar] == '\r' && pEngine->text[iChar+1] == '\n') {
            linesRemovedCount += 1;
            iChar += 1;
        }
    }


    size_t bytesToRemove = iLastChPlus1 - iFirstCh;
    if (bytesToRemove > 0)
    {
        // Update cursors and selections _before_ deleting the text.
        size_t iCharBeg = iFirstCh;
        size_t iCharEnd = iLastChPlus1;
        for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
            for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
                size_t iCursorChar = pView->pCursors[iCursor].iCharAbs;
                if (iCursorChar > iCharBeg && iCursorChar <= iCharEnd) {
                    drte_view_move_cursor_to_character(pView, iCursor, iCharBeg);
                } else {
                    if (iCursorChar >= iCharEnd) {
                        drte_view_move_cursor_to_character(pView, iCursor, iCursorChar - (iCharEnd - iCharBeg));
                    }
                }
            }

            // <---> = selection
            // |---| = selectionToDelete
            for (size_t iSelection = 0; iSelection < pView->selectionCount; ++iSelection) {
                drte_region selection = drte_region_normalize(pView->pSelections[iSelection]);
                if (selection.iCharBeg < iCharBeg) {
                    if (selection.iCharEnd > iCharBeg) {
                        if (selection.iCharEnd < iCharEnd) {
                            // <---|--->---|
                            selection.iCharEnd = iCharBeg;
                        } else {
                            // <---|---|--->
                            selection.iCharEnd -= iCharEnd - iCharBeg;
                        }
                    }
                } else {
                    if (selection.iCharBeg < iCharEnd) {
                        if (selection.iCharEnd < iCharEnd) {
                            // |---<--->---|
                            selection.iCharBeg = selection.iCharEnd = iCharBeg;
                        } else {
                            // |---<---|--->
                            selection.iCharEnd -= (iCharEnd - selection.iCharBeg);
                            selection.iCharBeg  = iCharBeg;
                        }
                    } else {
                        selection.iCharBeg -= (iCharEnd - iCharBeg);
                        selection.iCharEnd -= (iCharEnd - iCharBeg);
                    }
                }

                if (pView->pSelections[iSelection].iCharBeg < pView->pSelections[iSelection].iCharEnd) {
                    pView->pSelections[iSelection].iCharBeg = selection.iCharBeg;
                    pView->pSelections[iSelection].iCharEnd = selection.iCharEnd;
                } else {
                    pView->pSelections[iSelection].iCharBeg = selection.iCharEnd;
                    pView->pSelections[iSelection].iCharEnd = selection.iCharBeg;
                }
            }
        }



        // Add the change to the prepared state.
        if (pEngine->hasPreparedUndoState) {
            drte_engine__push_text_change_to_prepared_undo_state(pEngine, drte_undo_change_type_delete, iFirstCh, iLastChPlus1, pEngine->text + iFirstCh);
        }


        memmove(pEngine->text + iFirstCh, pEngine->text + iLastChPlus1, pEngine->textLength - iLastChPlus1);
        pEngine->textLength -= bytesToRemove;
        pEngine->text[pEngine->textLength] = '\0';

        if (linesRemovedCount > 0) {
            if (!drte_line_cache_remove_lines(pEngine->pUnwrappedLines, iLine+1, linesRemovedCount, bytesToRemove)) {
                return false;
            }
        } else {
            // No lines were removed, but we still need to update the character positions of the line cache.
            drte_line_cache_offset_lines_negative(pEngine->pUnwrappedLines, iLine+1, bytesToRemove);
        }


        // Refresh the lines if line wrap is enabled.
        // TODO: Optimize this.
        for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
            if (drte_view_is_word_wrap_enabled(pView)) {
                drte_view__refresh_word_wrapping(pView);    // <-- This will repaint.
            } else {
                // After line each cursor is sitting on may have changed.
                for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
                    drte_view_move_cursor_to_character(pView, iCursor, pView->pCursors[iCursor].iCharAbs);
                }

                drte_view_dirty(pView, drte_view_get_local_rect(pView));
            }
        }
        

        if (pEngine->onTextChanged) {
            pEngine->onTextChanged(pEngine);
        }

        return true;
    }

    return false;
}


bool drte_engine_get_start_of_word_containing_character(drte_engine* pEngine, size_t iChar, size_t* pWordBegOut)
{
    if (pEngine == NULL) {
        return false;
    }

    if (iChar > 0) {
        iChar -= 1;

        // Skip whitespace.
        if (drte_is_whitespace(pEngine->text[iChar])) {
            while (iChar > 0) {
                if (!drte_is_whitespace(pEngine->text[iChar])) {
                    break;
                }

                iChar -= 1;
            }
        }

        if (!drte_is_symbol_or_whitespace(pEngine->text[iChar])) {
            while (iChar > 0) {
                uint32_t c = pEngine->text[iChar-1];
                if (drte_is_symbol_or_whitespace(c)) {
                    break;
                }

                iChar -= 1;
            }
        }
    }

    if (pWordBegOut) *pWordBegOut = iChar;
    return true;
}


bool drte_engine_get_start_of_next_word_from_character(drte_engine* pEngine, size_t iChar, size_t* pWordBegOut)
{
    if (pEngine == NULL) {
        return false;
    }

    while (pEngine->text[iChar] != '\0' && pEngine->text[iChar] != '\n' && !(pEngine->text[iChar] == '\r' && pEngine->text[iChar+1])) {
        uint32_t c = pEngine->text[iChar];
        if (!drte_is_whitespace(c)) {
            break;
        }

        iChar += 1;
    }

    if (pWordBegOut) *pWordBegOut = iChar;
    return true;
}

bool drte_engine_get_end_of_word_containing_character(drte_engine* pEngine, size_t iChar, size_t* pWordEndOut)
{
    if (pEngine == NULL) {
        return false;
    }

    if (!drte_is_symbol_or_whitespace(pEngine->text[iChar])) {
        while (pEngine->text[iChar] != '\0' && pEngine->text[iChar] != '\n' && !(pEngine->text[iChar] == '\r' && pEngine->text[iChar+1])) {
            uint32_t c = pEngine->text[iChar];
            if (drte_is_symbol_or_whitespace(c)) {
                break;
            }

            iChar += 1;
        }
    } else {
        if (pEngine->text[iChar] != '\n' && !(pEngine->text[iChar] == '\r' && pEngine->text[iChar+1])) {
            iChar += 1;
        }
    }

    if (pWordEndOut) *pWordEndOut = iChar;
    return true;
}

bool drte_engine_get_word_containing_character(drte_engine* pEngine, size_t iChar, size_t* pWordBegOut, size_t* pWordEndOut)
{
    if (pEngine == NULL) {
        return false;
    }

    bool moveToStartOfNextWord = false;

    // Move to the start of the word if we're not already there.
    if (iChar > 0) {
        uint32_t c = pEngine->text[iChar];
        uint32_t cprev = pEngine->text[iChar-1];

        if (c == '\0') {
            if (pWordBegOut) *pWordBegOut = pEngine->textLength;
            if (pWordEndOut) *pWordEndOut = pEngine->textLength;
            return false;
        }

        if (!drte_is_whitespace(c) && !drte_is_whitespace(cprev) && !drte_is_symbol_or_whitespace(c)) {
            drte_engine_get_start_of_word_containing_character(pEngine, iChar, &iChar);
        } else if (drte_is_whitespace(c) && drte_is_whitespace(cprev)) {
            size_t iLineCharBeg = drte_line_cache_get_line_first_character(pEngine->pUnwrappedLines, drte_line_cache_find_line_by_character(pEngine->pUnwrappedLines, iChar));
            while (iChar > 0 && iChar > iLineCharBeg) {
                if (!drte_is_whitespace(pEngine->text[iChar-1])) {
                    break;
                }
                iChar -= 1;
            }

            moveToStartOfNextWord = true;
        }
    }

    size_t iWordCharEnd;
    bool result;
    if (moveToStartOfNextWord) {
        result = drte_engine_get_start_of_next_word_from_character(pEngine, iChar, &iWordCharEnd);
    } else {
        result = drte_engine_get_end_of_word_containing_character(pEngine, iChar, &iWordCharEnd);
    }

    if (pWordBegOut) *pWordBegOut = iChar;
    if (pWordEndOut) *pWordEndOut = iWordCharEnd;

    if (result) {
        result = iChar < iWordCharEnd;
    }

    return result;
}




bool drte_engine__capture_and_push_undo_state__user_data(drte_engine* pEngine, drte_stack_buffer* pStack)
{
    assert(pEngine != NULL);
    assert(pStack != NULL);

    // Make room in the undo buffer for the application-defined prepared state.
    size_t preparedStateUserDataSize = 0;
    if (pEngine->onGetUndoState) {
        preparedStateUserDataSize = pEngine->onGetUndoState(pEngine, NULL);
    }

    void* pPreparedUserData = drte_stack_buffer_alloc(pStack, sizeof(size_t) + preparedStateUserDataSize);     // <-- sizeof(size_t) is for storing the size of the user data.
    if (pPreparedUserData == NULL) {
        return false;
    }

    if (preparedStateUserDataSize > 0) {
        *(size_t*)pPreparedUserData = preparedStateUserDataSize;
        if (pEngine->onGetUndoState(pEngine, (uint8_t*)pPreparedUserData + sizeof(size_t)) != preparedStateUserDataSize) {
            return false;   // Inconsistent data size returned by onGetUndoState().
        }
    }

    return true;
}

bool drte_engine__capture_and_push_undo_state__cursors(drte_engine* pEngine, drte_stack_buffer* pStack, drte_view* pView)
{
    assert(pEngine != NULL);
    assert(pStack != NULL);
    assert(pView != NULL);

    (void)pEngine;

    size_t sizeInBytes =
        sizeof(pView->cursorCount) +
        sizeof(drte_cursor) * pView->cursorCount;

    uint8_t* pData = drte_stack_buffer_alloc(pStack, sizeInBytes);
    if (pData == NULL) {
        return false;
    }

    memcpy(pData, &pView->cursorCount, sizeof(pView->cursorCount));
    memcpy(pData + sizeof(pView->cursorCount), pView->pCursors, sizeof(drte_cursor) * pView->cursorCount);

    return true;
}

bool drte_engine__capture_and_push_undo_state__selections(drte_engine* pEngine, drte_stack_buffer* pStack, drte_view* pView)
{
    assert(pEngine != NULL);
    assert(pStack != NULL);
    assert(pView != NULL);

    (void)pEngine;

    size_t sizeInBytes =
        sizeof(pView->selectionCount) +
        sizeof(drte_region) * pView->selectionCount;

    uint8_t* pData = drte_stack_buffer_alloc(pStack, sizeInBytes);
    if (pData == NULL) {
        return false;
    }

    memcpy(pData, &pView->selectionCount, sizeof(pView->selectionCount));
    memcpy(pData + sizeof(pView->selectionCount), pView->pSelections, sizeof(drte_region) * pView->selectionCount);

    return true;
}

bool drte_engine__capture_and_push_undo_state__view(drte_engine* pEngine, drte_stack_buffer* pStack, drte_view* pView)
{
    if (pEngine == NULL || pStack == NULL || pView == NULL) {
        return false;
    }

    size_t sizeInBytes = 
        sizeof(pView->_id);

    uint8_t* pData = drte_stack_buffer_alloc(pStack, sizeInBytes);
    if (pData == NULL) {
        return false;
    }

    memcpy(pData, &pView->_id, sizeof(pView->_id));

    if (!drte_engine__capture_and_push_undo_state__cursors(pEngine, pStack, pView)) {
        return false;
    }

    if (!drte_engine__capture_and_push_undo_state__selections(pEngine, pStack, pView)) {
        return false;
    }

    return true;
}

bool drte_engine__capture_and_push_undo_state(drte_engine* pEngine, drte_stack_buffer* pStack)
{
    if (pEngine == NULL || pStack == NULL) {
        return false;
    }

    size_t oldStackPtr = drte_stack_buffer_get_stack_ptr(pStack);

    if (!drte_engine__capture_and_push_undo_state__user_data(pEngine, pStack)) {
        drte_stack_buffer_set_stack_ptr(pStack, oldStackPtr);
        return false;
    }


    // There is state that needs to be captured for each view. We'll need to view count to begin with.
    size_t *pViewCount = (size_t*)drte_stack_buffer_alloc(pStack, sizeof(size_t));
    if (pViewCount == NULL) {
        drte_stack_buffer_set_stack_ptr(pStack, oldStackPtr);
        return false;
    }

    *pViewCount = drte_engine_get_view_count(pEngine);


    // Each view...
    for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
        if (!drte_engine__capture_and_push_undo_state__view(pEngine, pStack, pView)) {
            drte_stack_buffer_set_stack_ptr(pStack, oldStackPtr);
            return false;
        }
    }

    return true;
}

size_t drte_engine__get_prev_undo_data_offset(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    if (pEngine->undoBuffer.pBuffer == NULL || pEngine->undoStackCount == 0) {
        return 0;
    }

    return *((size_t*)drte_stack_buffer_get_data_ptr(&pEngine->undoBuffer, pEngine->currentUndoDataOffset + 0));
}

size_t drte_engine__get_next_undo_data_offset(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    if (pEngine->undoBuffer.pBuffer == NULL || pEngine->undoStackCount == 0) {
        return 0;
    }

    return *((size_t*)drte_stack_buffer_get_data_ptr(&pEngine->undoBuffer, pEngine->currentUndoDataOffset + sizeof(size_t)));
}


typedef struct drte_undo_state_view_info drte_undo_state_view_info;
struct drte_undo_state_view_info
{
    size_t id;
    size_t cursorCount;
    const drte_cursor* pCursors;
    size_t selectionCount;
    const drte_region* pSelections;
};

typedef struct
{
    size_t prevUndoDataOffset;
    size_t nextUndoDataOffset;
    size_t oldStateLocalOffset;
    size_t newStateLocalOffset;
    size_t textChangesOffset;

    size_t textChangeCount;
    const uint8_t* pTextChanges;

    const uint8_t* pOldState;
    const uint8_t* pNewState;

    struct
    {
        size_t userDataSize;
        const void* pUserData;
        size_t viewCount;
        size_t firstViewOffset;
    } oldState;

    struct
    {
        size_t userDataSize;
        const void* pUserData;
        size_t viewCount;
        size_t firstViewOffset;
    } newState;
} drte_undo_state_info;

void drte_engine__breakdown_undo_state_view_info(const uint8_t* pUndoData, drte_undo_state_view_info* pInfoOut, size_t* pBytesReadOut)
{
    size_t runningOffset = 0;
    pInfoOut->id             = *(const size_t*)     (pUndoData + runningOffset); runningOffset += sizeof(size_t);
    pInfoOut->cursorCount    = *(const size_t*)     (pUndoData + runningOffset); runningOffset += sizeof(size_t);
    pInfoOut->pCursors       =  (const drte_cursor*)(pUndoData + runningOffset); runningOffset += sizeof(drte_cursor)*pInfoOut->cursorCount;
    pInfoOut->selectionCount = *(const size_t*)     (pUndoData + runningOffset); runningOffset += sizeof(size_t);
    pInfoOut->pSelections    =  (const drte_region*)(pUndoData + runningOffset); runningOffset += sizeof(drte_region)*pInfoOut->selectionCount;

    *pBytesReadOut = runningOffset;
}

DRTE_INLINE void drte_engine__breakdown_undo_state_info(const uint8_t* pUndoData, drte_undo_state_info* pInfoOut)
{
    assert(pInfoOut != NULL);

    drte_undo_state_info result;
    result.prevUndoDataOffset  = *(const size_t*)(pUndoData + sizeof(size_t)*0);
    result.nextUndoDataOffset  = *(const size_t*)(pUndoData + sizeof(size_t)*1);
    result.oldStateLocalOffset = *(const size_t*)(pUndoData + sizeof(size_t)*2);
    result.newStateLocalOffset = *(const size_t*)(pUndoData + sizeof(size_t)*3);
    result.textChangesOffset   = *(const size_t*)(pUndoData + sizeof(size_t)*4);

    result.textChangeCount = *(const size_t*)(pUndoData + result.textChangesOffset);
    result.pTextChanges = pUndoData + result.textChangesOffset + sizeof(size_t);

    result.pOldState = pUndoData + result.oldStateLocalOffset;
    result.pNewState = pUndoData + result.newStateLocalOffset;

    size_t runningOffset = 0;
    result.oldState.userDataSize = *(const size_t*)(result.pOldState + runningOffset); runningOffset += sizeof(size_t);
    result.oldState.pUserData = result.pOldState + runningOffset;                      runningOffset += result.oldState.userDataSize;
    result.oldState.viewCount = *(const size_t*)(result.pOldState + runningOffset);    runningOffset += sizeof(size_t);
    result.oldState.firstViewOffset = result.oldStateLocalOffset + runningOffset;

    runningOffset = 0;
    result.newState.userDataSize = *(const size_t*)(result.pNewState + runningOffset); runningOffset += sizeof(size_t);
    result.newState.pUserData = result.pNewState + runningOffset;                      runningOffset += result.newState.userDataSize;
    result.newState.viewCount = *(const size_t*)(result.pNewState + runningOffset);    runningOffset += sizeof(size_t);
    result.newState.firstViewOffset = result.newStateLocalOffset + runningOffset;

    *pInfoOut = result;
}


bool drte_engine_prepare_undo_point(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    // If we have a previously prepared state we'll need to clear it.
    if (pEngine->hasPreparedUndoState) {
        drte_stack_buffer_set_stack_ptr(&pEngine->preparedUndoState, 0);
    }

    // If the prepared undo point is cancelled later on we'll need to free the memory at some point.
    pEngine->preparedUndoTextChangeCount = 0;
    pEngine->hasPreparedUndoState = drte_engine__capture_and_push_undo_state(pEngine, &pEngine->preparedUndoState);
    if (!pEngine->hasPreparedUndoState) {
        return false;   // <-- An error occured while trying to capture the undo state.
    }

    // Allocate space for the text change count and initialize to 0 to begin with.
    pEngine->preparedUndoTextChangesOffset = drte_stack_buffer_get_stack_ptr(&pEngine->preparedUndoState);
    void* pTextChangeCount = drte_stack_buffer_alloc(&pEngine->preparedUndoState, sizeof(size_t));
    if (pTextChangeCount == NULL) {
        pEngine->hasPreparedUndoState = false;
        return false;
    }

    *(size_t*)pTextChangeCount = 0;


    return true;
}

bool drte_engine_commit_undo_point(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    // The undo point must have been prepared earlier.
    if (!pEngine->hasPreparedUndoState) {
        return false;
    }



    // The undo buffer needs to be trimmed.
    if (drte_engine_get_redo_points_remaining_count(pEngine) > 0) {
        drte_stack_buffer_set_stack_ptr(&pEngine->undoBuffer, pEngine->currentRedoDataOffset);
        if (pEngine->onUndoStackTrimmed) pEngine->onUndoStackTrimmed(pEngine);
    }

    pEngine->undoStackCount = pEngine->iUndoState;


    // The data to push onto the stack is done in 3 main parts. The first part is the header which stores the size and offsets of each major section. The
    // second part is the prepared data. The third part is the state at the time of comitting.
    
    // Header.
    size_t headerSize =
        sizeof(size_t) +    // Prev undo data offset.
        sizeof(size_t) +    // Next undo data offset.
        sizeof(size_t) +    // Old state local offset.
        sizeof(size_t) +    // New state local offset.
        sizeof(size_t);     // The offset of the text changes.
    size_t headerOffset = drte_stack_buffer_get_stack_ptr(&pEngine->undoBuffer);

    if (drte_stack_buffer_alloc(&pEngine->undoBuffer, headerSize) == NULL) {
        drte_stack_buffer_set_stack_ptr(&pEngine->undoBuffer, pEngine->currentUndoDataOffset);
        return false;
    }


    // Prepared data.
    size_t preparedDataSize = drte_stack_buffer_get_stack_ptr(&pEngine->preparedUndoState);
    size_t preparedDataOffset = drte_stack_buffer_get_stack_ptr(&pEngine->undoBuffer);

    if (drte_stack_buffer_alloc(&pEngine->undoBuffer, preparedDataSize) == NULL) {
        drte_stack_buffer_set_stack_ptr(&pEngine->undoBuffer, pEngine->currentUndoDataOffset);
        return false;
    }

    memcpy(drte_stack_buffer_get_data_ptr(&pEngine->undoBuffer, preparedDataOffset), drte_stack_buffer_get_data_ptr(&pEngine->preparedUndoState, 0), preparedDataSize);


    // Committed data.
    size_t committedDataOffset = drte_stack_buffer_get_stack_ptr(&pEngine->undoBuffer);

    if (!drte_engine__capture_and_push_undo_state(pEngine, &pEngine->undoBuffer)) {
        drte_stack_buffer_set_stack_ptr(&pEngine->undoBuffer, pEngine->currentUndoDataOffset);
        return false;
    }


    size_t prevUndoDataOffset = pEngine->currentUndoDataOffset;
    size_t nextUndoDataOffset = drte_stack_buffer_get_stack_ptr(&pEngine->undoBuffer);


    // The header needs to be written last.
    *((size_t*)drte_stack_buffer_get_data_ptr(&pEngine->undoBuffer, headerOffset + sizeof(size_t)*0)) = prevUndoDataOffset;
    *((size_t*)drte_stack_buffer_get_data_ptr(&pEngine->undoBuffer, headerOffset + sizeof(size_t)*1)) = nextUndoDataOffset;
    *((size_t*)drte_stack_buffer_get_data_ptr(&pEngine->undoBuffer, headerOffset + sizeof(size_t)*2)) = preparedDataOffset  - headerOffset;
    *((size_t*)drte_stack_buffer_get_data_ptr(&pEngine->undoBuffer, headerOffset + sizeof(size_t)*3)) = committedDataOffset - headerOffset;
    *((size_t*)drte_stack_buffer_get_data_ptr(&pEngine->undoBuffer, headerOffset + sizeof(size_t)*4)) = headerSize + pEngine->preparedUndoTextChangesOffset;

    pEngine->currentUndoDataOffset = headerOffset;
    pEngine->currentRedoDataOffset = headerOffset;

    drte_stack_buffer_set_stack_ptr(&pEngine->preparedUndoState, 0);
    pEngine->hasPreparedUndoState = false;
    pEngine->preparedUndoTextChangeCount = 0;


    pEngine->undoStackCount += 1;
    pEngine->iUndoState += 1;

    if (pEngine->onUndoPointChanged) {
        pEngine->onUndoPointChanged(pEngine, pEngine->iUndoState);
    }

    return true;
}

bool drte_engine_undo(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    if (drte_engine_get_undo_points_remaining_count(pEngine) > 0) {
        const void* pUndoDataPtr = drte_stack_buffer_get_data_ptr(&pEngine->undoBuffer, pEngine->currentUndoDataOffset);
        if (pUndoDataPtr == NULL) {
            return false;
        }

        drte_engine__apply_undo_state(pEngine, pUndoDataPtr);
        pEngine->iUndoState -= 1;

        if (pEngine->onUndoPointChanged) {
            pEngine->onUndoPointChanged(pEngine, pEngine->iUndoState);
        }


        pEngine->currentRedoDataOffset = pEngine->currentUndoDataOffset;
        pEngine->currentUndoDataOffset = drte_engine__get_prev_undo_data_offset(pEngine);
        return true;
    }

    return false;
}

bool drte_engine_redo(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    if (drte_engine_get_redo_points_remaining_count(pEngine) > 0) {
        const void* pUndoDataPtr = drte_stack_buffer_get_data_ptr(&pEngine->undoBuffer, pEngine->currentRedoDataOffset);
        if (pUndoDataPtr == NULL) {
            return false;
        }

        drte_engine__apply_redo_state(pEngine, pUndoDataPtr);
        pEngine->iUndoState += 1;

        if (pEngine->onUndoPointChanged) {
            pEngine->onUndoPointChanged(pEngine, pEngine->iUndoState);
        }

        pEngine->currentUndoDataOffset = pEngine->currentRedoDataOffset;
        if (drte_engine_get_redo_points_remaining_count(pEngine) > 0) {
            pEngine->currentRedoDataOffset = drte_engine__get_next_undo_data_offset(pEngine);
        }

        return true;
    }

    return false;
}

unsigned int drte_engine_get_undo_points_remaining_count(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return pEngine->iUndoState;
}

unsigned int drte_engine_get_redo_points_remaining_count(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    if (pEngine->undoStackCount > 0)
    {
        assert(pEngine->iUndoState <= pEngine->undoStackCount);
        return pEngine->undoStackCount - pEngine->iUndoState;
    }

    return 0;
}

void drte_engine_clear_undo_stack(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    drte_stack_buffer_set_stack_ptr(&pEngine->undoBuffer, 0);
    drte_stack_buffer_set_stack_ptr(&pEngine->preparedUndoState, 0);

    pEngine->undoStackCount = 0;

    if (pEngine->iUndoState > 0) {
        pEngine->iUndoState = 0;

        if (pEngine->onUndoPointChanged) {
            pEngine->onUndoPointChanged(pEngine, pEngine->iUndoState);
        }
    }
}



void drte_engine_set_on_paint_text(drte_engine* pEngine, drte_engine_on_paint_text_proc proc)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->onPaintText = proc;
}

void drte_engine_set_on_paint_rect(drte_engine* pEngine, drte_engine_on_paint_rect_proc proc)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->onPaintRect = proc;
}


void drte_engine_step(drte_engine* pEngine, unsigned int milliseconds)
{
    if (pEngine == NULL || milliseconds == 0) {
        return;
    }

    if (pEngine->timeToNextCursorBlink < milliseconds)
    {
        pEngine->isCursorBlinkOn = !pEngine->isCursorBlinkOn;
        pEngine->timeToNextCursorBlink = pEngine->cursorBlinkRate;

        for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
            drte_view_begin_dirty(pView);
            for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
                drte_view_dirty(pView, drte_view_get_cursor_rect(pView, iCursor));
            }
            drte_view_end_dirty(pView);
        }
    }
    else
    {
        pEngine->timeToNextCursorBlink -= milliseconds;
    }
}



void drte_engine__refresh(drte_engine* pEngine)
{
    assert(pEngine != NULL);

    for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
        drte_view__refresh_word_wrapping(pView);
    }
}



void drte_view__set_cursors(drte_view* pView, size_t cursorCount, const drte_cursor* pCursors)
{
    assert(pView != NULL);

    if (cursorCount > 0) {
        drte_cursor* pNewCursors = (drte_cursor*)realloc(pView->pCursors, cursorCount * sizeof(*pNewCursors));
        if (pNewCursors != NULL) {
            pView->pCursors = pNewCursors;
            for (size_t iCursor = 0; iCursor < cursorCount; ++iCursor) {
                pView->pCursors[iCursor] = pCursors[iCursor];
                drte_view__update_cursor_sticky_position(pView, &pView->pCursors[iCursor]);
            }

            pView->cursorCount = cursorCount;
        }
    } else {
        pView->cursorCount = 0;
    }
}

void drte_view__set_selections(drte_view* pView, size_t selectionCount, const drte_region* pSelections)
{
    assert(pView != NULL);

    if (selectionCount > 0) {
        drte_region* pNewSelections = (drte_region*)realloc(pView->pSelections, selectionCount * sizeof(*pNewSelections));
        if (pNewSelections != NULL) {
            pView->pSelections = pNewSelections;
            for (size_t iSelection = 0; iSelection < selectionCount; ++iSelection) {
                pView->pSelections[iSelection] = pSelections[iSelection];
            }

            pView->selectionCount = selectionCount;
        }
    } else {
        pView->selectionCount = 0;
    }
}

void drte_engine__apply_text_changes_reversed(drte_engine* pEngine, size_t changeCount, const uint8_t* pData)
{
    // Each item in pData is formatted as:
    //   type, iCharBeg, iCharEnd, text (null terminated).

    assert(pEngine != NULL);
    assert(pData != NULL);

    if (changeCount == 0) {
        return;
    }

    drte_undo_change_type type = *(drte_undo_change_type*)(pData + 0);
    size_t iCharBeg = *(size_t*)(pData + sizeof(drte_undo_change_type));
    size_t iCharEnd = *(size_t*)(pData + sizeof(drte_undo_change_type) + sizeof(size_t));
    const char* text = (const char*)(pData + sizeof(drte_undo_change_type) + sizeof(size_t) + sizeof(size_t));
    size_t sizeInBytes = sizeof(drte_undo_change_type) + sizeof(size_t) + sizeof(size_t) + (iCharEnd - iCharBeg) + 1;

    // We need to do the next changes before doing this one. This is how we do it in reverse.
    drte_engine__apply_text_changes_reversed(pEngine, changeCount - 1, pData + drte_round_up(sizeInBytes, DRTE_STACK_BUFFER_ALIGNMENT));

    // Now we apply the change, remembering to transform inserts into deletes and vice versa.
    if (type == drte_undo_change_type_insert) {
        drte_engine_delete_text(pEngine, iCharBeg, iCharEnd);
    } else {
        drte_engine_insert_text(pEngine, text, iCharBeg);
    }
}

void drte_engine__apply_text_changes(drte_engine* pEngine, size_t changeCount, const uint8_t* pData)
{
    // Each item in pData is formatted as:
    //   type, iCharBeg, iCharEnd, text (null terminated).

    assert(pEngine != NULL);
    assert(pData != NULL);

    for (size_t i = 0; i < changeCount; ++i) {
        drte_undo_change_type type = *(drte_undo_change_type*)(pData + 0);
        size_t iCharBeg = *(size_t*)(pData + sizeof(drte_undo_change_type));
        size_t iCharEnd = *(size_t*)(pData + sizeof(drte_undo_change_type) + sizeof(size_t));
        const char* text = (const char*)(pData + sizeof(drte_undo_change_type) + sizeof(size_t) + sizeof(size_t));
        size_t sizeInBytes = sizeof(drte_undo_change_type) + sizeof(size_t) + sizeof(size_t) + (iCharEnd - iCharBeg) + 1;

        // Now we apply the change, remembering to transform inserts into deletes and vice versa.
        if (type == drte_undo_change_type_insert) {
            drte_engine_insert_text(pEngine, text, iCharBeg);
        } else {
            drte_engine_delete_text(pEngine, iCharBeg, iCharEnd);
        }

        pData += drte_round_up(sizeInBytes, DRTE_STACK_BUFFER_ALIGNMENT);
    }
}

void drte_engine__apply_undo_state(drte_engine* pEngine, const void* pUndoDataPtr)
{
    if (pEngine == NULL) {
        return;
    }

    // Begin dirty.
    for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
        drte_view_begin_dirty(pView);
    }


    drte_undo_state_info state;
    drte_engine__breakdown_undo_state_info(pUndoDataPtr, &state);

    // Text.
    drte_engine__apply_text_changes_reversed(pEngine, state.textChangeCount, state.pTextChanges);


    // For each view with captured state...
    size_t viewDataOffset = state.oldState.firstViewOffset;
    for (size_t iView = 0; iView < state.oldState.viewCount; ++iView) {
        size_t viewDataSize;
        drte_undo_state_view_info viewData;
        drte_engine__breakdown_undo_state_view_info((uint8_t*)pUndoDataPtr + viewDataOffset, &viewData, &viewDataSize);

        // If the view that owns this captured state still exists it needs to have it's selections and cursors restored. Otherwise we just skip it.
        for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
            if (pView->_id == viewData.id) {
                drte_view__set_cursors(pView, viewData.cursorCount, viewData.pCursors);
                drte_view__set_selections(pView, viewData.selectionCount, viewData.pSelections);
                
                for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
                    drte_engine__on_cursor_move(pEngine, pView, iCursor);
                }

                break;
            }
        }

        viewDataOffset += viewDataSize;
    }


    if (pEngine->onTextChanged) {
        pEngine->onTextChanged(pEngine);
    }

    // Application-defined data.
    if (pEngine->onApplyUndoState) {
        pEngine->onApplyUndoState(pEngine, state.oldState.userDataSize, state.oldState.pUserData);
    }


    // End dirty.
    for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
        drte_view_end_dirty(pView);
    }
}

void drte_engine__apply_redo_state(drte_engine* pEngine, const void* pUndoDataPtr)
{
    if (pEngine == NULL) {
        return;
    }

    // Begin dirty.
    for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
        drte_view_begin_dirty(pView);
    }


    drte_undo_state_info state;
    drte_engine__breakdown_undo_state_info(pUndoDataPtr, &state);

    // Text.
    drte_engine__apply_text_changes(pEngine, state.textChangeCount, state.pTextChanges);

    size_t viewDataOffset = state.newState.firstViewOffset;
    for (size_t iView = 0; iView < state.newState.viewCount; ++iView) {
        size_t viewDataSize;
        drte_undo_state_view_info viewData;
        drte_engine__breakdown_undo_state_view_info((uint8_t*)pUndoDataPtr + viewDataOffset, &viewData, &viewDataSize);

        // If the view that owns this captured state still exists it needs to have it's selections and cursors restored. Otherwise we just skip it.
        for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
            if (pView->_id == viewData.id) {
                drte_view__set_cursors(pView, viewData.cursorCount, viewData.pCursors);
                drte_view__set_selections(pView, viewData.selectionCount, viewData.pSelections);
                
                for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
                    drte_engine__on_cursor_move(pEngine, pView, iCursor);
                }

                break;
            }
        }

        viewDataOffset += viewDataSize;
    }


    if (pEngine->onTextChanged) {
        pEngine->onTextChanged(pEngine);
    }

    // Application-defined data.
    if (pEngine->onApplyUndoState) {
        pEngine->onApplyUndoState(pEngine, state.newState.userDataSize, state.newState.pUserData);
    }


    // End dirty.
    for (drte_view* pView = drte_engine_first_view(pEngine); pView != NULL; pView = drte_view_next_view(pView)) {
        drte_view_end_dirty(pView);
    }
}



void drte_engine__on_cursor_move(drte_engine* pEngine, drte_view* pView, size_t cursorIndex)
{
    if (pEngine == NULL) {
        return;
    }

    // When the cursor moves we want to reset the cursor's blink state.
    pEngine->timeToNextCursorBlink = pEngine->cursorBlinkRate;
    pEngine->isCursorBlinkOn = true;

    if (pEngine->onCursorMove) {
        pEngine->onCursorMove(pEngine, pView, cursorIndex);
    }

    drte_view_dirty(pView, drte_view_get_cursor_rect(pView, cursorIndex));  // <-- Is this needed?
}



///////////////////////////////////////////////////////////////////////////////
//
// Views
//
///////////////////////////////////////////////////////////////////////////////

DRTE_INLINE void drte_view__repaint(drte_view* pView)
{
    drte_view_dirty(pView, drte_view_get_local_rect(pView));
}

static float drte_view__get_tab_width_in_pixels(drte_view* pView)
{
    float tabWidth = (float)(pView->pEngine->styles[pView->pEngine->defaultStyleSlot].fontMetrics.spaceWidth * pView->tabSizeInSpaces);
    if (tabWidth <= 0) {
        tabWidth = (float)pView->pEngine->styles[pView->pEngine->defaultStyleSlot].fontMetrics.spaceWidth;
        if (tabWidth <= 0) {
            tabWidth = 4;
        }
    }

    return tabWidth;
}

static void drte_view__refresh_word_wrapping(drte_view* pView)
{
    // When word wrap is enabled we need to recalculate the lines and then repaint. There is no need to do
    // this when word wrap is disabled, but it will need a repaint.
    if (drte_view_is_word_wrap_enabled(pView)) {
        // Make sure the cache is cleared to begin with.
        drte_line_cache_clear(pView->pWrappedLines);

        // Line wrapping is done by simply sub-diving each unwrapped line based on word boundaries.
        size_t lineCount = drte_line_cache_get_line_count(pView->pEngine->pUnwrappedLines);
        for (size_t iLine = 0; iLine < lineCount; ++iLine) {
            size_t iLineCharBeg;
            size_t iLineCharEnd;
            drte_view_get_line_character_range(pView, pView->pEngine->pUnwrappedLines, iLine, &iLineCharBeg, &iLineCharEnd);

            if (iLineCharBeg < iLineCharEnd) {
                float runningWidth = 0;
                while (iLineCharBeg < iLineCharEnd) {
                    drte_line_cache_append_line(pView->pWrappedLines, iLineCharBeg);

                    drte_segment segment;
                    if (!drte_engine__first_segment_on_line(pView, pView->pEngine->pUnwrappedLines, iLine, iLineCharBeg, &segment)) {
                        break;
                    }

                    do
                    {
                        if ((runningWidth + segment.width) > pView->sizeX) {
                            float unused = 0;
                            size_t iChar = iLineCharBeg;
                            if (pView->pEngine->onGetCursorPositionFromPoint) {
                                pView->pEngine->onGetCursorPositionFromPoint(pView->pEngine, drte_engine__get_style_token(pView->pEngine, segment.fgStyleSlot), pView->pEngine->text + segment.iCharBeg, segment.iCharEnd - segment.iCharBeg,
                                    segment.width, pView->sizeX - runningWidth, &unused, &iChar);
                            }

                            size_t iWordCharBeg;
                            size_t iWordCharEnd;
                            if (!drte_engine_get_word_containing_character(pView->pEngine, iLineCharBeg + iChar, &iWordCharBeg, &iWordCharEnd)) {
                                iLineCharBeg = segment.iCharEnd;
                                runningWidth = 0;
                                break;
                            }


                            size_t iPrevLineChar = pView->pWrappedLines->pLines[pView->pWrappedLines->count-1];
                            if (iWordCharBeg <= iPrevLineChar) {
                                iWordCharBeg  = segment.iCharBeg + iChar;   // The word itself is longer than the container which means it needs to be split based on the exact character.
                            }

                            // Always make sure wrapping has at least one character.
                            if (iWordCharBeg == iLineCharBeg) {
                                iWordCharBeg += 1;
                            }

                            iLineCharBeg = iWordCharBeg;
                            runningWidth = 0;
                            break;
                        } else {
                            runningWidth += segment.width;
                            iLineCharBeg = segment.iCharEnd;
                        }
                    } while (drte_engine__next_segment_on_line(pView, &segment));
                }
            } else {
                drte_line_cache_append_line(pView->pWrappedLines, iLineCharBeg);  // <-- Empty line.
            }
        }
    }

    // Cursors need to have their sticky positions refreshed.
    drte_view_begin_dirty(pView);
    {
        for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
            drte_view_move_cursor_to_character(pView, iCursor, drte_view_get_cursor_character(pView, iCursor));
        }

        drte_view__repaint(pView);
    }
    drte_view_end_dirty(pView);
}



drte_view* drte_view_create(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return NULL;
    }

    drte_view* pView = (drte_view*)calloc(1, sizeof(*pView));
    if (pView == NULL) {
        return NULL;
    }

    pView->pEngine = pEngine;
    pView->tabSizeInSpaces = 4;
    pView->cursorWidth = 1;
    
    pView->_id = drte_engine__acquire_view_id(pEngine);
    pView->_accumulatedDirtyRect = drte_make_inside_out_rect();
    pView->pWrappedLines = &pEngine->_unwrappedLines;


    // Attach the view to the start of the list.
    pView->_pPrevView = NULL;
    pView->_pNextView = pEngine->pRootView;

    if (pEngine->pRootView != NULL) {
        pEngine->pRootView->_pPrevView = pView;
    }

    pEngine->pRootView = pView;


    return pView;
}

void drte_view_delete(drte_view* pView)
{
    if (pView == NULL) {
        return;
    }

    // Remove the view from the list first.
    if (pView->pEngine->pRootView == pView) {
        pView->pEngine->pRootView = pView->_pNextView;
    }
    
    if (pView->_pNextView != NULL) {
        pView->_pNextView->_pPrevView = pView->_pPrevView;
    }
    if (pView->_pPrevView != NULL) {
        pView->_pPrevView->_pNextView = pView->_pNextView;
    }

    drte_line_cache_uninit(&pView->_wrappedLines);
    free(pView);
}


void drte_view_set_size(drte_view* pView, float sizeX, float sizeY)
{
    if (pView == NULL) {
        return;
    }

    bool sizeXChanged = pView->sizeX != sizeX;
    bool sizeYChanged = pView->sizeY != sizeY;
    if (!sizeXChanged && !sizeYChanged) {
        return;
    }

    pView->sizeX = sizeX;
    pView->sizeY = sizeY;

    if (sizeXChanged && drte_view_is_word_wrap_enabled(pView)) {
        drte_view__refresh_word_wrapping(pView);
    } else {
        drte_view__repaint(pView);
    }
}

void drte_view_get_size(drte_view* pView, float* pSizeXOut, float* pSizeYOut)
{
    if (pSizeXOut != NULL) *pSizeXOut = 0;
    if (pSizeYOut != NULL) *pSizeYOut = 0;
    if (pView == NULL) return;

    if (pSizeXOut != NULL) *pSizeXOut = pView->sizeX;
    if (pSizeYOut != NULL) *pSizeYOut = pView->sizeY;
}


void drte_view_set_inner_offset(drte_view* pView, float innerOffsetX, float innerOffsetY)
{
    if (pView == NULL) {
        return;
    }

    pView->innerOffsetX = innerOffsetX;
    pView->innerOffsetY = innerOffsetY;

    drte_view__repaint(pView);
}

void drte_view_get_inner_offset(drte_view* pView, float* pInnerOffsetXOut, float* pInnerOffsetYOut)
{
    if (pInnerOffsetXOut != NULL) *pInnerOffsetXOut = 0;
    if (pInnerOffsetYOut != NULL) *pInnerOffsetYOut = 0;
    if (pView == NULL) return;

    if (pInnerOffsetXOut != NULL) *pInnerOffsetXOut = pView->innerOffsetX;
    if (pInnerOffsetYOut != NULL) *pInnerOffsetYOut = pView->innerOffsetY;
}


void drte_view_set_tab_size(drte_view* pView, unsigned int sizeInSpaces)
{
    if (pView == NULL || pView->tabSizeInSpaces == sizeInSpaces) {
        return;
    }

    pView->tabSizeInSpaces = sizeInSpaces;
    drte_view__refresh_word_wrapping(pView);
}


void drte_view_begin_dirty(drte_view* pView)
{
    if (pView == NULL) {
        return;
    }

    pView->_dirtyCounter += 1;
}

void drte_view_end_dirty(drte_view* pView)
{
    if (pView == NULL) {
        return;
    }

    assert(pView->_dirtyCounter > 0);

    pView->_dirtyCounter -= 1;

    if (pView->_dirtyCounter == 0) {
        if (pView->pEngine->onDirty && drte_rect_has_volume(pView->_accumulatedDirtyRect)) {
            pView->pEngine->onDirty(pView->pEngine, pView, pView->_accumulatedDirtyRect);
        }

        pView->_accumulatedDirtyRect = drte_make_inside_out_rect();
    }
}

void drte_view_dirty(drte_view* pView, drte_rect rect)
{
    if (pView == NULL) {
        return;
    }

    drte_view_begin_dirty(pView);
    pView->_accumulatedDirtyRect = drte_rect_union(pView->_accumulatedDirtyRect, rect);
    drte_view_end_dirty(pView);
}

void drte_view_paint(drte_view* pView, drte_rect rect, void* pPaintData)
{
    if (pView == NULL || pView->pEngine->onPaintText == NULL || pView->pEngine->onPaintRect == NULL) {
        return;
    }

    if (rect.left < 0) {
        rect.left = 0;
    }
    if (rect.top < 0) {
        rect.top = 0;
    }
    if (rect.right > pView->sizeX) {
        rect.right = pView->sizeX;
    }
    if (rect.bottom > pView->sizeY) {
        rect.bottom = pView->sizeY;
    }

    if (rect.right <= rect.left || rect.bottom <= rect.top) {
        return;
    }

    float lineHeight = drte_engine_get_line_height(pView->pEngine);


    size_t iLineTop;
    size_t iLineBottom;
    drte_view_get_visible_lines(pView, &iLineTop, &iLineBottom);

    float linePosX = pView->innerOffsetX;
    float linePosY = 0;

    drte_segment segment;
    if (drte_engine__first_segment_on_line(pView, pView->pWrappedLines, iLineTop, (size_t)-1, &segment)) {
        size_t iLine = iLineTop;
        while (iLine <= iLineBottom) {
            float lineWidth = 0;

            do
            {
                if (linePosX + segment.posX > pView->sizeX) {
                    // All remaining segments on this line (including this one) is clipped. Go to the next line.
                    segment.iCharBeg = segment.iLineCharEnd;
                    segment.iCharEnd = segment.iLineCharEnd;
                    segment.isAtEndOfLine = true;
                    break;
                }

                lineWidth += segment.width;

                // Don't draw segments to the left of the container.
                if (linePosX + segment.posX + segment.width < 0) {
                    if (segment.iCharBeg == segment.iLineCharEnd) {
                        break;
                    }
                    continue;
                }

                uint32_t c = drte_engine_get_utf32(pView->pEngine, segment.iCharBeg);
                if (c == '\t' || segment.iCharBeg == segment.iLineCharEnd) {
                    // It's whitespace.
                    if (segment.iCharBeg == segment.iLineCharEnd) {
                        // TODO: Only do this if the character is selected.
                        uint32_t cend = drte_engine_get_utf32(pView->pEngine, segment.iCharBeg);
                        if (cend == '\r' || cend == '\n') {
                            segment.width = pView->pEngine->styles[pView->pEngine->defaultStyleSlot].fontMetrics.spaceWidth;
                            lineWidth += segment.width;
                        }
                    }

                    drte_style_token bgStyleToken = drte_engine__get_style_token(pView->pEngine, segment.bgStyleSlot);
                    if (pView->pEngine->onPaintRect && bgStyleToken != 0) {
                        pView->pEngine->onPaintRect(pView->pEngine, pView, bgStyleToken, drte_make_rect(linePosX + segment.posX, linePosY, linePosX + segment.posX + segment.width, linePosY + lineHeight), pPaintData);
                    }
                } else {
                    // It's normal text.
                    // TODO: Gather the text and properly support UTF-8.
                    const char* text = pView->pEngine->text + segment.iCharBeg;
                    size_t textLength = segment.iCharEnd - segment.iCharBeg;

                    // TODO: Draw text on the base line to properly handle font's of differing sizes.

                    drte_style_token fgStyleToken = drte_engine__get_style_token(pView->pEngine, segment.fgStyleSlot);
                    drte_style_token bgStyleToken = drte_engine__get_style_token(pView->pEngine, segment.bgStyleSlot);
                    if (pView->pEngine->onPaintText && fgStyleToken != 0 && bgStyleToken != 0) {
                        pView->pEngine->onPaintText(pView->pEngine, pView, fgStyleToken, bgStyleToken, text, textLength, linePosX + segment.posX, linePosY, pPaintData);
                    }
                }

                if (segment.iCharBeg == segment.iLineCharEnd) {
                    break;
                }
            } while (drte_engine__next_segment(pView, &segment));


            // The part after the end of the line needs to be drawn.
            float lineRight = linePosX + lineWidth;
            if (lineRight < pView->sizeX) {
                drte_style_token bgStyleToken = pView->pEngine->styles[pView->pEngine->defaultStyleSlot].styleToken;
                if (pView->cursorCount > 0 && segment.iLine == drte_view_get_cursor_line(pView, pView->cursorCount-1)) {
                    bgStyleToken = pView->pEngine->styles[pView->pEngine->activeLineStyleSlot].styleToken;
                }

                if (pView->pEngine->onPaintRect && bgStyleToken != 0) {
                    pView->pEngine->onPaintRect(pView->pEngine, pView, bgStyleToken, drte_make_rect(lineRight, linePosY, pView->sizeX, linePosY + lineHeight), pPaintData);
                }
            }

            linePosY += lineHeight;


            // Go to the first segment of the next line.
            if (!drte_engine__next_segment(pView, &segment)) {
                break;
            }

            iLine += 1;
        }
    } else {
        // Couldn't create a segment iterator. Likely means there is no text. Just draw a single blank line.
        drte_style_token bgStyleToken = pView->pEngine->styles[pView->pEngine->activeLineStyleSlot].styleToken;
        if (pView->pEngine->onPaintRect && bgStyleToken != 0) {
            pView->pEngine->onPaintRect(pView->pEngine, pView, bgStyleToken, drte_make_rect(linePosX, linePosY, pView->sizeX, linePosY + lineHeight), pPaintData);
        }
    }


    // Cursors.
    if (drte_view_is_showing_cursors(pView) && pView->pEngine->isCursorBlinkOn && pView->pEngine->styles[pView->pEngine->cursorStyleSlot].styleToken != 0) {
        for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
            pView->pEngine->onPaintRect(pView->pEngine, pView, pView->pEngine->styles[pView->pEngine->cursorStyleSlot].styleToken, drte_view_get_cursor_rect(pView, iCursor), pPaintData);
        }
    }


    // The rectangle region below the last line.
    if (linePosY < pView->sizeY && pView->pEngine->styles[pView->pEngine->defaultStyleSlot].styleToken != 0) {
        // TODO: Only draw the intersection of the bottom rectangle with the invalid rectangle.
        drte_rect tailRect;
        tailRect.left = 0;
        tailRect.top = (iLineBottom + 1) * drte_engine_get_line_height(pView->pEngine) + pView->innerOffsetY;
        tailRect.right = pView->sizeX;
        tailRect.bottom = pView->sizeY;
        pView->pEngine->onPaintRect(pView->pEngine, pView, pView->pEngine->styles[pView->pEngine->defaultStyleSlot].styleToken, tailRect, pPaintData);
    }
}

void drte_view_paint_line_numbers(drte_view* pView, float lineNumbersWidth, float lineNumbersHeight, drte_engine_on_paint_text_proc onPaintText, drte_engine_on_paint_rect_proc onPaintRect, void* pPaintData)
{
    if (pView == NULL || onPaintText == NULL || onPaintRect == NULL) {
        return;
    }

    float lineHeight = drte_engine_get_line_height(pView->pEngine);

    size_t iLineTop;
    size_t iLineBottom;
    drte_view_get_visible_lines(pView, &iLineTop, &iLineBottom);

    drte_style_token fgStyleToken = pView->pEngine->styles[pView->pEngine->lineNumbersStyleSlot].styleToken;
    drte_style_token bgStyleToken = pView->pEngine->styles[pView->pEngine->lineNumbersStyleSlot].styleToken;

    size_t lineNumber = iLineTop;

    float lineTop = pView->innerOffsetY + (iLineTop * lineHeight);
    for (size_t iLine = iLineTop; iLine <= iLineBottom; ++iLine) {
        float lineBottom = lineTop + lineHeight;
        bool drawLineNumber = false;

        size_t iLineCharBeg;
        size_t iLineCharEnd;
        drte_view_get_line_character_range(pView, pView->pWrappedLines, iLine, &iLineCharBeg, &iLineCharEnd);
        if (iLine == 0 || pView->pEngine->text[iLineCharBeg-1] == '\n') {
            lineNumber += 1;
            drawLineNumber = true;
        }

        if (drawLineNumber) {
            char iLineStr[64];
            snprintf(iLineStr, sizeof(iLineStr), "%d", (int)lineNumber);   // TODO: drte_string_to_int(). This snprintf() has shown up in profiling so best fix this.

            float textWidth = 0;
            float textHeight = 0;
            if (pView->pEngine->onMeasureString && fgStyleToken) {
                pView->pEngine->onMeasureString(pView->pEngine, fgStyleToken, iLineStr, strlen(iLineStr), &textWidth, &textHeight);
            }

            float textLeft = lineNumbersWidth - textWidth;
            float textTop  = lineTop + (lineHeight - textHeight) / 2;

            if (fgStyleToken != 0 && bgStyleToken != 0) {
                onPaintText(pView->pEngine, pView, fgStyleToken, bgStyleToken, iLineStr, strlen(iLineStr), textLeft, textTop, pPaintData);
                onPaintRect(pView->pEngine, pView, bgStyleToken, drte_make_rect(0, lineTop, textLeft, lineBottom), pPaintData);

                // There could be a region above and below the text. This will happen if the line height of the line numbers is
                // smaller than the main line height.
                if (textHeight < lineHeight) {
                    onPaintRect(pView->pEngine, pView, bgStyleToken, drte_make_rect(textLeft, lineTop, lineNumbersWidth, textTop), pPaintData);
                    onPaintRect(pView->pEngine, pView, bgStyleToken, drte_make_rect(textLeft, textTop + textHeight, lineNumbersWidth, lineBottom), pPaintData);
                }
            }
        } else {
            if (fgStyleToken != 0 && bgStyleToken != 0) {
                onPaintRect(pView->pEngine, pView, bgStyleToken, drte_make_rect(0, lineTop, lineNumbersWidth, lineBottom), pPaintData);
            }
        }

        lineTop = lineBottom;
    }

    // The region below the lines.
    if (lineTop < pView->sizeY && bgStyleToken != 0) {
        onPaintRect(pView->pEngine, pView, bgStyleToken, drte_make_rect(0, lineTop, lineNumbersWidth, lineNumbersHeight), pPaintData);
    }
}


drte_rect drte_view_get_local_rect(drte_view* pView)
{
    if (pView == NULL) {
        return drte_make_rect(0, 0, 0, 0);
    }

    return drte_make_rect(0, 0, pView->sizeX, pView->sizeY);
}


void drte_view_enable_word_wrap(drte_view* pView)
{
    if (pView == NULL && !drte_view_is_word_wrap_enabled(pView)) {
        return;
    }

    // The wrapped line cache will not have been initialized at this point so we need to do that first.
    if (!drte_line_cache_init(&pView->_wrappedLines)) {
        return;
    }
    pView->pWrappedLines = &pView->_wrappedLines;

    pView->flags |= DRTE_WORD_WRAP_ENABLED;
    drte_view__refresh_word_wrapping(pView);
}

void drte_view_disable_word_wrap(drte_view* pView)
{
    if (pView == NULL && drte_view_is_word_wrap_enabled(pView)) {
        return;
    }

    // We do not need the wrapped line cache.
    pView->pWrappedLines = &pView->pEngine->_unwrappedLines;
    drte_line_cache_uninit(&pView->_wrappedLines);

    pView->flags &= ~DRTE_WORD_WRAP_ENABLED;
    drte_view__refresh_word_wrapping(pView);
}

bool drte_view_is_word_wrap_enabled(drte_view* pView)
{
    return (pView->flags & DRTE_WORD_WRAP_ENABLED) != 0;
}


size_t drte_view_get_character_line(drte_view* pView, drte_line_cache* pLineCache, size_t characterIndex)
{
    if (pView == NULL) {
        return 0;
    }

    if (pLineCache == NULL) pLineCache = pView->pWrappedLines;

    return drte_line_cache_find_line_by_character(pLineCache, characterIndex);
}

void drte_view_get_character_position(drte_view* pView, drte_line_cache* pLineCache, size_t characterIndex, float* pPosXOut, float* pPosYOut)
{
    if (pPosXOut) *pPosXOut = 0;
    if (pPosYOut) *pPosYOut = 0;

    if (pView == NULL) {
        return;
    }

    size_t lineIndex = drte_view_get_character_line(pView, pLineCache, characterIndex);

    float posX = 0;
    float posY = lineIndex * drte_engine_get_line_height(pView->pEngine);

    drte_segment segment;
    if (drte_engine__first_segment_on_line(pView, pLineCache, lineIndex, (size_t)-1, &segment)) {
        do
        {
            if (characterIndex >= segment.iCharBeg && characterIndex < segment.iCharEnd) {
                // It's somewhere in this segment.
                if (drte_engine_get_utf32(pView->pEngine, segment.iCharBeg) == '\t') {
                    // If the first character in the segment is a tab character, then every character in this segment
                    // will be a tab. The location of the character is rounded to the nearest tab column.
                    posX = segment.posX;

                    size_t tabCount = characterIndex - segment.iCharBeg;
                    if (tabCount > 0) {
                        float tabWidth = drte_view__get_tab_width_in_pixels(pView);
                        float nextTabPos = (float)((int)(segment.posX / tabWidth) + 1) * tabWidth;
                        posX = nextTabPos + ((tabCount-1) * tabWidth);
                    }
                } else {
                    // We must refer to the backend in order to find the exact position of the character.
                    // TODO: Grab a copy of the string rather than a direct offset.
                    drte_style_token fgStyleToken = drte_engine__get_style_token(pView->pEngine, segment.fgStyleSlot);
                    if (pView->pEngine->onGetCursorPositionFromChar && fgStyleToken != 0) {
                        pView->pEngine->onGetCursorPositionFromChar(pView->pEngine, fgStyleToken, pView->pEngine->text + segment.iCharBeg, characterIndex - segment.iCharBeg, &posX);
                        posX += segment.posX;
                    }
                }

                break;
            }
        } while (drte_engine__next_segment_on_line(pView, &segment));
    }

    if (pPosXOut) *pPosXOut = posX;
    if (pPosYOut) *pPosYOut = posY;
}

bool drte_view_get_character_under_point_relative_to_text(drte_view* pView, drte_line_cache* pLineCache, float inputPosXRelativeToText, float inputPosYRelativeToText, size_t* piCharOut, size_t* piLineOut)
{
    if (piCharOut) *piCharOut = 0;
    if (piLineOut) *piLineOut = 0;

    if (pView == NULL) {
        return false;
    }

    size_t iLine = drte_view_get_line_at_pos_y(pView, pLineCache, inputPosYRelativeToText);
    size_t iChar = 0;

    if (piLineOut) *piLineOut = iLine;

    // Once we have the line, finding the specific character under the point is done by iterating over each segment and finding the one
    // containing the point on the x axis. Once the segment has been found, we use the backend to get the exact character.
    if (inputPosXRelativeToText < 0) {
        iChar = drte_view_get_line_first_character(pView, pLineCache, (size_t)iLine);   // It's to the left of the line, so just pin it to the first character in the line.
        
        if (piCharOut) *piCharOut = iChar;
        return false;   // <-- Return false because it's not actually over a character (it's to the left).
    }

    drte_segment segment;
    if (drte_engine__first_segment_on_line(pView, pLineCache, (size_t)iLine, (size_t)-1, &segment)) {
        do
        {
            if (inputPosXRelativeToText >= segment.posX && inputPosXRelativeToText < segment.posX + segment.width) {
                // It's somewhere on this run. If it's a tab segment it needs to be handled slightly differently because of the way tabs
                // are aligned to tab columns.
                if (drte_engine_get_utf32(pView->pEngine, segment.iCharBeg) == '\t') {
                    const float tabWidth = drte_view__get_tab_width_in_pixels(pView);

                    iChar = segment.iCharBeg;

                    float tabLeft = segment.posX;
                    for (/* Do Nothing*/; iChar < segment.iCharEnd; ++iChar)
                    {
                        float tabRight = tabWidth * ((segment.posX + (tabWidth*((iChar-segment.iCharBeg) + 1))) / tabWidth);
                        if (inputPosXRelativeToText >= tabLeft && inputPosXRelativeToText <= tabRight)
                        {
                            // The input position is somewhere on top of this character. If it's positioned on the left side of the character, set the output
                            // value to the character at iChar. Otherwise it should be set to the character at iChar + 1.
                            float charBoundsRightHalf = tabLeft + ceilf(((tabRight - tabLeft) / 2.0f));
                            if (inputPosXRelativeToText > charBoundsRightHalf) {
                                iChar += 1;
                            }

                            break;
                        }

                        tabLeft = tabRight;
                    }
                } else {
                    float unused;
                    size_t iCharTemp;

                    drte_style_token fgStyleToken = drte_engine__get_style_token(pView->pEngine, segment.fgStyleSlot);
                    if (pView->pEngine->onGetCursorPositionFromPoint) {
                        pView->pEngine->onGetCursorPositionFromPoint(pView->pEngine, fgStyleToken, pView->pEngine->text + segment.iCharBeg, segment.iCharEnd - segment.iCharBeg, segment.width, inputPosXRelativeToText - segment.posX, OUT &unused, OUT &iCharTemp);
                        iChar = segment.iCharBeg + iCharTemp;
                    }
                }

                if (piCharOut) *piCharOut = iChar;

                // It's possible that the Y position is not actually over a line. Whether or not we return true or false depends on this.
                if (inputPosYRelativeToText < 0) {
                    return false;
                } else {
                    if (inputPosYRelativeToText >= (drte_view_get_line_count(pView) * drte_engine_get_line_height(pView->pEngine))) {
                        return false;
                    }
                }

                return true;
            }
        } while (drte_engine__next_segment_on_line(pView, &segment));

        // If we get here it means the position is to the right of the line. Just pin it to the end of the line.
        iChar = segment.iCharBeg;   // <-- segment.iCharBeg should be sitting on a new line or null terminator.

        if (piCharOut) *piCharOut = iChar;
        return false;   // <-- Return false because it's not actually over a character (it's to the right)
    }

    return false;
}

bool drte_view_get_character_under_point(drte_view* pView, drte_line_cache* pLineCache, float inputPosXRelativeToContainer, float inputPosYRelativeToContainer, size_t* piCharOut, size_t* piLineOut)
{
    if (pView == NULL) {
        return 0;
    }

    float inputPosXRelativeToText = inputPosXRelativeToContainer - pView->innerOffsetX;
    float inputPosYRelativeToText = inputPosYRelativeToContainer - pView->innerOffsetY;
    return drte_view_get_character_under_point_relative_to_text(pView, pLineCache, inputPosXRelativeToText, inputPosYRelativeToText, piCharOut, piLineOut);
}

void drte_view_get_visible_lines(drte_view* pView, size_t* pFirstLineOut, size_t* pLastLineOut)
{
    if (pFirstLineOut) *pFirstLineOut = 0;
    if (pLastLineOut) *pLastLineOut = 0;
    if (pView == NULL) return;

    size_t iFirstLine = (size_t)(-pView->innerOffsetY / drte_engine_get_line_height(pView->pEngine));

    if (pFirstLineOut) {
        *pFirstLineOut = iFirstLine;
    }

    if (pLastLineOut) {
        size_t lineCount = drte_view_get_line_count(pView);
        size_t iLastLine = iFirstLine + ((size_t)(pView->sizeY / drte_engine_get_line_height(pView->pEngine)));
        if (lineCount == 0) {
            iLastLine = 0;
        } else {
            if (iLastLine >= lineCount) {
                iLastLine = lineCount - 1;
            }
        }

        *pLastLineOut = iLastLine;
    }
}

size_t drte_view_get_line_count(drte_view* pView)
{
    if (pView == NULL) return 0;
    return drte_line_cache_get_line_count(pView->pWrappedLines);
}

size_t drte_view_get_line_count_per_page(drte_view* pView)
{
    if (pView == NULL) return 1;

    size_t lineCount = (size_t)(pView->sizeY / drte_engine_get_line_height(pView->pEngine));
    if (lineCount == 0) {
        lineCount = 1;  // Always at least one line on a page.
    }

    return lineCount;
}

size_t drte_view_get_page_count(drte_view* pView)
{
    if (pView == NULL) return 1;

    size_t lineCount    = drte_view_get_line_count(pView);
    size_t linesPerPage = drte_view_get_line_count_per_page(pView);

    size_t pageCount = lineCount / linesPerPage;
    if (pageCount == 0) {
        pageCount = 1;  // Always at least one page.
    }

    if (lineCount % linesPerPage != 0) {
        pageCount += 1;
    }

    return pageCount;
}

size_t drte_view_get_visible_line_count(drte_view* pView)
{
    if (pView == NULL) return 0;
    return (size_t)(pView->sizeY / drte_engine_get_line_height(pView->pEngine)) + 1;
}

float drte_view_get_visible_line_width(drte_view* pView)
{
    if (pView == NULL) return 0;

    size_t iLineTop;
    size_t iLineBottom;
    drte_view_get_visible_lines(pView, &iLineTop, &iLineBottom);

    float maxLineWidth = 0;

    drte_segment segment;
    if (drte_engine__first_segment_on_line(pView, pView->pWrappedLines, iLineTop, (size_t)-1, &segment)) {
        size_t iLine = iLineTop;
        while (iLine <= iLineBottom) {
            float lineWidth = 0;

            do
            {
                lineWidth += segment.width;

                if (segment.iCharBeg == segment.iLineCharEnd) {
                    break;
                }
            } while (drte_engine__next_segment(pView, &segment));


            if (maxLineWidth < lineWidth) {
                maxLineWidth = lineWidth;
            }

            // Go to the first segment of the next line.
            if (!drte_engine__next_segment(pView, &segment)) {
                break;
            }

            iLine += 1;
        }
    }

    return maxLineWidth;
}

void drte_view_measure_line(drte_view* pView, size_t iLine, float* pSizeXOut, float* pSizeYOut)
{
    if (pSizeXOut) *pSizeXOut = 0;
    if (pSizeYOut) *pSizeYOut = 0;

    if (pView == NULL) {
        return;
    }

    if (pSizeYOut) *pSizeYOut = drte_engine_get_line_height(pView->pEngine);
    if (pSizeXOut) {
        float lineWidth = 0;

        drte_segment segment;
        if (drte_engine__first_segment_on_line(pView, pView->pWrappedLines, iLine, (size_t)-1, &segment)) {
            do
            {
                lineWidth += segment.width;
            } while (drte_engine__next_segment_on_line(pView, &segment));
        }

        *pSizeXOut = lineWidth;
    }
}

float drte_view_get_line_pos_y(drte_view* pView, size_t iLine)
{
    return iLine * drte_engine_get_line_height(pView->pEngine);
}

size_t drte_view_get_line_at_pos_y(drte_view* pView, drte_line_cache* pLineCache, float posY)
{
    if (pView == NULL) return 0;
    if (pLineCache == NULL) pLineCache = pView->pWrappedLines;

    size_t lineCount = drte_line_cache_get_line_count(pLineCache);
    if (lineCount == 0) {
        return 0;
    }

    intptr_t iLine = (intptr_t)(posY / drte_engine_get_line_height(pView->pEngine));
    if (iLine < 0) {
        iLine = 0;
    }
    if ((size_t)iLine >= lineCount) {
        iLine = lineCount-1;
    }

    return iLine;
}

size_t drte_view_get_line_first_character(drte_view* pView, drte_line_cache* pLineCache, size_t iLine)
{
    if (pView == NULL || iLine == 0) {
        return 0;
    }

    if (pLineCache == NULL) pLineCache = pView->pWrappedLines;
    return drte_line_cache_get_line_first_character(pLineCache, iLine);
}

size_t drte_view_get_line_last_character(drte_view* pView, drte_line_cache* pLineCache, size_t iLine)
{
    if (pView == NULL || pView->pEngine->text == NULL) {
        return 0;
    }

    if (pLineCache == NULL) pLineCache = pView->pWrappedLines;


    // The line caches only store the index of the first character of the line. We can quality get the last character of the line
    // by simply interrogating the first character of the _next_ line. However, there is no next line for the last line so we handle
    // that one in a special way.
    if (iLine+1 < drte_line_cache_get_line_count(pLineCache)) {
        size_t iLineEnd = drte_line_cache_get_line_first_character(pLineCache, iLine+1);
        assert(iLineEnd > 0);

        if (pView->pEngine->text[iLineEnd-1] == '\n') {
            iLineEnd -= 1;
            if (iLineEnd > 0) {
                if (pView->pEngine->text[iLineEnd-1] == '\r') {
                    iLineEnd -= 1;
                }
            }
        }

        return iLineEnd;
    }

    // It's the last line. Just return the position of the null terminator.
    return drte_line_cache_get_line_first_character(pLineCache, iLine) + strlen(pView->pEngine->text + drte_line_cache_get_line_first_character(pLineCache, iLine));
}

size_t drte_view_get_line_first_non_whitespace_character(drte_view* pView, drte_line_cache* pLineCache, size_t iLine)
{
    size_t iChar = drte_view_get_line_first_character(pView, pLineCache, iLine);
    for (;;) {
        uint32_t c = pView->pEngine->text[iChar];
        if (c == '\0' || c == '\r' || c == '\n' || !drte_is_whitespace(c)) {
            break;
        }

        iChar += 1;
    }

    return iChar;
}

void drte_view_get_line_character_range(drte_view* pView, drte_line_cache* pLineCache, size_t iLine, size_t* pCharStartOut, size_t* pCharEndOut)
{
    if (pView == NULL) {
        return;
    }

    if (pCharStartOut) *pCharStartOut = drte_view_get_line_first_character(pView, pLineCache, iLine);
    if (pCharEndOut) *pCharEndOut = drte_view_get_line_last_character(pView, pLineCache, iLine);
}




void drte_view_set_cursor_width(drte_view* pView, float cursorWidth)
{
    if (pView == NULL || pView->cursorWidth == cursorWidth) {
        return;
    }

    drte_view_begin_dirty(pView);
    {
        for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
            drte_view_dirty(pView, drte_view_get_cursor_rect(pView, iCursor));
        }

        pView->cursorWidth = cursorWidth;
        if (pView->cursorWidth > 0 && pView->cursorWidth < 1) {
            pView->cursorWidth = 1;
        }

        for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
            drte_view_dirty(pView, drte_view_get_cursor_rect(pView, iCursor));
        }
    }
    drte_view_end_dirty(pView);
}


void drte_view_show_cursors(drte_view* pView)
{
    if (pView == NULL || drte_view_is_showing_cursors(pView)) {
        return;
    }

    pView->flags |= DRTE_SHOWING_CURSORS;

    drte_view_begin_dirty(pView);
    for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
        drte_view_dirty(pView, drte_view_get_cursor_rect(pView, iCursor));
    }
    drte_view_end_dirty(pView);
}

void drte_view_hide_cursors(drte_view* pView)
{
    if (pView == NULL || !drte_view_is_showing_cursors(pView)) {
        return;
    }

    pView->flags &= ~DRTE_SHOWING_CURSORS;

    drte_view_begin_dirty(pView);
    for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
        drte_view_dirty(pView, drte_view_get_cursor_rect(pView, iCursor));
    }
    drte_view_end_dirty(pView);
}

bool drte_view_is_showing_cursors(drte_view* pView)
{
    if (pView == NULL) return false;
    return (pView->flags & DRTE_SHOWING_CURSORS) != 0;
}


void drte_view_get_cursor_position(drte_view* pView, size_t cursorIndex, float* pPosXOut, float* pPosYOut)
{
    if (pPosXOut) *pPosXOut = 0;
    if (pPosYOut) *pPosYOut = 0;

    if (pView == NULL || pView->cursorCount <= cursorIndex) {
        return;
    }

    // If the character is on a different line to the cursor, it means the cursor is pinned to the end of the previous line and the character
    // is the first character on the _next_ line. This will happen when word wrap is enabled. In this case things need to be treated a bit
    // differently to calculate the x position.
    float posX = 0;
    float posY = 0;
    if (pView->pCursors[cursorIndex].iLine != drte_view_get_character_line(pView, pView->pWrappedLines, pView->pCursors[cursorIndex].iCharAbs)) {
        drte_view_measure_line(pView, pView->pCursors[cursorIndex].iLine, &posX, NULL);
        posY = drte_view_get_line_pos_y(pView, pView->pCursors[cursorIndex].iLine);
    } else {
        drte_view_get_character_position(pView, pView->pWrappedLines, pView->pCursors[cursorIndex].iCharAbs, &posX, &posY);
    }

    if (pPosXOut) *pPosXOut = posX + pView->innerOffsetX;
    if (pPosYOut) *pPosYOut = posY + pView->innerOffsetY;
}

drte_rect drte_view_get_cursor_rect(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL) {
        return drte_make_rect(0, 0, 0, 0);
    }

    float cursorPosX;
    float cursorPosY;
    drte_view_get_cursor_position(pView, cursorIndex, &cursorPosX, &cursorPosY);

    return drte_make_rect(cursorPosX, cursorPosY, cursorPosX + pView->cursorWidth, cursorPosY + drte_engine_get_line_height(pView->pEngine));
}

size_t drte_view_insert_cursor(drte_view* pView, size_t iChar)
{
    if (pView == NULL) {
        return (size_t)-1;
    }

    drte_cursor* pNewCursors = (drte_cursor*)realloc(pView->pCursors, (pView->cursorCount+1) * sizeof(*pNewCursors));
    if (pNewCursors == NULL) {
        return (size_t)-1;
    }

    pView->pCursors = pNewCursors;
    pView->pCursors[pView->cursorCount].iCharAbs = 0;
    pView->pCursors[pView->cursorCount].iLine = 0;
    pView->pCursors[pView->cursorCount].absoluteSickyPosX = 0;
    pView->cursorCount += 1;

    drte_view_begin_dirty(pView);
        drte_view_move_cursor_to_character(pView, pView->cursorCount-1, iChar);
        drte_view__repaint(pView);
    drte_view_end_dirty(pView);

    return pView->cursorCount - 1;
}

size_t drte_view_insert_cursor_at_character_and_line(drte_view* pView, size_t iChar, size_t iLine)
{
    size_t iCursor = drte_view_insert_cursor(pView, iChar);
    if (iCursor == (size_t)-1) {
        return (size_t)-1;
    }

    pView->pCursors[iCursor].iLine = iLine;
    return iCursor;
}

void drte_view_remove_cursor(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->cursorCount <= cursorIndex) {
        return;
    }

    for (size_t i = cursorIndex; i < pView->cursorCount-1; ++i) {
        pView->pCursors[i] = pView->pCursors[i+1];
    }

    pView->cursorCount -= 1;
    drte_view__repaint(pView);
}

void drte_view_remove_overlapping_cursors(drte_view* pView)
{
    if (pView == NULL || pView->cursorCount == 0) {
        return;
    }

    drte_view_begin_dirty(pView);
    for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
        for (size_t jCursor = iCursor+1; jCursor < pView->cursorCount; ++jCursor) {
            if (pView->pCursors[iCursor].iCharAbs == pView->pCursors[jCursor].iCharAbs) {
                drte_view_remove_cursor(pView, jCursor);
                break;
            }
        }
    }
    drte_view_end_dirty(pView);
}

size_t drte_view_get_last_cursor(drte_view* pView)
{
    if (pView == NULL || pView->cursorCount == 0) {
        return 0;
    }

    return pView->cursorCount - 1;
}

size_t drte_view_get_cursor_line(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->cursorCount <= cursorIndex) {
        return 0;
    }

    return pView->pCursors[cursorIndex].iLine;
}

size_t drte_view_get_cursor_column(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->cursorCount <= cursorIndex) {
        return 0;
    }

    float posX;
    float posY;
    drte_view_get_cursor_position(pView, cursorIndex, &posX, &posY);

    return (unsigned int)((int)posX / pView->pEngine->styles[pView->pEngine->defaultStyleSlot].fontMetrics.spaceWidth);
}

size_t drte_view_get_cursor_character(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->cursorCount <= cursorIndex) {
        return 0;
    }

    return pView->pCursors[cursorIndex].iCharAbs;
}

bool drte_view_move_cursor_to_point(drte_view* pView, size_t cursorIndex, float posX, float posY)
{
    if (pView == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pView->pCursors[cursorIndex].iCharAbs;

    pView->pCursors[cursorIndex].iCharAbs = 0;
    pView->pCursors[cursorIndex].iLine = 0;
    pView->pCursors[cursorIndex].absoluteSickyPosX = 0;

    float inputPosXRelativeToText = posX - pView->innerOffsetX;
    float inputPosYRelativeToText = posY - pView->innerOffsetY;
    if (!drte_view_move_cursor_to_point_relative_to_text(pView, cursorIndex, inputPosXRelativeToText, inputPosYRelativeToText)) {
        return false;
    }

    drte_view__update_cursor_sticky_position(pView, &pView->pCursors[cursorIndex]);

    if (iPrevChar != pView->pCursors[cursorIndex].iCharAbs) {
        drte_view_begin_dirty(pView);
            drte_engine__on_cursor_move(pView->pEngine, pView, cursorIndex);
            drte_view_dirty(pView, drte_view_get_local_rect(pView));   // <-- TODO: This can be optimized. Only redraw the previous line and the new cursor rectangle.
        drte_view_end_dirty(pView);
    }

    return true;
}

bool drte_view_move_cursor_to_point_relative_to_text(drte_view* pView, size_t cursorIndex, float posXRelativeToText, float posYRelativeToText)
{
    if (pView == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iLine = drte_view_get_line_at_pos_y(pView, pView->pWrappedLines, posYRelativeToText);
    pView->pCursors[cursorIndex].iLine = iLine;

    // Once we have the line, finding the specific character under the point is done by iterating over each segment and finding the one
    // containing the point on the x axis. Once the segment has been found, we use the backend to get the exact character.
    if (posXRelativeToText < 0) {
        pView->pCursors[cursorIndex].iCharAbs = drte_view_get_line_first_character(pView, pView->pWrappedLines, (size_t)iLine);
        return true;    // It's to the left of the line, so just pin it to the first character in the line.
    }

    drte_segment segment;
    if (drte_engine__first_segment_on_line(pView, pView->pWrappedLines, (size_t)iLine, (size_t)-1, &segment)) {
        do
        {
            if (posXRelativeToText >= segment.posX && posXRelativeToText < segment.posX + segment.width) {
                // It's somewhere on this run. If it's a tab segment it needs to be handled slightly differently because of the way tabs
                // are aligned to tab columns.
                if (drte_engine_get_utf32(pView->pEngine, segment.iCharBeg) == '\t') {
                    const float tabWidth = drte_view__get_tab_width_in_pixels(pView);

                    pView->pCursors[cursorIndex].iCharAbs = segment.iCharBeg;

                    float tabLeft = segment.posX;
                    for (/* Do Nothing*/; pView->pCursors[cursorIndex].iCharAbs < segment.iCharEnd; ++pView->pCursors[cursorIndex].iCharAbs)
                    {
                        float tabRight = tabWidth * ((segment.posX + (tabWidth*((pView->pCursors[cursorIndex].iCharAbs-segment.iCharBeg) + 1))) / tabWidth);
                        if (posXRelativeToText >= tabLeft && posXRelativeToText <= tabRight)
                        {
                            // The input position is somewhere on top of this character. If it's positioned on the left side of the character, set the output
                            // value to the character at iChar. Otherwise it should be set to the character at iChar + 1.
                            float charBoundsRightHalf = tabLeft + ceilf(((tabRight - tabLeft) / 2.0f));
                            if (posXRelativeToText > charBoundsRightHalf) {
                                pView->pCursors[cursorIndex].iCharAbs += 1;
                            }

                            break;
                        }

                        tabLeft = tabRight;
                    }
                } else {
                    float unused;
                    size_t iChar;

                    drte_style_token fgStyleToken = drte_engine__get_style_token(pView->pEngine, segment.fgStyleSlot);
                    if (pView->pEngine->onGetCursorPositionFromPoint) {
                        pView->pEngine->onGetCursorPositionFromPoint(pView->pEngine, fgStyleToken, pView->pEngine->text + segment.iCharBeg, segment.iCharEnd - segment.iCharBeg, segment.width, posXRelativeToText - segment.posX, OUT &unused, OUT &iChar);
                        pView->pCursors[cursorIndex].iCharAbs = segment.iCharBeg + iChar;
                    }
                }

                return true;
            }
        } while (drte_engine__next_segment_on_line(pView, &segment));

        // If we get here it means the position is to the right of the line. Just pin it to the end of the line.
        pView->pCursors[cursorIndex].iCharAbs = segment.iCharBeg;   // <-- segment.iCharBeg should be sitting on a new line or null terminator.

        return true;
    }

    return false;
}

bool drte_view_move_cursor_left(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    if (pView->pCursors[cursorIndex].iCharAbs == 0) {
        return false;   // Already at the start of the string. Nowhere to go.
    }

    size_t iPrevChar = pView->pCursors[cursorIndex].iCharAbs;
    size_t iPrevLine = pView->pCursors[cursorIndex].iLine;

    // Line boundary.
    if (iPrevLine > 0) {
        size_t iLineCharBeg = drte_view_get_line_first_character(pView, pView->pWrappedLines, iPrevLine);
        if (iLineCharBeg == iPrevChar) {
            drte_view_move_cursor_to_end_of_line_by_index(pView, cursorIndex, iPrevLine-1);
            if (pView->pCursors[cursorIndex].iCharAbs == iPrevChar) {
                pView->pCursors[cursorIndex].iCharAbs -= 1;
            }
        } else {
            pView->pCursors[cursorIndex].iCharAbs -= 1;
        }
    } else {
        pView->pCursors[cursorIndex].iCharAbs -= 1;
    }

    if (iPrevChar != pView->pCursors[cursorIndex].iCharAbs || iPrevLine != pView->pCursors[cursorIndex].iLine) {
        pView->pCursors[cursorIndex].iLine = drte_view_get_character_line(pView, pView->pWrappedLines, pView->pCursors[cursorIndex].iCharAbs);
        drte_view__update_cursor_sticky_position(pView, &pView->pCursors[cursorIndex]);

        drte_view_begin_dirty(pView);
            drte_engine__on_cursor_move(pView->pEngine, pView, cursorIndex);
            drte_view_dirty(pView, drte_view_get_local_rect(pView));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        drte_view_end_dirty(pView);
    }

    return true;
}

bool drte_view_move_cursor_right(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    if (pView->pCursors[cursorIndex].iCharAbs >= pView->pEngine->textLength) {
        return false;   // Already at the end. Nowhere to go.
    }

    size_t iPrevChar = pView->pCursors[cursorIndex].iCharAbs;
    size_t iPrevLine = pView->pCursors[cursorIndex].iLine;

    // Line boundary.
    if (iPrevLine+1 < drte_view_get_line_count(pView)) {
        size_t iLineCharEnd = drte_view_get_line_last_character(pView, pView->pWrappedLines, iPrevLine);
        if (iLineCharEnd == iPrevChar) {
            drte_view_move_cursor_to_start_of_line_by_index(pView, cursorIndex, iPrevLine+1);
        } else {
            pView->pCursors[cursorIndex].iCharAbs += 1;
        }
    } else {
        pView->pCursors[cursorIndex].iCharAbs += 1;
    }

    if (iPrevChar != pView->pCursors[cursorIndex].iCharAbs || iPrevLine != pView->pCursors[cursorIndex].iLine) {
        pView->pCursors[cursorIndex].iLine = drte_view_get_character_line(pView, pView->pWrappedLines, pView->pCursors[cursorIndex].iCharAbs);
        drte_view__update_cursor_sticky_position(pView, &pView->pCursors[cursorIndex]);

        drte_view_begin_dirty(pView);
            drte_engine__on_cursor_move(pView->pEngine, pView, cursorIndex);
            drte_view_dirty(pView, drte_view_get_local_rect(pView));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        drte_view_end_dirty(pView);
    }

    return true;
}

bool drte_view_move_cursor_up(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    return drte_view_move_cursor_y(pView, cursorIndex, -1);
}

bool drte_view_move_cursor_down(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    return drte_view_move_cursor_y(pView, cursorIndex, 1);
}

bool drte_view_move_cursor_y(drte_view* pView, size_t cursorIndex, int amount)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pView->pCursors[cursorIndex].iCharAbs;

    size_t lineCount = drte_view_get_line_count(pView);
    if (lineCount == 0) {
        return false;
    }

    // Moving a marker up or down depends on it's sticky position.
    intptr_t iNewLine = pView->pCursors[cursorIndex].iLine + amount;
    if (iNewLine < 0) {
        iNewLine = 0;
    }
    if ((size_t)iNewLine > lineCount) {
        iNewLine = lineCount - 1;
    }

    float newMarkerPosX = pView->pCursors[cursorIndex].absoluteSickyPosX;
    float newMarkerPosY = drte_view_get_line_pos_y(pView, (size_t)iNewLine);
    drte_view_move_cursor_to_point_relative_to_text(pView, cursorIndex, newMarkerPosX, newMarkerPosY);

    if (iPrevChar != pView->pCursors[cursorIndex].iCharAbs) {
        drte_view_begin_dirty(pView);
            drte_engine__on_cursor_move(pView->pEngine, pView, cursorIndex);
            drte_view_dirty(pView, drte_view_get_local_rect(pView));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        drte_view_end_dirty(pView);
    }

    return true;
}

bool drte_view_move_cursor_to_end_of_line(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    return drte_view_move_cursor_to_end_of_line_by_index(pView, cursorIndex, pView->pCursors[cursorIndex].iLine);
}

bool drte_view_move_cursor_to_start_of_line(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    return drte_view_move_cursor_to_start_of_line_by_index(pView, cursorIndex, pView->pCursors[cursorIndex].iLine);
}

bool drte_view_move_cursor_to_end_of_line_by_index(drte_view* pView, size_t cursorIndex, size_t iLine)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    drte_view_move_cursor_to_character_and_line(pView, cursorIndex, drte_view_get_line_last_character(pView, pView->pWrappedLines, iLine), iLine);
    return true;
}

bool drte_view_move_cursor_to_start_of_line_by_index(drte_view* pView, size_t cursorIndex, size_t iLine)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    drte_view_move_cursor_to_character_and_line(pView, cursorIndex, drte_view_get_line_first_character(pView, pView->pWrappedLines, iLine), iLine);
    return true;
}

bool drte_view_move_cursor_to_end_of_unwrapped_line(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    drte_view_move_cursor_to_character(pView, cursorIndex, drte_view_get_line_last_character(pView, pView->pEngine->pUnwrappedLines, drte_view_get_character_line(pView, pView->pEngine->pUnwrappedLines, pView->pCursors[cursorIndex].iCharAbs)));
    return true;
}

bool drte_view_move_cursor_to_start_of_unwrapped_line(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    drte_view_move_cursor_to_character(pView, cursorIndex, drte_view_get_line_first_character(pView, pView->pEngine->pUnwrappedLines, drte_view_get_character_line(pView, pView->pEngine->pUnwrappedLines, pView->pCursors[cursorIndex].iCharAbs)));
    return true;
}

bool drte_view_is_cursor_at_end_of_wrapped_line(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iCursorChar = pView->pCursors[cursorIndex].iCharAbs;
    size_t iCursorLine = pView->pCursors[cursorIndex].iLine;

    size_t iWrappedLineLastChar = drte_view_get_line_last_character(pView, pView->pWrappedLines, iCursorLine);
    if (iCursorChar == iWrappedLineLastChar) {
        return true;
    }

    return false;
}

bool drte_view_is_cursor_at_start_of_wrapped_line(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iCursorChar = pView->pCursors[cursorIndex].iCharAbs;
    size_t iCursorLine = pView->pCursors[cursorIndex].iLine;

    size_t iWrappedLineLastChar = drte_view_get_line_first_character(pView, pView->pWrappedLines, iCursorLine);
    if (iCursorChar == iWrappedLineLastChar) {
        return true;
    }

    return false;
}

bool drte_view_move_cursor_to_end_of_text(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    drte_view_move_cursor_to_character(pView, cursorIndex, pView->pEngine->textLength);
    return true;
}

bool drte_view_move_cursor_to_start_of_text(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    drte_view_move_cursor_to_character(pView, cursorIndex, 0);
    return true;
}

void drte_view_move_cursor_to_start_of_selection(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->selectionCount == 0 || pView->cursorCount <= cursorIndex) {
        return;
    }

    drte_view_move_cursor_to_character(pView, cursorIndex, drte_region_normalize(pView->pSelections[pView->selectionCount-1]).iCharBeg);
}

void drte_view_move_cursor_to_end_of_selection(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->selectionCount == 0 || pView->cursorCount <= cursorIndex) {
        return;
    }

    drte_view_move_cursor_to_character(pView, cursorIndex, drte_region_normalize(pView->pSelections[pView->selectionCount-1]).iCharEnd);
}

void drte_view_move_cursor_to_character(drte_view* pView, size_t cursorIndex, size_t characterIndex)
{
    drte_view_move_cursor_to_character_and_line(pView, cursorIndex, characterIndex, drte_view_get_character_line(pView, pView->pWrappedLines, characterIndex));
}

void drte_view_move_cursor_to_character_and_line(drte_view* pView, size_t cursorIndex, size_t iChar, size_t iLine)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return;
    }

    // Clamp the character to the end of the string.
    if (iChar > pView->pEngine->textLength) {
        iChar = pView->pEngine->textLength;
    }

    size_t iPrevChar = pView->pCursors[cursorIndex].iCharAbs;
    size_t iPrevLine = pView->pCursors[cursorIndex].iLine;

    pView->pCursors[cursorIndex].iCharAbs = iChar;
    pView->pCursors[cursorIndex].iLine = iLine;

    if (iPrevChar != pView->pCursors[cursorIndex].iCharAbs || iPrevLine != pView->pCursors[cursorIndex].iLine) {
        drte_view_begin_dirty(pView);
            drte_engine__on_cursor_move(pView->pEngine, pView, cursorIndex);
            drte_view_dirty(pView, drte_view_get_local_rect(pView));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        drte_view_end_dirty(pView);
    }
}

size_t drte_view_move_cursor_to_end_of_word(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return 0;
    }

    size_t iChar = drte_view_get_cursor_character(pView, cursorIndex);
    if (!drte_is_symbol_or_whitespace(pView->pEngine->text[iChar])) {
        while (pView->pEngine->text[iChar] != '\0') {
            uint32_t c = pView->pEngine->text[iChar];
            if (drte_is_symbol_or_whitespace(c)) {
                break;
            }

            iChar += 1;
        }
    } else {
        iChar += 1;
    }



    drte_view_move_cursor_to_character(pView, cursorIndex, iChar);
    return iChar;
}

size_t drte_view_move_cursor_to_start_of_next_word(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return 0;
    }

    size_t iChar = drte_view_move_cursor_to_end_of_word(pView, cursorIndex);
    while (pView->pEngine->text[iChar] != '\0') {
        uint32_t c = pView->pEngine->text[iChar];
        if (!drte_is_whitespace(c)) {
            break;
        }

        iChar += 1;
    }

    drte_view_move_cursor_to_character(pView, cursorIndex, iChar);
    return iChar;
}

size_t drte_view_move_cursor_to_start_of_word(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return 0;
    }

    size_t iChar = drte_view_get_cursor_character(pView, cursorIndex);
    if (iChar == 0) {
        return 0;
    }

    iChar -= 1;

    // Skip whitespace.
    if (drte_is_whitespace(pView->pEngine->text[iChar])) {
        while (iChar > 0) {
            if (!drte_is_whitespace(pView->pEngine->text[iChar])) {
                break;
            }

            iChar -= 1;
        }
    }

    if (!drte_is_symbol_or_whitespace(pView->pEngine->text[iChar])) {
        while (iChar > 0) {
            uint32_t c = pView->pEngine->text[iChar-1];
            if (drte_is_symbol_or_whitespace(c)) {
                break;
            }

            iChar -= 1;
        }
    }

    drte_view_move_cursor_to_character(pView, cursorIndex, iChar);
    return iChar;
}

size_t drte_view_get_spaces_to_next_column_from_character(drte_view* pView, size_t iChar)
{
    if (pView == NULL || pView->pEngine->text == NULL) {
        return 0;
    }

    if (iChar > pView->pEngine->textLength) {
        iChar = pView->pEngine->textLength;
    }


    const float tabWidth = drte_view__get_tab_width_in_pixels(pView);

    float posX;
    float posY;
    drte_view_get_character_position(pView, pView->pWrappedLines, iChar, &posX, &posY);

    float tabColPosX = (posX + tabWidth) - ((size_t)posX % (size_t)tabWidth);

    return (size_t)(tabColPosX - posX) / pView->pEngine->styles[pView->pEngine->defaultStyleSlot].fontMetrics.spaceWidth;
}

size_t drte_view_get_spaces_to_next_column_from_cursor(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->pEngine->text == NULL || pView->cursorCount <= cursorIndex) {
        return 0;
    }

    return drte_view_get_spaces_to_next_column_from_character(pView, drte_view_get_cursor_character(pView, cursorIndex));
}

bool drte_view_is_cursor_at_start_of_selection(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->selectionCount == 0 || pView->cursorCount <= cursorIndex) {
        return false;
    }

    drte_region region = drte_region_normalize(pView->pSelections[pView->selectionCount-1]);
    return pView->pCursors[cursorIndex].iCharAbs == region.iCharBeg;
}

bool drte_view_is_cursor_at_end_of_selection(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->selectionCount == 0 || pView->cursorCount <= cursorIndex) {
        return false;
    }

    drte_region region = drte_region_normalize(pView->pSelections[pView->selectionCount-1]);
    return pView->pCursors[cursorIndex].iCharAbs == region.iCharEnd;
}

bool drte_view_get_word_under_cursor(drte_view* pView, size_t cursorIndex, size_t* pWordBegOut, size_t* pWordEndOut)
{
    if (pView == NULL || pView->cursorCount <= cursorIndex) {
        return false;
    }

    return drte_engine_get_word_containing_character(pView->pEngine, pView->pCursors[cursorIndex].iCharAbs, pWordBegOut, pWordEndOut);
}

bool drte_view_get_word_under_point(drte_view* pView, float posX, float posY, size_t* pWordBegOut, size_t* pWordEndOut)
{
    if (pView == NULL) {
        return false;
    }

    size_t iChar;
    if (!drte_view_get_character_under_point(pView, pView->pWrappedLines, posX, posY, &iChar, NULL)) {
        return false;
    }

    return drte_engine_get_word_containing_character(pView->pEngine, iChar, pWordBegOut, pWordEndOut);
}




bool drte_view_is_anything_selected(drte_view* pView)
{
    if (pView == NULL) {
        return false;
    }

    return pView->selectionCount > 0;
}

void drte_view_deselect_all(drte_view* pView)
{
    if (pView == NULL) {
        return;
    }

    pView->selectionCount = 0;

    drte_view_dirty(pView, drte_view_get_local_rect(pView));
}

void drte_view_select_all(drte_view* pView)
{
    if (pView == NULL) {
        return;
    }

    // Deselect everything first to ensure any multi-select stuff is cleared.
    drte_view_deselect_all(pView);

    drte_view_select(pView, 0, pView->pEngine->textLength);
}

void drte_view_select(drte_view* pView, size_t firstCharacter, size_t lastCharacter)
{
    if (pView == NULL) {
        return;
    }

    drte_view_begin_selection(pView, firstCharacter);
    drte_view_set_selection_end_point(pView, lastCharacter);

    drte_view_dirty(pView, drte_view_get_local_rect(pView));
}

void drte_view_select_word_under_cursor(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL || pView->cursorCount <= cursorIndex) {
        return;
    }

    size_t iWordBeg;
    size_t iWordEnd;
    drte_view_get_word_under_cursor(pView, cursorIndex, &iWordBeg, &iWordEnd);

    drte_view_select(pView, iWordBeg, iWordEnd);
}

size_t drte_view_get_selected_text(drte_view* pView, char* textOut, size_t textOutSize)
{
    // Safety.
    if (textOut != NULL && textOutSize > 0) {
        textOut[0] = '\0';
    }

    if (pView == NULL || (textOut != NULL && textOutSize == 0)) {
        return 0;
    }

    if (!drte_view_is_anything_selected(pView)) {
        return 0;
    }


    // The selected text is just every selection concatenated together.
    size_t length = 0;
    for (size_t iSelection = 0; iSelection < pView->selectionCount; ++iSelection) {
        drte_region region = drte_region_normalize(pView->pSelections[iSelection]);
        if (textOut != NULL) {
            drte__strncpy_s(textOut+length, textOutSize-length, pView->pEngine->text+region.iCharBeg, (region.iCharEnd - region.iCharBeg));
        }

        length += (region.iCharEnd - region.iCharBeg);
    }

    return length;
}

size_t drte_view_get_selection_first_line(drte_view* pView, size_t iSelection)
{
    if (pView == NULL || pView->selectionCount == 0) {
        return 0;
    }

    return drte_view_get_character_line(pView, pView->pWrappedLines, drte_region_normalize(pView->pSelections[iSelection]).iCharBeg);
}

size_t drte_view_get_selection_last_line(drte_view* pView, size_t iSelection)
{
    if (pView == NULL) {
        return 0;
    }

    return drte_view_get_character_line(pView, pView->pWrappedLines, drte_region_normalize(pView->pSelections[iSelection]).iCharEnd);
}

void drte_view_move_selection_anchor_to_end_of_line(drte_view* pView, size_t iLine)
{
    drte_view_set_selection_anchor(pView, drte_view_get_line_last_character(pView, pView->pWrappedLines, iLine));
}

void drte_view_move_selection_anchor_to_start_of_line(drte_view* pView, size_t iLine)
{
    drte_view_set_selection_anchor(pView, drte_view_get_line_first_character(pView, pView->pWrappedLines, iLine));
}

size_t drte_view_get_selection_anchor_line(drte_view* pView)
{
    if (pView == NULL || pView->selectionCount == 0) {
        return 0;
    }

    return drte_view_get_character_line(pView, pView->pWrappedLines, pView->pSelections[pView->selectionCount-1].iCharBeg);
}


void drte_view_begin_selection(drte_view* pView, size_t iCharBeg)
{
    if (pView == NULL) {
        return;
    }

    drte_region* pNewSelections = (drte_region*)realloc(pView->pSelections, (pView->selectionCount + 1) * sizeof(*pNewSelections));
    if (pNewSelections == NULL) {
        return;
    }

    pView->pSelections = pNewSelections;
    pView->pSelections[pView->selectionCount].iCharBeg = iCharBeg;
    pView->pSelections[pView->selectionCount].iCharEnd = iCharBeg;
    pView->selectionCount += 1;
}

void drte_view_cancel_selection(drte_view* pView, size_t iSelection)
{
    if (pView == NULL || pView->selectionCount == 0) {
        return;
    }

    for (/* Do Nothing */; iSelection < pView->selectionCount-1; ++iSelection) {
        pView->pSelections[iSelection] = pView->pSelections[iSelection+1];
    }

    pView->selectionCount -= 1;
}

void drte_view_cancel_last_selection(drte_view* pView)
{
    if (pView == NULL || pView->selectionCount == 0) {
        return;
    }

    pView->selectionCount -= 1;
}

void drte_view_set_selection_anchor(drte_view* pView, size_t iCharBeg)
{
    if (pView == NULL || pView->selectionCount == 0) {
        return;
    }

    if (pView->pSelections[pView->selectionCount-1].iCharBeg != iCharBeg) {
        pView->pSelections[pView->selectionCount-1].iCharBeg = iCharBeg;
        drte_view__repaint(pView);
    }
}

void drte_view_set_selection_end_point(drte_view* pView, size_t iCharEnd)
{
    if (pView == NULL || pView->selectionCount == 0) {
        return;
    }

    if (pView->pSelections[pView->selectionCount-1].iCharEnd != iCharEnd) {
        pView->pSelections[pView->selectionCount-1].iCharEnd = iCharEnd;
        drte_view__repaint(pView);
    }
}

bool drte_view_get_last_selection(drte_view* pView, size_t* iCharBegOut, size_t* iCharEndOut)
{
    if (pView == NULL || pView->selectionCount == 0) {
        return false;
    }

    drte_region selection = drte_region_normalize(pView->pSelections[pView->selectionCount-1]);

    if (iCharBegOut) *iCharBegOut = selection.iCharBeg;
    if (iCharEndOut) *iCharEndOut = selection.iCharEnd;
    return true;
}

bool drte_view_get_selection_under_point(drte_view* pView, float posX, float posY, size_t* piSelectionOut)
{
    if (pView == NULL) return false;

    size_t iChar;
    if (!drte_view_get_character_under_point(pView, pView->pWrappedLines, posX, posY, &iChar, NULL)) {
        return false;
    }

    for (size_t iSelection = 0; iSelection < pView->selectionCount; ++iSelection) {
        drte_region selection = drte_region_normalize(pView->pSelections[iSelection]);
        if (iChar >= selection.iCharBeg && iChar < selection.iCharEnd) {
            if (piSelectionOut) *piSelectionOut = iSelection;
            return true;
        }
    }

    return false;
}




bool drte_view_insert_character_at_cursor(drte_view* pView, size_t cursorIndex, unsigned int character)
{
    if (pView == NULL) {
        return false;
    }

    drte_view_begin_dirty(pView);
    {
        drte_engine_insert_character(pView->pEngine, pView->pCursors[cursorIndex].iCharAbs, character);
    }
    drte_view_end_dirty(pView);


    // The cursor's sticky position needs to be updated whenever the text is edited.
    drte_view__update_cursor_sticky_position(pView, &pView->pCursors[cursorIndex]);


    drte_engine__on_cursor_move(pView->pEngine, pView, cursorIndex);

    return true;
}

bool drte_view_insert_character_at_cursors(drte_view* pView, unsigned int character)
{
    if (pView == NULL) {
        return false;
    }

    // TODO: This can be improved because it is posting multiple onTextChanged messages.

    bool wasTextChanged = false;
    drte_view_begin_dirty(pView);
    {
        for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
            if (!drte_view_insert_character_at_cursor(pView, iCursor, character)) {
                continue;
            } else {
                wasTextChanged = true;
            }
        }
    }
    drte_view_end_dirty(pView);

    return wasTextChanged;
}

bool drte_view_insert_text_at_cursor(drte_view* pView, size_t cursorIndex, const char* text)
{
    if (pView == NULL || text == NULL) {
        return false;
    }

    drte_view_begin_dirty(pView);
    {
        size_t cursorPos = pView->pCursors[cursorIndex].iCharAbs;
        drte_engine_insert_text(pView->pEngine, text, cursorPos);
    }
    drte_view_end_dirty(pView);


    // The cursor's sticky position needs to be updated whenever the text is edited.
    drte_view__update_cursor_sticky_position(pView, &pView->pCursors[cursorIndex]);

    drte_engine__on_cursor_move(pView->pEngine, pView, cursorIndex);

    return true;
}

bool drte_view_delete_character_to_left_of_cursor(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL) {
        return false;
    }

    // We just move the cursor to the left, and then delete the character to the right.
    if (drte_view_move_cursor_left(pView, cursorIndex)) {
        drte_view_delete_character_to_right_of_cursor(pView, cursorIndex);
        return true;
    }

    return false;
}

bool drte_view_delete_character_to_left_of_cursors(drte_view* pView, bool leaveNewLines)
{
    if (pView == NULL) {
        return false;
    }

    bool wasTextChanged = false;
    drte_view_begin_dirty(pView);
    {
        for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
            size_t iCursorChar = pView->pCursors[iCursor].iCharAbs;
            if (iCursorChar == 0) {
                continue;
            }

            if (leaveNewLines) {
                size_t iLineCharBeg = drte_view_get_line_first_character(pView, pView->pWrappedLines, drte_view_get_cursor_line(pView, iCursor));
                if (iCursorChar == iLineCharBeg) {
                    continue;
                }
            }

            if (!drte_view_delete_character_to_left_of_cursor(pView, iCursor)) {
                continue;
            }

            wasTextChanged = true;
        }
    }
    drte_view_end_dirty(pView);

    return wasTextChanged;
}

bool drte_view_delete_character_to_right_of_cursor(drte_view* pView, size_t cursorIndex)
{
    if (pView == NULL) {
        return false;
    }

    size_t iCharBeg = pView->pCursors[cursorIndex].iCharAbs;
    if (iCharBeg < pView->pEngine->textLength)
    {
        size_t iCharEnd = iCharBeg+1;
        if (pView->pEngine->text[iCharBeg] == '\r' && pView->pEngine->text[iCharEnd] == '\n') {
            iCharEnd += 1;  // It's a \r\n line ending.
        }

        if (!drte_engine_delete_text(pView->pEngine, iCharBeg, iCharEnd)) {
            return false;
        }


        // The layout will have changed.
        drte_view_move_cursor_to_character(pView, cursorIndex, iCharBeg);

        if (pView->pEngine->onTextChanged) {
            pView->pEngine->onTextChanged(pView->pEngine);
        }

        drte_view_dirty(pView, drte_view_get_local_rect(pView));

        return true;
    }

    return false;
}

bool drte_view_delete_character_to_right_of_cursors(drte_view* pView, bool leaveNewLines)
{
    if (pView == NULL) {
        return false;
    }

    bool wasTextChanged = false;
    drte_view_begin_dirty(pView);
    {
        for (size_t iCursor = 0; iCursor < pView->cursorCount; ++iCursor) {
            size_t iCursorChar = pView->pCursors[iCursor].iCharAbs;
            if (leaveNewLines) {
                size_t iLineCharEnd = drte_view_get_line_last_character(pView, pView->pWrappedLines, drte_view_get_cursor_line(pView, iCursor));
                if (iCursorChar == iLineCharEnd) {
                    continue;
                }
            }

            if (!drte_view_delete_character_to_right_of_cursor(pView, iCursor)) {
                continue;
            }

            wasTextChanged = true;
        }
    }
    drte_view_end_dirty(pView);

    return wasTextChanged;
}


bool drte_view_delete_selected_text(drte_view* pView)
{
    if (pView == NULL || pView->selectionCount == 0) {
        return false;
    }

    bool wasTextChanged = false;
    drte_view_begin_dirty(pView);
    {
        for (size_t iSelection = 0; iSelection < pView->selectionCount; ++iSelection) {
            wasTextChanged = drte_view_delete_selection_text(pView, iSelection) || wasTextChanged;
        }
    }
    drte_view_end_dirty(pView);

    return wasTextChanged;
}

bool drte_view_delete_selection_text(drte_view* pView, size_t iSelectionToDelete)
{
    if (pView == NULL || pView->selectionCount == 0) {
        return false;
    }

    drte_region selectionToDelete = drte_region_normalize(pView->pSelections[iSelectionToDelete]);
    if (selectionToDelete.iCharBeg == selectionToDelete.iCharEnd) {
        return false;   // Nothing is selected.
    }

    return drte_engine_delete_text(pView->pEngine, selectionToDelete.iCharBeg, selectionToDelete.iCharEnd);
}


bool drte_view_find_next(drte_view* pView, const char* text, size_t* pSelectionStartOut, size_t* pSelectionEndOut)
{
    if (pView == NULL || pView->pEngine == NULL || pView->pEngine->text == NULL || text == NULL || text[0] == '\0') {
        return false;
    }

    size_t cursorPos = 0;
    if (pView->cursorCount > 0) {
        cursorPos = pView->pCursors[pView->cursorCount-1].iCharAbs;
    }

    char* nextOccurance = strstr(pView->pEngine->text + cursorPos, text);
    if (nextOccurance == NULL) {
        nextOccurance = strstr(pView->pEngine->text, text);
    }

    if (nextOccurance == NULL) {
        return false;
    }

    if (pSelectionStartOut) {
        *pSelectionStartOut = nextOccurance - pView->pEngine->text;
    }
    if (pSelectionEndOut) {
        *pSelectionEndOut = (nextOccurance - pView->pEngine->text) + strlen(text);
    }

    return true;
}

bool drte_view_find_next_no_loop(drte_view* pView, const char* text, size_t* pSelectionStartOut, size_t* pSelectionEndOut)
{
    if (pView == NULL || pView->pEngine == NULL || pView->pEngine->text == NULL || text == NULL || text[0] == '\0') {
        return false;
    }

    size_t cursorPos = 0;
    if (pView->cursorCount > 0) {
        cursorPos = pView->pCursors[pView->cursorCount-1].iCharAbs;
    }

    char* nextOccurance = strstr(pView->pEngine->text + cursorPos, text);
    if (nextOccurance == NULL) {
        return false;
    }

    if (pSelectionStartOut) {
        *pSelectionStartOut = nextOccurance - pView->pEngine->text;
    }
    if (pSelectionEndOut) {
        *pSelectionEndOut = (nextOccurance - pView->pEngine->text) + strlen(text);
    }

    return true;
}


#endif  //DR_TEXT_ENGINE_IMPLEMENTATION


// REQUIREMENTS
//
// - Full UTF-8, UTF-16 (LE and BE) and UTF-32 support
//   - UTF-8 internally, high-level conversion routines at a higher level for UTF-16 and UTF-32.
// - Syntax highlighting
//   - Must be able to plug-in custom highlighters.
// - Efficient editing of huge files
//   - Reallocating and moving around a monolithic buffer is way too slow.
//   - Need a way to easily split the file into chunks.
//   - Need to easily pick out individual characters by their index.
//   - Need a very simple high-level API while still maintaining good performance characteristics.
// - Notifications for when a region have become invalid and needs redrawing
// - Getting words under a specific point in the text region.
//   - Useful for right-click context menus and whatnot (go to definition, etc.)
// - Multi-line editing and selection
//   - Box selection, just like MSVC's ALT+LMB selection.
// - Carets
//   - Block caret for insert mode.
// - Overwrite mode
//
//
// BRAINSTORMING


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

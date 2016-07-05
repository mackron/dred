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


// BRAINSTORMING


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
// - Windows style line endings (\r\n) are converted to \n for the sake of simplicity. Keep this in mind when doing style highlighting which
//   takes character indices mark the styled region.


#ifndef dr_text_engine_h
#define dr_text_engine_h

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct drte_engine drte_engine;
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


typedef void   (* drte_engine_on_measure_string_proc)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textLength, float* pWidthOut, float* pHeightOut);
typedef void   (* drte_engine_on_get_cursor_position_from_point_proc)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut);
typedef void   (* drte_engine_on_get_cursor_position_from_char_proc)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t characterIndex, float* pTextCursorPosXOut);
typedef bool   (* drte_engine_on_get_next_highlight_proc)(drte_engine* pEngine, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut, drte_style_token* pStyleTokenOut, void* pUserData);
               
typedef void   (* drte_engine_on_paint_text_proc)        (drte_engine* pEngine, drte_style_token styleTokenFG, drte_style_token styleTokenBG, const char* text, size_t textLength, float posX, float posY, drgui_element* pElement, void* pPaintData);
typedef void   (* drte_engine_on_paint_rect_proc)        (drte_engine* pEngine, drte_style_token styleToken, drgui_rect rect, drgui_element* pElement, void* pPaintData);
typedef void   (* drte_engine_on_cursor_move_proc)       (drte_engine* pEngine);
typedef void   (* drte_engine_on_dirty_proc)             (drte_engine* pEngine, drgui_rect rect);
typedef void   (* drte_engine_on_text_changed_proc)      (drte_engine* pEngine);
typedef void   (* drte_engine_on_undo_point_changed_proc)(drte_engine* pEngine, unsigned int iUndoPoint);
typedef size_t (* drte_engine_on_get_undo_state_proc)    (drte_engine* pEngine, void* pDataOut);
typedef void   (* drte_engine_on_apply_undo_state_proc)  (drte_engine* pEngine, size_t dataSize, const void* pData);

typedef struct
{
    // The index of the character the marker is positioned at. This will be to the left of the character.
    size_t iCharAbs;

    // The absolute position on the x axis to place the marker when moving up and down lines. This is relative to
    // the left position of the line. This will be updated when the marker is moved left and right.
    float absoluteSickyPosX;

} drte_marker;

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

/// Keeps track of the current state of the text engine. Used for calculating the difference between two states for undo/redo.
typedef struct
{
    // The text. Can be null in some cases where it isn't used.
    char* text;

    // The list of cursors.
    drte_marker* pCursors;

    // The cursor count.
    size_t cursorCount;

    // The list of selection regions at the time the state was captured.
    drte_region* pSelections;

    // The selection count.
    size_t selectionCount;

    // The size of the application-defined data.
    size_t userDataSize;

    // A pointer to the application-defined data. This is malloc'd and free'd.
    void* pUserData;

} drte_engine_state;

typedef struct
{
    /// The position in the main string where the change is located. The length of the relevant string is used to determines how
    /// large of a chunk of text needs to be replaced.
    size_t diffPos;

    /// The string that was replaced. On undo, this will be inserted into the text engine. Can be empty, in which case this state
    /// object was created in response to an insert operation.
    char* oldText;

    /// The string that replaces the old text. On redo, this will be inserted into the text engine. This can be empty, in which case
    /// this state object was created in response to a delete operation.
    char* newText;

    /// The state of the text engine at the time the undo point was prepared, not including the text. The <text> attribute
    /// of this object is always null.
    drte_engine_state oldState;

    /// The state of the text engine at the time the undo point was committed, not including the text. The <text> attribute
    /// of this object is always null.
    drte_engine_state newState;

} drte_engine_undo_state;

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


    // The index of the first character of every line.
    size_t* pLines;
    size_t lineCount;
    size_t lineBufferSize;


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


    /// The width of the container.
    float containerWidth;

    /// The height of the container.
    float containerHeight;

    /// The inner offset of the container.
    float innerOffsetX;

    /// The inner offset of the container.
    float innerOffsetY;


    /// The size of a tab in spaces.
    unsigned int tabSizeInSpaces;

    /// The width of the text cursor.
    float cursorWidth;

    /// The blink rate in milliseconds of the cursor.
    unsigned int cursorBlinkRate;

    /// The amount of time in milliseconds to toggle the cursor's blink state.
    unsigned int timeToNextCursorBlink;

    /// Whether or not the cursor is showing based on it's blinking state.
    bool isCursorBlinkOn;

    /// Whether or not the cursor is being shown. False by default.
    bool isShowingCursor;


    // The list of active cursors.
    drte_marker* pCursors;

    // The number of active cursors.
    size_t cursorCount;


    // The list of selection regions.
    drte_region* pSelections;

    // The number of active selection regions. When this is 0, nothing is selected.
    size_t selectionCount;


    /// The function to call when a text run needs to be painted.
    drte_engine_on_paint_text_proc onPaintText;

    /// The function to call when a rectangle needs to be painted.
    drte_engine_on_paint_rect_proc onPaintRect;

    /// The function to call when the cursor moves.
    drte_engine_on_cursor_move_proc onCursorMove;


    /// The prepared undo/redo state. This will be filled with some state by PrepareUndoRedoPoint() and again with CreateUndoRedoPoint().
    drte_engine_state preparedState;

    /// The undo/redo stack.
    drte_engine_undo_state* pUndoStack;

    /// The number of items in the undo/redo stack.
    unsigned int undoStackCount;

    /// The index of the undo/redo state item we are currently sitting on.
    unsigned int iUndoState;


    /// The counter used to determine when an onDirty event needs to be posted.
    unsigned int dirtyCounter;

    /// The accumulated dirty rectangle. When dirtyCounter hits 0, this is the rectangle that's posted to the onDirty callback.
    drgui_rect accumulatedDirtyRect;



    // Application-defined data.
    void* pUserData;
};


/// Creates a new text engine object.
drte_engine* drte_engine_create(drgui_context* pContext, void* pUserData);

/// Deletes the given text engine.
void drte_engine_delete(drte_engine* pEngine);


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


// Retrieves the index of the line containing the character at the given index.
size_t drte_engine_get_character_line(drte_engine* pEngine, size_t characterIndex);

// Retrieves the position of the character at the given index, relative to the text rectangle.
//
// To make the position relative to the container simply add the inner offset to them.
void drte_engine_get_character_position(drte_engine* pEngine, size_t characterIndex, float* pPosXOut, float* pPosYOut);

// Retrieves the closest character to the given point relative to the text.
size_t drte_engine_get_character_by_point(drte_engine* pEngine, float inputPosXRelativeToText, float inputPosYRelativeToText);

// Retrieves the closest character to the given point relative to the container.
size_t drte_engine_get_character_by_point_relative_to_container(drte_engine* pEngine, float inputPosXRelativeToContainer, float inputPosYRelativeToContainer);


// Gets the character at the given index as a UTF-32 code point.
uint32_t drte_engine_get_utf32(drte_engine* pEngine, size_t characterIndex);

// Retrieves the indices of the visible lines.
void drte_engine_get_visible_lines(drte_engine* pEngine, size_t* pFirstLineOut, size_t* pLastLineOut);


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


/// Sets the size of the container.
void drte_engine_set_container_size(drte_engine* pEngine, float containerWidth, float containerHeight);

/// Retrieves the size of the container.
void drte_engine_get_container_size(drte_engine* pEngine, float* pContainerWidthOut, float* pContainerHeightOut);

/// Retrieves the width of the container.
float drte_engine_get_container_width(drte_engine* pEngine);

/// Retrieves the height of the container.
float drte_engine_get_container_height(drte_engine* pEngine);


/// Sets the inner offset of the given text engine.
void drte_engine_set_inner_offset(drte_engine* pEngine, float innerOffsetX, float innerOffsetY);

/// Sets the inner offset of the given text engine on the x axis.
void drte_engine_set_inner_offset_x(drte_engine* pEngine, float innerOffsetX);

/// Sets the inner offset of the given text engine on the y axis.
void drte_engine_set_inner_offset_y(drte_engine* pEngine, float innerOffsetY);

/// Retrieves the inner offset of the given text engine.
void drte_engine_get_inner_offset(drte_engine* pEngine, float* pInnerOffsetX, float* pInnerOffsetY);

/// Retrieves the inner offset of the given text engine on the x axis.
float drte_engine_get_inner_offset_x(drte_engine* pEngine);

/// Retrieves the inner offset of the given text engine on the x axis.
float drte_engine_get_inner_offset_y(drte_engine* pEngine);


/// Sets the size of a tab in spaces.
void drte_engine_set_tab_size(drte_engine* pEngine, unsigned int sizeInSpaces);

/// Retrieves the size of a tab in spaces.
unsigned int drte_engine_get_tab_size(drte_engine* pEngine);


/// Sets the width of the text cursor.
void drte_engine_set_cursor_width(drte_engine* pEngine, float cursorWidth);

/// Retrieves the width of the text cursor.
float drte_engine_get_cursor_width(drte_engine* pEngine);

/// Sets the blink rate of the cursor in milliseconds.
void drte_engine_set_cursor_blink_rate(drte_engine* pEngine, unsigned int blinkRateInMilliseconds);

/// Retrieves the blink rate of the cursor in milliseconds.
unsigned int drte_engine_get_cursor_blink_rate(drte_engine* pEngine);

/// Shows the cursor.
void drte_engine_show_cursor(drte_engine* pEngine);

/// Hides the cursor.
void drte_engine_hide_cursor(drte_engine* pEngine);

/// Determines whether or not the cursor is visible.
bool drte_engine_is_showing_cursor(drte_engine* pEngine);

// Inserts a new cursor. Returns the index of the new cursor.
size_t drte_engine_insert_cursor(drte_engine* pEngine, size_t iChar);

// Removes a cursor by it's index.
void drte_engine_remove_cursor(drte_engine* pEngine, size_t cursorIndex);

// Removes any overlapping cursors, leaving only one of the overlapping cursors remaining.
void drte_engine_remove_overlapping_cursors(drte_engine* pEngine);

// Retrieves the last cursor.
size_t drte_engine_get_last_cursor(drte_engine* pEngine);

/// Retrieves the position of the cursor, relative to the container.
void drte_engine_get_cursor_position(drte_engine* pEngine, size_t cursorIndex, float* pPosXOut, float* pPosYOut);

/// Retrieves the rectangle of the cursor, relative to the container.
drgui_rect drte_engine_get_cursor_rect(drte_engine* pEngine, size_t cursorIndex);

/// Retrieves the index of the line the cursor is currently sitting on.
size_t drte_engine_get_cursor_line(drte_engine* pEngine, size_t cursorIndex);

/// Retrieves the index of the column the cursor is currently sitting on.
size_t drte_engine_get_cursor_column(drte_engine* pEngine, size_t cursorIndex);

/// Retrieves the index of the character the cursor is currently sitting on.
size_t drte_engine_get_cursor_character(drte_engine* pEngine, size_t cursorIndex);

/// Moves the cursor to the closest character based on the given input position.
void drte_engine_move_cursor_to_point(drte_engine* pEngine, size_t cursorIndex, float posX, float posY);

/// Moves the cursor of the given text engine to the left by one character.
bool drte_engine_move_cursor_left(drte_engine* pEngine, size_t cursorIndex);

/// Moves the cursor of the given text engine to the right by one character.
bool drte_engine_move_cursor_right(drte_engine* pEngine, size_t cursorIndex);

/// Moves the cursor of the given text engine up one line.
bool drte_engine_move_cursor_up(drte_engine* pEngine, size_t cursorIndex);

/// Moves the cursor of the given text engine down one line.
bool drte_engine_move_cursor_down(drte_engine* pEngine, size_t cursorIndex);

/// Moves the cursor up or down the given number of lines.
bool drte_engine_move_cursor_y(drte_engine* pEngine, size_t cursorIndex, int amount);

/// Moves the cursor of the given text engine to the end of the line.
bool drte_engine_move_cursor_to_end_of_line(drte_engine* pEngine, size_t cursorIndex);

/// Moves the cursor of the given text engine to the start of the line.
bool drte_engine_move_cursor_to_start_of_line(drte_engine* pEngine, size_t cursorIndex);

/// Moves the cursor of the given text engine to the end of the line at the given index.
bool drte_engine_move_cursor_to_end_of_line_by_index(drte_engine* pEngine, size_t cursorIndex, size_t iLine);

/// Moves the cursor of the given text engine to the start of the line at the given index.
bool drte_engine_move_cursor_to_start_of_line_by_index(drte_engine* pEngine, size_t cursorIndex, size_t iLine);

/// Moves the cursor of the given text engine to the end of the text.
bool drte_engine_move_cursor_to_end_of_text(drte_engine* pEngine, size_t cursorIndex);

/// Moves the cursor of the given text engine to the end of the text.
bool drte_engine_move_cursor_to_start_of_text(drte_engine* pEngine, size_t cursorIndex);

/// Moves the cursor to the start of the selected text.
void drte_engine_move_cursor_to_start_of_selection(drte_engine* pEngine, size_t cursorIndex);

/// Moves the cursor to the end of the selected text.
void drte_engine_move_cursor_to_end_of_selection(drte_engine* pEngine, size_t cursorIndex);

/// Moves the cursor to the given character index.
void drte_engine_move_cursor_to_character(drte_engine* pEngine, size_t cursorIndex, size_t characterIndex);

// Moves the cursor to the end of the current word.
size_t drte_engine_move_cursor_to_end_of_word(drte_engine* pEngine, size_t cursorIndex);

// Moves the cursor to the start of the next word.
size_t drte_engine_move_cursor_to_start_of_next_word(drte_engine* pEngine, size_t cursorIndex);

// Moves the cursor to the start of the current word.
size_t drte_engine_move_cursor_to_start_of_word(drte_engine* pEngine, size_t cursorIndex);

// Retrieves the number of characters between the given character index and the next tab column.
size_t drte_engine_get_spaces_to_next_column_from_character(drte_engine* pEngine, size_t iChar);

/// Retrieves the number of characters between the cursor and the next tab column.
size_t drte_engine_get_spaces_to_next_column_from_cursor(drte_engine* pEngine, size_t cursorIndex);

/// Determines whether or not the cursor is sitting at the start of the selection.
bool drte_engine_is_cursor_at_start_of_selection(drte_engine* pEngine, size_t cursorIndex);

/// Determines whether or not the cursor is sitting at the end fo the selection.
bool drte_engine_is_cursor_at_end_of_selection(drte_engine* pEngine, size_t cursorIndex);

/// Sets the function to call when the cursor in the given text engine is mvoed.
void drte_engine_set_on_cursor_move(drte_engine* pEngine, drte_engine_on_cursor_move_proc proc);


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

/// Inserts a character at the position of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_insert_character_at_cursor(drte_engine* pEngine, size_t cursorIndex, unsigned int character);
bool drte_engine_insert_character_at_cursors(drte_engine* pEngine, unsigned int character);

/// Inserts a character at the position of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_insert_text_at_cursor(drte_engine* pEngine, size_t cursorIndex, const char* text);

/// Deletes the character to the left of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_delete_character_to_left_of_cursor(drte_engine* pEngine, size_t cursorIndex);
bool drte_engine_delete_character_to_left_of_cursors(drte_engine* pEngine, bool leaveNewLines);

/// Deletes the character to the right of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_delete_character_to_right_of_cursor(drte_engine* pEngine, size_t cursorIndex);
bool drte_engine_delete_character_to_right_of_cursors(drte_engine* pEngine, bool leaveNewLines);

/// Deletes the currently selected text.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_delete_selected_text(drte_engine* pEngine, bool updateCursors);

// Deletes the text of a specific selection.
bool drte_engine_delete_selection_text(drte_engine* pEngine, size_t iSelectionToDelete, bool updateCursorsAndSelection);


/// Determines whether or not anything is selected in the given text engine.
bool drte_engine_is_anything_selected(drte_engine* pEngine);

/// Deselects everything in the given text engine.
void drte_engine_deselect_all(drte_engine* pEngine);

/// Selects everything in the given text engine.
void drte_engine_select_all(drte_engine* pEngine);

/// Selects the given range of text.
void drte_engine_select(drte_engine* pEngine, size_t firstCharacter, size_t lastCharacter);

// Selects the word under a cursor.
void drte_engine_select_word_under_cursor(drte_engine* pEngine, size_t cursorIndex);

/// Retrieves a copy of the selected text.
///
/// @remarks
///     This returns the length of the selected text. Call this once with <textOut> set to NULL to calculate the required size of the
///     buffer.
///     @par
///     If the output buffer is not larger enough, the string will be truncated.
size_t drte_engine_get_selected_text(drte_engine* pEngine, char* textOut, size_t textOutLength);

/// Retrieves the index of the first line of the current selection.
size_t drte_engine_get_selection_first_line(drte_engine* pEngine, size_t iSelection);

/// Retrieves the index of the last line of the current selection.
size_t drte_engine_get_selection_last_line(drte_engine* pEngine, size_t iSelection);

/// Moves the selection anchor to the end of the given line.
void drte_engine_move_selection_anchor_to_end_of_line(drte_engine* pEngine, size_t iLine);

/// Moves the selection anchor to the start of the given line.
void drte_engine_move_selection_anchor_to_start_of_line(drte_engine* pEngine, size_t iLine);

/// Retrieves the line the selection anchor is sitting on.
size_t drte_engine_get_selection_anchor_line(drte_engine* pEngine);

// Begins a new selection region.
void drte_engine_begin_selection(drte_engine* pEngine, size_t iCharBeg);

// Cancels a selection by it's index.
void drte_engine_cancel_selection(drte_engine* pEngine, size_t iSelection);

// Cancels the most recent selection.
void drte_engine_cancel_last_selection(drte_engine* pEngine);

// Sets the anchor of the most recent selection region.
void drte_engine_set_selection_anchor(drte_engine* pEngine, size_t iCharBeg);

// Sets the end point of the most recent selection region.
void drte_engine_set_selection_end_point(drte_engine* pEngine, size_t iCharEnd);

// Retrieves the character range of the last selection, if any.
bool drte_engine_get_last_selection(drte_engine* pEngine, size_t* iCharBegOut, size_t* iCharEndOut);


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



/// Retrieves the number of lines in the given text engine.
size_t drte_engine_get_line_count(drte_engine* pEngine);

// Retrieves the number of lines that can fit on the visible region of the text engine.
//
// Use this for controlling the page size for scrollbars.
size_t drte_engine_get_visible_line_count(drte_engine* pEngine);

// Retrieves the width of the visible lines.
//
// Use this for implementing horizontal scrollbars.
float drte_engine_get_visible_line_width(drte_engine* pEngine);

/// Retrieves the position of the line at the given index on the y axis.
///
/// @remarks
///     Use this for calculating the inner offset for scrolling on the y axis.
float drte_engine_get_line_pos_y(drte_engine* pEngine, size_t iLine);

/// Finds the line under the given point on the y axis relative to the container.
size_t drte_engine_get_line_at_pos_y(drte_engine* pEngine, float posY);

/// Retrieves the index of the first character of the line at the given index.
size_t drte_engine_get_line_first_character(drte_engine* pEngine, size_t iLine);

/// Retrieves the index of the last character of the line at the given index.
size_t drte_engine_get_line_last_character(drte_engine* pEngine, size_t iLine);

/// Retrieves teh index of the first and last character of the line at the given index.
void drte_engine_get_line_character_range(drte_engine* pEngine, size_t iLine, size_t* pCharStartOut, size_t* pCharEndOut);


/// Sets the function to call when a run of text needs to be painted for the given text engine.
void drte_engine_set_on_paint_text(drte_engine* pEngine, drte_engine_on_paint_text_proc proc);

/// Sets the function to call when a quad needs to the be painted for the given text engine.
void drte_engine_set_on_paint_rect(drte_engine* pEngine, drte_engine_on_paint_rect_proc proc);

/// Paints the given text engine by calling the appropriate painting callbacks.
///
/// @remarks
///     Typically a text engine will be painted to a GUI element. A pointer to an element can be passed to this function
///     which will be passed to the callback functions. This is purely for convenience and nothing is actually drawn to
///     the element outside of the callback functions.
void drte_engine_paint(drte_engine* pEngine, drgui_rect rect, drgui_element* pElement, void* pPaintData);


/// Steps the given text engine by the given number of milliseconds.
///
/// @remarks
///     This will trigger the on_dirty callback when the cursor switches it's blink states.
void drte_engine_step(drte_engine* pEngine, unsigned int milliseconds);


/// Calls the given painting callbacks for the line numbers of the given text engine.
void drte_engine_paint_line_numbers(drte_engine* pEngine, float lineNumbersWidth, float lineNumbersHeight, drte_engine_on_paint_text_proc onPaintText, drte_engine_on_paint_rect_proc onPaintRect, drgui_element* pElement, void* pPaintData);


/// Finds the given string starting from the cursor and then looping back.
bool drte_engine_find_next(drte_engine* pEngine, const char* text, size_t* pSelectionStartOut, size_t* pSelectionEndOut);

/// Finds the given string starting from the cursor, but does not loop back.
bool drte_engine_find_next_no_loop(drte_engine* pEngine, const char* text, size_t* pSelectionStartOut, size_t* pSelectionEndOut);


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

#ifndef DRTE_PAGE_LINE_COUNT
#define DRTE_PAGE_LINE_COUNT    256
#endif

#define DRTE_INVALID_STYLE_SLOT 255

// Flags for the drte_engine::flags property.
#define DRTE_USE_EXPLICIT_LINE_HEIGHT   (1 << 0)



// min
#define drte_min(a, b) (((a) < (b)) ? (a) : (b))

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



// Performs a full repaint of the entire visible region of the text engine.
void drte_engine__repaint(drte_engine* pEngine);

/// Helper for calculating the width of a tab.
float drte_engine__get_tab_width(drte_engine* pEngine);

/// Creates a blank text marker.
drte_marker drte_engine__new_marker();

/// Moves the given text marker to the given point, relative to the container.
bool drte_engine__move_marker_to_point_relative_to_container(drte_engine* pEngine, drte_marker* pMarker, float inputPosX, float inputPosY);

/// Retrieves the position of the given text marker relative to the container.
void drte_engine__get_marker_position_relative_to_container(drte_engine* pEngine, drte_marker* pMarker, float* pPosXOut, float* pPosYOut);

/// Moves the marker to the given point, relative to the text rectangle.
bool drte_engine__move_marker_to_point(drte_engine* pEngine, drte_marker* pMarker, float inputPosXRelativeToText, float inputPosYRelativeToText);

/// Moves the given marker to the left by one character.
bool drte_engine__move_marker_left(drte_engine* pEngine, drte_marker* pMarker);

/// Moves the given marker to the right by one character.
bool drte_engine__move_marker_right(drte_engine* pEngine, drte_marker* pMarker);

/// Moves the given marker up one line.
bool drte_engine__move_marker_up(drte_engine* pEngine, drte_marker* pMarker);

/// Moves the given marker down one line.
bool drte_engine__move_marker_down(drte_engine* pEngine, drte_marker* pMarker);

/// Moves the given marker down one line.
bool drte_engine__move_marker_y(drte_engine* pEngine, drte_marker* pMarker, int amount);

/// Moves the given marker to the end of the line it's currently sitting on.
bool drte_engine__move_marker_to_end_of_line(drte_engine* pEngine, drte_marker* pMarker);

/// Moves the given marker to the start of the line it's currently sitting on.
bool drte_engine__move_marker_to_start_of_line(drte_engine* pEngine, drte_marker* pMarker);

/// Moves the given marker to the end of the line at the given index.
bool drte_engine__move_marker_to_end_of_line_by_index(drte_engine* pEngine, drte_marker* pMarker, size_t iLine);

/// Moves the given marker to the start of the line at the given index.
bool drte_engine__move_marker_to_start_of_line_by_index(drte_engine* pEngine, drte_marker* pMarker, size_t iLine);

/// Moves the given marker to the end of the text.
bool drte_engine__move_marker_to_end_of_text(drte_engine* pEngine, drte_marker* pMarker);

/// Moves the given marker to the start of the text.
bool drte_engine__move_marker_to_start_of_text(drte_engine* pEngine, drte_marker* pMarker);

/// Moves the given marker to the character at the given position.
bool drte_engine__move_marker_to_character(drte_engine* pEngine, drte_marker* pMarker, size_t iChar);


/// Updates the sticky position of the given marker.
void drte_engine__update_marker_sticky_position(drte_engine* pEngine, drte_marker* pMarker);


/// Retrieves the index of the character the given marker is located at.
size_t drte_engine__get_marker_absolute_char_index(drte_engine* pEngine, drte_marker* pMarker);



/// Removes the undo/redo state stack items after the current undo/redo point.
void drte_engine__trim_undo_stack(drte_engine* pEngine);

/// Initializes the given undo state object by diff-ing the given layout states.
bool drte_engine__diff_states(drte_engine_state* pPrevState, drte_engine_state* pCurrentState, drte_engine_undo_state* pUndoStateOut);

/// Uninitializes the given undo state object. This basically just free's the internal string.
void drte_engine__uninit_undo_state(drte_engine_undo_state* pUndoState);

/// Pushes an undo state onto the undo stack.
void drte_engine__push_undo_state(drte_engine* pEngine, drte_engine_undo_state* pUndoState);

/// Applies the given undo state.
void drte_engine__apply_undo_state(drte_engine* pEngine, drte_engine_undo_state* pUndoState);

/// Applies the given undo state as a redo operation.
void drte_engine__apply_redo_state(drte_engine* pEngine, drte_engine_undo_state* pUndoState);


/// Retrieves a rectangle relative to the given text engine that's equal to the size of the container.
drgui_rect drte_engine__local_rect(drte_engine* pEngine);


/// Called when a cursor moves.
void drte_engine__on_cursor_move(drte_engine* pEngine, size_t cursorIndex);

/// Called when the text engine needs to be redrawn.
void drte_engine__on_dirty(drte_engine* pEngine, drgui_rect rect);

/// Increments the counter. The counter is decremented with drte_engine__end_dirty(). Use this for batching redraws.
void drte_engine__begin_dirty(drte_engine* pEngine);

/// Decrements the dirty counter, and if it hits 0 posts the onDirty callback.
void drte_engine__end_dirty(drte_engine* pEngine);


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
bool drte_engine__get_next_selection_from_character(drte_engine* pEngine, uint32_t iChar, drte_region* pSelectionOut)
{
    assert(pEngine != NULL);
    assert(pSelectionOut != NULL);
    
    // Selections can be in any order. Need to first check every single one to determine if any are on top of the character. If so we
    // just return the first one. Otherwise we fall through to the next loop which finds the closest selection to the character.
    bool foundSelectionAfterChar = false;
    drte_region closestSelection;
    closestSelection.iCharBeg = (size_t)-1;
    closestSelection.iCharEnd = (size_t)-1;

    for (size_t iSelection = 0; iSelection < pEngine->selectionCount; ++iSelection) {
        drte_region selection = drte_region_normalize(pEngine->pSelections[iSelection]);
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


// A drte_segment object is used for iterating over the segments of a chunk of text.
typedef struct
{
    size_t iLine;
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

float drte_engine__measure_segment(drte_engine* pEngine, drte_segment* pSegment)
{
    assert(pEngine != NULL);
    assert(pSegment != NULL);

    float segmentWidth = 0;
    if (pSegment->iCharEnd > pSegment->iCharBeg) {
        uint32_t c = drte_engine_get_utf32(pEngine, pSegment->iCharBeg);
        if (c == '\t') {
            // It was a tab segment.
            float tabWidth = drte_engine__get_tab_width(pEngine);
            size_t tabCount = pSegment->iCharEnd - pSegment->iCharBeg;
            float nextTabPos = (float)((int)(pSegment->posX / tabWidth) + 1) * tabWidth;
            float distanceToNextTab = nextTabPos - pSegment->posX;
            segmentWidth = distanceToNextTab + ((tabCount-1) * tabWidth);
        } else if (c == '\n' || c == '\0') {
            // Add overhang if selected.
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

bool drte_engine__next_segment(drte_engine* pEngine, drte_segment* pSegment)
{
    assert(pEngine != NULL);
    assert(pSegment != NULL);

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
    }

    uint8_t fgStyleSlot = pEngine->defaultStyleSlot;
    uint8_t bgStyleSlot = pEngine->defaultStyleSlot;

    // Find the end of the next segment, but don't modify the segment yet. The reason for this is that we need to measure the segment later.
    size_t iCharBeg = pSegment->iCharEnd;
    size_t iCharEnd = iCharBeg;

    if (pEngine->cursorCount > 0 && pSegment->iLine == pSegment->iCursorLine) {
        bgStyleSlot = pEngine->activeLineStyleSlot;
    }

    
    bool isAnythingSelected = false;
    bool isInSelection = false;
    drte_region selection;
    if (drte_engine__get_next_selection_from_character(pEngine, iCharBeg, &selection)) {
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
        if (c == '\n') {
            pSegment->isAtEndOfLine = true;
            iCharEnd += 1;
        } else {
            for (;;) {
                c = pEngine->text[iCharEnd];
                if (c == '\0' || c == '\n') {
                    break;
                }

                if (c == '\t') {
                    if (pEngine->text[iCharBeg] != '\t') {
                        break;
                    } else {
                        // Group tabs into a single segment.
                        for (;;) {
                            c = pEngine->text[iCharEnd];
                            if (c == '\0' || c == '\n' || c != '\t') {
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
    pSegment->width = drte_engine__measure_segment(pEngine, pSegment);

    return true;
}

bool drte_engine__next_segment_on_line(drte_engine* pEngine, drte_segment* pSegment)
{
    assert(pEngine != NULL);
    assert(pSegment != NULL);

    if (pSegment->isAtEndOfLine) {
        return false;
    }

    return drte_engine__next_segment(pEngine, pSegment);
}

bool drte_engine__first_segment(drte_engine* pEngine, size_t iChar, drte_segment* pSegment)
{
    if (pEngine == NULL || pEngine->textLength == 0 || pSegment == NULL) {
        return false;
    }

    pSegment->iLine = drte_engine_get_character_line(pEngine, iChar);
    pSegment->iCursorLine = drte_engine_get_cursor_line(pEngine, pEngine->cursorCount-1);
    pSegment->iCharBeg = iChar;
    pSegment->iCharEnd = pSegment->iCharBeg;
    pSegment->fgStyleSlot = pEngine->defaultStyleSlot;
    pSegment->bgStyleSlot = pEngine->defaultStyleSlot;
    pSegment->posX = 0;
    pSegment->width = 0;
    pSegment->isAtEnd = false;
    pSegment->isAtEndOfLine = false;
    return drte_engine__next_segment(pEngine, pSegment);
}

bool drte_engine__first_segment_on_line(drte_engine* pEngine, size_t lineIndex, drte_segment* pSegment)
{
    if (pEngine == NULL || pEngine->textLength == 0 || pSegment == NULL) {
        return false;
    }

    pSegment->iLine = lineIndex;
    pSegment->iCursorLine = drte_engine_get_cursor_line(pEngine, pEngine->cursorCount-1);
    pSegment->iCharBeg = drte_engine_get_line_first_character(pEngine, lineIndex);
    pSegment->iCharEnd = pSegment->iCharBeg;
    pSegment->fgStyleSlot = pEngine->defaultStyleSlot;
    pSegment->bgStyleSlot = pEngine->defaultStyleSlot;
    pSegment->posX = 0;
    pSegment->width = 0;
    pSegment->isAtEnd = false;
    pSegment->isAtEndOfLine = false;
    return drte_engine__next_segment(pEngine, pSegment);
}


drte_engine* drte_engine_create(drgui_context* pContext, void* pUserData)
{
    if (pContext == NULL) {
        return NULL;
    }

    drte_engine* pEngine = (drte_engine*)calloc(1, sizeof(*pEngine));
    if (pEngine == NULL) {
        return NULL;
    }

    pEngine->defaultStyleSlot = DRTE_INVALID_STYLE_SLOT;
    pEngine->selectionStyleSlot = DRTE_INVALID_STYLE_SLOT;
    pEngine->activeLineStyleSlot = DRTE_INVALID_STYLE_SLOT;
    pEngine->cursorStyleSlot = DRTE_INVALID_STYLE_SLOT;
    pEngine->lineNumbersStyleSlot = DRTE_INVALID_STYLE_SLOT;

    pEngine->lineBufferSize = DRTE_PAGE_LINE_COUNT;
    pEngine->lineCount = 1; // <-- There's always at least one line in a text editor.
    pEngine->pLines = (size_t*)calloc(pEngine->lineBufferSize, sizeof(*pEngine->pLines));   // <-- calloc() is important here. It initializes the first line to 0.


    pEngine->pCursors = NULL;
    pEngine->cursorCount = 0;
    pEngine->pSelections = NULL;
    pEngine->selectionCount = 0;

    pEngine->tabSizeInSpaces       = 4;
    pEngine->cursorWidth           = 1;
    pEngine->cursorBlinkRate       = 500;
    pEngine->timeToNextCursorBlink = pEngine->cursorBlinkRate;
    pEngine->isCursorBlinkOn       = true;
    pEngine->isShowingCursor       = false;
    pEngine->accumulatedDirtyRect  = drgui_make_inside_out_rect();
    pEngine->pUserData             = pUserData;

    return pEngine;
}

void drte_engine_delete(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    drte_engine_clear_undo_stack(pEngine);
    
    free(pEngine->preparedState.text);
    free(pEngine->preparedState.pCursors);
    free(pEngine->preparedState.pSelections);
    free(pEngine->preparedState.pUserData);

    free(pEngine->pSelections);
    free(pEngine->pCursors);

    free(pEngine->text);
    free(pEngine);
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

        drte_engine__repaint(pEngine);
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
    drte_engine__repaint(pEngine);
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

    if (drte_engine_is_anything_selected(pEngine)) {
        drte_engine__repaint(pEngine);
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
    drte_engine__repaint(pEngine);
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
    drte_engine__repaint(pEngine);
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
    drte_engine__repaint(pEngine);
}


void drte_engine_set_highlighter(drte_engine* pEngine, drte_engine_on_get_next_highlight_proc proc, void* pUserData)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->onGetNextHighlight = proc;
    pEngine->pHighlightUserData = pUserData;

    drte_engine__repaint(pEngine);
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

    drte_engine__repaint(pEngine);
}

float drte_engine_get_line_height(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return pEngine->lineHeight;
}


size_t drte_engine_get_character_line(drte_engine* pEngine, size_t characterIndex)
{
    if (pEngine == NULL || characterIndex > pEngine->textLength) {
        return 0;
    }

    // TODO: Use an accelerated search for this.

#if 1
    // Linear search. Simple, but slow.
    size_t lineIndex = 0;
    for (size_t iLine = 0; iLine < pEngine->lineCount; ++iLine) {
        if (pEngine->pLines[iLine] > characterIndex) {
            break;
        }

        lineIndex = iLine;
    }

    return lineIndex;
#endif

#if 0
    // Brute force character by character.
    size_t lineIndex = 0;
    for (size_t i = 0; i < characterIndex; ++i) {
        if (pEngine->text[i] == '\n') {
            lineIndex += 1;
        }
    }

    return lineIndex;
#endif
}

void drte_engine_get_character_position(drte_engine* pEngine, size_t characterIndex, float* pPosXOut, float* pPosYOut)
{
    if (pPosXOut) *pPosXOut = 0;
    if (pPosYOut) *pPosYOut = 0;

    if (pEngine == NULL) {
        return;
    }

    size_t lineIndex = drte_engine_get_character_line(pEngine, characterIndex);

    float posX = 0;
    float posY = lineIndex * drte_engine_get_line_height(pEngine);

    drte_segment segment;
    if (drte_engine__first_segment_on_line(pEngine, lineIndex, &segment)) {
        do
        {
            if (characterIndex >= segment.iCharBeg && characterIndex < segment.iCharEnd) {
                // It's somewhere in this segment.
                if (drte_engine_get_utf32(pEngine, segment.iCharBeg) == '\t') {
                    // If the first character in the segment is a tab character, then every character in this segment
                    // will be a tab. The location of the character is rounded to the nearest tab column.
                    posX = segment.posX;

                    size_t tabCount = characterIndex - segment.iCharBeg;
                    if (tabCount > 0) {
                        float tabWidth = drte_engine__get_tab_width(pEngine);
                        float nextTabPos = (float)((int)(segment.posX / tabWidth) + 1) * tabWidth;
                        posX = nextTabPos + ((tabCount-1) * tabWidth);
                    }
                } else {
                    // We must refer to the backend in order to find the exact position of the character.
                    // TODO: Grab a copy of the string rather than a direct offset.
                    drte_style_token fgStyleToken = drte_engine__get_style_token(pEngine, segment.fgStyleSlot);
                    if (pEngine->onGetCursorPositionFromChar && fgStyleToken != 0) {
                        pEngine->onGetCursorPositionFromChar(pEngine, fgStyleToken, pEngine->text + segment.iCharBeg, characterIndex - segment.iCharBeg, &posX);
                        posX += segment.posX;
                    }
                }

                break;
            }
        } while (drte_engine__next_segment_on_line(pEngine, &segment));
    }

    if (pPosXOut) *pPosXOut = posX;
    if (pPosYOut) *pPosYOut = posY;
}

size_t drte_engine_get_character_by_point(drte_engine* pEngine, float inputPosXRelativeToText, float inputPosYRelativeToText)
{
    if (pEngine == NULL) {
        return 0;
    }

    size_t iLine = drte_engine_get_line_at_pos_y(pEngine, inputPosYRelativeToText);
    size_t iChar = 0;

    // Once we have the line, finding the specific character under the point is done by iterating over each segment and finding the one
    // containing the point on the x axis. Once the segment has been found, we use the backend to get the exact character.
    if (inputPosXRelativeToText < 0) {
        iChar = drte_engine_get_line_first_character(pEngine, (size_t)iLine);   // It's to the left of the line, so just pin it to the first character in the line.
        return iChar;
    }

    drte_segment segment;
    if (drte_engine__first_segment_on_line(pEngine, (size_t)iLine, &segment)) {
        do
        {
            if (inputPosXRelativeToText >= segment.posX && inputPosXRelativeToText < segment.posX + segment.width) {
                // It's somewhere on this run. If it's a tab segment it needs to be handled slightly differently because of the way tabs
                // are aligned to tab columns.
                if (drte_engine_get_utf32(pEngine, segment.iCharBeg) == '\t') {
                    const float tabWidth = drte_engine__get_tab_width(pEngine);

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

                    drte_style_token fgStyleToken = drte_engine__get_style_token(pEngine, segment.fgStyleSlot);
                    if (pEngine->onGetCursorPositionFromPoint) {
                        pEngine->onGetCursorPositionFromPoint(pEngine, fgStyleToken, pEngine->text + segment.iCharBeg, segment.iCharEnd - segment.iCharBeg, segment.width, inputPosXRelativeToText - segment.posX, OUT &unused, OUT &iCharTemp);
                        iChar = segment.iCharBeg + iCharTemp;
                    }
                }

                return iChar;
            }
        } while (drte_engine__next_segment_on_line(pEngine, &segment));

        // If we get here it means the position is to the right of the line. Just pin it to the end of the line.
        assert(drte_engine_get_utf32(pEngine, segment.iCharBeg) == '\n' || drte_engine_get_utf32(pEngine, segment.iCharBeg) == '\0');
        iChar = segment.iCharBeg;   // <-- segment.iCharBeg should be sitting on a new line or null terminator.

        return iChar;
    }

    return 0;
}

size_t drte_engine_get_character_by_point_relative_to_container(drte_engine* pEngine, float inputPosXRelativeToContainer, float inputPosYRelativeToContainer)
{
    if (pEngine == NULL) {
        return 0;
    }

    float inputPosXRelativeToText = inputPosXRelativeToContainer - pEngine->innerOffsetX;
    float inputPosYRelativeToText = inputPosYRelativeToContainer - pEngine->innerOffsetY;
    return drte_engine_get_character_by_point(pEngine, inputPosXRelativeToText, inputPosYRelativeToText);
}


uint32_t drte_engine_get_utf32(drte_engine* pEngine, size_t characterIndex)
{
    if (pEngine == NULL) {
        return 0;
    }

    // TODO: Handle UTF-8 properly.
    return pEngine->text[characterIndex];
}

void drte_engine_get_visible_lines(drte_engine* pEngine, size_t* pFirstLineOut, size_t* pLastLineOut)
{
    if (pEngine == NULL) {
        return;
    }

    size_t iFirstLine = (size_t)(-pEngine->innerOffsetY / drte_engine_get_line_height(pEngine));

    if (pFirstLineOut) {
        *pFirstLineOut = iFirstLine;
    }

    if (pLastLineOut) {
        size_t lineCount = drte_engine_get_line_count(pEngine);
        size_t iLastLine = iFirstLine + ((size_t)(pEngine->containerHeight / drte_engine_get_line_height(pEngine)));
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


    if (drgui__strcpy_s(textOut, textOutSize, (pEngine->text != NULL) ? pEngine->text : "") == 0) {
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


void drte_engine_set_container_size(drte_engine* pEngine, float containerWidth, float containerHeight)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->containerWidth  = containerWidth;
    pEngine->containerHeight = containerHeight;

    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
}

void drte_engine_get_container_size(drte_engine* pEngine, float* pContainerWidthOut, float* pContainerHeightOut)
{
    float containerWidth  = 0;
    float containerHeight = 0;

    if (pEngine != NULL)
    {
        containerWidth  = pEngine->containerWidth;
        containerHeight = pEngine->containerHeight;
    }


    if (pContainerWidthOut) {
        *pContainerWidthOut = containerWidth;
    }
    if (pContainerHeightOut) {
        *pContainerHeightOut = containerHeight;
    }
}

float drte_engine_get_container_width(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return pEngine->containerWidth;
}

float drte_engine_get_container_height(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return pEngine->containerHeight;
}


void drte_engine_set_inner_offset(drte_engine* pEngine, float innerOffsetX, float innerOffsetY)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->innerOffsetX = innerOffsetX;
    pEngine->innerOffsetY = innerOffsetY;

    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
}

void drte_engine_set_inner_offset_x(drte_engine* pEngine, float innerOffsetX)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->innerOffsetX = innerOffsetX;

    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
}

void drte_engine_set_inner_offset_y(drte_engine* pEngine, float innerOffsetY)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->innerOffsetY = innerOffsetY;

    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
}

void drte_engine_get_inner_offset(drte_engine* pEngine, float* pInnerOffsetX, float* pInnerOffsetY)
{
    float innerOffsetX = 0;
    float innerOffsetY = 0;

    if (pEngine != NULL)
    {
        innerOffsetX = pEngine->innerOffsetX;
        innerOffsetY = pEngine->innerOffsetY;
    }


    if (pInnerOffsetX) {
        *pInnerOffsetX = innerOffsetX;
    }
    if (pInnerOffsetY) {
        *pInnerOffsetY = innerOffsetY;
    }
}

float drte_engine_get_inner_offset_x(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return pEngine->innerOffsetX;
}

float drte_engine_get_inner_offset_y(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return pEngine->innerOffsetY;
}


void drte_engine_set_tab_size(drte_engine* pEngine, unsigned int sizeInSpaces)
{
    if (pEngine == NULL) {
        return;
    }

    if (pEngine->tabSizeInSpaces != sizeInSpaces)
    {
        pEngine->tabSizeInSpaces = sizeInSpaces;

        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
    }
}

unsigned int drte_engine_get_tab_size(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return pEngine->tabSizeInSpaces;
}


void drte_engine_set_cursor_width(drte_engine* pEngine, float cursorWidth)
{
    if (pEngine == NULL) {
        return;
    }

    drte_engine__begin_dirty(pEngine);
    {
        for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
            drte_engine__on_dirty(pEngine, drte_engine_get_cursor_rect(pEngine, iCursor));
        }

        pEngine->cursorWidth = cursorWidth;
        if (pEngine->cursorWidth > 0 && pEngine->cursorWidth < 1) {
            pEngine->cursorWidth = 1;
        }

        for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
            drte_engine__on_dirty(pEngine, drte_engine_get_cursor_rect(pEngine, iCursor));
        }
    }
    drte_engine__end_dirty(pEngine);
}

float drte_engine_get_cursor_width(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return pEngine->cursorWidth;
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

void drte_engine_show_cursor(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    if (!pEngine->isShowingCursor)
    {
        pEngine->isShowingCursor = true;

        pEngine->timeToNextCursorBlink = pEngine->cursorBlinkRate;
        pEngine->isCursorBlinkOn = true;

        drte_engine__begin_dirty(pEngine);
        for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
            drte_engine__on_dirty(pEngine, drte_engine_get_cursor_rect(pEngine, iCursor));
        }
        drte_engine__end_dirty(pEngine);
    }
}

void drte_engine_hide_cursor(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    if (pEngine->isShowingCursor)
    {
        pEngine->isShowingCursor = false;

        drte_engine__begin_dirty(pEngine);
        for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
            drte_engine__on_dirty(pEngine, drte_engine_get_cursor_rect(pEngine, iCursor));
        }
        drte_engine__end_dirty(pEngine);
    }
}

bool drte_engine_is_showing_cursor(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    return pEngine->isShowingCursor;
}

size_t drte_engine_insert_cursor(drte_engine* pEngine, size_t iChar)
{
    if (pEngine == NULL) {
        return (size_t)-1;
    }

    drte_marker* pNewCursors = (drte_marker*)realloc(pEngine->pCursors, (pEngine->cursorCount+1) * sizeof(*pNewCursors));
    if (pNewCursors == NULL) {
        return (size_t)-1;
    }

    pEngine->pCursors = pNewCursors;
    pEngine->pCursors[pEngine->cursorCount].iCharAbs = 0;
    pEngine->pCursors[pEngine->cursorCount].absoluteSickyPosX = 0;
    pEngine->cursorCount += 1;

    drte_engine__begin_dirty(pEngine);
        drte_engine_move_cursor_to_character(pEngine, pEngine->cursorCount-1, iChar);
        drte_engine__repaint(pEngine);
    drte_engine__end_dirty(pEngine);

    return pEngine->cursorCount - 1;
}

void drte_engine_remove_cursor(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->cursorCount <= cursorIndex) {
        return;
    }

    for (size_t i = cursorIndex; i < pEngine->cursorCount-1; ++i) {
        pEngine->pCursors[i] = pEngine->pCursors[i+1];
    }

    pEngine->cursorCount -= 1;
    drte_engine__repaint(pEngine);
}

void drte_engine_remove_overlapping_cursors(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->cursorCount == 0) {
        return;
    }

    drte_engine__begin_dirty(pEngine);
    for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
        for (size_t jCursor = iCursor+1; jCursor < pEngine->cursorCount; ++jCursor) {
            if (pEngine->pCursors[iCursor].iCharAbs == pEngine->pCursors[jCursor].iCharAbs) {
                drte_engine_remove_cursor(pEngine, jCursor);
                break;
            }
        }
    }
    drte_engine__end_dirty(pEngine);
}

size_t drte_engine_get_last_cursor(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->cursorCount == 0) {
        return 0;
    }

    return pEngine->cursorCount - 1;
}

void drte_engine_get_cursor_position(drte_engine* pEngine, size_t cursorIndex, float* pPosXOut, float* pPosYOut)
{
    if (pEngine == NULL || pEngine->cursorCount <= cursorIndex) {
        return;
    }

    drte_engine__get_marker_position_relative_to_container(pEngine, &pEngine->pCursors[cursorIndex], pPosXOut, pPosYOut);
}

drgui_rect drte_engine_get_cursor_rect(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->cursorCount <= cursorIndex) {
        return drgui_make_rect(0, 0, 0, 0);
    }


    float cursorPosX;
    float cursorPosY;
    drte_engine_get_cursor_position(pEngine, cursorIndex, &cursorPosX, &cursorPosY);

    return drgui_make_rect(cursorPosX, cursorPosY, cursorPosX + pEngine->cursorWidth, cursorPosY + drte_engine_get_line_height(pEngine));
}

size_t drte_engine_get_cursor_line(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->cursorCount <= cursorIndex) {
        return 0;
    }

    return drte_engine_get_character_line(pEngine, pEngine->pCursors[cursorIndex].iCharAbs);
}

size_t drte_engine_get_cursor_column(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->cursorCount <= cursorIndex) {
        return 0;
    }

    float posX;
    float posY;
    drte_engine_get_cursor_position(pEngine, cursorIndex, &posX, &posY);

    return (unsigned int)((int)posX / pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.spaceWidth);
}

size_t drte_engine_get_cursor_character(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->cursorCount <= cursorIndex) {
        return 0;
    }

    return drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->pCursors[cursorIndex]);
}

void drte_engine_move_cursor_to_point(drte_engine* pEngine, size_t cursorIndex, float posX, float posY)
{
    if (pEngine == NULL || pEngine->cursorCount <= cursorIndex) {
        return;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    drte_engine__move_marker_to_point_relative_to_container(pEngine, &pEngine->pCursors[cursorIndex], posX, posY);

    if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
        drte_engine__begin_dirty(pEngine);
            drte_engine__on_cursor_move(pEngine, cursorIndex);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: This can be optimized. Only redraw the previous line and the new cursor rectangle.
        drte_engine__end_dirty(pEngine);
    }
}

bool drte_engine_move_cursor_left(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_left(pEngine, &pEngine->pCursors[cursorIndex])) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_right(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_right(pEngine, &pEngine->pCursors[cursorIndex])) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_up(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_up(pEngine, &pEngine->pCursors[cursorIndex])) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_down(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_down(pEngine, &pEngine->pCursors[cursorIndex])) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_y(drte_engine* pEngine, size_t cursorIndex, int amount)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_y(pEngine, &pEngine->pCursors[cursorIndex], amount)) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_end_of_line(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_to_end_of_line(pEngine, &pEngine->pCursors[cursorIndex])) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_start_of_line(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_to_start_of_line(pEngine, &pEngine->pCursors[cursorIndex])) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_end_of_line_by_index(drte_engine* pEngine, size_t cursorIndex, size_t iLine)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_to_end_of_line_by_index(pEngine, &pEngine->pCursors[cursorIndex], iLine)) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_start_of_line_by_index(drte_engine* pEngine, size_t cursorIndex, size_t iLine)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_to_start_of_line_by_index(pEngine, &pEngine->pCursors[cursorIndex], iLine)) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_end_of_text(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_to_end_of_text(pEngine, &pEngine->pCursors[cursorIndex])) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_start_of_text(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_to_start_of_text(pEngine, &pEngine->pCursors[cursorIndex])) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }

        return true;
    }

    return false;
}

void drte_engine_move_cursor_to_start_of_selection(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->selectionCount == 0 || pEngine->cursorCount <= cursorIndex) {
        return;
    }

    drte_engine_move_cursor_to_character(pEngine, cursorIndex, drte_region_normalize(pEngine->pSelections[pEngine->selectionCount-1]).iCharBeg);
}

void drte_engine_move_cursor_to_end_of_selection(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->selectionCount == 0 || pEngine->cursorCount <= cursorIndex) {
        return;
    }

    drte_engine_move_cursor_to_character(pEngine, cursorIndex, drte_region_normalize(pEngine->pSelections[pEngine->selectionCount-1]).iCharEnd);
}

void drte_engine_move_cursor_to_character(drte_engine* pEngine, size_t cursorIndex, size_t characterIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return;
    }

    size_t iPrevChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (drte_engine__move_marker_to_character(pEngine, &pEngine->pCursors[cursorIndex], characterIndex)) {
        if (iPrevChar != pEngine->pCursors[cursorIndex].iCharAbs) {
            drte_engine__begin_dirty(pEngine);
                drte_engine__on_cursor_move(pEngine, cursorIndex);
                drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
            drte_engine__end_dirty(pEngine);
        }
    }
}

size_t drte_engine_move_cursor_to_end_of_word(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return 0;
    }

    size_t iChar = drte_engine_get_cursor_character(pEngine, cursorIndex);
    if (!drte_is_symbol_or_whitespace(pEngine->text[iChar])) {
        while (pEngine->text[iChar] != '\0') {
            uint32_t c = pEngine->text[iChar];
            if (drte_is_symbol_or_whitespace(c)) {
                break;
            }

            iChar += 1;
        }
    } else {
        iChar += 1;
    }



    drte_engine_move_cursor_to_character(pEngine, cursorIndex, iChar);
    return iChar;
}

size_t drte_engine_move_cursor_to_start_of_next_word(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return 0;
    }

    size_t iChar = drte_engine_move_cursor_to_end_of_word(pEngine, cursorIndex);
    while (pEngine->text[iChar] != '\0') {
        uint32_t c = pEngine->text[iChar];
        if (!dr_is_whitespace(c)) {
            break;
        }

        iChar += 1;
    }

    drte_engine_move_cursor_to_character(pEngine, cursorIndex, iChar);
    return iChar;
}

size_t drte_engine_move_cursor_to_start_of_word(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return 0;
    }

    size_t iChar = drte_engine_get_cursor_character(pEngine, cursorIndex);
    if (iChar == 0) {
        return 0;
    }

    iChar -= 1;

    // Skip whitespace.
    if (dr_is_whitespace(pEngine->text[iChar])) {
        while (iChar > 0) {
            if (!dr_is_whitespace(pEngine->text[iChar])) {
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

    drte_engine_move_cursor_to_character(pEngine, cursorIndex, iChar);
    return iChar;
}

size_t drte_engine_get_spaces_to_next_column_from_character(drte_engine* pEngine, size_t iChar)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return 0;
    }

    if (iChar > pEngine->textLength) {
        iChar = pEngine->textLength;
    }


    const float tabWidth = drte_engine__get_tab_width(pEngine);

    float posX;
    float posY;
    drte_engine_get_character_position(pEngine, iChar, &posX, &posY);

    float tabColPosX = (posX + tabWidth) - ((size_t)posX % (size_t)tabWidth);

    return (size_t)(tabColPosX - posX) / pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.spaceWidth;
}

size_t drte_engine_get_spaces_to_next_column_from_cursor(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->cursorCount <= cursorIndex) {
        return 0;
    }

    return drte_engine_get_spaces_to_next_column_from_character(pEngine, drte_engine_get_cursor_character(pEngine, cursorIndex));
}

bool drte_engine_is_cursor_at_start_of_selection(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->selectionCount == 0 || pEngine->cursorCount <= cursorIndex) {
        return false;
    }
    
    drte_region region = drte_region_normalize(pEngine->pSelections[pEngine->selectionCount-1]);
    return pEngine->pCursors[cursorIndex].iCharAbs == region.iCharBeg;
}

bool drte_engine_is_cursor_at_end_of_selection(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->selectionCount == 0 || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    drte_region region = drte_region_normalize(pEngine->pSelections[pEngine->selectionCount-1]);
    return pEngine->pCursors[cursorIndex].iCharAbs == region.iCharEnd;
}

void drte_engine_set_on_cursor_move(drte_engine* pEngine, drte_engine_on_cursor_move_proc proc)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->onCursorMove = proc;
}

bool drte_engine_insert_character(drte_engine* pEngine, size_t insertIndex, uint32_t utf32)
{
    // Transform '\r' to '\n'. Should this be done at a higher level, but the application?
    if (utf32 == '\r') {
        utf32 = '\n';
    }

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
        return false;;
    }

    if (insertIndex > pEngine->textLength) {
        return false;
    }

    size_t newTextLength = strlen(text);
    if (newTextLength == 0) {
        return false;
    }

    // We need to get the index of the line that's being inserted so we can know how to update the internal line cache.
    size_t iLine = drte_engine_get_character_line(pEngine, insertIndex);


    // TODO: Add proper support for UTF-8.
    char* pOldText = pEngine->text;
    char* pNewText = (char*)malloc(pEngine->textLength + newTextLength + 1);   // +1 for the new character and +1 for the null terminator.

    if (insertIndex > 0) {
        memcpy(pNewText, pOldText, insertIndex);
    }


    // Replace \r\n with \n.
    size_t linesAddedCount = 0;
    {
        char* dst = pNewText + insertIndex;
        const char* src = text;
        size_t srcLen = newTextLength;
        while (*src != '\0' && srcLen > 0)
        {
            if (*src != '\r') {
                *dst++ = *src;
            }

            if (*src == '\n') {
                linesAddedCount += 1;
            }

            src++;
            srcLen -= 1;
        }

        newTextLength = dst - (pNewText + insertIndex);
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
        size_t newLineCount = pEngine->lineCount + linesAddedCount;

        if (newLineCount >= pEngine->lineBufferSize) {
            size_t newLineBufferSize = (pEngine->lineBufferSize == 0) ? DRTE_PAGE_LINE_COUNT : dr_round_up(newLineCount, DRTE_PAGE_LINE_COUNT);
            size_t* pNewLines = (size_t*)realloc(pEngine->pLines, newLineBufferSize * sizeof(*pNewLines));
            if (pNewLines == NULL) {
                return false;   // Ran out of memory?
            }

            pEngine->lineBufferSize = newLineBufferSize;
            pEngine->pLines = pNewLines;
        }

        // All existing lines coming after the line the text was inserted at need to be moved down newLineCount slots. They also need to have their
        // first character index updated.
        for (size_t i = pEngine->lineCount; i > iLine; --i) {
            size_t iSrc = i-1;
            size_t iDst = iSrc + linesAddedCount;
            pEngine->pLines[iDst] = pEngine->pLines[iSrc] + newTextLength;
        }

        // The newly inserted lines need to be initialized.
        size_t iRunningChar;
        for (size_t i = iLine+1; i <= iLine + linesAddedCount; ++i) {
            iRunningChar = pEngine->pLines[i-1];
            while (pEngine->text[iRunningChar] != '\0') {
                if (pEngine->text[iRunningChar] == '\n') {
                    iRunningChar += 1;
                    break;
                }

                iRunningChar += 1;
            }

            pEngine->pLines[i] = iRunningChar;
        }

        pEngine->lineCount = newLineCount;
    } else {
        // No new lines were added, but we still need to update the character positions of the line cache.
        for (size_t i = iLine+1; i < pEngine->lineCount; ++i) {
            pEngine->pLines[i] += newTextLength;
        }
    }
    

    if (pEngine->onTextChanged) {
        pEngine->onTextChanged(pEngine);
    }

    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));

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
    size_t iLine = drte_engine_get_character_line(pEngine, iFirstCh);

    size_t linesRemovedCount = 0;
    for (size_t iChar = iFirstCh; iChar < iLastChPlus1; ++iChar) {
        if (pEngine->text[iChar] == '\n') {
            linesRemovedCount += 1;
        }
    }


    size_t bytesToRemove = iLastChPlus1 - iFirstCh;
    if (bytesToRemove > 0)
    {
        memmove(pEngine->text + iFirstCh, pEngine->text + iLastChPlus1, pEngine->textLength - iLastChPlus1);
        pEngine->textLength -= bytesToRemove;
        pEngine->text[pEngine->textLength] = '\0';

        if (linesRemovedCount > 0) {
            if (pEngine->lineCount <= linesRemovedCount) {
                pEngine->lineCount = 1;
            } else {
                for (size_t i = iLine+1; i < (pEngine->lineCount - linesRemovedCount); ++i) {
                    size_t iDst = i;
                    size_t iSrc = i + linesRemovedCount;
                    pEngine->pLines[iDst] = pEngine->pLines[iSrc] - bytesToRemove;
                }

                pEngine->lineCount -= linesRemovedCount;
            }
        } else {
            // No lines were removed, but we still need to update the character positions of the line cache.
            for (size_t i = iLine+1; i < pEngine->lineCount; ++i) {
                pEngine->pLines[i] -= bytesToRemove;
            }
        }

        if (pEngine->onTextChanged) {
            pEngine->onTextChanged(pEngine);
        }

        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));

        return true;
    }

    return false;
}

bool drte_engine_insert_character_at_cursor(drte_engine* pEngine, size_t cursorIndex, unsigned int character)
{
    if (pEngine == NULL) {
        return false;
    }

    drte_engine__begin_dirty(pEngine);
    {
        drte_engine_insert_character(pEngine, pEngine->pCursors[cursorIndex].iCharAbs, character);
        drte_engine__move_marker_to_character(pEngine, &pEngine->pCursors[cursorIndex], pEngine->pCursors[cursorIndex].iCharAbs + 1);
    }
    drte_engine__end_dirty(pEngine);


    // The cursor's sticky position needs to be updated whenever the text is edited.
    drte_engine__update_marker_sticky_position(pEngine, &pEngine->pCursors[cursorIndex]);


    drte_engine__on_cursor_move(pEngine, cursorIndex);

    return true;
}

bool drte_engine_insert_character_at_cursors(drte_engine* pEngine, unsigned int character)
{
    if (pEngine == NULL) {
        return false;
    }

    // TODO: This can be improved because it is posting multiple onTextChanged messages.

    bool wasTextChanged = false;
    drte_engine__begin_dirty(pEngine);
    {
        for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
            size_t iCursorChar = pEngine->pCursors[iCursor].iCharAbs;
            if (!drte_engine_insert_character_at_cursor(pEngine, iCursor, character)) {
                continue;
            }
            
            // Any cursor whose character position comes after this cursor needs to be moved.
            for (size_t iCursor2 = 0; iCursor2 < pEngine->cursorCount; ++iCursor2) {
                if (iCursor2 != iCursor) {
                    if (pEngine->pCursors[iCursor2].iCharAbs > iCursorChar) {
                        wasTextChanged = drte_engine__move_marker_to_character(pEngine, &pEngine->pCursors[iCursor2], pEngine->pCursors[iCursor2].iCharAbs + 1) || wasTextChanged;
                    }
                }
            }
        }
    }
    drte_engine__end_dirty(pEngine);

    return wasTextChanged;
}

bool drte_engine_insert_text_at_cursor(drte_engine* pEngine, size_t cursorIndex, const char* text)
{
    if (pEngine == NULL || text == NULL) {
        return false;
    }

    drte_engine__begin_dirty(pEngine);
    {
        size_t cursorPos = drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->pCursors[cursorIndex]);
        drte_engine_insert_text(pEngine, text, cursorPos);
        drte_engine__move_marker_to_character(pEngine, &pEngine->pCursors[cursorIndex], cursorPos + strlen(text));
    }
    drte_engine__end_dirty(pEngine);


    // The cursor's sticky position needs to be updated whenever the text is edited.
    drte_engine__update_marker_sticky_position(pEngine, &pEngine->pCursors[cursorIndex]);

    drte_engine__on_cursor_move(pEngine, cursorIndex);

    return true;
}

bool drte_engine_delete_character_to_left_of_cursor(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL) {
        return false;
    }

    // We just move the cursor to the left, and then delete the character to the right.
    if (drte_engine_move_cursor_left(pEngine, cursorIndex)) {
        drte_engine_delete_character_to_right_of_cursor(pEngine, cursorIndex);
        return true;
    }

    return false;
}

bool drte_engine_delete_character_to_left_of_cursors(drte_engine* pEngine, bool leaveNewLines)
{
    if (pEngine == NULL) {
        return false;
    }

    bool wasTextChanged = false;
    drte_engine__begin_dirty(pEngine);
    {
        for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
            size_t iCursorChar = pEngine->pCursors[iCursor].iCharAbs;
            if (iCursorChar == 0 || (leaveNewLines && pEngine->text[iCursorChar-1] == '\n') || !drte_engine_delete_character_to_left_of_cursor(pEngine, iCursor)) {
                continue;
            }

            wasTextChanged = true;

            for (size_t iCursor2 = 0; iCursor2 < pEngine->cursorCount; ++iCursor2) {
                if (iCursor2 != iCursor) {
                    if (pEngine->pCursors[iCursor2].iCharAbs > iCursorChar && pEngine->pCursors[iCursor2].iCharAbs > 0) {
                        wasTextChanged = drte_engine__move_marker_to_character(pEngine, &pEngine->pCursors[iCursor2], pEngine->pCursors[iCursor2].iCharAbs - 1) || wasTextChanged;
                    }
                }
            }
        }
    }
    drte_engine__end_dirty(pEngine);

    return wasTextChanged;
}

bool drte_engine_delete_character_to_right_of_cursor(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL) {
        return false;
    }

    size_t iAbsoluteMarkerChar = pEngine->pCursors[cursorIndex].iCharAbs;
    if (iAbsoluteMarkerChar < pEngine->textLength)
    {
        if (!drte_engine_delete_text(pEngine, iAbsoluteMarkerChar, iAbsoluteMarkerChar + 1)) {
            return false;
        }


        // The layout will have changed.
        drte_engine__move_marker_to_character(pEngine, &pEngine->pCursors[cursorIndex], iAbsoluteMarkerChar);

        if (pEngine->onTextChanged) {
            pEngine->onTextChanged(pEngine);
        }

        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));

        return true;
    }

    return false;
}

bool drte_engine_delete_character_to_right_of_cursors(drte_engine* pEngine, bool leaveNewLines)
{
    if (pEngine == NULL) {
        return false;
    }

    bool wasTextChanged = false;
    drte_engine__begin_dirty(pEngine);
    {
        for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
            size_t iCursorChar = pEngine->pCursors[iCursor].iCharAbs;
            if ((leaveNewLines && pEngine->text[pEngine->pCursors[iCursor].iCharAbs] == '\n') || !drte_engine_delete_character_to_right_of_cursor(pEngine, iCursor)) {
                continue;
            }

            wasTextChanged = true;

            for (size_t iCursor2 = 0; iCursor2 < pEngine->cursorCount; ++iCursor2) {
                if (iCursor2 != iCursor) {
                    if (pEngine->pCursors[iCursor2].iCharAbs > iCursorChar && pEngine->pCursors[iCursor2].iCharAbs > 0) {
                        wasTextChanged = drte_engine__move_marker_to_character(pEngine, &pEngine->pCursors[iCursor2], pEngine->pCursors[iCursor2].iCharAbs - 1) || wasTextChanged;
                    }
                }
            }
        }
    }
    drte_engine__end_dirty(pEngine);

    return wasTextChanged;
}


int drte_region_qsort(const void* pSelection0, const void* pSelection1)
{
    drte_region selection0 = *(const drte_region*)pSelection0;
    drte_region selection1 = *(const drte_region*)pSelection1;

    if (selection0.iCharBeg < selection1.iCharBeg) {
        return -1;
    }
    if (selection0.iCharBeg > selection1.iCharBeg) {
        return +1;
    }

    return 0;
}

bool drte_engine_delete_selected_text(drte_engine* pEngine, bool updateCursors)
{
    if (pEngine == NULL || pEngine->selectionCount == 0) {
        return false;
    }

    // To delete selected text we need to ensure there are no overlaps. Selections are then deleted back to front.
    drte_region* pSortedSelections = (drte_region*)malloc(pEngine->selectionCount * sizeof(*pSortedSelections));
    if (pSortedSelections == NULL) {
        return false;
    }

    for (size_t iSelection = 0; iSelection < pEngine->selectionCount; ++iSelection) {
        pSortedSelections[iSelection] = pEngine->pSelections[iSelection];
    }

    for (size_t iSelection = 0; iSelection < pEngine->selectionCount; ++iSelection) {
        drte_region selection = drte_region_normalize(pSortedSelections[iSelection]);
        for (size_t iSelection2 = iSelection+1; iSelection2 < pEngine->selectionCount; ++iSelection2) {
            if (iSelection != iSelection2) {
                drte_region selection2 = drte_region_normalize(pSortedSelections[iSelection2]);
                if (selection.iCharBeg < selection2.iCharBeg) {
                    if (selection2.iCharEnd < selection.iCharEnd) {
                        selection2.iCharEnd = selection.iCharEnd;
                        pSortedSelections[iSelection2] = selection2;
                    }
                    if (selection.iCharEnd > selection2.iCharBeg) {
                        selection.iCharEnd = selection2.iCharBeg;
                    }
                } else {
                    if (selection.iCharBeg < selection2.iCharEnd) {
                        selection.iCharBeg = selection2.iCharEnd;
                        if (selection.iCharEnd < selection.iCharBeg) {
                            selection.iCharEnd = selection.iCharBeg;
                        }
                    }
                }
            }
        }

        pSortedSelections[iSelection] = selection;
    }

    qsort(pSortedSelections, pEngine->selectionCount, sizeof(*pSortedSelections), drte_region_qsort);

    drte_engine__begin_dirty(pEngine);
    bool wasTextChanged = false;
    for (size_t iSelection = pEngine->selectionCount; iSelection > 0; --iSelection) {
        drte_region selection = pSortedSelections[iSelection-1];
        if (selection.iCharBeg < selection.iCharEnd) {
            wasTextChanged = drte_engine_delete_text(pEngine, selection.iCharBeg, selection.iCharEnd) || wasTextChanged;
            if (wasTextChanged) {
                if (updateCursors) {
                    for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
                        size_t iCursorChar = pEngine->pCursors[iCursor].iCharAbs;
                        if (iCursorChar > selection.iCharBeg) {
                            if (iCursorChar > selection.iCharEnd) {
                                iCursorChar -= (selection.iCharEnd - selection.iCharBeg);
                            } else {
                                iCursorChar -= (iCursorChar - selection.iCharBeg);
                            }

                            drte_engine_move_cursor_to_character(pEngine, iCursor, iCursorChar);
                        }
                    }
                }
            }
        }
    }
    drte_engine__end_dirty(pEngine);

    free(pSortedSelections);
    return wasTextChanged;
}

bool drte_engine_delete_selection_text(drte_engine* pEngine, size_t iSelectionToDelete, bool updateCursorsAndSelection)
{
    if (pEngine == NULL || pEngine->selectionCount == 0) {
        return false;
    }

    drte_region selectionToDelete = drte_region_normalize(pEngine->pSelections[iSelectionToDelete]);
    if (selectionToDelete.iCharBeg == selectionToDelete.iCharEnd) {
        return false;   // Nothing is selected.
    }

    bool wasTextChanged = false;
    drte_engine__begin_dirty(pEngine);
    {
        wasTextChanged = drte_engine_delete_text(pEngine, selectionToDelete.iCharBeg, selectionToDelete.iCharEnd) || wasTextChanged;

        if (updateCursorsAndSelection) {
            for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
                size_t iCursorChar = pEngine->pCursors[iCursor].iCharAbs;
                if (iCursorChar > selectionToDelete.iCharBeg && iCursorChar < selectionToDelete.iCharEnd) {
                    drte_engine_move_cursor_to_character(pEngine, iCursor, selectionToDelete.iCharBeg);
                } else {
                    if (iCursorChar >= selectionToDelete.iCharEnd) {
                        drte_engine_move_cursor_to_character(pEngine, iCursor, iCursorChar - (selectionToDelete.iCharEnd - selectionToDelete.iCharBeg));
                    }
                }
            }

            // <---> = selection
            // |---| = selectionToDelete
            for (size_t iSelection = 0; iSelection < pEngine->selectionCount; ++iSelection) {
                drte_region selection = drte_region_normalize(pEngine->pSelections[iSelection]);
                if (selection.iCharBeg < selectionToDelete.iCharBeg) {
                    if (selection.iCharEnd > selectionToDelete.iCharBeg) {
                        if (selection.iCharEnd < selectionToDelete.iCharEnd) {
                            // <---|--->---|
                            selection.iCharEnd = selectionToDelete.iCharBeg;
                        } else {
                            // <---|---|--->
                            selection.iCharEnd -= selectionToDelete.iCharEnd - selectionToDelete.iCharBeg;
                        }
                    }
                } else {
                    if (selection.iCharBeg < selectionToDelete.iCharEnd) {
                        if (selection.iCharEnd < selectionToDelete.iCharEnd) {
                            // |---<--->---|
                            selection.iCharBeg = selection.iCharEnd = selectionToDelete.iCharBeg;
                        } else {
                            // |---<---|--->
                            selection.iCharBeg = selectionToDelete.iCharBeg;
                        }
                    } else {
                        selection.iCharBeg -= (selectionToDelete.iCharEnd - selectionToDelete.iCharBeg);
                        selection.iCharEnd -= (selectionToDelete.iCharEnd - selectionToDelete.iCharBeg);
                    }
                }

                if (pEngine->pSelections[iSelection].iCharBeg < pEngine->pSelections[iSelection].iCharEnd) {
                    pEngine->pSelections[iSelection].iCharBeg = selection.iCharBeg;
                    pEngine->pSelections[iSelection].iCharEnd = selection.iCharEnd;
                } else {
                    pEngine->pSelections[iSelection].iCharBeg = selection.iCharEnd;
                    pEngine->pSelections[iSelection].iCharEnd = selection.iCharBeg;
                }
            }
        }
    }
    drte_engine__end_dirty(pEngine);

    return wasTextChanged;
}


bool drte_engine_is_anything_selected(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    return pEngine->selectionCount > 0;
}

void drte_engine_deselect_all(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->selectionCount = 0;

    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
}

void drte_engine_select_all(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    // Deselect everything first to ensure any multi-select stuff is cleared.
    drte_engine_deselect_all(pEngine);

    drte_engine_select(pEngine, 0, pEngine->textLength);
}

void drte_engine_select(drte_engine* pEngine, size_t firstCharacter, size_t lastCharacter)
{
    if (pEngine == NULL) {
        return;
    }

    drte_engine_begin_selection(pEngine, firstCharacter);
    drte_engine_set_selection_end_point(pEngine, lastCharacter);

    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
}


bool drte_engine_get_start_of_word_containing_character(drte_engine* pEngine, size_t iChar, size_t* pWordBegOut)
{
    if (pEngine == NULL) {
        return false;
    }

    if (iChar > 0) {
        iChar -= 1;

        // Skip whitespace.
        if (dr_is_whitespace(pEngine->text[iChar])) {
            while (iChar > 0) {
                if (!dr_is_whitespace(pEngine->text[iChar])) {
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

bool drte_engine_get_end_of_word_containing_character(drte_engine* pEngine, size_t iChar, size_t* pWordEndOut)
{
    if (pEngine == NULL) {
        return false;
    }

    if (!drte_is_symbol_or_whitespace(pEngine->text[iChar])) {
        while (pEngine->text[iChar] != '\0') {
            uint32_t c = pEngine->text[iChar];
            if (drte_is_symbol_or_whitespace(c)) {
                break;
            }

            iChar += 1;
        }
    } else {
        iChar += 1;
    }

    if (pWordEndOut) *pWordEndOut = iChar;
    return true;
}

bool drte_engine_get_start_of_next_word_from_character(drte_engine* pEngine, size_t iChar, size_t* pWordBegOut)
{
    if (pEngine == NULL) {
        return false;
    }

    while (pEngine->text[iChar] != '\0') {
        uint32_t c = pEngine->text[iChar];
        if (!dr_is_whitespace(c)) {
            break;
        }

        iChar += 1;
    }

    if (pWordBegOut) *pWordBegOut = iChar;
    return true;
}

// TODO: Make this public.
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

        if (!dr_is_whitespace(c) && !dr_is_whitespace(cprev)) {
            drte_engine_get_start_of_word_containing_character(pEngine, iChar, &iChar);
        } else if (dr_is_whitespace(c) && dr_is_whitespace(cprev)) {
            iChar -= 1;
            while (iChar > 0) {
                if (!dr_is_whitespace(pEngine->text[iChar-1]) || pEngine->text[iChar-1] == '\n') {
                    break;
                }
                iChar -= 1;
            }

            moveToStartOfNextWord = true;
        }
    }

    if (pWordBegOut) *pWordBegOut = iChar;

    if (moveToStartOfNextWord) {
        drte_engine_get_start_of_next_word_from_character(pEngine, iChar, pWordEndOut);
    } else {
        drte_engine_get_end_of_word_containing_character(pEngine, iChar, pWordEndOut);
    }

    return true;
}

// TODO: Make this public.
bool drte_engine_get_word_under_cursor(drte_engine* pEngine, size_t cursorIndex, size_t* pWordBegOut, size_t* pWordEndOut)
{
    if (pEngine == NULL || pEngine->cursorCount <= cursorIndex) {
        return false;
    }

    return drte_engine_get_word_containing_character(pEngine, pEngine->pCursors[cursorIndex].iCharAbs, pWordBegOut, pWordEndOut);
}

void drte_engine_select_word_under_cursor(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL || pEngine->cursorCount <= cursorIndex) {
        return;
    }

    size_t iWordBeg;
    size_t iWordEnd;
    if (!drte_engine_get_word_under_cursor(pEngine, cursorIndex, &iWordBeg, &iWordEnd)) {
        return;
    }

    drte_engine_select(pEngine, iWordBeg, iWordEnd);

#if 0
    bool moveToStartOfNextWord = false;

    // Move to the start of the word if we're not already there.
    size_t iChar = drte_engine_get_cursor_character(pEngine);
    if (iChar > 0) {
        uint32_t c = pEngine->text[iChar];
        uint32_t cprev = pEngine->text[iChar-1];

        if (!dr_is_whitespace(c) && !dr_is_whitespace(cprev)) {
            drte_engine_move_cursor_to_start_of_word(pEngine);
        } else if (dr_is_whitespace(c) && dr_is_whitespace(cprev)) {
            iChar -= 1;
            while (iChar > 0) {
                if (!dr_is_whitespace(pEngine->text[iChar-1]) || pEngine->text[iChar-1] == '\n') {
                    break;
                }
                iChar -= 1;
            }

            drte_engine_move_cursor_to_character(pEngine, iChar);
            moveToStartOfNextWord = true;
        }
    }


    drte_engine_enter_selection_mode(pEngine);
    if (moveToStartOfNextWord) {
        drte_engine_move_cursor_to_start_of_next_word(pEngine);
    } else {
        drte_engine_move_cursor_to_end_of_word(pEngine);
    }
    drte_engine_leave_selection_mode(pEngine);
#endif
}

size_t drte_engine_get_selected_text(drte_engine* pEngine, char* textOut, size_t textOutSize)
{
    // Safety.
    if (textOut != NULL && textOutSize > 0) {
        textOut[0] = '\0';
    }

    if (pEngine == NULL || (textOut != NULL && textOutSize == 0)) {
        return 0;
    }

    if (!drte_engine_is_anything_selected(pEngine)) {
        return 0;
    }


    // The selected text is just every selection concatenated together.
    size_t length = 0;
    for (size_t iSelection = 0; iSelection < pEngine->selectionCount; ++iSelection) {
        drte_region region = drte_region_normalize(pEngine->pSelections[iSelection]);
        if (textOut != NULL) {
            drgui__strncpy_s(textOut+length, textOutSize-length, pEngine->text+region.iCharBeg, (region.iCharEnd - region.iCharBeg));
        }
        
        length += (region.iCharEnd - region.iCharBeg);
    }

    return length;
}

size_t drte_engine_get_selection_first_line(drte_engine* pEngine, size_t iSelection)
{
    if (pEngine == NULL || pEngine->selectionCount == 0) {
        return 0;
    }

    return drte_engine_get_character_line(pEngine, drte_region_normalize(pEngine->pSelections[iSelection]).iCharBeg);
}

size_t drte_engine_get_selection_last_line(drte_engine* pEngine, size_t iSelection)
{
    if (pEngine == NULL) {
        return 0;
    }

    return drte_engine_get_character_line(pEngine, drte_region_normalize(pEngine->pSelections[iSelection]).iCharEnd);
}

void drte_engine_move_selection_anchor_to_end_of_line(drte_engine* pEngine, size_t iLine)
{
    drte_engine_set_selection_anchor(pEngine, drte_engine_get_line_last_character(pEngine, iLine));
}

void drte_engine_move_selection_anchor_to_start_of_line(drte_engine* pEngine, size_t iLine)
{
    drte_engine_set_selection_anchor(pEngine, drte_engine_get_line_first_character(pEngine, iLine));
}

size_t drte_engine_get_selection_anchor_line(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->selectionCount == 0) {
        return 0;
    }

    return drte_engine_get_character_line(pEngine, pEngine->pSelections[pEngine->selectionCount-1].iCharBeg);
}


void drte_engine_begin_selection(drte_engine* pEngine, size_t iCharBeg)
{
    if (pEngine == NULL) {
        return;
    }

    drte_region* pNewSelections = (drte_region*)realloc(pEngine->pSelections, (pEngine->selectionCount + 1) * sizeof(*pNewSelections));
    if (pNewSelections == NULL) {
        return;
    }

    pEngine->pSelections = pNewSelections;
    pEngine->pSelections[pEngine->selectionCount].iCharBeg = iCharBeg;
    pEngine->pSelections[pEngine->selectionCount].iCharEnd = iCharBeg;
    pEngine->selectionCount += 1;
}

void drte_engine_cancel_selection(drte_engine* pEngine, size_t iSelection)
{
    if (pEngine == NULL || pEngine->selectionCount == 0) {
        return;
    }

    for (/* Do Nothing */; iSelection < pEngine->selectionCount-1; ++iSelection) {
        pEngine->pSelections[iSelection] = pEngine->pSelections[iSelection+1];
    }

    pEngine->selectionCount -= 1;
}

void drte_engine_cancel_last_selection(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->selectionCount == 0) {
        return;
    }

    pEngine->selectionCount -= 1;
}

void drte_engine_set_selection_anchor(drte_engine* pEngine, size_t iCharBeg)
{
    if (pEngine == NULL || pEngine->selectionCount == 0) {
        return;
    }

    if (pEngine->pSelections[pEngine->selectionCount-1].iCharBeg != iCharBeg) {
        pEngine->pSelections[pEngine->selectionCount-1].iCharBeg = iCharBeg;
        drte_engine__repaint(pEngine);
    }
}

void drte_engine_set_selection_end_point(drte_engine* pEngine, size_t iCharEnd)
{
    if (pEngine == NULL || pEngine->selectionCount == 0) {
        return;
    }

    if (pEngine->pSelections[pEngine->selectionCount-1].iCharEnd != iCharEnd) {
        pEngine->pSelections[pEngine->selectionCount-1].iCharEnd = iCharEnd;
        drte_engine__repaint(pEngine);
    }
}

bool drte_engine_get_last_selection(drte_engine* pEngine, size_t* iCharBegOut, size_t* iCharEndOut)
{
    if (pEngine == NULL || pEngine->selectionCount == 0) {
        return false;
    }

    drte_region selection = drte_region_normalize(pEngine->pSelections[pEngine->selectionCount-1]);

    if (iCharBegOut) *iCharBegOut = selection.iCharBeg;
    if (iCharEndOut) *iCharEndOut = selection.iCharEnd;
    return true;
}



bool drte_engine_prepare_undo_point(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    // If we have a previously prepared state we'll need to clear it.
    if (pEngine->preparedState.text != NULL) {
        free(pEngine->preparedState.text);
        free(pEngine->preparedState.pCursors);
        free(pEngine->preparedState.pSelections);
        free(pEngine->preparedState.pUserData);
    }

    pEngine->preparedState.text = (char*)malloc(pEngine->textLength + 1);
    drgui__strcpy_s(pEngine->preparedState.text, pEngine->textLength + 1, (pEngine->text != NULL) ? pEngine->text : "");

    pEngine->preparedState.cursorCount = pEngine->cursorCount;
    pEngine->preparedState.pCursors = (drte_marker*)malloc(pEngine->cursorCount * sizeof(drte_marker));
    memcpy(pEngine->preparedState.pCursors, pEngine->pCursors, pEngine->cursorCount * sizeof(*pEngine->preparedState.pCursors));

    pEngine->preparedState.selectionCount = pEngine->selectionCount;
    pEngine->preparedState.pSelections = (drte_region*)malloc(pEngine->selectionCount * sizeof(drte_region));
    memcpy(pEngine->preparedState.pSelections, pEngine->pSelections, pEngine->selectionCount * sizeof(*pEngine->preparedState.pSelections));

    pEngine->preparedState.userDataSize = 0;
    pEngine->preparedState.pUserData = NULL;
    if (pEngine->onGetUndoState) {
        pEngine->preparedState.userDataSize = pEngine->onGetUndoState(pEngine, NULL);
        if (pEngine->preparedState.userDataSize > 0) {
            pEngine->preparedState.pUserData = malloc(pEngine->preparedState.userDataSize);
            if (pEngine->preparedState.pUserData != NULL) {
                pEngine->onGetUndoState(pEngine, pEngine->preparedState.pUserData);
            }
        }
    }

    return true;
}

bool drte_engine_commit_undo_point(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    // The undo point must have been prepared earlier.
    if (pEngine->preparedState.text == NULL) {
        return false;
    }


    // The undo state is creating by diff-ing the prepared state and the current state.
    drte_engine_state currentState;
    currentState.text = pEngine->text;
    currentState.pCursors = pEngine->pCursors;
    currentState.cursorCount = pEngine->cursorCount;
    currentState.pSelections = pEngine->pSelections;
    currentState.selectionCount = pEngine->selectionCount;
    currentState.userDataSize = 0;
    currentState.pUserData = NULL;

    drte_engine_undo_state undoState;
    if (!drte_engine__diff_states(&pEngine->preparedState, &currentState, &undoState)) {
        return false;
    }

    undoState.newState.userDataSize = 0;
    undoState.newState.pUserData = NULL;
    if (pEngine->onGetUndoState) {
        undoState.newState.userDataSize = pEngine->onGetUndoState(pEngine, NULL);
        if (undoState.newState.userDataSize > 0) {
            undoState.newState.pUserData = malloc(undoState.newState.userDataSize);
            if (undoState.newState.pUserData != NULL) {
                pEngine->onGetUndoState(pEngine, undoState.newState.pUserData);
            }
        }
    }


    // At this point we have the undo state ready and we just need to add it the undo stack. Before doing so, however,
    // we need to trim the end fo the stack.
    drte_engine__trim_undo_stack(pEngine);
    drte_engine__push_undo_state(pEngine, &undoState);

    return true;
}

bool drte_engine_undo(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->pUndoStack == NULL) {
        return false;
    }

    if (drte_engine_get_undo_points_remaining_count(pEngine) > 0)
    {
        drte_engine_undo_state* pUndoState = pEngine->pUndoStack + (pEngine->iUndoState - 1);
        assert(pUndoState != NULL);

        drte_engine__apply_undo_state(pEngine, pUndoState);
        pEngine->iUndoState -= 1;

        if (pEngine->onUndoPointChanged) {
            pEngine->onUndoPointChanged(pEngine, pEngine->iUndoState);
        }

        return true;
    }

    return false;
}

bool drte_engine_redo(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->pUndoStack == NULL) {
        return false;
    }

    if (drte_engine_get_redo_points_remaining_count(pEngine) > 0)
    {
        drte_engine_undo_state* pUndoState = pEngine->pUndoStack + pEngine->iUndoState;
        assert(pUndoState != NULL);

        drte_engine__apply_redo_state(pEngine, pUndoState);
        pEngine->iUndoState += 1;

        if (pEngine->onUndoPointChanged) {
            pEngine->onUndoPointChanged(pEngine, pEngine->iUndoState);
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
    if (pEngine == NULL || pEngine->pUndoStack == NULL) {
        return;
    }

    for (unsigned int i = 0; i < pEngine->undoStackCount; ++i) {
        drte_engine__uninit_undo_state(pEngine->pUndoStack + i);
    }

    free(pEngine->pUndoStack);

    pEngine->pUndoStack = NULL;
    pEngine->undoStackCount = 0;

    if (pEngine->iUndoState > 0) {
        pEngine->iUndoState = 0;

        if (pEngine->onUndoPointChanged) {
            pEngine->onUndoPointChanged(pEngine, pEngine->iUndoState);
        }
    }
}



size_t drte_engine_get_line_count(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL || pEngine->textLength == 0) {
        return 0;
    }

    return pEngine->lineCount;
}

size_t drte_engine_get_visible_line_count(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return (size_t)(pEngine->containerHeight / drte_engine_get_line_height(pEngine)) + 1;
}

float drte_engine_get_visible_line_width(drte_engine* pEngine)
{
    size_t iLineTop;
    size_t iLineBottom;
    drte_engine_get_visible_lines(pEngine, &iLineTop, &iLineBottom);

    float maxLineWidth = 0;

    drte_segment segment;
    if (drte_engine__first_segment_on_line(pEngine, iLineTop, &segment)) {
        size_t iLine = iLineTop;
        while (iLine <= iLineBottom) {
            float lineWidth = 0;

            do
            {
                lineWidth += segment.width;

                if (pEngine->text[segment.iCharBeg] == '\n') {
                    break;
                }
            } while (drte_engine__next_segment(pEngine, &segment));


            if (maxLineWidth < lineWidth) {
                maxLineWidth = lineWidth;
            }
            
            // Go to the first segment of the next line.
            if (!drte_engine__next_segment(pEngine, &segment)) {
                break;
            }

            iLine += 1;
        }
    }

    return maxLineWidth;
}

float drte_engine_get_line_pos_y(drte_engine* pEngine, size_t iLine)
{
    return iLine * drte_engine_get_line_height(pEngine);
}

size_t drte_engine_get_line_at_pos_y(drte_engine* pEngine, float posY)
{
    if (pEngine == NULL) {
        return 0;
    }

    size_t lineCount = drte_engine_get_line_count(pEngine);
    if (lineCount == 0) {
        return false;
    }

    intptr_t iLine = (intptr_t)(posY / drte_engine_get_line_height(pEngine));
    if (iLine < 0) {
        iLine = 0;
    }
    if ((size_t)iLine >= lineCount) {
        iLine = lineCount-1;
    }

    return iLine;
}

size_t drte_engine_get_line_first_character(drte_engine* pEngine, size_t iLine)
{
    if (pEngine == NULL || iLine == 0) {
        return 0;
    }

    return pEngine->pLines[iLine];

#if 0
    // Brute force
    size_t i = 0;
    while (i < pEngine->textLength && iLine > 0) {
        if (pEngine->text[i] == '\n') {
            iLine -= 1;
            if (iLine == 0) {
                return i + 1;
            }
        }

        i += 1;
    }

    return 0;
#endif
}

size_t drte_engine_get_line_last_character(drte_engine* pEngine, size_t iLine)
{
    if (pEngine == NULL) {
        return 0;
    }

    size_t i = drte_engine_get_line_first_character(pEngine, iLine);
    while (i < pEngine->textLength) {
        if (pEngine->text[i] == '\n') {
            return i;
        }

        i += 1;
    }

    return i;
}

void drte_engine_get_line_character_range(drte_engine* pEngine, size_t iLine, size_t* pCharStartOut, size_t* pCharEndOut)
{
    if (pEngine == NULL) {
        return;
    }

    size_t charStart = drte_engine_get_line_first_character(pEngine, iLine);

    size_t charEnd = charStart;
    while (charEnd < pEngine->textLength) {
        if (pEngine->text[charEnd] == '\n') {
            break;
        }

        charEnd += 1;
    }

    if (pCharStartOut) *pCharStartOut = charStart;
    if (pCharEndOut) *pCharEndOut = charEnd;
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

void drte_engine_paint(drte_engine* pEngine, drgui_rect rect, drgui_element* pElement, void* pPaintData)
{
    if (pEngine == NULL || pEngine->onPaintText == NULL || pEngine->onPaintRect == NULL) {
        return;
    }

    if (rect.left < 0) {
        rect.left = 0;
    }
    if (rect.top < 0) {
        rect.top = 0;
    }
    if (rect.right > pEngine->containerWidth) {
        rect.right = pEngine->containerWidth;
    }
    if (rect.bottom > pEngine->containerHeight) {
        rect.bottom = pEngine->containerHeight;
    }

    if (rect.right <= rect.left || rect.bottom <= rect.top) {
        return;
    }

    float lineHeight = drte_engine_get_line_height(pEngine);


    size_t iLineTop;
    size_t iLineBottom;
    drte_engine_get_visible_lines(pEngine, &iLineTop, &iLineBottom);

    float linePosX = pEngine->innerOffsetX;
    float linePosY = 0;

    drte_segment segment;
    if (drte_engine__first_segment_on_line(pEngine, iLineTop, &segment)) {
        size_t iLine = iLineTop;
        while (iLine <= iLineBottom) {
            float lineWidth = 0;

            do
            {
                if (segment.posX > pEngine->containerWidth) {
                    break;  // All remaining segments on this line (including this one) is clipped. Go to the next line.
                }

                lineWidth += segment.width;

                // Don't draw segments to the left of the container.
                if (linePosX + segment.posX + segment.width < 0) {
                    if (pEngine->text[segment.iCharBeg] == '\n') {
                        break;
                    }
                    continue;
                }

                uint32_t c = drte_engine_get_utf32(pEngine, segment.iCharBeg);
                if (c == '\t' || c == '\n') {
                    // It's whitespace.
                    if (c == '\n') {
                        // TODO: Only do this if the character is selected.
                        segment.width = pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.spaceWidth;
                        lineWidth += segment.width;
                    }

                    drte_style_token bgStyleToken = drte_engine__get_style_token(pEngine, segment.bgStyleSlot);
                    if (pEngine->onPaintRect && bgStyleToken != 0) {
                        pEngine->onPaintRect(pEngine, bgStyleToken, drgui_make_rect(linePosX + segment.posX, linePosY, linePosX + segment.posX + segment.width, linePosY + lineHeight), pElement, pPaintData);
                    }
                } else {
                    // It's normal text.
                    // TODO: Gather the text and properly support UTF-8.
                    const char* text = pEngine->text + segment.iCharBeg;
                    size_t textLength = segment.iCharEnd - segment.iCharBeg;

                    // TODO: Draw text on the base line to properly handle font's of differing sizes.

                    drte_style_token fgStyleToken = drte_engine__get_style_token(pEngine, segment.fgStyleSlot);
                    drte_style_token bgStyleToken = drte_engine__get_style_token(pEngine, segment.bgStyleSlot);
                    if (pEngine->onPaintText && fgStyleToken != 0 && bgStyleToken != 0) {
                        pEngine->onPaintText(pEngine, fgStyleToken, bgStyleToken, text, textLength, linePosX + segment.posX, linePosY, pElement, pPaintData);
                    }
                }

                if (pEngine->text[segment.iCharBeg] == '\n') {
                    break;
                }
            } while (drte_engine__next_segment(pEngine, &segment));


            // The part after the end of the line needs to be drawn.
            float lineRight = linePosX + lineWidth;
            if (lineRight < pEngine->containerWidth) {
                drte_style_token bgStyleToken = pEngine->styles[pEngine->defaultStyleSlot].styleToken;
                if (pEngine->cursorCount > 0 && segment.iLine == drte_engine_get_cursor_line(pEngine, pEngine->cursorCount-1)) {
                    bgStyleToken = pEngine->styles[pEngine->activeLineStyleSlot].styleToken;
                }

                if (pEngine->onPaintRect && bgStyleToken != 0) {
                    pEngine->onPaintRect(pEngine, bgStyleToken, drgui_make_rect(lineRight, linePosY, pEngine->containerWidth, linePosY + lineHeight), pElement, pPaintData);
                }
            }

            linePosY += lineHeight;


            // Go to the first segment of the next line.
            if (!drte_engine__next_segment(pEngine, &segment)) {
                break;
            }

            iLine += 1;
        }        
    } else {
        // Couldn't create a segment iterator. Likely means there is no text. Just draw a single blank line.
        drte_style_token bgStyleToken = pEngine->styles[pEngine->activeLineStyleSlot].styleToken;
        if (pEngine->onPaintRect && bgStyleToken != 0) {
            pEngine->onPaintRect(pEngine, bgStyleToken, drgui_make_rect(linePosX, linePosY, pEngine->containerWidth, linePosY + lineHeight), pElement, pPaintData);
        }
    }


    // Cursors.
    if (pEngine->isShowingCursor && pEngine->isCursorBlinkOn && pEngine->styles[pEngine->cursorStyleSlot].styleToken != 0) {
        for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
            pEngine->onPaintRect(pEngine, pEngine->styles[pEngine->cursorStyleSlot].styleToken, drte_engine_get_cursor_rect(pEngine, iCursor), pElement, pPaintData);
        }
    }


    // The rectangle region below the last line.
    if (linePosY < pEngine->containerHeight && pEngine->styles[pEngine->defaultStyleSlot].styleToken != 0) {
        // TODO: Only draw the intersection of the bottom rectangle with the invalid rectangle.
        drgui_rect tailRect;
        tailRect.left = 0;
        tailRect.top = (iLineBottom + 1) * drte_engine_get_line_height(pEngine) + pEngine->innerOffsetY;
        tailRect.right = pEngine->containerWidth;
        tailRect.bottom = pEngine->containerHeight;
        pEngine->onPaintRect(pEngine, pEngine->styles[pEngine->defaultStyleSlot].styleToken, tailRect /*drgui_rect_intersection(tailRect, rect)*/, pElement, pPaintData);
    }
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

        drte_engine__begin_dirty(pEngine);
        for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
            drte_engine__on_dirty(pEngine, drte_engine_get_cursor_rect(pEngine, iCursor));
        }
        drte_engine__end_dirty(pEngine);
    }
    else
    {
        pEngine->timeToNextCursorBlink -= milliseconds;
    }
}



void drte_engine_paint_line_numbers(drte_engine* pEngine, float lineNumbersWidth, float lineNumbersHeight, drte_engine_on_paint_text_proc onPaintText, drte_engine_on_paint_rect_proc onPaintRect, drgui_element* pElement, void* pPaintData)
{
    if (pEngine == NULL || onPaintText == NULL || onPaintRect == NULL) {
        return;
    }

    float lineHeight = drte_engine_get_line_height(pEngine);

    size_t iLineTop;
    size_t iLineBottom;
    drte_engine_get_visible_lines(pEngine, &iLineTop, &iLineBottom);

    drte_style_token fgStyleToken = pEngine->styles[pEngine->lineNumbersStyleSlot].styleToken;
    drte_style_token bgStyleToken = pEngine->styles[pEngine->lineNumbersStyleSlot].styleToken;

    float lineTop = pEngine->innerOffsetY + (iLineTop * lineHeight);
    for (size_t iLine = iLineTop; iLine <= iLineBottom; ++iLine) {
        char iLineStr[64];
        snprintf(iLineStr, sizeof(iLineStr), "%d", (int)iLine+1);   // TODO: drte_string_to_int(). This snprintf() has shown up in profiling so best fix this.

        float textWidth = 0;
        float textHeight = 0;
        if (pEngine->onMeasureString && fgStyleToken) {
            pEngine->onMeasureString(pEngine, fgStyleToken, iLineStr, strlen(iLineStr), &textWidth, &textHeight);
        }

        float textLeft = lineNumbersWidth - textWidth;
        float textTop  = lineTop + (lineHeight - textHeight) / 2;

        float lineBottom = lineTop + lineHeight;

        if (fgStyleToken != 0 && bgStyleToken != 0) {
            onPaintText(pEngine, fgStyleToken, bgStyleToken, iLineStr, strlen(iLineStr), textLeft, textTop, pElement, pPaintData);
            onPaintRect(pEngine, bgStyleToken, drgui_make_rect(0, lineTop, textLeft, lineBottom), pElement, pPaintData);

            // There could be a region above and below the text. This will happen if the line height of the line numbers is
            // smaller than the main line height.
            if (textHeight < lineHeight) {
                onPaintRect(pEngine, bgStyleToken, drgui_make_rect(textLeft, lineTop, lineNumbersWidth, textTop), pElement, pPaintData);
                onPaintRect(pEngine, bgStyleToken, drgui_make_rect(textLeft, textTop + textHeight, lineNumbersWidth, lineBottom), pElement, pPaintData);
            }
        }

        lineTop = lineBottom;
    }

    // The region below the lines.
    if (lineTop < pEngine->containerHeight && bgStyleToken != 0) {
        onPaintRect(pEngine, bgStyleToken, drgui_make_rect(0, lineTop, lineNumbersWidth, lineNumbersHeight), pElement, pPaintData);
    }
}


bool drte_engine_find_next(drte_engine* pEngine, const char* text, size_t* pSelectionStartOut, size_t* pSelectionEndOut)
{
    if (pEngine == NULL || pEngine->text == NULL || text == NULL || text[0] == '\0') {
        return false;
    }

    size_t cursorPos = 0;
    if (pEngine->cursorCount > 0) {
        cursorPos = drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->pCursors[pEngine->cursorCount-1]);
    }

    char* nextOccurance = strstr(pEngine->text + cursorPos, text);
    if (nextOccurance == NULL) {
        nextOccurance = strstr(pEngine->text, text);
    }

    if (nextOccurance == NULL) {
        return false;
    }

    if (pSelectionStartOut) {
        *pSelectionStartOut = nextOccurance - pEngine->text;
    }
    if (pSelectionEndOut) {
        *pSelectionEndOut = (nextOccurance - pEngine->text) + strlen(text);
    }

    return true;
}

bool drte_engine_find_next_no_loop(drte_engine* pEngine, const char* text, size_t* pSelectionStartOut, size_t* pSelectionEndOut)
{
    if (pEngine == NULL || pEngine->text == NULL || text == NULL || text[0] == '\0') {
        return false;
    }

    size_t cursorPos = 0;
    if (pEngine->cursorCount > 0) {
        drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->pCursors[pEngine->cursorCount-1]);
    }

    char* nextOccurance = strstr(pEngine->text + cursorPos, text);
    if (nextOccurance == NULL) {
        return false;
    }

    if (pSelectionStartOut) {
        *pSelectionStartOut = nextOccurance - pEngine->text;
    }
    if (pSelectionEndOut) {
        *pSelectionEndOut = (nextOccurance - pEngine->text) + strlen(text);
    }

    return true;
}


void drte_engine__repaint(drte_engine* pEngine)
{
    assert(pEngine != NULL);
    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
}


float drte_engine__get_tab_width(drte_engine* pEngine)
{
    float tabWidth = (float)(pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.spaceWidth * pEngine->tabSizeInSpaces);
    if (tabWidth <= 0) {
        tabWidth = (float)pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.spaceWidth;
        if (tabWidth <= 0) {
            tabWidth = 4;
        }
    }

    return tabWidth;
}



drte_marker drte_engine__new_marker()
{
    drte_marker marker;
    marker.iCharAbs = 0;
    marker.absoluteSickyPosX = 0;

    return marker;
}

bool drte_engine__move_marker_to_point_relative_to_container(drte_engine* pEngine, drte_marker* pMarker, float inputPosX, float inputPosY)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    pMarker->iCharAbs = 0;
    pMarker->absoluteSickyPosX = 0;

    float inputPosXRelativeToText = inputPosX - pEngine->innerOffsetX;
    float inputPosYRelativeToText = inputPosY - pEngine->innerOffsetY;
    if (drte_engine__move_marker_to_point(pEngine, pMarker, inputPosXRelativeToText, inputPosYRelativeToText))
    {
        drte_engine__update_marker_sticky_position(pEngine, pMarker);
        return true;
    }

    return false;
}

void drte_engine__get_marker_position_relative_to_container(drte_engine* pEngine, drte_marker* pMarker, float* pPosXOut, float* pPosYOut)
{
    if (pPosXOut) *pPosXOut = 0;
    if (pPosYOut) *pPosYOut = 0;

    if (pEngine == NULL || pMarker == NULL) {
        return;
    }

    float posX = 0;
    float posY = 0;
    drte_engine_get_character_position(pEngine, pMarker->iCharAbs, &posX, &posY);

    if (pPosXOut) *pPosXOut = posX + pEngine->innerOffsetX;
    if (pPosYOut) *pPosYOut = posY + pEngine->innerOffsetY;
}

bool drte_engine__move_marker_to_point(drte_engine* pEngine, drte_marker* pMarker, float inputPosXRelativeToText, float inputPosYRelativeToText)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    size_t iLine = drte_engine_get_line_at_pos_y(pEngine, inputPosYRelativeToText);

    // Once we have the line, finding the specific character under the point is done by iterating over each segment and finding the one
    // containing the point on the x axis. Once the segment has been found, we use the backend to get the exact character.
    if (inputPosXRelativeToText < 0) {
        pMarker->iCharAbs = drte_engine_get_line_first_character(pEngine, (size_t)iLine);
        return true;    // It's to the left of the line, so just pin it to the first character in the line.
    }

    drte_segment segment;
    if (drte_engine__first_segment_on_line(pEngine, (size_t)iLine, &segment)) {
        do
        {
            if (inputPosXRelativeToText >= segment.posX && inputPosXRelativeToText < segment.posX + segment.width) {
                // It's somewhere on this run. If it's a tab segment it needs to be handled slightly differently because of the way tabs
                // are aligned to tab columns.
                if (drte_engine_get_utf32(pEngine, segment.iCharBeg) == '\t') {
                    const float tabWidth = drte_engine__get_tab_width(pEngine);

                    pMarker->iCharAbs = segment.iCharBeg;

                    float tabLeft = segment.posX;
                    for (/* Do Nothing*/; pMarker->iCharAbs < segment.iCharEnd; ++pMarker->iCharAbs)
                    {
                        float tabRight = tabWidth * ((segment.posX + (tabWidth*((pMarker->iCharAbs-segment.iCharBeg) + 1))) / tabWidth);
                        if (inputPosXRelativeToText >= tabLeft && inputPosXRelativeToText <= tabRight)
                        {
                            // The input position is somewhere on top of this character. If it's positioned on the left side of the character, set the output
                            // value to the character at iChar. Otherwise it should be set to the character at iChar + 1.
                            float charBoundsRightHalf = tabLeft + ceilf(((tabRight - tabLeft) / 2.0f));
                            if (inputPosXRelativeToText > charBoundsRightHalf) {
                                pMarker->iCharAbs += 1;
                            }

                            break;
                        }

                        tabLeft = tabRight;
                    }
                } else {
                    float unused;
                    size_t iChar;

                    drte_style_token fgStyleToken = drte_engine__get_style_token(pEngine, segment.fgStyleSlot);
                    if (pEngine->onGetCursorPositionFromPoint) {
                        pEngine->onGetCursorPositionFromPoint(pEngine, fgStyleToken, pEngine->text + segment.iCharBeg, segment.iCharEnd - segment.iCharBeg, segment.width, inputPosXRelativeToText - segment.posX, OUT &unused, OUT &iChar);
                        pMarker->iCharAbs = segment.iCharBeg + iChar;
                    }
                }

                return true;
            }
        } while (drte_engine__next_segment_on_line(pEngine, &segment));

        // If we get here it means the position is to the right of the line. Just pin it to the end of the line.
        assert(drte_engine_get_utf32(pEngine, segment.iCharBeg) == '\n' || drte_engine_get_utf32(pEngine, segment.iCharBeg) == '\0');
        pMarker->iCharAbs = segment.iCharBeg;   // <-- segment.iCharBeg should be sitting on a new line or null terminator.

        return true;
    }

    return false;
}

bool drte_engine__move_marker_left(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (pMarker->iCharAbs == 0) {
        return false;   // Already at the start of the string. Nowhere to go.
    }

    pMarker->iCharAbs -= 1;

    drte_engine__update_marker_sticky_position(pEngine, pMarker);
    return true;
}

bool drte_engine__move_marker_right(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (pMarker->iCharAbs >= pEngine->textLength) {
        return false;   // Already at the end. Nowhere to go.
    }

    pMarker->iCharAbs += 1;

    drte_engine__update_marker_sticky_position(pEngine, pMarker);
    return true;
}

bool drte_engine__move_marker_up(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    return drte_engine__move_marker_y(pEngine, pMarker, -1);
}

bool drte_engine__move_marker_down(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    return drte_engine__move_marker_y(pEngine, pMarker, 1);
}

bool drte_engine__move_marker_y(drte_engine* pEngine, drte_marker* pMarker, int amount)
{
    if (pEngine == NULL || pMarker == NULL || amount == 0) {
        return false;
    }

    size_t lineCount = drte_engine_get_line_count(pEngine);
    if (lineCount == 0) {
        return false;
    }

    // Moving a marker up or down depends on it's sticky position.
    intptr_t iNewLine = drte_engine_get_character_line(pEngine, pMarker->iCharAbs) + amount;
    if (iNewLine < 0) {
        iNewLine = 0;
    }
    if ((size_t)iNewLine > lineCount) {
        iNewLine = lineCount - 1;
    }

    float newMarkerPosX = pMarker->absoluteSickyPosX;
    float newMarkerPosY = drte_engine_get_line_pos_y(pEngine, (size_t)iNewLine);
    drte_engine__move_marker_to_point(pEngine, pMarker, newMarkerPosX, newMarkerPosY);

    return true;
}

bool drte_engine__move_marker_to_end_of_line(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (!drte_engine__move_marker_to_character(pEngine, pMarker, drte_engine_get_line_last_character(pEngine, drte_engine_get_character_line(pEngine, pMarker->iCharAbs)))) {
        return false;
    }

    drte_engine__update_marker_sticky_position(pEngine, pMarker);
    return true;
}

bool drte_engine__move_marker_to_start_of_line(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (!drte_engine__move_marker_to_character(pEngine, pMarker, drte_engine_get_line_first_character(pEngine, drte_engine_get_character_line(pEngine, pMarker->iCharAbs)))) {
        return false;
    }

    drte_engine__update_marker_sticky_position(pEngine, pMarker);
    return true;
}

bool drte_engine__move_marker_to_end_of_line_by_index(drte_engine* pEngine, drte_marker* pMarker, size_t iLine)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (!drte_engine__move_marker_to_character(pEngine, pMarker, drte_engine_get_line_last_character(pEngine, iLine))) {
        return false;
    }

    drte_engine__update_marker_sticky_position(pEngine, pMarker);
    return true;
}

bool drte_engine__move_marker_to_start_of_line_by_index(drte_engine* pEngine, drte_marker* pMarker, size_t iLine)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (!drte_engine__move_marker_to_character(pEngine, pMarker, drte_engine_get_line_first_character(pEngine, iLine))) {
        return false;
    }

    drte_engine__update_marker_sticky_position(pEngine, pMarker);
    return true;
}

bool drte_engine__move_marker_to_end_of_text(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (!drte_engine__move_marker_to_character(pEngine, pMarker, pEngine->textLength)) {    // <-- Don't subtract 1 here because otherwise we'll end up on the _left_ of the last character.
        return false;
    }

    drte_engine__update_marker_sticky_position(pEngine, pMarker);
    return true;
}

bool drte_engine__move_marker_to_start_of_text(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (!drte_engine__move_marker_to_character(pEngine, pMarker, 0)) {
        return false;
    }

    drte_engine__update_marker_sticky_position(pEngine, pMarker);
    return true;
}

bool drte_engine__move_marker_to_character(drte_engine* pEngine, drte_marker* pMarker, size_t iChar)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    // Clamp the character to the end of the string.
    if (iChar > pEngine->textLength) {
        iChar = pEngine->textLength;
    }

    pMarker->iCharAbs = iChar;
    return true;
}


void drte_engine__update_marker_sticky_position(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return;
    }

    float charPosX;
    float charPosY;
    drte_engine_get_character_position(pEngine, pMarker->iCharAbs, &charPosX, &charPosY);

    pMarker->absoluteSickyPosX = charPosX;
}

size_t drte_engine__get_marker_absolute_char_index(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return 0;
    }

    return pMarker->iCharAbs;
}



void drte_engine__trim_undo_stack(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    while (pEngine->undoStackCount > pEngine->iUndoState)
    {
        unsigned int iLastItem = pEngine->undoStackCount - 1;

        drte_engine__uninit_undo_state(pEngine->pUndoStack + iLastItem);
        pEngine->undoStackCount -= 1;
    }
}

bool drte_engine__diff_states(drte_engine_state* pPrevState, drte_engine_state* pCurrentState, drte_engine_undo_state* pUndoStateOut)
{
    if (pPrevState == NULL || pCurrentState == NULL || pUndoStateOut == NULL) {
        return false;
    }

    if (pPrevState->text == NULL || pCurrentState->text == NULL) {
        return false;
    }

    const char* prevText = pPrevState->text;
    const char* currText = pCurrentState->text;

    const size_t prevLen = strlen(prevText);
    const size_t currLen = strlen(currText);


    // The first step is to find the position of the first differing character.
    size_t sameChCountStart;
    for (sameChCountStart = 0; sameChCountStart < prevLen && sameChCountStart < currLen; ++sameChCountStart)
    {
        char prevCh = prevText[sameChCountStart];
        char currCh = currText[sameChCountStart];

        if (prevCh != currCh) {
            break;
        }
    }

    // The next step is to find the position of the last differing character.
    size_t sameChCountEnd;
    for (sameChCountEnd = 0; sameChCountEnd < prevLen && sameChCountEnd < currLen; ++sameChCountEnd)
    {
        // Don't move beyond the first differing character.
        if (prevLen - sameChCountEnd <= sameChCountStart ||
            currLen - sameChCountEnd <= sameChCountStart)
        {
            break;
        }

        char prevCh = prevText[prevLen - sameChCountEnd - 1];
        char currCh = currText[currLen - sameChCountEnd - 1];

        if (prevCh != currCh) {
            break;
        }
    }


    // At this point we know which section of the text is different. We now need to initialize the undo state object.
    pUndoStateOut->diffPos       = sameChCountStart;
    pUndoStateOut->newState      = *pCurrentState;
    pUndoStateOut->newState.text = NULL;
    pUndoStateOut->oldState      = *pPrevState;
    pUndoStateOut->oldState.text = NULL;

    size_t oldTextLen = prevLen - sameChCountStart - sameChCountEnd;
    pUndoStateOut->oldText = (char*)malloc(oldTextLen + 1);
    drgui__strncpy_s(pUndoStateOut->oldText, oldTextLen + 1, prevText + sameChCountStart, oldTextLen);

    pUndoStateOut->oldState.cursorCount = pPrevState->cursorCount;
    pUndoStateOut->oldState.pCursors = (drte_marker*)malloc(pPrevState->cursorCount * sizeof(*pUndoStateOut->oldState.pCursors));
    memcpy(pUndoStateOut->oldState.pCursors, pPrevState->pCursors, pPrevState->cursorCount * sizeof(*pUndoStateOut->oldState.pCursors));

    pUndoStateOut->oldState.selectionCount = pPrevState->selectionCount;
    pUndoStateOut->oldState.pSelections = (drte_region*)malloc(pPrevState->selectionCount * sizeof(*pUndoStateOut->oldState.pSelections));
    memcpy(pUndoStateOut->oldState.pSelections, pPrevState->pSelections, pPrevState->selectionCount * sizeof(*pUndoStateOut->oldState.pSelections));

    pUndoStateOut->oldState.userDataSize = pPrevState->userDataSize;
    pUndoStateOut->oldState.pUserData = NULL;
    if (pPrevState->userDataSize > 0 && pPrevState->pUserData != NULL) {
        pUndoStateOut->oldState.pUserData = malloc(pPrevState->userDataSize);
        memcpy(pUndoStateOut->oldState.pUserData, pPrevState->pUserData, pPrevState->userDataSize);
    }


    size_t newTextLen = currLen - sameChCountStart - sameChCountEnd;
    pUndoStateOut->newText = (char*)malloc(newTextLen + 1);
    drgui__strncpy_s(pUndoStateOut->newText, newTextLen + 1, currText + sameChCountStart, newTextLen);

    pUndoStateOut->newState.cursorCount = pCurrentState->cursorCount;
    pUndoStateOut->newState.pCursors = (drte_marker*)malloc(pCurrentState->cursorCount * sizeof(*pUndoStateOut->newState.pCursors));
    memcpy(pUndoStateOut->newState.pCursors, pCurrentState->pCursors, pCurrentState->cursorCount * sizeof(*pUndoStateOut->newState.pCursors));

    pUndoStateOut->newState.selectionCount = pCurrentState->selectionCount;
    pUndoStateOut->newState.pSelections = (drte_region*)malloc(pCurrentState->selectionCount * sizeof(*pUndoStateOut->newState.pSelections));
    memcpy(pUndoStateOut->newState.pSelections, pCurrentState->pSelections, pCurrentState->selectionCount * sizeof(*pUndoStateOut->newState.pSelections));

    pUndoStateOut->newState.userDataSize = pCurrentState->userDataSize;
    pUndoStateOut->newState.pUserData = NULL;
    if (pPrevState->userDataSize > 0 && pCurrentState->pUserData != NULL) {
        pUndoStateOut->newState.pUserData = malloc(pCurrentState->userDataSize);
        memcpy(pUndoStateOut->newState.pUserData, pCurrentState->pUserData, pCurrentState->userDataSize);
    }

    return true;
}

void drte_engine__uninit_undo_state(drte_engine_undo_state* pUndoState)
{
    if (pUndoState == NULL) {
        return;
    }

    free(pUndoState->oldText);
    pUndoState->oldText = NULL;

    free(pUndoState->oldState.pCursors);
    pUndoState->oldState.pCursors = NULL;

    free(pUndoState->oldState.pSelections);
    pUndoState->oldState.pSelections = NULL;

    free(pUndoState->oldState.pUserData);
    pUndoState->oldState.pUserData = NULL;


    free(pUndoState->newText);
    pUndoState->newText = NULL;

    free(pUndoState->newState.pCursors);
    pUndoState->newState.pCursors = NULL;

    free(pUndoState->newState.pSelections);
    pUndoState->newState.pSelections = NULL;

    free(pUndoState->newState.pUserData);
    pUndoState->newState.pUserData = NULL;
}

void drte_engine__push_undo_state(drte_engine* pEngine, drte_engine_undo_state* pUndoState)
{
    if (pEngine == NULL || pUndoState == NULL) {
        return;
    }

    assert(pEngine->iUndoState == pEngine->undoStackCount);


    drte_engine_undo_state* pOldStack = pEngine->pUndoStack;
    drte_engine_undo_state* pNewStack = (drte_engine_undo_state*)malloc(sizeof(*pNewStack) * (pEngine->undoStackCount + 1));

    if (pEngine->undoStackCount > 0) {
        memcpy(pNewStack, pOldStack, sizeof(*pNewStack) * (pEngine->undoStackCount));
    }

    pNewStack[pEngine->undoStackCount] = *pUndoState;
    pEngine->pUndoStack = pNewStack;
    pEngine->undoStackCount += 1;
    pEngine->iUndoState += 1;

    if (pEngine->onUndoPointChanged) {
        pEngine->onUndoPointChanged(pEngine, pEngine->iUndoState);
    }

    free(pOldStack);
}

void drte_engine__apply_undo_state(drte_engine* pEngine, drte_engine_undo_state* pUndoState)
{
    if (pEngine == NULL || pUndoState == NULL) {
        return;
    }

    // When undoing we want to remove the new text and replace it with the old text.

    // TODO: This needs improving because it results in multiple onTextChanged events being posted.
    drte_engine__begin_dirty(pEngine);
    {
        // Remove the new text.
        size_t iFirstCh     = pUndoState->diffPos;
        size_t iLastChPlus1 = pUndoState->diffPos + strlen(pUndoState->newText);
        drte_engine_delete_text(pEngine, iFirstCh, iLastChPlus1);

        // Insert the old text.
        drte_engine_insert_text(pEngine, pUndoState->oldText, pUndoState->diffPos);

        // Cursors.
        if (pUndoState->oldState.cursorCount > 0) {
            drte_marker* pNewCursors = (drte_marker*)realloc(pEngine->pCursors, pUndoState->oldState.cursorCount * sizeof(*pNewCursors));
            if (pNewCursors != NULL) {
                pEngine->pCursors = pNewCursors;
                for (size_t iCursor = 0; iCursor < pUndoState->oldState.cursorCount; ++iCursor) {
                    pEngine->pCursors[iCursor] = pUndoState->oldState.pCursors[iCursor];
                    drte_engine__update_marker_sticky_position(pEngine, &pEngine->pCursors[iCursor]);
                }

                pEngine->cursorCount = pUndoState->oldState.cursorCount;
            }
        } else {
            pEngine->cursorCount = 0;
        }

        // Selections.
        if (pUndoState->oldState.selectionCount > 0) {
            drte_region* pNewSelections = (drte_region*)realloc(pEngine->pSelections, pUndoState->oldState.selectionCount * sizeof(*pNewSelections));
            if (pNewSelections != NULL) {
                pEngine->pSelections = pNewSelections;
                for (size_t iSelection = 0; iSelection < pUndoState->oldState.selectionCount; ++iSelection) {
                    pEngine->pSelections[iSelection] = pUndoState->oldState.pSelections[iSelection];
                }

                pEngine->selectionCount = pUndoState->oldState.selectionCount;
            }
        } else {
            pEngine->selectionCount = 0;
        }



        if (pEngine->onTextChanged) {
            pEngine->onTextChanged(pEngine);
        }

        for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
            drte_engine__on_cursor_move(pEngine, iCursor);
        }


        // Application-defined data.
        if (pEngine->onApplyUndoState) {
            pEngine->onApplyUndoState(pEngine, pUndoState->oldState.userDataSize, pUndoState->oldState.pUserData);
        }
    }
    drte_engine__end_dirty(pEngine);
}

void drte_engine__apply_redo_state(drte_engine* pEngine, drte_engine_undo_state* pUndoState)
{
    if (pEngine == NULL || pUndoState == NULL) {
        return;
    }

    // An redo is just the opposite of an undo. We want to remove the old text and replace it with the new text.

    // TODO: This needs improving because it results in multiple onTextChanged events being posted.
    drte_engine__begin_dirty(pEngine);
    {
        // Remove the old text.
        size_t iFirstCh     = pUndoState->diffPos;
        size_t iLastChPlus1 = pUndoState->diffPos + strlen(pUndoState->oldText);
        drte_engine_delete_text(pEngine, iFirstCh, iLastChPlus1);

        // Insert the new text.
        drte_engine_insert_text(pEngine, pUndoState->newText, pUndoState->diffPos);

        // Cursors.
        if (pUndoState->newState.cursorCount > 0) {
            drte_marker* pNewCursors = (drte_marker*)realloc(pEngine->pCursors, pUndoState->newState.cursorCount * sizeof(*pNewCursors));
            if (pNewCursors != NULL) {
                pEngine->pCursors = pNewCursors;
                for (size_t iCursor = 0; iCursor < pUndoState->newState.cursorCount; ++iCursor) {
                    pEngine->pCursors[iCursor] = pUndoState->newState.pCursors[iCursor];
                    drte_engine__update_marker_sticky_position(pEngine, &pEngine->pCursors[iCursor]);
                }

                pEngine->cursorCount = pUndoState->newState.cursorCount;
            }
        } else {
            pEngine->cursorCount = 0;
        }

        // Selections.
        if (pUndoState->newState.selectionCount > 0) {
            drte_region* pNewSelections = (drte_region*)realloc(pEngine->pSelections, pUndoState->newState.selectionCount * sizeof(*pNewSelections));
            if (pNewSelections != NULL) {
                pEngine->pSelections = pNewSelections;
                for (size_t iSelection = 0; iSelection < pUndoState->newState.selectionCount; ++iSelection) {
                    pEngine->pSelections[iSelection] = pUndoState->newState.pSelections[iSelection];
                }

                pEngine->selectionCount = pUndoState->newState.selectionCount;
            }
        } else {
            pEngine->selectionCount = 0;
        }


        if (pEngine->onTextChanged) {
            pEngine->onTextChanged(pEngine);
        }

        for (size_t iCursor = 0; iCursor < pEngine->cursorCount; ++iCursor) {
            drte_engine__on_cursor_move(pEngine, iCursor);
        }


        // Application-defined data.
        if (pEngine->onApplyUndoState) {
            pEngine->onApplyUndoState(pEngine, pUndoState->newState.userDataSize, pUndoState->newState.pUserData);
        }
    }
    drte_engine__end_dirty(pEngine);
}


drgui_rect drte_engine__local_rect(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return drgui_make_rect(0, 0, 0, 0);
    }

    return drgui_make_rect(0, 0, pEngine->containerWidth, pEngine->containerHeight);
}


void drte_engine__on_cursor_move(drte_engine* pEngine, size_t cursorIndex)
{
    if (pEngine == NULL) {
        return;
    }

    // When the cursor moves we want to reset the cursor's blink state.
    pEngine->timeToNextCursorBlink = pEngine->cursorBlinkRate;
    pEngine->isCursorBlinkOn = true;

    if (pEngine->onCursorMove) {
        pEngine->onCursorMove(pEngine);
    }

    drte_engine__on_dirty(pEngine, drte_engine_get_cursor_rect(pEngine, cursorIndex));
}

void drte_engine__on_dirty(drte_engine* pEngine, drgui_rect rect)
{
    drte_engine__begin_dirty(pEngine);
    {
        pEngine->accumulatedDirtyRect = drgui_rect_union(pEngine->accumulatedDirtyRect, rect);
    }
    drte_engine__end_dirty(pEngine);
}

void drte_engine__begin_dirty(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->dirtyCounter += 1;
}

void drte_engine__end_dirty(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    assert(pEngine->dirtyCounter > 0);

    pEngine->dirtyCounter -= 1;

    if (pEngine->dirtyCounter == 0) {
        if (pEngine->onDirty && drgui_rect_has_volume(pEngine->accumulatedDirtyRect)) {
            pEngine->onDirty(pEngine, pEngine->accumulatedDirtyRect);
        }

        pEngine->accumulatedDirtyRect = drgui_make_inside_out_rect();
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

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


typedef void (* drte_engine_on_measure_string_proc)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textLength, float* pWidthOut, float* pHeightOut);
typedef void (* drte_engine_on_get_cursor_position_from_point)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut);
typedef void (* drte_engine_on_get_cursor_position_from_char)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t characterIndex, float* pTextCursorPosXOut);

typedef void (* drte_engine_on_paint_text_proc)        (drte_engine* pEngine, drte_style_token styleTokenFG, drte_style_token styleTokenBG, const char* text, size_t textLength, float posX, float posY, drgui_element* pElement, void* pPaintData);
typedef void (* drte_engine_on_paint_rect_proc)        (drte_engine* pEngine, drte_style_token styleToken, drgui_rect rect, drgui_element* pElement, void* pPaintData);
typedef void (* drte_engine_on_cursor_move_proc)       (drte_engine* pEngine);
typedef void (* drte_engine_on_dirty_proc)             (drte_engine* pEngine, drgui_rect rect);
typedef void (* drte_engine_on_text_changed_proc)      (drte_engine* pEngine);
typedef void (* drte_engine_on_undo_point_changed_proc)(drte_engine* pEngine, unsigned int iUndoPoint);

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
    /// The text. Can be null in some cases where it isn't used.
    char* text;

    /// The index of the character the cursor is positioned at.
    size_t cursorPos;

    /// The index of the character the selection anchor is positioned at.
    size_t selectionAnchorPos;

    /// Whether or not anything is selected.
    bool isAnythingSelected;

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
    drte_engine_on_get_cursor_position_from_point onGetCursorPositionFromPoint;

    // The function to call when the position of the cursor needs to be retrieved based on a character at a specific index.
    drte_engine_on_get_cursor_position_from_char onGetCursorPositionFromChar;


    // The index of the first character of every line.
    //size_t* pLines;
    //size_t lineCount;
    //size_t lineBufferSize;



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


    /// The total width of the text.
    float textBoundsWidth;

    /// The total height of the text.
    float textBoundsHeight;


    /// The cursor.
    drte_marker cursor;

    /// The selection anchor.
    drte_marker selectionAnchor;


    /// The selection mode counter. When this is greater than 0 we are in selection mode, otherwise we are not. This
    /// is incremented by enter_selection_mode() and decremented by leave_selection_mode().
    unsigned int selectionModeCounter;

    /// Whether or not anything is selected.
    bool isAnythingSelected;    // <-- TODO: I don't like this. See if we can get rid of it.


    // The list of styling segments set by the application. This is used for syntax highlighting.
    drte_style_segment* pStyleSegments;

    // The number of active style segments.
    size_t styleSegmentCount;

    // The size of the buffer holding the style segments.
    size_t styleSegmentBufferSize;




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


// Gets the character at the given index as a UTF-32 code point.
uint32_t drte_engine_get_utf32(drte_engine* pEngine, size_t characterIndex);

// Retrieves the indices of the visible lines.
void drte_engine_get_visible_lines(drte_engine* pEngine, size_t* pFirstLineOut, size_t* pLastLineOut);


// Adds a style segment.
void drte_engine_add_style_segment(drte_engine* pEngine, size_t iCharBeg, size_t iCharEnd, drte_style_token styleToken);


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


/// Retrieves the rectangle of the text relative to the bounds, taking alignment into account.
drgui_rect drte_engine_get_text_rect_relative_to_bounds(drte_engine* pEngine);


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

/// Retrieves the position of the cursor, relative to the container.
void drte_engine_get_cursor_position(drte_engine* pEngine, float* pPosXOut, float* pPosYOut);

/// Retrieves the rectangle of the cursor, relative to the container.
drgui_rect drte_engine_get_cursor_rect(drte_engine* pEngine);

/// Retrieves the index of the line the cursor is currently sitting on.
size_t drte_engine_get_cursor_line(drte_engine* pEngine);

/// Retrieves the index of the column the cursor is currently sitting on.
size_t drte_engine_get_cursor_column(drte_engine* pEngine);

/// Retrieves the index of the character the cursor is currently sitting on.
size_t drte_engine_get_cursor_character(drte_engine* pEngine);

/// Moves the cursor to the closest character based on the given input position.
void drte_engine_move_cursor_to_point(drte_engine* pEngine, float posX, float posY);

/// Moves the cursor of the given text engine to the left by one character.
bool drte_engine_move_cursor_left(drte_engine* pEngine);

/// Moves the cursor of the given text engine to the right by one character.
bool drte_engine_move_cursor_right(drte_engine* pEngine);

/// Moves the cursor of the given text engine up one line.
bool drte_engine_move_cursor_up(drte_engine* pEngine);

/// Moves the cursor of the given text engine down one line.
bool drte_engine_move_cursor_down(drte_engine* pEngine);

/// Moves the cursor up or down the given number of lines.
bool drte_engine_move_cursor_y(drte_engine* pEngine, int amount);

/// Moves the cursor of the given text engine to the end of the line.
bool drte_engine_move_cursor_to_end_of_line(drte_engine* pEngine);

/// Moves the cursor of the given text engine to the start of the line.
bool drte_engine_move_cursor_to_start_of_line(drte_engine* pEngine);

/// Moves the cursor of the given text engine to the end of the line at the given index.
bool drte_engine_move_cursor_to_end_of_line_by_index(drte_engine* pEngine, size_t iLine);

/// Moves the cursor of the given text engine to the start of the line at the given index.
bool drte_engine_move_cursor_to_start_of_line_by_index(drte_engine* pEngine, size_t iLine);

/// Moves the cursor of the given text engine to the end of the text.
bool drte_engine_move_cursor_to_end_of_text(drte_engine* pEngine);

/// Moves the cursor of the given text engine to the end of the text.
bool drte_engine_move_cursor_to_start_of_text(drte_engine* pEngine);

/// Moves the cursor to the start of the selected text.
void drte_engine_move_cursor_to_start_of_selection(drte_engine* pEngine);

/// Moves the cursor to the end of the selected text.
void drte_engine_move_cursor_to_end_of_selection(drte_engine* pEngine);

/// Moves the cursor to the given character index.
void drte_engine_move_cursor_to_character(drte_engine* pEngine, size_t characterIndex);

// Moves the cursor to the end of the current word.
size_t drte_engine_move_cursor_to_end_of_word(drte_engine* pEngine);

// Moves the cursor to the start of the next word.
size_t drte_engine_move_cursor_to_start_of_next_word(drte_engine* pEngine);

// Moves the cursor to the start of the current word.
size_t drte_engine_move_cursor_to_start_of_word(drte_engine* pEngine);

/// Retrieves the number of characters between the cursor and the next tab column.
size_t drte_engine_get_spaces_to_next_colum_from_cursor(drte_engine* pEngine);

/// Determines whether or not the cursor is sitting at the start of the selection.
bool drte_engine_is_cursor_at_start_of_selection(drte_engine* pEngine);

/// Determines whether or not the cursor is sitting at the end fo the selection.
bool drte_engine_is_cursor_at_end_of_selection(drte_engine* pEngine);

/// Swaps the position of the cursor based on the current selection.
void drte_engine_swap_selection_markers(drte_engine* pEngine);

/// Sets the function to call when the cursor in the given text engine is mvoed.
void drte_engine_set_on_cursor_move(drte_engine* pEngine, drte_engine_on_cursor_move_proc proc);


/// Inserts a character into the given text engine.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_insert_character(drte_engine* pEngine, unsigned int character, size_t insertIndex);

/// Inserts the given string at the given character index.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_insert_text(drte_engine* pEngine, const char* text, size_t insertIndex);

/// Deletes a range of text in the given text engine.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_delete_text_range(drte_engine* pEngine, size_t iFirstCh, size_t iLastChPlus1);

/// Inserts a character at the position of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_insert_character_at_cursor(drte_engine* pEngine, unsigned int character);

/// Inserts a character at the position of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_insert_text_at_cursor(drte_engine* pEngine, const char* text);

/// Deletes the character to the left of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_delete_character_to_left_of_cursor(drte_engine* pEngine);

/// Deletes the character to the right of the cursor.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_delete_character_to_right_of_cursor(drte_engine* pEngine);

/// Deletes the currently selected text.
///
/// @return True if the text within the text engine has changed.
bool drte_engine_delete_selected_text(drte_engine* pEngine);


/// Enter's into selection mode.
///
/// @remarks
///     An application will typically enter selection mode when the Shift key is pressed, and then leave when the key is released.
///     @par
///     This will increment an internal counter, which is decremented with a corresponding call to drte_engine_leave_selection_mode().
///     Selection mode will be enabled so long as this counter is greater than 0. Thus, you must ensure you cleanly leave selection
///     mode.
void drte_engine_enter_selection_mode(drte_engine* pEngine);

/// Leaves selection mode.
///
/// @remarks
///     This decrements the internal counter. Selection mode will not be disabled while this reference counter is greater than 0. Always
///     ensure a leave is correctly matched with an enter.
void drte_engine_leave_selection_mode(drte_engine* pEngine);

/// Determines whether or not the given text engine is in selection mode.
bool drte_engine_is_in_selection_mode(drte_engine* pEngine);

/// Determines whether or not anything is selected in the given text engine.
bool drte_engine_is_anything_selected(drte_engine* pEngine);

/// Deselects everything in the given text engine.
void drte_engine_deselect_all(drte_engine* pEngine);

/// Selects everything in the given text engine.
void drte_engine_select_all(drte_engine* pEngine);

/// Selects the given range of text.
void drte_engine_select(drte_engine* pEngine, size_t firstCharacter, size_t lastCharacter);

// Selects the word under the cursor.
void drte_engine_select_word_under_cursor(drte_engine* pEngine);

/// Retrieves a copy of the selected text.
///
/// @remarks
///     This returns the length of the selected text. Call this once with <textOut> set to NULL to calculate the required size of the
///     buffer.
///     @par
///     If the output buffer is not larger enough, the string will be truncated.
size_t drte_engine_get_selected_text(drte_engine* pEngine, char* textOut, size_t textOutLength);

/// Retrieves the index of the first line of the current selection.
size_t drte_engine_get_selection_first_line(drte_engine* pEngine);

/// Retrieves the index of the last line of the current selection.
size_t drte_engine_get_selection_last_line(drte_engine* pEngine);

/// Moves the selection anchor to the end of the given line.
void drte_engine_move_selection_anchor_to_end_of_line(drte_engine* pEngine, size_t iLine);

/// Moves the selection anchor to the start of the given line.
void drte_engine_move_selection_anchor_to_start_of_line(drte_engine* pEngine, size_t iLine);

/// Retrieves the line the selection anchor is sitting on.
size_t drte_engine_get_selection_anchor_line(drte_engine* pEngine);


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

#define DRTE_INVALID_STYLE_SLOT    255

// Flags for the drte_engine::flags property.
#define DRTE_USE_EXPLICIT_LINE_HEIGHT   (1 << 0)


// min
#define drte_min(a, b) (((a) < (b)) ? (a) : (b))



// Helper for determining whether or not the given character is a symbol or whitespace.
bool drte_is_symbol_or_whitespace(uint32_t utf32)
{
    return (utf32 < '0') || (utf32 >= ':' && utf32 < 'A') || (utf32 >= '[' && utf32 < 'a') || (utf32 > '{'); 
}




// Performs a full repaint of the entire visible region of the text engine.
void drte_engine__repaint(drte_engine* pEngine);


/// Performs a complete refresh of the given text engine.
///
/// @remarks
///     This will delete every run and re-create them.
void drte_engine__refresh(drte_engine* pEngine);


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


/// Helper function for determining whether or not there is any spacing between the selection markers.
bool drte_engine__has_spacing_between_selection_markers(drte_engine* pEngine);


/// Retrieves pointers to the selection markers in the correct order.
bool drte_engine__get_selection_markers(drte_engine* pEngine, drte_marker** ppSelectionMarker0Out, drte_marker** ppSelectionMarker1Out);



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


/// Called when the cursor moves.
void drte_engine__on_cursor_move(drte_engine* pEngine);

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


// Retrieves the style segment that the character at the given index is sitting on.
bool drte_engine__get_character_style_segment(drte_engine* pEngine, size_t iChar, drte_style_segment* pSegmentOut)
{
    assert(pEngine != NULL);
    assert(pSegmentOut != NULL);

    // Just a simple linear search for now.
    for (size_t i = 0; i < pEngine->styleSegmentCount; ++i) {
        if (iChar >= pEngine->pStyleSegments[i].iCharBeg && iChar < pEngine->pStyleSegments[i].iCharEnd) {
            *pSegmentOut = pEngine->pStyleSegments[i];
            return true;
        }
    }

    pSegmentOut->iCharBeg = (size_t)-1;
    pSegmentOut->iCharEnd = (size_t)-1;
    pSegmentOut->styleSlot = DRTE_INVALID_STYLE_SLOT;
    return false;
}

// Retrieves the style segment coming after the given character. This will include the segment the character is sitting in, if any.
bool drte_engine__get_next_style_segment_from_character(drte_engine* pEngine, size_t iChar, drte_style_segment* pSegmentOut)
{
    assert(pEngine != NULL);
    assert(pSegmentOut != NULL);

    // Just a simple linear search for now.
    for (size_t i = 0; i < pEngine->styleSegmentCount; ++i) {
        if (iChar < pEngine->pStyleSegments[i].iCharEnd) {
            *pSegmentOut = pEngine->pStyleSegments[i];
            return true;
        }
    }

    pSegmentOut->iCharBeg = (size_t)-1;
    pSegmentOut->iCharEnd = (size_t)-1;
    pSegmentOut->styleSlot = DRTE_INVALID_STYLE_SLOT;
    return false;
}


// A drte_segment object is used for iterating over the segments of a line.
typedef struct
{
    size_t iLine;
    size_t iCharBeg;
    size_t iCharEnd;
    uint8_t fgStyleSlot;
    uint8_t bgStyleSlot;
    float posX;
    float width;
    bool isAtEnd;
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

    // TODO: Handle selection segments here.
    // TODO: Handle styling segments here.
    // TODO: Handle UTF-8 properly.
    // TODO: There is LOTS of optimization opportunity in this function.

    // The next segment is clamped to a specific character with the following priorities, from highest priority to lowest
    // - The end of the text
    // - The end of the line
    // - Tab boundaries
    // - Selection boundaries
    // - Styling segment boundaries
    //
    // If the next character begins in the middle of a segment, it will be clamped against said segment.


    if (pSegment->isAtEnd) {
        return false;
    }

    uint8_t fgStyleSlot = pEngine->defaultStyleSlot;
    uint8_t bgStyleSlot = pEngine->defaultStyleSlot;

    // Find the end of the next segment, but don't modify the segment yet. The reason for this is that we need to measure the segment later.
    size_t iCharBeg = pSegment->iCharEnd;
    size_t iCharEnd = iCharBeg;

    if (pSegment->iLine == drte_engine_get_cursor_line(pEngine)) {
        bgStyleSlot = pEngine->activeLineStyleSlot;
    }

    // TODO: Fix this for multi-select and multi-cursor.
    bool isAnythingSelected = false;
    bool isInSelection = false;
    size_t iSelectionCharBeg = 0;
    size_t iSelectionCharEnd = 0;

    drte_marker* pSelectionMarker0;
    drte_marker* pSelectionMarker1;
    if (drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1)) {
        iSelectionCharBeg = drte_engine__get_marker_absolute_char_index(pEngine, pSelectionMarker0);
        iSelectionCharEnd = drte_engine__get_marker_absolute_char_index(pEngine, pSelectionMarker1);
        isInSelection = iCharBeg >= iSelectionCharBeg && iCharBeg < iSelectionCharEnd;
        isAnythingSelected = iSelectionCharBeg < iSelectionCharEnd;
    }

    if (isInSelection) {
        bgStyleSlot = pEngine->selectionStyleSlot;
    }


    drte_style_segment styleSegment;
    bool isInStyleSegment = false;
    if (drte_engine__get_character_style_segment(pEngine, iCharBeg, &styleSegment)) {  // <-- Optimize this. This requires a full search, but we only care about segments on the line.
        isInStyleSegment = true;
    } else {
        if (!drte_engine__get_next_style_segment_from_character(pEngine, iCharBeg, &styleSegment)) {
            styleSegment.iCharBeg = (size_t)-1;
            styleSegment.iCharEnd = (size_t)-1;
        }
    }


    bool clampToChar = false;
    size_t iMaxChar = (size_t)-1;

    // Clamp to selection.
    if (isInSelection) {
        clampToChar = true;
        iMaxChar = iSelectionCharEnd;
    } else if (isAnythingSelected) {
        clampToChar = true;
        if (iSelectionCharBeg > iCharBeg) {
            iMaxChar = iSelectionCharBeg;
        }
    }

    // Clamp to style segment.
    if (isInStyleSegment) {
        clampToChar = true;
        fgStyleSlot = styleSegment.styleSlot;
        iMaxChar = drte_min(iMaxChar, styleSegment.iCharEnd);
    } else {
        iMaxChar = drte_min(iMaxChar, styleSegment.iCharBeg);
    }
    



    char c = pEngine->text[iCharBeg];
    if (c == '\0' || c == '\n') {
        pSegment->isAtEnd = true;
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

                        if (clampToChar && iCharEnd == iMaxChar) {
                            break;
                        }

                        iCharEnd += 1;
                    }

                    break;
                }
            }


            // Selection and styling segment clamp.
            if (clampToChar && iCharEnd == iMaxChar) {
                break;
            }

            iCharEnd += 1;
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

bool drte_engine__first_segment(drte_engine* pEngine, size_t lineIndex, drte_segment* pSegment)
{
    if (pEngine == NULL || pEngine->textLength == 0 || pSegment == NULL) {
        return false;
    }

    pSegment->iLine = lineIndex;
    pSegment->iCharBeg = drte_engine_get_line_first_character(pEngine, lineIndex);
    pSegment->iCharEnd = pSegment->iCharBeg;
    pSegment->fgStyleSlot = pEngine->defaultStyleSlot;
    pSegment->bgStyleSlot = pEngine->defaultStyleSlot;
    pSegment->posX = 0;
    pSegment->width = 0;
    pSegment->isAtEnd = false;
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

    //pEngine->lineBufferSize = 16;
    //pEngine->lineCount = 0;
    //pEngine->pLines = (size_t*)malloc(pEngine->lineBufferSize * sizeof(*pEngine->pLines));

    pEngine->styleSegmentBufferSize = 0;
    pEngine->styleSegmentCount = 0;
    pEngine->pStyleSegments = NULL;

    pEngine->tabSizeInSpaces          = 4;
    pEngine->cursorWidth              = 1;
    pEngine->cursorBlinkRate          = 500;
    pEngine->timeToNextCursorBlink    = pEngine->cursorBlinkRate;
    pEngine->isCursorBlinkOn          = true;
    pEngine->isShowingCursor          = false;
    pEngine->cursor                   = drte_engine__new_marker();
    pEngine->selectionAnchor          = drte_engine__new_marker();
    pEngine->accumulatedDirtyRect     = drgui_make_inside_out_rect();
    pEngine->pUserData                = pUserData;

    return pEngine;
}

void drte_engine_delete(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    drte_engine_clear_undo_stack(pEngine);

    free(pEngine->preparedState.text);
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

        drte_engine__refresh(pEngine);
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
    drte_engine__refresh(pEngine);
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
        drte_engine__refresh(pEngine);
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
    drte_engine__refresh(pEngine);
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
    drte_engine__refresh(pEngine);
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
    drte_engine__refresh(pEngine);
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

    drte_engine__refresh(pEngine);
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

    // TODO: Use an accelerated structure for this. Consider some kind of binary search.

    size_t lineIndex = 0;
    for (size_t i = 0; i < characterIndex; ++i) {
        if (pEngine->text[i] == '\n') {
            lineIndex += 1;
        }
    }

    return lineIndex;
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
    if (drte_engine__first_segment(pEngine, lineIndex, &segment)) {
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
        } while (drte_engine__next_segment(pEngine, &segment));
    }

    if (posX == 0) {
        int a; a = 1;
    }

    if (pPosXOut) *pPosXOut = posX;
    if (pPosYOut) *pPosYOut = posY;
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
        if (iLastLine >= lineCount && lineCount > 0) {
            iLastLine = lineCount - 1;
        } else {
            iLastLine = 0;
        }

        *pLastLineOut = iLastLine;
    }
}


void drte_engine_add_style_segment(drte_engine* pEngine, size_t iCharBeg, size_t iCharEnd, drte_style_token styleToken)
{
    if (pEngine == NULL) {
        return;
    }

    // Just add it to the end for now, but later on these should be sorted.
    if (pEngine->styleSegmentCount == pEngine->styleSegmentBufferSize) {
        size_t newBufferSize = (pEngine->styleSegmentBufferSize == 0) ? 16 : pEngine->styleSegmentBufferSize * 2;
        drte_style_segment* pNewStyleSegments = (drte_style_segment*)realloc(pEngine->pStyleSegments, newBufferSize * sizeof(*pNewStyleSegments));
        if (pNewStyleSegments == NULL) {
            return;
        }

        pEngine->styleSegmentBufferSize = newBufferSize;
        pEngine->pStyleSegments = pNewStyleSegments;
    }

    assert(pEngine->styleSegmentCount < pEngine->styleSegmentBufferSize);

    // TODO: Sort this based on iCharBeg.
    // TODO: Split and memory manage overlapping styles.

    pEngine->pStyleSegments[pEngine->styleSegmentCount].iCharBeg = iCharBeg;
    pEngine->pStyleSegments[pEngine->styleSegmentCount].iCharEnd = iCharEnd;
    pEngine->pStyleSegments[pEngine->styleSegmentCount].styleSlot = drte_engine__get_style_slot(pEngine, styleToken);
    pEngine->styleSegmentCount += 1;
}


void drte_engine_set_text(drte_engine* pEngine, const char* text)
{
    if (pEngine == NULL) {
        return;
    }

    size_t textLength = strlen(text);

    free(pEngine->text);
    pEngine->text = (char*)malloc(textLength + 1);     // +1 for null terminator.

    // We now need to copy over the text, however we need to skip past \r characters in order to normalize line endings
    // and keep everything simple.
          char* dst = pEngine->text;
    const char* src = text;
    while (*src != '\0')
    {
        if (*src != '\r') {
            *dst++ = *src;
        }

        src++;
    }
    *dst = '\0';

    pEngine->textLength = dst - pEngine->text;

    // A change in text means we need to refresh the layout.
    drte_engine__refresh(pEngine);

    // If the position of the cursor is past the last character we'll need to move it.
    if (drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->cursor) >= pEngine->textLength) {
        drte_engine_move_cursor_to_end_of_text(pEngine);
    }

    if (pEngine->onTextChanged) {
        pEngine->onTextChanged(pEngine);
    }

    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

        drte_engine__refresh(pEngine);
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


drgui_rect drte_engine_get_text_rect_relative_to_bounds(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return drgui_make_rect(0, 0, 0, 0);
    }

    drgui_rect rect;
    rect.left   = pEngine->innerOffsetX;
    rect.top    = pEngine->innerOffsetY;
    rect.right  = rect.left + pEngine->textBoundsWidth;
    rect.bottom = rect.top  + pEngine->textBoundsHeight;

    return rect;
}


void drte_engine_set_cursor_width(drte_engine* pEngine, float cursorWidth)
{
    if (pEngine == NULL) {
        return;
    }

    drgui_rect oldCursorRect = drte_engine_get_cursor_rect(pEngine);
    pEngine->cursorWidth = cursorWidth;
    if (pEngine->cursorWidth > 0 && pEngine->cursorWidth < 1) {
        pEngine->cursorWidth = 1;
    }

    drte_engine__on_dirty(pEngine, drgui_rect_union(oldCursorRect, drte_engine_get_cursor_rect(pEngine)));
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

        drte_engine__on_dirty(pEngine, drte_engine_get_cursor_rect(pEngine));
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

        drte_engine__on_dirty(pEngine, drte_engine_get_cursor_rect(pEngine));
    }
}

bool drte_engine_is_showing_cursor(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    return pEngine->isShowingCursor;
}

void drte_engine_get_cursor_position(drte_engine* pEngine, float* pPosXOut, float* pPosYOut)
{
    if (pEngine == NULL) {
        return;
    }

    drte_engine__get_marker_position_relative_to_container(pEngine, &pEngine->cursor, pPosXOut, pPosYOut);
}

drgui_rect drte_engine_get_cursor_rect(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return drgui_make_rect(0, 0, 0, 0);
    }


    float cursorPosX;
    float cursorPosY;
    drte_engine_get_cursor_position(pEngine, &cursorPosX, &cursorPosY);

    return drgui_make_rect(cursorPosX, cursorPosY, cursorPosX + pEngine->cursorWidth, cursorPosY + drte_engine_get_line_height(pEngine));
}

size_t drte_engine_get_cursor_line(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return drte_engine_get_character_line(pEngine, pEngine->cursor.iCharAbs);
}

size_t drte_engine_get_cursor_column(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    float posX;
    float posY;
    drte_engine_get_cursor_position(pEngine, &posX, &posY);

    return (unsigned int)((int)posX / pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.spaceWidth);
}

size_t drte_engine_get_cursor_character(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->cursor);
}

void drte_engine_move_cursor_to_point(drte_engine* pEngine, float posX, float posY)
{
    if (pEngine == NULL) {
        return;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    drte_engine__move_marker_to_point_relative_to_container(pEngine, &pEngine->cursor, posX, posY);

    if (drte_engine_is_in_selection_mode(pEngine)) {
        pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
    }

    if (iPrevChar != pEngine->cursor.iCharAbs) {
        drte_engine__on_cursor_move(pEngine);
        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: This can be optimized. Only redraw the previous line and the new cursor rectangle.
    }
}

bool drte_engine_move_cursor_left(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_left(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_right(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return false;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_right(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_up(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return false;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_up(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_down(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return false;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_down(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_y(drte_engine* pEngine, int amount)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return false;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_y(pEngine, &pEngine->cursor, amount)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_end_of_line(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return false;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_to_end_of_line(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_start_of_line(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return false;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_to_start_of_line(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_end_of_line_by_index(drte_engine* pEngine, size_t iLine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return false;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_to_end_of_line_by_index(pEngine, &pEngine->cursor, iLine)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_start_of_line_by_index(drte_engine* pEngine, size_t iLine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return false;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_to_start_of_line_by_index(pEngine, &pEngine->cursor, iLine)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_end_of_text(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return false;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_to_end_of_text(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }

        return true;
    }

    return false;
}

bool drte_engine_move_cursor_to_start_of_text(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return false;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_to_start_of_text(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }

        return true;
    }

    return false;
}

void drte_engine_move_cursor_to_start_of_selection(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return;
    }

    drte_marker* pSelectionMarker0;
    drte_marker* pSelectionMarker1;
    if (drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1))
    {
        pEngine->cursor = *pSelectionMarker0;
        pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);

        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
    }
}

void drte_engine_move_cursor_to_end_of_selection(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return;
    }

    drte_marker* pSelectionMarker0;
    drte_marker* pSelectionMarker1;
    if (drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1))
    {
        pEngine->cursor = *pSelectionMarker1;
        pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);

        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
    }
}

void drte_engine_move_cursor_to_character(drte_engine* pEngine, size_t characterIndex)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return;
    }

    size_t iPrevChar = pEngine->cursor.iCharAbs;
    if (drte_engine__move_marker_to_character(pEngine, &pEngine->cursor, characterIndex)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- TODO: Optimize this so that only the changed region is redrawn.
        }
    }
}

size_t drte_engine_move_cursor_to_end_of_word(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return 0;
    }

    size_t iChar = drte_engine_get_cursor_character(pEngine);
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

    

    drte_engine_move_cursor_to_character(pEngine, iChar);
    return iChar;
}

size_t drte_engine_move_cursor_to_start_of_next_word(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return 0;
    }

    size_t iChar = drte_engine_move_cursor_to_end_of_word(pEngine);
    while (pEngine->text[iChar] != '\0') {
        uint32_t c = pEngine->text[iChar];
        if (!dr_is_whitespace(c)) {
            break;
        }

        iChar += 1;
    }

    drte_engine_move_cursor_to_character(pEngine, iChar);
    return iChar;
}

size_t drte_engine_move_cursor_to_start_of_word(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return 0;
    }

    size_t iChar = drte_engine_get_cursor_character(pEngine);
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

    drte_engine_move_cursor_to_character(pEngine, iChar);
    return iChar;
}

size_t drte_engine_get_spaces_to_next_colum_from_cursor(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return 0;
    }

    const float tabWidth = drte_engine__get_tab_width(pEngine);

    float posX;
    float posY;
    drte_engine_get_cursor_position(pEngine, &posX, &posY);

    float tabColPosX = (posX + tabWidth) - ((size_t)posX % (size_t)tabWidth);

    return (size_t)(tabColPosX - posX) / pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.spaceWidth;
}

bool drte_engine_is_cursor_at_start_of_selection(drte_engine* pEngine)
{
    drte_marker* pSelectionMarker0;
    drte_marker* pSelectionMarker1;
    if (drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1)) {
        return &pEngine->cursor == pSelectionMarker0;
    }

    return false;
}

bool drte_engine_is_cursor_at_end_of_selection(drte_engine* pEngine)
{
    drte_marker* pSelectionMarker0;
    drte_marker* pSelectionMarker1;
    if (drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1)) {
        return &pEngine->cursor == pSelectionMarker1;
    }

    return false;
}

void drte_engine_swap_selection_markers(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    drte_marker* pSelectionMarker0;
    drte_marker* pSelectionMarker1;
    if (drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1))
    {
        size_t iPrevChar = pEngine->cursor.iCharAbs;

        drte_marker temp = *pSelectionMarker0;
        *pSelectionMarker0 = *pSelectionMarker1;
        *pSelectionMarker1 = temp;

        if (iPrevChar != pEngine->cursor.iCharAbs) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
        }
    }
}

void drte_engine_set_on_cursor_move(drte_engine* pEngine, drte_engine_on_cursor_move_proc proc)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->onCursorMove = proc;
}

bool drte_engine_insert_character(drte_engine* pEngine, unsigned int character, size_t insertIndex)
{
    if (pEngine == NULL) {
        return false;
    }

    // Transform '\r' to '\n'.
    if (character == '\r') {
        character = '\n';
    }


    // TODO: Add proper support for UTF-8.
    char* pOldText = pEngine->text;
    char* pNewText = (char*)malloc(pEngine->textLength + 1 + 1);   // +1 for the new character and +1 for the null terminator.

    if (insertIndex > 0) {
        memcpy(pNewText, pOldText, insertIndex);
    }

    pNewText[insertIndex] = (char)character;

    if (insertIndex < pEngine->textLength) {
        memcpy(pNewText + insertIndex + 1, pOldText + insertIndex, pEngine->textLength - insertIndex);
    }

    pEngine->textLength += 1;
    pEngine->text = pNewText;
    pNewText[pEngine->textLength] = '\0';

    free(pOldText);




    // The layout will have changed so it needs to be refreshed.
    drte_engine__refresh(pEngine);

    if (pEngine->onTextChanged) {
        pEngine->onTextChanged(pEngine);
    }

    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));

    return true;
}

bool drte_engine_insert_text(drte_engine* pEngine, const char* text, size_t insertIndex)
{
    if (pEngine == NULL || text == NULL) {
        return false;;
    }

    size_t newTextLength = strlen(text);
    if (newTextLength == 0) {
        return false;
    }


    // TODO: Add proper support for UTF-8.
    char* pOldText = pEngine->text;
    char* pNewText = (char*)malloc(pEngine->textLength + newTextLength + 1);   // +1 for the new character and +1 for the null terminator.

    if (insertIndex > 0) {
        memcpy(pNewText, pOldText, insertIndex);
    }


    // Replace \r\n with \n.
    {
        char* dst = pNewText + insertIndex;
        const char* src = text;
        size_t srcLen = newTextLength;
        while (*src != '\0' && srcLen > 0)
        {
            if (*src != '\r') {
                *dst++ = *src;
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


    // The layout will have changed so it needs to be refreshed.
    drte_engine__refresh(pEngine);

    if (pEngine->onTextChanged) {
        pEngine->onTextChanged(pEngine);
    }

    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));

    return true;
}

bool drte_engine_delete_text_range(drte_engine* pEngine, size_t iFirstCh, size_t iLastChPlus1)
{
    if (pEngine == NULL || iLastChPlus1 == iFirstCh) {
        return false;
    }

    if (iFirstCh > iLastChPlus1) {
        size_t temp = iFirstCh;
        iFirstCh = iLastChPlus1;
        iLastChPlus1 = temp;
    }


    size_t bytesToRemove = iLastChPlus1 - iFirstCh;
    if (bytesToRemove > 0)
    {
        memmove(pEngine->text + iFirstCh, pEngine->text + iLastChPlus1, pEngine->textLength - iLastChPlus1);
        pEngine->textLength -= bytesToRemove;
        pEngine->text[pEngine->textLength] = '\0';

        // The layout will have changed.
        drte_engine__refresh(pEngine);

        if (pEngine->onTextChanged) {
            pEngine->onTextChanged(pEngine);
        }

        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));

        return true;
    }

    return false;
}

bool drte_engine_insert_character_at_cursor(drte_engine* pEngine, unsigned int character)
{
    if (pEngine == NULL) {
        return false;
    }

    drte_engine__begin_dirty(pEngine);
    {
        drte_engine_insert_character(pEngine, character, pEngine->cursor.iCharAbs);
        drte_engine__move_marker_to_character(pEngine, &pEngine->cursor, pEngine->cursor.iCharAbs + 1);
    }
    drte_engine__end_dirty(pEngine);


    // The cursor's sticky position needs to be updated whenever the text is edited.
    drte_engine__update_marker_sticky_position(pEngine, &pEngine->cursor);


    drte_engine__on_cursor_move(pEngine);

    return true;
}

bool drte_engine_insert_text_at_cursor(drte_engine* pEngine, const char* text)
{
    if (pEngine == NULL || text == NULL) {
        return false;
    }

    drte_engine__begin_dirty(pEngine);
    {
        size_t cursorPos = drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->cursor);
        drte_engine_insert_text(pEngine, text, cursorPos);
        drte_engine__move_marker_to_character(pEngine, &pEngine->cursor, cursorPos + strlen(text));
    }
    drte_engine__end_dirty(pEngine);


    // The cursor's sticky position needs to be updated whenever the text is edited.
    drte_engine__update_marker_sticky_position(pEngine, &pEngine->cursor);

    drte_engine__on_cursor_move(pEngine);

    return true;
}

bool drte_engine_delete_character_to_left_of_cursor(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    // We just move the cursor to the left, and then delete the character to the right.
    if (drte_engine_move_cursor_left(pEngine)) {
        drte_engine_delete_character_to_right_of_cursor(pEngine);
        return true;
    }

    return false;
}

bool drte_engine_delete_character_to_right_of_cursor(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    size_t iAbsoluteMarkerChar = pEngine->cursor.iCharAbs;
    if (iAbsoluteMarkerChar < pEngine->textLength)
    {
        // TODO: Add proper support for UTF-8.
        memmove(pEngine->text + iAbsoluteMarkerChar, pEngine->text + iAbsoluteMarkerChar + 1, pEngine->textLength - iAbsoluteMarkerChar);
        pEngine->textLength -= 1;
        pEngine->text[pEngine->textLength] = '\0';



        // The layout will have changed.
        drte_engine__refresh(pEngine);
        drte_engine__move_marker_to_character(pEngine, &pEngine->cursor, iAbsoluteMarkerChar);

        if (pEngine->onTextChanged) {
            pEngine->onTextChanged(pEngine);
        }

        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));

        return true;
    }

    return false;
}

bool drte_engine_delete_selected_text(drte_engine* pEngine)
{
    // Don't do anything if nothing is selected.
    if (!drte_engine_is_anything_selected(pEngine)) {
        return false;
    }

    drte_marker* pSelectionMarker0 = &pEngine->selectionAnchor;
    drte_marker* pSelectionMarker1 = &pEngine->cursor;
    if (pSelectionMarker0->iCharAbs > pSelectionMarker1->iCharAbs)
    {
        drte_marker* temp = pSelectionMarker0;
        pSelectionMarker0 = pSelectionMarker1;
        pSelectionMarker1 = temp;
    }

    size_t iSelectionChar0 = pSelectionMarker0->iCharAbs;
    size_t iSelectionChar1 = pSelectionMarker1->iCharAbs;

    drte_engine__begin_dirty(pEngine);
    bool wasTextChanged = drte_engine_delete_text_range(pEngine, iSelectionChar0, iSelectionChar1);
    if (wasTextChanged)
    {
        // The marker needs to be updated based on the new layout.
        drte_engine__move_marker_to_character(pEngine, &pEngine->cursor, iSelectionChar0);

        // The cursor's sticky position also needs to be updated.
        drte_engine__update_marker_sticky_position(pEngine, &pEngine->cursor);

        drte_engine__on_cursor_move(pEngine);


        // Reset the selection marker.
        pEngine->selectionAnchor = pEngine->cursor;
        pEngine->isAnythingSelected = false;
    }

    drte_engine__end_dirty(pEngine);
    return wasTextChanged;
}


void drte_engine_enter_selection_mode(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    // If we've just entered selection mode and nothing is currently selected, we want to set the selection anchor to the current cursor position.
    if (!drte_engine_is_in_selection_mode(pEngine) && !pEngine->isAnythingSelected) {
        pEngine->selectionAnchor = pEngine->cursor;
    }

    pEngine->selectionModeCounter += 1;
}

void drte_engine_leave_selection_mode(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    if (pEngine->selectionModeCounter > 0) {
        pEngine->selectionModeCounter -= 1;
    }
}

bool drte_engine_is_in_selection_mode(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    return pEngine->selectionModeCounter > 0;
}

bool drte_engine_is_anything_selected(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    return pEngine->isAnythingSelected;
}

void drte_engine_deselect_all(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->isAnythingSelected = false;

    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
}

void drte_engine_select_all(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    drte_engine__move_marker_to_start_of_text(pEngine, &pEngine->selectionAnchor);
    drte_engine__move_marker_to_end_of_text(pEngine, &pEngine->cursor);

    pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);

    drte_engine__on_cursor_move(pEngine);
    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
}

void drte_engine_select(drte_engine* pEngine, size_t firstCharacter, size_t lastCharacter)
{
    if (pEngine == NULL) {
        return;
    }

    drte_engine__move_marker_to_character(pEngine, &pEngine->selectionAnchor, firstCharacter);
    drte_engine__move_marker_to_character(pEngine, &pEngine->cursor, lastCharacter);

    pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);

    drte_engine__on_cursor_move(pEngine);
    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));   // <-- Optimize this to only redraw the selected regions.
}

void drte_engine_select_word_under_cursor(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

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
}

size_t drte_engine_get_selected_text(drte_engine* pEngine, char* textOut, size_t textOutSize)
{
    if (pEngine == NULL || (textOut != NULL && textOutSize == 0)) {
        return 0;
    }

    if (!drte_engine_is_anything_selected(pEngine)) {
        return 0;
    }


    drte_marker* pSelectionMarker0;
    drte_marker* pSelectionMarker1;
    if (!drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1)) {
        return false;
    }

    size_t iSelectionChar0 = pSelectionMarker0->iCharAbs;
    size_t iSelectionChar1 = pSelectionMarker1->iCharAbs;

    size_t selectedTextLength = iSelectionChar1 - iSelectionChar0;

    if (textOut != NULL) {
        drgui__strncpy_s(textOut, textOutSize, pEngine->text + iSelectionChar0, selectedTextLength);
    }

    return selectedTextLength;
}

size_t drte_engine_get_selection_first_line(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    drte_marker* pSelectionMarker0;
    drte_marker* pSelectionMarker1;
    if (!drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1)) {
        return 0;
    }

    return drte_engine_get_character_line(pEngine, pSelectionMarker0->iCharAbs);
}

size_t drte_engine_get_selection_last_line(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    drte_marker* pSelectionMarker0;
    drte_marker* pSelectionMarker1;
    if (!drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1)) {
        return 0;
    }

    return drte_engine_get_character_line(pEngine, pSelectionMarker1->iCharAbs);
}

void drte_engine_move_selection_anchor_to_end_of_line(drte_engine* pEngine, size_t iLine)
{
    if (pEngine == NULL) {
        return;
    }

    drte_engine__move_marker_to_end_of_line_by_index(pEngine, &pEngine->selectionAnchor, iLine);
    pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
}

void drte_engine_move_selection_anchor_to_start_of_line(drte_engine* pEngine, size_t iLine)
{
    if (pEngine == NULL) {
        return;
    }

    drte_engine__move_marker_to_start_of_line_by_index(pEngine, &pEngine->selectionAnchor, iLine);
    pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
}

size_t drte_engine_get_selection_anchor_line(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return drte_engine_get_character_line(pEngine, pEngine->selectionAnchor.iCharAbs);
}



bool drte_engine_prepare_undo_point(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    // If we have a previously prepared state we'll need to clear it.
    if (pEngine->preparedState.text != NULL) {
        free(pEngine->preparedState.text);
    }

    pEngine->preparedState.text = (char*)malloc(pEngine->textLength + 1);
    drgui__strcpy_s(pEngine->preparedState.text, pEngine->textLength + 1, (pEngine->text != NULL) ? pEngine->text : "");

    pEngine->preparedState.cursorPos          = drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->cursor);
    pEngine->preparedState.selectionAnchorPos = drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->selectionAnchor);
    pEngine->preparedState.isAnythingSelected = pEngine->isAnythingSelected;

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
    currentState.text               = pEngine->text;
    currentState.cursorPos          = drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->cursor);
    currentState.selectionAnchorPos = drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->selectionAnchor);
    currentState.isAnythingSelected = pEngine->isAnythingSelected;

    drte_engine_undo_state undoState;
    if (!drte_engine__diff_states(&pEngine->preparedState, &currentState, &undoState)) {
        return false;
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

    // TODO: Accelerate this.

    size_t lineCount = 1;
    for (size_t i = 0; i < pEngine->textLength; ++i) {
        if (pEngine->text[i] == '\n') {
            lineCount += 1;
        }
    }

    return lineCount;
}

size_t drte_engine_get_visible_line_count(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return 0;
    }

    return (size_t)(pEngine->containerHeight / drte_engine_get_line_height(pEngine)) + 1;
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

    // TODO: Accelerate this:
    //return pEngine->pLines[iLine];

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
    for (size_t iLine = iLineTop; iLine <= iLineBottom; ++iLine) {
        float lineWidth = 0;

        drte_segment segment;
        if (drte_engine__first_segment(pEngine, iLine, &segment)) {
            do
            {
                if (segment.posX > pEngine->containerWidth) {
                    break;  // All remaining segments on this line (including this one) is clipped. Go to the next line.
                }

                lineWidth += segment.width;

                // Don't draw segments to the left of the container.
                if (linePosX + segment.posX + segment.width < 0) {
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
            } while (drte_engine__next_segment(pEngine, &segment));
        }

        // OPTIMIZE: Don't call drte_engine__first_segment() if the line terminated naturally. Instead just continue where we
        //           left off. Consider drte_engine__first_segment_by_character().
        //if (segment.isAtEnd) {
        //    Optimize me.
        //}

        // The part after the end of the line needs to be drawn.
        float lineRight = linePosX + lineWidth;
        if (lineRight < pEngine->containerWidth) {
            drte_style_token bgStyleToken = pEngine->styles[pEngine->defaultStyleSlot].styleToken;
            if (iLine == drte_engine_get_cursor_line(pEngine)) {
                bgStyleToken = pEngine->styles[pEngine->activeLineStyleSlot].styleToken;
            }

            if (pEngine->onPaintRect && bgStyleToken != 0) {
                pEngine->onPaintRect(pEngine, bgStyleToken, drgui_make_rect(lineRight, linePosY, pEngine->containerWidth, linePosY + lineHeight), pElement, pPaintData);
            }
        }

        linePosY += lineHeight;
    }

    // The cursor.
    if (pEngine->isShowingCursor && pEngine->isCursorBlinkOn && pEngine->styles[pEngine->cursorStyleSlot].styleToken != 0) {
        pEngine->onPaintRect(pEngine, pEngine->styles[pEngine->cursorStyleSlot].styleToken, drte_engine_get_cursor_rect(pEngine), pElement, pPaintData);
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

        drte_engine__on_dirty(pEngine, drte_engine_get_cursor_rect(pEngine));
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
        snprintf(iLineStr, sizeof(iLineStr), "%d", iLine+1);

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

    size_t cursorPos = drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->cursor);
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

    size_t cursorPos = drte_engine__get_marker_absolute_char_index(pEngine, &pEngine->cursor);

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




void drte_engine__refresh(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    // TODO: DELETE THIS ENTIRE FUNCTION. RESIZE LINES AS TEXT IS INSERTED AND DELETED.

    // All we are doing here is resizing the text bounds. This will be optimized later.
    float maxLineWidth = 0;

    size_t lineCount = drte_engine_get_line_count(pEngine);
    for (size_t iLine = 0; iLine < lineCount; ++iLine) {
        float thisLineWidth = 0;

        drte_segment segment;
        if (drte_engine__first_segment(pEngine, iLine, &segment)) {
            do
            {
                thisLineWidth += segment.width;
            } while (drte_engine__next_segment(pEngine, &segment));
        }

        if (maxLineWidth < thisLineWidth) {
            maxLineWidth = thisLineWidth;
        }
    }

    pEngine->textBoundsWidth  = maxLineWidth;
    pEngine->textBoundsHeight = lineCount * drte_engine_get_line_height(pEngine);
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

    drgui_rect textRect = drte_engine_get_text_rect_relative_to_bounds(pEngine);

    float inputPosXRelativeToText = inputPosX - textRect.left;
    float inputPosYRelativeToText = inputPosY - textRect.top;
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
    if (drte_engine__first_segment(pEngine, (size_t)iLine, &segment)) {
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
        } while (drte_engine__next_segment(pEngine, &segment));

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

    return drte_engine__move_marker_to_character(pEngine, pMarker, drte_engine_get_line_last_character(pEngine, drte_engine_get_character_line(pEngine, pMarker->iCharAbs)));
}

bool drte_engine__move_marker_to_start_of_line(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    return drte_engine__move_marker_to_character(pEngine, pMarker, drte_engine_get_line_first_character(pEngine, drte_engine_get_character_line(pEngine, pMarker->iCharAbs)));
}

bool drte_engine__move_marker_to_end_of_line_by_index(drte_engine* pEngine, drte_marker* pMarker, size_t iLine)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    return drte_engine__move_marker_to_character(pEngine, pMarker, drte_engine_get_line_last_character(pEngine, iLine));
}

bool drte_engine__move_marker_to_start_of_line_by_index(drte_engine* pEngine, drte_marker* pMarker, size_t iLine)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    return drte_engine__move_marker_to_character(pEngine, pMarker, drte_engine_get_line_first_character(pEngine, iLine));
}

bool drte_engine__move_marker_to_end_of_text(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    return drte_engine__move_marker_to_character(pEngine, pMarker, pEngine->textLength);    // <-- Don't subtract 1 here because otherwise we'll end up on the _left_ of the last character.
}

bool drte_engine__move_marker_to_start_of_text(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    return drte_engine__move_marker_to_character(pEngine, pMarker, 0);
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


bool drte_engine__has_spacing_between_selection_markers(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    return pEngine->cursor.iCharAbs || pEngine->selectionAnchor.iCharAbs;
}

bool drte_engine__get_selection_markers(drte_engine* pEngine, drte_marker** ppSelectionMarker0Out, drte_marker** ppSelectionMarker1Out)
{
    bool result = false;

    drte_marker* pSelectionMarker0 = NULL;
    drte_marker* pSelectionMarker1 = NULL;
    if (pEngine != NULL && drte_engine_is_anything_selected(pEngine))
    {
        pSelectionMarker0 = &pEngine->selectionAnchor;
        pSelectionMarker1 = &pEngine->cursor;
        if (pSelectionMarker0->iCharAbs > pSelectionMarker1->iCharAbs)
        {
            drte_marker* temp = pSelectionMarker0;
            pSelectionMarker0 = pSelectionMarker1;
            pSelectionMarker1 = temp;
        }

        result = true;
    }

    if (ppSelectionMarker0Out) {
        *ppSelectionMarker0Out = pSelectionMarker0;
    }
    if (ppSelectionMarker1Out) {
        *ppSelectionMarker1Out = pSelectionMarker1;
    }

    return result;
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

    size_t newTextLen = currLen - sameChCountStart - sameChCountEnd;
    pUndoStateOut->newText = (char*)malloc(newTextLen + 1);
    drgui__strncpy_s(pUndoStateOut->newText, newTextLen + 1, currText + sameChCountStart, newTextLen);

    return true;
}

void drte_engine__uninit_undo_state(drte_engine_undo_state* pUndoState)
{
    if (pUndoState == NULL) {
        return;
    }

    free(pUndoState->oldText);
    pUndoState->oldText = NULL;

    free(pUndoState->newText);
    pUndoState->newText = NULL;
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

    size_t iFirstCh     = pUndoState->diffPos;
    size_t iLastChPlus1 = pUndoState->diffPos + strlen(pUndoState->newText);
    size_t bytesToRemove = iLastChPlus1 - iFirstCh;
    if (bytesToRemove > 0)
    {
        memmove(pEngine->text + iFirstCh, pEngine->text + iLastChPlus1, pEngine->textLength - iLastChPlus1);
        pEngine->textLength -= bytesToRemove;
        pEngine->text[pEngine->textLength] = '\0';
    }

    // TODO: This needs improving because it results in multiple onTextChanged and onDirty events being posted.

    // Insert the old text.
    drte_engine_insert_text(pEngine, pUndoState->oldText, pUndoState->diffPos);


    // The layout will have changed so it needs to be refreshed.
    drte_engine__refresh(pEngine);


    // Markers needs to be updated after refreshing the layout.
    drte_engine__move_marker_to_character(pEngine, &pEngine->cursor, pUndoState->oldState.cursorPos);
    drte_engine__move_marker_to_character(pEngine, &pEngine->selectionAnchor, pUndoState->oldState.selectionAnchorPos);

    // The cursor's sticky position needs to be updated whenever the text is edited.
    drte_engine__update_marker_sticky_position(pEngine, &pEngine->cursor);

    // Ensure we mark the text as selected if appropriate.
    pEngine->isAnythingSelected = pUndoState->oldState.isAnythingSelected;


    if (pEngine->onTextChanged) {
        pEngine->onTextChanged(pEngine);
    }

    drte_engine__on_cursor_move(pEngine);

    if (pEngine->onDirty) {
        pEngine->onDirty(pEngine, drte_engine__local_rect(pEngine));
    }
}

void drte_engine__apply_redo_state(drte_engine* pEngine, drte_engine_undo_state* pUndoState)
{
    if (pEngine == NULL || pUndoState == NULL) {
        return;
    }

    // An redo is just the opposite of an undo. We want to remove the old text and replace it with the new text.

    size_t iFirstCh     = pUndoState->diffPos;
    size_t iLastChPlus1 = pUndoState->diffPos + strlen(pUndoState->oldText);
    size_t bytesToRemove = iLastChPlus1 - iFirstCh;
    if (bytesToRemove > 0)
    {
        memmove(pEngine->text + iFirstCh, pEngine->text + iLastChPlus1, pEngine->textLength - iLastChPlus1);
        pEngine->textLength -= bytesToRemove;
        pEngine->text[pEngine->textLength] = '\0';
    }

    // TODO: This needs improving because it results in multiple onTextChanged and onDirty events being posted.

    // Insert the new text.
    drte_engine_insert_text(pEngine, pUndoState->newText, pUndoState->diffPos);


    // The layout will have changed so it needs to be refreshed.
    drte_engine__refresh(pEngine);


    // Markers needs to be updated after refreshing the layout.
    drte_engine__move_marker_to_character(pEngine, &pEngine->cursor, pUndoState->newState.cursorPos);
    drte_engine__move_marker_to_character(pEngine, &pEngine->selectionAnchor, pUndoState->newState.selectionAnchorPos);

    // The cursor's sticky position needs to be updated whenever the text is edited.
    drte_engine__update_marker_sticky_position(pEngine, &pEngine->cursor);

    // Ensure we mark the text as selected if appropriate.
    pEngine->isAnythingSelected = pUndoState->newState.isAnythingSelected;


    if (pEngine->onTextChanged) {
        pEngine->onTextChanged(pEngine);
    }

    drte_engine__on_cursor_move(pEngine);

    if (pEngine->onDirty) {
        pEngine->onDirty(pEngine, drte_engine__local_rect(pEngine));
    }
}


drgui_rect drte_engine__local_rect(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return drgui_make_rect(0, 0, 0, 0);
    }

    return drgui_make_rect(0, 0, pEngine->containerWidth, pEngine->containerHeight);
}


void drte_engine__on_cursor_move(drte_engine* pEngine)
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

    drte_engine__on_dirty(pEngine, drte_engine_get_cursor_rect(pEngine));
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
        if (pEngine->onDirty) {
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

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

#if 0
typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} drte_color;

static drte_color drte_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    drte_color color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

static drte_color drte_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return drte_rgba(r, g, b, 255);
}
#endif

typedef struct
{
    drte_style_token styleToken;
    drte_font_metrics fontMetrics;
} drte_style;

typedef struct
{
    /// A pointer to the start of the string. This is NOT null terminated.
    const char* text;

    /// The length of the string, in bytes.
    size_t textLength;


    // The style slot to use for the foreground of this run.
    uint8_t fgStyleSlot;

    // The style slot to use for the background of this run.
    uint8_t bgStyleSlot;


    /// The position to draw the text on the x axis.
    float posX;

    /// The position to draw the text on the y axis.
    float posY;

    /// The width of the run.
    float width;

    /// The height of the run.
    float height;


    // PROPERTIES BELOW ARE FOR INTERNAL USE ONLY

    /// Index of the line the run is placed on. For runs that are new line characters, this will represent the number of lines that came before it. For
    /// example, if this run represents the new-line character for the first line, this will be 0 and so on.
    size_t iLine;

    /// Index in the main text string of the first character of the run.
    size_t iChar;

    /// Index in the main text string of the character just past the last character in the run.
    size_t iCharEnd;

} drte_text_run;


typedef void (* drte_engine_on_measure_string_proc)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textLength, float* pWidthOut, float* pHeightOut);
typedef void (* drte_engine_on_get_cursor_position_from_point)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut);
typedef void (* drte_engine_on_get_cursor_position_from_char)(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t characterIndex, float* pTextCursorPosXOut);

typedef void (* drte_engine_on_paint_text_proc)        (drte_engine* pEngine, drte_style_token styleTokenFG, drte_style_token styleTokenBG, drte_text_run* pRun, drgui_element* pElement, void* pPaintData);
typedef void (* drte_engine_on_paint_rect_proc)        (drte_engine* pEngine, drte_style_token styleToken, drgui_rect rect, drgui_element* pElement, void* pPaintData);
typedef void (* drte_engine_on_cursor_move_proc)       (drte_engine* pEngine);
typedef void (* drte_engine_on_dirty_proc)             (drte_engine* pEngine, drgui_rect rect);
typedef void (* drte_engine_on_text_changed_proc)      (drte_engine* pEngine);
typedef void (* drte_engine_on_undo_point_changed_proc)(drte_engine* pEngine, unsigned int iUndoPoint);

typedef struct
{
    /// The index of the run within the line the marker is positioned on.
    size_t iRun;

    /// The index of the character within the run the marker is positioned to the left of.
    size_t iChar;

    /// The position on the x axis, relative to the x position of the run.
    float relativePosX;

    /// The absolute position on the x axis to place the marker when moving up and down lines. Note that this is not relative
    /// to the run, but rather the line. This will be updated when the marker is moved left and right.
    float absoluteSickyPosX;

} drte_marker;

typedef struct
{
    // The index of the first character in the segment.
    size_t iCharBeg;

    // The index of the last character in the segment.
    size_t iCharEnd;

    // The style to apply to the segment.
    drte_style_token styleToken;

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


    // The function to call when a string needs to be measured.
    drte_engine_on_measure_string_proc onMeasureString;

    // The function to call when the position of the cursor needs to be retrieved based on a pixel position within that string.
    drte_engine_on_get_cursor_position_from_point onGetCursorPositionFromPoint;

    // The function to call when the position of the cursor needs to be retrieved based on a character at a specific index.
    drte_engine_on_get_cursor_position_from_char onGetCursorPositionFromChar;


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
    bool isAnythingSelected;


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


    /// A pointer to the buffer containing details about every run in the layout.
    drte_text_run* pRuns;

    /// The number of runs in <pRuns>.
    size_t runCount;

    /// The size of the <pRuns> buffer in drte_text_run's. This is used to determine whether or not the buffer
    /// needs to be reallocated upon adding a new run.
    size_t runBufferSize;


    // Application-defined data.
    void* pUserData;
};

/// Structure containing information about a line. This is used by first_line() and next_line().
typedef struct
{
    /// The index of the line.
    size_t index;

    /// The position of the line on the y axis.
    float posY;

    /// The height of the line.
    float height;

    /// The index of the first run on the line.
    size_t iFirstRun;

    /// The index of the last run on the line.
    size_t iLastRun;

} drte_engine_line;



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

/// Refreshes the cursor and selection marker positions.
void drte_engine_refresh_markers(drte_engine* pEngine);


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

/// Retrieves the number of lines that can fit on the visible portion of the layout, starting from the given line.
///
/// @remarks
///     Use this for controlling the page size for scrollbars.
size_t drte_engine_get_visible_line_count_starting_at(drte_engine* pEngine, size_t iFirstLine);

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

bool drte_is_symbol_or_whitespace(uint32_t utf32)
{
    return (utf32 < '0') || (utf32 >= ':' && utf32 < 'A') || (utf32 >= '[' && utf32 < 'a') || (utf32 > '{'); 
}


/// Performs a complete refresh of the given text engine.
///
/// @remarks
///     This will delete every run and re-create them.
void drte_engine__refresh(drte_engine* pEngine);

/// Appends a text run to the list of runs in the given text engine.
void drte_engine__push_text_run(drte_engine* pEngine, drte_text_run* pRun);

/// Clears the internal list of text runs.
void drte_engine__clear_text_runs(drte_engine* pEngine);

/// Helper for determine whether or not the given text run is whitespace.
bool drte_engine__is_text_run_whitespace(drte_engine* pEngine, drte_text_run* pRun);

/// Helper for calculating the width of a tab.
float drte_engine__get_tab_width(drte_engine* pEngine);


/// Finds the line that's closest to the given point relative to the text.
bool drte_engine__find_closest_line_to_point(drte_engine* pEngine, float inputPosYRelativeToText, size_t* pFirstRunIndexOnLineOut, size_t* pLastRunIndexOnLinePlus1Out);

/// Finds the run that's closest to the given point relative to the text.
bool drte_engine__find_closest_run_to_point(drte_engine* pEngine, float inputPosXRelativeToText, float inputPosYRelativeToText, size_t* pRunIndexOut);

/// Retrieves some basic information about a line, namely the index of the last run on the line, and the line's height.
bool drte_engine__find_line_info(drte_engine* pEngine, size_t iFirstRunOnLine, size_t* pLastRunIndexOnLinePlus1Out, float* pLineHeightOut);

/// Retrieves some basic information about a line by it's index.
bool drte_engine__find_line_info_by_index(drte_engine* pEngine, size_t iLine, drgui_rect* pRectOut, size_t* pFirstRunIndexOut, size_t* pLastRunIndexPlus1Out);

/// Finds the last run on the line that the given run is sitting on.
bool drte_engine__find_last_run_on_line_starting_from_run(drte_engine* pEngine, size_t iRun, size_t* pLastRunIndexOnLineOut);

/// Finds the first run on the line that the given run is sitting on.
bool drte_engine__find_first_run_on_line_starting_from_run(drte_engine* pEngine, size_t iRun, size_t* pFirstRunIndexOnLineOut);

/// Finds the run containing the character at the given index.
bool drte_engine__find_run_at_character(drte_engine* pEngine, size_t iChar, size_t* pRunIndexOut);


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

/// Moves the given marker to the last character of the given run.
bool drte_engine__move_marker_to_last_character_of_run(drte_engine* pEngine, drte_marker* pMarker, size_t iRun);

/// Moves the given marker to the first character of the given run.
bool drte_engine__move_marker_to_first_character_of_run(drte_engine* pEngine, drte_marker* pMarker, size_t iRun);

/// Moves the given marker to the last character of the previous run.
bool drte_engine__move_marker_to_last_character_of_prev_run(drte_engine* pEngine, drte_marker* pMarker);

/// Moves the given marker to the first character of the next run.
bool drte_engine__move_marker_to_first_character_of_next_run(drte_engine* pEngine, drte_marker* pMarker);

/// Moves the given marker to the character at the given position.
bool drte_engine__move_marker_to_character(drte_engine* pEngine, drte_marker* pMarker, size_t iChar);


/// Updates the relative position of the given marker.
///
/// @remarks
///     This assumes the iRun and iChar properties are valid.
bool drte_engine__update_marker_relative_position(drte_engine* pEngine, drte_marker* pMarker);

/// Updates the sticky position of the given marker.
void drte_engine__update_marker_sticky_position(drte_engine* pEngine, drte_marker* pMarker);


/// Retrieves the index of the character the given marker is located at.
size_t drte_engine__get_marker_absolute_char_index(drte_engine* pEngine, drte_marker* pMarker);


/// Helper function for determining whether or not there is any spacing between the selection markers.
bool drte_engine__has_spacing_between_selection_markers(drte_engine* pEngine);

/// Splits the given run into sub-runs based on the current selection rectangle. Returns the sub-run count.
size_t drte_engine__split_text_run_by_selection(drte_engine* pEngine, drte_text_run* pRunToSplit, drte_text_run pSubRunsOut[3]);


/// Retrieves pointers to the selection markers in the correct order.
bool drte_engine__get_selection_markers(drte_engine* pEngine, drte_marker** ppSelectionMarker0Out, drte_marker** ppSelectionMarker1Out);


/// Retrieves an iterator to the first line in the text engine.
bool drte_engine__first_line(drte_engine* pEngine, drte_engine_line* pLine);

/// Retrieves an iterator to the next line in the text engine.
bool drte_engine__next_line(drte_engine* pEngine, drte_engine_line* pLine);


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

    free(pEngine->pRuns);
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
        drte_engine__refresh(pEngine);
        return true;
    }


    // If we get here it means the style has not previously been registered. We don't actually need to refresh the engine
    // here because nothing will actually be using the style token yet.
    if (pEngine->styleCount == 255) {
        return false;   // Too many styles. The 256'th slot (index 255) is used as the error indicator.
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
    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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
        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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
    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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
    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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
    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    drgui_rect lineRect = drgui_make_rect(0, 0, 0, 0);

    if (pEngine->runCount > 0) {
        drte_engine__find_line_info_by_index(pEngine, pEngine->pRuns[pEngine->cursor.iRun].iLine, &lineRect, NULL, NULL);
    } else {
        lineRect.bottom = (float)pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.lineHeight;
    }



    float cursorPosX;
    float cursorPosY;
    drte_engine_get_cursor_position(pEngine, &cursorPosX, &cursorPosY);

    return drgui_make_rect(cursorPosX, cursorPosY, cursorPosX + pEngine->cursorWidth, cursorPosY + (lineRect.bottom - lineRect.top));
}

size_t drte_engine_get_cursor_line(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->runCount == 0) {
        return 0;
    }

    return pEngine->pRuns[pEngine->cursor.iRun].iLine;
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

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    drte_engine__move_marker_to_point_relative_to_container(pEngine, &pEngine->cursor, posX, posY);

    if (drte_engine_is_in_selection_mode(pEngine)) {
        pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
    }

    if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
        drte_engine__on_cursor_move(pEngine);
        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
    }
}

bool drte_engine_move_cursor_left(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_left(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_right(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_up(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_down(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_y(pEngine, &pEngine->cursor, amount)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_to_end_of_line(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_to_start_of_line(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_to_end_of_line_by_index(pEngine, &pEngine->cursor, iLine)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_to_start_of_line_by_index(pEngine, &pEngine->cursor, iLine)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_to_end_of_text(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_to_start_of_text(pEngine, &pEngine->cursor)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

        drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
    }
}

void drte_engine_move_cursor_to_character(drte_engine* pEngine, size_t characterIndex)
{
    if (pEngine == NULL || pEngine->text == NULL) {
        return;
    }

    size_t iRunOld  = pEngine->cursor.iRun;
    size_t iCharOld = pEngine->cursor.iChar;
    if (drte_engine__move_marker_to_character(pEngine, &pEngine->cursor, characterIndex)) {
        if (drte_engine_is_in_selection_mode(pEngine)) {
            pEngine->isAnythingSelected = drte_engine__has_spacing_between_selection_markers(pEngine);
        }

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
            drte_engine__on_cursor_move(pEngine);
            drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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
        size_t iRunOld  = pEngine->cursor.iRun;
        size_t iCharOld = pEngine->cursor.iChar;

        drte_marker temp = *pSelectionMarker0;
        *pSelectionMarker0 = *pSelectionMarker1;
        *pSelectionMarker1 = temp;

        if (iRunOld != pEngine->cursor.iRun || iCharOld != pEngine->cursor.iChar) {
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

void drte_engine_refresh_markers(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->pRuns == NULL) {
        return;
    }

    // Cursor.
    drte_text_run* pRun = pEngine->pRuns + pEngine->cursor.iRun;
    if (pEngine->onGetCursorPositionFromChar) {
        pEngine->onGetCursorPositionFromChar(pEngine, pEngine->styles[pRun->fgStyleSlot].styleToken, pEngine->text + pRun->iChar, pEngine->cursor.iChar, &pEngine->cursor.relativePosX);
    }

    pRun = pEngine->pRuns + pEngine->selectionAnchor.iRun;
    if (pEngine->onGetCursorPositionFromChar) {
        pEngine->onGetCursorPositionFromChar(pEngine, pEngine->styles[pRun->fgStyleSlot].styleToken, pEngine->text + pRun->iChar, pEngine->selectionAnchor.iChar, &pEngine->selectionAnchor.relativePosX);
    }
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

    size_t iAbsoluteMarkerChar = 0;

    drte_text_run* pRun = pEngine->pRuns + pEngine->cursor.iRun;
    if (pEngine->runCount > 0 && pRun != NULL) {
        iAbsoluteMarkerChar = pRun->iChar + pEngine->cursor.iChar;
    }

    drte_engine__begin_dirty(pEngine);
    {
        drte_engine_insert_character(pEngine, character, iAbsoluteMarkerChar);
        drte_engine__move_marker_to_character(pEngine, &pEngine->cursor, iAbsoluteMarkerChar + 1);
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
    if (pEngine == NULL || pEngine->runCount == 0) {
        return false;
    }

    drte_text_run* pRun = pEngine->pRuns + pEngine->cursor.iRun;
    size_t iAbsoluteMarkerChar = pRun->iChar + pEngine->cursor.iChar;

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
    if (pEngine->pRuns[pSelectionMarker0->iRun].iChar + pSelectionMarker0->iChar > pEngine->pRuns[pSelectionMarker1->iRun].iChar + pSelectionMarker1->iChar)
    {
        drte_marker* temp = pSelectionMarker0;
        pSelectionMarker0 = pSelectionMarker1;
        pSelectionMarker1 = temp;
    }

    size_t iSelectionChar0 = pEngine->pRuns[pSelectionMarker0->iRun].iChar + pSelectionMarker0->iChar;
    size_t iSelectionChar1 = pEngine->pRuns[pSelectionMarker1->iRun].iChar + pSelectionMarker1->iChar;

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
    drte_engine__on_dirty(pEngine, drte_engine__local_rect(pEngine));
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

    size_t iSelectionChar0 = pEngine->pRuns[pSelectionMarker0->iRun].iChar + pSelectionMarker0->iChar;
    size_t iSelectionChar1 = pEngine->pRuns[pSelectionMarker1->iRun].iChar + pSelectionMarker1->iChar;

    size_t selectedTextLength = iSelectionChar1 - iSelectionChar0;

    if (textOut != NULL) {
        drgui__strncpy_s(textOut, textOutSize, pEngine->text + iSelectionChar0, selectedTextLength);
    }

    return selectedTextLength;
}

size_t drte_engine_get_selection_first_line(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->runCount == 0) {
        return 0;
    }

    drte_marker* pSelectionMarker0;
    drte_marker* pSelectionMarker1;
    if (!drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1)) {
        return 0;
    }

    return pEngine->pRuns[pSelectionMarker0->iRun].iLine;
}

size_t drte_engine_get_selection_last_line(drte_engine* pEngine)
{
    if (pEngine == NULL || pEngine->runCount == 0) {
        return 0;
    }

    drte_marker* pSelectionMarker0;
    drte_marker* pSelectionMarker1;
    if (!drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1)) {
        return 0;
    }

    return pEngine->pRuns[pSelectionMarker1->iRun].iLine;
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
    if (pEngine == NULL || pEngine->runCount == 0) {
        return 0;
    }

    return pEngine->pRuns[pEngine->selectionAnchor.iRun].iLine;
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
    if (pEngine == NULL || pEngine->runCount == 0) {
        return 0;
    }

    return pEngine->pRuns[pEngine->runCount - 1].iLine + 1;
}

size_t drte_engine_get_visible_line_count_starting_at(drte_engine* pEngine, size_t iFirstLine)
{
    if (pEngine == NULL || pEngine->runCount == 0) {
        return 0;
    }

    unsigned int count = 0;
    float lastLineBottom = 0;

    // First thing we do is find the first line.
    unsigned int iLine = 0;
    drte_engine_line line;
    if (drte_engine__first_line(pEngine, &line))
    {
        do
        {
            if (iLine >= iFirstLine) {
                break;
            }

            iLine += 1;
        } while (drte_engine__next_line(pEngine, &line));


        // At this point we are at the first line and we need to start counting.
        do
        {
            if (line.posY + pEngine->innerOffsetY >= pEngine->containerHeight) {
                break;
            }

            count += 1;
            lastLineBottom = line.posY + line.height;

        } while (drte_engine__next_line(pEngine, &line));
    }


    // At this point there may be some empty space below the last line, in which case we use the line height of the default font to fill
    // out the remaining space.
    if (lastLineBottom + pEngine->innerOffsetY < pEngine->containerHeight) {
        count += (unsigned int)((pEngine->containerHeight - (lastLineBottom + pEngine->innerOffsetY)) / pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.lineHeight);
    }



    if (count == 0) {
        return 1;
    }

    return count;
}

float drte_engine_get_line_pos_y(drte_engine* pEngine, size_t iLine)
{
    drgui_rect lineRect;
    if (!drte_engine__find_line_info_by_index(pEngine, iLine, &lineRect, NULL, NULL)) {
        return 0;
    }

    return lineRect.top;
}

size_t drte_engine_get_line_at_pos_y(drte_engine* pEngine, float posY)
{
    if (pEngine == NULL || pEngine->runCount == 0) {
        return 0;
    }

    drgui_rect textRect = drte_engine_get_text_rect_relative_to_bounds(pEngine);

    size_t iRun;

    float inputPosYRelativeToText = posY - textRect.top;
    if (!drte_engine__find_closest_run_to_point(pEngine, 0, inputPosYRelativeToText, &iRun)) {
        return 0;
    }

    return pEngine->pRuns[iRun].iLine;
}

size_t drte_engine_get_line_first_character(drte_engine* pEngine, size_t iLine)
{
    if (pEngine == NULL || pEngine->runCount == 0) {
        return 0;
    }

    size_t firstRunIndex0;
    size_t lastRunIndexPlus1;
    if (drte_engine__find_line_info_by_index(pEngine, iLine, NULL, &firstRunIndex0, &lastRunIndexPlus1)) {
        return pEngine->pRuns[firstRunIndex0].iChar;
    }

    return 0;
}

size_t drte_engine_get_line_last_character(drte_engine* pEngine, size_t iLine)
{
    if (pEngine == NULL || pEngine->runCount == 0) {
        return 0;
    }

    size_t firstRunIndex0;
    size_t lastRunIndexPlus1;
    if (drte_engine__find_line_info_by_index(pEngine, iLine, NULL, &firstRunIndex0, &lastRunIndexPlus1)) {
        size_t charEnd = pEngine->pRuns[lastRunIndexPlus1 - 1].iCharEnd;
        if (charEnd > 0) {
            charEnd -= 1;
        }

        return charEnd;
    }

    return 0;
}

void drte_engine_get_line_character_range(drte_engine* pEngine, size_t iLine, size_t* pCharStartOut, size_t* pCharEndOut)
{
    if (pEngine == NULL || pEngine->runCount == 0) {
        return;
    }

    size_t charStart = 0;
    size_t charEnd = 0;

    size_t firstRunIndex0;
    size_t lastRunIndexPlus1;
    if (drte_engine__find_line_info_by_index(pEngine, iLine, NULL, &firstRunIndex0, &lastRunIndexPlus1)) {
        charStart = pEngine->pRuns[firstRunIndex0].iChar;
        charEnd   = pEngine->pRuns[lastRunIndexPlus1 - 1].iCharEnd;
        if (charEnd > 0) {
            charEnd -= 1;
        }
    }

    if (pCharStartOut) {
        *pCharStartOut = charStart;
    }
    if (pCharEndOut) {
        *pCharEndOut = charEnd;
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


    // The position of each run will be relative to the text bounds. We want to make it relative to the container bounds.
    drgui_rect textRect = drte_engine_get_text_rect_relative_to_bounds(pEngine);

    // We draw a rectangle above and below the text rectangle. The main text rectangle will be drawn by iterating over each visible run.
    drgui_rect rectTop    = drgui_make_rect(0, 0, pEngine->containerWidth, textRect.top);
    drgui_rect rectBottom = drgui_make_rect(0, textRect.bottom, pEngine->containerWidth, pEngine->containerHeight);

    if (rectTop.bottom > rect.top) {
        pEngine->onPaintRect(pEngine, pEngine->styles[pEngine->defaultStyleSlot].styleToken, rectTop, pElement, pPaintData);
    }

    if (rectBottom.top < rect.bottom) {
        pEngine->onPaintRect(pEngine, pEngine->styles[pEngine->defaultStyleSlot].styleToken, rectBottom, pElement, pPaintData);
    }


    // We draw line-by-line, starting from the first visible line.
    drte_engine_line line;
    if (drte_engine__first_line(pEngine, &line))
    {
        do
        {
            float lineTop    = line.posY + textRect.top;
            float lineBottom = lineTop + line.height;

            if (lineTop < rect.bottom)
            {
                if (lineBottom > rect.top)
                {
                    // The line is visible. We draw in 3 main parts - 1) the blank space to the left of the first run; 2) the runs themselves; 3) the blank
                    // space to the right of the last run.

                    uint8_t bgStyleSlot = pEngine->defaultStyleSlot;
                    if (line.index == drte_engine_get_cursor_line(pEngine)) {
                        bgStyleSlot = pEngine->activeLineStyleSlot;
                    }

                    float lineSelectionOverhangLeft  = 0;
                    float lineSelectionOverhangRight = 0;

                    if (drte_engine_is_anything_selected(pEngine))
                    {
                        drte_marker* pSelectionMarker0 = &pEngine->selectionAnchor;
                        drte_marker* pSelectionMarker1 = &pEngine->cursor;
                        if (pEngine->pRuns[pSelectionMarker0->iRun].iChar + pSelectionMarker0->iChar > pEngine->pRuns[pSelectionMarker1->iRun].iChar + pSelectionMarker1->iChar)
                        {
                            drte_marker* temp = pSelectionMarker0;
                            pSelectionMarker0 = pSelectionMarker1;
                            pSelectionMarker1 = temp;
                        }

                        size_t iSelectionLine0 = pEngine->pRuns[pSelectionMarker0->iRun].iLine;
                        size_t iSelectionLine1 = pEngine->pRuns[pSelectionMarker1->iRun].iLine;

                        if (line.index >= iSelectionLine0 && line.index < iSelectionLine1) {
                            lineSelectionOverhangRight = (float)pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.spaceWidth;
                        }
                    }


                    drte_text_run* pFirstRun = pEngine->pRuns + line.iFirstRun;
                    drte_text_run* pLastRun  = pEngine->pRuns + line.iLastRun;

                    float lineLeft  = pFirstRun->posX + textRect.left;
                    float lineRight = pLastRun->posX + pLastRun->width + textRect.left;

                    // 1) The blank space to the left of the first run.
                    if (lineLeft > 0)
                    {
                        if (lineSelectionOverhangLeft > 0) {
                            pEngine->onPaintRect(pEngine, pEngine->styles[pEngine->selectionStyleSlot].styleToken, drgui_make_rect(lineLeft - lineSelectionOverhangLeft, lineTop, lineLeft, lineBottom), pElement, pPaintData);
                        }

                        pEngine->onPaintRect(pEngine, pEngine->styles[bgStyleSlot].styleToken, drgui_make_rect(0, lineTop, lineLeft - lineSelectionOverhangLeft, lineBottom), pElement, pPaintData);
                    }


                    // 2) The runs themselves.
                    for (size_t iRun = line.iFirstRun; iRun <= line.iLastRun; ++iRun)
                    {
                        drte_text_run* pRun = pEngine->pRuns + iRun;

                        float runLeft  = pRun->posX + textRect.left;
                        float runRight = runLeft    + pRun->width;

                        if (runRight > 0 && runLeft < pEngine->containerWidth)
                        {
                            // The run is visible.
                            if (!drte_engine__is_text_run_whitespace(pEngine, pRun) || pEngine->text[pRun->iChar] == '\t')
                            {
                                drte_text_run run = pEngine->pRuns[iRun];
                                run.fgStyleSlot = pEngine->defaultStyleSlot;
                                run.bgStyleSlot = bgStyleSlot;
                                run.text        = pEngine->text + run.iChar;
                                run.posX        = runLeft;
                                run.posY        = lineTop;

                                // We paint the run differently depending on whether or not anything is selected. If something is selected
                                // we need to split the run into a maximum of 3 sub-runs so that the selection rectangle can be drawn correctly.
                                if (drte_engine_is_anything_selected(pEngine))
                                {
                                    drte_text_run subruns[3];
                                    size_t subrunCount = drte_engine__split_text_run_by_selection(pEngine, &run, subruns);
                                    for (size_t iSubRun = 0; iSubRun < subrunCount; ++iSubRun)
                                    {
                                        drte_text_run* pSubRun = subruns + iSubRun;

                                        if (!drte_engine__is_text_run_whitespace(pEngine, pRun)) {
                                            pEngine->onPaintText(pEngine, pEngine->styles[pSubRun->fgStyleSlot].styleToken, pEngine->styles[pSubRun->bgStyleSlot].styleToken, pSubRun, pElement, pPaintData);
                                        } else {
                                            pEngine->onPaintRect(pEngine, pEngine->styles[pSubRun->bgStyleSlot].styleToken, drgui_make_rect(pSubRun->posX, lineTop, pSubRun->posX + pSubRun->width, lineBottom), pElement, pPaintData);
                                        }
                                    }
                                }
                                else
                                {
                                    // Nothing is selected.
                                    if (!drte_engine__is_text_run_whitespace(pEngine, &run)) {
                                        pEngine->onPaintText(pEngine, pEngine->styles[run.fgStyleSlot].styleToken, pEngine->styles[run.bgStyleSlot].styleToken, &run, pElement, pPaintData);
                                    } else {
                                        pEngine->onPaintRect(pEngine, pEngine->styles[run.bgStyleSlot].styleToken, drgui_make_rect(run.posX, lineTop, run.posX + run.width, lineBottom), pElement, pPaintData);
                                    }
                                }
                            }
                        }
                    }


                    // 3) The blank space to the right of the last run.
                    if (lineRight < pEngine->containerWidth)
                    {
                        if (lineSelectionOverhangRight > 0) {
                            pEngine->onPaintRect(pEngine, pEngine->styles[pEngine->selectionStyleSlot].styleToken, drgui_make_rect(lineRight, lineTop, lineRight + lineSelectionOverhangRight, lineBottom), pElement, pPaintData);
                        }

                        pEngine->onPaintRect(pEngine, pEngine->styles[bgStyleSlot].styleToken, drgui_make_rect(lineRight + lineSelectionOverhangRight, lineTop, pEngine->containerWidth, lineBottom), pElement, pPaintData);
                    }
                }
            }
            else
            {
                // The line is below the rectangle which means no other line will be visible and we can terminate early.
                break;
            }

        } while (drte_engine__next_line(pEngine, &line));
    }
    else
    {
        // There are no lines so we do a simplified branch here.
        float lineTop    = textRect.top;
        float lineBottom = textRect.bottom;
        pEngine->onPaintRect(pEngine, pEngine->styles[pEngine->activeLineStyleSlot].styleToken, drgui_make_rect(0, lineTop, pEngine->containerWidth, lineBottom), pElement, pPaintData);
    }

    // The cursor.
    if (pEngine->isShowingCursor && pEngine->isCursorBlinkOn) {
        pEngine->onPaintRect(pEngine, pEngine->styles[pEngine->cursorStyleSlot].styleToken, drte_engine_get_cursor_rect(pEngine), pElement, pPaintData);
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


    // The position of each run will be relative to the text bounds. We want to make it relative to the container bounds.
    drgui_rect textRect = drte_engine_get_text_rect_relative_to_bounds(pEngine);

    // We draw a rectangle above and below the text rectangle. The main text rectangle will be drawn by iterating over each visible run.
    drgui_rect rectTop    = drgui_make_rect(0, 0, lineNumbersWidth, textRect.top);
    drgui_rect rectBottom = drgui_make_rect(0, textRect.bottom, lineNumbersWidth, lineNumbersHeight);

    if (pEngine->onPaintRect)
    {
        if (rectTop.bottom > 0) {
            onPaintRect(pEngine, pEngine->styles[pEngine->lineNumbersStyleSlot].styleToken, rectTop, pElement, pPaintData);
        }

        if (rectBottom.top < lineNumbersHeight) {
            onPaintRect(pEngine, pEngine->styles[pEngine->lineNumbersStyleSlot].styleToken, rectBottom, pElement, pPaintData);
        }
    }


    // Now we draw each line.
    int iLine = 1;
    drte_engine_line line;
    if (!drte_engine__first_line(pEngine, &line))
    {
        // We failed to retrieve the first line which is probably due to the text engine being empty. We just fake the first line to
        // ensure we get the number 1 to be drawn.
        line.height = (float)pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.lineHeight;
        line.posY = 0;
    }

    do
    {
        float lineTop    = line.posY + textRect.top;
        float lineBottom = lineTop + line.height;

        if (lineTop < lineNumbersHeight)
        {
            if (lineBottom > 0)
            {
                char iLineStr[64];
                #ifdef _MSC_VER
                _itoa_s(iLine, iLineStr, sizeof(iLineStr), 10);
                #else
                snprintf(iLineStr, sizeof(iLineStr), "%d", iLine);
                #endif

                float textWidth = 0;
                float textHeight = 0;
                if (pEngine->onMeasureString) {
                    pEngine->onMeasureString(pEngine, pEngine->styles[pEngine->lineNumbersStyleSlot].styleToken, iLineStr, strlen(iLineStr), &textWidth, &textHeight);
                }

                //drgui_measure_string(pFont, iLineStr, strlen(iLineStr), &textWidth, &textHeight);

                drte_text_run run = {0};
                run.fgStyleSlot = pEngine->lineNumbersStyleSlot;
                run.bgStyleSlot = pEngine->lineNumbersStyleSlot;
                run.text        = iLineStr;
                run.textLength  = strlen(iLineStr);
                run.posX        = lineNumbersWidth - textWidth;
                run.posY        = lineTop;  // TODO: Center this based on the height of the line.
                onPaintText(pEngine, pEngine->styles[run.fgStyleSlot].styleToken, pEngine->styles[run.bgStyleSlot].styleToken, &run, pElement, pPaintData);
                onPaintRect(pEngine, pEngine->styles[run.bgStyleSlot].styleToken, drgui_make_rect(0, lineTop, run.posX, lineBottom), pElement, pPaintData);
            }
        }
        else
        {
            // The line is below the rectangle which means no other line will be visible and we can terminate early.
            break;
        }

        iLine += 1;
    } while (drte_engine__next_line(pEngine, &line));
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




bool drte_next_run_string(const char* runStart, const char* textEndPastNullTerminator, const char** pRunEndOut)
{
    assert(runStart <= textEndPastNullTerminator);

    if (runStart == NULL || runStart == textEndPastNullTerminator)
    {
        // String is empty.
        return false;
    }


    char firstChar = runStart[0];
    if (firstChar == '\t')
    {
        // We loop until we hit anything that is not a tab character (tabs will be grouped together into a single run).
        do
        {
            runStart += 1;
            *pRunEndOut = runStart;
        } while (runStart[0] != '\0' && runStart[0] == '\t');
    }
    else if (firstChar == '\n')
    {
        runStart += 1;
        *pRunEndOut = runStart;
    }
    else if (firstChar == '\0')
    {
        assert(runStart + 1 == textEndPastNullTerminator);
        *pRunEndOut = textEndPastNullTerminator;
    }
    else
    {
        do
        {
            runStart += 1;
            *pRunEndOut = runStart;
        } while (runStart[0] != '\0' && runStart[0] != '\t' && runStart[0] != '\n');
    }

    return true;
}

void drte_engine__refresh(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    // We split the runs based on tabs and new-lines. We want to create runs for tabs and new-line characters as well because we want
    // to have the entire string covered by runs for the sake of simplicity when it comes to editing.
    //
    // The first pass positions the runs based on a top-to-bottom, left-to-right alignment. The second pass then repositions the runs
    // based on alignment.

    // Runs need to be cleared first.
    drte_engine__clear_text_runs(pEngine);

    // The text bounds also need to be reset at the top.
    pEngine->textBoundsWidth  = 0;
    pEngine->textBoundsHeight = 0;

    drte_font_metrics defaultFontMetrics = pEngine->styles[pEngine->defaultStyleSlot].fontMetrics;

    pEngine->textBoundsHeight = (float)defaultFontMetrics.lineHeight;

    float tabWidth = drte_engine__get_tab_width(pEngine);
    if (tabWidth <= 0) {
        tabWidth = (float)defaultFontMetrics.spaceWidth;
    }

    size_t iCurrentLine  = 0;
    float runningPosY       = 0;
    float runningLineHeight = 0;

    const char* nextRunStart = pEngine->text;
    const char* nextRunEnd;
    while (drte_next_run_string(nextRunStart, pEngine->text + pEngine->textLength + 1, OUT &nextRunEnd))
    {
        drte_text_run run;
        run.iLine      = iCurrentLine;
        run.iChar      = nextRunStart - pEngine->text;
        run.iCharEnd   = nextRunEnd   - pEngine->text;
        run.textLength = nextRunEnd - nextRunStart;
        run.width      = 0;
        run.height     = 0;
        run.posX       = 0;
        run.posY       = runningPosY;
        run.fgStyleSlot = pEngine->defaultStyleSlot;
        run.bgStyleSlot = pEngine->defaultStyleSlot;

        // X position
        //
        // The x position depends on the previous run that's on the same line.
        if (pEngine->runCount > 0)
        {
            drte_text_run* pPrevRun = pEngine->pRuns + (pEngine->runCount - 1);
            if (pPrevRun->iLine == iCurrentLine)
            {
                run.posX = pPrevRun->posX + pPrevRun->width;
            }
            else
            {
                // It's the first run on the line.
                run.posX = 0;
            }
        }


        // Width and height.
        assert(nextRunEnd > nextRunStart);
        if (nextRunStart[0] == '\t')
        {
            // Tab.
            size_t tabCount = run.iCharEnd - run.iChar;
            run.width  = (float)(((tabCount*(size_t)tabWidth) - ((size_t)run.posX % (size_t)tabWidth)));
            run.height = (float)defaultFontMetrics.lineHeight;
        }
        else if (nextRunStart[0] == '\n')
        {
            // New line.
            iCurrentLine += 1;
            run.width  = 0;
            run.height = (float)defaultFontMetrics.lineHeight;
        }
        else if (nextRunStart[0] == '\0')
        {
            // Null terminator.
            run.width      = 0;
            run.height     = (float)defaultFontMetrics.lineHeight;
            run.textLength = 0;
        }
        else
        {
            // Normal run.
            if (pEngine->onMeasureString) {
                pEngine->onMeasureString(pEngine, pEngine->styles[pEngine->defaultStyleSlot].styleToken, nextRunStart, run.textLength, &run.width, &run.height);
            }

            //drgui_measure_string(pEngine->pDefaultFont, nextRunStart, run.textLength, &run.width, &run.height);
        }


        // The running line height needs to be updated by setting to the maximum size.
        runningLineHeight = (run.height > runningLineHeight) ? run.height : runningLineHeight;


        // Update the text bounds.
        if (pEngine->textBoundsWidth < run.posX + run.width) {
            pEngine->textBoundsWidth = run.posX + run.width;
        }
        pEngine->textBoundsHeight = runningPosY + runningLineHeight;


        // A new line means we need to increment the running y position by the running line height.
        if (nextRunStart[0] == '\n')
        {
            runningPosY += runningLineHeight;
            runningLineHeight = 0;
        }

        // Add the run to the internal list.
        drte_engine__push_text_run(pEngine, &run);

        // Go to the next run string.
        nextRunStart = nextRunEnd;
    }
}


void drte_engine__push_text_run(drte_engine* pEngine, drte_text_run* pRun)
{
    if (pEngine == NULL) {
        return;
    }

    if (pEngine->runBufferSize == pEngine->runCount)
    {
        pEngine->runBufferSize = pEngine->runBufferSize*2;
        if (pEngine->runBufferSize == 0) {
            pEngine->runBufferSize = 1;
        }

        pEngine->pRuns = (drte_text_run*)realloc(pEngine->pRuns, sizeof(drte_text_run) * pEngine->runBufferSize);
        if (pEngine->pRuns == NULL) {
            pEngine->runCount = 0;
            pEngine->runBufferSize = 0;
            return;
        }
    }

    pEngine->pRuns[pEngine->runCount] = *pRun;
    pEngine->runCount += 1;
}

void drte_engine__clear_text_runs(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return;
    }

    pEngine->runCount = 0;
}

bool drte_engine__is_text_run_whitespace(drte_engine* pEngine, drte_text_run* pRun)
{
    if (pRun == NULL) {
        return false;
    }

    if (pEngine->text[pRun->iChar] != '\t' && pEngine->text[pRun->iChar] != '\n')
    {
        return false;
    }

    return true;
}

float drte_engine__get_tab_width(drte_engine* pEngine)
{
    return (float)(pEngine->styles[pEngine->defaultStyleSlot].fontMetrics.spaceWidth * pEngine->tabSizeInSpaces);
}


bool drte_engine__find_closest_line_to_point(drte_engine* pEngine, float inputPosYRelativeToText, size_t* pFirstRunIndexOnLineOut, size_t* pLastRunIndexOnLinePlus1Out)
{
    size_t iFirstRunOnLine     = 0;
    size_t iLastRunOnLinePlus1 = 0;

    bool result = true;
    if (pEngine == NULL || pEngine->runCount == 0)
    {
        result = false;
    }
    else
    {
        float runningLineTop = 0;

        float lineHeight;
        while (drte_engine__find_line_info(pEngine, iFirstRunOnLine, OUT &iLastRunOnLinePlus1, OUT &lineHeight))
        {
            const float lineTop    = runningLineTop;
            const float lineBottom = lineTop + lineHeight;

            if (inputPosYRelativeToText < lineBottom)
            {
                // It's on this line.
                break;
            }
            else
            {
                // It's not on this line - go to the next one, unless we're already on the last line.
                if (iLastRunOnLinePlus1 == pEngine->runCount) {
                    break;
                }

                iFirstRunOnLine = iLastRunOnLinePlus1;
                runningLineTop  = lineBottom;
            }
        }
    }

    if (pFirstRunIndexOnLineOut) {
        *pFirstRunIndexOnLineOut = iFirstRunOnLine;
    }
    if (pLastRunIndexOnLinePlus1Out) {
        *pLastRunIndexOnLinePlus1Out = iLastRunOnLinePlus1;
    }

    return result;
}

bool drte_engine__find_closest_run_to_point(drte_engine* pEngine, float inputPosXRelativeToText, float inputPosYRelativeToText, size_t* pRunIndexOut)
{
    if (pEngine == NULL) {
        return false;
    }

    size_t iFirstRunOnLine;
    size_t iLastRunOnLinePlus1;
    if (drte_engine__find_closest_line_to_point(pEngine, inputPosYRelativeToText, OUT &iFirstRunOnLine, OUT &iLastRunOnLinePlus1))
    {
        size_t iRunOut = 0;

        const drte_text_run* pFirstRunOnLine = pEngine->pRuns + iFirstRunOnLine;
        const drte_text_run* pLastRunOnLine  = pEngine->pRuns + (iLastRunOnLinePlus1 - 1);

        if (inputPosXRelativeToText < pFirstRunOnLine->posX)
        {
            // It's to the left of the first run.
            iRunOut = iFirstRunOnLine;
        }
        else if (inputPosXRelativeToText > pLastRunOnLine->posX + pLastRunOnLine->width)
        {
            // It's to the right of the last run.
            iRunOut = iLastRunOnLinePlus1 - 1;
        }
        else
        {
            // It's in the middle of the line. We just iterate over each run on the line and return the first one that contains the point.
            for (size_t iRun = iFirstRunOnLine; iRun < iLastRunOnLinePlus1; ++iRun)
            {
                const drte_text_run* pRun = pEngine->pRuns + iRun;
                iRunOut = iRun;

                if (inputPosXRelativeToText >= pRun->posX && inputPosXRelativeToText <= pRun->posX + pRun->width) {
                    break;
                }
            }
        }

        if (pRunIndexOut) {
            *pRunIndexOut = iRunOut;
        }

        return true;
    }
    else
    {
        // There was an error finding the closest line.
        return false;
    }
}

bool drte_engine__find_line_info(drte_engine* pEngine, size_t iFirstRunOnLine, size_t* pLastRunIndexOnLinePlus1Out, float* pLineHeightOut)
{
    if (pEngine == NULL) {
        return false;
    }

    if (iFirstRunOnLine < pEngine->runCount)
    {
        const size_t iLine = pEngine->pRuns[iFirstRunOnLine].iLine;
        float lineHeight = 0;

        size_t iRun;
        for (iRun = iFirstRunOnLine; iRun < pEngine->runCount && pEngine->pRuns[iRun].iLine == iLine; ++iRun)
        {
            if (lineHeight < pEngine->pRuns[iRun].height) {
                lineHeight = pEngine->pRuns[iRun].height;
            }
        }

        assert(iRun > iFirstRunOnLine);

        if (pLastRunIndexOnLinePlus1Out) {
            *pLastRunIndexOnLinePlus1Out = iRun;
        }
        if (pLineHeightOut) {
            *pLineHeightOut = lineHeight;
        }

        return true;
    }

    return false;
}

bool drte_engine__find_line_info_by_index(drte_engine* pEngine, size_t iLine, drgui_rect* pRectOut, size_t* pFirstRunIndexOut, size_t* pLastRunIndexPlus1Out)
{
    if (pEngine == NULL) {
        return false;
    }

    size_t iFirstRunOnLine     = 0;
    size_t iLastRunOnLinePlus1 = 0;

    float lineTop    = 0;
    float lineHeight = 0;

    for (size_t iCurrentLine = 0; iCurrentLine <= iLine; ++iCurrentLine)
    {
        iFirstRunOnLine = iLastRunOnLinePlus1;
        lineTop += lineHeight;

        if (!drte_engine__find_line_info(pEngine, iFirstRunOnLine, &iLastRunOnLinePlus1, &lineHeight))
        {
            // There was an error retrieving information about the line.
            return false;
        }
    }


    // At this point we have the first and last runs that make up the line and we can generate our output.
    if (iLastRunOnLinePlus1 > iFirstRunOnLine)
    {
        if (pFirstRunIndexOut) {
            *pFirstRunIndexOut = iFirstRunOnLine;
        }
        if (pLastRunIndexPlus1Out) {
            *pLastRunIndexPlus1Out = iLastRunOnLinePlus1;
        }

        if (pRectOut != NULL)
        {
            pRectOut->left   = pEngine->pRuns[iFirstRunOnLine].posX;
            pRectOut->right  = pEngine->pRuns[iLastRunOnLinePlus1 - 1].posX + pEngine->pRuns[iLastRunOnLinePlus1 - 1].width;
            pRectOut->top    = lineTop;
            pRectOut->bottom = pRectOut->top + lineHeight;
        }

        return true;
    }
    else
    {
        // We couldn't find any runs.
        return false;
    }
}

bool drte_engine__find_last_run_on_line_starting_from_run(drte_engine* pEngine, size_t iRun, size_t* pLastRunIndexOnLineOut)
{
    if (pEngine == NULL || pEngine->pRuns == NULL) {
        return false;
    }

    size_t result = iRun;

    size_t iLine = pEngine->pRuns[iRun].iLine;
    for (/* Do Nothing*/; iRun < pEngine->runCount && pEngine->pRuns[iRun].iLine == iLine; ++iRun)
    {
        result = iRun;
    }

    if (pLastRunIndexOnLineOut) {
        *pLastRunIndexOnLineOut = result;
    }

    return true;
}

bool drte_engine__find_first_run_on_line_starting_from_run(drte_engine* pEngine, size_t iRun, size_t* pFirstRunIndexOnLineOut)
{
    if (pEngine == NULL) {
        return false;
    }

    size_t result = iRun;

    size_t iLine = pEngine->pRuns[iRun].iLine;
    for (/* Do Nothing*/; iRun > 0 && pEngine->pRuns[iRun - 1].iLine == iLine; --iRun)
    {
        result = iRun - 1;
    }

    if (pFirstRunIndexOnLineOut) {
        *pFirstRunIndexOnLineOut = result;
    }

    return true;
}

bool drte_engine__find_run_at_character(drte_engine* pEngine, size_t iChar, size_t* pRunIndexOut)
{
    if (pEngine == NULL || pEngine->runCount == 0) {
        return false;
    }

    size_t result = 0;
    if (iChar < pEngine->textLength)
    {
        for (size_t iRun = 0; iRun < pEngine->runCount; ++iRun)
        {
            const drte_text_run* pRun = pEngine->pRuns + iRun;

            if (iChar < pRun->iCharEnd)
            {
                result = iRun;
                break;
            }
        }
    }
    else
    {
        // The character index is too high. Return the last run.
        result = pEngine->runCount - 1;
    }

    if (pRunIndexOut) {
        *pRunIndexOut = result;
    }

    return true;
}


drte_marker drte_engine__new_marker()
{
    drte_marker marker;
    marker.iRun              = 0;
    marker.iChar             = 0;
    marker.relativePosX      = 0;
    marker.absoluteSickyPosX = 0;

    return marker;
}

bool drte_engine__move_marker_to_point_relative_to_container(drte_engine* pEngine, drte_marker* pMarker, float inputPosX, float inputPosY)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    pMarker->iRun              = 0;
    pMarker->iChar             = 0;
    pMarker->relativePosX      = 0;
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
    if (pEngine == NULL || pMarker == NULL) {
        return;
    }

    float posX = 0;
    float posY = 0;

    if (pMarker->iRun < pEngine->runCount)
    {
        posX = pEngine->pRuns[pMarker->iRun].posX + pMarker->relativePosX;
        posY = pEngine->pRuns[pMarker->iRun].posY;
    }

    drgui_rect textRect = drte_engine_get_text_rect_relative_to_bounds(pEngine);
    posX += textRect.left;
    posY += textRect.top;


    if (pPosXOut) {
        *pPosXOut = posX;
    }
    if (pPosYOut) {
        *pPosYOut = posY;
    }
}

bool drte_engine__move_marker_to_point(drte_engine* pEngine, drte_marker* pMarker, float inputPosXRelativeToText, float inputPosYRelativeToText)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    size_t iClosestRunToPoint;
    if (drte_engine__find_closest_run_to_point(pEngine, inputPosXRelativeToText, inputPosYRelativeToText, OUT &iClosestRunToPoint))
    {
        const drte_text_run* pRun = pEngine->pRuns + iClosestRunToPoint;

        pMarker->iRun = iClosestRunToPoint;

        if (inputPosXRelativeToText < pRun->posX)
        {
            // It's to the left of the run.
            pMarker->iChar        = 0;
            pMarker->relativePosX = 0;
        }
        else if (inputPosXRelativeToText > pRun->posX + pRun->width)
        {
            // It's to the right of the run. It may be a new-line run. If so, we need to move the marker to the front of it, not the back.
            pMarker->iChar        = pRun->textLength;
            pMarker->relativePosX = pRun->width;

            if (pEngine->text[pRun->iChar] == '\n') {
                assert(pMarker->iChar == 1);
                pMarker->iChar        = 0;
                pMarker->relativePosX = 0;
            }
        }
        else
        {
            // It's somewhere in the middle of the run. We need to handle this a little different for tab runs since they are aligned differently.
            if (pEngine->text[pRun->iChar] == '\n')
            {
                // It's a new line character. It needs to be placed at the beginning of it.
                pMarker->iChar        = 0;
                pMarker->relativePosX = 0;
            }
            else if (pEngine->text[pRun->iChar] == '\t')
            {
                // It's a tab run.
                pMarker->iChar        = 0;
                pMarker->relativePosX = 0;

                const float tabWidth = drte_engine__get_tab_width(pEngine);

                float tabLeft = pRun->posX + pMarker->relativePosX;
                for (/* Do Nothing*/; pMarker->iChar < pRun->textLength; ++pMarker->iChar)
                {
                    float tabRight = tabWidth * ((pRun->posX + (tabWidth*(pMarker->iChar + 1))) / tabWidth);
                    if (tabRight > pRun->posX + pRun->width) {
                        tabRight = pRun->posX + pRun->width;
                    }

                    if (inputPosXRelativeToText >= tabLeft && inputPosXRelativeToText <= tabRight)
                    {
                        // The input position is somewhere on top of this character. If it's positioned on the left side of the character, set the output
                        // value to the character at iChar. Otherwise it should be set to the character at iChar + 1.
                        float charBoundsRightHalf = tabLeft + ceilf(((tabRight - tabLeft) / 2.0f));
                        if (inputPosXRelativeToText <= charBoundsRightHalf) {
                            pMarker->relativePosX = tabLeft - pRun->posX;
                        } else {
                            pMarker->relativePosX = tabRight - pRun->posX;
                            pMarker->iChar += 1;
                        }

                        break;
                    }

                    tabLeft = tabRight;
                }

                // If we're past the last character in the tab run, we want to move to the start of the next run.
                if (pMarker->iChar == pRun->textLength) {
                    drte_engine__move_marker_to_first_character_of_next_run(pEngine, pMarker);
                }
            }
            else
            {
                // It's a standard run.
                float inputPosXRelativeToRun = inputPosXRelativeToText - pRun->posX;
                if (pEngine->onGetCursorPositionFromPoint) {
                    pEngine->onGetCursorPositionFromPoint(pEngine, pEngine->styles[pRun->fgStyleSlot].styleToken, pEngine->text + pRun->iChar, pRun->textLength, pRun->width, inputPosXRelativeToRun, OUT &pMarker->relativePosX, OUT &pMarker->iChar);

                    // If the marker is past the last character of the run it needs to be moved to the start of the next one.
                    if (pMarker->iChar == pRun->textLength) {
                        drte_engine__move_marker_to_first_character_of_next_run(pEngine, pMarker);
                    }
                } else {
                    return false;
                }
            }
        }

        return true;
    }
    else
    {
        // Couldn't find a run.
        return false;
    }
}

bool drte_engine__move_marker_left(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (pEngine->runCount > 0)
    {
        if (pMarker->iChar > 0)
        {
            pMarker->iChar -= 1;

            const drte_text_run* pRun = pEngine->pRuns + pMarker->iRun;
            if (pEngine->text[pRun->iChar] == '\t')
            {
                const float tabWidth = drte_engine__get_tab_width(pEngine);

                if (pMarker->iChar == 0)
                {
                    // Simple case - it's the first tab character which means the relative position is just 0.
                    pMarker->relativePosX = 0;
                }
                else
                {
                    pMarker->relativePosX  = tabWidth * ((pRun->posX + (tabWidth*(pMarker->iChar + 0))) / tabWidth);
                    pMarker->relativePosX -= pRun->posX;
                }
            }
            else
            {
                if (pEngine->onGetCursorPositionFromChar) {
                    pEngine->onGetCursorPositionFromChar(pEngine, pEngine->styles[pRun->fgStyleSlot].styleToken, pEngine->text + pEngine->pRuns[pMarker->iRun].iChar, pMarker->iChar, OUT &pMarker->relativePosX);
                } else {
                    return false;
                }
            }
        }
        else
        {
            // We're at the beginning of the run which means we need to transfer the cursor to the end of the previous run.
            if (!drte_engine__move_marker_to_last_character_of_prev_run(pEngine, pMarker)) {
                return false;
            }
        }

        drte_engine__update_marker_sticky_position(pEngine, pMarker);
        return true;
    }

    return false;
}

bool drte_engine__move_marker_right(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (pEngine->runCount > 0)
    {
        if (pMarker->iChar + 1 < pEngine->pRuns[pMarker->iRun].textLength)
        {
            pMarker->iChar += 1;

            const drte_text_run* pRun = pEngine->pRuns + pMarker->iRun;
            if (pEngine->text[pRun->iChar] == '\t')
            {
                const float tabWidth = drte_engine__get_tab_width(pEngine);

                pMarker->relativePosX  = tabWidth * ((pRun->posX + (tabWidth*(pMarker->iChar + 0))) / tabWidth);
                pMarker->relativePosX -= pRun->posX;
            }
            else
            {
                if (pEngine->onGetCursorPositionFromChar) {
                    pEngine->onGetCursorPositionFromChar(pEngine, pEngine->styles[pRun->fgStyleSlot].styleToken, pEngine->text + pEngine->pRuns[pMarker->iRun].iChar, pMarker->iChar, OUT &pMarker->relativePosX);
                } else {
                    return false;
                }
            }
        }
        else
        {
            // We're at the end of the run which means we need to transfer the cursor to the beginning of the next run.
            if (!drte_engine__move_marker_to_first_character_of_next_run(pEngine, pMarker)) {
                return false;
            }
        }

        drte_engine__update_marker_sticky_position(pEngine, pMarker);
        return true;
    }

    return false;
}

bool drte_engine__move_marker_up(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL || pEngine->runCount == 0) {
        return false;
    }

    return drte_engine__move_marker_y(pEngine, pMarker, -1);
}

bool drte_engine__move_marker_down(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL || pEngine->runCount == 0) {
        return false;
    }

    return drte_engine__move_marker_y(pEngine, pMarker, 1);
}

bool drte_engine__move_marker_y(drte_engine* pEngine, drte_marker* pMarker, int amount)
{
    if (pEngine == NULL || pMarker == NULL || pEngine->runCount == 0) {
        return false;
    }

    const drte_text_run* pOldRun = pEngine->pRuns + pMarker->iRun;

    int iNewLine = (int)pOldRun->iLine + amount;
    if (iNewLine >= (int)drte_engine_get_line_count(pEngine)) {
        iNewLine = (int)drte_engine_get_line_count(pEngine) - 1;
    }
    if (iNewLine < 0) {
        iNewLine = 0;
    }

    if ((int)pOldRun->iLine == iNewLine) {
        return false;   // The lines are the same.
    }

    drgui_rect lineRect;
    size_t iFirstRunOnLine;
    size_t iLastRunOnLinePlus1;
    if (drte_engine__find_line_info_by_index(pEngine, iNewLine, OUT &lineRect, OUT &iFirstRunOnLine, OUT &iLastRunOnLinePlus1))
    {
        float newMarkerPosX = pMarker->absoluteSickyPosX;
        float newMarkerPosY = lineRect.top;
        drte_engine__move_marker_to_point(pEngine, pMarker, newMarkerPosX, newMarkerPosY);

        return true;
    }
    else
    {
        // An error occured while finding information about the line above.
        return false;
    }
}

bool drte_engine__move_marker_to_end_of_line(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    size_t iLastRunOnLine;
    if (drte_engine__find_last_run_on_line_starting_from_run(pEngine, pMarker->iRun, &iLastRunOnLine))
    {
        return drte_engine__move_marker_to_last_character_of_run(pEngine, pMarker, iLastRunOnLine);
    }

    return false;
}

bool drte_engine__move_marker_to_start_of_line(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    size_t iFirstRunOnLine;
    if (drte_engine__find_first_run_on_line_starting_from_run(pEngine, pMarker->iRun, &iFirstRunOnLine))
    {
        return drte_engine__move_marker_to_first_character_of_run(pEngine, pMarker, iFirstRunOnLine);
    }

    return false;
}

bool drte_engine__move_marker_to_end_of_line_by_index(drte_engine* pEngine, drte_marker* pMarker, size_t iLine)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    size_t iFirstRun;
    size_t iLastRunPlus1;
    if (drte_engine__find_line_info_by_index(pEngine, iLine, NULL, &iFirstRun, &iLastRunPlus1))
    {
        return drte_engine__move_marker_to_last_character_of_run(pEngine, pMarker, iLastRunPlus1 - 1);
    }

    return false;
}

bool drte_engine__move_marker_to_start_of_line_by_index(drte_engine* pEngine, drte_marker* pMarker, size_t iLine)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    size_t iFirstRun;
    size_t iLastRunPlus1;
    if (drte_engine__find_line_info_by_index(pEngine, iLine, NULL, &iFirstRun, &iLastRunPlus1))
    {
        return drte_engine__move_marker_to_first_character_of_run(pEngine, pMarker, iFirstRun);
    }

    return false;
}

bool drte_engine__move_marker_to_end_of_text(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (pEngine->runCount > 0) {
        return drte_engine__move_marker_to_last_character_of_run(pEngine, pMarker, pEngine->runCount - 1);
    }

    return false;
}

bool drte_engine__move_marker_to_start_of_text(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    return drte_engine__move_marker_to_first_character_of_run(pEngine, pMarker, 0);
}

bool drte_engine__move_marker_to_last_character_of_run(drte_engine* pEngine, drte_marker* pMarker, size_t iRun)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (iRun < pEngine->runCount)
    {
        pMarker->iRun         = iRun;
        pMarker->iChar        = pEngine->pRuns[pMarker->iRun].textLength;
        pMarker->relativePosX = pEngine->pRuns[pMarker->iRun].width;

        if (pMarker->iChar > 0)
        {
            // At this point we are located one character past the last character - we need to move it left.
            return drte_engine__move_marker_left(pEngine, pMarker);
        }

        return true;
    }

    return false;
}

bool drte_engine__move_marker_to_first_character_of_run(drte_engine* pEngine, drte_marker* pMarker, size_t iRun)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (iRun < pEngine->runCount)
    {
        pMarker->iRun         = iRun;
        pMarker->iChar        = 0;
        pMarker->relativePosX = 0;

        drte_engine__update_marker_sticky_position(pEngine, pMarker);

        return true;
    }

    return false;
}

bool drte_engine__move_marker_to_last_character_of_prev_run(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (pMarker->iRun > 0) {
        return drte_engine__move_marker_to_last_character_of_run(pEngine, pMarker, pMarker->iRun - 1);
    }

    return false;
}

bool drte_engine__move_marker_to_first_character_of_next_run(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return false;
    }

    if (pEngine->runCount > 0 && pMarker->iRun < pEngine->runCount - 1) {
        return drte_engine__move_marker_to_first_character_of_run(pEngine, pMarker, pMarker->iRun + 1);
    }

    return false;
}

bool drte_engine__move_marker_to_character(drte_engine* pEngine, drte_marker* pMarker, size_t iChar)
{
    if (pEngine == NULL || pMarker == NULL || pEngine->runCount == 0) {
        return false;
    }

    // Clamp the character to the end of the string.
    if (iChar > pEngine->textLength) {
        iChar = pEngine->textLength;
    }

    drte_engine__find_run_at_character(pEngine, iChar, &pMarker->iRun);

    assert(pMarker->iRun < pEngine->runCount);
    pMarker->iChar = iChar - pEngine->pRuns[pMarker->iRun].iChar;


    // The relative position depends on whether or not the run is a tab character.
    return drte_engine__update_marker_relative_position(pEngine, pMarker);
}


bool drte_engine__update_marker_relative_position(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL || pEngine->runCount == 0) {
        return false;
    }

    const drte_text_run* pRun = pEngine->pRuns + pMarker->iRun;
    if (pEngine->text[pRun->iChar] == '\t')
    {
        const float tabWidth = drte_engine__get_tab_width(pEngine);

        if (pMarker->iChar == 0)
        {
            // Simple case - it's the first tab character which means the relative position is just 0.
            pMarker->relativePosX = 0;
        }
        else
        {
            pMarker->relativePosX  = tabWidth * ((pRun->posX + (tabWidth*(pMarker->iChar + 0))) / tabWidth);
            pMarker->relativePosX -= pRun->posX;
        }

        return true;
    }
    else
    {
        if (pEngine->onGetCursorPositionFromChar) {
            pEngine->onGetCursorPositionFromChar(pEngine, pEngine->styles[pRun->fgStyleSlot].styleToken, pEngine->text + pEngine->pRuns[pMarker->iRun].iChar, pMarker->iChar, OUT &pMarker->relativePosX);
            return true;
        }

        return false;
    }
}

void drte_engine__update_marker_sticky_position(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL) {
        return;
    }

    pMarker->absoluteSickyPosX = pEngine->pRuns[pMarker->iRun].posX + pMarker->relativePosX;
}

size_t drte_engine__get_marker_absolute_char_index(drte_engine* pEngine, drte_marker* pMarker)
{
    if (pEngine == NULL || pMarker == NULL || pEngine->runCount == 0) {
        return 0;
    }

    return pEngine->pRuns[pMarker->iRun].iChar + pMarker->iChar;
}


bool drte_engine__has_spacing_between_selection_markers(drte_engine* pEngine)
{
    if (pEngine == NULL) {
        return false;
    }

    return (pEngine->cursor.iRun != pEngine->selectionAnchor.iRun || pEngine->cursor.iChar != pEngine->selectionAnchor.iChar);
}

size_t drte_engine__split_text_run_by_selection(drte_engine* pEngine, drte_text_run* pRunToSplit, drte_text_run pSubRunsOut[3])
{
    if (pEngine == NULL || pRunToSplit == NULL || pSubRunsOut == NULL) {
        return 0;
    }

    drte_marker* pSelectionMarker0 = &pEngine->selectionAnchor;
    drte_marker* pSelectionMarker1 = &pEngine->cursor;
    if (pEngine->pRuns[pSelectionMarker0->iRun].iChar + pSelectionMarker0->iChar > pEngine->pRuns[pSelectionMarker1->iRun].iChar + pSelectionMarker1->iChar)
    {
        drte_marker* temp = pSelectionMarker0;
        pSelectionMarker0 = pSelectionMarker1;
        pSelectionMarker1 = temp;
    }

    drte_text_run* pSelectionRun0 = pEngine->pRuns + pSelectionMarker0->iRun;
    drte_text_run* pSelectionRun1 = pEngine->pRuns + pSelectionMarker1->iRun;

    size_t iSelectionChar0 = pSelectionRun0->iChar + pSelectionMarker0->iChar;
    size_t iSelectionChar1 = pSelectionRun1->iChar + pSelectionMarker1->iChar;

    if (drte_engine_is_anything_selected(pEngine))
    {
        if (pRunToSplit->iChar < iSelectionChar1 && pRunToSplit->iCharEnd > iSelectionChar0)
        {
            // The run is somewhere inside the selection region.
            for (int i = 0; i < 3; ++i) {
                pSubRunsOut[i] = *pRunToSplit;
            }

            if (pRunToSplit->iChar >= iSelectionChar0)
            {
                // The first part of the run is selected.
                if (pRunToSplit->iCharEnd <= iSelectionChar1)
                {
                    // The entire run is selected.
                    pSubRunsOut[0].bgStyleSlot = pEngine->selectionStyleSlot;
                    return 1;
                }
                else
                {
                    // The head part of the run is selected, and the tail is deselected.

                    // Head.
                    pSubRunsOut[0].bgStyleSlot     = pEngine->selectionStyleSlot;
                    pSubRunsOut[0].iCharEnd        = iSelectionChar1;
                    pSubRunsOut[0].width           = pSelectionMarker1->relativePosX;
                    pSubRunsOut[0].text            = pEngine->text + pSubRunsOut[0].iChar;
                    pSubRunsOut[0].textLength      = pSubRunsOut[0].iCharEnd - pSubRunsOut[0].iChar;

                    // Tail.
                    pSubRunsOut[1].iChar      = iSelectionChar1;
                    pSubRunsOut[1].width      = pRunToSplit->width - pSelectionMarker1->relativePosX;
                    pSubRunsOut[1].posX       = pSubRunsOut[0].posX + pSubRunsOut[0].width;
                    pSubRunsOut[1].text       = pEngine->text + pSubRunsOut[1].iChar;
                    pSubRunsOut[1].textLength = pSubRunsOut[1].iCharEnd - pSubRunsOut[1].iChar;

                    return 2;
                }
            }
            else
            {
                // The first part of the run is deselected. There will be at least 2, but possibly 3 sub-runs in this case.
                if (pRunToSplit->iCharEnd <= iSelectionChar1)
                {
                    // The head of the run is deselected and the tail is selected.

                    // Head.
                    pSubRunsOut[0].iCharEnd        = iSelectionChar0;
                    pSubRunsOut[0].width           = pSelectionMarker0->relativePosX;
                    pSubRunsOut[0].text            = pEngine->text + pSubRunsOut[0].iChar;
                    pSubRunsOut[0].textLength      = pSubRunsOut[0].iCharEnd - pSubRunsOut[0].iChar;

                    // Tail.
                    pSubRunsOut[1].bgStyleSlot     = pEngine->selectionStyleSlot;
                    pSubRunsOut[1].iChar           = iSelectionChar0;
                    pSubRunsOut[1].width           = pRunToSplit->width - pSubRunsOut[0].width;
                    pSubRunsOut[1].posX            = pSubRunsOut[0].posX + pSubRunsOut[0].width;
                    pSubRunsOut[1].text            = pEngine->text + pSubRunsOut[1].iChar;
                    pSubRunsOut[1].textLength      = pSubRunsOut[1].iCharEnd - pSubRunsOut[1].iChar;

                    return 2;
                }
                else
                {
                    // The head and tail are both deselected, and the middle section is selected.

                    // Head.
                    pSubRunsOut[0].iCharEnd   = iSelectionChar0;
                    pSubRunsOut[0].width      = pSelectionMarker0->relativePosX;
                    pSubRunsOut[0].text       = pEngine->text + pSubRunsOut[0].iChar;
                    pSubRunsOut[0].textLength = pSubRunsOut[0].iCharEnd - pSubRunsOut[0].iChar;

                    // Mid.
                    pSubRunsOut[1].iChar           = iSelectionChar0;
                    pSubRunsOut[1].iCharEnd        = iSelectionChar1;
                    pSubRunsOut[1].bgStyleSlot     = pEngine->selectionStyleSlot;
                    pSubRunsOut[1].width           = pSelectionMarker1->relativePosX - pSelectionMarker0->relativePosX;
                    pSubRunsOut[1].posX            = pSubRunsOut[0].posX + pSubRunsOut[0].width;
                    pSubRunsOut[1].text            = pEngine->text + pSubRunsOut[1].iChar;
                    pSubRunsOut[1].textLength      = pSubRunsOut[1].iCharEnd - pSubRunsOut[1].iChar;

                    // Tail.
                    pSubRunsOut[2].iChar      = iSelectionChar1;
                    pSubRunsOut[2].width      = pRunToSplit->width - pSelectionMarker1->relativePosX;
                    pSubRunsOut[2].posX       = pSubRunsOut[1].posX + pSubRunsOut[1].width;
                    pSubRunsOut[2].text       = pEngine->text + pSubRunsOut[2].iChar;
                    pSubRunsOut[2].textLength = pSubRunsOut[2].iCharEnd - pSubRunsOut[2].iChar;

                    return 3;
                }
            }
        }
    }

    // If we get here it means the run is not within the selected region.
    pSubRunsOut[0] = *pRunToSplit;
    return 1;
}

#if 0
bool drte_engine__is_run_selected(drte_engine* pEngine, unsigned int iRun)
{
    if (drte_engine_is_anything_selected(pEngine))
    {
        drte_marker* pSelectionMarker0;
        drte_marker* pSelectionMarker1;
        if (!drte_engine__get_selection_markers(pEngine, &pSelectionMarker0, &pSelectionMarker1)) {
            return false;
        }

        unsigned int iSelectionChar0 = pEngine->pRuns[pSelectionMarker0->iRun].iChar + pSelectionMarker0->iChar;
        unsigned int iSelectionChar1 = pEngine->pRuns[pSelectionMarker1->iRun].iChar + pSelectionMarker1->iChar;

        return pEngine->pRuns[iRun].iChar < iSelectionChar1 && pEngine->pRuns[iRun].iCharEnd > iSelectionChar0;
    }

    return false;
}
#endif

bool drte_engine__get_selection_markers(drte_engine* pEngine, drte_marker** ppSelectionMarker0Out, drte_marker** ppSelectionMarker1Out)
{
    bool result = false;

    drte_marker* pSelectionMarker0 = NULL;
    drte_marker* pSelectionMarker1 = NULL;
    if (pEngine != NULL && drte_engine_is_anything_selected(pEngine))
    {
        pSelectionMarker0 = &pEngine->selectionAnchor;
        pSelectionMarker1 = &pEngine->cursor;
        if (pEngine->pRuns[pSelectionMarker0->iRun].iChar + pSelectionMarker0->iChar > pEngine->pRuns[pSelectionMarker1->iRun].iChar + pSelectionMarker1->iChar)
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


bool drte_engine__first_line(drte_engine* pEngine, drte_engine_line* pLine)
{
    if (pEngine == NULL || pLine == NULL || pEngine->runCount == 0) {
        return false;
    }

    pLine->index     = 0;
    pLine->posY      = 0;
    pLine->height    = 0;
    pLine->iFirstRun = 0;
    pLine->iLastRun  = 0;

    // We need to find the last run in the line and the height. The height is determined by the run with the largest height.
    while (pLine->iLastRun < pEngine->runCount)
    {
        if (pLine->height < pEngine->pRuns[pLine->iLastRun].height) {
            pLine->height = pEngine->pRuns[pLine->iLastRun].height;
        }

        pLine->iLastRun += 1;

        if (pEngine->pRuns[pLine->iLastRun].iLine != pLine->index) {
            break;
        }
    }

    if (pLine->iLastRun > 0) {
        pLine->iLastRun -= 1;
    }

    return true;
}

bool drte_engine__next_line(drte_engine* pEngine, drte_engine_line* pLine)
{
    if (pEngine == NULL || pLine == NULL || pEngine->runCount == 0) {
        return false;
    }

    // If there's no more runs, there is no next line.
    if (pLine->iLastRun == pEngine->runCount - 1) {
        return false;
    }

    pLine->index     += 1;
    pLine->posY      += pLine->height;
    pLine->height    = 0;
    pLine->iFirstRun = pLine->iLastRun + 1;
    pLine->iLastRun  = pLine->iFirstRun;

    while (pLine->iLastRun < pEngine->runCount)
    {
        if (pLine->height < pEngine->pRuns[pLine->iLastRun].height) {
            pLine->height = pEngine->pRuns[pLine->iLastRun].height;
        }

        pLine->iLastRun += 1;

        if (pEngine->pRuns[pLine->iLastRun].iLine != pLine->index) {
            break;
        }
    }

    if (pLine->iLastRun > 0) {
        pLine->iLastRun -= 1;
    }

    return true;
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

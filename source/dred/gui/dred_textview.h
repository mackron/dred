// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_TEXTVIEW  "dred.textview"

typedef struct dred_textview dred_textview;
#define DRED_TEXTVIEW(a) ((dred_textview*)(a))

typedef void (* dred_textview_on_cursor_move_proc)(dred_textview* pTextView);
typedef void (* dred_textview_on_undo_point_changed_proc)(dred_textview* pTextView, unsigned int iUndoPoint);

// A cursor in a textbox is tied to either 1 or 0 selection regions. When a cursor is not associated with a selection, the
// index of the selection region is set to -1.
typedef struct
{
    size_t iEngineCursor;        // <-- Always >= 0.
    size_t iEngineSelection;     // <-- Set to -1 if the cursor is not associated with a selection.
} dred_textview_cursor;

struct dred_textview
{
    // The base control.
    dred_control control;


    // The text engine.
    drte_engine* pTextEngine;

    // The view that's attached to the engine and used for displaying the contents of the text engine.
    drte_view* pView;


    // The default style for use by the text engine.
    dred_text_style defaultStyle;

    // The style to apply to selected text. Only the background color is used.
    dred_text_style selectionStyle;

    // The style to apply to active lines.
    dred_text_style activeLineStyle;

    // The style to apply to the cursor.
    dred_text_style cursorStyle;

    // The style to apply to line numbers.
    dred_text_style lineNumbersStyle;


    /// The vertical scrollbar.
    dred_scrollbar vertScrollbar;
    dred_scrollbar* pVertScrollbar;

    /// The horizontal scrollbar.
    dred_scrollbar horzScrollbar;
    dred_scrollbar* pHorzScrollbar;

    /// The line numbers element.
    dred_control lineNumbers;
    dred_control* pLineNumbers; // <-- Always equal to &lineNumbers and only used for convenience.


    /// The color of the border.
    dred_color borderColor;

    /// The width of the border.
    float borderWidth;

    /// The amount of padding to apply the left and right of the text.
    float padding;

    // The width of the line numbers.
    float lineNumbersWidth;

    /// The padding to the right of the line numbers.
    float lineNumbersPaddingRight;


    /// The desired width of the vertical scrollbar.
    float vertScrollbarSize;

    /// The desired height of the horizontal scrollbar.
    float horzScrollbarSize;

    /// Whether or not the vertical scrollbar is enabled.
    dr_bool32 isVertScrollbarEnabled;

    /// Whether or not the horizontal scrollbar is enabled.
    dr_bool32 isHorzScrollbarEnabled;

    // Whether or not an extra page is added to the scroll range so that the user can scroll past the last line. When set to DR_TRUE,
    // the user can scroll until the last line is sitting at the top of the text box. When disabled the user can scroll until the
    // last line is sitting at the bottom.
    dr_bool32 isExcessScrollingEnabled;

    // Whether or not tabs to spaces is enabled.
    dr_bool32 isTabsToSpacesEnabled;

    // Whether or not drag-and-drop is enabled.
    dr_bool32 isDragAndDropEnabled;

    // Whether or not the use is about to start drag-and-dropping.
    dr_bool32 isWantingToDragAndDrop;


    // Whether or not we are doing a rectangle selection.
    dr_bool32 isDoingRectangleSelect;

    // The selection rectangle in pixels.
    drte_rect selectionRect;



    // The number of active cursors.
    size_t cursorCount;

    // The buffer containing the active cursors.
    dred_textview_cursor* pCursors;


    /// When selecting lines by clicking and dragging on the line numbers, keeps track of the line to anchor the selection to.
    size_t iLineSelectAnchor;


    // Whether or not we are doing word based mouse-drag selection.
    dr_bool32 isDoingWordSelect;

    // The word to act as the achor when doing word based selection.
    drte_region wordSelectionAnchor;


    /// The function to call when the text cursor/caret moves.
    dred_textview_on_cursor_move_proc onCursorMove;

    /// The function to call when the undo point changes.
    dred_textview_on_undo_point_changed_proc onUndoPointChanged;


    // The timer for stepping the cursor.
    dtk_timer* pTimer;
};


// Creates a new text box control.
dr_bool32 dred_textview_init(dred_textview* pTextView, dred_context* pDred, dred_control* pParent, drte_engine* pTextEngine);

// Deletes the given text box control.
void dred_textview_uninit(dred_textview* pTextView);


// Retrieves a pointer to the internal text engine.
drte_engine* dred_textview_get_engine(dred_textview* pTextView);


// Sets the font to use with the given text box.
void dred_textview_set_font(dred_textview* pTextView, dred_gui_font* pFont);

// Retrieves the font being used with the given text box.
dred_gui_font* dred_textview_get_font(dred_textview* pTextView);

// Sets the color of the text in teh given text box.
void dred_textview_set_text_color(dred_textview* pTextView, dred_color color);

// Sets the background color of the given text box.
void dred_textview_set_background_color(dred_textview* pTextView, dred_color color);

// Sets the background color of selected text.
void dred_textview_set_selection_background_color(dred_textview* pTextView, dred_color color);

// Retrieves the background color of selected text.
dred_color dred_textview_get_selection_background_color(dred_textview* pTextView);

// Sets the background color for the line the caret is currently sitting on.
void dred_textview_set_active_line_background_color(dred_textview* pTextView, dred_color color);

// Sets the width of the text cursor.
void dred_textview_set_cursor_width(dred_textview* pTextView, float cursorWidth);

// Retrieves the width of the text cursor.
float dred_textview_get_cursor_width(dred_textview* pTextView);

// Sets the color of the cursor of the given text box.
void dred_textview_set_cursor_color(dred_textview* pTextView, dred_color color);

// Sets the border color of the given text box.
void dred_textview_set_border_color(dred_textview* pTextView, dred_color color);

// Sets the border width of the given text box.
void dred_textview_set_border_width(dred_textview* pTextView, float borderWidth);

// Sets the amount of padding to apply to given text box.
void dred_textview_set_padding(dred_textview* pTextView, float padding);

// Retrieves the amound of vertical padding to apply to the given text box.
float dred_textview_get_padding_vert(dred_textview* pTextView);

// Retrieves the amound of horizontal padding to apply to the given text box.
float dred_textview_get_padding_horz(dred_textview* pTextView);

// Sets the width of the line numbers.
void dred_textview_set_line_numbers_width(dred_textview* pTextView, float lineNumbersWidth);

// Retrieves the width of the line numbers.
float dred_textview_get_line_numbers_width(dred_textview* pTextView);

// Sets the padding to apply between the line numbers and the text.
void dred_textview_set_line_numbers_padding(dred_textview* pTextView, float lineNumbersPadding);

// Retrieves the padding to apply between the line numbers and the text.
float dred_textview_get_line_numbers_padding(dred_textview* pTextView);

// Sets the color of the text of the line numbers.
void dred_textview_set_line_numbers_color(dred_textview* pTextView, dred_color color);

// Retrieves the color of the text of the line numbers.
dred_color dred_textview_get_line_numbers_color(dred_textview* pTextView);

// Sets the color of the background of the line numbers.
void dred_textview_set_line_numbers_background_color(dred_textview* pTextView, dred_color color);

// Retrieves the color of the background of the line numbers.
dred_color dred_textview_get_line_numbers_background_color(dred_textview* pTextView);

// Sets the color of the track of the scrollbars.
void dred_textview_set_scrollbar_track_color(dred_textview* pTextView, dred_color color);

// Sets the color of the thumb of the scrollbars.
void dred_textview_set_scrollbar_thumb_color(dred_textview* pTextView, dred_color color);

// Sets the color of the thumb of the scrollbars while hovered.
void dred_textview_set_scrollbar_thumb_color_hovered(dred_textview* pTextView, dred_color color);

// Sets the color of the thumb of the scrollbars while pressed.
void dred_textview_set_scrollbar_thumb_color_pressed(dred_textview* pTextView, dred_color color);


// Enables word wrap.
void dred_textview_enable_word_wrap(dred_textview* pTextView);

// Disables word wrap.
void dred_textview_disable_word_wrap(dred_textview* pTextView);

// Determines whether or not word wrap is enabled.
dr_bool32 dred_textview_is_word_wrap_enabled(dred_textview* pTextView);


// Enables drag-and-drop.
void dred_textview_enable_drag_and_drop(dred_textview* pTextView);

// Disables drag-and-drop.
void dred_textview_disable_drag_and_drop(dred_textview* pTextView);

// Determines whether or not drag and drop is enabled.
dr_bool32 dred_textview_is_drag_and_drop_enabled(dred_textview* pTextView);



// Sets the text of the given text box.
void dred_textview_set_text(dred_textview* pTextView, const char* text);

// Retrieves the text of the given text box.
size_t dred_textview_get_text(dred_textview* pTextView, char* pTextOut, size_t textOutSize);

// Steps the text box to allow it to blink the cursor.
void dred_textview_step(dred_textview* pTextView, unsigned int milliseconds);

// Sets the blink rate of the cursor in milliseconds.
void dred_textview_set_cursor_blink_rate(dred_textview* pTextView, unsigned int blinkRateInMilliseconds);

// Moves the caret to the end of the text.
void dred_textview_move_cursor_to_end_of_text(dred_textview* pTextView);

// Moves the caret to the beginning of the line at the given index.
void dred_textview_move_cursor_to_start_of_line_by_index(dred_textview* pTextView, size_t iLine);

// Moves the caret to the beginnning of the unwrapped line at the given index.
void dred_textview_move_cursor_to_start_of_unwrapped_line_by_index(dred_textview* pTextView, size_t iLine);

// Determines whether or not anything is selected in the given text box.
dr_bool32 dred_textview_is_anything_selected(dred_textview* pTextView);

// Selects all of the text inside the text box.
void dred_textview_select_all(dred_textview* pTextView);

// Deselect everything.
void dred_textview_deselect_all(dred_textview* pTextView);

// Retrieves a copy of the selected text.
//
// @remarks
//     This returns the length of the selected text. Call this once with <textOut> set to NULL to calculate the required size of the
//     buffer.
//     @par
//     If the output buffer is not larger enough, the string will be truncated.
size_t dred_textview_get_selected_text(dred_textview* pTextView, char* textOut, size_t textOutLength);

// Deletes the character to the right of the cursor.
//
// @return True if the text within the text engine has changed.
dr_bool32 dred_textview_delete_character_to_right_of_cursor(dred_textview* pTextView);

// Deletes the currently selected text.
//
// @return True if the text within the text engine has changed.
dr_bool32 dred_textview_delete_selected_text_no_undo(dred_textview* pTextView);
dr_bool32 dred_textview_delete_selected_text(dred_textview* pTextView);

// Inserts a character at the position of the cursor.
//
// @return True if the text within the text engine has changed.
dr_bool32 dred_textview_insert_text_at_cursors_no_undo(dred_textview* pTextView, const char* text);
dr_bool32 dred_textview_insert_text_at_cursors(dred_textview* pTextView, const char* text);

// Unindents selected text.
dr_bool32 dred_textview_unindent_selected_blocks(dred_textview* pTextView);

// Performs an undo operation.
dr_bool32 dred_textview_undo(dred_textview* pTextView);

// Performs a redo operation.
dr_bool32 dred_textview_redo(dred_textview* pTextView);

// Retrieves the number of undo points remaining.
unsigned int dred_textview_get_undo_points_remaining_count(dred_textview* pTextView);

// Retrieves the number of redo points remaining.
unsigned int dred_textview_get_redo_points_remaining_count(dred_textview* pTextView);

// Clears the undo/redo stack.
void dred_textview_clear_undo_stack(dred_textview* pTextView);

// Retrieves the index of the line the cursor is current sitting on.
size_t dred_textview_get_cursor_line(dred_textview* pTextView);

// Retrieves the index of the column the cursor is current sitting on.
size_t dred_textview_get_cursor_column(dred_textview* pTextView);

// Retrieves the number of lines in the given text box.
size_t dred_textview_get_line_count(dred_textview* pTextView);


// Finds and selects the next occurance of the given string, starting from the cursor and looping back to the start.
dr_bool32 dred_textview_find_and_select_next(dred_textview* pTextView, const char* text);

// Finds the next occurance of the given string and replaces it with another.
dr_bool32 dred_textview_find_and_replace_next(dred_textview* pTextView, const char* text, const char* replacement);

// Finds every occurance of the given string and replaces it with another.
dr_bool32 dred_textview_find_and_replace_all(dred_textview* pTextView, const char* text, const char* replacement);


// Shows the line numbers.
void dred_textview_show_line_numbers(dred_textview* pTextView);

// Hides the line numbers.
void dred_textview_hide_line_numbers(dred_textview* pTextView);


// Disables the vertical scrollbar.
void dred_textview_disable_vertical_scrollbar(dred_textview* pTextView);

// Enables the vertical scrollbar.
void dred_textview_enable_vertical_scrollbar(dred_textview* pTextView);

// Disables the horizontal scrollbar.
void dred_textview_disable_horizontal_scrollbar(dred_textview* pTextView);

// Enables the horizontal scrollbar.
void dred_textview_enable_horizontal_scrollbar(dred_textview* pTextView);

// Retrieves the vertical scrollbar.
dred_scrollbar* dred_textview_get_vertical_scrollbar(dred_textview* pTextView);

// Retrieves the horizontal scrollbar.
dred_scrollbar* dred_textview_get_horizontal_scrollbar(dred_textview* pTextView);

// Sets the size of both the vertical and horizontal scrollbars.
void dred_textview_set_scrollbar_size(dred_textview* pTextView, float size);

// Enables excess scrolling on the vertical scrollbar.
void dred_textview_enable_excess_scrolling(dred_textview* pTextView);

// Disables excess scrolling on the vertical scrollbar.
void dred_textview_disable_excess_scrolling(dred_textview* pTextView);


// Determines whether or not a wrapped line is currently inside the view.
dr_bool32 dred_textview_is_line_in_view(dred_textview* pTextView, size_t iLine);

// Determines whether or not the unwrapped line is currently inside the view.
dr_bool32 dred_textview_is_unwrapped_line_in_view(dred_textview* pTextView, size_t iLine);

// Determines whether or not the active cursor is currently inside the view.
dr_bool32 dred_textview_is_cursor_in_view(dred_textview* pTextView);

// Centers the view onto the active cursor.
void dred_textview_center_on_cursor(dred_textview* pTextView);


// Sets the size of tabs in spaces.
void dred_textview_set_tab_size_in_spaces(dred_textview* pTextView, unsigned int tabSizeInSpaces);

// Retrieves the size of tabs in spaces.
unsigned int dred_textview_get_tab_size_in_spaces(dred_textview* pTextView);

// Enables tabs-to-spaces.
void dred_textview_enable_tabs_to_spaces(dred_textview* pTextView);

// Disables tab-to-spaces.
void dred_textview_disable_tabs_to_spaces(dred_textview* pTextView);

// Determines whether or not the textbox should convert tabs to spaces.
dr_bool32 dred_textview_is_tabs_to_spaces_enabled(dred_textview* pTextView);


// Prepares an undo point.
dr_bool32 dred_textview_prepare_undo_point(dred_textview* pTextView);

// Commits an undo point.
dr_bool32 dred_textview_commit_undo_point(dred_textview* pTextView);


// Sets the function to call when the cursor moves.
void dred_textview_set_on_cursor_move(dred_textview* pTextView, dred_textview_on_cursor_move_proc proc);

// Sets the function to call when the undo point changes.
void dred_textview_set_on_undo_point_changed(dred_textview* pTextView, dred_textview_on_undo_point_changed_proc proc);



// on_size.
void dred_textview_on_size(dred_control* pControl, float newWidth, float newHeight);

// on_mouse_move.
void dred_textview_on_mouse_move(dred_control* pControl, int relativeMousePosX, int relativeMousePosY, int stateFlags);

// on_mouse_button_down.
void dred_textview_on_mouse_button_down(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

// on_mouse_button_up.
void dred_textview_on_mouse_button_up(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

// on_mouse_button_dblclick.
void dred_textview_on_mouse_button_dblclick(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

// on_mouse_wheel
void dred_textview_on_mouse_wheel(dred_control* pControl, int delta, int relativeMousePosX, int relativeMousePosY, int stateFlags);

// on_key_down.
void dred_textview_on_key_down(dred_control* pControl, dred_key key, int stateFlags);

// on_key_up.
void dred_textview_on_key_up(dred_control* pControl, dred_key key, int stateFlags);

// on_printable_key_down.
void dred_textview_on_printable_key_down(dred_control* pControl, unsigned int utf32, int stateFlags);

// on_paint.
void dred_textview_on_paint(dred_control* pControl, dred_rect relativeRect, void* pPaintData);

// on_capture_keyboard
void dred_textview_on_capture_keyboard(dred_control* pControl, dred_control* pPrevCapturedControl);

// on_release_keyboard
void dred_textview_on_release_keyboard(dred_control* pControl, dred_control* pNewCapturedControl);

// on_capture_mouse
void dred_textview_on_capture_mouse(dred_control* pControl);

// on_release_mouse
void dred_textview_on_release_mouse(dred_control* pControl);

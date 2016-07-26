// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_TEXTBOX  "dred.textview.textbox"

typedef struct dred_textbox dred_textbox;
#define DRED_TEXTBOX(a) ((dred_textbox*)(a))

struct dred_textbox
{
    // A text box control is just a text view control with a text engine.
    dred_textview view;

    // The text engine.
    drte_engine textEngine;
    drte_engine* pTextEngine;
};


// Creates a new text box control.
bool dred_textbox_init(dred_textbox* pTextBox, dred_context* pDred, dred_control* pParent);

// Deletes the given text box control.
void dred_textbox_uninit(dred_textbox* pTextBox);


// Retrieves a pointer to the internal text engine.
drte_engine* dred_textbox_get_engine(dred_textbox* pTextBox);


// Sets the font to use with the given text box.
void dred_textbox_set_font(dred_textbox* pTextBox, dred_gui_font* pFont);

// Retrieves the font being used with the given text box.
dred_gui_font* dred_textbox_get_font(dred_textbox* pTextBox);

// Sets the color of the text in teh given text box.
void dred_textbox_set_text_color(dred_textbox* pTextBox, dred_color color);

// Sets the background color of the given text box.
void dred_textbox_set_background_color(dred_textbox* pTextBox, dred_color color);

// Sets the background color of selected text.
void dred_textbox_set_selection_background_color(dred_textbox* pTextBox, dred_color color);

// Retrieves the background color of selected text.
dred_color dred_textbox_get_selection_background_color(dred_textbox* pTextBox);

// Sets the background color for the line the caret is currently sitting on.
void dred_textbox_set_active_line_background_color(dred_textbox* pTextBox, dred_color color);

// Sets the width of the text cursor.
void dred_textbox_set_cursor_width(dred_textbox* pTextBox, float cursorWidth);

// Retrieves the width of the text cursor.
float dred_textbox_get_cursor_width(dred_textbox* pTextBox);

// Sets the color of the cursor of the given text box.
void dred_textbox_set_cursor_color(dred_textbox* pTextBox, dred_color color);

// Sets the border color of the given text box.
void dred_textbox_set_border_color(dred_textbox* pTextBox, dred_color color);

// Sets the border width of the given text box.
void dred_textbox_set_border_width(dred_textbox* pTextBox, float borderWidth);

// Sets the amount of padding to apply to given text box.
void dred_textbox_set_padding(dred_textbox* pTextBox, float padding);

// Retrieves the amound of vertical padding to apply to the given text box.
float dred_textbox_get_padding_vert(dred_textbox* pTextBox);

// Retrieves the amound of horizontal padding to apply to the given text box.
float dred_textbox_get_padding_horz(dred_textbox* pTextBox);

// Sets the width of the line numbers.
void dred_textbox_set_line_numbers_width(dred_textbox* pTextBox, float lineNumbersWidth);

// Retrieves the width of the line numbers.
float dred_textbox_get_line_numbers_width(dred_textbox* pTextBox);

// Sets the padding to apply between the line numbers and the text.
void dred_textbox_set_line_numbers_padding(dred_textbox* pTextBox, float lineNumbersPadding);

// Retrieves the padding to apply between the line numbers and the text.
float dred_textbox_get_line_numbers_padding(dred_textbox* pTextBox);

// Sets the color of the text of the line numbers.
void dred_textbox_set_line_numbers_color(dred_textbox* pTextBox, dred_color color);

// Retrieves the color of the text of the line numbers.
dred_color dred_textbox_get_line_numbers_color(dred_textbox* pTextBox);

// Sets the color of the background of the line numbers.
void dred_textbox_set_line_numbers_background_color(dred_textbox* pTextBox, dred_color color);

// Retrieves the color of the background of the line numbers.
dred_color dred_textbox_get_line_numbers_background_color(dred_textbox* pTextBox);

// Sets the color of the track of the scrollbars.
void dred_textbox_set_scrollbar_track_color(dred_textbox* pTextBox, dred_color color);

// Sets the color of the thumb of the scrollbars.
void dred_textbox_set_scrollbar_thumb_color(dred_textbox* pTextBox, dred_color color);

// Sets the color of the thumb of the scrollbars while hovered.
void dred_textbox_set_scrollbar_thumb_color_hovered(dred_textbox* pTextBox, dred_color color);

// Sets the color of the thumb of the scrollbars while pressed.
void dred_textbox_set_scrollbar_thumb_color_pressed(dred_textbox* pTextBox, dred_color color);


// Enables word wrap.
void dred_textbox_enable_word_wrap(dred_textbox* pTextBox);

// Disables word wrap.
void dred_textbox_disable_word_wrap(dred_textbox* pTextBox);

// Determines whether or not word wrap is enabled.
bool dred_textbox_is_word_wrap_enabled(dred_textbox* pTextBox);


// Sets the text of the given text box.
void dred_textbox_set_text(dred_textbox* pTextBox, const char* text);

// Retrieves the text of the given text box.
size_t dred_textbox_get_text(dred_textbox* pTextBox, char* pTextOut, size_t textOutSize);

// Steps the text box to allow it to blink the cursor.
void dred_textbox_step(dred_textbox* pTextBox, unsigned int milliseconds);

// Sets the blink rate of the cursor in milliseconds.
void dred_textbox_set_cursor_blink_rate(dred_textbox* pTextBox, unsigned int blinkRateInMilliseconds);

// Moves the caret to the end of the text.
void dred_textbox_move_cursor_to_end_of_text(dred_textbox* pTextBox);

// Moves the caret to the beginning of the line at the given index.
void dred_textbox_move_cursor_to_start_of_line_by_index(dred_textbox* pTextBox, size_t iLine);

// Determines whether or not anything is selected in the given text box.
bool dred_textbox_is_anything_selected(dred_textbox* pTextBox);

// Selects all of the text inside the text box.
void dred_textbox_select_all(dred_textbox* pTextBox);

// Deselect everything.
void dred_textbox_deselect_all(dred_textbox* pTextBox);

// Retrieves a copy of the selected text.
//
// @remarks
//     This returns the length of the selected text. Call this once with <textOut> set to NULL to calculate the required size of the
//     buffer.
//     @par
//     If the output buffer is not larger enough, the string will be truncated.
size_t dred_textbox_get_selected_text(dred_textbox* pTextBox, char* textOut, size_t textOutLength);

// Deletes the character to the right of the cursor.
//
// @return True if the text within the text engine has changed.
bool dred_textbox_delete_character_to_right_of_cursor(dred_textbox* pTextBox);

// Deletes the currently selected text.
//
// @return True if the text within the text engine has changed.
bool dred_textbox_delete_selected_text_no_undo(dred_textbox* pTextBox);
bool dred_textbox_delete_selected_text(dred_textbox* pTextBox);

// Inserts a character at the position of the cursor.
//
// @return True if the text within the text engine has changed.
bool dred_textbox_insert_text_at_cursors_no_undo(dred_textbox* pTextBox, const char* text);
bool dred_textbox_insert_text_at_cursors(dred_textbox* pTextBox, const char* text);

// Unindents selected text.
bool dred_textbox_unindent_selected_blocks(dred_textbox* pTextBox);

// Performs an undo operation.
bool dred_textbox_undo(dred_textbox* pTextBox);

// Performs a redo operation.
bool dred_textbox_redo(dred_textbox* pTextBox);

// Retrieves the number of undo points remaining.
unsigned int dred_textbox_get_undo_points_remaining_count(dred_textbox* pTextBox);

// Retrieves the number of redo points remaining.
unsigned int dred_textbox_get_redo_points_remaining_count(dred_textbox* pTextBox);

// Clears the undo/redo stack.
void dred_textbox_clear_undo_stack(dred_textbox* pTextBox);

// Retrieves the index of the line the cursor is current sitting on.
size_t dred_textbox_get_cursor_line(dred_textbox* pTextBox);

// Retrieves the index of the column the cursor is current sitting on.
size_t dred_textbox_get_cursor_column(dred_textbox* pTextBox);

// Retrieves the number of lines in the given text box.
size_t dred_textbox_get_line_count(dred_textbox* pTextBox);


// Finds and selects the next occurance of the given string, starting from the cursor and looping back to the start.
bool dred_textbox_find_and_select_next(dred_textbox* pTextBox, const char* text);

// Finds the next occurance of the given string and replaces it with another.
bool dred_textbox_find_and_replace_next(dred_textbox* pTextBox, const char* text, const char* replacement);

// Finds every occurance of the given string and replaces it with another.
bool dred_textbox_find_and_replace_all(dred_textbox* pTextBox, const char* text, const char* replacement);


// Shows the line numbers.
void dred_textbox_show_line_numbers(dred_textbox* pTextBox);

// Hides the line numbers.
void dred_textbox_hide_line_numbers(dred_textbox* pTextBox);


// Disables the vertical scrollbar.
void dred_textbox_disable_vertical_scrollbar(dred_textbox* pTextBox);

// Enables the vertical scrollbar.
void dred_textbox_enable_vertical_scrollbar(dred_textbox* pTextBox);

// Disables the horizontal scrollbar.
void dred_textbox_disable_horizontal_scrollbar(dred_textbox* pTextBox);

// Enables the horizontal scrollbar.
void dred_textbox_enable_horizontal_scrollbar(dred_textbox* pTextBox);

// Retrieves the vertical scrollbar.
dred_scrollbar* dred_textbox_get_vertical_scrollbar(dred_textbox* pTextBox);

// Retrieves the horizontal scrollbar.
dred_scrollbar* dred_textbox_get_horizontal_scrollbar(dred_textbox* pTextBox);

// Sets the size of both the vertical and horizontal scrollbars.
void dred_textbox_set_scrollbar_size(dred_textbox* pTextBox, float size);

// Enables excess scrolling on the vertical scrollbar.
void dred_textbox_enable_excess_scrolling(dred_textbox* pTextBox);

// Disables excess scrolling on the vertical scrollbar.
void dred_textbox_disable_excess_scrolling(dred_textbox* pTextBox);


// Sets the size of tabs in spaces.
void dred_textbox_set_tab_size_in_spaces(dred_textbox* pTextBox, unsigned int tabSizeInSpaces);

// Retrieves the size of tabs in spaces.
unsigned int dred_textbox_get_tab_size_in_spaces(dred_textbox* pTextBox);

// Enables tabs-to-spaces.
void dred_textbox_enable_tabs_to_spaces(dred_textbox* pTextBox);

// Disables tab-to-spaces.
void dred_textbox_disable_tabs_to_spaces(dred_textbox* pTextBox);

// Determines whether or not the textbox should convert tabs to spaces.
bool dred_textbox_is_tabs_to_spaces_enabled(dred_textbox* pTextBox);


// Prepares an undo point.
bool dred_textbox_prepare_undo_point(dred_textbox* pTextBox);

// Commits an undo point.
bool dred_textbox_commit_undo_point(dred_textbox* pTextBox);

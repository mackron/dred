// Copyright (C) 2016 David Reid. See included LICENSE file.

bool dred_textbox_init(dred_textbox* pTextBox, dred_context* pDred, dred_control* pParent)
{
    if (pTextBox == NULL) {
        return false;
    }

    memset(pTextBox, 0, sizeof(*pTextBox));

    pTextBox->pTextEngine = &pTextBox->textEngine;
    if (!drte_engine_init(pTextBox->pTextEngine, NULL)) {
        return false;
    }

    if (!dred_textview_init(DRED_TEXTVIEW(pTextBox), pDred, pParent, pTextBox->pTextEngine)) {
        return false;
    }

    dred_control_set_type(DRED_CONTROL(pTextBox), DRED_CONTROL_TYPE_TEXTBOX);
    return true;
}

void dred_textbox_uninit(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    dred_textview_uninit(DRED_TEXTVIEW(pTextBox));

    if (pTextBox->pTextEngine) {
        drte_engine_uninit(pTextBox->pTextEngine);
        pTextBox->pTextEngine = NULL;
    }
}


drte_engine* dred_textbox_get_engine(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return NULL;
    }

    return pTextBox->pTextEngine;
}


void dred_textbox_set_font(dred_textbox* pTextBox, dred_gui_font* pFont)
{
    dred_textview_set_font(DRED_TEXTVIEW(pTextBox), pFont);
}

dred_gui_font* dred_textbox_get_font(dred_textbox* pTextBox)
{
    return dred_textview_get_font(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_set_text_color(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_text_color(DRED_TEXTVIEW(pTextBox), color);
}

void dred_textbox_set_background_color(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_background_color(DRED_TEXTVIEW(pTextBox), color);
}

void dred_textbox_set_selection_background_color(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_selection_background_color(DRED_TEXTVIEW(pTextBox), color);
}

dred_color dred_textbox_get_selection_background_color(dred_textbox* pTextBox)
{
    return dred_textview_get_selection_background_color(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_set_active_line_background_color(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_active_line_background_color(DRED_TEXTVIEW(pTextBox), color);
}

void dred_textbox_set_cursor_width(dred_textbox* pTextBox, float cursorWidth)
{
    dred_textview_set_cursor_width(DRED_TEXTVIEW(pTextBox), cursorWidth);
}

float dred_textbox_get_cursor_width(dred_textbox* pTextBox)
{
    return dred_textview_get_cursor_width(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_set_cursor_color(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_cursor_color(DRED_TEXTVIEW(pTextBox), color);
}

void dred_textbox_set_border_color(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_border_color(DRED_TEXTVIEW(pTextBox), color);
}

void dred_textbox_set_border_width(dred_textbox* pTextBox, float borderWidth)
{
    dred_textview_set_border_width(DRED_TEXTVIEW(pTextBox), borderWidth);
}

void dred_textbox_set_padding(dred_textbox* pTextBox, float padding)
{
    dred_textview_set_padding(DRED_TEXTVIEW(pTextBox), padding);
}

float dred_textbox_get_padding_vert(dred_textbox* pTextBox)
{
    return dred_textview_get_padding_vert(DRED_TEXTVIEW(pTextBox));
}

float dred_textbox_get_padding_horz(dred_textbox* pTextBox)
{
    return dred_textview_get_padding_horz(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_set_line_numbers_width(dred_textbox* pTextBox, float lineNumbersWidth)
{
    dred_textview_set_line_numbers_width(DRED_TEXTVIEW(pTextBox), lineNumbersWidth);
}

float dred_textbox_get_line_numbers_width(dred_textbox* pTextBox)
{
    return dred_textview_get_line_numbers_width(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_set_line_numbers_padding(dred_textbox* pTextBox, float lineNumbersPadding)
{
    dred_textview_set_line_numbers_padding(DRED_TEXTVIEW(pTextBox), lineNumbersPadding);
}

float dred_textbox_get_line_numbers_padding(dred_textbox* pTextBox)
{
    return dred_textview_get_line_numbers_padding(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_set_line_numbers_color(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_line_numbers_color(DRED_TEXTVIEW(pTextBox), color);
}

dred_color dred_textbox_get_line_numbers_color(dred_textbox* pTextBox)
{
    return dred_textview_get_line_numbers_color(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_set_line_numbers_background_color(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_line_numbers_background_color(DRED_TEXTVIEW(pTextBox), color);
}

dred_color dred_textbox_get_line_numbers_background_color(dred_textbox* pTextBox)
{
    return dred_textview_get_line_numbers_background_color(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_set_scrollbar_track_color(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_scrollbar_track_color(DRED_TEXTVIEW(pTextBox), color);
}

void dred_textbox_set_scrollbar_thumb_color(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_scrollbar_thumb_color(DRED_TEXTVIEW(pTextBox), color);
}

void dred_textbox_set_scrollbar_thumb_color_hovered(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_scrollbar_thumb_color_hovered(DRED_TEXTVIEW(pTextBox), color);
}

void dred_textbox_set_scrollbar_thumb_color_pressed(dred_textbox* pTextBox, dred_color color)
{
    dred_textview_set_scrollbar_thumb_color_pressed(DRED_TEXTVIEW(pTextBox), color);
}


void dred_textbox_enable_word_wrap(dred_textbox* pTextBox)
{
    dred_textview_enable_word_wrap(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_disable_word_wrap(dred_textbox* pTextBox)
{
    dred_textview_disable_word_wrap(DRED_TEXTVIEW(pTextBox));
}

bool dred_textbox_is_word_wrap_enabled(dred_textbox* pTextBox)
{
    return dred_textview_is_word_wrap_enabled(DRED_TEXTVIEW(pTextBox));
}



void dred_textbox_set_text(dred_textbox* pTextBox, const char* text)
{
    dred_textview_set_text(DRED_TEXTVIEW(pTextBox), text);
}

size_t dred_textbox_get_text(dred_textbox* pTextBox, char* pTextOut, size_t textOutSize)
{
    return dred_textview_get_text(DRED_TEXTVIEW(pTextBox), pTextOut, textOutSize);
}

void dred_textbox_step(dred_textbox* pTextBox, unsigned int milliseconds)
{
    dred_textview_step(DRED_TEXTVIEW(pTextBox), milliseconds);
}

void dred_textbox_set_cursor_blink_rate(dred_textbox* pTextBox, unsigned int blinkRateInMilliseconds)
{
    dred_textview_set_cursor_blink_rate(DRED_TEXTVIEW(pTextBox), blinkRateInMilliseconds);
}

void dred_textbox_move_cursor_to_end_of_text(dred_textbox* pTextBox)
{
    dred_textview_move_cursor_to_end_of_text(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_move_cursor_to_start_of_line_by_index(dred_textbox* pTextBox, size_t iLine)
{
    dred_textview_move_cursor_to_start_of_line_by_index(DRED_TEXTVIEW(pTextBox), iLine);
}


bool dred_textbox_is_anything_selected(dred_textbox* pTextBox)
{
    return dred_textview_is_anything_selected(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_select_all(dred_textbox* pTextBox)
{
    dred_textview_select_all(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_deselect_all(dred_textbox* pTextBox)
{
    dred_textview_deselect_all(DRED_TEXTVIEW(pTextBox));
}

size_t dred_textbox_get_selected_text(dred_textbox* pTextBox, char* textOut, size_t textOutLength)
{
    return dred_textview_get_selected_text(DRED_TEXTVIEW(pTextBox), textOut, textOutLength);
}

bool dred_textbox_delete_character_to_right_of_cursor(dred_textbox* pTextBox)
{
    return dred_textview_delete_character_to_right_of_cursor(DRED_TEXTVIEW(pTextBox));
}

bool dred_textbox_delete_selected_text_no_undo(dred_textbox* pTextBox)
{
    return dred_textview_delete_selected_text_no_undo(DRED_TEXTVIEW(pTextBox));
}

bool dred_textbox_delete_selected_text(dred_textbox* pTextBox)
{
    return dred_textview_delete_selected_text(DRED_TEXTVIEW(pTextBox));
}

bool dred_textbox_insert_text_at_cursors_no_undo(dred_textbox* pTextBox, const char* text)
{
    return dred_textview_insert_text_at_cursors_no_undo(DRED_TEXTVIEW(pTextBox), text);
}

bool dred_textbox_insert_text_at_cursors(dred_textbox* pTextBox, const char* text)
{
    return dred_textview_insert_text_at_cursors(DRED_TEXTVIEW(pTextBox), text);
}

bool dred_textbox_unindent_selected_blocks(dred_textbox* pTextBox)
{
    return dred_textview_unindent_selected_blocks(DRED_TEXTVIEW(pTextBox));
}

bool dred_textbox_undo(dred_textbox* pTextBox)
{
    return dred_textview_undo(DRED_TEXTVIEW(pTextBox));
}

bool dred_textbox_redo(dred_textbox* pTextBox)
{
    return dred_textview_redo(DRED_TEXTVIEW(pTextBox));
}

unsigned int dred_textbox_get_undo_points_remaining_count(dred_textbox* pTextBox)
{
    return dred_textview_get_undo_points_remaining_count(DRED_TEXTVIEW(pTextBox));
}

unsigned int dred_textbox_get_redo_points_remaining_count(dred_textbox* pTextBox)
{
    return dred_textview_get_redo_points_remaining_count(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_clear_undo_stack(dred_textbox* pTextBox)
{
    dred_textview_clear_undo_stack(DRED_TEXTVIEW(pTextBox));
}


size_t dred_textbox_get_cursor_line(dred_textbox* pTextBox)
{
    return dred_textview_get_cursor_line(DRED_TEXTVIEW(pTextBox));
}

size_t dred_textbox_get_cursor_column(dred_textbox* pTextBox)
{
    return dred_textview_get_cursor_column(DRED_TEXTVIEW(pTextBox));
}

size_t dred_textbox_get_line_count(dred_textbox* pTextBox)
{
    return dred_textview_get_line_count(DRED_TEXTVIEW(pTextBox));
}


bool dred_textbox_find_and_select_next(dred_textbox* pTextBox, const char* text)
{
    return dred_textview_find_and_select_next(DRED_TEXTVIEW(pTextBox), text);
}

bool dred_textbox_find_and_replace_next(dred_textbox* pTextBox, const char* text, const char* replacement)
{
    return dred_textview_find_and_replace_next(DRED_TEXTVIEW(pTextBox), text, replacement);
}

bool dred_textbox_find_and_replace_all(dred_textbox* pTextBox, const char* text, const char* replacement)
{
    return dred_textview_find_and_replace_all(DRED_TEXTVIEW(pTextBox), text, replacement);
}


void dred_textbox_show_line_numbers(dred_textbox* pTextBox)
{
    dred_textview_show_line_numbers(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_hide_line_numbers(dred_textbox* pTextBox)
{
    dred_textview_hide_line_numbers(DRED_TEXTVIEW(pTextBox));
}


void dred_textbox_disable_vertical_scrollbar(dred_textbox* pTextBox)
{
    dred_textview_disable_vertical_scrollbar(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_enable_vertical_scrollbar(dred_textbox* pTextBox)
{
    dred_textview_enable_vertical_scrollbar(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_disable_horizontal_scrollbar(dred_textbox* pTextBox)
{
    dred_textview_disable_horizontal_scrollbar(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_enable_horizontal_scrollbar(dred_textbox* pTextBox)
{
    dred_textview_enable_horizontal_scrollbar(DRED_TEXTVIEW(pTextBox));
}

dred_scrollbar* dred_textbox_get_vertical_scrollbar(dred_textbox* pTextBox)
{
    return dred_textview_get_vertical_scrollbar(DRED_TEXTVIEW(pTextBox));
}

dred_scrollbar* dred_textbox_get_horizontal_scrollbar(dred_textbox* pTextBox)
{
    return dred_textview_get_horizontal_scrollbar(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_set_scrollbar_size(dred_textbox* pTextBox, float size)
{
    dred_textview_set_scrollbar_size(DRED_TEXTVIEW(pTextBox), size);
}

void dred_textbox_enable_excess_scrolling(dred_textbox* pTextBox)
{
    dred_textview_enable_excess_scrolling(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_disable_excess_scrolling(dred_textbox* pTextBox)
{
    dred_textview_disable_excess_scrolling(DRED_TEXTVIEW(pTextBox));
}


void dred_textbox_set_tab_size_in_spaces(dred_textbox* pTextBox, unsigned int tabSizeInSpaces)
{
    dred_textview_set_tab_size_in_spaces(DRED_TEXTVIEW(pTextBox), tabSizeInSpaces);
}

unsigned int dred_textbox_get_tab_size_in_spaces(dred_textbox* pTextBox)
{
    return dred_textview_get_tab_size_in_spaces(DRED_TEXTVIEW(pTextBox));
}


void dred_textbox_enable_tabs_to_spaces(dred_textbox* pTextBox)
{
    dred_textview_enable_tabs_to_spaces(DRED_TEXTVIEW(pTextBox));
}

void dred_textbox_disable_tabs_to_spaces(dred_textbox* pTextBox)
{
    dred_textview_disable_tabs_to_spaces(DRED_TEXTVIEW(pTextBox));
}

bool dred_textbox_is_tabs_to_spaces_enabled(dred_textbox* pTextBox)
{
    return dred_textview_is_tabs_to_spaces_enabled(DRED_TEXTVIEW(pTextBox));
}


bool dred_textbox_prepare_undo_point(dred_textbox* pTextBox)
{
    return dred_textview_prepare_undo_point(DRED_TEXTVIEW(pTextBox));
}

bool dred_textbox_commit_undo_point(dred_textbox* pTextBox)
{
    return dred_textview_commit_undo_point(DRED_TEXTVIEW(pTextBox));
}

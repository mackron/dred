#include "dred_textbox.h"

typedef struct
{
    drgui_element* pInternalTB;
    dred_timer* pTimer;
    dred_textbox_on_cursor_move_proc onCursorMove;
    dred_textbox_on_undo_point_changed_proc onUndoPointChanged;
} dred_textbox_data;

drgui_element* dred_textbox__get_internal_tb(dred_textbox* pTextBox)
{
    dred_textbox_data* data = dred_control_get_extra_data(pTextBox);
    if (data == NULL) {
        return NULL;
    }

    return data->pInternalTB;
}


void dred_textbox__on_timer(dred_timer* pTimer, void* pUserData)
{
    (void)pTimer;

    dred_textbox* pTextBox = (dred_textbox*)pUserData;
    assert(pTextBox != NULL);

    dred_textbox_data* data = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(data != NULL);

    drgui_textbox_step(data->pInternalTB, 100);
}

void dred_textbox__on_capture_keyboard(dred_textbox* pTextBox, drgui_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;

    // The internal text box needs to be given the keyboard focus whenever a dred_textbox receives it.
    dred_textbox_data* data = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (data == NULL) {
        return;
    }

    drgui_capture_keyboard(data->pInternalTB);
}


void dred_textbox_innertb__on_mouse_wheel(drgui_element* pInternalTB, int delta, int mousePosX, int mousePosY, int stateFlags)
{
    dred_textbox* pTextBox = dred_control_get_parent(pInternalTB);
    assert(pTextBox != NULL);

    if (pTextBox->onMouseWheel) {
        pTextBox->onMouseWheel(pTextBox, delta, mousePosX, mousePosY, stateFlags);
    } else {
        drgui_textbox_on_mouse_wheel(pInternalTB, delta, mousePosX, mousePosY, stateFlags);
    }
}

void dred_textbox_innertb__on_key_down(drgui_element* pInternalTB, drgui_key key, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = dred_control_get_parent(pInternalTB);
    assert(pTextBox != NULL);

    if (pTextBox->onKeyDown) {
        pTextBox->onKeyDown(pTextBox, key, stateFlags);
    } else {
        drgui_textbox_on_key_down(pInternalTB, key, stateFlags);
    }
}

void dred_textbox_innertb__on_key_up(drgui_element* pInternalTB, drgui_key key, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = dred_control_get_parent(pInternalTB);
    assert(pTextBox != NULL);

    if (pTextBox->onKeyUp) {
        pTextBox->onKeyUp(pTextBox, key, stateFlags);
    } else {
        drgui_textbox_on_key_up(pInternalTB, key, stateFlags);
    }
}

void dred_textbox_innertb__on_printable_key_down(drgui_element* pInternalTB, uint32_t utf32, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = dred_control_get_parent(pInternalTB);
    assert(pTextBox != NULL);

    if (pTextBox->onPrintableKeyDown) {
        pTextBox->onPrintableKeyDown(pTextBox, utf32, stateFlags);
    } else {
        drgui_textbox_on_printable_key_down(pInternalTB, utf32, stateFlags);
    }
}

void dred_textbox_innertb__on_capture_keyboard(drgui_element* pInternalTB, drgui_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;

    dred_textbox* pTextBox = dred_control_get_parent(pInternalTB);
    assert(pTextBox != NULL);

    dred_textbox_data* data = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(data != NULL);

    if (data->pTimer == NULL) {
        data->pTimer = dred_timer_create(100, dred_textbox__on_timer, pTextBox);
    }

    drgui_textbox_on_capture_keyboard(pInternalTB, pPrevCapturedElement);
}

void dred_textbox_innertb__on_release_keyboard(drgui_element* pInternalTB, drgui_element* pNewCapturedElement)
{
    dred_textbox* pTextBox = dred_control_get_parent(pInternalTB);
    assert(pTextBox != NULL);

    dred_textbox_data* data = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(data != NULL);

    if (data->pTimer != NULL) {
        dred_timer_delete(data->pTimer);
        data->pTimer = NULL;
    }

    drgui_textbox_on_release_keyboard(pInternalTB, pNewCapturedElement);
}

void dred_textbox_innertb__on_cursor_move(drgui_element* pInternalTB)
{
    dred_textbox* pTextBox = dred_control_get_parent(pInternalTB);
    assert(pTextBox != NULL);

    dred_textbox_data* data = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(data != NULL);

    if (data->onCursorMove) {
        data->onCursorMove(pTextBox);
    }
}

void dred_textbox_innertb__on_undo_point_changed(drgui_element* pInternalTB, unsigned int iUndoPoint)
{
    dred_textbox* pTextBox = dred_control_get_parent(pInternalTB);
    assert(pTextBox != NULL);

    dred_textbox_data* data = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(data != NULL);

    if (data->onUndoPointChanged) {
        data->onUndoPointChanged(pTextBox, iUndoPoint);
    }
}


dred_textbox* dred_textbox_create(dred_context* pDred, dred_control* pParent)
{
    dred_textbox* pTextBox = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_TEXTBOX, sizeof(dred_textbox_data));
    if (pTextBox == NULL) {
        return NULL;
    }

    dred_textbox_data* data = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(data != NULL);

    data->pInternalTB = drgui_create_textbox(pDred->pGUI, pTextBox, 0, NULL);
    if (data->pInternalTB == NULL) {
        dred_control_delete(pTextBox);
        return NULL;
    }

    data->pTimer = NULL;
    data->onCursorMove = NULL;
    data->onUndoPointChanged = NULL;

    drgui_textbox_set_background_color(data->pInternalTB, drgui_rgb(64, 64, 64));
    drgui_textbox_set_active_line_background_color(data->pInternalTB, drgui_rgb(48, 48, 48));
    drgui_textbox_set_text_color(data->pInternalTB, drgui_rgb(224, 224, 224));
    drgui_textbox_set_cursor_color(data->pInternalTB, drgui_rgb(224, 224, 224));
    drgui_textbox_set_cursor_width(data->pInternalTB, 1 * (float)pDred->uiScale);
    drgui_textbox_set_font(data->pInternalTB, dred_font_acquire_subfont(pDred->config.pUIFont, (float)pDred->uiScale)); // TODO: <-- This font needs to be unacquired.
    drgui_textbox_set_border_width(data->pInternalTB, 0);
    drgui_textbox_set_scrollbar_size(data->pInternalTB, 16 * (float)pDred->uiScale);


    // Events.
    dred_control_set_on_size(pTextBox, drgui_on_size_fit_children_to_parent);
    dred_control_set_on_capture_keyboard(pTextBox, dred_textbox__on_capture_keyboard);

    // Internal text box overrides.
    drgui_set_on_mouse_wheel(data->pInternalTB, dred_textbox_innertb__on_mouse_wheel);
    drgui_set_on_key_down(data->pInternalTB, dred_textbox_innertb__on_key_down);
    drgui_set_on_key_up(data->pInternalTB, dred_textbox_innertb__on_key_up);
    drgui_set_on_printable_key_down(data->pInternalTB, dred_textbox_innertb__on_printable_key_down);
    drgui_set_on_capture_keyboard(data->pInternalTB, dred_textbox_innertb__on_capture_keyboard);
    drgui_set_on_release_keyboard(data->pInternalTB, dred_textbox_innertb__on_release_keyboard);
    drgui_textbox_set_on_cursor_move(data->pInternalTB, dred_textbox_innertb__on_cursor_move);
    drgui_textbox_set_on_undo_point_changed(data->pInternalTB, dred_textbox_innertb__on_undo_point_changed);

    return pTextBox;
}

void dred_textbox_delete(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    dred_textbox_data* data = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (data != NULL) {
        // Keyboard focus needs to be released first. If we don't do this we'll not free delete the internal timer.
        if (drgui_has_keyboard_capture(data->pInternalTB)) {
            drgui_release_keyboard(data->pInternalTB->pContext);
        }

        drgui_delete_textbox(data->pInternalTB);
    }

    dred_control_delete(pTextBox);
}


bool dred_textbox_has_keyboard_capture(dred_textbox* pTextBox)
{
    return drgui_has_keyboard_capture(dred_textbox__get_internal_tb(pTextBox));
}


void dred_textbox_set_text(dred_textbox* pTextBox, const char* text)
{
    dred_textbox_data* data = dred_control_get_extra_data(pTextBox);
    if (data == NULL) {
        return;
    }

    drgui_textbox_set_text(data->pInternalTB, text);
}

size_t dred_textbox_get_text(dred_textbox* pTextBox, char* textOut, size_t textOutSize)
{
    dred_textbox_data* data = dred_control_get_extra_data(pTextBox);
    if (data == NULL) {
        return 0;
    }

    return drgui_textbox_get_text(data->pInternalTB, textOut, textOutSize);
}



void dred_textbox_set_font(dred_textbox* pTextBox, drgui_font* pFont)
{
    drgui_textbox_set_font(dred_textbox__get_internal_tb(pTextBox), pFont);
}

drgui_font* dred_textbox_get_font(dred_textbox* pTextBox)
{
    return drgui_textbox_get_font(dred_textbox__get_internal_tb(pTextBox));
}

void dred_textbox_set_text_color(dred_textbox* pTextBox, drgui_color color)
{
    drgui_textbox_set_text_color(dred_textbox__get_internal_tb(pTextBox), color);
}

void dred_textbox_set_background_color(dred_textbox* pTextBox, drgui_color color)
{
    drgui_textbox_set_background_color(dred_textbox__get_internal_tb(pTextBox), color);
}

void dred_textbox_set_active_line_background_color(dred_textbox* pTextBox, drgui_color color)
{
    drgui_textbox_set_active_line_background_color(dred_textbox__get_internal_tb(pTextBox), color);
}

void dred_textbox_set_cursor_width(dred_textbox* pTextBox, float cursorWidth)
{
    drgui_textbox_set_cursor_width(dred_textbox__get_internal_tb(pTextBox), cursorWidth);
}

float dred_textbox_get_cursor_width(dred_textbox* pTextBox)
{
    return drgui_textbox_get_cursor_width(dred_textbox__get_internal_tb(pTextBox));
}

void dred_textbox_set_cursor_color(dred_textbox* pTextBox, drgui_color color)
{
    drgui_textbox_set_cursor_color(dred_textbox__get_internal_tb(pTextBox), color);
}

void dred_textbox_set_border_color(dred_textbox* pTextBox, drgui_color color)
{
    drgui_textbox_set_border_color(dred_textbox__get_internal_tb(pTextBox), color);
}

void dred_textbox_set_border_width(dred_textbox* pTextBox, float borderWidth)
{
    drgui_textbox_set_border_width(dred_textbox__get_internal_tb(pTextBox), borderWidth);
}

void dred_textbox_set_padding(dred_textbox* pTextBox, float padding)
{
    drgui_textbox_set_padding(dred_textbox__get_internal_tb(pTextBox), padding);
}

float dred_textbox_get_padding_vert(dred_textbox* pTextBox)
{
    return drgui_textbox_get_padding_vert(dred_textbox__get_internal_tb(pTextBox));
}

float dred_textbox_get_padding_horz(dred_textbox* pTextBox)
{
    return drgui_textbox_get_padding_horz(dred_textbox__get_internal_tb(pTextBox));
}

void dred_textbox_set_vertical_align(dred_textbox* pTextBox, drgui_text_engine_alignment align)
{
    drgui_textbox_set_vertical_align(dred_textbox__get_internal_tb(pTextBox), align);
}

void dred_textbox_set_horizontal_align(dred_textbox* pTextBox, drgui_text_engine_alignment align)
{
    drgui_textbox_set_horizontal_align(dred_textbox__get_internal_tb(pTextBox), align);
}

void dred_textbox_set_line_numbers_width(dred_textbox* pTextBox, float lineNumbersWidth)
{
    drgui_textbox_set_line_numbers_width(dred_textbox__get_internal_tb(pTextBox), lineNumbersWidth);
}

float dred_textbox_get_line_numbers_width(dred_textbox* pTextBox)
{
    return drgui_textbox_get_line_numbers_width(dred_textbox__get_internal_tb(pTextBox));
}


void dred_textbox_set_cursor_blink_rate(dred_textbox* pTextBox, unsigned int blinkRateInMilliseconds)
{
    drgui_textbox_set_cursor_blink_rate(dred_textbox__get_internal_tb(pTextBox), blinkRateInMilliseconds);
}

void dred_textbox_move_cursor_to_end_of_text(dred_textbox* pTextBox)
{
    drgui_textbox_move_cursor_to_end_of_text(dred_textbox__get_internal_tb(pTextBox));
}

void dred_textbox_move_cursor_to_start_of_line_by_index(dred_textbox* pTextBox, size_t iLine)
{
    drgui_textbox_move_cursor_to_start_of_line_by_index(dred_textbox__get_internal_tb(pTextBox), iLine);
}

bool dred_textbox_is_anything_selected(dred_textbox* pTextBox)
{
    return drgui_textbox_is_anything_selected(dred_textbox__get_internal_tb(pTextBox));
}

void dred_textbox_select_all(dred_textbox* pTextBox)
{
    drgui_textbox_select_all(dred_textbox__get_internal_tb(pTextBox));
}

void dred_textbox_deselect_all(dred_textbox* pTextBox)
{
    drgui_textbox_deselect_all(dred_textbox__get_internal_tb(pTextBox));
}

size_t dred_textbox_get_selected_text(dred_textbox* pTextBox, char* textOut, size_t textOutLength)
{
    return drgui_textbox_get_selected_text(dred_textbox__get_internal_tb(pTextBox), textOut, textOutLength);
}

bool dred_textbox_delete_character_to_right_of_cursor(dred_textbox* pTextBox)
{
    return drgui_textbox_delete_character_to_right_of_cursor(dred_textbox__get_internal_tb(pTextBox));
}

bool dred_textbox_delete_selected_text(dred_textbox* pTextBox)
{
    return drgui_textbox_delete_selected_text(dred_textbox__get_internal_tb(pTextBox));
}

bool dred_textbox_insert_text_at_cursor(dred_textbox* pTextBox, const char* text)
{
    return drgui_textbox_insert_text_at_cursor(dred_textbox__get_internal_tb(pTextBox), text);
}

bool dred_textbox_undo(dred_textbox* pTextBox)
{
    return drgui_textbox_undo(dred_textbox__get_internal_tb(pTextBox));
}

bool dred_textbox_redo(dred_textbox* pTextBox)
{
    return drgui_textbox_redo(dred_textbox__get_internal_tb(pTextBox));
}

unsigned int dred_textbox_get_undo_points_remaining_count(dred_textbox* pTextBox)
{
    return drgui_textbox_get_undo_points_remaining_count(dred_textbox__get_internal_tb(pTextBox));
}

unsigned int dred_textbox_get_redo_points_remaining_count(dred_textbox* pTextBox)
{
    return drgui_textbox_get_redo_points_remaining_count(dred_textbox__get_internal_tb(pTextBox));
}

void dred_textbox_clear_undo_stack(dred_textbox* pTextBox)
{
    drgui_textbox_clear_undo_stack(dred_textbox__get_internal_tb(pTextBox));
}

size_t dred_textbox_get_cursor_line(dred_textbox* pTextBox)
{
    return drgui_textbox_get_cursor_line(dred_textbox__get_internal_tb(pTextBox));
}

size_t dred_textbox_get_cursor_column(dred_textbox* pTextBox)
{
    return drgui_textbox_get_cursor_column(dred_textbox__get_internal_tb(pTextBox));
}

size_t dred_textbox_get_line_count(dred_textbox* pTextBox)
{
    return drgui_textbox_get_line_count(dred_textbox__get_internal_tb(pTextBox));
}


bool dred_textbox_find_and_select_next(dred_textbox* pTextBox, const char* text)
{
    return drgui_textbox_find_and_select_next(dred_textbox__get_internal_tb(pTextBox), text);
}

bool dred_textbox_find_and_replace_next(dred_textbox* pTextBox, const char* text, const char* replacement)
{
    return drgui_textbox_find_and_replace_next(dred_textbox__get_internal_tb(pTextBox), text, replacement);
}

bool dred_textbox_find_and_replace_all(dred_textbox* pTextBox, const char* text, const char* replacement)
{
    return drgui_textbox_find_and_replace_all(dred_textbox__get_internal_tb(pTextBox), text, replacement);
}


void dred_textbox_show_line_numbers(dred_textbox* pTextBox)
{
    drgui_textbox_show_line_numbers(dred_textbox__get_internal_tb(pTextBox));
}

void dred_textbox_hide_line_numbers(dred_textbox* pTextBox)
{
    drgui_textbox_hide_line_numbers(dred_textbox__get_internal_tb(pTextBox));
}


void dred_textbox_disable_vertical_scrollbar(dred_textbox* pTextBox)
{
    drgui_textbox_disable_vertical_scrollbar(dred_textbox__get_internal_tb(pTextBox));
}

void dred_textbox_enable_vertical_scrollbar(dred_textbox* pTextBox)
{
    drgui_textbox_enable_vertical_scrollbar(dred_textbox__get_internal_tb(pTextBox));
}

void dred_textbox_disable_horizontal_scrollbar(dred_textbox* pTextBox)
{
    drgui_textbox_disable_horizontal_scrollbar(dred_textbox__get_internal_tb(pTextBox));
}

void dred_textbox_enable_horizontal_scrollbar(dred_textbox* pTextBox)
{
    drgui_textbox_enable_horizontal_scrollbar(dred_textbox__get_internal_tb(pTextBox));
}


void dred_textbox_set_on_cursor_move(dred_textbox* pTextBox, dred_textbox_on_cursor_move_proc proc)
{
    dred_textbox_data* data = dred_control_get_extra_data(pTextBox);
    if (data == NULL) {
        return;
    }

    data->onCursorMove = proc;
}

void dred_textbox_set_on_undo_point_changed(dred_textbox* pTextBox, dred_textbox_on_undo_point_changed_proc proc)
{
    dred_textbox_data* data = dred_control_get_extra_data(pTextBox);
    if (data == NULL) {
        return;
    }

    data->onUndoPointChanged = proc;
}


void dred_textbox_on_mouse_wheel(dred_textbox* pTextBox, int delta, int mousePosX, int mousePosY, int stateFlags)
{
    dred_textbox_data* data = dred_control_get_extra_data(pTextBox);
    if (data == NULL) {
        return;
    }

    drgui_textbox_on_mouse_wheel(data->pInternalTB, delta, mousePosX, mousePosY, stateFlags);
}

void dred_textbox_on_key_down(dred_textbox* pTextBox, drgui_key key, int stateFlags)
{
    dred_textbox_data* data = dred_control_get_extra_data(pTextBox);
    if (data == NULL) {
        return;
    }

    drgui_textbox_on_key_down(data->pInternalTB, key, stateFlags);
}

void dred_textbox_on_key_up(dred_textbox* pTextBox, drgui_key key, int stateFlags)
{
    dred_textbox_data* data = dred_control_get_extra_data(pTextBox);
    if (data == NULL) {
        return;
    }

    drgui_textbox_on_key_up(data->pInternalTB, key, stateFlags);
}

void dred_textbox_on_printable_key_down(dred_textbox* pTextBox, uint32_t utf32, int stateFlags)
{
    dred_textbox_data* data = dred_control_get_extra_data(pTextBox);
    if (data == NULL) {
        return;
    }

    drgui_textbox_on_printable_key_down(data->pInternalTB, utf32, stateFlags);
}

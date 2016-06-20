
typedef struct
{
    /// The text engine.
    drte_engine* pTL;

    /// The vertical scrollbar.
    drgui_element* pVertScrollbar;

    /// The horizontal scrollbar.
    drgui_element* pHorzScrollbar;

    /// The line numbers element.
    drgui_element* pLineNumbers;


    /// The color of the border.
    drgui_color borderColor;

    /// The width of the border.
    float borderWidth;

    /// The amount of padding to apply the left and right of the text.
    float padding;

    // The width of the line numbers.
    float lineNumbersWidth;

    /// The padding to the right of the line numbers.
    float lineNumbersPaddingRight;

    // The color of the text of the line numbers.
    drgui_color lineNumbersColor;

    // The color of the background of the line numbers.
    drgui_color lineNumbersBackgroundColor;


    /// The desired width of the vertical scrollbar.
    float vertScrollbarSize;

    /// The desired height of the horizontal scrollbar.
    float horzScrollbarSize;

    /// Whether or not the vertical scrollbar is enabled.
    bool isVertScrollbarEnabled;

    /// Whether or not the horizontal scrollbar is enabled.
    bool isHorzScrollbarEnabled;

    
    // Whether or not tabs to spaces is enabled.
    bool isTabsToSpacesEnabled;


    /// When selecting lines by clicking and dragging on the line numbers, keeps track of the line to anchor the selection to.
    size_t iLineSelectAnchor;


    /// The function to call when the text cursor/caret moves.
    dred_textbox_on_cursor_move_proc onCursorMove;

    /// The function to call when the undo point changes.
    dred_textbox_on_undo_point_changed_proc onUndoPointChanged;


    // The timer for stepping the cursor.
    dred_timer* pTimer;

} dred_textbox_data;


/// Retrieves the offset to draw the text in the text box.
void dred_textbox__get_text_offset(dred_textbox* pTextBox, float* pOffsetXOut, float* pOffsetYOut);

/// Calculates the required size of the text engine.
void dred_textbox__calculate_text_engine_container_size(dred_textbox* pTextBox, float* pWidthOut, float* pHeightOut);

/// Retrieves the rectangle of the text engine's container.
drgui_rect dred_textbox__get_text_rect(dred_textbox* pTextBox);

/// Refreshes the range, page sizes and layouts of the scrollbars.
void dred_textbox__refresh_scrollbars(dred_textbox* pTextBox);

/// Refreshes the range and page sizes of the scrollbars.
void dred_textbox__refresh_scrollbar_ranges(dred_textbox* pTextBox);

/// Refreshes the size and position of the scrollbars.
void dred_textbox__refresh_scrollbar_layouts(dred_textbox* pTextBox);

/// Retrieves a rectangle representing the space between the edges of the two scrollbars.
drgui_rect dred_textbox__get_scrollbar_dead_space_rect(dred_textbox* pTextBox);


/// Called when a mouse button is pressed on the line numbers element.
void dred_textbox__on_mouse_move_line_numbers(drgui_element* pLineNumbers, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when a mouse button is pressed on the line numbers element.
void dred_textbox__on_mouse_button_down_line_numbers(drgui_element* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when a mouse button is pressed on the line numbers element.
void dred_textbox__on_mouse_button_up_line_numbers(drgui_element* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when the line numbers element needs to be drawn.
void dred_textbox__on_paint_line_numbers(drgui_element* pLineNumbers, drgui_rect relativeRect, void* pPaintData);

/// Refreshes the line number of the given text editor.
void dred_textbox__refresh_line_numbers(dred_textbox* pTextBox);


/// on_paint_rect()
void dred_textbox__on_text_engine_paint_rect(drte_engine* pLayout, drgui_rect rect, drgui_color color, dred_textbox* pTextBox, void* pPaintData);

/// on_paint_text()
void dred_textbox__on_text_engine_paint_text(drte_engine* pTL, drte_text_run* pRun, dred_textbox* pTextBox, void* pPaintData);

/// on_dirty()
void dred_textbox__on_text_engine_dirty(drte_engine* pTL, drgui_rect rect);

/// on_cursor_move()
void dred_textbox__on_text_engine_cursor_move(drte_engine* pTL);

/// on_text_changed()
void dred_textbox__on_text_engine_text_changed(drte_engine* pTL);

/// on_undo_point_changed()
void dred_textbox__on_text_engine_undo_point_changed(drte_engine* pTL, unsigned int iUndoPoint);


void dred_textbox__on_vscroll(drgui_element* pSBElement, int scrollPos)
{
    dred_textbox* pTextBox = *(drgui_element**)drgui_sb_get_extra_data(pSBElement);
    assert(pTextBox != NULL);

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    drte_engine_set_inner_offset_y(pTB->pTL, -drte_engine_get_line_pos_y(pTB->pTL, scrollPos));

    // The line numbers need to be redrawn.
    drgui_dirty(pTB->pLineNumbers, drgui_get_local_rect(pTB->pLineNumbers));
}

void dred_textbox__on_hscroll(drgui_element* pSBElement, int scrollPos)
{
    dred_textbox* pTextBox = *(drgui_element**)drgui_sb_get_extra_data(pSBElement);
    assert(pTextBox != NULL);

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    drte_engine_set_inner_offset_x(pTB->pTL, (float)-scrollPos);
}



dred_textbox* dred_textbox_create(dred_context* pDred, dred_control* pParent)
{
    dred_textbox* pTextBox = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_TEXTBOX, sizeof(dred_textbox_data));
    if (pTextBox == NULL) {
        return NULL;
    }

    drgui_set_cursor(pTextBox, drgui_cursor_text);
    drgui_set_on_size(pTextBox, dred_textbox_on_size);
    drgui_set_on_mouse_move(pTextBox, dred_textbox_on_mouse_move);
    drgui_set_on_mouse_button_down(pTextBox, dred_textbox_on_mouse_button_down);
    drgui_set_on_mouse_button_up(pTextBox, dred_textbox_on_mouse_button_up);
    drgui_set_on_mouse_button_dblclick(pTextBox, dred_textbox_on_mouse_button_dblclick);
    drgui_set_on_mouse_wheel(pTextBox, dred_textbox_on_mouse_wheel);
    drgui_set_on_key_down(pTextBox, dred_textbox_on_key_down);
    drgui_set_on_printable_key_down(pTextBox, dred_textbox_on_printable_key_down);
    drgui_set_on_paint(pTextBox, dred_textbox_on_paint);
    drgui_set_on_capture_keyboard(pTextBox, dred_textbox_on_capture_keyboard);
    drgui_set_on_release_keyboard(pTextBox, dred_textbox_on_release_keyboard);
    drgui_set_on_capture_mouse(pTextBox, dred_textbox_on_capture_mouse);
    drgui_set_on_release_mouse(pTextBox, dred_textbox_on_release_mouse);

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    pTB->pVertScrollbar = drgui_create_scrollbar(pDred->pGUI, pTextBox, drgui_sb_orientation_vertical, sizeof(pTextBox), &pTextBox);
    drgui_sb_set_on_scroll(pTB->pVertScrollbar, dred_textbox__on_vscroll);
    drgui_sb_set_mouse_wheel_scele(pTB->pVertScrollbar, 3);

    pTB->pHorzScrollbar = drgui_create_scrollbar(pDred->pGUI, pTextBox, drgui_sb_orientation_horizontal, sizeof(pTextBox), &pTextBox);
    drgui_sb_set_on_scroll(pTB->pHorzScrollbar, dred_textbox__on_hscroll);

    pTB->pLineNumbers = drgui_create_element(pDred->pGUI, pTextBox, sizeof(pTextBox), &pTextBox);
    drgui_hide(pTB->pLineNumbers);
    drgui_set_on_mouse_move(pTB->pLineNumbers, dred_textbox__on_mouse_move_line_numbers);
    drgui_set_on_mouse_button_down(pTB->pLineNumbers, dred_textbox__on_mouse_button_down_line_numbers);
    drgui_set_on_mouse_button_up(pTB->pLineNumbers, dred_textbox__on_mouse_button_up_line_numbers);
    drgui_set_on_paint(pTB->pLineNumbers, dred_textbox__on_paint_line_numbers);

    pTB->pTL = drte_engine_create(pDred->pGUI, pTextBox);
    if (pTB->pTL == NULL) {
        drgui_delete_element(pTextBox);
        return NULL;
    }

    drte_engine_set_on_paint_rect(pTB->pTL, dred_textbox__on_text_engine_paint_rect);
    drte_engine_set_on_paint_text(pTB->pTL, dred_textbox__on_text_engine_paint_text);
    drte_engine_set_on_dirty(pTB->pTL, dred_textbox__on_text_engine_dirty);
    drte_engine_set_on_cursor_move(pTB->pTL, dred_textbox__on_text_engine_cursor_move);
    drte_engine_set_on_text_changed(pTB->pTL, dred_textbox__on_text_engine_text_changed);
    drte_engine_set_on_undo_point_changed(pTB->pTL, dred_textbox__on_text_engine_undo_point_changed);
    drte_engine_set_default_text_color(pTB->pTL, drgui_rgb(0, 0, 0));
    drte_engine_set_cursor_color(pTB->pTL, drgui_rgb(0, 0, 0));
    drte_engine_set_default_bg_color(pTB->pTL, drgui_rgb(64, 64, 64));
    drte_engine_set_active_line_bg_color(pTB->pTL, drgui_rgb(64, 64, 64));
    drte_engine_set_vertical_align(pTB->pTL, drte_alignment_center);
    drte_engine_set_default_font(pTB->pTL, dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale));

    pTB->borderColor = drgui_rgb(0, 0, 0);
    pTB->borderWidth = 0;
    pTB->padding     = 2;
    pTB->lineNumbersWidth = 64;
    pTB->lineNumbersPaddingRight = 16;
    pTB->lineNumbersColor = drgui_rgb(80, 160, 192);
    pTB->lineNumbersBackgroundColor = drte_engine_get_default_bg_color(pTB->pTL);
    pTB->vertScrollbarSize = 16;
    pTB->horzScrollbarSize = 16;
    pTB->isVertScrollbarEnabled = true;
    pTB->isHorzScrollbarEnabled = true;
    pTB->iLineSelectAnchor = 0;
    pTB->onCursorMove = NULL;
    pTB->onUndoPointChanged = NULL;

    return pTextBox;
}

void dred_textbox_delete(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    // Keyboard focus needs to be released first. If we don't do this we'll not free delete the internal timer.
    if (drgui_has_keyboard_capture(pTextBox)) {
        drgui_release_keyboard(pTextBox->pContext);
    }

    if (pTB->pTL) {
        drte_engine_delete(pTB->pTL);
        pTB->pTL = NULL;
    }

    if (pTB->pLineNumbers) {
        drgui_delete_element(pTB->pLineNumbers);
        pTB->pLineNumbers = NULL;
    }

    if (pTB->pHorzScrollbar) {
        drgui_delete_element(pTB->pHorzScrollbar);
        pTB->pHorzScrollbar = NULL;
    }

    if (pTB->pVertScrollbar) {
        drgui_delete_element(pTB->pVertScrollbar);
        pTB->pVertScrollbar = NULL;
    }

    drgui_delete_element(pTextBox);
}


void dred_textbox_set_font(dred_textbox* pTextBox, drgui_font* pFont)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drgui_begin_dirty(pTextBox);
    {
        drte_engine_set_default_font(pTB->pTL, pFont);

        // The font used for line numbers are tied to the main font at the moment.
        dred_textbox__refresh_line_numbers(pTextBox);

        // Emulate a scroll to ensure the scroll position is pinned to a line.
        dred_textbox__on_vscroll(pTB->pVertScrollbar, drgui_sb_get_scroll_position(pTB->pVertScrollbar));
        dred_textbox__refresh_scrollbars(pTextBox);

        // The caret position needs to be refreshes. We'll cheat here a little bit and just do a full refresh of the text engine.
        //drte_engine__refresh(pTB->pTL);
        drte_engine_refresh_markers(pTB->pTL);
    }
    drgui_end_dirty(pTextBox);
}

drgui_font* dred_textbox_get_font(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return NULL;
    }

    return drte_engine_get_default_font(pTB->pTL);
}

void dred_textbox_set_text_color(dred_textbox* pTextBox, drgui_color color)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_set_default_text_color(pTB->pTL, color);
}

void dred_textbox_set_background_color(dred_textbox* pTextBox, drgui_color color)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_set_default_bg_color(pTB->pTL, color);
}

void dred_textbox_set_selection_background_color(dred_textbox* pTextBox, drgui_color color)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_set_selection_bg_color(pTB->pTL, color);
}

drgui_color dred_textbox_get_selection_background_color(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return drte_engine_get_selection_bg_color(pTB->pTL);
}

void dred_textbox_set_active_line_background_color(dred_textbox* pTextBox, drgui_color color)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_set_active_line_bg_color(pTB->pTL, color);
}

void dred_textbox_set_cursor_width(dred_textbox* pTextBox, float cursorWidth)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_set_cursor_width(pTB->pTL, cursorWidth);
}

float dred_textbox_get_cursor_width(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    return drte_engine_get_cursor_width(pTB->pTL);
}

void dred_textbox_set_cursor_color(dred_textbox* pTextBox, drgui_color color)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_set_cursor_color(pTB->pTL, color);
}

void dred_textbox_set_border_color(dred_textbox* pTextBox, drgui_color color)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    pTB->borderColor = color;
}

void dred_textbox_set_border_width(dred_textbox* pTextBox, float borderWidth)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    pTB->borderWidth = borderWidth;
}

void dred_textbox_set_padding(dred_textbox* pTextBox, float padding)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    pTB->padding = padding;
}

float dred_textbox_get_padding_vert(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    return pTB->padding;
}

float dred_textbox_get_padding_horz(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    return pTB->padding;
}

void dred_textbox_set_vertical_align(dred_textbox* pTextBox, drte_alignment align)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_set_vertical_align(pTB->pTL, align);
}

void dred_textbox_set_horizontal_align(dred_textbox* pTextBox, drte_alignment align)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_set_horizontal_align(pTB->pTL, align);
}

void dred_textbox_set_line_numbers_width(dred_textbox* pTextBox, float lineNumbersWidth)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    pTB->lineNumbersWidth = lineNumbersWidth;
}

float dred_textbox_get_line_numbers_width(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    return pTB->lineNumbersWidth;
}

void dred_textbox_set_line_numbers_padding(dred_textbox* pTextBox, float lineNumbersPadding)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    pTB->lineNumbersPaddingRight = lineNumbersPadding;
}

float dred_textbox_get_line_numbers_padding(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    return pTB->lineNumbersPaddingRight;
}

void dred_textbox_set_line_numbers_color(dred_textbox* pTextBox, drgui_color color)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    pTB->lineNumbersColor = color;
    dred_textbox__refresh_line_numbers(pTextBox);
}

drgui_color dred_textbox_get_line_numbers_color(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pTB->lineNumbersColor;
}

void dred_textbox_set_line_numbers_background_color(dred_textbox* pTextBox, drgui_color color)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }
    
    pTB->lineNumbersBackgroundColor = color;
    dred_textbox__refresh_line_numbers(pTextBox);
}

drgui_color dred_textbox_get_line_numbers_background_color(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pTB->lineNumbersBackgroundColor;
}



void dred_textbox_set_text(dred_textbox* pTextBox, const char* text)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_prepare_undo_point(pTB->pTL);
    {
        drte_engine_set_text(pTB->pTL, text);
    }
    drte_engine_commit_undo_point(pTB->pTL);
}

size_t dred_textbox_get_text(dred_textbox* pTextBox, char* pTextOut, size_t textOutSize)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    return drte_engine_get_text(pTB->pTL, pTextOut, textOutSize);
}

void dred_textbox_step(dred_textbox* pTextBox, unsigned int milliseconds)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_step(pTB->pTL, milliseconds);
}

void dred_textbox_set_cursor_blink_rate(dred_textbox* pTextBox, unsigned int blinkRateInMilliseconds)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_set_cursor_blink_rate(pTB->pTL, blinkRateInMilliseconds);
}

void dred_textbox_move_cursor_to_end_of_text(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_move_cursor_to_end_of_text(pTB->pTL);
}

void dred_textbox_move_cursor_to_start_of_line_by_index(dred_textbox* pTextBox, size_t iLine)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_move_cursor_to_start_of_line_by_index(pTB->pTL, iLine);
}


bool dred_textbox_is_anything_selected(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return false;
    }

    return drte_engine_is_anything_selected(pTB->pTL);
}

void dred_textbox_select_all(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_select_all(pTB->pTL);
}

void dred_textbox_deselect_all(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_deselect_all(pTB->pTL);
}

size_t dred_textbox_get_selected_text(dred_textbox* pTextBox, char* textOut, size_t textOutLength)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    return drte_engine_get_selected_text(pTB->pTL, textOut, textOutLength);
}

bool dred_textbox_delete_character_to_right_of_cursor(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return false;
    }

    bool wasTextChanged = false;
    drte_engine_prepare_undo_point(pTB->pTL);
    {
        wasTextChanged = drte_engine_delete_character_to_right_of_cursor(pTB->pTL);
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTB->pTL); }

    return wasTextChanged;
}

bool dred_textbox_delete_selected_text(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return false;
    }

    bool wasTextChanged = false;
    drte_engine_prepare_undo_point(pTB->pTL);
    {
        wasTextChanged = drte_engine_delete_selected_text(pTB->pTL);
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTB->pTL); }

    return wasTextChanged;
}

bool dred_textbox_insert_text_at_cursor(dred_textbox* pTextBox, const char* text)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return false;;
    }

    bool wasTextChanged = false;
    drte_engine_prepare_undo_point(pTB->pTL);
    {
        wasTextChanged = drte_engine_insert_text_at_cursor(pTB->pTL, text);
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTB->pTL); }

    return wasTextChanged;
}

bool dred_textbox_undo(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return false;
    }

    return drte_engine_undo(pTB->pTL);
}

bool dred_textbox_redo(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return false;
    }

    return drte_engine_redo(pTB->pTL);
}

unsigned int dred_textbox_get_undo_points_remaining_count(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return false;
    }

    return drte_engine_get_undo_points_remaining_count(pTB->pTL);
}

unsigned int dred_textbox_get_redo_points_remaining_count(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return false;
    }

    return drte_engine_get_redo_points_remaining_count(pTB->pTL);
}

void dred_textbox_clear_undo_stack(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_clear_undo_stack(pTB->pTL);
}


size_t dred_textbox_get_cursor_line(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    return drte_engine_get_cursor_line(pTB->pTL);
}

size_t dred_textbox_get_cursor_column(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    return drte_engine_get_cursor_column(pTB->pTL);
}

size_t dred_textbox_get_line_count(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    return drte_engine_get_line_count(pTB->pTL);
}


bool dred_textbox_find_and_select_next(dred_textbox* pTextBox, const char* text)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    size_t selectionStart;
    size_t selectionEnd;
    if (drte_engine_find_next(pTB->pTL, text, &selectionStart, &selectionEnd))
    {
        drte_engine_select(pTB->pTL, selectionStart, selectionEnd);
        drte_engine_move_cursor_to_end_of_selection(pTB->pTL);

        return true;
    }

    return false;
}

bool dred_textbox_find_and_replace_next(dred_textbox* pTextBox, const char* text, const char* replacement)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    bool wasTextChanged = false;
    drte_engine_prepare_undo_point(pTB->pTL);
    {
        size_t selectionStart;
        size_t selectionEnd;
        if (drte_engine_find_next(pTB->pTL, text, &selectionStart, &selectionEnd))
        {
            drte_engine_select(pTB->pTL, selectionStart, selectionEnd);
            drte_engine_move_cursor_to_end_of_selection(pTB->pTL);

            wasTextChanged = drte_engine_delete_selected_text(pTB->pTL) || wasTextChanged;
            wasTextChanged = drte_engine_insert_text_at_cursor(pTB->pTL, replacement) || wasTextChanged;
        }
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTB->pTL); }

    return wasTextChanged;
}

bool dred_textbox_find_and_replace_all(dred_textbox* pTextBox, const char* text, const char* replacement)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    size_t originalCursorLine = drte_engine_get_cursor_line(pTB->pTL);
    size_t originalCursorPos = drte_engine_get_cursor_character(pTB->pTL) - drte_engine_get_line_first_character(pTB->pTL, originalCursorLine);
    int originalScrollPosX = drgui_sb_get_scroll_position(pTB->pHorzScrollbar);
    int originalScrollPosY = drgui_sb_get_scroll_position(pTB->pVertScrollbar);

    bool wasTextChanged = false;
    drte_engine_prepare_undo_point(pTB->pTL);
    {
        // It's important that we don't replace the replacement text. To handle this, we just move the cursor to the top of the text and find
        // and replace every occurance without looping.
        drte_engine_move_cursor_to_start_of_text(pTB->pTL);

        size_t selectionStart;
        size_t selectionEnd;
        while (drte_engine_find_next_no_loop(pTB->pTL, text, &selectionStart, &selectionEnd))
        {
            drte_engine_select(pTB->pTL, selectionStart, selectionEnd);
            drte_engine_move_cursor_to_end_of_selection(pTB->pTL);

            wasTextChanged = drte_engine_delete_selected_text(pTB->pTL) || wasTextChanged;
            wasTextChanged = drte_engine_insert_text_at_cursor(pTB->pTL, replacement) || wasTextChanged;
        }

        // The cursor may have moved so we'll need to restore it.
        size_t lineCharStart;
        size_t lineCharEnd;
        drte_engine_get_line_character_range(pTB->pTL, originalCursorLine, &lineCharStart, &lineCharEnd);

        size_t newCursorPos = lineCharStart + originalCursorPos;
        if (newCursorPos > lineCharEnd) {
            newCursorPos = lineCharEnd;
        }
        drte_engine_move_cursor_to_character(pTB->pTL, newCursorPos);
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTB->pTL); }


    // The scroll positions may have moved so we'll need to restore them.
    drgui_sb_scroll_to(pTB->pHorzScrollbar, originalScrollPosX);
    drgui_sb_scroll_to(pTB->pVertScrollbar, originalScrollPosY);

    return wasTextChanged;
}


void dred_textbox_show_line_numbers(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drgui_show(pTB->pLineNumbers);
    dred_textbox__refresh_line_numbers(pTextBox);
}

void dred_textbox_hide_line_numbers(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drgui_hide(pTB->pLineNumbers);
    dred_textbox__refresh_line_numbers(pTextBox);
}


void dred_textbox_disable_vertical_scrollbar(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    if (pTB->isVertScrollbarEnabled) {
        pTB->isVertScrollbarEnabled = false;
        dred_textbox__refresh_scrollbars(pTextBox);
    }
}

void dred_textbox_enable_vertical_scrollbar(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    if (!pTB->isVertScrollbarEnabled) {
        pTB->isVertScrollbarEnabled = true;
        dred_textbox__refresh_scrollbars(pTextBox);
    }
}

void dred_textbox_disable_horizontal_scrollbar(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    if (pTB->isHorzScrollbarEnabled) {
        pTB->isHorzScrollbarEnabled = false;
        dred_textbox__refresh_scrollbars(pTextBox);
    }
}

void dred_textbox_enable_horizontal_scrollbar(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    if (!pTB->isHorzScrollbarEnabled) {
        pTB->isHorzScrollbarEnabled = true;
        dred_textbox__refresh_scrollbars(pTextBox);
    }
}

drgui_element* dred_textbox_get_vertical_scrollbar(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pVertScrollbar;
}

drgui_element* dred_textbox_get_horizontal_scrollbar(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return NULL;
    }

    return pTB->pHorzScrollbar;
}

void dred_textbox_set_scrollbar_size(dred_textbox* pTextBox, float size)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    pTB->horzScrollbarSize = size;
    pTB->vertScrollbarSize = size;

    dred_textbox__refresh_scrollbars(pTextBox);
}


void dred_textbox_set_tab_size_in_spaces(dred_textbox* pTextBox, unsigned int tabSizeInSpaces)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_set_tab_size(pTB->pTL, tabSizeInSpaces);
}

unsigned int dred_textbox_get_tab_size_in_spaces(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return 0;
    }

    return drte_engine_get_tab_size(pTB->pTL);
}


void dred_textbox_enable_tabs_to_spaces(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    pTB->isTabsToSpacesEnabled = true;
}

void dred_textbox_disable_tabs_to_spaces(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    pTB->isTabsToSpacesEnabled = false;
}

bool dred_textbox_is_tabs_to_spaces_enabled(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return false;;
    }

    return pTB->isTabsToSpacesEnabled;
}


void dred_textbox_set_on_cursor_move(dred_textbox* pTextBox, dred_textbox_on_cursor_move_proc proc)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    pTB->onCursorMove = proc;
}

void dred_textbox_set_on_undo_point_changed(dred_textbox* pTextBox, dred_textbox_on_undo_point_changed_proc proc)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    pTB->onUndoPointChanged = proc;
}


void dred_textbox_on_size(dred_textbox* pTextBox, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    // The text engine needs to be resized.
    float containerWidth;
    float containerHeight;
    dred_textbox__calculate_text_engine_container_size(pTextBox, &containerWidth, &containerHeight);
    drte_engine_set_container_size(pTB->pTL, containerWidth, containerHeight);

    // Scrollbars need to be refreshed first.
    dred_textbox__refresh_scrollbars(pTextBox);

    // Line numbers need to be refreshed.
    dred_textbox__refresh_line_numbers(pTextBox);
}

void dred_textbox_on_mouse_move(dred_textbox* pTextBox, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    if (drgui_get_element_with_mouse_capture(pTextBox->pContext) == pTextBox)
    {
        float offsetX;
        float offsetY;
        dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);

        drte_engine_move_cursor_to_point(pTB->pTL, (float)relativeMousePosX - offsetX, (float)relativeMousePosY - offsetY);
    }
}

void dred_textbox_on_mouse_button_down(dred_textbox* pTextBox, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    // Focus the text editor.
    drgui_capture_keyboard(pTextBox);

    if (mouseButton == DRGUI_MOUSE_BUTTON_LEFT)
    {
        // If we are not in selection mode, make sure everything is deselected.
        if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) == 0) {
            drte_engine_deselect_all(pTB->pTL);
            drte_engine_leave_selection_mode(pTB->pTL);
        } else {
            drte_engine_enter_selection_mode(pTB->pTL);
        }

        float offsetX;
        float offsetY;
        dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);
        drte_engine_move_cursor_to_point(pTB->pTL, (float)relativeMousePosX - offsetX, (float)relativeMousePosY - offsetY);

        // In order to support selection with the mouse we need to capture the mouse and enter selection mode.
        drgui_capture_mouse(pTextBox);

        // If we didn't previously enter selection mode we'll need to do that now so we can drag select.
        if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) == 0) {
            drte_engine_enter_selection_mode(pTB->pTL);
        }
    }

    if (mouseButton == DRGUI_MOUSE_BUTTON_RIGHT)
    {
        drte_engine_leave_selection_mode(pTB->pTL);
    }
}

void dred_textbox_on_mouse_button_up(dred_textbox* pTextBox, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    if (mouseButton == DRGUI_MOUSE_BUTTON_LEFT)
    {
        if (drgui_get_element_with_mouse_capture(pTextBox->pContext) == pTextBox)
        {
            // Releasing the mouse will leave selectionmode.
            drgui_release_mouse(pTextBox->pContext);
        }
    }
}

void dred_textbox_on_mouse_button_dblclick(dred_textbox* pTextBox, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)mouseButton;
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    if (mouseButton == DRGUI_MOUSE_BUTTON_LEFT) {
        if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) == 0) {
            drte_engine_leave_selection_mode(pTB->pTL);
            drte_engine_deselect_all(pTB->pTL);

            drte_engine_select_word_under_cursor(pTB->pTL);
        }
    }
}

void dred_textbox_on_mouse_wheel(dred_textbox* pTextBox, int delta, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drgui_sb_scroll(pTB->pVertScrollbar, -delta * drgui_sb_get_mouse_wheel_scale(pTB->pVertScrollbar));
}

void dred_textbox_on_key_down(dred_textbox* pTextBox, drgui_key key, int stateFlags)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    switch (key)
    {
        case DRGUI_BACKSPACE:
        {
            bool wasTextChanged = false;
            drte_engine_prepare_undo_point(pTB->pTL);
            {
                if (drte_engine_is_anything_selected(pTB->pTL)) {
                    wasTextChanged = drte_engine_delete_selected_text(pTB->pTL);
                } else {
                    wasTextChanged = drte_engine_delete_character_to_left_of_cursor(pTB->pTL);
                }
            }
            if (wasTextChanged) { drte_engine_commit_undo_point(pTB->pTL); }
        } break;

        case DRGUI_DELETE:
        {
            bool wasTextChanged = false;
            drte_engine_prepare_undo_point(pTB->pTL);
            {
                if (drte_engine_is_anything_selected(pTB->pTL)) {
                    wasTextChanged = drte_engine_delete_selected_text(pTB->pTL);
                } else {
                    wasTextChanged = drte_engine_delete_character_to_right_of_cursor(pTB->pTL);
                }
            }
            if (wasTextChanged) { drte_engine_commit_undo_point(pTB->pTL); }
        } break;


        case DRGUI_ARROW_LEFT:
        {
            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_enter_selection_mode(pTB->pTL);
            }

            if (drte_engine_is_anything_selected(pTB->pTL) && !drte_engine_is_in_selection_mode(pTB->pTL)) {
                drte_engine_move_cursor_to_start_of_selection(pTB->pTL);
                drte_engine_deselect_all(pTB->pTL);
            } else {
                if ((stateFlags & DRGUI_KEY_STATE_CTRL_DOWN) != 0) {
                    drte_engine_move_cursor_to_start_of_word(pTB->pTL);
                } else {
                    drte_engine_move_cursor_left(pTB->pTL);
                }
            }

            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_leave_selection_mode(pTB->pTL);
            }
        } break;

        case DRGUI_ARROW_RIGHT:
        {
            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_enter_selection_mode(pTB->pTL);
            }

            if (drte_engine_is_anything_selected(pTB->pTL) && !drte_engine_is_in_selection_mode(pTB->pTL)) {
                drte_engine_move_cursor_to_end_of_selection(pTB->pTL);
                drte_engine_deselect_all(pTB->pTL);
            } else {
                if ((stateFlags & DRGUI_KEY_STATE_CTRL_DOWN) != 0) {
                    drte_engine_move_cursor_to_start_of_next_word(pTB->pTL);
                } else {
                    drte_engine_move_cursor_right(pTB->pTL);
                }
            }

            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_leave_selection_mode(pTB->pTL);
            }
        } break;

        case DRGUI_ARROW_UP:
        {
            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_enter_selection_mode(pTB->pTL);
            }

            if (drte_engine_is_anything_selected(pTB->pTL) && !drte_engine_is_in_selection_mode(pTB->pTL)) {
                drte_engine_deselect_all(pTB->pTL);
            }

            drte_engine_move_cursor_up(pTB->pTL);

            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_leave_selection_mode(pTB->pTL);
            }
        } break;

        case DRGUI_ARROW_DOWN:
        {
            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_enter_selection_mode(pTB->pTL);
            }

            if (drte_engine_is_anything_selected(pTB->pTL) && !drte_engine_is_in_selection_mode(pTB->pTL)) {
                drte_engine_deselect_all(pTB->pTL);
            }

            drte_engine_move_cursor_down(pTB->pTL);

            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_leave_selection_mode(pTB->pTL);
            }
        } break;


        case DRGUI_END:
        {
            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_enter_selection_mode(pTB->pTL);
            }

            if (drte_engine_is_anything_selected(pTB->pTL) && !drte_engine_is_in_selection_mode(pTB->pTL)) {
                drte_engine_deselect_all(pTB->pTL);
            }

            if ((stateFlags & DRGUI_KEY_STATE_CTRL_DOWN) != 0) {
                drte_engine_move_cursor_to_end_of_text(pTB->pTL);
            } else {
                drte_engine_move_cursor_to_end_of_line(pTB->pTL);
            }

            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_leave_selection_mode(pTB->pTL);
            }
        } break;

        case DRGUI_HOME:
        {
            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_enter_selection_mode(pTB->pTL);
            }

            if (drte_engine_is_anything_selected(pTB->pTL) && !drte_engine_is_in_selection_mode(pTB->pTL)) {
                drte_engine_deselect_all(pTB->pTL);
            }

            if ((stateFlags & DRGUI_KEY_STATE_CTRL_DOWN) != 0) {
                drte_engine_move_cursor_to_start_of_text(pTB->pTL);
            } else {
                drte_engine_move_cursor_to_start_of_line(pTB->pTL);
            }

            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_leave_selection_mode(pTB->pTL);
            }
        } break;

        case DRGUI_PAGE_UP:
        {
            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_enter_selection_mode(pTB->pTL);
            }

            if (drte_engine_is_anything_selected(pTB->pTL) && !drte_engine_is_in_selection_mode(pTB->pTL)) {
                drte_engine_deselect_all(pTB->pTL);
            }

            int scrollOffset = drgui_sb_get_page_size(pTB->pVertScrollbar);
            if ((stateFlags & DRGUI_KEY_STATE_CTRL_DOWN) == 0) {
                drgui_sb_scroll(pTB->pVertScrollbar, -scrollOffset);
            }

            drte_engine_move_cursor_y(pTB->pTL, -drgui_sb_get_page_size(pTB->pVertScrollbar));

            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_leave_selection_mode(pTB->pTL);
            }
        } break;

        case DRGUI_PAGE_DOWN:
        {
            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_enter_selection_mode(pTB->pTL);
            }

            if (drte_engine_is_anything_selected(pTB->pTL) && !drte_engine_is_in_selection_mode(pTB->pTL)) {
                drte_engine_deselect_all(pTB->pTL);
            }

            int scrollOffset = drgui_sb_get_page_size(pTB->pVertScrollbar);
            if (scrollOffset > (int)(drte_engine_get_line_count(pTB->pTL) - drte_engine_get_cursor_line(pTB->pTL))) {
                scrollOffset = 0;
            }

            if ((stateFlags & DRGUI_KEY_STATE_CTRL_DOWN) == 0) {
                drgui_sb_scroll(pTB->pVertScrollbar, scrollOffset);
            }

            drte_engine_move_cursor_y(pTB->pTL, drgui_sb_get_page_size(pTB->pVertScrollbar));

            if ((stateFlags & DRGUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_engine_leave_selection_mode(pTB->pTL);
            }
        } break;

        default: break;
    }
}

void dred_textbox_on_key_up(dred_textbox* pTextBox, drgui_key key, int stateFlags)
{
    (void)pTextBox;
    (void)key;
    (void)stateFlags;
}

void dred_textbox_on_printable_key_down(dred_textbox* pTextBox, unsigned int utf32, int stateFlags)
{
    (void)stateFlags;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_prepare_undo_point(pTB->pTL);
    {
        if (drte_engine_is_anything_selected(pTB->pTL)) {
            drte_engine_delete_selected_text(pTB->pTL);
        }

        if (utf32 == '\t' && pTB->isTabsToSpacesEnabled) {
            // This can be optimized...
            size_t spaceCount = drte_engine_get_spaces_to_next_colum_from_cursor(pTB->pTL);
            for (size_t i = 0; i < spaceCount; ++i) {
                drte_engine_insert_character_at_cursor(pTB->pTL, ' ');
            }
        } else {
            drte_engine_insert_character_at_cursor(pTB->pTL, utf32);
        }
        
    }
    drte_engine_commit_undo_point(pTB->pTL);
}


void dred_textbox__on_text_engine_paint_rect(drte_engine* pTL, drgui_rect rect, drgui_color color, dred_textbox* pTextBox, void* pPaintData)
{
    (void)pTL;

    float offsetX;
    float offsetY;
    dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);

    drgui_draw_rect(pTextBox, drgui_offset_rect(rect, offsetX, offsetY), color, pPaintData);
}

void dred_textbox__on_text_engine_paint_text(drte_engine* pTL, drte_text_run* pRun, dred_textbox* pTextBox, void* pPaintData)
{
    (void)pTL;

    float offsetX;
    float offsetY;
    dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);

    drgui_draw_text(pTextBox, pRun->pFont, pRun->text, (int)pRun->textLength, (float)pRun->posX + offsetX, (float)pRun->posY + offsetY, pRun->textColor, pRun->backgroundColor, pPaintData);
}

void dred_textbox__on_text_engine_dirty(drte_engine* pTL, drgui_rect rect)
{
    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;
    if (pTextBox == NULL) {
        return;
    }

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    float offsetX;
    float offsetY;
    dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);

    drgui_dirty(pTextBox, drgui_offset_rect(rect, offsetX, offsetY));
}

void dred_textbox__on_text_engine_cursor_move(drte_engine* pTL)
{
    // If the cursor is off the edge of the container we want to scroll it into position.
    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;
    if (pTextBox == NULL) {
        return;
    }

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    // If the cursor is above or below the container, we need to scroll vertically.
    int iLine = (int)drte_engine_get_cursor_line(pTB->pTL);
    if (iLine < drgui_sb_get_scroll_position(pTB->pVertScrollbar)) {
        drgui_sb_scroll_to(pTB->pVertScrollbar, iLine);
    }

    int iBottomLine = drgui_sb_get_scroll_position(pTB->pVertScrollbar) + drgui_sb_get_page_size(pTB->pVertScrollbar) - 1;
    if (iLine >= iBottomLine) {
        drgui_sb_scroll_to(pTB->pVertScrollbar, iLine - (drgui_sb_get_page_size(pTB->pVertScrollbar) - 1) + 1);
    }


    // If the cursor is to the left or right of the container we need to scroll horizontally.
    float cursorPosX;
    float cursorPosY;
    drte_engine_get_cursor_position(pTB->pTL, &cursorPosX, &cursorPosY);

    if (cursorPosX < 0) {
        drgui_sb_scroll_to(pTB->pHorzScrollbar, (int)(cursorPosX - drte_engine_get_inner_offset_x(pTB->pTL)) - 100);
    }
    if (cursorPosX >= drte_engine_get_container_width(pTB->pTL)) {
        drgui_sb_scroll_to(pTB->pHorzScrollbar, (int)(cursorPosX - drte_engine_get_inner_offset_x(pTB->pTL) - drte_engine_get_container_width(pTB->pTL)) + 100);
    }


    if (pTB->onCursorMove) {
        pTB->onCursorMove(pTextBox);
    }
}

void dred_textbox__on_text_engine_text_changed(drte_engine* pTL)
{
    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;
    if (pTextBox == NULL) {
        return;
    }

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    // Scrollbars need to be refreshed whenever text is changed.
    dred_textbox__refresh_scrollbars(pTextBox);

    // The line numbers need to be redrawn.
    // TODO: This can probably be optimized a bit so that it is only redrawn if a line was inserted or deleted.
    drgui_dirty(pTB->pLineNumbers, drgui_get_local_rect(pTB->pLineNumbers));
}

void dred_textbox__on_text_engine_undo_point_changed(drte_engine* pTL, unsigned int iUndoPoint)
{
    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;
    if (pTextBox == NULL) {
        return;
    }

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    if (pTB->onUndoPointChanged) {
        pTB->onUndoPointChanged(pTextBox, iUndoPoint);
    }
}


void dred_textbox_on_paint(dred_textbox* pTextBox, drgui_rect relativeRect, void* pPaintData)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drgui_rect textRect = dred_textbox__get_text_rect(pTextBox);

    // The dead space between the scrollbars should always be drawn with the default background color.
    drgui_draw_rect(pTextBox, dred_textbox__get_scrollbar_dead_space_rect(pTextBox), drte_engine_get_default_bg_color(pTB->pTL), pPaintData);

    // Border.
    drgui_rect borderRect = drgui_get_local_rect(pTextBox);
    drgui_draw_rect_outline(pTextBox, borderRect, pTB->borderColor, pTB->borderWidth, pPaintData);

    // Padding.
    drgui_rect paddingRect = drgui_grow_rect(textRect, pTB->padding);
    drgui_draw_rect_outline(pTextBox, paddingRect, drte_engine_get_default_bg_color(pTB->pTL), pTB->padding, pPaintData);

    // Text.
    drgui_set_clip(pTextBox, drgui_clamp_rect(textRect, relativeRect), pPaintData);
    drte_engine_paint(pTB->pTL, drgui_offset_rect(drgui_clamp_rect(textRect, relativeRect), -textRect.left, -textRect.top), pTextBox, pPaintData);
}

void dred_textbox__on_timer(dred_timer* pTimer, void* pUserData)
{
    (void)pTimer;

    dred_textbox* pTextBox = (dred_textbox*)pUserData;
    assert(pTextBox != NULL);

    dred_textbox_step(pTextBox, 100);
}

void dred_textbox_on_capture_keyboard(dred_textbox* pTextBox, drgui_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_show_cursor(pTB->pTL);

    if (pTB->pTimer == NULL) {
        pTB->pTimer = dred_timer_create(100, dred_textbox__on_timer, pTextBox);
    }
}

void dred_textbox_on_release_keyboard(dred_textbox* pTextBox, drgui_element* pNewCapturedElement)
{
    (void)pNewCapturedElement;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_hide_cursor(pTB->pTL);

    if (pTB->pTimer != NULL) {
        dred_timer_delete(pTB->pTimer);
        pTB->pTimer = NULL;
    }
}

void dred_textbox_on_capture_mouse(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

}

void dred_textbox_on_release_mouse(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return;
    }

    drte_engine_leave_selection_mode(pTB->pTL);
}



void dred_textbox__get_text_offset(dred_textbox* pTextBox, float* pOffsetXOut, float* pOffsetYOut)
{
    float offsetX = 0;
    float offsetY = 0;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB != NULL)
    {
        float lineNumbersWidth = 0;
        if (drgui_is_visible(pTB->pLineNumbers)) {
            lineNumbersWidth = drgui_get_width(pTB->pLineNumbers);
        }

        offsetX = pTB->borderWidth + pTB->padding + lineNumbersWidth;
        offsetY = pTB->borderWidth + pTB->padding;
    }


    if (pOffsetXOut != NULL) {
        *pOffsetXOut = offsetX;
    }
    if (pOffsetYOut != NULL) {
        *pOffsetYOut = offsetY;
    }
}

void dred_textbox__calculate_text_engine_container_size(dred_textbox* pTextBox, float* pWidthOut, float* pHeightOut)
{
    float width  = 0;
    float height = 0;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB != NULL)
    {
        float horzScrollbarSize = 0;
        if (drgui_is_visible(pTB->pHorzScrollbar)) {
            horzScrollbarSize = drgui_get_height(pTB->pHorzScrollbar);
        }

        float vertScrollbarSize = 0;
        if (drgui_is_visible(pTB->pVertScrollbar)) {
            vertScrollbarSize = drgui_get_width(pTB->pVertScrollbar);
        }

        float lineNumbersWidth = 0;
        if (drgui_is_visible(pTB->pLineNumbers)) {
            lineNumbersWidth = drgui_get_width(pTB->pLineNumbers);
        }

        width  = drgui_get_width(pTextBox)  - (pTB->borderWidth + pTB->padding)*2 - vertScrollbarSize - lineNumbersWidth;
        height = drgui_get_height(pTextBox) - (pTB->borderWidth + pTB->padding)*2 - horzScrollbarSize;
    }

    if (pWidthOut != NULL) {
        *pWidthOut = width;
    }
    if (pHeightOut != NULL) {
        *pHeightOut = height;
    }
}

drgui_rect dred_textbox__get_text_rect(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    if (pTB == NULL) {
        return drgui_make_rect(0, 0, 0, 0);
    }

    float offsetX;
    float offsetY;
    dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);

    float width;
    float height;
    dred_textbox__calculate_text_engine_container_size(pTextBox, &width, &height);

    return drgui_make_rect(offsetX, offsetY, offsetX + width, offsetY + height);
}


void dred_textbox__refresh_scrollbars(dred_textbox* pTextBox)
{
    // The layout depends on the range because we may be dynamically hiding and showing the scrollbars depending on the range. Thus, we
    // refresh the range first. However, dynamically showing and hiding the scrollbars (which is done when the layout is refreshed) affects
    // the size of the text box, which in turn affects the range. Thus, we need to refresh the ranges a second time after the layouts.

    dred_textbox__refresh_scrollbar_ranges(pTextBox);
    dred_textbox__refresh_scrollbar_layouts(pTextBox);
    dred_textbox__refresh_scrollbar_ranges(pTextBox);
}

void dred_textbox__refresh_scrollbar_ranges(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    // The vertical scrollbar is based on the line count.
    size_t lineCount = drte_engine_get_line_count(pTB->pTL);
    size_t pageSize  = drte_engine_get_visible_line_count_starting_at(pTB->pTL, drgui_sb_get_scroll_position(pTB->pVertScrollbar));
    drgui_sb_set_range_and_page_size(pTB->pVertScrollbar, 0, (int)(lineCount + pageSize - 1 - 1), (int)pageSize);     // -1 to make the range 0 based. -1 to ensure at least one line is visible.

    if (drgui_sb_is_thumb_visible(pTB->pVertScrollbar)) {
        if (!drgui_is_visible(pTB->pVertScrollbar)) {
            drgui_show(pTB->pVertScrollbar);
        }
    } else {
        if (drgui_is_visible(pTB->pVertScrollbar)) {
            drgui_hide(pTB->pVertScrollbar);
        }
    }


    // The horizontal scrollbar is a per-pixel scrollbar, and is based on the width of the text versus the width of the container.
    drgui_rect textRect = drte_engine_get_text_rect_relative_to_bounds(pTB->pTL);
    float containerWidth;
    drte_engine_get_container_size(pTB->pTL, &containerWidth, NULL);
    drgui_sb_set_range_and_page_size(pTB->pHorzScrollbar, 0, (int)(textRect.right - textRect.left + (containerWidth/2)), (int)containerWidth);

    if (drgui_sb_is_thumb_visible(pTB->pHorzScrollbar)) {
        if (!drgui_is_visible(pTB->pHorzScrollbar)) {
            drgui_show(pTB->pHorzScrollbar);
            dred_textbox__refresh_line_numbers(pTextBox);
        }
    } else {
        if (drgui_is_visible(pTB->pHorzScrollbar)) {
            drgui_hide(pTB->pHorzScrollbar);
            dred_textbox__refresh_line_numbers(pTextBox);
        }
    }
}

void dred_textbox__refresh_scrollbar_layouts(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    float offsetLeft   = pTB->borderWidth;
    float offsetTop    = pTB->borderWidth;
    float offsetRight  = pTB->borderWidth;
    float offsetBottom = pTB->borderWidth;

    float scrollbarSizeH = (drgui_sb_is_thumb_visible(pTB->pHorzScrollbar) && pTB->isHorzScrollbarEnabled) ? pTB->horzScrollbarSize : 0;
    float scrollbarSizeV = (drgui_sb_is_thumb_visible(pTB->pVertScrollbar) && pTB->isVertScrollbarEnabled) ? pTB->vertScrollbarSize : 0;

    drgui_set_size(pTB->pVertScrollbar, scrollbarSizeV, drgui_get_height(pTextBox) - scrollbarSizeH - (offsetTop + offsetBottom));
    drgui_set_size(pTB->pHorzScrollbar, drgui_get_width(pTextBox) - scrollbarSizeV - (offsetLeft + offsetRight), scrollbarSizeH);

    drgui_set_relative_position(pTB->pVertScrollbar, drgui_get_width(pTextBox) - scrollbarSizeV - offsetRight, offsetTop);
    drgui_set_relative_position(pTB->pHorzScrollbar, offsetLeft, drgui_get_height(pTextBox) - scrollbarSizeH - offsetBottom);


    // A change in the layout of the horizontal scrollbar will affect the layout of the line numbers.
    dred_textbox__refresh_line_numbers(pTextBox);
}

drgui_rect dred_textbox__get_scrollbar_dead_space_rect(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    float offsetLeft   = pTB->borderWidth;
    float offsetTop    = pTB->borderWidth;
    float offsetRight  = pTB->borderWidth;
    float offsetBottom = pTB->borderWidth;

    float scrollbarSizeH = (drgui_is_visible(pTB->pHorzScrollbar) && pTB->isHorzScrollbarEnabled) ? drgui_get_width(pTB->pHorzScrollbar) : 0;
    float scrollbarSizeV = (drgui_is_visible(pTB->pVertScrollbar) && pTB->isHorzScrollbarEnabled) ? drgui_get_height(pTB->pVertScrollbar) : 0;

    if (scrollbarSizeH == 0 && scrollbarSizeV == 0) {
        return drgui_make_rect(0, 0, 0, 0);
    }

    return drgui_make_rect(scrollbarSizeH + offsetLeft, scrollbarSizeV + offsetTop, drgui_get_width(pTextBox) - offsetRight, drgui_get_height(pTextBox) - offsetBottom);
}


void dred_textbox__on_mouse_move_line_numbers(drgui_element* pLineNumbers, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;

    dred_textbox* pTextBox = *(drgui_element**)drgui_get_extra_data(pLineNumbers);
    assert(pTextBox != NULL);

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    if ((stateFlags & DRGUI_MOUSE_BUTTON_LEFT_DOWN) != 0)
    {
        if (drgui_get_element_with_mouse_capture(pLineNumbers->pContext) == pLineNumbers)
        {
            // We just move the cursor around based on the line number we've moved over.
            drte_engine_enter_selection_mode(pTB->pTL);
            {
                //float offsetX = pTextEditorData->padding;
                float offsetY = pTB->padding;
                size_t iLine = drte_engine_get_line_at_pos_y(pTB->pTL, relativeMousePosY - offsetY);
                size_t iAnchorLine = pTB->iLineSelectAnchor;
                size_t lineCount = drte_engine_get_line_count(pTB->pTL);

                size_t iSelectionFirstLine = drte_engine_get_selection_first_line(pTB->pTL);
                size_t iSelectionLastLine = drte_engine_get_selection_last_line(pTB->pTL);
                if (iSelectionLastLine != iSelectionFirstLine) {
                    iSelectionLastLine -= 1;
                }

                // If we're moving updwards we want to position the cursor at the start of the line. Otherwise we want to move the cursor to the start
                // of the next line, or the end of the text.
                bool movingUp = false;
                if (iLine < iAnchorLine) {
                    movingUp = true;
                }

                // If we're moving up the selection anchor needs to be placed at the end of the last line. Otherwise we need to move it to the start
                // of the first line.
                if (movingUp) {
                    if (iAnchorLine + 1 < lineCount) {
                        drte_engine_move_selection_anchor_to_start_of_line(pTB->pTL, iAnchorLine + 1);
                    } else {
                        drte_engine_move_selection_anchor_to_end_of_line(pTB->pTL, iAnchorLine);
                    }
                } else {
                    drte_engine_move_selection_anchor_to_start_of_line(pTB->pTL, iAnchorLine);
                }


                // If we're moving up we want the cursor to be placed at the start of the selection range. Otherwise we want to place the cursor
                // at the end of the selection range.
                if (movingUp) {
                    drte_engine_move_cursor_to_start_of_line_by_index(pTB->pTL, iLine);
                } else {
                    if (iLine + 1 < lineCount) {
                        drte_engine_move_cursor_to_start_of_line_by_index(pTB->pTL, iLine + 1);
                    } else {
                        drte_engine_move_cursor_to_end_of_line_by_index(pTB->pTL, iLine);
                    }
                }
            }
            drte_engine_leave_selection_mode(pTB->pTL);
        }
    }
}

void dred_textbox__on_mouse_button_down_line_numbers(drgui_element* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)stateFlags;

    dred_textbox* pTextBox = *(drgui_element**)drgui_get_extra_data(pLineNumbers);
    assert(pTextBox != NULL);

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    if (mouseButton == DRGUI_MOUSE_BUTTON_LEFT)
    {
        //float offsetX = pTextEditorData->padding;
        float offsetY = pTB->padding;
        pTB->iLineSelectAnchor = drte_engine_get_line_at_pos_y(pTB->pTL, relativeMousePosY - offsetY);

        drte_engine_deselect_all(pTB->pTL);

        drte_engine_move_cursor_to_start_of_line_by_index(pTB->pTL, pTB->iLineSelectAnchor);

        drte_engine_enter_selection_mode(pTB->pTL);
        {
            if (pTB->iLineSelectAnchor + 1 < drte_engine_get_line_count(pTB->pTL)) {
                drte_engine_move_cursor_to_start_of_line_by_index(pTB->pTL, pTB->iLineSelectAnchor + 1);
            } else {
                drte_engine_move_cursor_to_end_of_line(pTB->pTL);
            }
        }
        drte_engine_leave_selection_mode(pTB->pTL);

        drgui_capture_mouse(pLineNumbers);
    }
}

void dred_textbox__on_mouse_button_up_line_numbers(drgui_element* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    if (mouseButton == DRGUI_MOUSE_BUTTON_LEFT) {
        if (drgui_has_mouse_capture(pLineNumbers)) {
            drgui_release_mouse(pLineNumbers->pContext);
        }
    }
}

void dred_textbox__on_paint_rect_line_numbers(drte_engine* pLayout, drgui_rect rect, drgui_color color, dred_textbox* pTextBox, void* pPaintData)
{
    (void)pLayout;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    float offsetX = pTB->padding;
    float offsetY = pTB->padding;

    drgui_draw_rect(pTB->pLineNumbers, drgui_offset_rect(rect, offsetX, offsetY), color, pPaintData);
}

void dred_textbox__on_paint_text_line_numbers(drte_engine* pLayout, drte_text_run* pRun, dred_textbox* pTextBox, void* pPaintData)
{
    (void)pLayout;

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    float offsetX = pTB->padding;
    float offsetY = pTB->padding;
    drgui_draw_text(pTB->pLineNumbers, pRun->pFont, pRun->text, (int)pRun->textLength, (float)pRun->posX + offsetX, (float)pRun->posY + offsetY, pRun->textColor, pRun->backgroundColor, pPaintData);
}

void dred_textbox__on_paint_line_numbers(drgui_element* pLineNumbers, drgui_rect relativeRect, void* pPaintData)
{
    (void)relativeRect;

    dred_textbox* pTextBox = *((drgui_element**)drgui_get_extra_data(pLineNumbers));
    assert(pTextBox != NULL);

    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    float lineNumbersWidth  = drgui_get_width(pLineNumbers) - (pTB->padding*2) - pTB->lineNumbersPaddingRight;
    float lineNumbersHeight = drgui_get_height(pLineNumbers) - (pTB->padding*2);

    drte_engine_paint_line_numbers(pTB->pTL, lineNumbersWidth, lineNumbersHeight, pTB->lineNumbersColor, pTB->lineNumbersBackgroundColor, dred_textbox__on_paint_text_line_numbers, dred_textbox__on_paint_rect_line_numbers, pTextBox, pPaintData);

    drgui_draw_rect_outline(pLineNumbers, drgui_get_local_rect(pLineNumbers), pTB->lineNumbersBackgroundColor, pTB->padding, pPaintData);

    // Right padding.
    drgui_rect rightPaddingRect = drgui_get_local_rect(pLineNumbers);
    rightPaddingRect.right -= pTB->padding;
    rightPaddingRect.left   = rightPaddingRect.right - pTB->lineNumbersPaddingRight;
    drgui_draw_rect(pLineNumbers, rightPaddingRect, pTB->lineNumbersBackgroundColor, pPaintData);
}

void dred_textbox__refresh_line_numbers(dred_textbox* pTextBox)
{
    dred_textbox_data* pTB = (dred_textbox_data*)dred_control_get_extra_data(pTextBox);
    assert(pTB != NULL);

    float lineNumbersWidth = 0;
    if (drgui_is_visible(pTB->pLineNumbers)) {
        lineNumbersWidth = pTB->lineNumbersWidth;
    }

    float scrollbarHeight = drgui_is_visible(pTB->pHorzScrollbar) ? drgui_get_height(pTB->pHorzScrollbar) : 0;
    drgui_set_size(pTB->pLineNumbers, lineNumbersWidth, drgui_get_height(pTextBox) - scrollbarHeight);


    // The size of the text container may have changed.
    float textEditorWidth;
    float textEditorHeight;
    dred_textbox__calculate_text_engine_container_size(pTextBox, &textEditorWidth, &textEditorHeight);
    drte_engine_set_container_size(pTB->pTL, textEditorWidth, textEditorHeight);


    // Force a redraw just to be sure everything is in a valid state.
    drgui_dirty(pTextBox, drgui_get_local_rect(pTextBox));
}
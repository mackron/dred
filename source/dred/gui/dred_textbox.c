// Copyright (C) 2016 David Reid. See included LICENSE file.

/// Retrieves the offset to draw the text in the text box.
void dred_textbox__get_text_offset(dred_textbox* pTextBox, float* pOffsetXOut, float* pOffsetYOut);

/// Calculates the required size of the text engine.
void dred_textbox__calculate_text_engine_container_size(dred_textbox* pTextBox, float* pWidthOut, float* pHeightOut);

/// Retrieves the rectangle of the text engine's container.
dred_rect dred_textbox__get_text_rect(dred_textbox* pTextBox);

/// Refreshes the range, page sizes and layouts of the scrollbars.
void dred_textbox__refresh_scrollbars(dred_textbox* pTextBox);

/// Refreshes the range and page sizes of the scrollbars.
void dred_textbox__refresh_scrollbar_ranges(dred_textbox* pTextBox);

/// Refreshes the size and position of the scrollbars.
void dred_textbox__refresh_scrollbar_layouts(dred_textbox* pTextBox);

/// Retrieves a rectangle representing the space between the edges of the two scrollbars.
dred_rect dred_textbox__get_scrollbar_dead_space_rect(dred_textbox* pTextBox);


/// Called when a mouse button is pressed on the line numbers element.
void dred_textbox__on_mouse_move_line_numbers(dred_control* pLineNumbers, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when a mouse button is pressed on the line numbers element.
void dred_textbox__on_mouse_button_down_line_numbers(dred_control* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when a mouse button is pressed on the line numbers element.
void dred_textbox__on_mouse_button_up_line_numbers(dred_control* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when the line numbers element needs to be drawn.
void dred_textbox__on_paint_line_numbers(dred_control* pLineNumbers, dred_rect relativeRect, void* pPaintData);

/// Refreshes the line number of the given text editor.
void dred_textbox__refresh_line_numbers(dred_textbox* pTextBox);


/// on_paint_rect()
void dred_textbox_engine__on_paint_rect(drte_engine* pLayout, drte_style_token styleToken, drte_rect rect, void* pPaintData);

/// on_paint_text()
void dred_textbox_engine__on_paint_text(drte_engine* pTL, drte_style_token styleTokenFG, drte_style_token styleTokenBG, const char* text, size_t textLength, float posX, float posY, void* pPaintData);

/// on_dirty()
void dred_textbox_engine__on_dirty(drte_engine* pTL, drte_rect rect);

/// on_cursor_move()
void dred_textbox_engine__on_cursor_move(drte_engine* pTL);

/// on_text_changed()
void dred_textbox_engine__on_text_changed(drte_engine* pTL);

/// on_undo_point_changed()
void dred_textbox_engine__on_undo_point_changed(drte_engine* pTL, unsigned int iUndoPoint);

// on_get_undo_state()
size_t dred_textbox_engine__on_get_undo_state(drte_engine* pTL, void* pDataOut);

// on_apply_undo_state()
void dred_textbox_engine__on_apply_undo_state(drte_engine* pTL, size_t dataSize, const void* pData);


// dred_textbox__refresh_horizontal_scrollbar()
void dred_textbox__refresh_horizontal_scrollbar(dred_textbox* pTextBox);

void dred_textbox__on_vscroll(dred_scrollbar* pSBControl, int scrollPos)
{
    dred_textbox* pTextBox = (dred_textbox*)DRED_CONTROL(pSBControl)->pUserData;
    assert(pTextBox != NULL);

    drte_engine_set_inner_offset_y(pTextBox->pTL, -drte_engine_get_line_pos_y(pTextBox->pTL, scrollPos));
    dred_textbox__refresh_scrollbars(pTextBox);

    // The line numbers need to be redrawn.
    dred_control_dirty(pTextBox->pLineNumbers, dred_control_get_local_rect(pTextBox->pLineNumbers));
}

void dred_textbox__on_hscroll(dred_scrollbar* pSBControl, int scrollPos)
{
    dred_textbox* pTextBox = (dred_textbox*)DRED_CONTROL(pSBControl)->pUserData;
    assert(pTextBox != NULL);

    drte_engine_set_inner_offset_x(pTextBox->pTL, (float)-scrollPos);
}

void dred_textbox__refresh_style(dred_textbox* pTextBox)
{
    assert(pTextBox != NULL);

    dred_gui_font_metrics fontMetrics;
    dred_gui_get_font_metrics(pTextBox->defaultStyle.pFont, &fontMetrics);

    // Default.
    drte_engine_register_style_token(pTextBox->pTL, (drte_style_token)&pTextBox->defaultStyle, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));

    // Selection.
    drte_engine_register_style_token(pTextBox->pTL, (drte_style_token)&pTextBox->selectionStyle, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));

    // Active line.
    drte_engine_register_style_token(pTextBox->pTL, (drte_style_token)&pTextBox->activeLineStyle, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));

    // Cursor.
    drte_engine_register_style_token(pTextBox->pTL, (drte_style_token)&pTextBox->cursorStyle, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));

    // Line numbers.
    drte_engine_register_style_token(pTextBox->pTL, (drte_style_token)&pTextBox->lineNumbersStyle, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));
}


void dred_textbox_engine__on_measure_string_proc(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textLength, float* pWidthOut, float* pHeightOut)
{
    (void)pEngine;
    dred_gui_measure_string(((dred_text_style*)styleToken)->pFont, text, textLength, pWidthOut, pHeightOut);
}

void dred_textbox_engine__on_get_cursor_position_from_point(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut)
{
    (void)pEngine;
    dred_gui_get_text_cursor_position_from_point(((dred_text_style*)styleToken)->pFont, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosXOut, pCharacterIndexOut);
}

void dred_textbox_engine__on_get_cursor_position_from_char(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t characterIndex, float* pTextCursorPosXOut)
{
    (void)pEngine;
    dred_gui_get_text_cursor_position_from_char(((dred_text_style*)styleToken)->pFont, text, characterIndex, pTextCursorPosXOut);
}


void dred_textbox__clear_all_cursors(dred_textbox* pTextBox)
{
    // The last cursor is _not_ cleared.
    assert(pTextBox != NULL);

    // Engine.
    drte_engine__begin_dirty(pTextBox->pTL);
    while (pTextBox->pTL->cursorCount > 1) {
        drte_engine_remove_cursor(pTextBox->pTL, pTextBox->pTL->cursorCount-2);
    }
    drte_engine__end_dirty(pTextBox->pTL);


    // Local list.
    if (pTextBox->cursorCount > 1) {
        pTextBox->pCursors[0] = pTextBox->pCursors[pTextBox->cursorCount-1];
        pTextBox->pCursors[0].iEngineSelection = (size_t)-1;
        pTextBox->cursorCount = 1;
    }
}

void dred_textbox__remove_cursor(dred_textbox* pTextBox, size_t iCursor)
{
    assert(pTextBox != NULL);

    // Remove from the engine.
    drte_engine_remove_cursor(pTextBox->pTL, iCursor);

    // Remove from the local list.
    for (size_t i = iCursor; i < pTextBox->cursorCount-1; ++i) {
        pTextBox->pCursors[i] = pTextBox->pCursors[i+1];
    }
    pTextBox->cursorCount -= 1;
}

void dred_textbox__insert_cursor(dred_textbox* pTextBox, size_t iChar)
{
    assert(pTextBox != NULL);

    // If we are trying to insert a cursor on top of an existing cursor we need to just move the existing one to the end of the list,
    // thus making it the current cursor. We don't want cursors to be sitting on top of each other.
    size_t iExistingCursor = (size_t)-1;
    for (size_t iCursor = 0; iCursor < pTextBox->pTL->cursorCount; ++iCursor) {
        if (pTextBox->pTL->pCursors[iCursor].iCharAbs == iChar) {
            iExistingCursor = iCursor;
            break;
        }
    }

    size_t iEngineSelection = (size_t)-1;
    if (iExistingCursor != (size_t)-1) {
        iEngineSelection = pTextBox->pCursors[iExistingCursor].iEngineSelection;
        dred_textbox__remove_cursor(pTextBox, iExistingCursor);
    }

    size_t iEngineCursor = drte_engine_insert_cursor(pTextBox->pTL, iChar);

    dred_textbox_cursor* pNewCursors = (dred_textbox_cursor*)realloc(pTextBox->pCursors, (pTextBox->cursorCount+1) * sizeof(*pNewCursors));
    if (pNewCursors == NULL) {
        return;
    }

    pTextBox->pCursors = pNewCursors;
    pTextBox->pCursors[pTextBox->cursorCount].iEngineSelection = iEngineSelection;
    pTextBox->pCursors[pTextBox->cursorCount].iEngineCursor = iEngineCursor;
    pTextBox->cursorCount += 1;
}

bool dred_textbox__get_cursor_selection(dred_textbox* pTextBox, size_t iCursor, size_t* iSelectionOut)
{
    assert(pTextBox != NULL);

    for (size_t iSelection = 0; iSelection < pTextBox->pTL->selectionCount; ++iSelection) {
        drte_region selection = drte_region_normalize(pTextBox->pTL->pSelections[iSelection]);
        if (selection.iCharBeg == pTextBox->pTL->pCursors[iCursor].iCharAbs || selection.iCharEnd == pTextBox->pTL->pCursors[iCursor].iCharAbs) {
            if (iSelectionOut) *iSelectionOut = iSelection;
            return true;
        }
    }

    return false;
}

bool dred_textbox__is_cursor_on_selection(dred_textbox* pTextBox)
{
    assert(pTextBox != NULL);

    return dred_textbox__get_cursor_selection(pTextBox, drte_engine_get_last_cursor(pTextBox->pTL), NULL);
}

bool dred_textbox__move_cursor_to_start_of_selection(dred_textbox* pTextBox, size_t* iSelectionOut)
{
    assert(pTextBox != NULL);

    if (pTextBox->pTL->cursorCount == 0) {
        return false;
    }

    // We need to find the selection region that the last cursor is sitting at the end of. If there isn't one, we just return false.
    size_t iCursor = drte_engine_get_last_cursor(pTextBox->pTL);
    for (size_t iSelection = 0; iSelection < pTextBox->pTL->selectionCount; ++iSelection) {
        drte_region selection = drte_region_normalize(pTextBox->pTL->pSelections[iSelection]);
        if (selection.iCharEnd == pTextBox->pTL->pCursors[iCursor].iCharAbs) {
            // It's on this selection.
            drte_engine_move_cursor_to_character(pTextBox->pTL, iCursor, selection.iCharBeg);
            if (iSelectionOut) *iSelectionOut = iSelection;
            return true;
        }
    }

    return false;
}

bool dred_textbox__move_cursor_to_end_of_selection(dred_textbox* pTextBox, size_t* iSelectionOut)
{
    assert(pTextBox != NULL);

    if (pTextBox->pTL->cursorCount == 0) {
        return false;
    }

    // We need to find the selection region that the last cursor is sitting at the end of. If there isn't one, we just return false.
    size_t iCursor = drte_engine_get_last_cursor(pTextBox->pTL);
    for (size_t iSelection = 0; iSelection < pTextBox->pTL->selectionCount; ++iSelection) {
        drte_region selection = drte_region_normalize(pTextBox->pTL->pSelections[iSelection]);
        if (selection.iCharBeg == pTextBox->pTL->pCursors[iCursor].iCharAbs) {
            // It's on this selection.
            drte_engine_move_cursor_to_character(pTextBox->pTL, iCursor, selection.iCharEnd);
            if (iSelectionOut) *iSelectionOut = iSelection;
            return true;
        }
    }

    return false;
}

bool dred_textbox__insert_tab(dred_textbox* pTextBox, size_t iChar)
{
    assert(pTextBox != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pTextBox));
    assert(pDred != NULL);

    drte_engine__begin_dirty(pTextBox->pTL);

    bool wasTextChanged = false;
    size_t insertedCharacterCount;
    if (pDred->config.textEditorTabsToSpacesEnabled) {
        insertedCharacterCount = drte_engine_get_spaces_to_next_column_from_character(pTextBox->pTL, iChar);
        for (size_t i = 0; i < insertedCharacterCount; ++i) {
            wasTextChanged = drte_engine_insert_character(pTextBox->pTL, iChar, ' ') || wasTextChanged;
        }
    } else {
        insertedCharacterCount = 1;
        wasTextChanged = drte_engine_insert_character(pTextBox->pTL, iChar, '\t') || wasTextChanged;
    }

    
    // Any cursor whose character position comes after this cursor needs to be moved.
    for (size_t iCursor2 = 0; iCursor2 < pTextBox->pTL->cursorCount; ++iCursor2) {
        if (pTextBox->pTL->pCursors[iCursor2].iCharAbs >= iChar) {
            drte_engine_move_cursor_to_character(pTextBox->pTL, iCursor2, pTextBox->pTL->pCursors[iCursor2].iCharAbs + insertedCharacterCount);
        }
    }

    // As with cursors, selections need to be updated too.
    for (size_t iSelection = 0; iSelection < pTextBox->pTL->selectionCount; ++iSelection) {
        drte_region selection = drte_region_normalize(pTextBox->pTL->pSelections[iSelection]);
        if (selection.iCharBeg > iChar) {
            pTextBox->pTL->pSelections[iSelection].iCharBeg += insertedCharacterCount;
            pTextBox->pTL->pSelections[iSelection].iCharEnd += insertedCharacterCount;
        } else {
            if (selection.iCharEnd > iChar) {
                pTextBox->pTL->pSelections[iSelection].iCharEnd += insertedCharacterCount;
            }
        }
    }


    drte_engine__end_dirty(pTextBox->pTL);
    return wasTextChanged;
}

bool dred_textbox__insert_tab_at_cursor(dred_textbox* pTextBox, size_t iCursor)
{
    assert(pTextBox != NULL);

    return dred_textbox__insert_tab(pTextBox, pTextBox->pTL->pCursors[iCursor].iCharAbs);
}


bool dred_textbox_init(dred_textbox* pTextBox, dred_context* pDred, dred_control* pParent)
{
    if (pTextBox == NULL) {
        return false;
    }

    memset(pTextBox, 0, sizeof(*pTextBox));
    if (!dred_control_init(DRED_CONTROL(pTextBox), pDred, pParent, DRED_CONTROL_TYPE_TEXTBOX)) {
        return false;
    }

    dred_control_set_cursor(DRED_CONTROL(pTextBox), dred_cursor_text);
    dred_control_set_on_size(DRED_CONTROL(pTextBox), dred_textbox_on_size);
    dred_control_set_on_mouse_move(DRED_CONTROL(pTextBox), dred_textbox_on_mouse_move);
    dred_control_set_on_mouse_button_down(DRED_CONTROL(pTextBox), dred_textbox_on_mouse_button_down);
    dred_control_set_on_mouse_button_up(DRED_CONTROL(pTextBox), dred_textbox_on_mouse_button_up);
    dred_control_set_on_mouse_button_dblclick(DRED_CONTROL(pTextBox), dred_textbox_on_mouse_button_dblclick);
    dred_control_set_on_mouse_wheel(DRED_CONTROL(pTextBox), dred_textbox_on_mouse_wheel);
    dred_control_set_on_key_down(DRED_CONTROL(pTextBox), dred_textbox_on_key_down);
    dred_control_set_on_printable_key_down(DRED_CONTROL(pTextBox), dred_textbox_on_printable_key_down);
    dred_control_set_on_paint(DRED_CONTROL(pTextBox), dred_textbox_on_paint);
    dred_control_set_on_capture_keyboard(DRED_CONTROL(pTextBox), dred_textbox_on_capture_keyboard);
    dred_control_set_on_release_keyboard(DRED_CONTROL(pTextBox), dred_textbox_on_release_keyboard);
    dred_control_set_on_capture_mouse(DRED_CONTROL(pTextBox), dred_textbox_on_capture_mouse);
    dred_control_set_on_release_mouse(DRED_CONTROL(pTextBox), dred_textbox_on_release_mouse);

    pTextBox->pVertScrollbar = &pTextBox->vertScrollbar;
    dred_scrollbar_init(pTextBox->pVertScrollbar, pDred, DRED_CONTROL(pTextBox), dred_scrollbar_orientation_vertical);
    DRED_CONTROL(pTextBox->pVertScrollbar)->pUserData = pTextBox;
    dred_scrollbar_set_on_scroll(pTextBox->pVertScrollbar, dred_textbox__on_vscroll);
    dred_scrollbar_set_mouse_wheel_scele(pTextBox->pVertScrollbar, 3);

    pTextBox->pHorzScrollbar = &pTextBox->horzScrollbar;
    dred_scrollbar_init(pTextBox->pHorzScrollbar, pDred, DRED_CONTROL(pTextBox), dred_scrollbar_orientation_horizontal);
    DRED_CONTROL(pTextBox->pHorzScrollbar)->pUserData = pTextBox;
    dred_scrollbar_set_on_scroll(pTextBox->pHorzScrollbar, dred_textbox__on_hscroll);

    pTextBox->pLineNumbers = &pTextBox->lineNumbers;
    dred_control_init(pTextBox->pLineNumbers, pDred, DRED_CONTROL(pTextBox), "dred.common.linenumbers");
    dred_control_hide(pTextBox->pLineNumbers);
    dred_control_set_on_mouse_move(pTextBox->pLineNumbers, dred_textbox__on_mouse_move_line_numbers);
    dred_control_set_on_mouse_button_down(pTextBox->pLineNumbers, dred_textbox__on_mouse_button_down_line_numbers);
    dred_control_set_on_mouse_button_up(pTextBox->pLineNumbers, dred_textbox__on_mouse_button_up_line_numbers);
    dred_control_set_on_paint(pTextBox->pLineNumbers, dred_textbox__on_paint_line_numbers);

    pTextBox->pTL = drte_engine_create(pTextBox);
    if (pTextBox->pTL == NULL) {
        dred_control_uninit(DRED_CONTROL(pTextBox));
        return false;
    }

    dred_textbox__insert_cursor(pTextBox, 0);


    pTextBox->pTL->onMeasureString = dred_textbox_engine__on_measure_string_proc;
    pTextBox->pTL->onGetCursorPositionFromPoint = dred_textbox_engine__on_get_cursor_position_from_point;
    pTextBox->pTL->onGetCursorPositionFromChar = dred_textbox_engine__on_get_cursor_position_from_char;


    pTextBox->defaultStyle.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextBox->defaultStyle.bgColor = dred_rgb(64, 64, 64);
    pTextBox->defaultStyle.fgColor = dred_rgb(0, 0, 0);

    pTextBox->selectionStyle.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextBox->selectionStyle.bgColor = dred_rgb(64, 128, 192);
    pTextBox->selectionStyle.fgColor = dred_rgb(0, 0, 0);

    pTextBox->activeLineStyle.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextBox->activeLineStyle.bgColor = dred_rgb(64, 64, 64);
    pTextBox->activeLineStyle.fgColor = dred_rgb(0, 0, 0);

    pTextBox->cursorStyle.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextBox->cursorStyle.bgColor = dred_rgb(0, 0, 0);
    pTextBox->cursorStyle.fgColor = dred_rgb(0, 0, 0);

    pTextBox->lineNumbersStyle.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextBox->lineNumbersStyle.bgColor = dred_rgb(64, 64, 64);
    pTextBox->lineNumbersStyle.fgColor = dred_rgb(80, 160, 192);


    // Test styling.
    /*pTextBox->testStyle0.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextBox->testStyle0.bgColor = dred_rgb(64, 64, 64);
    pTextBox->testStyle0.fgColor = dred_rgb(64, 160, 255);

    pTextBox->testStyle1.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextBox->testStyle1.bgColor = dred_rgb(64, 64, 64);
    pTextBox->testStyle1.fgColor = dred_rgb(64, 192, 92);*/


    // Register the styles with the text engine.
    dred_textbox__refresh_style(pTextBox);


    //drte_engine_set_highlighter(pTextBox->pTL, dred_textbox_engine__on_get_next_highlight, pTextBox);
    


    drte_engine_set_default_style(pTextBox->pTL, (drte_style_token)&pTextBox->defaultStyle);
    drte_engine_set_selection_style(pTextBox->pTL, (drte_style_token)&pTextBox->selectionStyle);
    drte_engine_set_active_line_style(pTextBox->pTL, (drte_style_token)&pTextBox->activeLineStyle);
    drte_engine_set_cursor_style(pTextBox->pTL, (drte_style_token)&pTextBox->cursorStyle);
    drte_engine_set_line_numbers_style(pTextBox->pTL, (drte_style_token)&pTextBox->lineNumbersStyle);


    drte_engine_set_on_paint_rect(pTextBox->pTL, dred_textbox_engine__on_paint_rect);
    drte_engine_set_on_paint_text(pTextBox->pTL, dred_textbox_engine__on_paint_text);
    drte_engine_set_on_dirty(pTextBox->pTL, dred_textbox_engine__on_dirty);
    drte_engine_set_on_cursor_move(pTextBox->pTL, dred_textbox_engine__on_cursor_move);
    drte_engine_set_on_text_changed(pTextBox->pTL, dred_textbox_engine__on_text_changed);
    drte_engine_set_on_undo_point_changed(pTextBox->pTL, dred_textbox_engine__on_undo_point_changed);
    pTextBox->pTL->onGetUndoState = dred_textbox_engine__on_get_undo_state;
    pTextBox->pTL->onApplyUndoState = dred_textbox_engine__on_apply_undo_state;

    //drte_engine_set_default_text_color(pTextBox->pTL, dred_rgb(0, 0, 0));
    //drte_engine_set_cursor_color(pTextBox->pTL, dred_rgb(0, 0, 0));
    //drte_engine_set_default_bg_color(pTextBox->pTL, dred_rgb(64, 64, 64));
    //drte_engine_set_active_line_bg_color(pTextBox->pTL, dred_rgb(64, 64, 64));

    pTextBox->borderColor = dred_rgb(0, 0, 0);
    pTextBox->borderWidth = 0;
    pTextBox->padding     = 2;
    pTextBox->lineNumbersWidth = 64;
    pTextBox->lineNumbersPaddingRight = 16;
    //pTextBox->lineNumbersColor = dred_rgb(80, 160, 192);
    //pTextBox->lineNumbersBackgroundColor = pTextBox->defaultStyle.bgColor;
    pTextBox->vertScrollbarSize = 16;
    pTextBox->horzScrollbarSize = 16;
    pTextBox->isVertScrollbarEnabled = true;
    pTextBox->isHorzScrollbarEnabled = true;
    pTextBox->isExcessScrollingEnabled = true;
    pTextBox->iLineSelectAnchor = 0;
    pTextBox->onCursorMove = NULL;
    pTextBox->onUndoPointChanged = NULL;

    return true;
}

void dred_textbox_uninit(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    // Keyboard focus needs to be released first. If we don't do this we'll not free delete the internal timer.
    if (dred_control_has_keyboard_capture(DRED_CONTROL(pTextBox))) {
        dred_gui_release_keyboard(dred_control_get_gui(DRED_CONTROL(pTextBox)));
    }

    if (pTextBox->pTL) {
        drte_engine_delete(pTextBox->pTL);
        pTextBox->pTL = NULL;
    }

    if (pTextBox->pLineNumbers) {
        dred_control_uninit(pTextBox->pLineNumbers);
        pTextBox->pLineNumbers = NULL;
    }

    if (pTextBox->pHorzScrollbar) {
        dred_scrollbar_uninit(pTextBox->pHorzScrollbar);
        pTextBox->pHorzScrollbar = NULL;
    }

    if (pTextBox->pVertScrollbar) {
        dred_scrollbar_uninit(pTextBox->pVertScrollbar);
        pTextBox->pVertScrollbar = NULL;
    }

    dred_control_uninit(DRED_CONTROL(pTextBox));
}


drte_engine* dred_textbox_get_engine(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return NULL;
    }

    return pTextBox->pTL;
}


void dred_textbox_set_font(dred_textbox* pTextBox, dred_gui_font* pFont)
{
    if (pTextBox == NULL) {
        return;
    }

    dred_control_begin_dirty(DRED_CONTROL(pTextBox));
    {
        //drte_engine_set_default_font(pTextBox->pTL, pFont);
        pTextBox->defaultStyle.pFont = pFont;
        pTextBox->lineNumbersStyle.pFont = pFont;
        //pTextBox->testStyle0.pFont = pFont;

        dred_textbox__refresh_style(pTextBox);

        // The font used for line numbers are tied to the main font at the moment.
        dred_textbox__refresh_line_numbers(pTextBox);

        // Emulate a scroll to ensure the scroll position is pinned to a line.
        dred_textbox__on_vscroll(pTextBox->pVertScrollbar, dred_scrollbar_get_scroll_position(pTextBox->pVertScrollbar));
        dred_textbox__refresh_scrollbars(pTextBox);
    }
    dred_control_end_dirty(DRED_CONTROL(pTextBox));
}

dred_gui_font* dred_textbox_get_font(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return NULL;
    }

    return pTextBox->defaultStyle.pFont;
}

void dred_textbox_set_text_color(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->defaultStyle.fgColor = color;
    dred_textbox__refresh_style(pTextBox);
}

void dred_textbox_set_background_color(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->defaultStyle.bgColor = color;
    dred_textbox__refresh_style(pTextBox);
}

void dred_textbox_set_selection_background_color(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->selectionStyle.bgColor = color;
    dred_textbox__refresh_style(pTextBox);
}

dred_color dred_textbox_get_selection_background_color(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTextBox->selectionStyle.bgColor;
}

void dred_textbox_set_active_line_background_color(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->activeLineStyle.bgColor = color;
    dred_textbox__refresh_style(pTextBox);
    //drte_engine_set_active_line_bg_color(pTextBox->pTL, color);
}

void dred_textbox_set_cursor_width(dred_textbox* pTextBox, float cursorWidth)
{
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_set_cursor_width(pTextBox->pTL, cursorWidth);
}

float dred_textbox_get_cursor_width(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return drte_engine_get_cursor_width(pTextBox->pTL);
}

void dred_textbox_set_cursor_color(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->cursorStyle.bgColor = color;
    dred_textbox__refresh_style(pTextBox);
    //drte_engine_set_cursor_color(pTextBox->pTL, color);
}

void dred_textbox_set_border_color(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->borderColor = color;
}

void dred_textbox_set_border_width(dred_textbox* pTextBox, float borderWidth)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->borderWidth = borderWidth;
}

void dred_textbox_set_padding(dred_textbox* pTextBox, float padding)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->padding = padding;
}

float dred_textbox_get_padding_vert(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return pTextBox->padding;
}

float dred_textbox_get_padding_horz(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return pTextBox->padding;
}

void dred_textbox_set_line_numbers_width(dred_textbox* pTextBox, float lineNumbersWidth)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->lineNumbersWidth = lineNumbersWidth;
}

float dred_textbox_get_line_numbers_width(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return pTextBox->lineNumbersWidth;
}

void dred_textbox_set_line_numbers_padding(dred_textbox* pTextBox, float lineNumbersPadding)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->lineNumbersPaddingRight = lineNumbersPadding;
}

float dred_textbox_get_line_numbers_padding(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return pTextBox->lineNumbersPaddingRight;
}

void dred_textbox_set_line_numbers_color(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }

    //pTextBox->lineNumbersColor = color;
    pTextBox->lineNumbersStyle.fgColor = color;
    dred_textbox__refresh_style(pTextBox);
    dred_textbox__refresh_line_numbers(pTextBox);
}

dred_color dred_textbox_get_line_numbers_color(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTextBox->lineNumbersStyle.fgColor;
}

void dred_textbox_set_line_numbers_background_color(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }
    
    pTextBox->lineNumbersStyle.bgColor = color;
    dred_textbox__refresh_style(pTextBox);
    dred_textbox__refresh_line_numbers(pTextBox);
}

dred_color dred_textbox_get_line_numbers_background_color(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTextBox->lineNumbersStyle.bgColor;
}

void dred_textbox_set_scrollbar_track_color(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }

    dred_scrollbar_set_track_color(pTextBox->pHorzScrollbar, color);
    dred_scrollbar_set_track_color(pTextBox->pVertScrollbar, color);
}

void dred_textbox_set_scrollbar_thumb_color(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }

    dred_scrollbar_set_default_thumb_color(pTextBox->pHorzScrollbar, color);
    dred_scrollbar_set_default_thumb_color(pTextBox->pVertScrollbar, color);
}

void dred_textbox_set_scrollbar_thumb_color_hovered(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }

    dred_scrollbar_set_hovered_thumb_color(pTextBox->pHorzScrollbar, color);
    dred_scrollbar_set_hovered_thumb_color(pTextBox->pVertScrollbar, color);
}

void dred_textbox_set_scrollbar_thumb_color_pressed(dred_textbox* pTextBox, dred_color color)
{
    if (pTextBox == NULL) {
        return;
    }

    dred_scrollbar_set_pressed_thumb_color(pTextBox->pHorzScrollbar, color);
    dred_scrollbar_set_pressed_thumb_color(pTextBox->pVertScrollbar, color);
}


void dred_textbox_enable_word_wrap(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_enable_word_wrap(pTextBox->pTL);

    // Line numbers need to be redrawn.
    dred_textbox__refresh_line_numbers(pTextBox);
}

void dred_textbox_disable_word_wrap(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_disable_word_wrap(pTextBox->pTL);

    // Line numbers need to be redrawn.
    dred_textbox__refresh_line_numbers(pTextBox);
}

bool dred_textbox_is_word_wrap_enabled(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;
    }

    return drte_engine_is_word_wrap_enabled(pTextBox->pTL);
}



void dred_textbox_set_text(dred_textbox* pTextBox, const char* text)
{
    if (pTextBox == NULL) {
        return;
    }


    // The cursors and selection regions need to be cancelled here to ensure they don't reference invalid regions due to a
    // change in text. This should not have any major usability issues, but it can be tweaked if need be.
    dred_textbox__clear_all_cursors(pTextBox);
    drte_engine_deselect_all(pTextBox->pTL);    
    size_t iCursorChar = drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));


    // Set the text.
    drte_engine_prepare_undo_point(pTextBox->pTL);
    {
        drte_engine_set_text(pTextBox->pTL, text);
    }
    drte_engine_commit_undo_point(pTextBox->pTL);


    // Restore cursors.
    drte_engine_move_cursor_to_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), iCursorChar);
}

size_t dred_textbox_get_text(dred_textbox* pTextBox, char* pTextOut, size_t textOutSize)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return drte_engine_get_text(pTextBox->pTL, pTextOut, textOutSize);
}

void dred_textbox_step(dred_textbox* pTextBox, unsigned int milliseconds)
{
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_step(pTextBox->pTL, milliseconds);
}

void dred_textbox_set_cursor_blink_rate(dred_textbox* pTextBox, unsigned int blinkRateInMilliseconds)
{
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_set_cursor_blink_rate(pTextBox->pTL, blinkRateInMilliseconds);
}

void dred_textbox_move_cursor_to_end_of_text(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_move_cursor_to_end_of_text(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
}

void dred_textbox_move_cursor_to_start_of_line_by_index(dred_textbox* pTextBox, size_t iLine)
{
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_move_cursor_to_start_of_line_by_index(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), iLine);
}


bool dred_textbox_is_anything_selected(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;
    }

    return drte_engine_is_anything_selected(pTextBox->pTL);
}

void dred_textbox_select_all(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_select_all(pTextBox->pTL);
}

void dred_textbox_deselect_all(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_deselect_all(pTextBox->pTL);
    dred_textbox__clear_all_cursors(pTextBox);
}

size_t dred_textbox_get_selected_text(dred_textbox* pTextBox, char* textOut, size_t textOutLength)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return drte_engine_get_selected_text(pTextBox->pTL, textOut, textOutLength);
}

bool dred_textbox_delete_character_to_right_of_cursor(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;
    }

    bool wasTextChanged = false;
    drte_engine_prepare_undo_point(pTextBox->pTL);
    {
        wasTextChanged = drte_engine_delete_character_to_right_of_cursor(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextBox->pTL); }

    return wasTextChanged;
}

bool dred_textbox_delete_selected_text_no_undo(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;
    }

    bool wasTextChanged = drte_engine_delete_selected_text(pTextBox->pTL, true);  // <-- "true" means to update the positions of cursors to compensate.
    drte_engine_deselect_all(pTextBox->pTL);

    return wasTextChanged;
}

bool dred_textbox_delete_selected_text(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;
    }

    bool wasTextChanged = false;
    drte_engine_prepare_undo_point(pTextBox->pTL);
    {
        wasTextChanged = dred_textbox_delete_selected_text_no_undo(pTextBox);
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextBox->pTL); }

    return wasTextChanged;
}

bool dred_textbox_insert_text_at_cursors_no_undo(dred_textbox* pTextBox, const char* text)
{
    if (pTextBox == NULL) {
        return false;
    }

    size_t insertedCharacterCount = strlen(text);

    bool wasTextChanged = false;
    for (size_t iCursor = 0; iCursor < pTextBox->cursorCount; ++iCursor) {
        size_t iChar = drte_engine_get_cursor_character(pTextBox->pTL, iCursor);

        wasTextChanged = drte_engine_insert_text_at_cursor(pTextBox->pTL, iCursor, text) || wasTextChanged;
            
        // Cursors and selections after this cursor need to be updated.
        for (size_t iCursor2 = 0; iCursor2 < pTextBox->pTL->cursorCount; ++iCursor2) {
            if (iCursor != iCursor2) {
                if (pTextBox->pTL->pCursors[iCursor2].iCharAbs >= iChar) {
                    drte_engine_move_cursor_to_character(pTextBox->pTL, iCursor2, pTextBox->pTL->pCursors[iCursor2].iCharAbs + insertedCharacterCount);
                }
            }
        }

        // As with cursors, selections need to be updated too.
        for (size_t iSelection = 0; iSelection < pTextBox->pTL->selectionCount; ++iSelection) {
            drte_region selection = drte_region_normalize(pTextBox->pTL->pSelections[iSelection]);
            if (selection.iCharBeg > iChar) {
                pTextBox->pTL->pSelections[iSelection].iCharBeg += insertedCharacterCount;
                pTextBox->pTL->pSelections[iSelection].iCharEnd += insertedCharacterCount;
            } else {
                if (selection.iCharEnd > iChar) {
                    pTextBox->pTL->pSelections[iSelection].iCharEnd += insertedCharacterCount;
                }
            }
        }
    }

    return wasTextChanged;
}

bool dred_textbox_insert_text_at_cursors(dred_textbox* pTextBox, const char* text)
{
    if (pTextBox == NULL) {
        return false;
    }

    bool wasTextChanged = false;
    drte_engine_prepare_undo_point(pTextBox->pTL);
    {
        wasTextChanged = dred_textbox_insert_text_at_cursors_no_undo(pTextBox, text) || wasTextChanged;
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextBox->pTL); }

    return wasTextChanged;
}

bool dred_textbox_unindent_selected_blocks(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;
    }

    drte_engine__begin_dirty(pTextBox->pTL);

    bool wasTextChanged = false;
    drte_engine_prepare_undo_point(pTextBox->pTL);
    {
        for (size_t iSelection = 0; iSelection < pTextBox->pTL->selectionCount; ++iSelection) {
            size_t iLineBeg = drte_engine_get_selection_first_line(pTextBox->pTL, iSelection);
            size_t iLineEnd = drte_engine_get_selection_last_line(pTextBox->pTL, iSelection);
            if (iLineBeg != iLineEnd) {
                for (size_t iLine = iLineBeg; iLine <= iLineEnd; ++iLine) {
                    size_t iLineChar = drte_engine_get_line_first_character(pTextBox->pTL, NULL, iLine);
                    size_t iLineCharNonWS = iLineChar;
                    for (;;) {
                        uint32_t c = drte_engine_get_utf32(pTextBox->pTL, iLineCharNonWS);
                        if (c == '\0' || c == '\n' || !dr_is_whitespace(c)) {
                            break;
                        }

                        iLineCharNonWS += 1;
                    }

                    if (iLineCharNonWS > iLineChar) {
                        size_t charactersRemovedCount = 0;
                        uint32_t c = drte_engine_get_utf32(pTextBox->pTL, iLineChar);
                        if (c == '\t') {
                            charactersRemovedCount = 1;
                        } else {
                            charactersRemovedCount = 0; //(iLineCharNonWS - iLineChar);
                            for (size_t iChar = iLineChar; iChar < iLineCharNonWS; ++iChar) {
                                if (charactersRemovedCount >= drte_engine_get_tab_size(pTextBox->pTL)) {
                                    break;
                                }

                                c = drte_engine_get_utf32(pTextBox->pTL, iChar);
                                if (c == '\t') {
                                    break;
                                }

                                charactersRemovedCount += 1;
                            }
                        }

                        wasTextChanged = drte_engine_delete_text(pTextBox->pTL, iLineChar, iLineChar + charactersRemovedCount) || wasTextChanged;

                        // Cursors and selections need to be updated.
                        for (size_t iCursor2 = 0; iCursor2 < pTextBox->pTL->cursorCount; ++iCursor2) {
                            if (pTextBox->pTL->pCursors[iCursor2].iCharAbs >= iLineChar) {
                                drte_engine_move_cursor_to_character(pTextBox->pTL, iCursor2, pTextBox->pTL->pCursors[iCursor2].iCharAbs - charactersRemovedCount);
                            }
                        }

                        for (size_t iSelection2 = 0; iSelection2 < pTextBox->pTL->selectionCount; ++iSelection2) {
                            drte_region selection = drte_region_normalize(pTextBox->pTL->pSelections[iSelection2]);
                            if (selection.iCharBeg > iLineChar + charactersRemovedCount) {
                                pTextBox->pTL->pSelections[iSelection2].iCharBeg -= charactersRemovedCount;
                                pTextBox->pTL->pSelections[iSelection2].iCharEnd -= charactersRemovedCount;
                            } else {
                                if (selection.iCharEnd > iLineChar) {
                                    pTextBox->pTL->pSelections[iSelection2].iCharEnd -= charactersRemovedCount;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextBox->pTL); }

    drte_engine__end_dirty(pTextBox->pTL);

    return wasTextChanged;
}

bool dred_textbox_undo(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;
    }

    return drte_engine_undo(pTextBox->pTL);
}

bool dred_textbox_redo(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;
    }

    return drte_engine_redo(pTextBox->pTL);
}

unsigned int dred_textbox_get_undo_points_remaining_count(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;
    }

    return drte_engine_get_undo_points_remaining_count(pTextBox->pTL);
}

unsigned int dred_textbox_get_redo_points_remaining_count(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;
    }

    return drte_engine_get_redo_points_remaining_count(pTextBox->pTL);
}

void dred_textbox_clear_undo_stack(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_clear_undo_stack(pTextBox->pTL);
}


size_t dred_textbox_get_cursor_line(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return drte_engine_get_cursor_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
}

size_t dred_textbox_get_cursor_column(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return drte_engine_get_cursor_column(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
}

size_t dred_textbox_get_line_count(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return drte_engine_get_line_count(pTextBox->pTL);
}


bool dred_textbox_find_and_select_next(dred_textbox* pTextBox, const char* text)
{
    if (pTextBox == NULL) {
        return 0;
    }

    size_t selectionStart;
    size_t selectionEnd;
    if (drte_engine_find_next(pTextBox->pTL, text, &selectionStart, &selectionEnd))
    {
        drte_engine_select(pTextBox->pTL, selectionStart, selectionEnd);
        drte_engine_move_cursor_to_end_of_selection(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));

        return true;
    }

    return false;
}

bool dred_textbox_find_and_replace_next(dred_textbox* pTextBox, const char* text, const char* replacement)
{
    if (pTextBox == NULL) {
        return 0;
    }

    bool wasTextChanged = false;
    drte_engine_prepare_undo_point(pTextBox->pTL);
    {
        size_t selectionStart;
        size_t selectionEnd;
        if (drte_engine_find_next(pTextBox->pTL, text, &selectionStart, &selectionEnd))
        {
            drte_engine_select(pTextBox->pTL, selectionStart, selectionEnd);
            drte_engine_move_cursor_to_end_of_selection(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));

            wasTextChanged = dred_textbox_delete_selected_text_no_undo(pTextBox) || wasTextChanged;
            wasTextChanged = drte_engine_insert_text_at_cursor(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), replacement) || wasTextChanged;
        }
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextBox->pTL); }

    return wasTextChanged;
}

bool dred_textbox_find_and_replace_all(dred_textbox* pTextBox, const char* text, const char* replacement)
{
    if (pTextBox == NULL) {
        return 0;
    }

    size_t originalCursorLine = drte_engine_get_cursor_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
    size_t originalCursorPos = drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)) - drte_engine_get_line_first_character(pTextBox->pTL, NULL, originalCursorLine);
    int originalScrollPosX = dred_scrollbar_get_scroll_position(pTextBox->pHorzScrollbar);
    int originalScrollPosY = dred_scrollbar_get_scroll_position(pTextBox->pVertScrollbar);

    bool wasTextChanged = false;
    drte_engine_prepare_undo_point(pTextBox->pTL);
    {
        // It's important that we don't replace the replacement text. To handle this, we just move the cursor to the top of the text and find
        // and replace every occurance without looping.
        drte_engine_move_cursor_to_start_of_text(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));

        size_t selectionStart;
        size_t selectionEnd;
        while (drte_engine_find_next_no_loop(pTextBox->pTL, text, &selectionStart, &selectionEnd))
        {
            drte_engine_select(pTextBox->pTL, selectionStart, selectionEnd);
            drte_engine_move_cursor_to_end_of_selection(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));

            wasTextChanged = dred_textbox_delete_selected_text_no_undo(pTextBox) || wasTextChanged;
            wasTextChanged = drte_engine_insert_text_at_cursor(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), replacement) || wasTextChanged;
        }

        // The cursor may have moved so we'll need to restore it.
        size_t lineCharStart;
        size_t lineCharEnd;
        drte_engine_get_line_character_range(pTextBox->pTL, NULL, originalCursorLine, &lineCharStart, &lineCharEnd);

        size_t newCursorPos = lineCharStart + originalCursorPos;
        if (newCursorPos > lineCharEnd) {
            newCursorPos = lineCharEnd;
        }
        drte_engine_move_cursor_to_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), newCursorPos);
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextBox->pTL); }


    // The scroll positions may have moved so we'll need to restore them.
    dred_scrollbar_scroll_to(pTextBox->pHorzScrollbar, originalScrollPosX);
    dred_scrollbar_scroll_to(pTextBox->pVertScrollbar, originalScrollPosY);

    return wasTextChanged;
}


void dred_textbox_show_line_numbers(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    dred_control_show(pTextBox->pLineNumbers);
    dred_textbox__refresh_line_numbers(pTextBox);
}

void dred_textbox_hide_line_numbers(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    dred_control_hide(pTextBox->pLineNumbers);
    dred_textbox__refresh_line_numbers(pTextBox);
}


void dred_textbox_disable_vertical_scrollbar(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    if (pTextBox->isVertScrollbarEnabled) {
        pTextBox->isVertScrollbarEnabled = false;
        dred_textbox__refresh_scrollbars(pTextBox);
    }
}

void dred_textbox_enable_vertical_scrollbar(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    if (!pTextBox->isVertScrollbarEnabled) {
        pTextBox->isVertScrollbarEnabled = true;
        dred_textbox__refresh_scrollbars(pTextBox);
    }
}

void dred_textbox_disable_horizontal_scrollbar(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    if (pTextBox->isHorzScrollbarEnabled) {
        pTextBox->isHorzScrollbarEnabled = false;
        dred_textbox__refresh_scrollbars(pTextBox);
    }
}

void dred_textbox_enable_horizontal_scrollbar(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    if (!pTextBox->isHorzScrollbarEnabled) {
        pTextBox->isHorzScrollbarEnabled = true;
        dred_textbox__refresh_scrollbars(pTextBox);
    }
}

dred_scrollbar* dred_textbox_get_vertical_scrollbar(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return NULL;
    }

    return pTextBox->pVertScrollbar;
}

dred_scrollbar* dred_textbox_get_horizontal_scrollbar(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return NULL;
    }

    return pTextBox->pHorzScrollbar;
}

void dred_textbox_set_scrollbar_size(dred_textbox* pTextBox, float size)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->horzScrollbarSize = size;
    pTextBox->vertScrollbarSize = size;

    dred_textbox__refresh_scrollbars(pTextBox);
}

void dred_textbox_enable_excess_scrolling(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->isExcessScrollingEnabled = true;

    dred_textbox__refresh_scrollbars(pTextBox);
}

void dred_textbox_disable_excess_scrolling(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->isExcessScrollingEnabled = false;

    dred_textbox__refresh_scrollbars(pTextBox);
}


void dred_textbox_set_tab_size_in_spaces(dred_textbox* pTextBox, unsigned int tabSizeInSpaces)
{
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_set_tab_size(pTextBox->pTL, tabSizeInSpaces);
}

unsigned int dred_textbox_get_tab_size_in_spaces(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return drte_engine_get_tab_size(pTextBox->pTL);
}


void dred_textbox_enable_tabs_to_spaces(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->isTabsToSpacesEnabled = true;
}

void dred_textbox_disable_tabs_to_spaces(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->isTabsToSpacesEnabled = false;
}

bool dred_textbox_is_tabs_to_spaces_enabled(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;;
    }

    return pTextBox->isTabsToSpacesEnabled;
}


bool dred_textbox_prepare_undo_point(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;;
    }

    return drte_engine_prepare_undo_point(pTextBox->pTL);
}

bool dred_textbox_commit_undo_point(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return false;;
    }

    return drte_engine_commit_undo_point(pTextBox->pTL);
}


void dred_textbox_set_on_cursor_move(dred_textbox* pTextBox, dred_textbox_on_cursor_move_proc proc)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->onCursorMove = proc;
}

void dred_textbox_set_on_undo_point_changed(dred_textbox* pTextBox, dred_textbox_on_undo_point_changed_proc proc)
{
    if (pTextBox == NULL) {
        return;
    }

    pTextBox->onUndoPointChanged = proc;
}


void dred_textbox_on_size(dred_control* pControl, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    if (pTextBox == NULL) {
        return;
    }

    // The text engine needs to be resized.
    float containerWidth;
    float containerHeight;
    dred_textbox__calculate_text_engine_container_size(pTextBox, &containerWidth, &containerHeight);

    if (containerWidth != newWidth || containerHeight != newHeight) {
        drte_engine_set_container_size(pTextBox->pTL, containerWidth, containerHeight);

        // Scrollbars need to be refreshed first.
        dred_textbox__refresh_scrollbars(pTextBox);

        // Line numbers need to be refreshed.
        dred_textbox__refresh_line_numbers(pTextBox);
    }
}

void dred_textbox_on_mouse_move(dred_control* pControl, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    if (pTextBox == NULL) {
        return;
    }

    if (dred_gui_get_element_with_mouse_capture(dred_control_get_gui(pControl)) == pControl)
    {
        float offsetX;
        float offsetY;
        dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);

        drte_engine__begin_dirty(pTextBox->pTL);
        {
            size_t iPrevCursorChar = drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));

            if (pTextBox->isDoingWordSelect) {
                size_t iWordCharBeg;
                size_t iWordCharEnd;
                if (drte_engine_get_word_under_point(pTextBox->pTL, relativeMousePosX - offsetX, relativeMousePosY - offsetY, &iWordCharBeg, &iWordCharEnd)) {
                    if (iWordCharEnd < pTextBox->wordSelectionAnchor.iCharEnd) {
                        pTextBox->pTL->pSelections[pTextBox->pTL->selectionCount-1].iCharBeg = pTextBox->wordSelectionAnchor.iCharEnd;
                        pTextBox->pTL->pSelections[pTextBox->pTL->selectionCount-1].iCharEnd = iWordCharBeg;
                    } else {
                        pTextBox->pTL->pSelections[pTextBox->pTL->selectionCount-1].iCharBeg = pTextBox->wordSelectionAnchor.iCharBeg;
                        pTextBox->pTL->pSelections[pTextBox->pTL->selectionCount-1].iCharEnd = iWordCharEnd;
                    }

                    drte_engine_move_cursor_to_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), pTextBox->pTL->pSelections[pTextBox->pTL->selectionCount-1].iCharEnd);
                } else {
                    // There is no word under the point, so just fall back to standard character selection for this case.
                    drte_engine_move_cursor_to_point(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), (float)relativeMousePosX - offsetX, (float)relativeMousePosY - offsetY);
                }
            } else {
                drte_engine_move_cursor_to_point(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), (float)relativeMousePosX - offsetX, (float)relativeMousePosY - offsetY);
            }

            size_t iNextCursorChar = drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
            if (iPrevCursorChar != iNextCursorChar) {
                drte_engine_set_selection_end_point(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
            }
        }
        drte_engine__end_dirty(pTextBox->pTL);
    }
}

void dred_textbox_on_mouse_button_down(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    if (pTextBox == NULL) {
        return;
    }

    // Focus the text editor.
    dred_gui_capture_keyboard(pControl);

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT)
    {
        pTextBox->isDoingWordSelect = false;

        if ((stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) != 0) {
            if (!drte_engine_is_anything_selected(pTextBox->pTL)) {
                drte_engine_begin_selection(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
            }
        } else {
            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) == 0) {
                drte_engine_deselect_all(pTextBox->pTL);
                dred_textbox__clear_all_cursors(pTextBox);
            }
        }


        float offsetX;
        float offsetY;
        dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);

        size_t iLine;
        size_t iChar = drte_engine_get_character_by_point_relative_to_container(pTextBox->pTL, NULL, (float)relativeMousePosX - offsetX, (float)relativeMousePosY - offsetY, &iLine);

        if ((stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) != 0) {
            drte_engine_set_selection_end_point(pTextBox->pTL, iChar);
        } else {
            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) != 0) {
                dred_textbox__insert_cursor(pTextBox, iChar);
            }

            drte_engine_begin_selection(pTextBox->pTL, iChar);
        }
        

        drte_engine_move_cursor_to_character_and_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), iChar, iLine);
        drte_engine__update_cursor_sticky_position(pTextBox->pTL, &pTextBox->pTL->pCursors[drte_engine_get_last_cursor(pTextBox->pTL)]);


        // In order to support selection with the mouse we need to capture the mouse and enter selection mode.
        dred_gui_capture_mouse(pControl);
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_RIGHT)
    {
    }
}

void dred_textbox_on_mouse_button_up(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    if (pTextBox == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT)
    {
        if (dred_gui_get_element_with_mouse_capture(pControl->pGUI) == pControl)
        {
            // When we first pressed the mouse we may have started a new selection. If we never ended up selecting anything we'll want to
            // cancel that selection.
            size_t iCharBeg;
            size_t iCharEnd;
            if (drte_engine_get_last_selection(pTextBox->pTL, &iCharBeg, &iCharEnd)) {
                if (iCharBeg == iCharEnd) {
                    drte_engine_cancel_last_selection(pTextBox->pTL);
                }
            }

            // Releasing the mouse will leave selectionmode.
            dred_gui_release_mouse(pControl->pGUI);
        }
    }
}

void dred_textbox_on_mouse_button_dblclick(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)mouseButton;
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    if (pTextBox == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if ((stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) == 0) {
            // If the control key is not being held down make sure other selection regions are cleared.
            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) == 0) {
                drte_engine_deselect_all(pTextBox->pTL);
            }
            
            drte_engine_select_word_under_cursor(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));

            if (pTextBox->pTL->selectionCount > 0) {
                
            }

            size_t iCharBeg;
            size_t iCharEnd;
            if (drte_engine_get_last_selection(pTextBox->pTL, &iCharBeg, &iCharEnd)) {
                drte_engine_move_cursor_to_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), iCharEnd);

                pTextBox->isDoingWordSelect = true;
                pTextBox->wordSelectionAnchor.iCharBeg = iCharBeg;
                pTextBox->wordSelectionAnchor.iCharEnd = iCharEnd;
            }
        }
    }
}

void dred_textbox_on_mouse_wheel(dred_control* pControl, int delta, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    if (pTextBox == NULL) {
        return;
    }

    dred_scrollbar_scroll(pTextBox->pVertScrollbar, -delta * dred_scrollbar_get_mouse_wheel_scale(pTextBox->pVertScrollbar));
}

void dred_textbox_on_key_down(dred_control* pControl, dred_key key, int stateFlags)
{
    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    if (pTextBox == NULL) {
        return;
    }

    drte_engine__begin_dirty(pTextBox->pTL);

    bool isShiftDown = (stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) != 0;
    bool isCtrlDown  = (stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) != 0;

    switch (key)
    {
        case DRED_GUI_BACKSPACE:
        {
            bool wasTextChanged = false;
            drte_engine_prepare_undo_point(pTextBox->pTL);
            {
                if (drte_engine_is_anything_selected(pTextBox->pTL)) {
                    wasTextChanged = dred_textbox_delete_selected_text_no_undo(pTextBox);
                    drte_engine_remove_overlapping_cursors(pTextBox->pTL);
                } else {
                    wasTextChanged = drte_engine_delete_character_to_left_of_cursors(pTextBox->pTL, pTextBox->pTL->cursorCount > 1);  // <-- Last argument controls whether or not new lines should block deletion.
                }
            }
            if (wasTextChanged) { drte_engine_commit_undo_point(pTextBox->pTL); }
        } break;

        case DRED_GUI_DELETE:
        {
            bool wasTextChanged = false;
            drte_engine_prepare_undo_point(pTextBox->pTL);
            {
                if (drte_engine_is_anything_selected(pTextBox->pTL)) {
                    wasTextChanged = dred_textbox_delete_selected_text_no_undo(pTextBox);
                    drte_engine_remove_overlapping_cursors(pTextBox->pTL);
                } else {
                    wasTextChanged = drte_engine_delete_character_to_right_of_cursors(pTextBox->pTL, pTextBox->pTL->cursorCount > 1); // <-- Last argument controls whether or not new lines should block deletion.
                }
            }
            if (wasTextChanged) { drte_engine_commit_undo_point(pTextBox->pTL); }
        } break;


        case DRED_GUI_ARROW_LEFT:
        {
            if (drte_engine_is_anything_selected(pTextBox->pTL) && !isShiftDown) {
                dred_textbox__move_cursor_to_start_of_selection(pTextBox, NULL);
                dred_textbox_deselect_all(pTextBox);
            } else {
                if ((!drte_engine_is_anything_selected(pTextBox->pTL) || !dred_textbox__is_cursor_on_selection(pTextBox)) && isShiftDown) {
                    drte_engine_begin_selection(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
                }

                if (isCtrlDown) {
                    drte_engine_move_cursor_to_start_of_word(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
                } else {
                    drte_engine_move_cursor_left(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
                }

                if (isShiftDown) {
                    drte_engine_set_selection_end_point(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));

                    size_t iSelCharBeg;
                    size_t iSelCharEnd;
                    if (drte_engine_get_last_selection(pTextBox->pTL, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                        drte_engine_cancel_last_selection(pTextBox->pTL);
                    }
                }
            }
        } break;

        case DRED_GUI_ARROW_RIGHT:
        {
            if (drte_engine_is_anything_selected(pTextBox->pTL) && !isShiftDown) {
                dred_textbox__move_cursor_to_end_of_selection(pTextBox, NULL);
                dred_textbox_deselect_all(pTextBox);
            } else {
                if ((!drte_engine_is_anything_selected(pTextBox->pTL) || !dred_textbox__is_cursor_on_selection(pTextBox)) && isShiftDown) {
                    drte_engine_begin_selection(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
                }

                if (isCtrlDown) {
                    drte_engine_move_cursor_to_start_of_next_word(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
                } else {
                    drte_engine_move_cursor_right(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
                }

                if (isShiftDown) {
                    drte_engine_set_selection_end_point(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));

                    size_t iSelCharBeg;
                    size_t iSelCharEnd;
                    if (drte_engine_get_last_selection(pTextBox->pTL, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                        drte_engine_cancel_last_selection(pTextBox->pTL);
                    }
                }
            }
        } break;

        case DRED_GUI_ARROW_UP:
        {
            if (drte_engine_is_anything_selected(pTextBox->pTL) && !isShiftDown) {
                dred_textbox_deselect_all(pTextBox);
            } else {
                if ((!drte_engine_is_anything_selected(pTextBox->pTL) || !dred_textbox__is_cursor_on_selection(pTextBox)) && isShiftDown) {
                    drte_engine_begin_selection(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
                }
            }

            drte_engine_move_cursor_up(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));

            if (isShiftDown) {
                drte_engine_set_selection_end_point(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_engine_get_last_selection(pTextBox->pTL, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_engine_cancel_last_selection(pTextBox->pTL);
                }
            }
        } break;

        case DRED_GUI_ARROW_DOWN:
        {
            if (drte_engine_is_anything_selected(pTextBox->pTL) && !isShiftDown) {
                dred_textbox_deselect_all(pTextBox);
            } else {
                if ((!drte_engine_is_anything_selected(pTextBox->pTL) || !dred_textbox__is_cursor_on_selection(pTextBox)) && isShiftDown) {
                    drte_engine_begin_selection(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
                }
            }

            drte_engine_move_cursor_down(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));

            if (isShiftDown) {
                drte_engine_set_selection_end_point(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_engine_get_last_selection(pTextBox->pTL, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_engine_cancel_last_selection(pTextBox->pTL);
                }
            }
        } break;


        case DRED_GUI_END:
        {
            if (drte_engine_is_anything_selected(pTextBox->pTL) && !isShiftDown) {
                dred_textbox_deselect_all(pTextBox);
            } else {
                if ((!drte_engine_is_anything_selected(pTextBox->pTL) || !dred_textbox__is_cursor_on_selection(pTextBox)) && isShiftDown) {
                    drte_engine_begin_selection(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
                }
            }

            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) != 0) {
                drte_engine_move_cursor_to_end_of_text(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
            } else {
                if (drte_engine_is_cursor_at_end_of_wrapped_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL))) {
                    drte_engine_move_cursor_to_end_of_unwrapped_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
                } else {
                    drte_engine_move_cursor_to_end_of_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
                }
            }

            if (isShiftDown) {
                drte_engine_set_selection_end_point(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_engine_get_last_selection(pTextBox->pTL, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_engine_cancel_last_selection(pTextBox->pTL);
                }
            }

            drte_engine__update_cursor_sticky_position(pTextBox->pTL, &pTextBox->pTL->pCursors[drte_engine_get_last_cursor(pTextBox->pTL)]);
        } break;

        case DRED_GUI_HOME:
        {
            if (drte_engine_is_anything_selected(pTextBox->pTL) && !isShiftDown) {
                dred_textbox_deselect_all(pTextBox);
            } else {
                if ((!drte_engine_is_anything_selected(pTextBox->pTL) || !dred_textbox__is_cursor_on_selection(pTextBox)) && isShiftDown) {
                    drte_engine_begin_selection(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
                }
            }

            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) != 0) {
                drte_engine_move_cursor_to_start_of_text(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
            } else {
                if (drte_engine_is_cursor_at_start_of_wrapped_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL))) {
                    drte_engine_move_cursor_to_start_of_unwrapped_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
                } else {
                    drte_engine_move_cursor_to_start_of_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
                }
            }

            if (isShiftDown) {
                drte_engine_set_selection_end_point(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_engine_get_last_selection(pTextBox->pTL, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_engine_cancel_last_selection(pTextBox->pTL);
                }
            }

            drte_engine__update_cursor_sticky_position(pTextBox->pTL, &pTextBox->pTL->pCursors[drte_engine_get_last_cursor(pTextBox->pTL)]);
        } break;

        case DRED_GUI_PAGE_UP:
        {
            if (drte_engine_is_anything_selected(pTextBox->pTL) && !isShiftDown) {
                dred_textbox_deselect_all(pTextBox);
            } else {
                if ((!drte_engine_is_anything_selected(pTextBox->pTL) || !dred_textbox__is_cursor_on_selection(pTextBox)) && isShiftDown) {
                    drte_engine_begin_selection(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
                }
            }

            int scrollOffset = dred_scrollbar_get_page_size(pTextBox->pVertScrollbar);
            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) == 0) {
                dred_scrollbar_scroll(pTextBox->pVertScrollbar, -scrollOffset);
            }

            drte_engine_move_cursor_y(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), -dred_scrollbar_get_page_size(pTextBox->pVertScrollbar));

            if (isShiftDown) {
                drte_engine_set_selection_end_point(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_engine_get_last_selection(pTextBox->pTL, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_engine_cancel_last_selection(pTextBox->pTL);
                }
            }
        } break;

        case DRED_GUI_PAGE_DOWN:
        {
            if (drte_engine_is_anything_selected(pTextBox->pTL) && !isShiftDown) {
                dred_textbox_deselect_all(pTextBox);
            } else {
                if ((!drte_engine_is_anything_selected(pTextBox->pTL) || !dred_textbox__is_cursor_on_selection(pTextBox)) && isShiftDown) {
                    drte_engine_begin_selection(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
                }
            }

            int scrollOffset = dred_scrollbar_get_page_size(pTextBox->pVertScrollbar);
            if (scrollOffset > (int)(drte_engine_get_line_count(pTextBox->pTL) - drte_engine_get_cursor_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)))) {
                scrollOffset = 0;
            }

            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) == 0) {
                dred_scrollbar_scroll(pTextBox->pVertScrollbar, scrollOffset);
            }

            drte_engine_move_cursor_y(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), dred_scrollbar_get_page_size(pTextBox->pVertScrollbar));

            if (isShiftDown) {
                drte_engine_set_selection_end_point(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_engine_get_last_selection(pTextBox->pTL, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_engine_cancel_last_selection(pTextBox->pTL);
                }
            }

        } break;

        default: break;
    }

    drte_engine__end_dirty(pTextBox->pTL);
}

void dred_textbox_on_key_up(dred_control* pControl, dred_key key, int stateFlags)
{
    (void)pControl;
    (void)key;
    (void)stateFlags;
}

void dred_textbox_on_printable_key_down(dred_control* pControl, unsigned int utf32, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_prepare_undo_point(pTextBox->pTL);
    {
        drte_engine__begin_dirty(pTextBox->pTL);
        if (utf32 == '\t') {
            // The tab key is a complex case because it can be handled differently depending on the configuration:
            //   - If multiple lines are selected, they need to be block-indented
            //     - Otherwise they need to be inserted like any other character, unless...
            //   - If tabs-to-spaces is enabled, tabs need to be converted to spaces.
            //
            // The tab key is handled for each cursor.
            for (size_t iCursor = 0; iCursor < pTextBox->cursorCount; ++iCursor) {
                bool isDoingBlockIndent = false;

                size_t iSelection;
                bool isSomethingSelected = dred_textbox__get_cursor_selection(pTextBox, iCursor, &iSelection);
                if (isSomethingSelected) {
                    isDoingBlockIndent = drte_engine_get_selection_first_line(pTextBox->pTL, iSelection) != drte_engine_get_selection_last_line(pTextBox->pTL, iSelection);
                }

                if (isDoingBlockIndent) {
                    // A block indent is done by simply inserting a tab at the beginning of each selected line.
                    size_t iLineBeg = drte_engine_get_selection_first_line(pTextBox->pTL, iSelection);
                    size_t iLineEnd = drte_engine_get_selection_last_line(pTextBox->pTL, iSelection);

                    for (size_t iLine = iLineBeg; iLine <= iLineEnd; ++iLine) {
                        dred_textbox__insert_tab(pTextBox, drte_engine_get_line_first_character(pTextBox->pTL, NULL, iLine));
                    }
                } else {
                    // We're not doing a block indent so we just insert a tab at the cursor like normal.
                    if (isSomethingSelected) {
                        drte_engine_delete_selection_text(pTextBox->pTL, iSelection, true);
                        drte_engine_cancel_selection(pTextBox->pTL, iSelection);
                    }

                    dred_textbox__insert_tab_at_cursor(pTextBox, iCursor);
                }
            }
        } else {
            if (drte_engine_is_anything_selected(pTextBox->pTL)) {
                dred_textbox_delete_selected_text_no_undo(pTextBox);
            }

            // Convert to line endings.
            if (utf32 == '\r') {
                utf32 = '\n';
            }

            // TODO: Check if the character is a line feed, and if so convert it to the standard line endings.

            drte_engine_insert_character_at_cursors(pTextBox->pTL, utf32);

            if (utf32 == '\n') {
                dred_context* pDred = dred_control_get_context(DRED_CONTROL(pTextBox));
                if (pDred->config.textEditorEnableAutoIndent) {
                    for (size_t iCursor = 0; iCursor < pTextBox->pTL->cursorCount; ++iCursor) {
                        size_t iCursorChar = pTextBox->pTL->pCursors[iCursor].iCharAbs;
                        size_t iCursorLine = drte_engine_get_character_line(pTextBox->pTL, NULL, iCursorChar);
                        if (iCursorLine > 0) {
                            size_t iPrevLineCharBeg;
                            size_t iPrevLineCharEnd;
                            drte_engine_get_line_character_range(pTextBox->pTL, NULL, iCursorLine-1, &iPrevLineCharBeg, &iPrevLineCharEnd);

                            size_t indentationCount = 0;
                            for (;;) {
                                if (iPrevLineCharBeg == iPrevLineCharEnd) {
                                    break;  // End of line.
                                }

                                // TODO: Use a character iterator for this.
                                uint32_t c = drte_engine_get_utf32(pTextBox->pTL, iPrevLineCharBeg);
                                if (c == '\0') {
                                    break;
                                }

                                if (!dr_is_whitespace(c)) {
                                    break;
                                }


                                iPrevLineCharBeg += 1;

                                if (c == '\t') {
                                    indentationCount += pDred->config.textEditorTabSizeInSpaces;
                                } else {
                                    indentationCount += 1;
                                }
                            }

                            size_t extraTabCount    = indentationCount / pDred->config.textEditorTabSizeInSpaces;
                            size_t extraSpacesCount = indentationCount - (extraTabCount * pDred->config.textEditorTabSizeInSpaces);
                            if (pTextBox->isTabsToSpacesEnabled) {
                                extraSpacesCount = extraTabCount * pDred->config.textEditorTabSizeInSpaces;
                                extraTabCount = 0;
                            }

                            size_t extraCharactersCount = extraTabCount + extraSpacesCount;

                            for (size_t i = 0; i < extraTabCount; ++i) {
                                drte_engine_insert_character_at_cursor(pTextBox->pTL, iCursor, '\t');
                            }
                            for (size_t i = 0; i < extraSpacesCount; ++i) {
                                drte_engine_insert_character_at_cursor(pTextBox->pTL, iCursor, ' ');
                            }

                            // Any cursor whose character position comes after this cursor needs to be moved.
                            for (size_t iCursor2 = 0; iCursor2 < pTextBox->pTL->cursorCount; ++iCursor2) {
                                if (iCursor2 != iCursor) {
                                    if (pTextBox->pTL->pCursors[iCursor2].iCharAbs > iCursorChar) {
                                        drte_engine_move_cursor_to_character(pTextBox->pTL, iCursor2, pTextBox->pTL->pCursors[iCursor2].iCharAbs + extraCharactersCount);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        drte_engine__end_dirty(pTextBox->pTL);
    }
    drte_engine_commit_undo_point(pTextBox->pTL);
}


void dred_textbox_engine__on_paint_rect(drte_engine* pTL, drte_style_token styleToken, drte_rect rect, void* pPaintData)
{
    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;
    dred_text_style* pStyle = (dred_text_style*)styleToken;

    float offsetX;
    float offsetY;
    dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);

    dred_control_draw_rect(DRED_CONTROL(pTextBox), dred_offset_rect(drte_rect_to_dred(rect), offsetX, offsetY), pStyle->bgColor, pPaintData);
}

void dred_textbox_engine__on_paint_text(drte_engine* pTL, drte_style_token styleTokenFG, drte_style_token styleTokenBG, const char* text, size_t textLength, float posX, float posY, void* pPaintData)
{
    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;

    dred_text_style* pStyleFG = (dred_text_style*)styleTokenFG;
    dred_text_style* pStyleBG = (dred_text_style*)styleTokenBG;

    float offsetX;
    float offsetY;
    dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);

    dred_control_draw_text(DRED_CONTROL(pTextBox), pStyleFG->pFont, text, (int)textLength, posX + offsetX, posY + offsetY, pStyleFG->fgColor, pStyleBG->bgColor, pPaintData);
}

void dred_textbox_engine__on_dirty(drte_engine* pTL, drte_rect rect)
{
    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;
    if (pTextBox == NULL) {
        return;
    }

    if (pTextBox == NULL) {
        return;
    }

    float offsetX;
    float offsetY;
    dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);

    dred_control_dirty(DRED_CONTROL(pTextBox), dred_offset_rect(drte_rect_to_dred(rect), offsetX, offsetY));
}

void dred_textbox_engine__on_cursor_move(drte_engine* pTL)
{
    // If the cursor is off the edge of the container we want to scroll it into position.
    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;
    if (pTextBox == NULL) {
        return;
    }

    if (pTextBox == NULL) {
        return;
    }

    // If the cursor is above or below the container, we need to scroll vertically.
    int iLine = (int)drte_engine_get_cursor_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
    if (iLine < dred_scrollbar_get_scroll_position(pTextBox->pVertScrollbar)) {
        dred_scrollbar_scroll_to(pTextBox->pVertScrollbar, iLine);
    }

    int iBottomLine = dred_scrollbar_get_scroll_position(pTextBox->pVertScrollbar) + dred_scrollbar_get_page_size(pTextBox->pVertScrollbar) - 1;
    if (iLine >= iBottomLine) {
        dred_scrollbar_scroll_to(pTextBox->pVertScrollbar, iLine - (dred_scrollbar_get_page_size(pTextBox->pVertScrollbar) - 1) + 1);
    }


    // If the cursor is to the left or right of the container we need to scroll horizontally.
    float cursorPosX;
    float cursorPosY;
    drte_engine_get_cursor_position(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), &cursorPosX, &cursorPosY);

    if (cursorPosX < 0) {
        dred_scrollbar_scroll_to(pTextBox->pHorzScrollbar, (int)(cursorPosX - drte_engine_get_inner_offset_x(pTextBox->pTL)) - 100);
    }
    if (cursorPosX >= drte_engine_get_container_width(pTextBox->pTL)) {
        dred_scrollbar_scroll_to(pTextBox->pHorzScrollbar, (int)(cursorPosX - drte_engine_get_inner_offset_x(pTextBox->pTL) - drte_engine_get_container_width(pTextBox->pTL)) + 100);
    }


    if (pTextBox->onCursorMove) {
        pTextBox->onCursorMove(pTextBox);
    }
}

void dred_textbox_engine__on_text_changed(drte_engine* pTL)
{
    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;
    if (pTextBox == NULL) {
        return;
    }

    if (pTextBox == NULL) {
        return;
    }

    // Scrollbars need to be refreshed whenever text is changed.
    dred_textbox__refresh_scrollbars(pTextBox);

    // The line numbers need to be redrawn.
    // TODO: This can probably be optimized a bit so that it is only redrawn if a line was inserted or deleted.
    dred_control_dirty(pTextBox->pLineNumbers, dred_control_get_local_rect(pTextBox->pLineNumbers));
}

void dred_textbox_engine__on_undo_point_changed(drte_engine* pTL, unsigned int iUndoPoint)
{
    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;
    if (pTextBox == NULL) {
        return;
    }

    if (pTextBox == NULL) {
        return;
    }

    if (pTextBox->onUndoPointChanged) {
        pTextBox->onUndoPointChanged(pTextBox, iUndoPoint);
    }
}

size_t dred_textbox_engine__on_get_undo_state(drte_engine* pTL, void* pDataOut)
{
    //printf("Getting Undo State...\n");

    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;
    if (pTextBox == NULL) {
        return 0;
    }

    if (pTextBox == NULL) {
        return 0;
    }

    if (pDataOut != NULL) {
        *((size_t*)pDataOut) = pTextBox->cursorCount;

        dred_textbox_cursor* pCursorOut = (dred_textbox_cursor*)((uint8_t*)pDataOut + sizeof(pTextBox->cursorCount));
        for (size_t i = 0; i < pTextBox->cursorCount; ++i) {
            pCursorOut[i] = pTextBox->pCursors[i];
        }
    }

    return sizeof(pTextBox->cursorCount) + (sizeof(*pTextBox->pCursors) * pTextBox->cursorCount);
}

void dred_textbox_engine__on_apply_undo_state(drte_engine* pTL, size_t dataSize, const void* pData)
{
    //printf("Applying Undo State...\n");

    assert(pData != NULL);
    (void)dataSize;

    dred_textbox* pTextBox = (dred_textbox*)pTL->pUserData;
    if (pTextBox == NULL) {
        return;
    }

    if (pTextBox == NULL) {
        return;
    }

    
    size_t cursorCount = *((const size_t*)pData);
    const dred_textbox_cursor* pCursors = (const dred_textbox_cursor*)((const uint8_t*)pData + sizeof(pTextBox->cursorCount));

    if (cursorCount > pTextBox->cursorCount) {
        dred_textbox_cursor* pNewCursors = (dred_textbox_cursor*)realloc(pTextBox->pCursors, cursorCount * sizeof(*pTextBox->pCursors));
        if (pNewCursors == NULL) {
            return;
        }

        pTextBox->pCursors = pNewCursors;
    }

    memcpy(pTextBox->pCursors, pCursors, cursorCount * sizeof(*pTextBox->pCursors));
    pTextBox->cursorCount = cursorCount;
}




void dred_textbox_on_paint(dred_control* pControl, dred_rect relativeRect, void* pPaintData)
{
    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    if (pTextBox == NULL) {
        return;
    }

    dred_rect textRect = dred_textbox__get_text_rect(pTextBox);

    // The dead space between the scrollbars should always be drawn with the default background color.
    //dred_control_draw_rect(pTextBox, dred_textbox__get_scrollbar_dead_space_rect(pTextBox), pTextBox->defaultStyle.bgColor, pPaintData);

    // Border.
    dred_rect borderRect = dred_control_get_local_rect(pControl);
    dred_control_draw_rect_outline(pControl, borderRect, pTextBox->borderColor, pTextBox->borderWidth, pPaintData);

    // Padding.
    dred_rect paddingRect = dred_grow_rect(textRect, pTextBox->padding);
    dred_control_draw_rect_outline(pControl, paddingRect, pTextBox->defaultStyle.bgColor, pTextBox->padding, pPaintData);

    // Text.
    dred_control_set_clip(pControl, dred_clamp_rect(textRect, relativeRect), pPaintData);
    drte_engine_paint(pTextBox->pTL, dred_rect_to_drte(dred_offset_rect(dred_clamp_rect(textRect, relativeRect), -textRect.left, -textRect.top)), pPaintData);
}

void dred_textbox__on_timer(dred_timer* pTimer, void* pUserData)
{
    (void)pTimer;

    dred_textbox* pTextBox = (dred_textbox*)pUserData;
    assert(pTextBox != NULL);

    dred_textbox_step(pTextBox, 100);
}

void dred_textbox_on_capture_keyboard(dred_control* pControl, dred_control* pPrevCapturedControl)
{
    (void)pPrevCapturedControl;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_show_cursor(pTextBox->pTL);

    if (pTextBox->pTimer == NULL) {
        pTextBox->pTimer = dred_timer_create(100, dred_textbox__on_timer, pTextBox);
    }
}

void dred_textbox_on_release_keyboard(dred_control* pControl, dred_control* pNewCapturedControl)
{
    (void)pNewCapturedControl;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    if (pTextBox == NULL) {
        return;
    }

    drte_engine_hide_cursor(pTextBox->pTL);

    if (pTextBox->pTimer != NULL) {
        dred_timer_delete(pTextBox->pTimer);
        pTextBox->pTimer = NULL;
    }
}

void dred_textbox_on_capture_mouse(dred_control* pControl)
{
    if (pControl == NULL) {
        return;
    }

}

void dred_textbox_on_release_mouse(dred_control* pControl)
{
    if (pControl == NULL) {
        return;
    }


}



void dred_textbox__get_text_offset(dred_textbox* pTextBox, float* pOffsetXOut, float* pOffsetYOut)
{
    float offsetX = 0;
    float offsetY = 0;

    if (pTextBox != NULL)
    {
        float lineNumbersWidth = 0;
        if (dred_control_is_visible(pTextBox->pLineNumbers)) {
            lineNumbersWidth = dred_control_get_width(pTextBox->pLineNumbers);
        }

        offsetX = pTextBox->borderWidth + pTextBox->padding + lineNumbersWidth;
        offsetY = pTextBox->borderWidth + pTextBox->padding;
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

    if (pTextBox != NULL)
    {
        float horzScrollbarSize = 0;
        if (dred_control_is_visible(DRED_CONTROL(pTextBox->pHorzScrollbar))) {
            horzScrollbarSize = dred_control_get_height(DRED_CONTROL(pTextBox->pHorzScrollbar));
        }

        float vertScrollbarSize = 0;
        if (dred_control_is_visible(DRED_CONTROL(pTextBox->pVertScrollbar))) {
            vertScrollbarSize = dred_control_get_width(DRED_CONTROL(pTextBox->pVertScrollbar));
        }

        float lineNumbersWidth = 0;
        if (dred_control_is_visible(pTextBox->pLineNumbers)) {
            lineNumbersWidth = dred_control_get_width(pTextBox->pLineNumbers);
        }

        width  = dred_control_get_width(DRED_CONTROL(pTextBox))  - (pTextBox->borderWidth + pTextBox->padding)*2 - vertScrollbarSize - lineNumbersWidth;
        height = dred_control_get_height(DRED_CONTROL(pTextBox)) - (pTextBox->borderWidth + pTextBox->padding)*2 - horzScrollbarSize;
    }

    if (pWidthOut != NULL) {
        *pWidthOut = width;
    }
    if (pHeightOut != NULL) {
        *pHeightOut = height;
    }
}

dred_rect dred_textbox__get_text_rect(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return dred_make_rect(0, 0, 0, 0);
    }

    float offsetX;
    float offsetY;
    dred_textbox__get_text_offset(pTextBox, &offsetX, &offsetY);

    float width;
    float height;
    dred_textbox__calculate_text_engine_container_size(pTextBox, &width, &height);

    return dred_make_rect(offsetX, offsetY, offsetX + width, offsetY + height);
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

void dred_textbox__refresh_horizontal_scrollbar(dred_textbox* pTextBox)
{
    assert(pTextBox != NULL);

    float textWidth = drte_engine_get_visible_line_width(pTextBox->pTL);
    float containerWidth;
    drte_engine_get_container_size(pTextBox->pTL, &containerWidth, NULL);
    dred_scrollbar_set_range_and_page_size(pTextBox->pHorzScrollbar, 0, (int)(textWidth + (containerWidth/4)), (int)containerWidth);

    if (dred_scrollbar_is_thumb_visible(pTextBox->pHorzScrollbar)) {
        if (!dred_control_is_visible(DRED_CONTROL(pTextBox->pHorzScrollbar))) {
            dred_control_show(DRED_CONTROL(pTextBox->pHorzScrollbar));
            dred_textbox__refresh_line_numbers(pTextBox);
        }
    } else {
        if (dred_control_is_visible(DRED_CONTROL(pTextBox->pHorzScrollbar))) {
            dred_control_hide(DRED_CONTROL(pTextBox->pHorzScrollbar));
            dred_textbox__refresh_line_numbers(pTextBox);
        }
    }
}

void dred_textbox__refresh_scrollbar_ranges(dred_textbox* pTextBox)
{
    assert(pTextBox != NULL);

    // The vertical scrollbar is based on the line count.
    size_t lineCount = drte_engine_get_line_count(pTextBox->pTL);
    size_t pageSize  = drte_engine_get_visible_line_count(pTextBox->pTL);//drte_engine_get_visible_line_count_starting_at(pTextBox->pTL, dred_scrollbar_get_scroll_position(pTextBox->pVertScrollbar));

    size_t extraScroll = 0;
    if (pTextBox->isExcessScrollingEnabled) {
        extraScroll = drte_engine_get_visible_line_count(pTextBox->pTL) - 1 - 1;  // -1 to make the range 0 based. -1 to ensure at least one line is visible.
    }

    dred_scrollbar_set_range_and_page_size(pTextBox->pVertScrollbar, 0, (int)(lineCount + extraScroll), (int)pageSize);

    if (dred_scrollbar_is_thumb_visible(pTextBox->pVertScrollbar)) {
        if (!dred_control_is_visible(DRED_CONTROL(pTextBox->pVertScrollbar))) {
            dred_control_show(DRED_CONTROL(pTextBox->pVertScrollbar));
        }
    } else {
        if (dred_control_is_visible(DRED_CONTROL(pTextBox->pVertScrollbar))) {
            dred_control_hide(DRED_CONTROL(pTextBox->pVertScrollbar));
        }
    }


    // The horizontal scrollbar is a per-pixel scrollbar, and is based on the width of the text versus the width of the container.
    dred_textbox__refresh_horizontal_scrollbar(pTextBox);
}

void dred_textbox__refresh_scrollbar_layouts(dred_textbox* pTextBox)
{
    assert(pTextBox != NULL);

    float offsetLeft   = pTextBox->borderWidth;
    float offsetTop    = pTextBox->borderWidth;
    float offsetRight  = pTextBox->borderWidth;
    float offsetBottom = pTextBox->borderWidth;

    float scrollbarSizeH = (dred_scrollbar_is_thumb_visible(pTextBox->pHorzScrollbar) && pTextBox->isHorzScrollbarEnabled) ? pTextBox->horzScrollbarSize : 0;
    float scrollbarSizeV = (dred_scrollbar_is_thumb_visible(pTextBox->pVertScrollbar) && pTextBox->isVertScrollbarEnabled) ? pTextBox->vertScrollbarSize : 0;

    dred_control_set_size(DRED_CONTROL(pTextBox->pVertScrollbar), scrollbarSizeV, dred_control_get_height(DRED_CONTROL(pTextBox)) /*- scrollbarSizeH*/ - (offsetTop + offsetBottom));
    dred_control_set_size(DRED_CONTROL(pTextBox->pHorzScrollbar), dred_control_get_width(DRED_CONTROL(pTextBox)) - scrollbarSizeV - (offsetLeft + offsetRight), scrollbarSizeH);

    dred_control_set_relative_position(DRED_CONTROL(pTextBox->pVertScrollbar), dred_control_get_width(DRED_CONTROL(pTextBox)) - scrollbarSizeV - offsetRight, offsetTop);
    dred_control_set_relative_position(DRED_CONTROL(pTextBox->pHorzScrollbar), offsetLeft, dred_control_get_height(DRED_CONTROL(pTextBox)) - scrollbarSizeH - offsetBottom);


    // A change in the layout of the horizontal scrollbar will affect the layout of the line numbers.
    dred_textbox__refresh_line_numbers(pTextBox);
}

dred_rect dred_textbox__get_scrollbar_dead_space_rect(dred_textbox* pTextBox)
{
    assert(pTextBox != NULL);

    float offsetLeft   = pTextBox->borderWidth;
    float offsetTop    = pTextBox->borderWidth;
    float offsetRight  = pTextBox->borderWidth;
    float offsetBottom = pTextBox->borderWidth;

    float scrollbarSizeH = (dred_control_is_visible(DRED_CONTROL(pTextBox->pHorzScrollbar)) && pTextBox->isHorzScrollbarEnabled) ? dred_control_get_width(DRED_CONTROL(pTextBox->pHorzScrollbar)) : 0;
    float scrollbarSizeV = (dred_control_is_visible(DRED_CONTROL(pTextBox->pVertScrollbar)) && pTextBox->isHorzScrollbarEnabled) ? dred_control_get_height(DRED_CONTROL(pTextBox->pVertScrollbar)) : 0;

    if (scrollbarSizeH == 0 && scrollbarSizeV == 0) {
        return dred_make_rect(0, 0, 0, 0);
    }

    return dred_make_rect(scrollbarSizeH + offsetLeft, scrollbarSizeV + offsetTop, dred_control_get_width(DRED_CONTROL(pTextBox)) - offsetRight, dred_control_get_height(DRED_CONTROL(pTextBox)) - offsetBottom);
}


void dred_textbox__on_mouse_move_line_numbers(dred_control* pLineNumbers, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;

    dred_textbox* pTextBox = DRED_TEXTBOX(dred_control_get_parent(pLineNumbers));
    assert(pTextBox != NULL);

    if ((stateFlags & DRED_GUI_MOUSE_BUTTON_LEFT_DOWN) != 0)
    {
        if (dred_gui_get_element_with_mouse_capture(pLineNumbers->pGUI) == pLineNumbers)
        {
            // We just move the cursor around based on the line number we've moved over.

            //float offsetX = pTextEditorData->padding;
            float offsetY = pTextBox->padding + pTextBox->pTL->innerOffsetY;
            size_t iLine = drte_engine_get_line_at_pos_y(pTextBox->pTL, NULL, relativeMousePosY - offsetY);
            size_t iAnchorLine = pTextBox->iLineSelectAnchor;
            size_t lineCount = drte_engine_get_line_count(pTextBox->pTL);

            size_t iSelectionFirstLine = drte_engine_get_selection_first_line(pTextBox->pTL, pTextBox->pTL->selectionCount-1);
            size_t iSelectionLastLine = drte_engine_get_selection_last_line(pTextBox->pTL, pTextBox->pTL->selectionCount-1);
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
                    drte_engine_move_selection_anchor_to_start_of_line(pTextBox->pTL, iAnchorLine + 1);
                } else {
                    drte_engine_move_selection_anchor_to_end_of_line(pTextBox->pTL, iAnchorLine);
                }
            } else {
                drte_engine_move_selection_anchor_to_start_of_line(pTextBox->pTL, iAnchorLine);
            }


            // If we're moving up we want the cursor to be placed at the start of the selection range. Otherwise we want to place the cursor
            // at the end of the selection range.
            if (movingUp) {
                drte_engine_move_cursor_to_start_of_line_by_index(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), iLine);
            } else {
                if (iLine + 1 < lineCount) {
                    drte_engine_move_cursor_to_start_of_line_by_index(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), iLine + 1);
                } else {
                    drte_engine_move_cursor_to_end_of_line_by_index(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), iLine);
                }
            }


            drte_engine_set_selection_end_point(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
        }
    }
}

void dred_textbox__on_mouse_button_down_line_numbers(dred_control* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)stateFlags;

    dred_textbox* pTextBox = DRED_TEXTBOX(dred_control_get_parent(pLineNumbers));
    assert(pTextBox != NULL);

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT)
    {
        dred_gui_capture_mouse(pLineNumbers);

        // If the shift key is down and we already have a selection, this is equivalent to a mouse drag.
        if ((stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) != 0) {
            if (drte_engine_is_anything_selected(pTextBox->pTL)) {
                dred_textbox__on_mouse_move_line_numbers(pLineNumbers, relativeMousePosX, relativeMousePosY, stateFlags | DRED_GUI_MOUSE_BUTTON_LEFT_DOWN);
                return;
            }
        }


        //float offsetX = pTextEditorData->padding;
        float offsetY = pTextBox->padding + pTextBox->pTL->innerOffsetY;
        size_t iClickedLine = drte_engine_get_line_at_pos_y(pTextBox->pTL, NULL, relativeMousePosY - offsetY);

        if ((stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) != 0) {
            pTextBox->iLineSelectAnchor = drte_engine_get_cursor_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
        } else {
            pTextBox->iLineSelectAnchor = iClickedLine;
        }
        

        if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) == 0) {
            dred_textbox_deselect_all(pTextBox);
        }

        drte_engine_begin_selection(pTextBox->pTL, drte_engine_get_line_first_character(pTextBox->pTL, NULL, pTextBox->iLineSelectAnchor));


        if (iClickedLine + 1 < drte_engine_get_line_count(pTextBox->pTL)) {
            drte_engine_move_cursor_to_start_of_line_by_index(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL), iClickedLine + 1);
        } else {
            drte_engine_move_cursor_to_end_of_line(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL));
        }

        drte_engine_set_selection_end_point(pTextBox->pTL, drte_engine_get_cursor_character(pTextBox->pTL, drte_engine_get_last_cursor(pTextBox->pTL)));
    }
}

void dred_textbox__on_mouse_button_up_line_numbers(dred_control* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (dred_control_has_mouse_capture(pLineNumbers)) {
            dred_gui_release_mouse(pLineNumbers->pGUI);
        }
    }
}

void dred_textbox__on_paint_rect_line_numbers(drte_engine* pEngine, drte_style_token styleToken, drte_rect rect, void* pPaintData)
{
    dred_textbox* pTextBox = (dred_textbox*)pEngine->pUserData;
    dred_text_style* pStyle = (dred_text_style*)styleToken;

    assert(pTextBox != NULL);

    float offsetX = pTextBox->padding;
    float offsetY = pTextBox->padding;

    dred_control_draw_rect(pTextBox->pLineNumbers, dred_offset_rect(drte_rect_to_dred(rect), offsetX, offsetY), pStyle->bgColor, pPaintData);
}

void dred_textbox__on_paint_text_line_numbers(drte_engine* pEngine, drte_style_token styleTokenFG, drte_style_token styleTokenBG, const char* text, size_t textLength, float posX, float posY, void* pPaintData)
{
    dred_textbox* pTextBox = (dred_textbox*)pEngine->pUserData;

    dred_text_style* pStyleFG = (dred_text_style*)styleTokenFG;
    dred_text_style* pStyleBG = (dred_text_style*)styleTokenBG;

    assert(pTextBox != NULL);

    float offsetX = pTextBox->padding;
    float offsetY = pTextBox->padding;

    dred_control_draw_text(pTextBox->pLineNumbers, pStyleFG->pFont, text, (int)textLength, posX + offsetX, posY + offsetY, pStyleFG->fgColor, pStyleBG->bgColor, pPaintData);
}

void dred_textbox__on_paint_line_numbers(dred_control* pLineNumbers, dred_rect relativeRect, void* pPaintData)
{
    (void)relativeRect;

    dred_textbox* pTextBox = DRED_TEXTBOX(dred_control_get_parent(pLineNumbers));
    assert(pTextBox != NULL);

    float lineNumbersWidth  = dred_control_get_width(pLineNumbers) - (pTextBox->padding*2) - pTextBox->lineNumbersPaddingRight;
    float lineNumbersHeight = dred_control_get_height(pLineNumbers) - (pTextBox->padding*2);

    drte_engine_paint_line_numbers(pTextBox->pTL, lineNumbersWidth, lineNumbersHeight, dred_textbox__on_paint_text_line_numbers, dred_textbox__on_paint_rect_line_numbers, pPaintData);

    dred_control_draw_rect_outline(pLineNumbers, dred_control_get_local_rect(pLineNumbers), pTextBox->lineNumbersStyle.bgColor, pTextBox->padding, pPaintData);

    // Right padding.
    dred_rect rightPaddingRect = dred_control_get_local_rect(pLineNumbers);
    rightPaddingRect.right -= pTextBox->padding;
    rightPaddingRect.left   = rightPaddingRect.right - pTextBox->lineNumbersPaddingRight;
    dred_control_draw_rect(pLineNumbers, rightPaddingRect, pTextBox->lineNumbersStyle.bgColor, pPaintData);
}

void dred_textbox__refresh_line_numbers(dred_textbox* pTextBox)
{
    assert(pTextBox != NULL);

    dred_rect lineNumbersRectOld = dred_control_get_local_rect(pTextBox->pLineNumbers);
    dred_control_begin_dirty(pTextBox->pLineNumbers);
    
    float lineNumbersWidth = 0;
    if (dred_control_is_visible(pTextBox->pLineNumbers)) {
        lineNumbersWidth = pTextBox->lineNumbersWidth;
    }

    float scrollbarHeight = dred_control_is_visible(DRED_CONTROL(pTextBox->pHorzScrollbar)) ? dred_control_get_height(DRED_CONTROL(pTextBox->pHorzScrollbar)) : 0;
    dred_control_set_size(pTextBox->pLineNumbers, lineNumbersWidth, dred_control_get_height(DRED_CONTROL(pTextBox)) - scrollbarHeight);


    // The size of the text container may have changed.
    float textEditorWidth;
    float textEditorHeight;
    dred_textbox__calculate_text_engine_container_size(pTextBox, &textEditorWidth, &textEditorHeight);
    drte_engine_set_container_size(pTextBox->pTL, textEditorWidth, textEditorHeight);


    // Force a redraw just to be sure everything is in a valid state.
    dred_control_dirty(DRED_CONTROL(pTextBox), dred_rect_union(lineNumbersRectOld, dred_control_get_local_rect(pTextBox->pLineNumbers)));
    dred_control_end_dirty(pTextBox->pLineNumbers);
}
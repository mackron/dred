// Copyright (C) 2016 David Reid. See included LICENSE file.

/// Retrieves the offset to draw the text in the text box.
void dred_textview__get_text_offset(dred_textview* pTextView, float* pOffsetXOut, float* pOffsetYOut);

/// Calculates the required size of the text engine.
void dred_textview__calculate_text_engine_container_size(dred_textview* pTextView, float* pWidthOut, float* pHeightOut);

/// Retrieves the rectangle of the text engine's container.
dred_rect dred_textview__get_text_rect(dred_textview* pTextView);

/// Refreshes the range, page sizes and layouts of the scrollbars.
void dred_textview__refresh_scrollbars(dred_textview* pTextView);

/// Refreshes the range and page sizes of the scrollbars.
void dred_textview__refresh_scrollbar_ranges(dred_textview* pTextView);

/// Refreshes the size and position of the scrollbars.
void dred_textview__refresh_scrollbar_layouts(dred_textview* pTextView);

/// Retrieves a rectangle representing the space between the edges of the two scrollbars.
dred_rect dred_textview__get_scrollbar_dead_space_rect(dred_textview* pTextView);


/// Called when a mouse button is pressed on the line numbers element.
void dred_textview__on_mouse_move_line_numbers(dred_control* pLineNumbers, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when a mouse button is pressed on the line numbers element.
void dred_textview__on_mouse_button_down_line_numbers(dred_control* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when a mouse button is pressed on the line numbers element.
void dred_textview__on_mouse_button_up_line_numbers(dred_control* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);

/// Called when the line numbers element needs to be drawn.
void dred_textview__on_paint_line_numbers(dred_control* pLineNumbers, dred_rect relativeRect, void* pPaintData);

/// Refreshes the line number of the given text editor.
void dred_textview__refresh_line_numbers(dred_textview* pTextView);


/// on_paint_rect()
void dred_textview_engine__on_paint_rect(drte_engine* pLayout, drte_view* pView, drte_style_token styleToken, drte_rect rect, void* pPaintData);

/// on_paint_text()
void dred_textview_engine__on_paint_text(drte_engine* pTextEngine, drte_view* pView, drte_style_token styleTokenFG, drte_style_token styleTokenBG, const char* text, size_t textLength, float posX, float posY, void* pPaintData);

/// on_dirty()
void dred_textview_engine__on_dirty(drte_engine* pTextEngine, drte_view* pView, drte_rect rect);

/// on_cursor_move()
void dred_textview_engine__on_cursor_move(drte_engine* pTextEngine, drte_view* pView, size_t iCursor);

/// on_text_changed()
void dred_textview__on_text_changed(dred_textview* pTextView);

// on_get_undo_state()
size_t dred_textview__on_get_undo_state(dred_textview* pTextView, void* pDataOut);

// on_apply_undo_state()
void dred_textview__on_apply_undo_state(dred_textview* pTextView, size_t dataSize, const void* pData);


// dred_textview__refresh_horizontal_scrollbar()
void dred_textview__refresh_horizontal_scrollbar(dred_textview* pTextView);

void dred_textview__on_vscroll(dred_scrollbar* pSBControl, int scrollPos)
{
    dred_textview* pTextView = (dred_textview*)DRED_CONTROL(pSBControl)->pUserData;
    assert(pTextView != NULL);

    drte_view_set_inner_offset_y(pTextView->pView, -drte_view_get_line_pos_y(pTextView->pView, scrollPos));
    dred_textview__refresh_scrollbars(pTextView);

    // The line numbers need to be redrawn.
    dred_control_dirty(pTextView->pLineNumbers, dred_control_get_local_rect(pTextView->pLineNumbers));
}

void dred_textview__on_hscroll(dred_scrollbar* pSBControl, int scrollPos)
{
    dred_textview* pTextView = (dred_textview*)DRED_CONTROL(pSBControl)->pUserData;
    assert(pTextView != NULL);

    drte_view_set_inner_offset_x(pTextView->pView, (float)-scrollPos);
}

void dred_textview__refresh_style(dred_textview* pTextView)
{
    assert(pTextView != NULL);

    dred_gui_font_metrics fontMetrics;
    dred_gui_get_font_metrics(pTextView->defaultStyle.pFont, &fontMetrics);

    // Default.
    drte_engine_register_style_token(pTextView->pTextEngine, (drte_style_token)&pTextView->defaultStyle, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));

    // Selection.
    drte_engine_register_style_token(pTextView->pTextEngine, (drte_style_token)&pTextView->selectionStyle, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));

    // Active line.
    drte_engine_register_style_token(pTextView->pTextEngine, (drte_style_token)&pTextView->activeLineStyle, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));

    // Cursor.
    drte_engine_register_style_token(pTextView->pTextEngine, (drte_style_token)&pTextView->cursorStyle, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));

    // Line numbers.
    drte_engine_register_style_token(pTextView->pTextEngine, (drte_style_token)&pTextView->lineNumbersStyle, drte_font_metrics_create(fontMetrics.ascent, fontMetrics.descent, fontMetrics.lineHeight, fontMetrics.spaceWidth));
}


void dred_textview_engine__on_measure_string_proc(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textLength, float* pWidthOut, float* pHeightOut)
{
    (void)pEngine;
    dred_gui_measure_string(((dred_text_style*)styleToken)->pFont, text, textLength, pWidthOut, pHeightOut);
}

void dred_textview_engine__on_get_cursor_position_from_point(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut)
{
    (void)pEngine;
    dred_gui_get_text_cursor_position_from_point(((dred_text_style*)styleToken)->pFont, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosXOut, pCharacterIndexOut);
}

void dred_textview_engine__on_get_cursor_position_from_char(drte_engine* pEngine, drte_style_token styleToken, const char* text, size_t characterIndex, float* pTextCursorPosXOut)
{
    (void)pEngine;
    dred_gui_get_text_cursor_position_from_char(((dred_text_style*)styleToken)->pFont, text, characterIndex, pTextCursorPosXOut);
}


void dred_textview__clear_all_cursors_except_last(dred_textview* pTextView)
{
    // The last cursor is _not_ cleared.
    assert(pTextView != NULL);

    // Engine.
    drte_view_begin_dirty(pTextView->pView);
    while (pTextView->pView->cursorCount > 1) {
        drte_view_remove_cursor(pTextView->pView, pTextView->pView->cursorCount-2);
    }
    drte_view_end_dirty(pTextView->pView);


    // Local list.
    if (pTextView->cursorCount > 1) {
        pTextView->pCursors[0] = pTextView->pCursors[pTextView->cursorCount-1];
        pTextView->pCursors[0].iEngineSelection = (size_t)-1;
        pTextView->cursorCount = 1;
    }
}

void dred_textview__clear_all_cursors(dred_textview* pTextView)
{
    // The last cursor is _not_ cleared.
    assert(pTextView != NULL);

    // Engine.
    drte_view_begin_dirty(pTextView->pView);
    while (pTextView->pView->cursorCount > 0) {
        drte_view_remove_cursor(pTextView->pView, pTextView->pView->cursorCount-1);
    }
    drte_view_end_dirty(pTextView->pView);


    // Local list.
    if (pTextView->cursorCount > 0) {
        pTextView->pCursors[0] = pTextView->pCursors[pTextView->cursorCount-1];
        pTextView->pCursors[0].iEngineSelection = (size_t)-1;
        pTextView->cursorCount = 0;
    }
}

void dred_textview__remove_cursor(dred_textview* pTextView, size_t iCursor)
{
    assert(pTextView != NULL);

    // Remove from the engine.
    drte_view_remove_cursor(pTextView->pView, iCursor);

    // Remove from the local list.
    for (size_t i = iCursor; i < pTextView->cursorCount-1; ++i) {
        pTextView->pCursors[i] = pTextView->pCursors[i+1];
    }
    pTextView->cursorCount -= 1;
}

void dred_textview__insert_cursor(dred_textview* pTextView, size_t iChar, size_t iLine)
{
    assert(pTextView != NULL);

    // If we are trying to insert a cursor on top of an existing cursor we need to just move the existing one to the end of the list,
    // thus making it the current cursor. We don't want cursors to be sitting on top of each other.
    size_t iExistingCursor = (size_t)-1;
    for (size_t iCursor = 0; iCursor < pTextView->pView->cursorCount; ++iCursor) {
        if (pTextView->pView->pCursors[iCursor].iCharAbs == iChar) {
            iExistingCursor = iCursor;
            break;
        }
    }

    size_t iEngineSelection = (size_t)-1;
    if (iExistingCursor != (size_t)-1) {
        iEngineSelection = pTextView->pCursors[iExistingCursor].iEngineSelection;
        dred_textview__remove_cursor(pTextView, iExistingCursor);
    }

    size_t iEngineCursor = drte_view_insert_cursor_at_character_and_line(pTextView->pView, iChar, iLine);

    dred_textview_cursor* pNewCursors = (dred_textview_cursor*)realloc(pTextView->pCursors, (pTextView->cursorCount+1) * sizeof(*pNewCursors));
    if (pNewCursors == NULL) {
        return;
    }

    pTextView->pCursors = pNewCursors;
    pTextView->pCursors[pTextView->cursorCount].iEngineSelection = iEngineSelection;
    pTextView->pCursors[pTextView->cursorCount].iEngineCursor = iEngineCursor;
    pTextView->cursorCount += 1;
}

dr_bool32 dred_textview__get_cursor_selection(dred_textview* pTextView, size_t iCursor, size_t* iSelectionOut)
{
    assert(pTextView != NULL);

    for (size_t iSelection = 0; iSelection < pTextView->pView->selectionCount; ++iSelection) {
        drte_region selection = drte_region_normalize(pTextView->pView->pSelections[iSelection]);
        if (selection.iCharBeg == pTextView->pView->pCursors[iCursor].iCharAbs || selection.iCharEnd == pTextView->pView->pCursors[iCursor].iCharAbs) {
            if (iSelectionOut) *iSelectionOut = iSelection;
            return DR_TRUE;
        }
    }

    return DR_FALSE;
}

dr_bool32 dred_textview__is_cursor_on_selection(dred_textview* pTextView)
{
    assert(pTextView != NULL);

    return dred_textview__get_cursor_selection(pTextView, drte_view_get_last_cursor(pTextView->pView), NULL);
}

dr_bool32 dred_textview__move_cursor_to_start_of_selection(dred_textview* pTextView, size_t* iSelectionOut)
{
    assert(pTextView != NULL);

    if (pTextView->pView->cursorCount == 0) {
        return DR_FALSE;
    }

    // We need to find the selection region that the last cursor is sitting at the end of. If there isn't one, we just return DR_FALSE.
    size_t iCursor = drte_view_get_last_cursor(pTextView->pView);
    for (size_t iSelection = 0; iSelection < pTextView->pView->selectionCount; ++iSelection) {
        drte_region selection = drte_region_normalize(pTextView->pView->pSelections[iSelection]);
        if (selection.iCharEnd == pTextView->pView->pCursors[iCursor].iCharAbs) {
            // It's on this selection.
            drte_view_move_cursor_to_character(pTextView->pView, iCursor, selection.iCharBeg);
            if (iSelectionOut) *iSelectionOut = iSelection;
            return DR_TRUE;
        }
    }

    return DR_FALSE;
}

dr_bool32 dred_textview__move_cursor_to_end_of_selection(dred_textview* pTextView, size_t* iSelectionOut)
{
    assert(pTextView != NULL);

    if (pTextView->pView->cursorCount == 0) {
        return DR_FALSE;
    }

    // We need to find the selection region that the last cursor is sitting at the end of. If there isn't one, we just return DR_FALSE.
    size_t iCursor = drte_view_get_last_cursor(pTextView->pView);
    for (size_t iSelection = 0; iSelection < pTextView->pView->selectionCount; ++iSelection) {
        drte_region selection = drte_region_normalize(pTextView->pView->pSelections[iSelection]);
        if (selection.iCharBeg == pTextView->pView->pCursors[iCursor].iCharAbs) {
            // It's on this selection.
            drte_view_move_cursor_to_character(pTextView->pView, iCursor, selection.iCharEnd);
            if (iSelectionOut) *iSelectionOut = iSelection;
            return DR_TRUE;
        }
    }

    return DR_FALSE;
}

dr_bool32 dred_textview__insert_tab(dred_textview* pTextView, size_t iChar)
{
    assert(pTextView != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pTextView));
    assert(pDred != NULL);

    drte_view_begin_dirty(pTextView->pView);

    dr_bool32 wasTextChanged = DR_FALSE;
    if (pDred->config.textEditorTabsToSpacesEnabled) {
        size_t insertedCharacterCount = drte_view_get_spaces_to_next_column_from_character(pTextView->pView, iChar);
        for (size_t i = 0; i < insertedCharacterCount; ++i) {
            wasTextChanged = drte_engine_insert_character(pTextView->pTextEngine, iChar, ' ') || wasTextChanged;
        }
    } else {
        wasTextChanged = drte_engine_insert_character(pTextView->pTextEngine, iChar, '\t') || wasTextChanged;
    }


    drte_view_end_dirty(pTextView->pView);
    return wasTextChanged;
}

dr_bool32 dred_textview__insert_tab_at_cursor(dred_textview* pTextView, size_t iCursor)
{
    assert(pTextView != NULL);

    return dred_textview__insert_tab(pTextView, pTextView->pView->pCursors[iCursor].iCharAbs);
}


dr_bool32 dred_textview_init(dred_textview* pTextView, dred_context* pDred, dred_control* pParent, drte_engine* pTextEngine)
{
    if (pTextView == NULL || pTextEngine == NULL) {
        return DR_FALSE;
    }

    memset(pTextView, 0, sizeof(*pTextView));
    if (!dred_control_init(DRED_CONTROL(pTextView), pDred, pParent, DRED_CONTROL_TYPE_TEXTVIEW)) {
        return DR_FALSE;
    }

    pTextView->pTextEngine = pTextEngine;

    pTextView->pView = drte_view_create(pTextView->pTextEngine);
    if (pTextView->pView == NULL) {
        dred_control_uninit(DRED_CONTROL(pTextView));
        return DR_FALSE;
    }

    pTextView->pView->pUserData = pTextView;

    dred_textview__insert_cursor(pTextView, 0, 0);



    dred_control_set_cursor(DRED_CONTROL(pTextView), dred_cursor_type_text);
    dred_control_set_on_size(DRED_CONTROL(pTextView), dred_textview_on_size);
    dred_control_set_on_mouse_move(DRED_CONTROL(pTextView), dred_textview_on_mouse_move);
    dred_control_set_on_mouse_button_down(DRED_CONTROL(pTextView), dred_textview_on_mouse_button_down);
    dred_control_set_on_mouse_button_up(DRED_CONTROL(pTextView), dred_textview_on_mouse_button_up);
    dred_control_set_on_mouse_button_dblclick(DRED_CONTROL(pTextView), dred_textview_on_mouse_button_dblclick);
    dred_control_set_on_mouse_wheel(DRED_CONTROL(pTextView), dred_textview_on_mouse_wheel);
    dred_control_set_on_key_down(DRED_CONTROL(pTextView), dred_textview_on_key_down);
    dred_control_set_on_printable_key_down(DRED_CONTROL(pTextView), dred_textview_on_printable_key_down);
    dred_control_set_on_paint(DRED_CONTROL(pTextView), dred_textview_on_paint);
    dred_control_set_on_capture_keyboard(DRED_CONTROL(pTextView), dred_textview_on_capture_keyboard);
    dred_control_set_on_release_keyboard(DRED_CONTROL(pTextView), dred_textview_on_release_keyboard);
    dred_control_set_on_capture_mouse(DRED_CONTROL(pTextView), dred_textview_on_capture_mouse);
    dred_control_set_on_release_mouse(DRED_CONTROL(pTextView), dred_textview_on_release_mouse);

    pTextView->pVertScrollbar = &pTextView->vertScrollbar;
    dred_scrollbar_init(pTextView->pVertScrollbar, pDred, DRED_CONTROL(pTextView), dred_scrollbar_orientation_vertical);
    DRED_CONTROL(pTextView->pVertScrollbar)->pUserData = pTextView;
    dred_scrollbar_set_on_scroll(pTextView->pVertScrollbar, dred_textview__on_vscroll);
    dred_scrollbar_set_mouse_wheel_scele(pTextView->pVertScrollbar, 3);

    pTextView->pHorzScrollbar = &pTextView->horzScrollbar;
    dred_scrollbar_init(pTextView->pHorzScrollbar, pDred, DRED_CONTROL(pTextView), dred_scrollbar_orientation_horizontal);
    DRED_CONTROL(pTextView->pHorzScrollbar)->pUserData = pTextView;
    dred_scrollbar_set_on_scroll(pTextView->pHorzScrollbar, dred_textview__on_hscroll);

    pTextView->pLineNumbers = &pTextView->lineNumbers;
    dred_control_init(pTextView->pLineNumbers, pDred, DRED_CONTROL(pTextView), "dred.common.linenumbers");
    dred_control_hide(pTextView->pLineNumbers);
    dred_control_set_on_mouse_move(pTextView->pLineNumbers, dred_textview__on_mouse_move_line_numbers);
    dred_control_set_on_mouse_button_down(pTextView->pLineNumbers, dred_textview__on_mouse_button_down_line_numbers);
    dred_control_set_on_mouse_button_up(pTextView->pLineNumbers, dred_textview__on_mouse_button_up_line_numbers);
    dred_control_set_on_paint(pTextView->pLineNumbers, dred_textview__on_paint_line_numbers);


    pTextView->pTextEngine->onMeasureString = dred_textview_engine__on_measure_string_proc;
    pTextView->pTextEngine->onGetCursorPositionFromPoint = dred_textview_engine__on_get_cursor_position_from_point;
    pTextView->pTextEngine->onGetCursorPositionFromChar = dred_textview_engine__on_get_cursor_position_from_char;


    pTextView->defaultStyle.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextView->defaultStyle.bgColor = dred_rgb(64, 64, 64);
    pTextView->defaultStyle.fgColor = dred_rgb(0, 0, 0);

    pTextView->selectionStyle.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextView->selectionStyle.bgColor = dred_rgb(64, 128, 192);
    pTextView->selectionStyle.fgColor = dred_rgb(0, 0, 0);

    pTextView->activeLineStyle.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextView->activeLineStyle.bgColor = dred_rgb(64, 64, 64);
    pTextView->activeLineStyle.fgColor = dred_rgb(0, 0, 0);

    pTextView->cursorStyle.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextView->cursorStyle.bgColor = dred_rgb(0, 0, 0);
    pTextView->cursorStyle.fgColor = dred_rgb(0, 0, 0);

    pTextView->lineNumbersStyle.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextView->lineNumbersStyle.bgColor = dred_rgb(64, 64, 64);
    pTextView->lineNumbersStyle.fgColor = dred_rgb(80, 160, 192);


    // Test styling.
    /*pTextView->testStyle0.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextView->testStyle0.bgColor = dred_rgb(64, 64, 64);
    pTextView->testStyle0.fgColor = dred_rgb(64, 160, 255);

    pTextView->testStyle1.pFont = dred_font_acquire_subfont(pDred->config.pTextEditorFont, pDred->uiScale);
    pTextView->testStyle1.bgColor = dred_rgb(64, 64, 64);
    pTextView->testStyle1.fgColor = dred_rgb(64, 192, 92);*/


    // Register the styles with the text engine.
    dred_textview__refresh_style(pTextView);


    //drte_engine_set_highlighter(pTextView->pTextEngine, dred_textview_engine__on_get_next_highlight, pTextView);



    drte_engine_set_default_style(pTextView->pTextEngine, (drte_style_token)&pTextView->defaultStyle);
    drte_engine_set_selection_style(pTextView->pTextEngine, (drte_style_token)&pTextView->selectionStyle);
    drte_engine_set_active_line_style(pTextView->pTextEngine, (drte_style_token)&pTextView->activeLineStyle);
    drte_engine_set_cursor_style(pTextView->pTextEngine, (drte_style_token)&pTextView->cursorStyle);
    drte_engine_set_line_numbers_style(pTextView->pTextEngine, (drte_style_token)&pTextView->lineNumbersStyle);

    // TODO: These need to be moved out of here.
    drte_engine_set_on_paint_rect(pTextView->pTextEngine, dred_textview_engine__on_paint_rect);
    drte_engine_set_on_paint_text(pTextView->pTextEngine, dred_textview_engine__on_paint_text);
    drte_engine_set_on_dirty(pTextView->pTextEngine, dred_textview_engine__on_dirty);
    drte_engine_set_on_cursor_move(pTextView->pTextEngine, dred_textview_engine__on_cursor_move);
    //drte_engine_set_on_text_changed(pTextView->pTextEngine, dred_textview_engine__on_text_changed);
    //drte_engine_set_on_undo_point_changed(pTextView->pTextEngine, dred_textview_engine__on_undo_point_changed);
    //pTextView->pTextEngine->onGetUndoState = dred_textview_engine__on_get_undo_state;
    //pTextView->pTextEngine->onApplyUndoState = dred_textview_engine__on_apply_undo_state;

    //drte_engine_set_default_text_color(pTextView->pTextEngine, dred_rgb(0, 0, 0));
    //drte_engine_set_cursor_color(pTextView->pTextEngine, dred_rgb(0, 0, 0));
    //drte_engine_set_default_bg_color(pTextView->pTextEngine, dred_rgb(64, 64, 64));
    //drte_engine_set_active_line_bg_color(pTextView->pTextEngine, dred_rgb(64, 64, 64));

    pTextView->borderColor = dred_rgb(0, 0, 0);
    pTextView->borderWidth = 0;
    pTextView->padding     = 2;
    pTextView->lineNumbersWidth = 64;
    pTextView->lineNumbersPaddingRight = 16;
    //pTextView->lineNumbersColor = dred_rgb(80, 160, 192);
    //pTextView->lineNumbersBackgroundColor = pTextView->defaultStyle.bgColor;
    pTextView->vertScrollbarSize = 16;
    pTextView->horzScrollbarSize = 16;
    pTextView->isVertScrollbarEnabled = DR_TRUE;
    pTextView->isHorzScrollbarEnabled = DR_TRUE;
    pTextView->isExcessScrollingEnabled = pDred->config.textEditorEnableExcessScrolling;
    pTextView->isDragAndDropEnabled = pDred->config.textEditorEnableDragAndDrop;
    pTextView->isWantingToDragAndDrop = DR_FALSE;
    pTextView->iLineSelectAnchor = 0;
    pTextView->onCursorMove = NULL;
    pTextView->onUndoPointChanged = NULL;

    return DR_TRUE;
}

void dred_textview_uninit(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    // Keyboard focus needs to be released first. If we don't do this we'll not free delete the internal timer.
    if (dred_control_has_keyboard_capture(DRED_CONTROL(pTextView))) {
        dred_gui_release_keyboard(dred_control_get_gui(DRED_CONTROL(pTextView)));
    }

    if (pTextView->pLineNumbers) {
        dred_control_uninit(pTextView->pLineNumbers);
        pTextView->pLineNumbers = NULL;
    }

    if (pTextView->pHorzScrollbar) {
        dred_scrollbar_uninit(pTextView->pHorzScrollbar);
        pTextView->pHorzScrollbar = NULL;
    }

    if (pTextView->pVertScrollbar) {
        dred_scrollbar_uninit(pTextView->pVertScrollbar);
        pTextView->pVertScrollbar = NULL;
    }

    if (pTextView->pCursors) {
        free(pTextView->pCursors);
        pTextView->pCursors = NULL;
    }

    if (pTextView->pView) {
        drte_view_delete(pTextView->pView);
        pTextView->pView = NULL;
    }

    dred_control_uninit(DRED_CONTROL(pTextView));
}


drte_engine* dred_textview_get_engine(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return NULL;
    }

    return pTextView->pTextEngine;
}


void dred_textview_set_font(dred_textview* pTextView, dred_gui_font* pFont)
{
    if (pTextView == NULL) {
        return;
    }

    dred_control_begin_dirty(DRED_CONTROL(pTextView));
    {
        //drte_engine_set_default_font(pTextView->pTextEngine, pFont);
        pTextView->defaultStyle.pFont = pFont;
        pTextView->lineNumbersStyle.pFont = pFont;
        //pTextView->testStyle0.pFont = pFont;

        dred_textview__refresh_style(pTextView);

        // The font used for line numbers are tied to the main font at the moment.
        dred_textview__refresh_line_numbers(pTextView);

        // Emulate a scroll to ensure the scroll position is pinned to a line.
        dred_textview__on_vscroll(pTextView->pVertScrollbar, dred_scrollbar_get_scroll_position(pTextView->pVertScrollbar));
        dred_textview__refresh_scrollbars(pTextView);
    }
    dred_control_end_dirty(DRED_CONTROL(pTextView));
}

dred_gui_font* dred_textview_get_font(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return NULL;
    }

    return pTextView->defaultStyle.pFont;
}

void dred_textview_set_text_color(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->defaultStyle.fgColor = color;
    dred_textview__refresh_style(pTextView);
}

void dred_textview_set_background_color(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->defaultStyle.bgColor = color;
    dred_textview__refresh_style(pTextView);
}

void dred_textview_set_selection_background_color(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->selectionStyle.bgColor = color;
    dred_textview__refresh_style(pTextView);
}

dred_color dred_textview_get_selection_background_color(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTextView->selectionStyle.bgColor;
}

void dred_textview_set_active_line_background_color(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->activeLineStyle.bgColor = color;
    dred_textview__refresh_style(pTextView);
    //drte_engine_set_active_line_bg_color(pTextView->pTextEngine, color);
}

void dred_textview_set_cursor_width(dred_textview* pTextView, float cursorWidth)
{
    if (pTextView == NULL) {
        return;
    }

    drte_view_set_cursor_width(pTextView->pView, cursorWidth);
}

float dred_textview_get_cursor_width(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return 0;
    }

    return drte_view_get_cursor_width(pTextView->pView);
}

void dred_textview_set_cursor_color(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->cursorStyle.bgColor = color;
    dred_textview__refresh_style(pTextView);
    //drte_engine_set_cursor_color(pTextView->pTextEngine, color);
}

void dred_textview_set_border_color(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->borderColor = color;
}

void dred_textview_set_border_width(dred_textview* pTextView, float borderWidth)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->borderWidth = borderWidth;
}

void dred_textview_set_padding(dred_textview* pTextView, float padding)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->padding = padding;
}

float dred_textview_get_padding_vert(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return 0;
    }

    return pTextView->padding;
}

float dred_textview_get_padding_horz(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return 0;
    }

    return pTextView->padding;
}

void dred_textview_set_line_numbers_width(dred_textview* pTextView, float lineNumbersWidth)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->lineNumbersWidth = lineNumbersWidth;
}

float dred_textview_get_line_numbers_width(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return 0;
    }

    return pTextView->lineNumbersWidth;
}

void dred_textview_set_line_numbers_padding(dred_textview* pTextView, float lineNumbersPadding)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->lineNumbersPaddingRight = lineNumbersPadding;
}

float dred_textview_get_line_numbers_padding(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return 0;
    }

    return pTextView->lineNumbersPaddingRight;
}

void dred_textview_set_line_numbers_color(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    //pTextView->lineNumbersColor = color;
    pTextView->lineNumbersStyle.fgColor = color;
    dred_textview__refresh_style(pTextView);
    dred_textview__refresh_line_numbers(pTextView);
}

dred_color dred_textview_get_line_numbers_color(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTextView->lineNumbersStyle.fgColor;
}

void dred_textview_set_line_numbers_background_color(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->lineNumbersStyle.bgColor = color;
    dred_textview__refresh_style(pTextView);
    dred_textview__refresh_line_numbers(pTextView);
}

dred_color dred_textview_get_line_numbers_background_color(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pTextView->lineNumbersStyle.bgColor;
}

void dred_textview_set_scrollbar_track_color(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    dred_scrollbar_set_track_color(pTextView->pHorzScrollbar, color);
    dred_scrollbar_set_track_color(pTextView->pVertScrollbar, color);
}

void dred_textview_set_scrollbar_thumb_color(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    dred_scrollbar_set_default_thumb_color(pTextView->pHorzScrollbar, color);
    dred_scrollbar_set_default_thumb_color(pTextView->pVertScrollbar, color);
}

void dred_textview_set_scrollbar_thumb_color_hovered(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    dred_scrollbar_set_hovered_thumb_color(pTextView->pHorzScrollbar, color);
    dred_scrollbar_set_hovered_thumb_color(pTextView->pVertScrollbar, color);
}

void dred_textview_set_scrollbar_thumb_color_pressed(dred_textview* pTextView, dred_color color)
{
    if (pTextView == NULL) {
        return;
    }

    dred_scrollbar_set_pressed_thumb_color(pTextView->pHorzScrollbar, color);
    dred_scrollbar_set_pressed_thumb_color(pTextView->pVertScrollbar, color);
}


void dred_textview_enable_word_wrap(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    drte_view_enable_word_wrap(pTextView->pView);
    dred_textview__refresh_scrollbars(pTextView);
}

void dred_textview_disable_word_wrap(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    drte_view_disable_word_wrap(pTextView->pView);
    dred_textview__refresh_scrollbars(pTextView);
}

dr_bool32 dred_textview_is_word_wrap_enabled(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    return drte_view_is_word_wrap_enabled(pTextView->pView);
}


void dred_textview_enable_drag_and_drop(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->isDragAndDropEnabled = DR_TRUE;
}

void dred_textview_disable_drag_and_drop(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->isDragAndDropEnabled = DR_FALSE;
}

dr_bool32 dred_textview_is_drag_and_drop_enabled(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    return pTextView->isDragAndDropEnabled;
}



void dred_textview_set_text(dred_textview* pTextView, const char* text)
{
    if (pTextView == NULL) {
        return;
    }


    // The cursors and selection regions need to be cancelled here to ensure they don't reference invalid regions due to a
    // change in text. This should not have any major usability issues, but it can be tweaked if need be.
    dred_textview__clear_all_cursors_except_last(pTextView);
    drte_view_deselect_all(pTextView->pView);
    size_t iCursorChar = drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));


    // Set the text.
    drte_engine_prepare_undo_point(pTextView->pTextEngine);
    {
        drte_engine_set_text(pTextView->pTextEngine, text);
    }
    drte_engine_commit_undo_point(pTextView->pTextEngine);


    // Restore cursors.
    drte_view_move_cursor_to_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), iCursorChar);
}

size_t dred_textview_get_text(dred_textview* pTextView, char* pTextOut, size_t textOutSize)
{
    if (pTextView == NULL) {
        return 0;
    }

    return drte_engine_get_text(pTextView->pTextEngine, pTextOut, textOutSize);
}

void dred_textview_step(dred_textview* pTextView, unsigned int milliseconds)
{
    if (pTextView == NULL) {
        return;
    }

    drte_engine_step(pTextView->pTextEngine, milliseconds);
}

void dred_textview_set_cursor_blink_rate(dred_textview* pTextView, unsigned int blinkRateInMilliseconds)
{
    if (pTextView == NULL) {
        return;
    }

    drte_engine_set_cursor_blink_rate(pTextView->pTextEngine, blinkRateInMilliseconds);
}

void dred_textview_move_cursor_to_end_of_text(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    drte_view_move_cursor_to_end_of_text(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
}

void dred_textview_move_cursor_to_start_of_line_by_index(dred_textview* pTextView, size_t iLine)
{
    if (pTextView == NULL) {
        return;
    }

    drte_view_move_cursor_to_start_of_line_by_index(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), iLine);
}

void dred_textview_move_cursor_to_start_of_unwrapped_line_by_index(dred_textview* pTextView, size_t iLine)
{
    if (pTextView == NULL) {
        return;
    }

    drte_view_move_cursor_to_start_of_unwrapped_line_by_index(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), iLine);
}


dr_bool32 dred_textview_is_anything_selected(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    return drte_view_is_anything_selected(pTextView->pView);
}

void dred_textview_select_all(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    drte_view_select_all(pTextView->pView);
}

void dred_textview_deselect_all(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    drte_view_deselect_all(pTextView->pView);
    dred_textview__clear_all_cursors_except_last(pTextView);
}

size_t dred_textview_get_selected_text(dred_textview* pTextView, char* textOut, size_t textOutLength)
{
    if (pTextView == NULL) {
        return 0;
    }

    return drte_view_get_selected_text(pTextView->pView, textOut, textOutLength);
}

dr_bool32 dred_textview_delete_character_to_right_of_cursor(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    dr_bool32 wasTextChanged = DR_FALSE;
    drte_engine_prepare_undo_point(pTextView->pTextEngine);
    {
        wasTextChanged = drte_view_delete_character_to_right_of_cursor(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextView->pTextEngine); }

    return wasTextChanged;
}

dr_bool32 dred_textview_delete_selected_text_no_undo(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    dr_bool32 wasTextChanged = drte_view_delete_selected_text(pTextView->pView);
    drte_view_deselect_all(pTextView->pView);

    return wasTextChanged;
}

dr_bool32 dred_textview_delete_selected_text(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    dr_bool32 wasTextChanged = DR_FALSE;
    drte_engine_prepare_undo_point(pTextView->pTextEngine);
    {
        wasTextChanged = dred_textview_delete_selected_text_no_undo(pTextView);
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextView->pTextEngine); }

    return wasTextChanged;
}

dr_bool32 dred_textview_insert_text_at_cursors_no_undo(dred_textview* pTextView, const char* text)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    dr_bool32 wasTextChanged = DR_FALSE;
    for (size_t iCursor = 0; iCursor < pTextView->cursorCount; ++iCursor) {
        wasTextChanged = drte_view_insert_text_at_cursor(pTextView->pView, iCursor, text) || wasTextChanged;
    }

    return wasTextChanged;
}

dr_bool32 dred_textview_insert_text_at_cursors(dred_textview* pTextView, const char* text)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    dr_bool32 wasTextChanged = DR_FALSE;
    drte_engine_prepare_undo_point(pTextView->pTextEngine);
    {
        wasTextChanged = dred_textview_insert_text_at_cursors_no_undo(pTextView, text) || wasTextChanged;
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextView->pTextEngine); }

    return wasTextChanged;
}

dr_bool32 dred_textview_unindent_selected_blocks(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    drte_view_begin_dirty(pTextView->pView);

    dr_bool32 wasTextChanged = DR_FALSE;
    drte_engine_prepare_undo_point(pTextView->pTextEngine);
    {
        for (size_t iSelection = 0; iSelection < pTextView->pView->selectionCount; ++iSelection) {
            size_t iLineBeg = drte_view_get_selection_first_line(pTextView->pView, iSelection);
            size_t iLineEnd = drte_view_get_selection_last_line(pTextView->pView, iSelection);

            for (size_t iLine = iLineBeg; iLine <= iLineEnd; ++iLine) {
                size_t iLineChar = drte_view_get_line_first_character(pTextView->pView, NULL, iLine);
                size_t iLineCharNonWS = iLineChar;
                for (;;) {
                    uint32_t c = drte_engine_get_utf32(pTextView->pTextEngine, iLineCharNonWS);
                    if (c == '\0' || c == '\r' || c == '\n' || !dr_is_whitespace(c)) {
                        break;
                    }

                    iLineCharNonWS += 1;
                }

                if (iLineCharNonWS > iLineChar) {
                    size_t charactersRemovedCount = 0;
                    uint32_t c = drte_engine_get_utf32(pTextView->pTextEngine, iLineChar);
                    if (c == '\t') {
                        charactersRemovedCount = 1;
                    } else {
                        charactersRemovedCount = 0; //(iLineCharNonWS - iLineChar);
                        for (size_t iChar = iLineChar; iChar < iLineCharNonWS; ++iChar) {
                            if (charactersRemovedCount >= drte_view_get_tab_size(pTextView->pView)) {
                                break;
                            }

                            c = drte_engine_get_utf32(pTextView->pTextEngine, iChar);
                            if (c == '\t') {
                                break;
                            }

                            charactersRemovedCount += 1;
                        }
                    }

                    wasTextChanged = drte_engine_delete_text(pTextView->pTextEngine, iLineChar, iLineChar + charactersRemovedCount) || wasTextChanged;
                }
            }
        }
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextView->pTextEngine); }

    drte_view_end_dirty(pTextView->pView);

    return wasTextChanged;
}

dr_bool32 dred_textview_undo(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    dr_bool32 result;
    dred_control_begin_dirty(DRED_CONTROL(pTextView));
    {
        result = drte_engine_undo(pTextView->pTextEngine);
    }
    dred_control_end_dirty(DRED_CONTROL(pTextView));

    return result;
}

dr_bool32 dred_textview_redo(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    dr_bool32 result;
    dred_control_begin_dirty(DRED_CONTROL(pTextView));
    {
        result =  drte_engine_redo(pTextView->pTextEngine);
    }
    dred_control_end_dirty(DRED_CONTROL(pTextView));

    return result;
}

unsigned int dred_textview_get_undo_points_remaining_count(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    return drte_engine_get_undo_points_remaining_count(pTextView->pTextEngine);
}

unsigned int dred_textview_get_redo_points_remaining_count(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    return drte_engine_get_redo_points_remaining_count(pTextView->pTextEngine);
}

void dred_textview_clear_undo_stack(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    drte_engine_clear_undo_stack(pTextView->pTextEngine);
}


size_t dred_textview_get_cursor_line(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return 0;
    }

    return drte_view_get_cursor_line(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
}

size_t dred_textview_get_cursor_column(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return 0;
    }

    return drte_view_get_cursor_column(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
}

size_t dred_textview_get_line_count(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return 0;
    }

    return drte_view_get_line_count(pTextView->pView);
}


dr_bool32 dred_textview_find_and_select_next(dred_textview* pTextView, const char* text)
{
    if (pTextView == NULL) {
        return 0;
    }

    size_t selectionStart;
    size_t selectionEnd;
    if (drte_view_find_next(pTextView->pView, text, &selectionStart, &selectionEnd))
    {
        drte_view_select(pTextView->pView, selectionStart, selectionEnd);
        drte_view_move_cursor_to_end_of_selection(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));

        return DR_TRUE;
    }

    return DR_FALSE;
}

dr_bool32 dred_textview_find_and_replace_next(dred_textview* pTextView, const char* text, const char* replacement)
{
    if (pTextView == NULL) {
        return 0;
    }

    dr_bool32 wasTextChanged = DR_FALSE;
    drte_engine_prepare_undo_point(pTextView->pTextEngine);
    {
        drte_view_begin_dirty(pTextView->pView);
        {
            drte_view_deselect_all(pTextView->pView);

            size_t selectionStart;
            size_t selectionEnd;
            if (drte_view_find_next(pTextView->pView, text, &selectionStart, &selectionEnd))
            {
                drte_view_select(pTextView->pView, selectionStart, selectionEnd);
                drte_view_move_cursor_to_end_of_selection(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));

                wasTextChanged = dred_textview_delete_selected_text_no_undo(pTextView) || wasTextChanged;
                wasTextChanged = drte_view_insert_text_at_cursor(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), replacement) || wasTextChanged;
            }
        }
        drte_view_end_dirty(pTextView->pView);
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextView->pTextEngine); }
    

    return wasTextChanged;
}

dr_bool32 dred_textview_find_and_replace_all(dred_textview* pTextView, const char* text, const char* replacement)
{
    if (pTextView == NULL) {
        return 0;
    }

    size_t originalCursorLine = drte_view_get_cursor_line(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
    size_t originalCursorPos = drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)) - drte_view_get_line_first_character(pTextView->pView, NULL, originalCursorLine);
    int originalScrollPosX = dred_scrollbar_get_scroll_position(pTextView->pHorzScrollbar);
    int originalScrollPosY = dred_scrollbar_get_scroll_position(pTextView->pVertScrollbar);

    dr_bool32 wasTextChanged = DR_FALSE;
    drte_engine_prepare_undo_point(pTextView->pTextEngine);
    {
        drte_view_begin_dirty(pTextView->pView);
        {
            drte_view_deselect_all(pTextView->pView);

            // It's important that we don't replace the replacement text. To handle this, we just move the cursor to the top of the text and find
            // and replace every occurance without looping.
            drte_view_move_cursor_to_start_of_text(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));

            size_t selectionStart;
            size_t selectionEnd;
            while (drte_view_find_next_no_loop(pTextView->pView, text, &selectionStart, &selectionEnd))
            {
                drte_view_select(pTextView->pView, selectionStart, selectionEnd);
                drte_view_move_cursor_to_end_of_selection(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));

                wasTextChanged = dred_textview_delete_selected_text_no_undo(pTextView) || wasTextChanged;
                wasTextChanged = drte_view_insert_text_at_cursor(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), replacement) || wasTextChanged;
            }

            // The cursor may have moved so we'll need to restore it.
            size_t lineCharStart;
            size_t lineCharEnd;
            drte_view_get_line_character_range(pTextView->pView, NULL, originalCursorLine, &lineCharStart, &lineCharEnd);

            size_t newCursorPos = lineCharStart + originalCursorPos;
            if (newCursorPos > lineCharEnd) {
                newCursorPos = lineCharEnd;
            }
            drte_view_move_cursor_to_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), newCursorPos);
        }
        drte_view_end_dirty(pTextView->pView);
    }
    if (wasTextChanged) { drte_engine_commit_undo_point(pTextView->pTextEngine); }


    // The scroll positions may have moved so we'll need to restore them.
    dred_scrollbar_scroll_to(pTextView->pHorzScrollbar, originalScrollPosX);
    dred_scrollbar_scroll_to(pTextView->pVertScrollbar, originalScrollPosY);

    return wasTextChanged;
}


void dred_textview_show_line_numbers(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    dred_control_show(pTextView->pLineNumbers);
    dred_textview__refresh_line_numbers(pTextView);
}

void dred_textview_hide_line_numbers(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    dred_control_hide(pTextView->pLineNumbers);
    dred_textview__refresh_line_numbers(pTextView);
}


void dred_textview_disable_vertical_scrollbar(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    if (pTextView->isVertScrollbarEnabled) {
        pTextView->isVertScrollbarEnabled = DR_FALSE;
        dred_textview__refresh_scrollbars(pTextView);
    }
}

void dred_textview_enable_vertical_scrollbar(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    if (!pTextView->isVertScrollbarEnabled) {
        pTextView->isVertScrollbarEnabled = DR_TRUE;
        dred_textview__refresh_scrollbars(pTextView);
    }
}

void dred_textview_disable_horizontal_scrollbar(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    if (pTextView->isHorzScrollbarEnabled) {
        pTextView->isHorzScrollbarEnabled = DR_FALSE;
        dred_textview__refresh_scrollbars(pTextView);
    }
}

void dred_textview_enable_horizontal_scrollbar(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    if (!pTextView->isHorzScrollbarEnabled) {
        pTextView->isHorzScrollbarEnabled = DR_TRUE;
        dred_textview__refresh_scrollbars(pTextView);
    }
}

dred_scrollbar* dred_textview_get_vertical_scrollbar(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return NULL;
    }

    return pTextView->pVertScrollbar;
}

dred_scrollbar* dred_textview_get_horizontal_scrollbar(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return NULL;
    }

    return pTextView->pHorzScrollbar;
}

void dred_textview_set_scrollbar_size(dred_textview* pTextView, float size)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->horzScrollbarSize = size;
    pTextView->vertScrollbarSize = size;

    dred_textview__refresh_scrollbars(pTextView);
}

void dred_textview_enable_excess_scrolling(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->isExcessScrollingEnabled = DR_TRUE;

    dred_textview__refresh_scrollbars(pTextView);
}

void dred_textview_disable_excess_scrolling(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->isExcessScrollingEnabled = DR_FALSE;

    dred_textview__refresh_scrollbars(pTextView);
}


dr_bool32 dred_textview_is_line_in_view(dred_textview* pTextView, size_t iLine)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    dr_uint64 scrollPosTop = dred_scrollbar_get_scroll_position(pTextView->pVertScrollbar);
    dr_uint64 scrollPosBot = scrollPosTop + dred_scrollbar_get_page_size(pTextView->pVertScrollbar);
    if (iLine >= scrollPosTop && iLine < scrollPosBot) {
        return DR_TRUE;
    }

    return DR_FALSE;
}

dr_bool32 dred_textview_is_unwrapped_line_in_view(dred_textview* pTextView, size_t iLine)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    // This is silly... need a cleaner way to do line management.
    size_t iWrappedLine = drte_view_get_character_line(pTextView->pView, pTextView->pView->pWrappedLines, drte_view_get_line_first_character(pTextView->pView, pTextView->pView->pEngine->pUnwrappedLines, iLine));

    return dred_textview_is_line_in_view(pTextView, iWrappedLine);
}

dr_bool32 dred_textview_is_cursor_in_view(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;
    }

    return dred_textview_is_line_in_view(pTextView, dred_textview_get_cursor_line(pTextView));
}

void dred_textview_center_on_cursor(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    dred_scrollbar_scroll_to(pTextView->pVertScrollbar, dred_textview_get_cursor_line(pTextView) - (dred_scrollbar_get_page_size(pTextView->pVertScrollbar)/2));
}


void dred_textview_set_tab_size_in_spaces(dred_textview* pTextView, unsigned int tabSizeInSpaces)
{
    if (pTextView == NULL) {
        return;
    }

    drte_view_set_tab_size(pTextView->pView, tabSizeInSpaces);
}

unsigned int dred_textview_get_tab_size_in_spaces(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return 0;
    }

    return drte_view_get_tab_size(pTextView->pView);
}


void dred_textview_enable_tabs_to_spaces(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->isTabsToSpacesEnabled = DR_TRUE;
}

void dred_textview_disable_tabs_to_spaces(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->isTabsToSpacesEnabled = DR_FALSE;
}

dr_bool32 dred_textview_is_tabs_to_spaces_enabled(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;;
    }

    return pTextView->isTabsToSpacesEnabled;
}


dr_bool32 dred_textview_prepare_undo_point(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;;
    }

    return drte_engine_prepare_undo_point(pTextView->pTextEngine);
}

dr_bool32 dred_textview_commit_undo_point(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return DR_FALSE;;
    }

    return drte_engine_commit_undo_point(pTextView->pTextEngine);
}


void dred_textview_set_on_cursor_move(dred_textview* pTextView, dred_textview_on_cursor_move_proc proc)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->onCursorMove = proc;
}

void dred_textview_set_on_undo_point_changed(dred_textview* pTextView, dred_textview_on_undo_point_changed_proc proc)
{
    if (pTextView == NULL) {
        return;
    }

    pTextView->onUndoPointChanged = proc;
}


void dred_textview_on_size(dred_control* pControl, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    dred_textview* pTextView = DRED_TEXTVIEW(pControl);
    if (pTextView == NULL) {
        return;
    }

    // The text engine needs to be resized.
    float containerWidth;
    float containerHeight;
    dred_textview__calculate_text_engine_container_size(pTextView, &containerWidth, &containerHeight);

    if (containerWidth != newWidth || containerHeight != newHeight) {
        drte_view_set_size(pTextView->pView, containerWidth, containerHeight);

        // Scrollbars need to be refreshed first.
        dred_textview__refresh_scrollbars(pTextView);

        // Line numbers need to be refreshed.
        dred_textview__refresh_line_numbers(pTextView);
    }
}

void dred_textview__select_rectangle(dred_textview* pTextView, drte_rect rect)
{
    if (pTextView == NULL) return;

    // Disable the onCursorMove callback while we're constructing the sub-selections. It's restored at the end.
    drte_engine_on_cursor_move_proc prevOnCursorMoveProc = pTextView->pTextEngine->onCursorMove;
    pTextView->pTextEngine->onCursorMove = NULL;

    size_t iLineBeg = drte_view_get_line_at_pos_y(pTextView->pView, pTextView->pView->pWrappedLines, rect.top);
    size_t iLineEnd = drte_view_get_line_at_pos_y(pTextView->pView, pTextView->pView->pWrappedLines, rect.bottom);  // <-- Inclusive.
    for (size_t iLine = iLineBeg; iLine <= iLineEnd; ++iLine) {
        // Restore the onCursorMove callback for the last cursor.
        if (iLine == iLineEnd) {
            pTextView->pTextEngine->onCursorMove = prevOnCursorMoveProc;
        }

        float linePosY = iLine * drte_engine_get_line_height(pTextView->pTextEngine);
        
        size_t iCharBeg;
        drte_view_get_character_under_point_relative_to_text(pTextView->pView, pTextView->pView->pWrappedLines, rect.left,  linePosY, &iCharBeg, NULL);

        size_t iCharEnd;
        drte_view_get_character_under_point_relative_to_text(pTextView->pView, pTextView->pView->pWrappedLines, rect.right, linePosY, &iCharEnd, NULL);

        drte_view_begin_selection(pTextView->pView, iCharBeg);
        drte_view_set_selection_end_point(pTextView->pView, iCharEnd);

        // Place a cursor at the end of the selection.
        dred_textview__insert_cursor(pTextView, iCharEnd, iLine);
    }

    pTextView->pTextEngine->onCursorMove = prevOnCursorMoveProc;
}

void dred_textview_on_mouse_move(dred_control* pControl, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_textview* pTextView = DRED_TEXTVIEW(pControl);
    if (pTextView == NULL) {
        return;
    }

    float offsetX;
    float offsetY;
    dred_textview__get_text_offset(pTextView, &offsetX, &offsetY);

    float mousePosXRelativeToTextArea = (float)relativeMousePosX - offsetX;
    float mousePosYRelativeToTextArea = (float)relativeMousePosY - offsetY;

    if (dred_gui_get_element_with_mouse_capture(dred_control_get_gui(pControl)) == pControl)
    {
        drte_view_begin_dirty(pTextView->pView);
        {
            size_t iPrevCursorChar = drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));

            if (pTextView->isDoingWordSelect) {
                size_t iWordCharBeg;
                size_t iWordCharEnd;
                if (drte_view_get_word_under_point(pTextView->pView, relativeMousePosX - offsetX, relativeMousePosY - offsetY, &iWordCharBeg, &iWordCharEnd)) {
                    if (iWordCharEnd < pTextView->wordSelectionAnchor.iCharEnd) {
                        pTextView->pView->pSelections[pTextView->pView->selectionCount-1].iCharBeg = pTextView->wordSelectionAnchor.iCharEnd;
                        pTextView->pView->pSelections[pTextView->pView->selectionCount-1].iCharEnd = iWordCharBeg;
                    } else {
                        pTextView->pView->pSelections[pTextView->pView->selectionCount-1].iCharBeg = pTextView->wordSelectionAnchor.iCharBeg;
                        pTextView->pView->pSelections[pTextView->pView->selectionCount-1].iCharEnd = iWordCharEnd;
                    }

                    drte_view_move_cursor_to_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), pTextView->pView->pSelections[pTextView->pView->selectionCount-1].iCharEnd);
                } else {
                    // There is no word under the point, so just fall back to standard character selection for this case.
                    drte_view_move_cursor_to_point(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), mousePosXRelativeToTextArea, mousePosYRelativeToTextArea);
                }
            } else {
                if (pTextView->isDoingRectangleSelect) {
                    if ((stateFlags & DRED_GUI_KEY_STATE_ALT_DOWN) != 0) {
                        // We're doing rectangle selection.
                        pTextView->selectionRect.right  = mousePosXRelativeToTextArea - pTextView->pView->innerOffsetX;
                        pTextView->selectionRect.bottom = mousePosYRelativeToTextArea - pTextView->pView->innerOffsetY;

                        drte_view_deselect_all(pTextView->pView);
                        dred_textview__clear_all_cursors(pTextView);
                        dred_textview__select_rectangle(pTextView, drte_rect_make_right_way_out(pTextView->selectionRect));
                    } else {
                        pTextView->isDoingRectangleSelect = DR_FALSE;  // Mouse movement was detected while the Alt key was not held down. Cancel the rectangle selection.
                    }
                } else {
                    drte_view_move_cursor_to_point(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), mousePosXRelativeToTextArea, mousePosYRelativeToTextArea);
                }
            }

            size_t iNextCursorChar = drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
            if (iPrevCursorChar != iNextCursorChar) {
                drte_view_set_selection_end_point(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));
            }
        }
        drte_view_end_dirty(pTextView->pView);
    }
    else
    {
        // If we get to this branch it means we are not selecting anything. If we are hovered over a selected piece of text we will want to change
        // the cursor to an arrow for drag-and-drop feedback.
        if (dred_textview_is_drag_and_drop_enabled(pTextView)) {
            size_t iHoveredSelection;
            if (drte_view_get_selection_under_point(pTextView->pView, mousePosXRelativeToTextArea, mousePosYRelativeToTextArea, &iHoveredSelection)) {
                if (pTextView->isWantingToDragAndDrop) {
                    //printf("Begin dragging...\n");
                    dred_begin_drag_and_drop(dred_data_type_text, "Hello!", strlen("Hello")+1);
                    pTextView->isWantingToDragAndDrop = DR_FALSE;
                }

                dred_control_set_cursor(DRED_CONTROL(pTextView), dred_cursor_type_arrow);
            } else {
                dred_control_set_cursor(DRED_CONTROL(pTextView), dred_cursor_type_text);
            }
        }
    }
}

void dred_textview_on_mouse_button_down(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    dred_textview* pTextView = DRED_TEXTVIEW(pControl);
    if (pTextView == NULL) {
        return;
    }

    // Focus the text editor.
    dred_gui_capture_keyboard(pControl);

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT)
    {
        float offsetX;
        float offsetY;
        dred_textview__get_text_offset(pTextView, &offsetX, &offsetY);

        float mousePosXRelativeToTextArea = (float)relativeMousePosX - offsetX;
        float mousePosYRelativeToTextArea = (float)relativeMousePosY - offsetY;

        size_t iSelection;
        if (dred_textview_is_drag_and_drop_enabled(pTextView) && drte_view_get_selection_under_point(pTextView->pView, mousePosXRelativeToTextArea, mousePosYRelativeToTextArea, &iSelection)) {
            pTextView->isWantingToDragAndDrop = DR_TRUE;
        }


        if (!pTextView->isWantingToDragAndDrop) {
            pTextView->isDoingWordSelect = DR_FALSE;

            if ((stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) != 0) {
                if (!drte_view_is_anything_selected(pTextView->pView)) {
                    drte_view_begin_selection(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));
                }
            } else {
                if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) == 0 || (stateFlags & DRED_GUI_KEY_STATE_ALT_DOWN) != 0) {
                    drte_view_deselect_all(pTextView->pView);
                    dred_textview__clear_all_cursors_except_last(pTextView);
                }
            }

            size_t iLine;
            size_t iChar;
            drte_view_get_character_under_point(pTextView->pView, NULL, mousePosXRelativeToTextArea, mousePosYRelativeToTextArea, &iChar, &iLine);

            if ((stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) != 0) {
                drte_view_set_selection_end_point(pTextView->pView, iChar);
            } else {
                if ((stateFlags & DRED_GUI_KEY_STATE_ALT_DOWN) != 0) {
                    pTextView->isDoingRectangleSelect = DR_TRUE;
                    pTextView->selectionRect.left = pTextView->selectionRect.right = mousePosXRelativeToTextArea - pTextView->pView->innerOffsetX;
                    pTextView->selectionRect.top = pTextView->selectionRect.bottom = mousePosYRelativeToTextArea - pTextView->pView->innerOffsetY;
                } else {
                    if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) != 0) {
                        dred_textview__insert_cursor(pTextView, iChar, iLine);
                    }
                }

                drte_view_begin_selection(pTextView->pView, iChar);
            }


            drte_view_move_cursor_to_character_and_line(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), iChar, iLine);
            drte_view__update_cursor_sticky_position(pTextView->pView, &pTextView->pView->pCursors[drte_view_get_last_cursor(pTextView->pView)]);


            // In order to support selection with the mouse we need to capture the mouse and enter selection mode.
            dred_gui_capture_mouse(pControl);
        }
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_RIGHT)
    {
    }
}

void dred_textview_on_mouse_button_up(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    dred_textview* pTextView = DRED_TEXTVIEW(pControl);
    if (pTextView == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT)
    {
        pTextView->isDoingRectangleSelect = DR_FALSE;

        if (dred_gui_get_element_with_mouse_capture(pControl->pGUI) == pControl)
        {
            // When we first pressed the mouse we may have started a new selection. If we never ended up selecting anything we'll want to
            // cancel that selection.
            size_t iCharBeg;
            size_t iCharEnd;
            if (drte_view_get_last_selection(pTextView->pView, &iCharBeg, &iCharEnd)) {
                if (iCharBeg == iCharEnd) {
                    drte_view_cancel_last_selection(pTextView->pView);
                }
            }

            // Releasing the mouse will leave selectionmode.
            dred_gui_release_mouse(pControl->pGUI);
        }

        if (pTextView->isWantingToDragAndDrop) {
            drte_view_deselect_all(pTextView->pView);

            float offsetX;
            float offsetY;
            dred_textview__get_text_offset(pTextView, &offsetX, &offsetY);

            float mousePosXRelativeToTextArea = (float)relativeMousePosX - offsetX;
            float mousePosYRelativeToTextArea = (float)relativeMousePosY - offsetY;

            size_t iLine;
            size_t iChar;
            drte_view_get_character_under_point(pTextView->pView, NULL, mousePosXRelativeToTextArea, mousePosYRelativeToTextArea, &iChar, &iLine);

            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) != 0) {
                dred_textview__insert_cursor(pTextView, iChar, iLine);
            }

            drte_view_move_cursor_to_character_and_line(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), iChar, iLine);
            drte_view__update_cursor_sticky_position(pTextView->pView, &pTextView->pView->pCursors[drte_view_get_last_cursor(pTextView->pView)]);
        }

        pTextView->isWantingToDragAndDrop = DR_FALSE;
    }
}

void dred_textview_on_mouse_button_dblclick(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)mouseButton;
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_textview* pTextView = DRED_TEXTVIEW(pControl);
    if (pTextView == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if ((stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) == 0) {
            // If the control key is not being held down make sure other selection regions are cleared.
            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) == 0) {
                drte_view_deselect_all(pTextView->pView);
            }

            drte_view_select_word_under_cursor(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));

            if (pTextView->pView->selectionCount > 0) {

            }

            size_t iCharBeg;
            size_t iCharEnd;
            if (drte_view_get_last_selection(pTextView->pView, &iCharBeg, &iCharEnd)) {
                drte_view_move_cursor_to_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), iCharEnd);

                pTextView->isDoingWordSelect = DR_TRUE;
                pTextView->wordSelectionAnchor.iCharBeg = iCharBeg;
                pTextView->wordSelectionAnchor.iCharEnd = iCharEnd;
            }
        }
    }
}

void dred_textview_on_mouse_wheel(dred_control* pControl, int delta, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_textview* pTextView = DRED_TEXTVIEW(pControl);
    if (pTextView == NULL) {
        return;
    }

    dred_scrollbar_scroll(pTextView->pVertScrollbar, -delta * dred_scrollbar_get_mouse_wheel_scale(pTextView->pVertScrollbar));
}

void dred_textview__move_cursor_left(dred_textview* pTextView, size_t iCursor, int stateFlags)
{
    assert(pTextView != NULL);

    if (stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) {
        drte_view_move_cursor_to_start_of_word(pTextView->pView, iCursor);
    } else {
        drte_view_move_cursor_left(pTextView->pView, iCursor);
    }
}

void dred_textview__move_cursor_right(dred_textview* pTextView, size_t iCursor, int stateFlags)
{
    assert(pTextView != NULL);

    if (stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) {
        drte_view_move_cursor_to_start_of_next_word(pTextView->pView, iCursor);
    } else {
        drte_view_move_cursor_right(pTextView->pView, iCursor);
    }
}

void dred_textview_on_key_down(dred_control* pControl, dred_key key, int stateFlags)
{
    dred_textview* pTextView = DRED_TEXTVIEW(pControl);
    if (pTextView == NULL) {
        return;
    }

    drte_view_begin_dirty(pTextView->pView);

    dr_bool32 isShiftDown = (stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) != 0;
    //dr_bool32 isCtrlDown  = (stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) != 0;

    size_t iLastCursor = drte_view_get_last_cursor(pTextView->pView);
    size_t iLastCursorLine = drte_view_get_cursor_line(pTextView->pView, iLastCursor);
    size_t iLastCursorChar = drte_view_get_cursor_character(pTextView->pView, iLastCursor);

    switch (key)
    {
        case DRED_GUI_BACKSPACE:
        {
            dr_bool32 wasTextChanged = DR_FALSE;
            drte_engine_prepare_undo_point(pTextView->pTextEngine);
            {
                if (drte_view_is_anything_selected(pTextView->pView)) {
                    wasTextChanged = dred_textview_delete_selected_text_no_undo(pTextView);
                    drte_view__update_cursor_sticky_position(pTextView->pView, &pTextView->pView->pCursors[iLastCursor]);
                    drte_view_remove_overlapping_cursors(pTextView->pView);
                } else {
                    drte_view_begin_dirty(pTextView->pView);
                    {
                        for (size_t iCursor = 0; iCursor < pTextView->pView->cursorCount; ++iCursor) {
                            size_t iCursorChar = pTextView->pView->pCursors[iCursor].iCharAbs;
                            if (iCursorChar == 0) {
                                continue;
                            }

                            dr_bool32 leaveNewLines = pTextView->pView->cursorCount > 1;
                            if (leaveNewLines) {
                                size_t iLineCharBeg = drte_view_get_line_first_character(pTextView->pView, pTextView->pView->pWrappedLines, drte_view_get_cursor_line(pTextView->pView, iCursor));
                                if (iCursorChar == iLineCharBeg) {
                                    continue;
                                }
                            }

                            size_t iCharEnd = drte_view_get_cursor_character(pTextView->pView, iCursor);
                            dred_textview__move_cursor_left(pTextView, iCursor, stateFlags);
                            size_t iCharBeg = drte_view_get_cursor_character(pTextView->pView, iCursor);

                            if (iCharEnd == iCharBeg) {
                                continue;   // Nothing to delete.
                            }

                            wasTextChanged = drte_engine_delete_text(pTextView->pTextEngine, iCharBeg, iCharEnd);
                        }
                    }
                    drte_view_end_dirty(pTextView->pView);
                }
            }
            if (wasTextChanged) { drte_engine_commit_undo_point(pTextView->pTextEngine); }
        } break;

        case DRED_GUI_DELETE:
        {
            dr_bool32 wasTextChanged = DR_FALSE;
            drte_engine_prepare_undo_point(pTextView->pTextEngine);
            {
                if (drte_view_is_anything_selected(pTextView->pView)) {
                    wasTextChanged = dred_textview_delete_selected_text_no_undo(pTextView);
                    drte_view__update_cursor_sticky_position(pTextView->pView, &pTextView->pView->pCursors[iLastCursor]);
                    drte_view_remove_overlapping_cursors(pTextView->pView);
                } else {
                    drte_view_begin_dirty(pTextView->pView);
                    {
                        for (size_t iCursor = 0; iCursor < pTextView->pView->cursorCount; ++iCursor) {
                            size_t iCharBeg = drte_view_get_cursor_character(pTextView->pView, iCursor);
                            dred_textview__move_cursor_right(pTextView, iCursor, stateFlags);
                            size_t iCharEnd = drte_view_get_cursor_character(pTextView->pView, iCursor);

                            if (iCharEnd == iCharBeg) {
                                continue;   // Nothing to delete.
                            }

                            wasTextChanged = drte_engine_delete_text(pTextView->pTextEngine, iCharBeg, iCharEnd);
                            drte_view__update_cursor_sticky_position(pTextView->pView, &pTextView->pView->pCursors[iCursor]);
                        }
                    }
                    drte_view_end_dirty(pTextView->pView);
                }
            }
            if (wasTextChanged) { drte_engine_commit_undo_point(pTextView->pTextEngine); }
        } break;


        case DRED_GUI_ARROW_LEFT:
        {
            if (drte_view_is_anything_selected(pTextView->pView) && !isShiftDown) {
                dred_textview__move_cursor_to_start_of_selection(pTextView, NULL);
                drte_view__update_cursor_sticky_position(pTextView->pView, &pTextView->pView->pCursors[iLastCursor]);
                dred_textview_deselect_all(pTextView);
            } else {
                if ((!drte_view_is_anything_selected(pTextView->pView) || !dred_textview__is_cursor_on_selection(pTextView)) && isShiftDown) {
                    drte_view_begin_selection(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));
                }

                dred_textview__move_cursor_left(pTextView, iLastCursor, stateFlags);

                if (isShiftDown) {
                    drte_view_set_selection_end_point(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));

                    size_t iSelCharBeg;
                    size_t iSelCharEnd;
                    if (drte_view_get_last_selection(pTextView->pView, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                        drte_view_cancel_last_selection(pTextView->pView);
                    }
                }
            }
        } break;

        case DRED_GUI_ARROW_RIGHT:
        {
            if (drte_view_is_anything_selected(pTextView->pView) && !isShiftDown) {
                dred_textview__move_cursor_to_end_of_selection(pTextView, NULL);
                drte_view__update_cursor_sticky_position(pTextView->pView, &pTextView->pView->pCursors[iLastCursor]);
                dred_textview_deselect_all(pTextView);
            } else {
                if ((!drte_view_is_anything_selected(pTextView->pView) || !dred_textview__is_cursor_on_selection(pTextView)) && isShiftDown) {
                    drte_view_begin_selection(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));
                }

                dred_textview__move_cursor_right(pTextView, iLastCursor, stateFlags);

                if (isShiftDown) {
                    drte_view_set_selection_end_point(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));

                    size_t iSelCharBeg;
                    size_t iSelCharEnd;
                    if (drte_view_get_last_selection(pTextView->pView, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                        drte_view_cancel_last_selection(pTextView->pView);
                    }
                }
            }
        } break;

        case DRED_GUI_ARROW_UP:
        {
            if (drte_view_is_anything_selected(pTextView->pView) && !isShiftDown) {
                dred_textview_deselect_all(pTextView);
            } else {
                if ((!drte_view_is_anything_selected(pTextView->pView) || !dred_textview__is_cursor_on_selection(pTextView)) && isShiftDown) {
                    drte_view_begin_selection(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));
                }
            }

            drte_view_move_cursor_up(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));

            if (isShiftDown) {
                drte_view_set_selection_end_point(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_view_get_last_selection(pTextView->pView, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_view_cancel_last_selection(pTextView->pView);
                }
            }
        } break;

        case DRED_GUI_ARROW_DOWN:
        {
            if (drte_view_is_anything_selected(pTextView->pView) && !isShiftDown) {
                dred_textview_deselect_all(pTextView);
            } else {
                if ((!drte_view_is_anything_selected(pTextView->pView) || !dred_textview__is_cursor_on_selection(pTextView)) && isShiftDown) {
                    drte_view_begin_selection(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));
                }
            }

            drte_view_move_cursor_down(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));

            if (isShiftDown) {
                drte_view_set_selection_end_point(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_view_get_last_selection(pTextView->pView, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_view_cancel_last_selection(pTextView->pView);
                }
            }
        } break;


        case DRED_GUI_END:
        {
            if (drte_view_is_anything_selected(pTextView->pView) && !isShiftDown) {
                dred_textview_deselect_all(pTextView);
            } else {
                if ((!drte_view_is_anything_selected(pTextView->pView) || !dred_textview__is_cursor_on_selection(pTextView)) && isShiftDown) {
                    drte_view_begin_selection(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));
                }
            }

            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) != 0) {
                drte_view_move_cursor_to_end_of_text(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
            } else {
                if (drte_view_is_cursor_at_end_of_wrapped_line(pTextView->pView, drte_view_get_last_cursor(pTextView->pView))) {
                    drte_view_move_cursor_to_end_of_unwrapped_line(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
                } else {
                    drte_view_move_cursor_to_end_of_line(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
                }
            }

            if (isShiftDown) {
                drte_view_set_selection_end_point(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_view_get_last_selection(pTextView->pView, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_view_cancel_last_selection(pTextView->pView);
                }
            }

            drte_view__update_cursor_sticky_position(pTextView->pView, &pTextView->pView->pCursors[drte_view_get_last_cursor(pTextView->pView)]);
        } break;

        case DRED_GUI_HOME:
        {
            if (drte_view_is_anything_selected(pTextView->pView) && !isShiftDown) {
                dred_textview_deselect_all(pTextView);
            } else {
                if ((!drte_view_is_anything_selected(pTextView->pView) || !dred_textview__is_cursor_on_selection(pTextView)) && isShiftDown) {
                    drte_view_begin_selection(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, iLastCursor));
                }
            }

            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) != 0) {
                drte_view_move_cursor_to_start_of_text(pTextView->pView, iLastCursor);
            } else {
                size_t iNewChar = iLastCursorChar;

                drte_line_cache* pLineCache;
                if (drte_view_is_cursor_at_start_of_wrapped_line(pTextView->pView, iLastCursor)) {
                    pLineCache = pTextView->pTextEngine->pUnwrappedLines;
                } else {
                    pLineCache = pTextView->pView->pWrappedLines;
                }

                size_t iFirstChar = drte_view_get_line_first_character(pTextView->pView, pLineCache, iLastCursorLine);
                size_t iFirstNonWhitespaceChar = drte_view_get_line_first_non_whitespace_character(pTextView->pView, pLineCache, iLastCursorLine);                    
                if (iLastCursorChar == iFirstNonWhitespaceChar) {
                    iNewChar = iFirstChar;
                } else {
                    iNewChar = iFirstNonWhitespaceChar;
                }

                drte_view_move_cursor_to_character(pTextView->pView, iLastCursor, iNewChar);
            }

            if (isShiftDown) {
                drte_view_set_selection_end_point(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, iLastCursor));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_view_get_last_selection(pTextView->pView, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_view_cancel_last_selection(pTextView->pView);
                }
            }

            drte_view__update_cursor_sticky_position(pTextView->pView, &pTextView->pView->pCursors[iLastCursor]);
        } break;

        case DRED_GUI_PAGE_UP:
        {
            if (drte_view_is_anything_selected(pTextView->pView) && !isShiftDown) {
                dred_textview_deselect_all(pTextView);
            } else {
                if ((!drte_view_is_anything_selected(pTextView->pView) || !dred_textview__is_cursor_on_selection(pTextView)) && isShiftDown) {
                    drte_view_begin_selection(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));
                }
            }

            int scrollOffset = dred_scrollbar_get_page_size(pTextView->pVertScrollbar);
            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) == 0) {
                dred_scrollbar_scroll(pTextView->pVertScrollbar, -scrollOffset);
            }

            drte_view_move_cursor_y(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), -dred_scrollbar_get_page_size(pTextView->pVertScrollbar));

            if (isShiftDown) {
                drte_view_set_selection_end_point(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_view_get_last_selection(pTextView->pView, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_view_cancel_last_selection(pTextView->pView);
                }
            }
        } break;

        case DRED_GUI_PAGE_DOWN:
        {
            if (drte_view_is_anything_selected(pTextView->pView) && !isShiftDown) {
                dred_textview_deselect_all(pTextView);
            } else {
                if ((!drte_view_is_anything_selected(pTextView->pView) || !dred_textview__is_cursor_on_selection(pTextView)) && isShiftDown) {
                    drte_view_begin_selection(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));
                }
            }

            int scrollOffset = dred_scrollbar_get_page_size(pTextView->pVertScrollbar);
            if (scrollOffset > (int)(drte_view_get_line_count(pTextView->pView) - drte_view_get_cursor_line(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)))) {
                scrollOffset = 0;
            }

            if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) == 0) {
                dred_scrollbar_scroll(pTextView->pVertScrollbar, scrollOffset);
            }

            drte_view_move_cursor_y(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), dred_scrollbar_get_page_size(pTextView->pVertScrollbar));

            if (isShiftDown) {
                drte_view_set_selection_end_point(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));

                size_t iSelCharBeg;
                size_t iSelCharEnd;
                if (drte_view_get_last_selection(pTextView->pView, &iSelCharBeg, &iSelCharEnd) && iSelCharBeg == iSelCharEnd) {
                    drte_view_cancel_last_selection(pTextView->pView);
                }
            }

        } break;

        default: break;
    }

    drte_view_end_dirty(pTextView->pView);
}

void dred_textview_on_key_up(dred_control* pControl, dred_key key, int stateFlags)
{
    (void)pControl;
    (void)key;
    (void)stateFlags;
}

void dred_textview_on_printable_key_down(dred_control* pControl, unsigned int utf32, int stateFlags)
{
    (void)stateFlags;

    dred_textview* pTextView = DRED_TEXTVIEW(pControl);
    if (pTextView == NULL) {
        return;
    }

    dred_control_begin_dirty(pControl);
    drte_engine_prepare_undo_point(pTextView->pTextEngine);
    {
        drte_view_begin_dirty(pTextView->pView);
        if (utf32 == '\t') {
            // The tab key is a complex case because it can be handled differently depending on the configuration:
            //   - If multiple lines are selected, they need to be block-indented
            //     - Otherwise they need to be inserted like any other character, unless...
            //   - If tabs-to-spaces is enabled, tabs need to be converted to spaces.
            //
            // The tab key is handled for each cursor.
            for (size_t iCursor = 0; iCursor < pTextView->cursorCount; ++iCursor) {
                dr_bool32 isDoingBlockIndent = DR_FALSE;

                size_t iSelection;
                dr_bool32 isSomethingSelected = dred_textview__get_cursor_selection(pTextView, iCursor, &iSelection);
                if (isSomethingSelected) {
                    //isDoingBlockIndent = drte_view_get_selection_first_line(pTextView->pView, iSelection) != drte_view_get_selection_last_line(pTextView->pView, iSelection);
                    isDoingBlockIndent = DR_TRUE;
                }

                if (isDoingBlockIndent) {
                    // A block indent is done by simply inserting a tab at the beginning of each selected line.
                    size_t iLineBeg = drte_view_get_selection_first_line(pTextView->pView, iSelection);
                    size_t iLineEnd = drte_view_get_selection_last_line(pTextView->pView, iSelection);

                    for (size_t iLine = iLineBeg; iLine <= iLineEnd; ++iLine) {
                        dred_textview__insert_tab(pTextView, drte_view_get_line_first_character(pTextView->pView, NULL, iLine));
                    }
                } else {
                    // We're not doing a block indent so we just insert a tab at the cursor like normal.
                    if (isSomethingSelected) {
                        drte_view_delete_selection_text(pTextView->pView, iSelection);
                        drte_view_cancel_selection(pTextView->pView, iSelection);
                    }

                    dred_textview__insert_tab_at_cursor(pTextView, iCursor);
                }
            }
        } else {
            if (drte_view_is_anything_selected(pTextView->pView)) {
                dred_textview_delete_selected_text_no_undo(pTextView);
            }

            // Convert to line endings.
            if (utf32 == '\r') {
                utf32 = '\n';
            }

            // TODO: Check if the character is a line feed, and if so convert it to the standard line endings.

            drte_view_insert_character_at_cursors(pTextView->pView, utf32);

            if (utf32 == '\n') {
                dred_context* pDred = dred_control_get_context(DRED_CONTROL(pTextView));
                if (pDred->config.textEditorEnableAutoIndent) {
                    drte_view_begin_dirty(pTextView->pView);
                    for (size_t iCursor = 0; iCursor < pTextView->pView->cursorCount; ++iCursor) {
                        size_t iCursorChar = pTextView->pView->pCursors[iCursor].iCharAbs;
                        size_t iCursorLine = drte_view_get_character_line(pTextView->pView, NULL, iCursorChar);
                        if (iCursorLine > 0) {
                            size_t iPrevLineCharBeg;
                            size_t iPrevLineCharEnd;
                            drte_view_get_line_character_range(pTextView->pView, NULL, iCursorLine-1, &iPrevLineCharBeg, &iPrevLineCharEnd);

                            size_t indentationCount = 0;
                            for (;;) {
                                if (iPrevLineCharBeg == iPrevLineCharEnd) {
                                    break;  // End of line.
                                }

                                uint32_t c = drte_engine_get_utf32(pTextView->pTextEngine, iPrevLineCharBeg);
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
                            if (pTextView->isTabsToSpacesEnabled) {
                                extraSpacesCount += extraTabCount * pDred->config.textEditorTabSizeInSpaces;
                                extraTabCount = 0;
                            }

                            for (size_t i = 0; i < extraTabCount; ++i) {
                                drte_view_insert_character_at_cursor(pTextView->pView, iCursor, '\t');
                            }
                            for (size_t i = 0; i < extraSpacesCount; ++i) {
                                drte_view_insert_character_at_cursor(pTextView->pView, iCursor, ' ');
                            }
                        }
                    }
                    drte_view_end_dirty(pTextView->pView);
                }
            }
        }
        drte_view_end_dirty(pTextView->pView);
    }
    drte_engine_commit_undo_point(pTextView->pTextEngine);
    dred_control_end_dirty(pControl);
}


void dred_textview_engine__on_paint_rect(drte_engine* pTextEngine, drte_view* pView, drte_style_token styleToken, drte_rect rect, void* pPaintData)
{
    (void)pTextEngine;

    dred_textview* pTextView = (dred_textview*)pView->pUserData;
    dred_text_style* pStyle = (dred_text_style*)styleToken;

    float offsetX;
    float offsetY;
    dred_textview__get_text_offset(pTextView, &offsetX, &offsetY);

    dred_control_draw_rect(DRED_CONTROL(pTextView), dred_offset_rect(drte_rect_to_dred(rect), offsetX, offsetY), pStyle->bgColor, pPaintData);
}

void dred_textview_engine__on_paint_text(drte_engine* pTextEngine, drte_view* pView, drte_style_token styleTokenFG, drte_style_token styleTokenBG, const char* text, size_t textLength, float posX, float posY, void* pPaintData)
{
    (void)pTextEngine;

    dred_textview* pTextView = (dred_textview*)pView->pUserData;

    dred_text_style* pStyleFG = (dred_text_style*)styleTokenFG;
    dred_text_style* pStyleBG = (dred_text_style*)styleTokenBG;

    float offsetX;
    float offsetY;
    dred_textview__get_text_offset(pTextView, &offsetX, &offsetY);

    dred_control_draw_text(DRED_CONTROL(pTextView), pStyleFG->pFont, text, (int)textLength, posX + offsetX, posY + offsetY, pStyleFG->fgColor, pStyleBG->bgColor, pPaintData);
}

void dred_textview_engine__on_dirty(drte_engine* pTextEngine, drte_view* pView, drte_rect rect)
{
    (void)pTextEngine;

    dred_textview* pTextView = (dred_textview*)pView->pUserData;
    if (pTextView == NULL) {
        return;
    }

    float offsetX;
    float offsetY;
    dred_textview__get_text_offset(pTextView, &offsetX, &offsetY);

    dred_control_dirty(DRED_CONTROL(pTextView), dred_offset_rect(drte_rect_to_dred(rect), offsetX, offsetY));
}

void dred_textview_engine__on_cursor_move(drte_engine* pTextEngine, drte_view* pView, size_t iCursor)
{
    (void)pTextEngine;

    // We only care about the last cursor.
    if (iCursor != pView->cursorCount-1) {
        return;
    }

    // If the cursor is off the edge of the container we want to scroll it into position.
    dred_textview* pTextView = (dred_textview*)pView->pUserData;
    if (pTextView == NULL) {
        return;
    }

    // If the cursor is above or below the container, we need to scroll vertically.
    int iLine = (int)drte_view_get_cursor_line(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
    if (iLine < dred_scrollbar_get_scroll_position(pTextView->pVertScrollbar)) {
        dred_scrollbar_scroll_to(pTextView->pVertScrollbar, iLine);
    }

    int iBottomLine = dred_scrollbar_get_scroll_position(pTextView->pVertScrollbar) + dred_scrollbar_get_page_size(pTextView->pVertScrollbar) - 1;
    if (iLine >= iBottomLine) {
        dred_scrollbar_scroll_to(pTextView->pVertScrollbar, iLine - (dred_scrollbar_get_page_size(pTextView->pVertScrollbar) - 1) + 1);
    }


    // If the cursor is to the left or right of the container we need to scroll horizontally.
    float cursorPosX;
    float cursorPosY;
    drte_view_get_cursor_position(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), &cursorPosX, &cursorPosY);

    float cursorWidth = drte_view_get_cursor_width(pTextView->pView);

    if (cursorPosX < 0) {
        dred_scrollbar_scroll_to(pTextView->pHorzScrollbar, (int)(cursorPosX - drte_view_get_inner_offset_x(pTextView->pView)));
    }
    if (cursorPosX >= drte_view_get_size_x(pTextView->pView)) {
        dred_scrollbar_scroll_to(pTextView->pHorzScrollbar, (int)(cursorPosX - drte_view_get_inner_offset_x(pTextView->pView) - drte_view_get_size_x(pTextView->pView)) + (int)cursorWidth);
    }


    if (pTextView->onCursorMove) {
        pTextView->onCursorMove(pTextView);
    }
}


void dred_textview__on_text_changed(dred_textview* pTextView)
{
    // Scrollbars need to be refreshed whenever text is changed.
    dred_textview__refresh_scrollbars(pTextView);

    // The line numbers need to be redrawn.
    // TODO: This can probably be optimized a bit so that it is only redrawn if a line was inserted or deleted.
    dred_control_dirty(pTextView->pLineNumbers, dred_control_get_local_rect(pTextView->pLineNumbers));
}

size_t dred_textview__on_get_undo_state(dred_textview* pTextView, void* pDataOut)
{
    //printf("Getting Undo State...\n");

    if (pDataOut != NULL) {
        *((size_t*)pDataOut) = pTextView->cursorCount;

        dred_textview_cursor* pCursorOut = (dred_textview_cursor*)((uint8_t*)pDataOut + sizeof(pTextView->cursorCount));
        for (size_t i = 0; i < pTextView->cursorCount; ++i) {
            pCursorOut[i] = pTextView->pCursors[i];
        }
    }

    return sizeof(pTextView->cursorCount) + (sizeof(*pTextView->pCursors) * pTextView->cursorCount);
}

void dred_textview__on_apply_undo_state(dred_textview* pTextView, size_t dataSize, const void* pData)
{
    //printf("Applying Undo State...\n");

    assert(pData != NULL);
    (void)dataSize;

    size_t cursorCount = *((const size_t*)pData);
    const dred_textview_cursor* pCursors = (const dred_textview_cursor*)((const uint8_t*)pData + sizeof(pTextView->cursorCount));

    if (cursorCount > pTextView->cursorCount) {
        dred_textview_cursor* pNewCursors = (dred_textview_cursor*)realloc(pTextView->pCursors, cursorCount * sizeof(*pTextView->pCursors));
        if (pNewCursors == NULL) {
            return;
        }

        pTextView->pCursors = pNewCursors;
    }

    memcpy(pTextView->pCursors, pCursors, cursorCount * sizeof(*pTextView->pCursors));
    pTextView->cursorCount = cursorCount;
}



void dred_textview_on_paint(dred_control* pControl, dred_rect relativeRect, void* pPaintData)
{
    dred_textview* pTextView = DRED_TEXTVIEW(pControl);
    if (pTextView == NULL) {
        return;
    }

    dred_rect textRect = dred_textview__get_text_rect(pTextView);

    // The dead space between the scrollbars should always be drawn with the default background color.
    //dred_control_draw_rect(pTextView, dred_textview__get_scrollbar_dead_space_rect(pTextView), pTextView->defaultStyle.bgColor, pPaintData);

    // Border.
    dred_rect borderRect = dred_control_get_local_rect(pControl);
    dred_control_draw_rect_outline(pControl, borderRect, pTextView->borderColor, pTextView->borderWidth, pPaintData);

    // Padding.
    dred_rect paddingRect = dred_grow_rect(textRect, pTextView->padding);
    dred_control_draw_rect_outline(pControl, paddingRect, pTextView->defaultStyle.bgColor, pTextView->padding, pPaintData);

    // Text.
    dred_control_set_clip(pControl, dred_clamp_rect(textRect, relativeRect), pPaintData);
    drte_view_paint(pTextView->pView, dred_rect_to_drte(dred_offset_rect(dred_clamp_rect(textRect, relativeRect), -textRect.left, -textRect.top)), pPaintData);
}

void dred_textview__on_timer(dred_timer* pTimer, void* pUserData)
{
    (void)pTimer;

    dred_textview* pTextView = (dred_textview*)pUserData;
    assert(pTextView != NULL);

    dred_textview_step(pTextView, 100);
}

void dred_textview_on_capture_keyboard(dred_control* pControl, dred_control* pPrevCapturedControl)
{
    (void)pPrevCapturedControl;

    dred_textview* pTextView = DRED_TEXTVIEW(pControl);
    if (pTextView == NULL) {
        return;
    }

    drte_view_show_cursors(pTextView->pView);

    if (pTextView->pTimer == NULL) {
        pTextView->pTimer = dred_timer_create(100, dred_textview__on_timer, pTextView);
    }
}

void dred_textview_on_release_keyboard(dred_control* pControl, dred_control* pNewCapturedControl)
{
    (void)pNewCapturedControl;

    dred_textview* pTextView = DRED_TEXTVIEW(pControl);
    if (pTextView == NULL) {
        return;
    }

    drte_view_hide_cursors(pTextView->pView);

    if (pTextView->pTimer != NULL) {
        dred_timer_delete(pTextView->pTimer);
        pTextView->pTimer = NULL;
    }
}

void dred_textview_on_capture_mouse(dred_control* pControl)
{
    if (pControl == NULL) {
        return;
    }

}

void dred_textview_on_release_mouse(dred_control* pControl)
{
    if (pControl == NULL) {
        return;
    }


}



void dred_textview__get_text_offset(dred_textview* pTextView, float* pOffsetXOut, float* pOffsetYOut)
{
    float offsetX = 0;
    float offsetY = 0;

    if (pTextView != NULL)
    {
        float lineNumbersWidth = 0;
        if (dred_control_is_visible(pTextView->pLineNumbers)) {
            lineNumbersWidth = dred_control_get_width(pTextView->pLineNumbers);
        }

        offsetX = pTextView->borderWidth + pTextView->padding + lineNumbersWidth;
        offsetY = pTextView->borderWidth + pTextView->padding;
    }


    if (pOffsetXOut != NULL) {
        *pOffsetXOut = offsetX;
    }
    if (pOffsetYOut != NULL) {
        *pOffsetYOut = offsetY;
    }
}

void dred_textview__calculate_text_engine_container_size(dred_textview* pTextView, float* pWidthOut, float* pHeightOut)
{
    float width  = 0;
    float height = 0;

    if (pTextView != NULL)
    {
        float horzScrollbarSize = 0;
        if (dred_control_is_visible(DRED_CONTROL(pTextView->pHorzScrollbar))) {
            horzScrollbarSize = dred_control_get_height(DRED_CONTROL(pTextView->pHorzScrollbar));
        }

        float vertScrollbarSize = 0;
        if (dred_control_is_visible(DRED_CONTROL(pTextView->pVertScrollbar))) {
            vertScrollbarSize = dred_control_get_width(DRED_CONTROL(pTextView->pVertScrollbar));
        }

        float lineNumbersWidth = 0;
        if (dred_control_is_visible(pTextView->pLineNumbers)) {
            lineNumbersWidth = dred_control_get_width(pTextView->pLineNumbers);
        }

        width  = dred_control_get_width(DRED_CONTROL(pTextView))  - (pTextView->borderWidth + pTextView->padding)*2 - vertScrollbarSize - lineNumbersWidth;
        height = dred_control_get_height(DRED_CONTROL(pTextView)) - (pTextView->borderWidth + pTextView->padding)*2 - horzScrollbarSize;
    }

    if (pWidthOut != NULL) {
        *pWidthOut = width;
    }
    if (pHeightOut != NULL) {
        *pHeightOut = height;
    }
}

dred_rect dred_textview__get_text_rect(dred_textview* pTextView)
{
    if (pTextView == NULL) {
        return dred_make_rect(0, 0, 0, 0);
    }

    float offsetX;
    float offsetY;
    dred_textview__get_text_offset(pTextView, &offsetX, &offsetY);

    float width;
    float height;
    dred_textview__calculate_text_engine_container_size(pTextView, &width, &height);

    return dred_make_rect(offsetX, offsetY, offsetX + width, offsetY + height);
}


void dred_textview__refresh_scrollbars(dred_textview* pTextView)
{
    // The layout depends on the range because we may be dynamically hiding and showing the scrollbars depending on the range. Thus, we
    // refresh the range first. However, dynamically showing and hiding the scrollbars (which is done when the layout is refreshed) affects
    // the size of the text box, which in turn affects the range. Thus, we need to refresh the ranges a second time after the layouts.

    dred_textview__refresh_scrollbar_ranges(pTextView);
    dred_textview__refresh_scrollbar_layouts(pTextView);
    dred_textview__refresh_scrollbar_ranges(pTextView);
}

void dred_textview__refresh_horizontal_scrollbar(dred_textview* pTextView)
{
    assert(pTextView != NULL);

    float textWidth = drte_view_get_visible_line_width(pTextView->pView);
    float containerWidth;
    drte_view_get_size(pTextView->pView, &containerWidth, NULL);
    dred_scrollbar_set_range_and_page_size(pTextView->pHorzScrollbar, 0, (int)textWidth, (int)containerWidth);

    if (dred_scrollbar_is_thumb_visible(pTextView->pHorzScrollbar)) {
        if (!dred_control_is_visible(DRED_CONTROL(pTextView->pHorzScrollbar))) {
            dred_control_show(DRED_CONTROL(pTextView->pHorzScrollbar));
            dred_textview__refresh_line_numbers(pTextView);
        }
    } else {
        if (dred_control_is_visible(DRED_CONTROL(pTextView->pHorzScrollbar))) {
            dred_control_hide(DRED_CONTROL(pTextView->pHorzScrollbar));
            dred_textview__refresh_line_numbers(pTextView);
        }
    }
}

void dred_textview__refresh_scrollbar_ranges(dred_textview* pTextView)
{
    assert(pTextView != NULL);

    // The vertical scrollbar is based on the line count.
    size_t lineCount = drte_view_get_line_count(pTextView->pView);
    size_t pageSize  = drte_view_get_visible_line_count(pTextView->pView);

    size_t extraScroll = 0;
    if (pTextView->isExcessScrollingEnabled) {
        extraScroll = drte_view_get_visible_line_count(pTextView->pView) - 1 - 1;  // -1 to make the range 0 based. -1 to ensure at least one line is visible.
    }

    dred_scrollbar_set_range_and_page_size(pTextView->pVertScrollbar, 0, (int)(lineCount + extraScroll), (int)pageSize);

    if (dred_scrollbar_is_thumb_visible(pTextView->pVertScrollbar)) {
        if (!dred_control_is_visible(DRED_CONTROL(pTextView->pVertScrollbar))) {
            dred_control_show(DRED_CONTROL(pTextView->pVertScrollbar));
        }
    } else {
        if (dred_control_is_visible(DRED_CONTROL(pTextView->pVertScrollbar))) {
            dred_control_hide(DRED_CONTROL(pTextView->pVertScrollbar));
        }
    }


    // The horizontal scrollbar is a per-pixel scrollbar, and is based on the width of the text versus the width of the container.
    dred_textview__refresh_horizontal_scrollbar(pTextView);
}

void dred_textview__refresh_scrollbar_layouts(dred_textview* pTextView)
{
    assert(pTextView != NULL);

    float offsetLeft   = pTextView->borderWidth;
    float offsetTop    = pTextView->borderWidth;
    float offsetRight  = pTextView->borderWidth;
    float offsetBottom = pTextView->borderWidth;

    float scrollbarSizeH = (dred_scrollbar_is_thumb_visible(pTextView->pHorzScrollbar) && pTextView->isHorzScrollbarEnabled) ? pTextView->horzScrollbarSize : 0;
    float scrollbarSizeV = (dred_scrollbar_is_thumb_visible(pTextView->pVertScrollbar) && pTextView->isVertScrollbarEnabled) ? pTextView->vertScrollbarSize : 0;

    dred_control_set_size(DRED_CONTROL(pTextView->pVertScrollbar), scrollbarSizeV, dred_control_get_height(DRED_CONTROL(pTextView)) /*- scrollbarSizeH*/ - (offsetTop + offsetBottom));
    dred_control_set_size(DRED_CONTROL(pTextView->pHorzScrollbar), dred_control_get_width(DRED_CONTROL(pTextView)) - scrollbarSizeV - (offsetLeft + offsetRight), scrollbarSizeH);

    dred_control_set_relative_position(DRED_CONTROL(pTextView->pVertScrollbar), dred_control_get_width(DRED_CONTROL(pTextView)) - scrollbarSizeV - offsetRight, offsetTop);
    dred_control_set_relative_position(DRED_CONTROL(pTextView->pHorzScrollbar), offsetLeft, dred_control_get_height(DRED_CONTROL(pTextView)) - scrollbarSizeH - offsetBottom);


    // A change in the layout of the horizontal scrollbar will affect the layout of the line numbers.
    dred_textview__refresh_line_numbers(pTextView);
}

dred_rect dred_textview__get_scrollbar_dead_space_rect(dred_textview* pTextView)
{
    assert(pTextView != NULL);

    float offsetLeft   = pTextView->borderWidth;
    float offsetTop    = pTextView->borderWidth;
    float offsetRight  = pTextView->borderWidth;
    float offsetBottom = pTextView->borderWidth;

    float scrollbarSizeH = (dred_control_is_visible(DRED_CONTROL(pTextView->pHorzScrollbar)) && pTextView->isHorzScrollbarEnabled) ? dred_control_get_width(DRED_CONTROL(pTextView->pHorzScrollbar)) : 0;
    float scrollbarSizeV = (dred_control_is_visible(DRED_CONTROL(pTextView->pVertScrollbar)) && pTextView->isHorzScrollbarEnabled) ? dred_control_get_height(DRED_CONTROL(pTextView->pVertScrollbar)) : 0;

    if (scrollbarSizeH == 0 && scrollbarSizeV == 0) {
        return dred_make_rect(0, 0, 0, 0);
    }

    return dred_make_rect(scrollbarSizeH + offsetLeft, scrollbarSizeV + offsetTop, dred_control_get_width(DRED_CONTROL(pTextView)) - offsetRight, dred_control_get_height(DRED_CONTROL(pTextView)) - offsetBottom);
}


void dred_textview__on_mouse_move_line_numbers(dred_control* pLineNumbers, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;

    dred_textview* pTextView = DRED_TEXTVIEW(dred_control_get_parent(pLineNumbers));
    assert(pTextView != NULL);

    if ((stateFlags & DRED_GUI_MOUSE_BUTTON_LEFT_DOWN) != 0)
    {
        if (dred_gui_get_element_with_mouse_capture(pLineNumbers->pGUI) == pLineNumbers)
        {
            // We just move the cursor around based on the line number we've moved over.

            //float offsetX = pTextEditorData->padding;
            float offsetY = pTextView->padding + pTextView->pView->innerOffsetY;
            size_t iLine = drte_view_get_line_at_pos_y(pTextView->pView, NULL, relativeMousePosY - offsetY);
            size_t iAnchorLine = pTextView->iLineSelectAnchor;
            size_t lineCount = drte_view_get_line_count(pTextView->pView);

            // If we're moving updwards we want to position the cursor at the start of the line. Otherwise we want to move the cursor to the start
            // of the next line, or the end of the text.
            dr_bool32 movingUp = DR_FALSE;
            if (iLine < iAnchorLine) {
                movingUp = DR_TRUE;
            }

            // If we're moving up the selection anchor needs to be placed at the end of the last line. Otherwise we need to move it to the start
            // of the first line.
            if (movingUp) {
                if (iAnchorLine + 1 < lineCount) {
                    drte_view_move_selection_anchor_to_start_of_line(pTextView->pView, iAnchorLine + 1);
                } else {
                    drte_view_move_selection_anchor_to_end_of_line(pTextView->pView, iAnchorLine);
                }
            } else {
                drte_view_move_selection_anchor_to_start_of_line(pTextView->pView, iAnchorLine);
            }


            // If we're moving up we want the cursor to be placed at the start of the selection range. Otherwise we want to place the cursor
            // at the end of the selection range.
            if (movingUp) {
                drte_view_move_cursor_to_start_of_line_by_index(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), iLine);
            } else {
                if (iLine + 1 < lineCount) {
                    drte_view_move_cursor_to_start_of_line_by_index(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), iLine + 1);
                } else {
                    drte_view_move_cursor_to_end_of_line_by_index(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), iLine);
                }
            }


            drte_view_set_selection_end_point(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));
        }
    }
}

void dred_textview__on_mouse_button_down_line_numbers(dred_control* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)stateFlags;

    dred_textview* pTextView = DRED_TEXTVIEW(dred_control_get_parent(pLineNumbers));
    assert(pTextView != NULL);

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT)
    {
        dred_gui_capture_mouse(pLineNumbers);

        // If the shift key is down and we already have a selection, this is equivalent to a mouse drag.
        if ((stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) != 0) {
            if (drte_view_is_anything_selected(pTextView->pView)) {
                dred_textview__on_mouse_move_line_numbers(pLineNumbers, relativeMousePosX, relativeMousePosY, stateFlags | DRED_GUI_MOUSE_BUTTON_LEFT_DOWN);
                return;
            }
        }


        //float offsetX = pTextEditorData->padding;
        float offsetY = pTextView->padding + pTextView->pView->innerOffsetY;
        size_t iClickedLine = drte_view_get_line_at_pos_y(pTextView->pView, NULL, relativeMousePosY - offsetY);

        if ((stateFlags & DRED_GUI_KEY_STATE_SHIFT_DOWN) != 0) {
            pTextView->iLineSelectAnchor = drte_view_get_cursor_line(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
        } else {
            pTextView->iLineSelectAnchor = iClickedLine;
        }


        if ((stateFlags & DRED_GUI_KEY_STATE_CTRL_DOWN) == 0) {
            dred_textview_deselect_all(pTextView);
        }

        drte_view_begin_selection(pTextView->pView, drte_view_get_line_first_character(pTextView->pView, NULL, pTextView->iLineSelectAnchor));


        if (iClickedLine + 1 < drte_view_get_line_count(pTextView->pView)) {
            drte_view_move_cursor_to_start_of_line_by_index(pTextView->pView, drte_view_get_last_cursor(pTextView->pView), iClickedLine + 1);
        } else {
            drte_view_move_cursor_to_end_of_line(pTextView->pView, drte_view_get_last_cursor(pTextView->pView));
        }

        drte_view_set_selection_end_point(pTextView->pView, drte_view_get_cursor_character(pTextView->pView, drte_view_get_last_cursor(pTextView->pView)));
    }
}

void dred_textview__on_mouse_button_up_line_numbers(dred_control* pLineNumbers, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
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

void dred_textview__on_paint_rect_line_numbers(drte_engine* pEngine, drte_view* pView, drte_style_token styleToken, drte_rect rect, void* pPaintData)
{
    (void)pEngine;

    dred_textview* pTextView = (dred_textview*)pView->pUserData;
    dred_text_style* pStyle = (dred_text_style*)styleToken;

    assert(pTextView != NULL);

    float offsetX = pTextView->padding;
    float offsetY = pTextView->padding;

    dred_control_draw_rect(pTextView->pLineNumbers, dred_offset_rect(drte_rect_to_dred(rect), offsetX, offsetY), pStyle->bgColor, pPaintData);
}

void dred_textview__on_paint_text_line_numbers(drte_engine* pEngine, drte_view* pView, drte_style_token styleTokenFG, drte_style_token styleTokenBG, const char* text, size_t textLength, float posX, float posY, void* pPaintData)
{
    (void)pEngine;

    dred_textview* pTextView = (dred_textview*)pView->pUserData;

    dred_text_style* pStyleFG = (dred_text_style*)styleTokenFG;
    dred_text_style* pStyleBG = (dred_text_style*)styleTokenBG;

    assert(pTextView != NULL);

    float offsetX = pTextView->padding;
    float offsetY = pTextView->padding;

    dred_control_draw_text(pTextView->pLineNumbers, pStyleFG->pFont, text, (int)textLength, posX + offsetX, posY + offsetY, pStyleFG->fgColor, pStyleBG->bgColor, pPaintData);
}

void dred_textview__on_paint_line_numbers(dred_control* pLineNumbers, dred_rect relativeRect, void* pPaintData)
{
    (void)relativeRect;

    dred_textview* pTextView = DRED_TEXTVIEW(dred_control_get_parent(pLineNumbers));
    assert(pTextView != NULL);

    float lineNumbersWidth  = dred_control_get_width(pLineNumbers) - (pTextView->padding*2) - pTextView->lineNumbersPaddingRight;
    float lineNumbersHeight = dred_control_get_height(pLineNumbers) - (pTextView->padding*2);

    drte_view_paint_line_numbers(pTextView->pView, lineNumbersWidth, lineNumbersHeight, dred_textview__on_paint_text_line_numbers, dred_textview__on_paint_rect_line_numbers, pPaintData);

    dred_control_draw_rect_outline(pLineNumbers, dred_control_get_local_rect(pLineNumbers), pTextView->lineNumbersStyle.bgColor, pTextView->padding, pPaintData);

    // Right padding.
    dred_rect rightPaddingRect = dred_control_get_local_rect(pLineNumbers);
    rightPaddingRect.right -= pTextView->padding;
    rightPaddingRect.left   = rightPaddingRect.right - pTextView->lineNumbersPaddingRight;
    dred_control_draw_rect(pLineNumbers, rightPaddingRect, pTextView->lineNumbersStyle.bgColor, pPaintData);
}

void dred_textview__refresh_line_numbers(dred_textview* pTextView)
{
    assert(pTextView != NULL);

    dred_rect lineNumbersRectOld = dred_control_get_local_rect(pTextView->pLineNumbers);
    dred_control_begin_dirty(pTextView->pLineNumbers);

    float lineNumbersWidth = 0;
    if (dred_control_is_visible(pTextView->pLineNumbers)) {
        lineNumbersWidth = pTextView->lineNumbersWidth;
    }

    float scrollbarHeight = dred_control_is_visible(DRED_CONTROL(pTextView->pHorzScrollbar)) ? dred_control_get_height(DRED_CONTROL(pTextView->pHorzScrollbar)) : 0;
    dred_control_set_size(pTextView->pLineNumbers, lineNumbersWidth, dred_control_get_height(DRED_CONTROL(pTextView)) - scrollbarHeight);


    // The size of the text container may have changed.
    float textEditorWidth;
    float textEditorHeight;
    dred_textview__calculate_text_engine_container_size(pTextView, &textEditorWidth, &textEditorHeight);
    drte_view_set_size(pTextView->pView, textEditorWidth, textEditorHeight);


    // Force a redraw just to be sure everything is in a valid state.
    dred_control_dirty(DRED_CONTROL(pTextView), dred_rect_union(lineNumbersRectOld, dred_control_get_local_rect(pTextView->pLineNumbers)));
    dred_control_end_dirty(pTextView->pLineNumbers);
}

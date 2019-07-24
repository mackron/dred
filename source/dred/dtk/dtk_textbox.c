// Copyright (C) 2019 David Reid. See included LICENSE file.

// ISSUES
// ======
//
// - Not thread-safe. Editing the text on a second thread while in the middle of drawing will cause an error.
//   - Perhaps only do text editing in the event handler (force all operations that edit text to go through the event handler)?

DTK_PRIVATE dtk_font* dtk_textbox__get_default_font(dtk_textbox* pTextBox)
{
    return dtk_get_ui_font(DTK_CONTROL(pTextBox)->pTK);
}

dtk_result dtk_textbox_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    dtk_zero_object(pTextBox);

    dtk_result result = dtk_control_init(pTK, DTK_CONTROL_TYPE_TEXTBOX, (onEvent != NULL) ? onEvent : dtk_textbox_default_event_handler, pParent, &pTextBox->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    dtk_control_set_cursor(DTK_CONTROL(pTextBox), dtk_system_cursor_type_text);

    pTextBox->mode = dtk_textbox_mode_single_line;

    pTextBox->style.bgColor        = dtk_color_white;
    pTextBox->style.fgColor        = dtk_color_black;
    pTextBox->style.selectionColor = dtk_rgb(192, 224, 248);
    pTextBox->style.borderColor    = dtk_rgb(96, 96, 96);
    pTextBox->style.borderWidth    = 1;
    pTextBox->style.paddingLeft    = 2;
    pTextBox->style.paddingTop     = 2;
    pTextBox->style.paddingRight   = 2;
    pTextBox->style.paddingBottom  = 2;
    pTextBox->style.alignmentHorz  = dtk_text_alignment_left;
    pTextBox->style.alignmentVert  = dtk_text_alignment_center;
    pTextBox->style.caretWidth     = 1;

    return DTK_SUCCESS;
}

dtk_result dtk_textbox_uninit(dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_control_uninit(&pTextBox->control);
}

dtk_result dtk_textbox_set_mode(dtk_textbox* pTextBox, dtk_textbox_mode mode)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    pTextBox->mode = mode;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTextBox), dtk_control_get_local_rect(DTK_CONTROL(pTextBox)));
    return DTK_SUCCESS;
}

dtk_textbox_mode dtk_textbox_get_mode(dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return dtk_textbox_mode_single_line;
    }

    return pTextBox->mode;
}

dtk_result dtk_textbox_set_text_utf8(dtk_textbox* pTextBox, const char* pTextUTF8, size_t textLen)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    if (pTextUTF8 == NULL) {
        pTextUTF8 = "";
    }

    if (textLen == (size_t)-1) {
        textLen = strlen(pTextUTF8);
    }

    if (pTextBox->textCap <= textLen) {
        size_t newCap = pTextBox->textCap*2;
        if (newCap < textLen+1) {
            newCap = textLen+1;
        }

        char* pNewTextUTF8 = (char*)dtk_realloc(pTextBox->pTextUTF8, sizeof(*pNewTextUTF8) * newCap);
        if (pNewTextUTF8 == NULL) {
            return DTK_OUT_OF_MEMORY;
        }

        pTextBox->pTextUTF8 = pNewTextUTF8;
        pTextBox->textCap   = newCap;
    }

    dtk_assert(pTextBox->textCap >= textLen+1);
    dtk_strncpy_s(pTextBox->pTextUTF8, textLen+1, pTextUTF8, textLen);
    pTextBox->textLen = textLen;

    pTextBox->selectionBeg = 0;
    pTextBox->selectionEnd = 0;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTextBox), dtk_control_get_local_rect(DTK_CONTROL(pTextBox)));
    return DTK_SUCCESS;
}

const char* dtk_textbox_get_text_utf8(dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return NULL;
    }

    if (pTextBox->pTextUTF8 == NULL) {
        return "";
    }

    return pTextBox->pTextUTF8;
}

size_t dtk_textbox_get_text_length_utf8(dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return pTextBox->textLen;
}

dtk_result dtk_textbox_set_font(dtk_textbox* pTextBox, dtk_font* pFont)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    pTextBox->style.pFont = pFont;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTextBox), dtk_control_get_local_rect(DTK_CONTROL(pTextBox)));
    return DTK_SUCCESS;
}

dtk_font* dtk_textbox_get_font(dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return NULL;
    }

    if (pTextBox->style.pFont == NULL) {
        return dtk_textbox__get_default_font(pTextBox);
    }

    return pTextBox->style.pFont;
}

dtk_result dtk_textbox_set_background_color(dtk_textbox* pTextBox, dtk_color color)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    pTextBox->style.bgColor = color;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTextBox), dtk_control_get_local_rect(DTK_CONTROL(pTextBox)));
    return DTK_SUCCESS;
}

dtk_color dtk_textbox_get_background_color(dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return dtk_color_white;
    }

    return pTextBox->style.bgColor;
}

dtk_result dtk_textbox_set_foreground_color(dtk_textbox* pTextBox, dtk_color color)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    pTextBox->style.fgColor = color;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTextBox), dtk_control_get_local_rect(DTK_CONTROL(pTextBox)));
    return DTK_SUCCESS;
}

dtk_color dtk_textbox_get_foreground_color(dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return dtk_color_black;
    }

    return pTextBox->style.fgColor;
}

dtk_result dtk_textbox_set_border_color(dtk_textbox* pTextBox, dtk_color color)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    pTextBox->style.borderColor = color;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTextBox), dtk_control_get_local_rect(DTK_CONTROL(pTextBox)));
    return DTK_INVALID_ARGS;
}

dtk_color dtk_textbox_get_border_color(dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return dtk_color_black;
    }

    return pTextBox->style.borderColor;
}

dtk_result dtk_textbox_set_border_width(dtk_textbox* pTextBox, dtk_uint32 width)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    pTextBox->style.borderWidth = width;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTextBox), dtk_control_get_local_rect(DTK_CONTROL(pTextBox)));
    return DTK_INVALID_ARGS;
}

dtk_uint32 dtk_textbox_get_border_width(dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return 0;
    }

    return pTextBox->style.borderWidth;
}

dtk_result dtk_textbox_set_padding(dtk_textbox* pTextBox, dtk_uint32 left, dtk_uint32 top, dtk_uint32 right, dtk_uint32 bottom)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    pTextBox->style.paddingLeft   = left;
    pTextBox->style.paddingTop    = top;
    pTextBox->style.paddingRight  = right;
    pTextBox->style.paddingBottom = bottom;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTextBox), dtk_control_get_local_rect(DTK_CONTROL(pTextBox)));
    return DTK_INVALID_ARGS;
}

dtk_result dtk_textbox_get_padding(dtk_textbox* pTextBox, dtk_uint32* pLeft, dtk_uint32* pTop, dtk_uint32* pRight, dtk_uint32* pBottom)
{
    if (pLeft != NULL) {
        *pLeft = 0;
    }
    if (pTop != NULL) {
        *pTop = 0;
    }
    if (pRight != NULL) {
        *pRight = 0;
    }
    if (pBottom != NULL) {
        *pBottom = 0;
    }

    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    if (pLeft != NULL) {
        *pLeft = pTextBox->style.paddingLeft;
    }
    if (pTop != NULL) {
        *pTop = pTextBox->style.paddingTop;
    }
    if (pRight != NULL) {
        *pRight = pTextBox->style.paddingRight;
    }
    if (pBottom != NULL) {
        *pBottom = pTextBox->style.paddingBottom;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_textbox_set_text_alignment(dtk_textbox* pTextBox, dtk_text_alignment alignmentHorz, dtk_text_alignment alignmentVert)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    if (alignmentHorz == dtk_text_alignment_top || alignmentVert == dtk_text_alignment_bottom) {
        return DTK_INVALID_ARGS;
    }
    if (alignmentVert == dtk_text_alignment_left || alignmentHorz == dtk_text_alignment_right) {
        return DTK_INVALID_ARGS;
    }

    pTextBox->style.alignmentHorz = alignmentHorz;
    pTextBox->style.alignmentVert = alignmentVert;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTextBox), dtk_control_get_local_rect(DTK_CONTROL(pTextBox)));
    return DTK_SUCCESS;
}

dtk_result dtk_textbox_get_text_alignment(dtk_textbox* pTextBox, dtk_text_alignment* pAlignmentHorz, dtk_text_alignment* pAlignmentVert)
{
    if (pAlignmentHorz != NULL) {
        *pAlignmentHorz = dtk_text_alignment_left;
    }
    if (pAlignmentVert != NULL) {
        *pAlignmentVert = dtk_text_alignment_top;
    }

    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    if (pAlignmentHorz != NULL) {
        *pAlignmentHorz = pTextBox->style.alignmentHorz;
    }
    if (pAlignmentVert != NULL) {
        *pAlignmentVert = pTextBox->style.alignmentVert;
    }

    return DTK_SUCCESS;
}


DTK_PRIVATE void dtk_textbox__get_first_visible_line(dtk_textbox* pTextBox, dtk_uint32* pIndexOut, dtk_uint32* pLineBegOut, dtk_uint32* pLineEndPlus1Out)
{
    dtk_assert(pTextBox != NULL);

    if (pTextBox->mode == dtk_textbox_mode_single_line) {
        if (pIndexOut) {
            *pIndexOut = 0;
        }
        if (pLineBegOut) {
            *pLineBegOut = 0;
        }
        if (pLineEndPlus1Out) {
            *pLineEndPlus1Out = strlen(pTextBox->pTextUTF8);
        }
    } else {
        /* TODO: Multiline. Depends on the vertical alignment. Implement me. */
        if (pIndexOut) {
            *pIndexOut = 0;
        }
        if (pLineBegOut) {
            *pLineBegOut = 0;
        }
        if (pLineEndPlus1Out) {
            *pLineEndPlus1Out = strlen(pTextBox->pTextUTF8);
        }
    }
}

DTK_PRIVATE void dtk_textbox__get_last_visible_line(dtk_textbox* pTextBox, dtk_uint32* pIndexOut, dtk_uint32* pLineBegOut, dtk_uint32* pLineEndPlus1Out)
{
    dtk_assert(pTextBox != NULL);

    if (pTextBox->mode == dtk_textbox_mode_single_line) {
        if (pIndexOut) {
            *pIndexOut = 0;
        }
        if (pLineBegOut) {
            *pLineBegOut = 0;
        }
        if (pLineEndPlus1Out) {
            *pLineEndPlus1Out = strlen(pTextBox->pTextUTF8);
        }
    } else {
        /* TODO: Multiline. Depends on the vertical alignment. Implement me. */
        if (pIndexOut) {
            *pIndexOut = 0;
        }
        if (pLineBegOut) {
            *pLineBegOut = 0;
        }
        if (pLineEndPlus1Out) {
            *pLineEndPlus1Out = strlen(pTextBox->pTextUTF8);
        }
    }
}

DTK_PRIVATE dtk_rect dtk_textbox__get_inner_rect(dtk_textbox* pTextBox)
{
    dtk_rect rect;
    rect.left   = pTextBox->style.borderWidth + pTextBox->style.paddingLeft;
    rect.top    = pTextBox->style.borderWidth + pTextBox->style.paddingTop;
    rect.right  = dtk_control_get_width(DTK_CONTROL(pTextBox))  - pTextBox->style.paddingRight  - pTextBox->style.borderWidth;
    rect.bottom = dtk_control_get_height(DTK_CONTROL(pTextBox)) - pTextBox->style.paddingBottom - pTextBox->style.borderWidth;

    return rect;
}

dtk_result dtk_textbox_xy_to_cp(dtk_textbox* pTextBox, dtk_int32 x, dtk_int32 y, size_t* cp)
{
    dtk_result result;
    dtk_uint32 lineBeg;
    dtk_uint32 lineEndPlus1;
    float scale;
    dtk_int32 lineSizeX;
    dtk_int32 lineSizeY;

    if (cp != NULL) {
        *cp = 0;
    }

    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    /* Determine the y position first. */
    if (pTextBox->mode == dtk_textbox_mode_single_line) {
        lineBeg      = 0;
        lineEndPlus1 = pTextBox->textLen;
    } else {
        if (y < (dtk_int32)pTextBox->style.paddingTop) {
            dtk_textbox__get_first_visible_line(pTextBox, NULL, &lineBeg, &lineEndPlus1);
        } else if (y > (dtk_control_get_height(DTK_CONTROL(pTextBox) - pTextBox->style.paddingBottom))) {
            dtk_textbox__get_last_visible_line(pTextBox, NULL, &lineBeg, &lineEndPlus1);
        } else {
            if (pTextBox->textLen == 0 || dtk_string_is_null_or_empty(pTextBox->pTextUTF8)) {
                lineBeg      = 0;
                lineEndPlus1 = 0;
            } else {
                /* TODO: Implement me. Need to iterate over each line and keep a running total of the line height. */
                lineBeg      = 0;
                lineEndPlus1 = 0;
            }
        }
    }

    /* Now that we have the start position of the line we can grab the x position. This depends on the horizontal alignment and the scroll position. */
    scale = dtk_control_get_scaling_factor(DTK_CONTROL(pTextBox));

    dtk_font_measure_string(dtk_textbox_get_font(pTextBox), scale, pTextBox->pTextUTF8+lineBeg, (lineEndPlus1-lineBeg), &lineSizeX, &lineSizeY);

    if (pTextBox->style.alignmentHorz == dtk_text_alignment_center) {
        /* Center alignment. */
    } else if (pTextBox->style.alignmentHorz == dtk_text_alignment_right) {
        /* Right alignment. */
    } else {
        /* Left alignment. */
        int maxWidth  = lineSizeX;
        int inputPosX = (int)(x - ((pTextBox->style.borderWidth + pTextBox->style.paddingLeft) * scale));
        if (inputPosX < 0) {
            inputPosX = 0;
        }

        result = dtk_font_get_text_cursor_position_from_point(dtk_textbox_get_font(pTextBox), scale, pTextBox->pTextUTF8+lineBeg, (lineEndPlus1-lineBeg), maxWidth, inputPosX, &inputPosX, cp);
        if (result != DTK_SUCCESS) {
            return result;  /* Failed to retrieve the position. */
        }
    }

    return DTK_SUCCESS;
}

dtk_result dtk_textbox_cp_to_xy(dtk_textbox* pTextBox, size_t cp, dtk_int32* x, dtk_int32* y)
{
    dtk_int32 caretPosX;
    dtk_int32 caretPosY;
    float scale;
    dtk_font_metrics fontMetrics;
    dtk_rect innerRect;

    if (x) *x = 0;
    if (y) *y = 0;

    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    scale = dtk_control_get_scaling_factor(DTK_CONTROL(pTextBox));
    dtk_font_get_metrics(dtk_textbox_get_font(pTextBox), scale, &fontMetrics);
    innerRect = dtk_textbox__get_inner_rect(pTextBox);

    if (pTextBox->mode == dtk_textbox_mode_single_line) {
        dtk_int32 textSizeX;
        dtk_int32 textSizeY;
        dtk_font_measure_string(dtk_textbox_get_font(pTextBox), scale, pTextBox->pTextUTF8, cp, &textSizeX, &textSizeY);

        textSizeY = fontMetrics.lineHeight;

        /* X position. */
        if (pTextBox->style.alignmentHorz == dtk_text_alignment_center) {
            /* Center. */
            caretPosX = 0;
        } else if (pTextBox->style.alignmentHorz == dtk_text_alignment_right) {
            /* Right. */
            caretPosX = 0;
        } else {
            /* Left. */
            caretPosX = (dtk_int32)((pTextBox->style.borderWidth + pTextBox->style.paddingLeft) * scale) + textSizeX;
        }
        

        /* Y position. */
        if (pTextBox->style.alignmentVert == dtk_text_alignment_center) {
            /* Center. */
            caretPosY = (dtk_int32)(((((innerRect.bottom - innerRect.top) * scale) - textSizeY) / 2) + (innerRect.top * scale));
        } else if (pTextBox->style.alignmentVert == dtk_text_alignment_bottom) {
            /* Bottom. */
            caretPosY = 0;
        } else {
            /* Top. */
            caretPosY = (dtk_int32)((pTextBox->style.borderWidth + pTextBox->style.paddingTop) * scale);
        }
    } else {
        /* TODO: Implement me. */
        caretPosX = 0;
        caretPosY = 0;
    }
    
    if (x) *x = caretPosX;
    if (y) *y = caretPosY;

    return DTK_SUCCESS;
}

dtk_result dtk_textbox_move_caret_to_xy(dtk_textbox* pTextBox, dtk_int32 x, dtk_int32 y)
{
    dtk_result result;
    size_t newCaretPosInBytes;

    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    result = dtk_textbox_xy_to_cp(pTextBox, x, y, &newCaretPosInBytes);
    if (result != DTK_SUCCESS) {
        return result;
    }

    return dtk_textbox_move_caret_to_cp(pTextBox, newCaretPosInBytes);
}

dtk_result dtk_textbox_move_caret_to_cp(dtk_textbox* pTextBox, size_t cp)
{
    if (pTextBox == NULL) {
        return DTK_INVALID_ARGS;
    }

    if (cp > pTextBox->textLen) {
        return DTK_INVALID_ARGS;
    }

    dtk_control_scheduled_redraw(DTK_CONTROL(pTextBox), dtk_textbox_get_caret_rect(pTextBox));  /* Redraw the caret's previous rectangle. */
    {
        pTextBox->caretPosInBytes = cp;
    }
    dtk_control_scheduled_redraw(DTK_CONTROL(pTextBox), dtk_textbox_get_caret_rect(pTextBox));  /* Redraw the caret's new rectangle. */

    return DTK_SUCCESS;
}

dtk_rect dtk_textbox_get_caret_rect(dtk_textbox* pTextBox)
{
    dtk_rect rect;
    dtk_font_metrics fontMetrics;
    float scale;

    if (pTextBox == NULL) {
        return dtk_rect_init(0, 0, 0, 0);
    }

    scale = dtk_control_get_scaling_factor(DTK_CONTROL(pTextBox));
    dtk_font_get_metrics(dtk_textbox_get_font(pTextBox), scale, &fontMetrics);

    dtk_textbox_cp_to_xy(pTextBox, pTextBox->caretPosInBytes, &rect.left, &rect.top);
    rect.right  = rect.left + (dtk_uint32)(pTextBox->style.caretWidth * scale);
    rect.bottom = rect.top  + fontMetrics.lineHeight;

    return rect;
}



dtk_bool32 dtk_textbox_default_event_handler__single_line(dtk_event* pEvent)
{
    dtk_textbox* pTextBox = DTK_TEXTBOX(pEvent->pControl);
    dtk_assert(pTextBox != NULL);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            // Draw outside in:
            //   1) Border
            //   2) Padding
            //   3) Text (with clipping)

            float scale = dtk_control_get_scaling_factor(DTK_CONTROL(pTextBox));
            dtk_rect outerRect = dtk_control_get_local_rect(DTK_CONTROL(pTextBox));
            dtk_rect innerRect = outerRect;

            // Border.
            if (pTextBox->style.borderWidth > 0) {
                dtk_surface_draw_rect_outline(pEvent->paint.pSurface, outerRect, pTextBox->style.borderColor, (dtk_int32)(pTextBox->style.borderWidth*scale));
                innerRect = dtk_rect_grow(innerRect, -(dtk_int32)(pTextBox->style.borderWidth * scale));
                outerRect = dtk_rect_grow(outerRect, -(dtk_int32)(pTextBox->style.borderWidth * scale));
            }

            // Padding.
            innerRect.left   += (dtk_int32)(pTextBox->style.paddingLeft   * scale);
            innerRect.top    += (dtk_int32)(pTextBox->style.paddingTop    * scale);
            innerRect.right  -= (dtk_int32)(pTextBox->style.paddingRight  * scale);
            innerRect.bottom -= (dtk_int32)(pTextBox->style.paddingBottom * scale);
            dtk_surface_draw_outer_rect(pEvent->paint.pSurface, outerRect, innerRect, pTextBox->style.bgColor);
            outerRect = innerRect;

            // Text Clip
            dtk_surface_set_clip(pEvent->paint.pSurface, innerRect);

            // Text Outer Area
            dtk_int32 textSizeX;
            dtk_int32 textSizeY;
            dtk_font_measure_string(dtk_textbox_get_font(pTextBox), scale, dtk_textbox_get_text_utf8(pTextBox), (size_t)-1, &textSizeX, &textSizeY);

            dtk_int32 textPosX;
            dtk_int32 textPosY;
            switch (pTextBox->style.alignmentHorz)
            {
                case dtk_text_alignment_center:
                {
                    textPosX = innerRect.left + ((dtk_rect_width(innerRect) - textSizeX) / 2);
                } break;
                case dtk_text_alignment_right:
                {
                    textPosX = innerRect.right - textSizeX;
                } break;
                case dtk_text_alignment_left:
                default:
                {
                    textPosX = innerRect.left;
                } break;
            }

            switch (pTextBox->style.alignmentVert)
            {
                case dtk_text_alignment_center:
                {
                    textPosY = innerRect.top + ((dtk_rect_height(innerRect) - textSizeY) / 2);
                } break;
                case dtk_text_alignment_bottom:
                {
                    textPosY = innerRect.bottom - textSizeY;
                } break;
                case dtk_text_alignment_top:
                default:
                {
                    textPosY = innerRect.top;
                } break;
            }

            innerRect.left   = textPosX;
            innerRect.top    = textPosY;
            innerRect.right  = innerRect.left + textSizeX;
            innerRect.bottom = innerRect.top  + textSizeY;
            dtk_surface_draw_outer_rect(pEvent->paint.pSurface, outerRect, innerRect, pTextBox->style.bgColor);

            // Text. TODO: Selections.
            dtk_surface_draw_text(pEvent->paint.pSurface, dtk_textbox_get_font(pTextBox), scale, dtk_textbox_get_text_utf8(pTextBox), (size_t)-1, textPosX, textPosY, pTextBox->style.fgColor, pTextBox->style.bgColor);

            /* Caret. */
            if (dtk_control_has_keyboard_capture(DTK_CONTROL(pTextBox))) {
                dtk_surface_draw_rect(pEvent->paint.pSurface, dtk_textbox_get_caret_rect(pTextBox), pTextBox->style.fgColor);
            }
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        {
            dtk_textbox_move_caret_to_xy(pTextBox, pEvent->mouseButton.x, pEvent->mouseButton.y);
            dtk_control_capture_mouse(pEvent->pControl);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_UP:
        {
            if (dtk_control_has_mouse_capture(pEvent->pControl)) {
                dtk_control_release_mouse(pEvent->pControl);
            }
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            if (dtk_control_has_mouse_capture(pEvent->pControl)) {
                dtk_textbox_move_caret_to_xy(pTextBox, pEvent->mouseMove.x, pEvent->mouseMove.y);
            }
        } break;

        case DTK_EVENT_CAPTURE_KEYBOARD:
        {
            dtk_control_scheduled_redraw(pEvent->pControl, dtk_control_get_local_rect(pEvent->pControl));
        } break;

        case DTK_EVENT_RELEASE_KEYBOARD:
        {
            dtk_control_scheduled_redraw(pEvent->pControl, dtk_control_get_local_rect(pEvent->pControl));
        } break;

        default: break;
    }

    return dtk_control_default_event_handler(pEvent);
}

dtk_bool32 dtk_textbox_default_event_handler__multi_line(dtk_event* pEvent)
{
    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
        } break;

        default: break;
    }

    return dtk_control_default_event_handler(pEvent);
}

dtk_bool32 dtk_textbox_default_event_handler(dtk_event* pEvent)
{
    dtk_textbox* pTextBox = DTK_TEXTBOX(pEvent->pControl);
    dtk_assert(pTextBox != NULL);

    if (pTextBox->mode == dtk_textbox_mode_single_line) {
        return dtk_textbox_default_event_handler__single_line(pEvent);
    } else {
        return dtk_textbox_default_event_handler__multi_line(pEvent);
    }
}
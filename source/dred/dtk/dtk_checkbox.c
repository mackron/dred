// Copyright (C) 2017 David Reid. See included LICENSE file.

dtk_rect dtk_checkbox__get_box_rect(dtk_checkbox* pCheckbox)
{
    // The size of the box is based on the size of the font.
    assert(pCheckbox != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pCheckbox));
    
    dtk_font_metrics metrics;
    dtk_font_get_metrics(pCheckbox->pFont, uiScale, &metrics);

    dtk_rect checkboxRect = dtk_control_get_local_rect(DTK_CONTROL(pCheckbox));

    dtk_rect rect;
    rect.left = 0;
    rect.top  = ((dtk_int32)dtk_control_get_height(DTK_CONTROL(pCheckbox)) - metrics.lineHeight) / 2;
    rect.right = rect.left + metrics.lineHeight;
    rect.bottom = rect.top + metrics.lineHeight;

    // Clip.
    if (rect.top    < 0)                   rect.top    = 0;
    if (rect.left   < 0)                   rect.left   = 0;
    if (rect.right  > checkboxRect.right)  rect.right  = checkboxRect.right;
    if (rect.bottom > checkboxRect.bottom) rect.bottom = checkboxRect.bottom;

    // Make it square.
    dtk_int32 boxSizeX = rect.right - rect.left;
    dtk_int32 boxSizeY = rect.bottom - rect.top;
    if (boxSizeX != boxSizeY) {
        boxSizeX = boxSizeY = dtk_min(boxSizeX, boxSizeY);
        rect.right  = rect.left + boxSizeX;
        rect.bottom = rect.top  + boxSizeY;
    }

    return rect;
}

void dtk_checkbox__on_paint(dtk_checkbox* pCheckbox, dtk_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pCheckbox));

    // Draw the box first.
    dtk_color boxBGColor = pCheckbox->boxBGColor;
    if (dtk_control_has_mouse_capture(DTK_CONTROL(pCheckbox))) {
        boxBGColor = pCheckbox->boxBGColorHovered;
        if (pCheckbox->isMouseOver) {
            boxBGColor = pCheckbox->boxBGColorPressed;
        }
    } else if (pCheckbox->isMouseOver) {
        boxBGColor = pCheckbox->boxBGColorHovered;
    }

    dtk_rect bgrect = dtk_control_get_local_rect(DTK_CONTROL(pCheckbox));
    dtk_rect boxRect = dtk_checkbox__get_box_rect(pCheckbox);

    dtk_surface_draw_rect_outline(pSurface, boxRect, pCheckbox->boxBorderColor, (dtk_int32)pCheckbox->borderWidth);
    dtk_surface_draw_rect(pSurface, dtk_rect_grow(boxRect, -(dtk_int32)pCheckbox->borderWidth), boxBGColor);

    if (pCheckbox->isChecked) {
        dtk_surface_draw_rect(pSurface, dtk_rect_grow(boxRect, -(dtk_int32)pCheckbox->borderWidth - 2), pCheckbox->checkColor);
    }


    // The text is positioned to the right of the box, and centered vertically.
    dtk_int32 textWidth;
    dtk_int32 textHeight;
    dtk_font_measure_string(pCheckbox->pFont, uiScale, pCheckbox->text, strlen(pCheckbox->text), &textWidth, &textHeight);

    dtk_int32 textPosX = boxRect.right + pCheckbox->padding;
    dtk_int32 textPosY = (dtk_control_get_height(DTK_CONTROL(pCheckbox)) - textHeight) / 2;
    dtk_surface_draw_text(pSurface, pCheckbox->pFont, uiScale, pCheckbox->text, strlen(pCheckbox->text), textPosX, textPosY, pCheckbox->textColor, pCheckbox->bgColor);

    // Background
    dtk_surface_draw_rect(pSurface, dtk_rect_init(boxRect.right,        boxRect.top,    boxRect.right + pCheckbox->padding, boxRect.bottom), pCheckbox->bgColor);    // Padding bettween checkbox and text.
    dtk_surface_draw_rect(pSurface, dtk_rect_init(bgrect.left,          bgrect.top,     bgrect.right,                       boxRect.top),    pCheckbox->bgColor);
    dtk_surface_draw_rect(pSurface, dtk_rect_init(bgrect.left,          boxRect.bottom, bgrect.right,                       bgrect.bottom),  pCheckbox->bgColor);
    dtk_surface_draw_rect(pSurface, dtk_rect_init(textPosX + textWidth, boxRect.top,    bgrect.right,                       boxRect.bottom), pCheckbox->bgColor);
}

void dtk_checkbox__on_mouse_enter(dtk_checkbox* pCheckbox)
{
    pCheckbox->isMouseOver = DR_TRUE;
    dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dtk_control_get_local_rect(DTK_CONTROL(pCheckbox)));
}

void dtk_checkbox__on_mouse_leave(dtk_checkbox* pCheckbox)
{
    pCheckbox->isMouseOver = DR_FALSE;
    dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dtk_control_get_local_rect(DTK_CONTROL(pCheckbox)));
}

void dtk_checkbox__on_mouse_move(dtk_checkbox* pCheckbox, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    if (dtk_control_has_mouse_capture(DTK_CONTROL(pCheckbox))) {
        pCheckbox->isMouseOver = (mousePosX >= 0 && mousePosX < (dtk_int32)dtk_control_get_width(DTK_CONTROL(pCheckbox))) && (mousePosY >= 0 && mousePosY < (dtk_int32)dtk_control_get_height(DTK_CONTROL(pCheckbox)));
        dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dtk_control_get_local_rect(DTK_CONTROL(pCheckbox)));
    }
}

void dtk_checkbox__on_mouse_button_down(dtk_checkbox* pCheckbox, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT) {
        if (!dtk_control_has_mouse_capture(DTK_CONTROL(pCheckbox))) {
            dtk_capture_mouse(pCheckbox->control.pTK, DTK_CONTROL(pCheckbox));
            dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dtk_control_get_local_rect(DTK_CONTROL(pCheckbox)));
        }
    }
}

void dtk_checkbox__on_mouse_button_up(dtk_checkbox* pCheckbox, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT) {
        if (dtk_control_has_mouse_capture(DTK_CONTROL(pCheckbox))) {
            dtk_release_mouse(DTK_CONTROL(pCheckbox)->pTK);
            dtk_checkbox_toggle(pCheckbox);
        }
    }
}

void dtk_checkbox__on_release_mouse(dtk_checkbox* pCheckbox)
{
    dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dtk_control_get_local_rect(DTK_CONTROL(pCheckbox)));
}

void dtk_checkbox__refresh_layout(dtk_checkbox* pCheckbox)
{
    dtk_assert(pCheckbox != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pCheckbox));

    if (pCheckbox->isAutoSizeEnabled) {
        dtk_int32 textWidth;
        dtk_int32 textHeight;
        dtk_font_measure_string(pCheckbox->pFont, uiScale, pCheckbox->text, strlen(pCheckbox->text), &textWidth, &textHeight);

        dtk_rect boxRect = dtk_checkbox__get_box_rect(pCheckbox);
        dtk_int32 boxWidth = (boxRect.right - boxRect.left);

        dtk_control_set_size(DTK_CONTROL(pCheckbox), textWidth + boxWidth + pCheckbox->padding, textHeight);
    }

    dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dtk_control_get_local_rect(DTK_CONTROL(pCheckbox)));
}

dtk_result dtk_checkbox_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, const char* text, dtk_bool32 checked, dtk_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pCheckbox);

    dtk_result result = dtk_control_init(pTK, pParent, DTK_CONTROL_TYPE_CHECKBOX, onEvent, &pCheckbox->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    dtk_strncpy_s(pCheckbox->text, sizeof(pCheckbox->text), text, _TRUNCATE);
    pCheckbox->pFont = dtk_get_ui_font(pTK);
    pCheckbox->textColor = dtk_rgb(0, 0, 0);
    pCheckbox->bgColor = dtk_rgb(255, 255, 255);
    pCheckbox->boxBGColor = dtk_rgb(224, 224, 224);
    pCheckbox->boxBGColorHovered = dtk_rgb(224, 240, 255);
    pCheckbox->boxBGColorPressed = dtk_rgb(200, 224, 240);
    pCheckbox->boxBorderColor = dtk_rgb(32, 64, 160);
    pCheckbox->checkColor = dtk_rgb(64, 128, 64);
    pCheckbox->borderWidth = 1;
    pCheckbox->padding = 4;
    pCheckbox->isAutoSizeEnabled = DR_TRUE;
    pCheckbox->isChecked = checked;

    dtk_checkbox__refresh_layout(pCheckbox);
    return DTK_SUCCESS;
}

dtk_result dtk_checkbox_uninit(dtk_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) return DTK_INVALID_ARGS;

    dtk_control_uninit(&pCheckbox->control);
    return DTK_SUCCESS;
}

dtk_bool32 dtk_checkbox_default_event_handler(dtk_event* pEvent)
{
    dtk_checkbox* pCheckbox = DTK_CHECKBOX(pEvent->pControl);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dtk_checkbox__on_paint(pCheckbox, pEvent->paint.rect, pEvent->paint.pSurface);
        } break;

        case DTK_EVENT_MOUSE_ENTER:
        {
            dtk_checkbox__on_mouse_enter(pCheckbox);
        } break;

        case DTK_EVENT_MOUSE_LEAVE:
        {
            dtk_checkbox__on_mouse_leave(pCheckbox);
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            dtk_checkbox__on_mouse_move(pCheckbox, pEvent->mouseMove.x, pEvent->mouseMove.y, pEvent->mouseMove.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        {
            dtk_checkbox__on_mouse_button_down(pCheckbox, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_UP:
        {
            dtk_checkbox__on_mouse_button_up(pCheckbox, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_RELEASE_MOUSE:
        {
            dtk_checkbox__on_release_mouse(pCheckbox);
        } break;

        case DTK_EVENT_CHECKBOX_CHECK_CHANGED:
        {
            if (pCheckbox->onCheckChanged) {
                pCheckbox->onCheckChanged(pCheckbox);
            }
        } break;
    }

    return dtk_control_default_event_handler(pEvent);
}


void dtk_checkbox_set_text(dtk_checkbox* pCheckbox, const char* text)
{
    if (pCheckbox == NULL) return;

    strncpy_s(pCheckbox->text, sizeof(pCheckbox->text), text, _TRUNCATE);
    dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dtk_control_get_local_rect(DTK_CONTROL(pCheckbox)));
}

void dtk_checkbox_enable_auto_size(dtk_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) return;

    pCheckbox->isAutoSizeEnabled = DR_TRUE;
    dtk_checkbox__refresh_layout(pCheckbox);
}

void dtk_checkbox_disable_auto_size(dtk_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) return;

    pCheckbox->isAutoSizeEnabled = DR_FALSE;
    dtk_checkbox__refresh_layout(pCheckbox);
}

void dtk_checkbox_auto_size(dtk_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) return;

    dtk_checkbox__refresh_layout(pCheckbox);
}


void dtk_checkbox_set_font(dtk_checkbox* pCheckbox, dtk_font* pFont)
{
    if (pCheckbox == NULL) return;

    if (pCheckbox->pFont == pFont) {
        return;
    }

    pCheckbox->pFont = pFont;
    dtk_checkbox__refresh_layout(pCheckbox);
}

void dtk_checkbox_set_background_color(dtk_checkbox* pCheckbox, dtk_color color)
{
    if (pCheckbox == NULL) return;

    pCheckbox->bgColor = color;
    dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dtk_control_get_local_rect(DTK_CONTROL(pCheckbox)));
}

void dtk_checkbox_set_border_color(dtk_checkbox* pCheckbox, dtk_color color)
{
    if (pCheckbox == NULL) return;

    pCheckbox->boxBorderColor = color;
    dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dtk_control_get_local_rect(DTK_CONTROL(pCheckbox)));
}

void dtk_checkbox_set_border_width(dtk_checkbox* pCheckbox, dtk_int32 width)
{
    if (pCheckbox == NULL) return;

    pCheckbox->borderWidth = width;
    dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dtk_control_get_local_rect(DTK_CONTROL(pCheckbox)));
}

void dtk_checkbox_set_padding(dtk_checkbox* pCheckbox, dtk_int32 padding)
{
    if (pCheckbox == NULL) return;

    pCheckbox->padding = padding;
    dtk_checkbox__refresh_layout(pCheckbox);
}


void dtk_checkbox_check(dtk_checkbox* pCheckbox)
{
    dtk_checkbox_set_checked(pCheckbox, DR_TRUE, DR_FALSE);
}

void dtk_checkbox_uncheck(dtk_checkbox* pCheckbox)
{
    dtk_checkbox_set_checked(pCheckbox, DR_FALSE, DR_FALSE);
}

void dtk_checkbox_toggle(dtk_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) return;
    
    if (pCheckbox->isChecked) {
        dtk_checkbox_uncheck(pCheckbox);
    } else {
        dtk_checkbox_check(pCheckbox);
    }
}

void dtk_checkbox_set_checked(dtk_checkbox* pCheckbox, dtk_bool32 checked, dtk_bool32 blockEvent)
{
    if (pCheckbox == NULL) return;

    if (pCheckbox->isChecked == checked) {
        return;
    }

    pCheckbox->isChecked = checked;

    if (!blockEvent) {
        dtk_event e = dtk_event_init(DTK_CONTROL(pCheckbox)->pTK, DTK_EVENT_CHECKBOX_CHECK_CHANGED, DTK_CONTROL(pCheckbox));
        e.checkbox.checked = checked;
        dtk_post_local_event(e.pTK, &e);
    }

    dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dtk_checkbox__get_box_rect(pCheckbox));
}

dtk_bool32 dtk_checkbox_is_checked(dtk_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) return DR_FALSE;
    return pCheckbox->isChecked;
}


void dtk_checkbox_set_on_check_changed(dtk_checkbox* pCheckbox, dtk_checkbox_on_check_changed_proc proc)
{
    if (pCheckbox == NULL) return;
    pCheckbox->onCheckChanged = proc;
}
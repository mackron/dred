// Copyright (C) 2017 David Reid. See included LICENSE file.

dtk_rect dtk_color_button__get_box_rect(dtk_color_button* pButton)
{
    // The size of the box is based on the size of the font.
    assert(pButton != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pButton));
    
    dtk_font_metrics metrics;
    dtk_font_get_metrics(pButton->pFont, uiScale, &metrics);

    dtk_rect checkboxRect = dtk_control_get_local_rect(DTK_CONTROL(pButton));

    dtk_rect rect;
    rect.left = 0;
    rect.top  = ((dtk_int32)dtk_control_get_height(DTK_CONTROL(pButton)) - metrics.lineHeight) / 2;
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

void dtk_color_button__on_paint(dtk_color_button* pButton, dtk_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pButton));

    // Draw the box first.
    dtk_color bgColor = pButton->bgColor;
    dtk_rect bgrect = dtk_control_get_local_rect(DTK_CONTROL(pButton));
    dtk_rect boxRect = dtk_color_button__get_box_rect(pButton);

    dtk_surface_draw_rect_outline(pSurface, boxRect, pButton->boxBorderColor, (dtk_int32)(pButton->borderWidth*uiScale));
    dtk_surface_draw_rect(pSurface, dtk_rect_grow(boxRect, -(dtk_int32)(pButton->borderWidth*uiScale)), pButton->color);


    // The text is positioned to the right of the box, and centered vertically.
    dtk_int32 textWidth;
    dtk_int32 textHeight;
    dtk_font_measure_string(pButton->pFont, uiScale, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

    dtk_int32 textPosX = boxRect.right + (dtk_int32)(pButton->padding*uiScale);
    dtk_int32 textPosY = (dtk_control_get_height(DTK_CONTROL(pButton)) - textHeight) / 2;
    dtk_surface_draw_text(pSurface, pButton->pFont, uiScale, pButton->text, strlen(pButton->text), textPosX, textPosY, pButton->textColor, bgColor);

    // Background
    dtk_surface_draw_rect(pSurface, dtk_rect_init(boxRect.right,        boxRect.top,    boxRect.right + (dtk_int32)(pButton->padding*uiScale), boxRect.bottom), bgColor);    // Padding bettween checkbox and text.
    dtk_surface_draw_rect(pSurface, dtk_rect_init(bgrect.left,          bgrect.top,     bgrect.right,                                            boxRect.top),    bgColor);
    dtk_surface_draw_rect(pSurface, dtk_rect_init(bgrect.left,          boxRect.bottom, bgrect.right,                                            bgrect.bottom),  bgColor);
    dtk_surface_draw_rect(pSurface, dtk_rect_init(textPosX + textWidth, boxRect.top,    bgrect.right,                                            boxRect.bottom), bgColor);
}

void dtk_color_button__on_mouse_enter(dtk_color_button* pButton)
{
    pButton->isMouseOver = DTK_FALSE;
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_color_button__on_mouse_leave(dtk_color_button* pButton)
{
    pButton->isMouseOver = DTK_FALSE;
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_color_button__on_mouse_move(dtk_color_button* pButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    if (dtk_control_has_mouse_capture(DTK_CONTROL(pButton))) {
        pButton->isMouseOver = (mousePosX >= 0 && mousePosX < (dtk_int32)dtk_control_get_width(DTK_CONTROL(pButton))) && (mousePosY >= 0 && mousePosY < (dtk_int32)dtk_control_get_height(DTK_CONTROL(pButton)));
        dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
    }
}

void dtk_color_button__on_mouse_button_down(dtk_color_button* pButton, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT) {
        if (!dtk_control_has_mouse_capture(DTK_CONTROL(pButton))) {
            dtk_control_capture_mouse(DTK_CONTROL(pButton));
            dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
        }
    }
}

void dtk_color_button__on_mouse_button_up(dtk_color_button* pButton, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dtk_window* pOwnerWindow = dtk_control_get_window(DTK_CONTROL(pButton));
    assert(pOwnerWindow != NULL);

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT) {
        if (dtk_control_has_mouse_capture(DTK_CONTROL(pButton))) {
            dtk_release_mouse(DTK_CONTROL(pButton)->pTK);
            
            // Show a color picker.
            dtk_color newColor;
            if (dtk_show_color_picker_dialog(DTK_CONTROL(pButton)->pTK, pOwnerWindow, pButton->color, &newColor) == DTK_DIALOG_RESULT_OK) {
                dtk_color_button_set_color(pButton, newColor, DTK_FALSE);
                dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
            }
        }
    }
}

void dtk_color_button__on_release_mouse(dtk_color_button* pButton)
{
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_color_button__refresh_layout(dtk_color_button* pButton)
{
    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pButton));

    if (pButton->isAutoSizeEnabled) {
        dtk_int32 textWidth;
        dtk_int32 textHeight;
        dtk_font_measure_string(pButton->pFont, uiScale, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

        dtk_rect boxRect = dtk_color_button__get_box_rect(pButton);
        dtk_int32 boxWidth = (boxRect.right - boxRect.left);

        dtk_control_set_size(DTK_CONTROL(pButton), textWidth + boxWidth + (dtk_int32)(pButton->padding*uiScale), textHeight);
    }

    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

dtk_result dtk_color_button_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, const char* text, dtk_color color, dtk_color_button* pButton)
{
    if (pButton == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pButton);

    dtk_result result = dtk_control_init(pTK, DTK_CONTROL_TYPE_COLOR_BUTTON, (onEvent != NULL) ? onEvent : dtk_color_button_default_event_handler, pParent, &pButton->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    strncpy_s(pButton->text, sizeof(pButton->text), text, _TRUNCATE);
    pButton->pFont = dtk_get_ui_font(pTK);
    pButton->textColor = dtk_rgb(0, 0, 0);
    pButton->bgColor = dtk_rgb(255, 255, 255);
    pButton->bgColorHovered = dtk_rgb(224, 240, 255);
    pButton->bgColorPressed = dtk_rgb(200, 224, 240);
    pButton->boxBorderColor = dtk_rgb(0, 0, 0);
    pButton->borderWidth = 1;
    pButton->padding = 4;
    pButton->isAutoSizeEnabled = DTK_TRUE;
    pButton->color = color;

    dtk_color_button__refresh_layout(pButton);
    return DTK_SUCCESS;
}

dtk_result dtk_color_button_uninit(dtk_color_button* pButton)
{
    if (pButton == NULL) return DTK_INVALID_ARGS;

    dtk_control_uninit(DTK_CONTROL(pButton));
    return DTK_SUCCESS;
}

dtk_bool32 dtk_color_button_default_event_handler(dtk_event* pEvent)
{
    dtk_color_button* pButton = DTK_COLOR_BUTTON(pEvent->pControl);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dtk_color_button__on_paint(pButton, pEvent->paint.rect, pEvent->paint.pSurface);
        } break;

        case DTK_EVENT_MOUSE_ENTER:
        {
            dtk_color_button__on_mouse_enter(pButton);
        } break;

        case DTK_EVENT_MOUSE_LEAVE:
        {
            dtk_color_button__on_mouse_leave(pButton);
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            dtk_color_button__on_mouse_move(pButton, pEvent->mouseMove.x, pEvent->mouseMove.y, pEvent->mouseMove.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        {
            dtk_color_button__on_mouse_button_down(pButton, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_UP:
        {
            dtk_color_button__on_mouse_button_up(pButton, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_RELEASE_MOUSE:
        {
            dtk_color_button__on_release_mouse(pButton);
        } break;

        case DTK_EVENT_REFRESH_LAYOUT:
        {
            dtk_color_button__refresh_layout(pButton);
        } break;

        case DTK_EVENT_COLOR_BUTTON_COLOR_CHANGED:
        {
            if (pButton->onColorChanged) {
                pButton->onColorChanged(pButton, pEvent->colorButton.color);
            }
        } break;

        default: break;
    }

    return dtk_control_default_event_handler(pEvent);
}


void dtk_color_button_set_text(dtk_color_button* pButton, const char* text)
{
    if (pButton == NULL) return;

    strncpy_s(pButton->text, sizeof(pButton->text), text, _TRUNCATE);
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_color_button_enable_auto_size(dtk_color_button* pButton)
{
    if (pButton == NULL) return;

    pButton->isAutoSizeEnabled = DTK_FALSE;
    dtk_color_button__refresh_layout(pButton);
}

void dtk_color_button_disable_auto_size(dtk_color_button* pButton)
{
    if (pButton == NULL) return;

    pButton->isAutoSizeEnabled = DTK_FALSE;
    dtk_color_button__refresh_layout(pButton);
}

void dtk_color_button_auto_size(dtk_color_button* pButton)
{
    if (pButton == NULL) return;

    dtk_color_button__refresh_layout(pButton);
}


void dtk_color_button_set_font(dtk_color_button* pButton, dtk_font* pFont)
{
    if (pButton == NULL) return;

    if (pButton->pFont == pFont) {
        return;
    }

    pButton->pFont = pFont;
    dtk_color_button__refresh_layout(pButton);
}

void dtk_color_button_set_background_color(dtk_color_button* pButton, dtk_color color)
{
    if (pButton == NULL) return;

    pButton->bgColor = color;
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_color_button_set_border_color(dtk_color_button* pButton, dtk_color color)
{
    if (pButton == NULL) return;

    pButton->boxBorderColor = color;
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_color_button_set_border_width(dtk_color_button* pButton, dtk_int32 width)
{
    if (pButton == NULL) return;

    pButton->borderWidth = width;
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_color_button_set_padding(dtk_color_button* pButton, dtk_int32 padding)
{
    if (pButton == NULL) return;

    pButton->padding = padding;
    dtk_color_button__refresh_layout(pButton);
}


void dtk_color_button_set_color(dtk_color_button* pButton, dtk_color color, dtk_bool32 blockEvent)
{
    if (pButton == NULL) return;

    pButton->color = color;

    if (blockEvent) {
        return;
    }

    dtk_event e = dtk_event_init(DTK_CONTROL(pButton)->pTK, DTK_EVENT_COLOR_BUTTON_COLOR_CHANGED, DTK_CONTROL(pButton));
    e.colorButton.color = color;
    dtk_post_local_event(&e);
}

dtk_color dtk_color_button_get_color(dtk_color_button* pButton)
{
    if (pButton == NULL) return dtk_rgb(0, 0, 0);
    return pButton->color;
}


void dtk_color_button_set_on_color_changed(dtk_color_button* pButton, dtk_color_button_on_color_changed_proc proc)
{
    if (pButton == NULL) return;

    pButton->onColorChanged = proc;
}
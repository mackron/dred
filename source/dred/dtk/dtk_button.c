// Copyright (C) 2017 David Reid. See included LICENSE file.

void dtk_button__on_paint(dtk_button* pButton, dtk_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pButton));

    dtk_color bgColor = pButton->bgColor;
    if (dtk_control_has_mouse_capture(DTK_CONTROL(pButton))) {
        bgColor = pButton->bgColorHovered;
        if (pButton->isMouseOver) {
            bgColor = pButton->bgColorPressed;
        }
    } else if (pButton->isMouseOver) {
        bgColor = pButton->bgColorHovered;
    }


    // Draw the border first.
    dtk_surface_draw_rect_outline(pSurface, dtk_control_get_local_rect(DTK_CONTROL(pButton)), pButton->borderColor, pButton->borderWidth);

    // Text and background. The text is centered.
    dtk_rect bgrect = dtk_rect_grow(dtk_control_get_local_rect(DTK_CONTROL(pButton)), -pButton->borderWidth);

    dtk_int32 textWidth;
    dtk_int32 textHeight;
    dtk_font_measure_string(pButton->pFont, uiScale, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

    dtk_int32 textPosX = ((bgrect.right - bgrect.left) - textWidth) / 2;
    dtk_int32 textPosY = ((bgrect.bottom - bgrect.top) - textHeight) / 2;
    dtk_surface_draw_text(pSurface, pButton->pFont, uiScale, pButton->text, strlen(pButton->text), textPosX, textPosY, pButton->textColor, bgColor);
    
    // Make sure the background does not overdraw the text.
    dtk_surface_draw_rect(pSurface, dtk_rect_init(bgrect.left,          bgrect.top,            textPosX,             bgrect.bottom), bgColor);
    dtk_surface_draw_rect(pSurface, dtk_rect_init(textPosX + textWidth, bgrect.top,            bgrect.right,         bgrect.bottom), bgColor);
    dtk_surface_draw_rect(pSurface, dtk_rect_init(textPosX,             bgrect.top,            textPosX + textWidth, textPosY),      bgColor);
    dtk_surface_draw_rect(pSurface, dtk_rect_init(textPosX,             textPosY + textHeight, textPosX + textWidth, bgrect.bottom), bgColor);
}

void dtk_button__on_mouse_enter(dtk_button* pButton)
{
    pButton->isMouseOver = DTK_TRUE;
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_button__on_mouse_leave(dtk_button* pButton)
{
    pButton->isMouseOver = DTK_FALSE;
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_button__on_mouse_move(dtk_button* pButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    if (dtk_control_has_mouse_capture(DTK_CONTROL(pButton))) {
        pButton->isMouseOver = (mousePosX >= 0 && mousePosX < (dtk_int32)dtk_control_get_width(DTK_CONTROL(pButton))) && (mousePosY >= 0 && mousePosY < (dtk_int32)dtk_control_get_height(DTK_CONTROL(pButton)));
        dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
    }
}

void dtk_button__on_mouse_button_down(dtk_button* pButton, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT) {
        if (!dtk_control_has_mouse_capture(DTK_CONTROL(pButton))) {
            dtk_capture_mouse(DTK_CONTROL(pButton)->pTK, DTK_CONTROL(pButton));

            // Redraw to show the pressed state.
            dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
        }
    }
}

void dtk_button__on_mouse_button_up(dtk_button* pButton, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT) {
        if (dtk_control_has_mouse_capture(DTK_CONTROL(pButton))) {
            dtk_release_mouse(DTK_CONTROL(pButton)->pTK);

            if (dtk_control_is_under_mouse(DTK_CONTROL(pButton))) {
                dtk_event e = dtk_event_init(DTK_CONTROL(pButton)->pTK, DTK_EVENT_BUTTON_PRESSED, DTK_CONTROL(pButton));
                dtk_post_local_event(&e);
            }
        }
    }
}

void dtk_button__on_release_mouse(dtk_button* pButton)
{
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_button__refresh_layout(dtk_button* pButton)
{
    if (pButton == NULL) return;

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pButton));

    if (pButton->isAutoSizeEnabled) {
        dtk_int32 textWidth;
        dtk_int32 textHeight;
        dtk_font_measure_string(pButton->pFont, uiScale, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

        dtk_control_set_size(DTK_CONTROL(pButton), textWidth + (pButton->paddingHorz*2), textHeight + (pButton->paddingVert*2));
    }

    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}


dtk_result dtk_button_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, const char* text, dtk_button* pButton)
{
    if (pButton == NULL) return DTK_FALSE;
    dtk_zero_object(pButton);

    dtk_result result = dtk_control_init(pTK, DTK_CONTROL_TYPE_BUTTON, (onEvent != NULL) ? onEvent : dtk_button_default_event_handler, pParent, DTK_CONTROL(pButton));
    if (result != DTK_SUCCESS) {
        return result;
    }

    strncpy_s(pButton->text, sizeof(pButton->text), text, _TRUNCATE);
    pButton->pFont = dtk_get_ui_font(pTK);
    pButton->textColor = dtk_rgb(32, 32, 32);
    pButton->bgColor = dtk_rgb(224, 224, 224);
    pButton->bgColorHovered = dtk_rgb(224, 240, 255);
    pButton->bgColorPressed = dtk_rgb(200, 224, 240);
    pButton->borderColor = dtk_rgb(32, 64, 160);
    pButton->borderWidth = 1;
    pButton->paddingHorz = 16;
    pButton->paddingVert = 4;
    pButton->isAutoSizeEnabled = DTK_TRUE;

    dtk_button__refresh_layout(pButton);

    return DTK_SUCCESS;
}

void dtk_button_uninit(dtk_button* pButton)
{
    dtk_control_uninit(DTK_CONTROL(pButton));
}

dtk_bool32 dtk_button_default_event_handler(dtk_event* pEvent)
{
    dtk_button* pButton = DTK_BUTTON(pEvent->pControl);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dtk_button__on_paint(pButton, pEvent->paint.rect, pEvent->paint.pSurface);
        } break;

        case DTK_EVENT_MOUSE_ENTER:
        {
            dtk_button__on_mouse_enter(pButton);
        } break;

        case DTK_EVENT_MOUSE_LEAVE:
        {
            dtk_button__on_mouse_leave(pButton);
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            dtk_button__on_mouse_move(pButton, pEvent->mouseMove.x, pEvent->mouseMove.y, pEvent->mouseMove.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        {
            dtk_button__on_mouse_button_down(pButton, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_UP:
        {
            dtk_button__on_mouse_button_up(pButton, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_RELEASE_MOUSE:
        {
            dtk_button__on_release_mouse(pButton);
        };

        case DTK_EVENT_REFRESH_LAYOUT:
        {
            dtk_button__refresh_layout(pButton);
        } break;

        case DTK_EVENT_BUTTON_PRESSED:
        {
            if (pButton->onPressed) {
                pButton->onPressed(pButton);
            }
        } break;

        default: break;
    }

    return dtk_control_default_event_handler(pEvent);
}


void dtk_button_set_text(dtk_button* pButton, const char* text)
{
    if (pButton == NULL) return;

    strncpy_s(pButton->text, sizeof(pButton->text), text, _TRUNCATE);
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_button_enable_auto_size(dtk_button* pButton)
{
    if (pButton == NULL) return;

    pButton->isAutoSizeEnabled = DTK_TRUE;
    dtk_button__refresh_layout(pButton);
}

void dtk_button_disable_auto_size(dtk_button* pButton)
{
    if (pButton == NULL) return;

    pButton->isAutoSizeEnabled = DTK_FALSE;
    dtk_button__refresh_layout(pButton);
}


void dtk_button_set_font(dtk_button* pButton, dtk_font* pFont)
{
    if (pButton == NULL) return;

    if (pButton->pFont == pFont) {
        return;
    }

    pButton->pFont = pFont;
    dtk_button__refresh_layout(pButton);
}

void dtk_button_set_background_color(dtk_button* pButton, dtk_color color)
{
    if (pButton == NULL) return;

    pButton->bgColor = color;
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_button_set_border_color(dtk_button* pButton, dtk_color color)
{
    if (pButton == NULL) return;

    pButton->borderColor = color;
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_button_set_border_width(dtk_button* pButton, dtk_int32 width)
{
    if (pButton == NULL) return;

    pButton->borderWidth = width;
    dtk_control_scheduled_redraw(DTK_CONTROL(pButton), dtk_control_get_local_rect(DTK_CONTROL(pButton)));
}

void dtk_button_set_padding(dtk_button* pButton, dtk_int32 paddingHorz, dtk_int32 paddingVert)
{
    if (pButton == NULL) return;

    pButton->paddingHorz = paddingHorz;
    pButton->paddingVert = paddingVert;
    dtk_button__refresh_layout(pButton);
}


void dtk_button_set_on_pressed(dtk_button* pButton, dtk_button_on_pressed_proc proc)
{
    if (pButton == NULL) return;

    pButton->onPressed = proc;
}
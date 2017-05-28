// Copyright (C) 2017 David Reid. See included LICENSE file.

void dred_button__on_paint(dred_control* pControl, dred_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dred_button* pButton = DRED_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pControl));


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
    dred_control_draw_rect_outline(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)), pButton->borderColor, pButton->borderWidth, pSurface);

    // Text and background. The text is centered.
    dred_rect bgrect = dred_grow_rect(dred_control_get_local_rect(DRED_CONTROL(pButton)), -pButton->borderWidth);

    float textWidth;
    float textHeight;
    dtk_font_measure_string(&pButton->pFont->fontDTK, uiScale, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

    float textPosX = roundf(((bgrect.right - bgrect.left) - textWidth) / 2);
    float textPosY = roundf(((bgrect.bottom - bgrect.top) - textHeight) / 2);
    dred_control_draw_text(DRED_CONTROL(pButton), &pButton->pFont->fontDTK, uiScale, pButton->text, (int)strlen(pButton->text), textPosX, textPosY, pButton->textColor, bgColor, pSurface);
    
    // Make sure the background does not overdraw the text.
    dred_control_draw_rect(DRED_CONTROL(pButton), dred_make_rect(bgrect.left, bgrect.top, textPosX, bgrect.bottom), bgColor, pSurface);
    dred_control_draw_rect(DRED_CONTROL(pButton), dred_make_rect(textPosX + textWidth, bgrect.top, bgrect.right, bgrect.bottom), bgColor, pSurface);
    dred_control_draw_rect(DRED_CONTROL(pButton), dred_make_rect(textPosX, bgrect.top, textPosX + textWidth, textPosY), bgColor, pSurface);
    dred_control_draw_rect(DRED_CONTROL(pButton), dred_make_rect(textPosX, textPosY + textHeight, textPosX + textWidth, bgrect.bottom), bgColor, pSurface);
}

void dred_button__on_mouse_enter(dred_control* pControl)
{
    dred_button* pButton = DRED_BUTTON(pControl);
    assert(pButton != NULL);

    pButton->isMouseOver = DR_TRUE;

    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_button__on_mouse_leave(dred_control* pControl)
{
    dred_button* pButton = DRED_BUTTON(pControl);
    assert(pButton != NULL);

    pButton->isMouseOver = DR_FALSE;

    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_button__on_mouse_move(dred_control* pControl, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_button* pButton = DRED_BUTTON(pControl);
    assert(pButton != NULL);

    if (dtk_control_has_mouse_capture(DTK_CONTROL(pButton))) {
        pButton->isMouseOver = (mousePosX >= 0 && mousePosX < dred_control_get_width(DRED_CONTROL(pButton))) && (mousePosY >= 0 && mousePosY < dred_control_get_height(DRED_CONTROL(pButton)));
        dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
    }
}

void dred_button__on_mouse_button_down(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_button* pButton = DRED_BUTTON(pControl);
    assert(pButton != NULL);

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT) {
        if (!dtk_control_has_mouse_capture(DTK_CONTROL(pButton))) {
            dred_gui_capture_mouse(DRED_CONTROL(pButton));

            // Redraw to show the pressed state.
            dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
        }
    }
}

void dred_button__on_mouse_button_up(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_button* pButton = DRED_BUTTON(pControl);
    assert(pButton != NULL);

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT) {
        if (dtk_control_has_mouse_capture(DTK_CONTROL(pButton))) {
            dred_gui_release_mouse(dred_control_get_gui(DRED_CONTROL(pButton)));

            if (pButton->onPressed && dred_control_is_under_mouse(DRED_CONTROL(pButton))) {
                pButton->onPressed(pButton);
            }
        }
    }
}

void dred_button__on_release_mouse(dred_control* pControl)
{
    dred_button* pButton = DRED_BUTTON(pControl);
    assert(pButton != NULL);

    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_button__refresh_layout(dred_button* pButton)
{
    if (pButton == NULL) {
        return;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pButton));

    if (pButton->isAutoSizeEnabled) {
        float textWidth;
        float textHeight;
        dtk_font_measure_string(&pButton->pFont->fontDTK, uiScale, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

        dred_control_set_size(DRED_CONTROL(pButton), textWidth + (pButton->paddingHorz*2), textHeight + (pButton->paddingVert*2));
    }


    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

dr_bool32 dred_button_init(dred_button* pButton, dred_context* pDred, dred_control* pParent, const char* text)
{
    if (pButton == NULL) {
        return DR_FALSE;
    }

    memset(pButton, 0, sizeof(*pButton));
    if (!dred_control_init(DRED_CONTROL(pButton), pDred, pParent, NULL, DRED_CONTROL_TYPE_BUTTON)) {
        return DR_FALSE;
    }

    strncpy_s(pButton->text, sizeof(pButton->text), text, _TRUNCATE);
    pButton->pFont = pDred->config.pUIFont;
    pButton->textColor = dred_rgb(32, 32, 32);
    pButton->bgColor = dred_rgb(224, 224, 224);
    pButton->bgColorHovered = dred_rgb(224, 240, 255);
    pButton->bgColorPressed = dred_rgb(200, 224, 240);
    pButton->borderColor = dred_rgb(32, 64, 160);
    pButton->borderWidth = 1;
    pButton->paddingHorz = 16;
    pButton->paddingVert = 4;
    pButton->isAutoSizeEnabled = DR_TRUE;

    // Events.
    dred_control_set_on_paint(DRED_CONTROL(pButton), dred_button__on_paint);
    dred_control_set_on_mouse_enter(DRED_CONTROL(pButton), dred_button__on_mouse_enter);
    dred_control_set_on_mouse_leave(DRED_CONTROL(pButton), dred_button__on_mouse_leave);
    dred_control_set_on_mouse_move(DRED_CONTROL(pButton), dred_button__on_mouse_move);
    dred_control_set_on_mouse_button_down(DRED_CONTROL(pButton), dred_button__on_mouse_button_down);
    dred_control_set_on_mouse_button_up(DRED_CONTROL(pButton), dred_button__on_mouse_button_up);
    dred_control_set_on_release_mouse(DRED_CONTROL(pButton), dred_button__on_release_mouse);

    dred_button__refresh_layout(pButton);

    return DR_TRUE;
}

void dred_button_uninit(dred_button* pButton)
{
    dred_control_uninit(DRED_CONTROL(pButton));
}


void dred_button_set_text(dred_button* pButton, const char* text)
{
    if (pButton == NULL) {
        return;
    }

    strncpy_s(pButton->text, sizeof(pButton->text), text, _TRUNCATE);


    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_button_enable_auto_size(dred_button* pButton)
{
    if (pButton == NULL) {
        return;
    }

    pButton->isAutoSizeEnabled = DR_TRUE;

    dred_button__refresh_layout(pButton);
}

void dred_button_disable_auto_size(dred_button* pButton)
{
    if (pButton == NULL) {
        return;
    }

    pButton->isAutoSizeEnabled = DR_FALSE;

    dred_button__refresh_layout(pButton);
}


void dred_button_set_font(dred_button* pButton, dred_font* pFont)
{
    if (pButton == NULL) {
        return;
    }

    if (pButton->pFont == pFont) {
        return;
    }

    pButton->pFont = pFont;

    dred_button__refresh_layout(pButton);
}

void dred_button_set_background_color(dred_button* pButton, dtk_color color)
{
    if (pButton == NULL) {
        return;
    }

    pButton->bgColor = color;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_button_set_border_color(dred_button* pButton, dtk_color color)
{
    if (pButton == NULL) {
        return;
    }

    pButton->borderColor = color;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_button_set_border_width(dred_button* pButton, float width)
{
    if (pButton == NULL) {
        return;
    }

    pButton->borderWidth = width;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_button_set_padding(dred_button* pButton, float paddingHorz, float paddingVert)
{
    if (pButton == NULL) {
        return;
    }

    pButton->paddingHorz = paddingHorz;
    pButton->paddingVert = paddingVert;

    dred_button__refresh_layout(pButton);
}


void dred_button_set_on_pressed(dred_button* pButton, dred_button_on_pressed_proc proc)
{
    if (pButton == NULL) {
        return;
    }

    pButton->onPressed = proc;
}
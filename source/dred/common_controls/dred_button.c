// Copyright (C) 2016 David Reid. See included LICENSE file.

void dred_button__on_paint(dred_control* pControl, dred_rect rect, void* pPaintData)
{
    (void)rect;

    dred_button* pButton = DRED_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pButton));
    if (pDred == NULL) {
        return;
    }


    dred_color bgColor = pButton->bgColor;
    if (dred_control_has_mouse_capture(DRED_CONTROL(pButton))) {
        bgColor = pButton->bgColorHovered;
        if (pButton->isMouseOver) {
            bgColor = pButton->bgColorPressed;
        }
    } else if (pButton->isMouseOver) {
        bgColor = pButton->bgColorHovered;
    }


    // Draw the border first.
    dred_control_draw_rect_outline(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)), pButton->borderColor, pButton->borderWidth, pPaintData);

    // Text and background. The text is centered.
    dred_rect bgrect = dred_grow_rect(dred_control_get_local_rect(DRED_CONTROL(pButton)), -pButton->borderWidth);

    float textWidth;
    float textHeight;
    dred_gui_measure_string(pButton->pSubFont, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

    float textPosX = roundf(((bgrect.right - bgrect.left) - textWidth) / 2);
    float textPosY = roundf(((bgrect.bottom - bgrect.top) - textHeight) / 2);
    dred_control_draw_text(DRED_CONTROL(pButton), pButton->pSubFont, pButton->text, (int)strlen(pButton->text), textPosX, textPosY, pButton->textColor, bgColor, pPaintData);
    
    // Make sure the background does not overdraw the text.
    dred_control_draw_rect(DRED_CONTROL(pButton), dred_make_rect(bgrect.left, bgrect.top, textPosX, bgrect.bottom), bgColor, pPaintData);
    dred_control_draw_rect(DRED_CONTROL(pButton), dred_make_rect(textPosX + textWidth, bgrect.top, bgrect.right, bgrect.bottom), bgColor, pPaintData);
    dred_control_draw_rect(DRED_CONTROL(pButton), dred_make_rect(textPosX, bgrect.top, textPosX + textWidth, textPosY), bgColor, pPaintData);
    dred_control_draw_rect(DRED_CONTROL(pButton), dred_make_rect(textPosX, textPosY + textHeight, textPosX + textWidth, bgrect.bottom), bgColor, pPaintData);
}

void dred_button__on_mouse_enter(dred_control* pControl)
{
    dred_button* pButton = DRED_BUTTON(pControl);
    assert(pButton != NULL);

    pButton->isMouseOver = true;

    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_button__on_mouse_leave(dred_control* pControl)
{
    dred_button* pButton = DRED_BUTTON(pControl);
    assert(pButton != NULL);

    pButton->isMouseOver = false;

    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_button__on_mouse_move(dred_control* pControl, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_button* pButton = DRED_BUTTON(pControl);
    assert(pButton != NULL);

    if (dred_control_has_mouse_capture(DRED_CONTROL(pButton))) {
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

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (!dred_control_has_mouse_capture(DRED_CONTROL(pButton))) {
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

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (dred_control_has_mouse_capture(DRED_CONTROL(pButton))) {
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

    if (pButton->isAutoSizeEnabled) {
        float textWidth;
        float textHeight;
        dred_gui_measure_string(pButton->pSubFont, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

        dred_control_set_size(DRED_CONTROL(pButton), textWidth + (pButton->paddingHorz*2), textHeight + (pButton->paddingVert*2));
    }


    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

dred_button* dred_button_create(dred_context* pDred, dred_control* pParent, const char* text)
{
    dred_button* pButton = (dred_button*)calloc(1, sizeof(*pButton));
    if (pButton == NULL) {
        return NULL;
    }

    if (!dred_control_init(DRED_CONTROL(pButton), pDred, pParent, DRED_CONTROL_TYPE_BUTTON)) {
        free(pButton);
        return NULL;
    }


    strncpy_s(pButton->text, sizeof(pButton->text), text, _TRUNCATE);
    pButton->pFont = pDred->config.pUIFont;
    pButton->pSubFont = dred_font_acquire_subfont(pButton->pFont, pDred->uiScale);
    pButton->textColor = dred_rgb(32, 32, 32);
    pButton->bgColor = dred_rgb(224, 224, 224);
    pButton->bgColorHovered = dred_rgb(224, 240, 255);
    pButton->bgColorPressed = dred_rgb(200, 224, 240);
    pButton->borderColor = dred_rgb(32, 64, 160);
    pButton->borderWidth = 1;
    pButton->paddingHorz = 16;
    pButton->paddingVert = 4;
    pButton->isAutoSizeEnabled = true;

    // Events.
    dred_control_set_on_paint(DRED_CONTROL(pButton), dred_button__on_paint);
    dred_control_set_on_mouse_enter(DRED_CONTROL(pButton), dred_button__on_mouse_enter);
    dred_control_set_on_mouse_leave(DRED_CONTROL(pButton), dred_button__on_mouse_leave);
    dred_control_set_on_mouse_move(DRED_CONTROL(pButton), dred_button__on_mouse_move);
    dred_control_set_on_mouse_button_down(DRED_CONTROL(pButton), dred_button__on_mouse_button_down);
    dred_control_set_on_mouse_button_up(DRED_CONTROL(pButton), dred_button__on_mouse_button_up);
    dred_control_set_on_release_mouse(DRED_CONTROL(pButton), dred_button__on_release_mouse);

    dred_button__refresh_layout(pButton);

    return pButton;
}

void dred_button_delete(dred_button* pButton)
{
    dred_font_release_subfont(pButton->pFont, pButton->pSubFont);
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

    pButton->isAutoSizeEnabled = true;

    dred_button__refresh_layout(pButton);
}

void dred_button_disable_auto_size(dred_button* pButton)
{
    if (pButton == NULL) {
        return;
    }

    pButton->isAutoSizeEnabled = false;

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

    dred_font_release_subfont(pButton->pFont, pButton->pSubFont);

    pButton->pFont = pFont;
    pButton->pSubFont = dred_font_acquire_subfont(pButton->pFont, dred_control_get_context(DRED_CONTROL(pButton))->uiScale);

    dred_button__refresh_layout(pButton);
}

void dred_button_set_background_color(dred_button* pButton, dred_color color)
{
    if (pButton == NULL) {
        return;
    }

    pButton->bgColor = color;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_button_set_border_color(dred_button* pButton, dred_color color)
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
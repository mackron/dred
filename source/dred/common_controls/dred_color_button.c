// Copyright (C) 2016 David Reid. See included LICENSE file.

dred_rect dred_colorbutton__get_box_rect(dred_colorbutton* pButton)
{
    // The size of the box is based on the size of the font.
    assert(pButton != NULL);

    assert(pButton != NULL);
    
    dred_gui_font_metrics metrics;
    dred_gui_get_font_metrics(pButton->pSubFont, &metrics);

    float posX = 0;
    float posY = (dred_control_get_height(DRED_CONTROL(pButton)) - metrics.lineHeight) / 2;
    return dred_make_rect(posX, posY, posX + metrics.lineHeight, posY + metrics.lineHeight);
}

void dred_colorbutton__on_paint(dred_control* pControl, dred_rect rect, void* pPaintData)
{
    (void)rect;

    dred_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pControl);
    if (pDred == NULL) {
        return;
    }

    // Draw the box first.
    dred_color bgColor = pButton->bgColor;
    /*if (dred_control_has_mouse_capture(pButton)) {
        boxBGColor = pButton->boxBGColorHovered;
        if (pButton->isMouseOver) {
            boxBGColor = pButton->boxBGColorPressed;
        }
    } else if (pButton->isMouseOver) {
        boxBGColor = pButton->boxBGColorHovered;
    }*/

    dred_rect bgrect = dred_control_get_local_rect(DRED_CONTROL(pButton));
    dred_rect boxRect = dred_colorbutton__get_box_rect(pButton);

    dred_control_draw_rect_outline(pControl, boxRect, pButton->boxBorderColor, pButton->borderWidth, pPaintData);
    dred_control_draw_rect(pControl, dred_grow_rect(boxRect, -pButton->borderWidth), pButton->color, pPaintData);


    // The text is positioned to the right of the box, and centered vertically.
    float textWidth;
    float textHeight;
    dred_gui_measure_string(pButton->pSubFont, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

    float textPosX = boxRect.right + pButton->padding;
    float textPosY = roundf(dred_control_get_height(pControl) - textHeight) / 2;
    dred_control_draw_text(pControl, pButton->pSubFont, pButton->text, (int)strlen(pButton->text), textPosX, textPosY, pButton->textColor, bgColor, pPaintData);

    // Background
    dred_control_draw_rect(pControl, dred_make_rect(boxRect.right, boxRect.top, boxRect.right + pButton->padding, boxRect.bottom), bgColor, pPaintData);    // Padding bettween colorbutton and text.
    dred_control_draw_rect(pControl, dred_make_rect(bgrect.left, bgrect.top, bgrect.right, boxRect.top), bgColor, pPaintData);
    dred_control_draw_rect(pControl, dred_make_rect(bgrect.left, boxRect.bottom, bgrect.right, bgrect.bottom), bgColor, pPaintData);
    dred_control_draw_rect(pControl, dred_make_rect(textPosX + textWidth, boxRect.top, bgrect.right, boxRect.bottom), bgColor, pPaintData);
}

void dred_colorbutton__on_mouse_enter(dred_control* pControl)
{
    dred_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    pButton->isMouseOver = true;

    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dred_colorbutton__on_mouse_leave(dred_control* pControl)
{
    dred_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    pButton->isMouseOver = false;

    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dred_colorbutton__on_mouse_move(dred_control* pControl, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    if (dred_control_has_mouse_capture(pControl)) {
        pButton->isMouseOver = (mousePosX >= 0 && mousePosX < dred_control_get_width(pControl)) && (mousePosY >= 0 && mousePosY < dred_control_get_height(pControl));
        dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
    }
}

void dred_colorbutton__on_mouse_button_down(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (!dred_control_has_mouse_capture(pControl)) {
            dred_gui_capture_mouse(pControl);

            // Redraw to show the pressed state.
            dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
        }
    }
}

void dred_colorbutton__on_mouse_button_up(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_window* pOwnerWindow = dred_get_control_window(pControl);
    assert(pOwnerWindow != NULL);

    dred_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (dred_control_has_mouse_capture(pControl)) {
            dred_gui_release_mouse(dred_control_get_gui(pControl));
            
            // Show a color picker.
            dred_color newColor;
            if (dred_show_color_picker_dialog(pOwnerWindow->pDred, pOwnerWindow, pButton->color, &newColor)) {
                dred_colorbutton_set_color(pButton, newColor, false);
                dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
            }
        }
    }
}

void dred_colorbutton__on_release_mouse(dred_control* pControl)
{
    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dred_colorbutton__refresh_layout(dred_colorbutton* pButton)
{
    if (pButton == NULL) {
        return;
    }

    if (pButton->isAutoSizeEnabled) {
        float textWidth;
        float textHeight;
        dred_gui_measure_string(pButton->pSubFont, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

        dred_rect boxRect = dred_colorbutton__get_box_rect(pButton);
        float boxWidth = (boxRect.right - boxRect.left);

        dred_control_set_size(DRED_CONTROL(pButton), textWidth + boxWidth + pButton->padding, textHeight);
    }


    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

bool dred_colorbutton_init(dred_colorbutton* pButton, dred_context* pDred, dred_control* pParent, const char* text, dred_color color)
{
    if (pButton == NULL) {
        return false;
    }

    memset(pButton, 0, sizeof(*pButton));
    if (!dred_control_init(DRED_CONTROL(pButton), pDred, pParent, DRED_CONTROL_TYPE_COLOR_BUTTON)) {
        return false;
    }

    strncpy_s(pButton->text, sizeof(pButton->text), text, _TRUNCATE);
    pButton->pFont = pDred->config.pUIFont;
    pButton->pSubFont = dred_font_acquire_subfont(pButton->pFont, pDred->uiScale);
    pButton->textColor = dred_rgb(0, 0, 0);
    pButton->bgColor = dred_rgb(255, 255, 255);
    pButton->bgColorHovered = dred_rgb(224, 240, 255);
    pButton->bgColorPressed = dred_rgb(200, 224, 240);
    pButton->boxBorderColor = dred_rgb(0, 0, 0);
    pButton->borderWidth = 1;
    pButton->padding = 4;
    pButton->isAutoSizeEnabled = true;
    pButton->color = color;

    // Events.
    dred_control_set_on_paint(DRED_CONTROL(pButton), dred_colorbutton__on_paint);
    dred_control_set_on_mouse_enter(DRED_CONTROL(pButton), dred_colorbutton__on_mouse_enter);
    dred_control_set_on_mouse_leave(DRED_CONTROL(pButton), dred_colorbutton__on_mouse_leave);
    dred_control_set_on_mouse_move(DRED_CONTROL(pButton), dred_colorbutton__on_mouse_move);
    dred_control_set_on_mouse_button_down(DRED_CONTROL(pButton), dred_colorbutton__on_mouse_button_down);
    dred_control_set_on_mouse_button_up(DRED_CONTROL(pButton), dred_colorbutton__on_mouse_button_up);
    dred_control_set_on_release_mouse(DRED_CONTROL(pButton), dred_colorbutton__on_release_mouse);

    dred_colorbutton__refresh_layout(pButton);

    return true;
}

void dred_colorbutton_uninit(dred_colorbutton* pButton)
{
    if (pButton == NULL) {
        return;
    }

    dred_font_release_subfont(pButton->pFont, pButton->pSubFont);
    dred_control_uninit(DRED_CONTROL(pButton));
}


void dred_colorbutton_set_text(dred_colorbutton* pButton, const char* text)
{
    if (pButton == NULL) {
        return;
    }

    strncpy_s(pButton->text, sizeof(pButton->text), text, _TRUNCATE);


    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_colorbutton_enable_auto_size(dred_colorbutton* pButton)
{
    if (pButton == NULL) {
        return;
    }

    pButton->isAutoSizeEnabled = true;

    dred_colorbutton__refresh_layout(pButton);
}

void dred_colorbutton_disable_auto_size(dred_colorbutton* pButton)
{
    if (pButton == NULL) {
        return;
    }

    pButton->isAutoSizeEnabled = false;

    dred_colorbutton__refresh_layout(pButton);
}


void dred_colorbutton_set_font(dred_colorbutton* pButton, dred_font* pFont)
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

    dred_colorbutton__refresh_layout(pButton);
}

void dred_colorbutton_set_background_color(dred_colorbutton* pButton, dred_color color)
{
    if (pButton == NULL) {
        return;
    }

    pButton->bgColor = color;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_colorbutton_set_border_color(dred_colorbutton* pButton, dred_color color)
{
    if (pButton == NULL) {
        return;
    }

    pButton->boxBorderColor = color;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_colorbutton_set_border_width(dred_colorbutton* pButton, float width)
{
    if (pButton == NULL) {
        return;
    }

    pButton->borderWidth = width;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dred_colorbutton_set_padding(dred_colorbutton* pButton, float padding)
{
    if (pButton == NULL) {
        return;
    }

    pButton->padding = padding;

    dred_colorbutton__refresh_layout(pButton);
}


void dred_colorbutton_set_color(dred_colorbutton* pButton, dred_color color, bool blockEvent)
{
    if (pButton == NULL) {
        return;
    }

    pButton->color = color;

    if (blockEvent) {
        return;
    }

    if (pButton->varBinding[0] != '\0') {
        char colorStr[256];
        snprintf(colorStr, sizeof(colorStr), "%d %d %d %d", color.r, color.g, color.b, color.a);
        dred_config_set(&dred_control_get_context(DRED_CONTROL(pButton))->config, pButton->varBinding, colorStr);
    }

    if (pButton->onColorChanged) {
        pButton->onColorChanged(pButton, color);
    }
}

dred_color dred_colorbutton_get_color(dred_colorbutton* pButton)
{
    if (pButton == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pButton->color;
}


void dred_colorbutton_set_bind_to_config_var(dred_colorbutton* pButton, const char* varName)
{
    if (pButton == NULL) {
        return;
    }

    if (varName != NULL) {
        strcpy_s(pButton->varBinding, sizeof(pButton->varBinding), varName);
    } else {
        pButton->varBinding[0] = '\0';
    }
}

void dred_colorbutton_set_on_color_changed(dred_colorbutton* pButton, dred_colorbutton_on_color_changed_proc proc)
{
    if (pButton == NULL) {
        return;
    }

    pButton->onColorChanged = proc;
}
// Copyright (C) 2017 David Reid. See included LICENSE file.

dtk_rect dtk_colorbutton__get_box_rect(dtk_colorbutton* pButton)
{
    // The size of the box is based on the size of the font.
    assert(pButton != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pButton));
    
    dtk_font_metrics metrics;
    dtk_font_get_metrics(&pButton->pFont->fontDTK, uiScale, &metrics);

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

void dtk_colorbutton__on_paint(dred_control* pControl, dred_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dtk_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pControl);
    if (pDred == NULL) {
        return;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pButton));

    // Draw the box first.
    dtk_color bgColor = pButton->bgColor;
    /*if (dred_control_has_mouse_capture(pButton)) {
        boxBGColor = pButton->boxBGColorHovered;
        if (pButton->isMouseOver) {
            boxBGColor = pButton->boxBGColorPressed;
        }
    } else if (pButton->isMouseOver) {
        boxBGColor = pButton->boxBGColorHovered;
    }*/

    dtk_rect bgrect = dtk_control_get_local_rect(DTK_CONTROL(pButton));
    dtk_rect boxRect = dtk_colorbutton__get_box_rect(pButton);

    dtk_surface_draw_rect_outline(pSurface, boxRect, pButton->boxBorderColor, (dtk_int32)pButton->borderWidth);
    dtk_surface_draw_rect(pSurface, dtk_rect_grow(boxRect, -(dtk_int32)pButton->borderWidth), pButton->color);


    // The text is positioned to the right of the box, and centered vertically.
    float textWidth;
    float textHeight;
    dtk_font_measure_string(&pButton->pFont->fontDTK, uiScale, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

    float textPosX = boxRect.right + pButton->padding;
    float textPosY = roundf(dred_control_get_height(pControl) - textHeight) / 2;
    dred_control_draw_text(pControl, &pButton->pFont->fontDTK, uiScale, pButton->text, (int)strlen(pButton->text), textPosX, textPosY, pButton->textColor, bgColor, pSurface);

    // Background
    dred_control_draw_rect(pControl, dred_make_rect((float)boxRect.right,        (float)boxRect.top,    (float)boxRect.right + pButton->padding, (float)boxRect.bottom), bgColor, pSurface);    // Padding bettween checkbox and text.
    dred_control_draw_rect(pControl, dred_make_rect((float)bgrect.left,          (float)bgrect.top,     (float)bgrect.right,                     (float)boxRect.top),    bgColor, pSurface);
    dred_control_draw_rect(pControl, dred_make_rect((float)bgrect.left,          (float)boxRect.bottom, (float)bgrect.right,                     (float)bgrect.bottom),  bgColor, pSurface);
    dred_control_draw_rect(pControl, dred_make_rect((float)textPosX + textWidth, (float)boxRect.top,    (float)bgrect.right,                     (float)boxRect.bottom), bgColor, pSurface);
}

void dtk_colorbutton__on_mouse_enter(dred_control* pControl)
{
    dtk_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    pButton->isMouseOver = DR_TRUE;

    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dtk_colorbutton__on_mouse_leave(dred_control* pControl)
{
    dtk_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    pButton->isMouseOver = DR_FALSE;

    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dtk_colorbutton__on_mouse_move(dred_control* pControl, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dtk_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    if (dtk_control_has_mouse_capture(DTK_CONTROL(pControl))) {
        pButton->isMouseOver = (mousePosX >= 0 && mousePosX < dred_control_get_width(pControl)) && (mousePosY >= 0 && mousePosY < dred_control_get_height(pControl));
        dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
    }
}

void dtk_colorbutton__on_mouse_button_down(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dtk_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT) {
        if (!dtk_control_has_mouse_capture(DTK_CONTROL(pControl))) {
            dred_gui_capture_mouse(pControl);

            // Redraw to show the pressed state.
            dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
        }
    }
}

void dtk_colorbutton__on_mouse_button_up(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_window* pOwnerWindow = dred_get_control_window(pControl);
    assert(pOwnerWindow != NULL);

    dtk_colorbutton* pButton = DRED_COLOR_BUTTON(pControl);
    if (pButton == NULL) {
        return;
    }

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT) {
        if (dtk_control_has_mouse_capture(DTK_CONTROL(pControl))) {
            dred_gui_release_mouse(dred_control_get_gui(pControl));
            
            // Show a color picker.
            dtk_color newColor;
            if (dred_show_color_picker_dialog(pOwnerWindow->pDred, pOwnerWindow, pButton->color, &newColor)) {
                dtk_colorbutton_set_color(pButton, newColor, DR_FALSE);
                dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
            }
        }
    }
}

void dtk_colorbutton__on_release_mouse(dred_control* pControl)
{
    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dtk_colorbutton__refresh_layout(dtk_colorbutton* pButton)
{
    if (pButton == NULL) {
        return;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pButton));

    if (pButton->isAutoSizeEnabled) {
        float textWidth;
        float textHeight;
        dtk_font_measure_string(&pButton->pFont->fontDTK, uiScale, pButton->text, strlen(pButton->text), &textWidth, &textHeight);

        dtk_rect boxRect = dtk_colorbutton__get_box_rect(pButton);
        dtk_int32 boxWidth = (boxRect.right - boxRect.left);

        dred_control_set_size(DRED_CONTROL(pButton), textWidth + boxWidth + pButton->padding, textHeight);
    }


    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

dr_bool32 dtk_colorbutton_init(dtk_colorbutton* pButton, dred_context* pDred, dred_control* pParent, const char* text, dtk_color color)
{
    if (pButton == NULL) {
        return DR_FALSE;
    }

    memset(pButton, 0, sizeof(*pButton));
    if (!dred_control_init(DRED_CONTROL(pButton), pDred, pParent, NULL, DRED_CONTROL_TYPE_COLOR_BUTTON)) {
        return DR_FALSE;
    }

    strncpy_s(pButton->text, sizeof(pButton->text), text, _TRUNCATE);
    pButton->pFont = pDred->config.pUIFont;
    pButton->textColor = dred_rgb(0, 0, 0);
    pButton->bgColor = dred_rgb(255, 255, 255);
    pButton->bgColorHovered = dred_rgb(224, 240, 255);
    pButton->bgColorPressed = dred_rgb(200, 224, 240);
    pButton->boxBorderColor = dred_rgb(0, 0, 0);
    pButton->borderWidth = 1;
    pButton->padding = 4;
    pButton->isAutoSizeEnabled = DR_TRUE;
    pButton->color = color;

    // Events.
    dred_control_set_on_paint(DRED_CONTROL(pButton), dtk_colorbutton__on_paint);
    dred_control_set_on_mouse_enter(DRED_CONTROL(pButton), dtk_colorbutton__on_mouse_enter);
    dred_control_set_on_mouse_leave(DRED_CONTROL(pButton), dtk_colorbutton__on_mouse_leave);
    dred_control_set_on_mouse_move(DRED_CONTROL(pButton), dtk_colorbutton__on_mouse_move);
    dred_control_set_on_mouse_button_down(DRED_CONTROL(pButton), dtk_colorbutton__on_mouse_button_down);
    dred_control_set_on_mouse_button_up(DRED_CONTROL(pButton), dtk_colorbutton__on_mouse_button_up);
    dred_control_set_on_release_mouse(DRED_CONTROL(pButton), dtk_colorbutton__on_release_mouse);

    dtk_colorbutton__refresh_layout(pButton);

    return DR_TRUE;
}

void dtk_colorbutton_uninit(dtk_colorbutton* pButton)
{
    if (pButton == NULL) {
        return;
    }

    dred_control_uninit(DRED_CONTROL(pButton));
}


void dtk_colorbutton_set_text(dtk_colorbutton* pButton, const char* text)
{
    if (pButton == NULL) {
        return;
    }

    strncpy_s(pButton->text, sizeof(pButton->text), text, _TRUNCATE);


    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dtk_colorbutton_enable_auto_size(dtk_colorbutton* pButton)
{
    if (pButton == NULL) {
        return;
    }

    pButton->isAutoSizeEnabled = DR_TRUE;

    dtk_colorbutton__refresh_layout(pButton);
}

void dtk_colorbutton_disable_auto_size(dtk_colorbutton* pButton)
{
    if (pButton == NULL) {
        return;
    }

    pButton->isAutoSizeEnabled = DR_FALSE;

    dtk_colorbutton__refresh_layout(pButton);
}


void dtk_colorbutton_set_font(dtk_colorbutton* pButton, dred_font* pFont)
{
    if (pButton == NULL) {
        return;
    }

    if (pButton->pFont == pFont) {
        return;
    }

    pButton->pFont = pFont;
    dtk_colorbutton__refresh_layout(pButton);
}

void dtk_colorbutton_set_background_color(dtk_colorbutton* pButton, dtk_color color)
{
    if (pButton == NULL) {
        return;
    }

    pButton->bgColor = color;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dtk_colorbutton_set_border_color(dtk_colorbutton* pButton, dtk_color color)
{
    if (pButton == NULL) {
        return;
    }

    pButton->boxBorderColor = color;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dtk_colorbutton_set_border_width(dtk_colorbutton* pButton, float width)
{
    if (pButton == NULL) {
        return;
    }

    pButton->borderWidth = width;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pButton), dred_control_get_local_rect(DRED_CONTROL(pButton)));
}

void dtk_colorbutton_set_padding(dtk_colorbutton* pButton, float padding)
{
    if (pButton == NULL) {
        return;
    }

    pButton->padding = padding;

    dtk_colorbutton__refresh_layout(pButton);
}


void dtk_colorbutton_set_color(dtk_colorbutton* pButton, dtk_color color, dr_bool32 blockEvent)
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

dtk_color dtk_colorbutton_get_color(dtk_colorbutton* pButton)
{
    if (pButton == NULL) {
        return dred_rgb(0, 0, 0);
    }

    return pButton->color;
}


void dtk_colorbutton_set_bind_to_config_var(dtk_colorbutton* pButton, const char* varName)
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

void dtk_colorbutton_set_on_color_changed(dtk_colorbutton* pButton, dtk_colorbutton_on_color_changed_proc proc)
{
    if (pButton == NULL) {
        return;
    }

    pButton->onColorChanged = proc;
}
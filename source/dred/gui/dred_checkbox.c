// Copyright (C) 2017 David Reid. See included LICENSE file.

dtk_rect dred_checkbox__get_box_rect(dred_checkbox* pCheckbox)
{
    // The size of the box is based on the size of the font.
    assert(pCheckbox != NULL);
    
    dtk_font_metrics metrics;
    dtk_font_get_metrics(&pCheckbox->pFont->fontDTK, DRED_CONTROL(pCheckbox)->pGUI->pDred->uiScale, &metrics);

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

void dred_checkbox__on_paint(dred_control* pControl, dred_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dred_checkbox* pCheckbox = DRED_CHECKBOX(pControl);
    if (pCheckbox == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pControl);
    if (pDred == NULL) {
        return;
    }

    // Draw the box first.
    dtk_color boxBGColor = pCheckbox->boxBGColor;
    if (dtk_control_has_mouse_capture(DTK_CONTROL(pControl))) {
        boxBGColor = pCheckbox->boxBGColorHovered;
        if (pCheckbox->isMouseOver) {
            boxBGColor = pCheckbox->boxBGColorPressed;
        }
    } else if (pCheckbox->isMouseOver) {
        boxBGColor = pCheckbox->boxBGColorHovered;
    }

    dtk_rect bgrect = dtk_control_get_local_rect(DTK_CONTROL(pCheckbox));
    dtk_rect boxRect = dred_checkbox__get_box_rect(pCheckbox);

    dtk_surface_draw_rect_outline(pSurface, boxRect, pCheckbox->boxBorderColor, (dtk_int32)pCheckbox->borderWidth);
    dtk_surface_draw_rect(pSurface, dtk_rect_grow(boxRect, -(dtk_int32)pCheckbox->borderWidth), boxBGColor);

    if (pCheckbox->isChecked) {
        dtk_surface_draw_rect(pSurface, dtk_rect_grow(boxRect, -(dtk_int32)pCheckbox->borderWidth - 2), pCheckbox->checkColor);
    }


    // The text is positioned to the right of the box, and centered vertically.
    float textWidth;
    float textHeight;
    dtk_font_measure_string(&pCheckbox->pFont->fontDTK, DRED_CONTROL(pCheckbox)->pGUI->pDred->uiScale, pCheckbox->text, strlen(pCheckbox->text), &textWidth, &textHeight);

    float textPosX = boxRect.right + pCheckbox->padding;
    float textPosY = roundf(dred_control_get_height(pControl) - textHeight) / 2;
    dred_control_draw_text(pControl, &pCheckbox->pFont->fontDTK, DRED_CONTROL(pCheckbox)->pGUI->pDred->uiScale, pCheckbox->text, (int)strlen(pCheckbox->text), textPosX, textPosY, pCheckbox->textColor, pCheckbox->bgColor, pSurface);

    // Background
    dred_control_draw_rect(pControl, dred_make_rect((float)boxRect.right,        (float)boxRect.top,    (float)boxRect.right + pCheckbox->padding, (float)boxRect.bottom), pCheckbox->bgColor, pSurface);    // Padding bettween checkbox and text.
    dred_control_draw_rect(pControl, dred_make_rect((float)bgrect.left,          (float)bgrect.top,     (float)bgrect.right,                       (float)boxRect.top),    pCheckbox->bgColor, pSurface);
    dred_control_draw_rect(pControl, dred_make_rect((float)bgrect.left,          (float)boxRect.bottom, (float)bgrect.right,                       (float)bgrect.bottom),  pCheckbox->bgColor, pSurface);
    dred_control_draw_rect(pControl, dred_make_rect((float)textPosX + textWidth, (float)boxRect.top,    (float)bgrect.right,                       (float)boxRect.bottom), pCheckbox->bgColor, pSurface);
}

void dred_checkbox__on_mouse_enter(dred_control* pControl)
{
    dred_checkbox* pCheckbox = DRED_CHECKBOX(pControl);
    if (pCheckbox == NULL) {
        return;
    }

    pCheckbox->isMouseOver = DR_TRUE;

    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dred_checkbox__on_mouse_leave(dred_control* pControl)
{
    dred_checkbox* pCheckbox = DRED_CHECKBOX(pControl);
    if (pCheckbox == NULL) {
        return;
    }

    pCheckbox->isMouseOver = DR_FALSE;

    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dred_checkbox__on_mouse_move(dred_control* pControl, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_checkbox* pCheckbox = DRED_CHECKBOX(pControl);
    if (pCheckbox == NULL) {
        return;
    }

    if (dtk_control_has_mouse_capture(DTK_CONTROL(pControl))) {
        pCheckbox->isMouseOver = (mousePosX >= 0 && mousePosX < dred_control_get_width(pControl)) && (mousePosY >= 0 && mousePosY < dred_control_get_height(pControl));
        dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
    }
}

void dred_checkbox__on_mouse_button_down(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_checkbox* pCheckbox = DRED_CHECKBOX(pControl);
    if (pCheckbox == NULL) {
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

void dred_checkbox__on_mouse_button_up(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_checkbox* pCheckbox = DRED_CHECKBOX(pControl);
    if (pCheckbox == NULL) {
        return;
    }

    if (mouseButton == DTK_MOUSE_BUTTON_LEFT) {
        if (dtk_control_has_mouse_capture(DTK_CONTROL(pControl))) {
            dred_gui_release_mouse(pControl->pGUI);
            dred_checkbox_toggle(pCheckbox);
        }
    }
}

void dred_checkbox__on_release_mouse(dred_control* pControl)
{
    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dred_checkbox__refresh_layout(dred_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) {
        return;
    }

    if (pCheckbox->isAutoSizeEnabled) {
        float textWidth;
        float textHeight;
        dtk_font_measure_string(&pCheckbox->pFont->fontDTK, DRED_CONTROL(pCheckbox)->pGUI->pDred->uiScale, pCheckbox->text, strlen(pCheckbox->text), &textWidth, &textHeight);

        dtk_rect boxRect = dred_checkbox__get_box_rect(pCheckbox);
        dtk_int32 boxWidth = (boxRect.right - boxRect.left);

        dred_control_set_size(DRED_CONTROL(pCheckbox), textWidth + boxWidth + pCheckbox->padding, textHeight);
    }


    // Redraw.
    dred_control_dirty(DRED_CONTROL(pCheckbox), dred_control_get_local_rect(DRED_CONTROL(pCheckbox)));
}

dr_bool32 dred_checkbox_init(dred_checkbox* pCheckbox, dred_context* pDred, dred_control* pParent, const char* text, dr_bool32 checked)
{
    if (pCheckbox == NULL) {
        return DR_FALSE;
    }

    memset(pCheckbox, 0, sizeof(*pCheckbox));
    if (!dred_control_init(DRED_CONTROL(pCheckbox), pDred, pParent, NULL, DRED_CONTROL_TYPE_CHECKBOX)) {
        return DR_FALSE;
    }

    strncpy_s(pCheckbox->text, sizeof(pCheckbox->text), text, _TRUNCATE);
    pCheckbox->pFont = pDred->config.pUIFont;
    pCheckbox->textColor = dred_rgb(0, 0, 0);
    pCheckbox->bgColor = dred_rgb(255, 255, 255);
    pCheckbox->boxBGColor = dred_rgb(224, 224, 224);
    pCheckbox->boxBGColorHovered = dred_rgb(224, 240, 255);
    pCheckbox->boxBGColorPressed = dred_rgb(200, 224, 240);
    pCheckbox->boxBorderColor = dred_rgb(32, 64, 160);
    pCheckbox->checkColor = dred_rgb(64, 128, 64);
    pCheckbox->borderWidth = 1;
    pCheckbox->padding = 4;
    pCheckbox->isAutoSizeEnabled = DR_TRUE;
    pCheckbox->isChecked = checked;

    // Events.
    dred_control_set_on_paint(DRED_CONTROL(pCheckbox), dred_checkbox__on_paint);
    dred_control_set_on_mouse_enter(DRED_CONTROL(pCheckbox), dred_checkbox__on_mouse_enter);
    dred_control_set_on_mouse_leave(DRED_CONTROL(pCheckbox), dred_checkbox__on_mouse_leave);
    dred_control_set_on_mouse_move(DRED_CONTROL(pCheckbox), dred_checkbox__on_mouse_move);
    dred_control_set_on_mouse_button_down(DRED_CONTROL(pCheckbox), dred_checkbox__on_mouse_button_down);
    dred_control_set_on_mouse_button_up(DRED_CONTROL(pCheckbox), dred_checkbox__on_mouse_button_up);
    dred_control_set_on_release_mouse(DRED_CONTROL(pCheckbox), dred_checkbox__on_release_mouse);

    dred_checkbox__refresh_layout(pCheckbox);

    return DR_TRUE;
}

void dred_checkbox_uninit(dred_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) {
        return;
    }

    dred_control_uninit(DRED_CONTROL(pCheckbox));
}


void dred_checkbox_set_text(dred_checkbox* pCheckbox, const char* text)
{
    if (pCheckbox == NULL) {
        return;
    }

    strncpy_s(pCheckbox->text, sizeof(pCheckbox->text), text, _TRUNCATE);


    // Redraw.
    dred_control_dirty(DRED_CONTROL(pCheckbox), dred_control_get_local_rect(DRED_CONTROL(pCheckbox)));
}

void dred_checkbox_enable_auto_size(dred_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) {
        return;
    }

    pCheckbox->isAutoSizeEnabled = DR_TRUE;

    dred_checkbox__refresh_layout(pCheckbox);
}

void dred_checkbox_disable_auto_size(dred_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) {
        return;
    }

    pCheckbox->isAutoSizeEnabled = DR_FALSE;

    dred_checkbox__refresh_layout(pCheckbox);
}


void dred_checkbox_set_font(dred_checkbox* pCheckbox, dred_font* pFont)
{
    if (pCheckbox == NULL) {
        return;
    }

    if (pCheckbox->pFont == pFont) {
        return;
    }

    pCheckbox->pFont = pFont;
    dred_checkbox__refresh_layout(pCheckbox);
}

void dred_checkbox_set_background_color(dred_checkbox* pCheckbox, dtk_color color)
{
    if (pCheckbox == NULL) {
        return;
    }

    pCheckbox->bgColor = color;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pCheckbox), dred_control_get_local_rect(DRED_CONTROL(pCheckbox)));
}

void dred_checkbox_set_border_color(dred_checkbox* pCheckbox, dtk_color color)
{
    if (pCheckbox == NULL) {
        return;
    }

    pCheckbox->boxBorderColor = color;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pCheckbox), dred_control_get_local_rect(DRED_CONTROL(pCheckbox)));
}

void dred_checkbox_set_border_width(dred_checkbox* pCheckbox, float width)
{
    if (pCheckbox == NULL) {
        return;
    }

    pCheckbox->borderWidth = width;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pCheckbox), dred_control_get_local_rect(DRED_CONTROL(pCheckbox)));
}

void dred_checkbox_set_padding(dred_checkbox* pCheckbox, float padding)
{
    if (pCheckbox == NULL) {
        return;
    }

    pCheckbox->padding = padding;

    dred_checkbox__refresh_layout(pCheckbox);
}


void dred_checkbox_check(dred_checkbox* pCheckbox)
{
    dred_checkbox_set_checked(pCheckbox, DR_TRUE, DR_FALSE);
}

void dred_checkbox_uncheck(dred_checkbox* pCheckbox)
{
    dred_checkbox_set_checked(pCheckbox, DR_FALSE, DR_FALSE);
}

void dred_checkbox_toggle(dred_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) {
        return;
    }
    
    if (pCheckbox->isChecked) {
        dred_checkbox_uncheck(pCheckbox);
    } else {
        dred_checkbox_check(pCheckbox);
    }
}

void dred_checkbox_set_checked(dred_checkbox* pCheckbox, dr_bool32 checked, dr_bool32 blockEvent)
{
    if (pCheckbox == NULL) {
        return;
    }

    if (pCheckbox->isChecked == checked) {
        return;
    }

    pCheckbox->isChecked = checked;

    if (!blockEvent) {
        if (pCheckbox->varBinding[0] != '\0') {
            dred_config_set(&dred_control_get_context(DRED_CONTROL(pCheckbox))->config, pCheckbox->varBinding, checked ? "true" : "false");
        }

        if (pCheckbox->onCheckChanged) {
            pCheckbox->onCheckChanged(pCheckbox);
        }
    }

    //dred_control_dirty(DRED_CONTROL(pCheckbox), dred_checkbox__get_box_rect(pCheckbox));
    dtk_control_scheduled_redraw(DTK_CONTROL(pCheckbox), dred_checkbox__get_box_rect(pCheckbox));
}

dr_bool32 dred_is_checked(dred_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) {
        return DR_FALSE;
    }

    return pCheckbox->isChecked;
}


void dred_checkbox_set_bind_to_config_var(dred_checkbox* pCheckbox, const char* varName)
{
    if (pCheckbox == NULL) {
        return;
    }

    if (varName != NULL) {
        strcpy_s(pCheckbox->varBinding, sizeof(pCheckbox->varBinding), varName);
    } else {
        pCheckbox->varBinding[0] = '\0';
    }
}

void dred_checkbox_set_on_checked_changed(dred_checkbox* pCheckbox, dred_checkbox_on_checked_changed_proc proc)
{
    if (pCheckbox == NULL) {
        return;
    }

    pCheckbox->onCheckChanged = proc;
}
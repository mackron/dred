// Copyright (C) 2016 David Reid. See included LICENSE file.

dred_rect dred_checkbox__get_box_rect(dred_checkbox* pData)
{
    // The size of the box is based on the size of the font.
    assert(pData != NULL);
    
    dred_gui_font_metrics metrics;
    dred_gui_get_font_metrics(pData->pSubFont, &metrics);

    float posX = 0;
    float posY = (dred_control_get_height(DRED_CONTROL(pData)) - metrics.lineHeight) / 2;
    return dred_make_rect(posX, posY, posX + metrics.lineHeight, posY + metrics.lineHeight);
}

void dred_checkbox__on_paint(dred_control* pControl, dred_rect rect, void* pPaintData)
{
    (void)rect;

    dred_checkbox* pData = DRED_CHECKBOX(pControl);
    if (pData == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pControl);
    if (pDred == NULL) {
        return;
    }

    // Draw the box first.
    dred_color boxBGColor = pData->boxBGColor;
    if (dred_control_has_mouse_capture(pControl)) {
        boxBGColor = pData->boxBGColorHovered;
        if (pData->isMouseOver) {
            boxBGColor = pData->boxBGColorPressed;
        }
    } else if (pData->isMouseOver) {
        boxBGColor = pData->boxBGColorHovered;
    }

    dred_rect bgrect = dred_control_get_local_rect(DRED_CONTROL(pData));
    dred_rect boxRect = dred_checkbox__get_box_rect(pData);

    dred_control_draw_rect_outline(pControl, boxRect, pData->boxBorderColor, pData->borderWidth, pPaintData);
    dred_control_draw_rect(pControl, dred_grow_rect(boxRect, -pData->borderWidth), boxBGColor, pPaintData);

    if (pData->isChecked) {
        dred_control_draw_rect(pControl, dred_grow_rect(boxRect, -pData->borderWidth - 2), pData->checkColor, pPaintData);
    }


    // The text is positioned to the right of the box, and centered vertically.
    float textWidth;
    float textHeight;
    dred_gui_measure_string(pData->pSubFont, pData->text, strlen(pData->text), &textWidth, &textHeight);

    float textPosX = boxRect.right + pData->padding;
    float textPosY = roundf(dred_control_get_height(pControl) - textHeight) / 2;
    dred_control_draw_text(pControl, pData->pSubFont, pData->text, (int)strlen(pData->text), textPosX, textPosY, pData->textColor, pData->bgColor, pPaintData);

    // Background
    dred_control_draw_rect(pControl, dred_make_rect(boxRect.right, boxRect.top, boxRect.right + pData->padding, boxRect.bottom), pData->bgColor, pPaintData);    // Padding bettween checkbox and text.
    dred_control_draw_rect(pControl, dred_make_rect(bgrect.left, bgrect.top, bgrect.right, boxRect.top), pData->bgColor, pPaintData);
    dred_control_draw_rect(pControl, dred_make_rect(bgrect.left, boxRect.bottom, bgrect.right, bgrect.bottom), pData->bgColor, pPaintData);
    dred_control_draw_rect(pControl, dred_make_rect(textPosX + textWidth, boxRect.top, bgrect.right, boxRect.bottom), pData->bgColor, pPaintData);
}

void dred_checkbox__on_mouse_enter(dred_control* pControl)
{
    dred_checkbox* pData = DRED_CHECKBOX(pControl);
    if (pData == NULL) {
        return;
    }

    pData->isMouseOver = true;

    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dred_checkbox__on_mouse_leave(dred_control* pControl)
{
    dred_checkbox* pData = DRED_CHECKBOX(pControl);
    if (pData == NULL) {
        return;
    }

    pData->isMouseOver = false;

    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dred_checkbox__on_mouse_move(dred_control* pControl, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_checkbox* pData = DRED_CHECKBOX(pControl);
    if (pData == NULL) {
        return;
    }

    if (dred_control_has_mouse_capture(pControl)) {
        pData->isMouseOver = (mousePosX >= 0 && mousePosX < dred_control_get_width(pControl)) && (mousePosY >= 0 && mousePosY < dred_control_get_height(pControl));
        dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
    }
}

void dred_checkbox__on_mouse_button_down(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_checkbox* pData = DRED_CHECKBOX(pControl);
    if (pData == NULL) {
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

void dred_checkbox__on_mouse_button_up(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_checkbox* pData = DRED_CHECKBOX(pControl);
    if (pData == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (dred_control_has_mouse_capture(pControl)) {
            dred_gui_release_mouse(pControl->pGUI);
            dred_checkbox_toggle(pData);
        }
    }
}

void dred_checkbox__on_release_mouse(dred_control* pControl)
{
    dred_control_dirty(pControl, dred_control_get_local_rect(pControl));
}

void dred_checkbox__refresh_layout(dred_checkbox* pData)
{
    if (pData == NULL) {
        return;
    }

    if (pData->isAutoSizeEnabled) {
        float textWidth;
        float textHeight;
        dred_gui_measure_string(pData->pSubFont, pData->text, strlen(pData->text), &textWidth, &textHeight);

        dred_rect boxRect = dred_checkbox__get_box_rect(pData);
        float boxWidth = (boxRect.right - boxRect.left);

        dred_control_set_size(DRED_CONTROL(pData), textWidth + boxWidth + pData->padding, textHeight);
    }


    // Redraw.
    dred_control_dirty(DRED_CONTROL(pData), dred_control_get_local_rect(DRED_CONTROL(pData)));
}

dred_checkbox* dred_checkbox_create(dred_context* pDred, dred_control* pParent, const char* text, bool checked)
{
    dred_checkbox* pData = (dred_checkbox*)calloc(1, sizeof(*pData));
    if (pData == NULL) {
        return NULL;
    }

    if (!dred_control_init(DRED_CONTROL(pData), pDred, pParent, DRED_CONTROL_TYPE_CHECKBOX)) {
        free(pData);
        return NULL;
    }

    strncpy_s(pData->text, sizeof(pData->text), text, _TRUNCATE);
    pData->pFont = pDred->config.pUIFont;
    pData->pSubFont = dred_font_acquire_subfont(pData->pFont, pDred->uiScale);
    pData->textColor = dred_rgb(0, 0, 0);
    pData->bgColor = dred_rgb(255, 255, 255);
    pData->boxBGColor = dred_rgb(224, 224, 224);
    pData->boxBGColorHovered = dred_rgb(224, 240, 255);
    pData->boxBGColorPressed = dred_rgb(200, 224, 240);
    pData->boxBorderColor = dred_rgb(32, 64, 160);
    pData->checkColor = dred_rgb(64, 128, 64);
    pData->borderWidth = 1;
    pData->padding = 4;
    pData->isAutoSizeEnabled = true;
    pData->isChecked = checked;

    // Events.
    dred_control_set_on_paint(DRED_CONTROL(pData), dred_checkbox__on_paint);
    dred_control_set_on_mouse_enter(DRED_CONTROL(pData), dred_checkbox__on_mouse_enter);
    dred_control_set_on_mouse_leave(DRED_CONTROL(pData), dred_checkbox__on_mouse_leave);
    dred_control_set_on_mouse_move(DRED_CONTROL(pData), dred_checkbox__on_mouse_move);
    dred_control_set_on_mouse_button_down(DRED_CONTROL(pData), dred_checkbox__on_mouse_button_down);
    dred_control_set_on_mouse_button_up(DRED_CONTROL(pData), dred_checkbox__on_mouse_button_up);
    dred_control_set_on_release_mouse(DRED_CONTROL(pData), dred_checkbox__on_release_mouse);

    dred_checkbox__refresh_layout(pData);

    return pData;
}

void dred_checkbox_delete(dred_checkbox* pData)
{
    if (pData == NULL) {
        return;
    }

    dred_font_release_subfont(pData->pFont, pData->pSubFont);
    dred_control_uninit(DRED_CONTROL(pData));
    free(pData);
}


void dred_checkbox_set_text(dred_checkbox* pData, const char* text)
{
    if (pData == NULL) {
        return;
    }

    strncpy_s(pData->text, sizeof(pData->text), text, _TRUNCATE);


    // Redraw.
    dred_control_dirty(DRED_CONTROL(pData), dred_control_get_local_rect(DRED_CONTROL(pData)));
}

void dred_checkbox_enable_auto_size(dred_checkbox* pData)
{
    if (pData == NULL) {
        return;
    }

    pData->isAutoSizeEnabled = true;

    dred_checkbox__refresh_layout(pData);
}

void dred_checkbox_disable_auto_size(dred_checkbox* pData)
{
    if (pData == NULL) {
        return;
    }

    pData->isAutoSizeEnabled = false;

    dred_checkbox__refresh_layout(pData);
}


void dred_checkbox_set_font(dred_checkbox* pData, dred_font* pFont)
{
    if (pData == NULL) {
        return;
    }

    if (pData->pFont == pFont) {
        return;
    }

    dred_font_release_subfont(pData->pFont, pData->pSubFont);

    pData->pFont = pFont;
    pData->pSubFont = dred_font_acquire_subfont(pData->pFont, dred_control_get_context(DRED_CONTROL(pData))->uiScale);

    dred_checkbox__refresh_layout(pData);
}

void dred_checkbox_set_background_color(dred_checkbox* pData, dred_color color)
{
    if (pData == NULL) {
        return;
    }

    pData->bgColor = color;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pData), dred_control_get_local_rect(DRED_CONTROL(pData)));
}

void dred_checkbox_set_border_color(dred_checkbox* pData, dred_color color)
{
    if (pData == NULL) {
        return;
    }

    pData->boxBorderColor = color;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pData), dred_control_get_local_rect(DRED_CONTROL(pData)));
}

void dred_checkbox_set_border_width(dred_checkbox* pData, float width)
{
    if (pData == NULL) {
        return;
    }

    pData->borderWidth = width;

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pData), dred_control_get_local_rect(DRED_CONTROL(pData)));
}

void dred_checkbox_set_padding(dred_checkbox* pData, float padding)
{
    if (pData == NULL) {
        return;
    }

    pData->padding = padding;

    dred_checkbox__refresh_layout(pData);
}


void dred_checkbox_check(dred_checkbox* pData)
{
    dred_checkbox_set_checked(pData, true, false);
}

void dred_checkbox_uncheck(dred_checkbox* pData)
{
    dred_checkbox_set_checked(pData, false, false);
}

void dred_checkbox_toggle(dred_checkbox* pData)
{
    if (pData == NULL) {
        return;
    }
    
    if (pData->isChecked) {
        dred_checkbox_uncheck(pData);
    } else {
        dred_checkbox_check(pData);
    }
}

void dred_checkbox_set_checked(dred_checkbox* pData, bool checked, bool blockEvent)
{
    if (pData == NULL) {
        return;
    }

    if (pData->isChecked == checked) {
        return;
    }

    pData->isChecked = checked;

    if (!blockEvent) {
        if (pData->varBinding[0] != '\0') {
            dred_config_set(&dred_control_get_context(DRED_CONTROL(pData))->config, pData->varBinding, checked ? "true" : "false");
        }

        if (pData->onCheckChanged) {
            pData->onCheckChanged(pData);
        }
    }

    dred_control_dirty(DRED_CONTROL(pData), dred_checkbox__get_box_rect(pData));
}

bool dred_is_checked(dred_checkbox* pData)
{
    if (pData == NULL) {
        return false;
    }

    return pData->isChecked;
}


void dred_checkbox_set_bind_to_config_var(dred_checkbox* pData, const char* varName)
{
    if (pData == NULL) {
        return;
    }

    if (varName != NULL) {
        strcpy_s(pData->varBinding, sizeof(pData->varBinding), varName);
    } else {
        pData->varBinding[0] = '\0';
    }
}

void dred_checkbox_set_on_checked_changed(dred_checkbox* pData, dred_checkbox_on_checked_changed_proc proc)
{
    if (pData == NULL) {
        return;
    }

    pData->onCheckChanged = proc;
}
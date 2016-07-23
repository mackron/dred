// Copyright (C) 2016 David Reid. See included LICENSE file.


typedef struct
{
    char text[64];
    dred_font* pFont;
    dred_gui_font* pSubFont;
    dred_color textColor;
    dred_color bgColor;
    dred_color boxBGColor;
    dred_color boxBGColorHovered;
    dred_color boxBGColorPressed;
    dred_color boxBorderColor;
    dred_color checkColor;
    float borderWidth;
    float padding;
    bool isMouseOver;
    bool isAutoSizeEnabled;
    bool isChecked;
    char varBinding[128];
    dred_checkbox_on_checked_changed_proc onCheckChanged;
} dred_checkbox_data;

dred_rect dred_checkbox__get_box_rect(dred_checkbox* pCheckbox)
{
    // The size of the box is based on the size of the font.
    assert(pCheckbox != NULL);

    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    assert(pData != NULL);
    
    dred_gui_font_metrics metrics;
    drgui_get_font_metrics(pData->pSubFont, &metrics);

    float posX = 0;
    float posY = (drgui_get_height(pCheckbox) - metrics.lineHeight) / 2;
    return drgui_make_rect(posX, posY, posX + metrics.lineHeight, posY + metrics.lineHeight);
}

void dred_checkbox__on_paint(dred_checkbox* pCheckbox, dred_rect rect, void* pPaintData)
{
    (void)rect;

    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_gui(pCheckbox);
    if (pDred == NULL) {
        return;
    }

    // Draw the box first.
    dred_color boxBGColor = pData->boxBGColor;
    if (dred_control_has_mouse_capture(pCheckbox)) {
        boxBGColor = pData->boxBGColorHovered;
        if (pData->isMouseOver) {
            boxBGColor = pData->boxBGColorPressed;
        }
    } else if (pData->isMouseOver) {
        boxBGColor = pData->boxBGColorHovered;
    }

    dred_rect bgrect = drgui_get_local_rect(pCheckbox);
    dred_rect boxRect = dred_checkbox__get_box_rect(pCheckbox);

    drgui_draw_rect_outline(pCheckbox, boxRect, pData->boxBorderColor, pData->borderWidth, pPaintData);
    drgui_draw_rect(pCheckbox, drgui_grow_rect(boxRect, -pData->borderWidth), boxBGColor, pPaintData);

    if (pData->isChecked) {
        drgui_draw_rect(pCheckbox, drgui_grow_rect(boxRect, -pData->borderWidth - 2), pData->checkColor, pPaintData);
    }


    // The text is positioned to the right of the box, and centered vertically.
    float textWidth;
    float textHeight;
    drgui_measure_string(pData->pSubFont, pData->text, strlen(pData->text), &textWidth, &textHeight);

    float textPosX = boxRect.right + pData->padding;
    float textPosY = roundf(drgui_get_height(pCheckbox) - textHeight) / 2;
    drgui_draw_text(pCheckbox, pData->pSubFont, pData->text, (int)strlen(pData->text), textPosX, textPosY, pData->textColor, pData->bgColor, pPaintData);

    // Background
    drgui_draw_rect(pCheckbox, drgui_make_rect(boxRect.right, boxRect.top, boxRect.right + pData->padding, boxRect.bottom), pData->bgColor, pPaintData);    // Padding bettween checkbox and text.
    drgui_draw_rect(pCheckbox, drgui_make_rect(bgrect.left, bgrect.top, bgrect.right, boxRect.top), pData->bgColor, pPaintData);
    drgui_draw_rect(pCheckbox, drgui_make_rect(bgrect.left, boxRect.bottom, bgrect.right, bgrect.bottom), pData->bgColor, pPaintData);
    drgui_draw_rect(pCheckbox, drgui_make_rect(textPosX + textWidth, boxRect.top, bgrect.right, boxRect.bottom), pData->bgColor, pPaintData);
}

void dred_checkbox__on_mouse_enter(dred_checkbox* pCheckbox)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    pData->isMouseOver = true;

    drgui_dirty(pCheckbox, drgui_get_local_rect(pCheckbox));
}

void dred_checkbox__on_mouse_leave(dred_checkbox* pCheckbox)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    pData->isMouseOver = false;

    drgui_dirty(pCheckbox, drgui_get_local_rect(pCheckbox));
}

void dred_checkbox__on_mouse_move(dred_checkbox* pCheckbox, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    if (dred_control_has_mouse_capture(pCheckbox)) {
        pData->isMouseOver = (mousePosX >= 0 && mousePosX < drgui_get_width(pCheckbox)) && (mousePosY >= 0 && mousePosY < drgui_get_height(pCheckbox));
        drgui_dirty(pCheckbox, drgui_get_local_rect(pCheckbox));
    }
}

void dred_checkbox__on_mouse_button_down(dred_checkbox* pCheckbox, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (!dred_control_has_mouse_capture(pCheckbox)) {
            dred_gui_capture_mouse(pCheckbox);

            // Redraw to show the pressed state.
            drgui_dirty(pCheckbox, drgui_get_local_rect(pCheckbox));
        }
    }
}

void dred_checkbox__on_mouse_button_up(dred_checkbox* pCheckbox, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (dred_control_has_mouse_capture(pCheckbox)) {
            dred_gui_release_mouse(pCheckbox->pGUI);
            dred_checkbox_toggle(pCheckbox);
        }
    }
}

void dred_checkbox__on_release_mouse(dred_checkbox* pCheckbox)
{
    drgui_dirty(pCheckbox, drgui_get_local_rect(pCheckbox));
}

void dred_checkbox__refresh_layout(dred_checkbox* pCheckbox)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    if (pData->isAutoSizeEnabled) {
        float textWidth;
        float textHeight;
        drgui_measure_string(pData->pSubFont, pData->text, strlen(pData->text), &textWidth, &textHeight);

        dred_rect boxRect = dred_checkbox__get_box_rect(pCheckbox);
        float boxWidth = (boxRect.right - boxRect.left);

        drgui_set_size(pCheckbox, textWidth + boxWidth + pData->padding, textHeight);
    }


    // Redraw.
    drgui_dirty(pCheckbox, drgui_get_local_rect(pCheckbox));
}

dred_checkbox* dred_checkbox_create(dred_context* pDred, dred_control* pParent, const char* text, bool checked)
{
    dred_checkbox* pCheckbox = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_BUTTON, sizeof(dred_checkbox_data));
    if (pCheckbox == NULL) {
        return NULL;
    }

    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    assert(pData != NULL);

    strncpy_s(pData->text, sizeof(pData->text), text, _TRUNCATE);
    pData->pFont = pDred->config.pUIFont;
    pData->pSubFont = dred_font_acquire_subfont(pData->pFont, pDred->uiScale);
    pData->textColor = drgui_rgb(0, 0, 0);
    pData->bgColor = drgui_rgb(255, 255, 255);
    pData->boxBGColor = drgui_rgb(224, 224, 224);
    pData->boxBGColorHovered = drgui_rgb(224, 240, 255);
    pData->boxBGColorPressed = drgui_rgb(200, 224, 240);
    pData->boxBorderColor = drgui_rgb(32, 64, 160);
    pData->checkColor = drgui_rgb(64, 128, 64);
    pData->borderWidth = 1;
    pData->padding = 4;
    pData->isAutoSizeEnabled = true;
    pData->isChecked = checked;

    // Events.
    pCheckbox->onPaint = dred_checkbox__on_paint;
    pCheckbox->onMouseEnter = dred_checkbox__on_mouse_enter;
    pCheckbox->onMouseLeave = dred_checkbox__on_mouse_leave;
    pCheckbox->onMouseMove = dred_checkbox__on_mouse_move;
    pCheckbox->onMouseButtonDown = dred_checkbox__on_mouse_button_down;
    pCheckbox->onMouseButtonUp = dred_checkbox__on_mouse_button_up;
    pCheckbox->onReleaseMouse = dred_checkbox__on_release_mouse;

    dred_checkbox__refresh_layout(pCheckbox);

    return pCheckbox;
}

void dred_checkbox_delete(dred_checkbox* pCheckbox)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData != NULL) {
        dred_font_release_subfont(pData->pFont, pData->pSubFont);
    }

    dred_control_delete(pCheckbox);
}


void dred_checkbox_set_text(dred_checkbox* pCheckbox, const char* text)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    strncpy_s(pData->text, sizeof(pData->text), text, _TRUNCATE);


    // Redraw.
    drgui_dirty(pCheckbox, drgui_get_local_rect(pCheckbox));
}

void dred_checkbox_enable_auto_size(dred_checkbox* pCheckbox)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    pData->isAutoSizeEnabled = true;

    dred_checkbox__refresh_layout(pCheckbox);
}

void dred_checkbox_disable_auto_size(dred_checkbox* pCheckbox)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    pData->isAutoSizeEnabled = false;

    dred_checkbox__refresh_layout(pCheckbox);
}


void dred_checkbox_set_font(dred_checkbox* pCheckbox, dred_font* pFont)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    if (pData->pFont == pFont) {
        return;
    }

    dred_font_release_subfont(pData->pFont, pData->pSubFont);

    pData->pFont = pFont;
    pData->pSubFont = dred_font_acquire_subfont(pData->pFont, dred_control_get_gui(pCheckbox)->uiScale);

    dred_checkbox__refresh_layout(pCheckbox);
}

void dred_checkbox_set_background_color(dred_checkbox* pCheckbox, dred_color color)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    pData->bgColor = color;

    // Redraw.
    drgui_dirty(pCheckbox, drgui_get_local_rect(pCheckbox));
}

void dred_checkbox_set_border_color(dred_checkbox* pCheckbox, dred_color color)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    pData->boxBorderColor = color;

    // Redraw.
    drgui_dirty(pCheckbox, drgui_get_local_rect(pCheckbox));
}

void dred_checkbox_set_border_width(dred_checkbox* pCheckbox, float width)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    pData->borderWidth = width;

    // Redraw.
    drgui_dirty(pCheckbox, drgui_get_local_rect(pCheckbox));
}

void dred_checkbox_set_padding(dred_checkbox* pCheckbox, float padding)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    pData->padding = padding;

    dred_checkbox__refresh_layout(pCheckbox);
}


void dred_checkbox_check(dred_checkbox* pCheckbox)
{
    dred_checkbox_set_checked(pCheckbox, true, false);
}

void dred_checkbox_uncheck(dred_checkbox* pCheckbox)
{
    dred_checkbox_set_checked(pCheckbox, false, false);
}

void dred_checkbox_toggle(dred_checkbox* pCheckbox)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }
    
    if (pData->isChecked) {
        dred_checkbox_uncheck(pCheckbox);
    } else {
        dred_checkbox_check(pCheckbox);
    }
}

void dred_checkbox_set_checked(dred_checkbox* pCheckbox, bool checked, bool blockEvent)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    if (pData->isChecked == checked) {
        return;
    }

    pData->isChecked = checked;

    if (!blockEvent) {
        if (pData->varBinding[0] != '\0') {
            dred_config_set(&dred_control_get_gui(pCheckbox)->config, pData->varBinding, checked ? "true" : "false");
        }

        if (pData->onCheckChanged) {
            pData->onCheckChanged(pCheckbox);
        }
    }

    drgui_dirty(pCheckbox, dred_checkbox__get_box_rect(pCheckbox));
}

bool dred_is_checked(dred_checkbox* pCheckbox)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return false;
    }

    return pData->isChecked;
}


void dred_checkbox_set_bind_to_config_var(dred_checkbox* pCheckbox, const char* varName)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    if (varName != NULL) {
        strcpy_s(pData->varBinding, sizeof(pData->varBinding), varName);
    } else {
        pData->varBinding[0] = '\0';
    }
}

void dred_checkbox_set_on_checked_changed(dred_checkbox* pCheckbox, dred_checkbox_on_checked_changed_proc proc)
{
    dred_checkbox_data* pData = (dred_checkbox_data*)dred_control_get_extra_data(pCheckbox);
    if (pData == NULL) {
        return;
    }

    pData->onCheckChanged = proc;
}
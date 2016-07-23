// Copyright (C) 2016 David Reid. See included LICENSE file.


typedef struct
{
    char text[64];
    dred_font* pFont;
    dred_gui_font* pSubFont;
    dred_color textColor;
    dred_color bgColor;
    dred_color bgColorHovered;
    dred_color bgColorPressed;
    dred_color boxBorderColor;
    dred_color color;
    float borderWidth;
    float padding;
    bool isMouseOver;
    bool isAutoSizeEnabled;
    char varBinding[128];
    dred_colorbutton_on_color_changed_proc onColorChanged;
} dred_colorbutton_data;

dred_rect dred_colorbutton__get_box_rect(dred_colorbutton* pButton)
{
    // The size of the box is based on the size of the font.
    assert(pButton != NULL);

    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    assert(pData != NULL);
    
    dred_gui_font_metrics metrics;
    drgui_get_font_metrics(pData->pSubFont, &metrics);

    float posX = 0;
    float posY = (drgui_get_height(pButton) - metrics.lineHeight) / 2;
    return drgui_make_rect(posX, posY, posX + metrics.lineHeight, posY + metrics.lineHeight);
}

void dred_colorbutton__on_paint(dred_colorbutton* pButton, dred_rect rect, void* pPaintData)
{
    (void)rect;

    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pButton);
    if (pDred == NULL) {
        return;
    }

    // Draw the box first.
    dred_color bgColor = pData->bgColor;
    /*if (drgui_has_mouse_capture(pButton)) {
        boxBGColor = pData->boxBGColorHovered;
        if (pData->isMouseOver) {
            boxBGColor = pData->boxBGColorPressed;
        }
    } else if (pData->isMouseOver) {
        boxBGColor = pData->boxBGColorHovered;
    }*/

    dred_rect bgrect = drgui_get_local_rect(pButton);
    dred_rect boxRect = dred_colorbutton__get_box_rect(pButton);

    drgui_draw_rect_outline(pButton, boxRect, pData->boxBorderColor, pData->borderWidth, pPaintData);
    drgui_draw_rect(pButton, drgui_grow_rect(boxRect, -pData->borderWidth), pData->color, pPaintData);


    // The text is positioned to the right of the box, and centered vertically.
    float textWidth;
    float textHeight;
    drgui_measure_string(pData->pSubFont, pData->text, strlen(pData->text), &textWidth, &textHeight);

    float textPosX = boxRect.right + pData->padding;
    float textPosY = roundf(drgui_get_height(pButton) - textHeight) / 2;
    drgui_draw_text(pButton, pData->pSubFont, pData->text, (int)strlen(pData->text), textPosX, textPosY, pData->textColor, bgColor, pPaintData);

    // Background
    drgui_draw_rect(pButton, drgui_make_rect(boxRect.right, boxRect.top, boxRect.right + pData->padding, boxRect.bottom), bgColor, pPaintData);    // Padding bettween colorbutton and text.
    drgui_draw_rect(pButton, drgui_make_rect(bgrect.left, bgrect.top, bgrect.right, boxRect.top), bgColor, pPaintData);
    drgui_draw_rect(pButton, drgui_make_rect(bgrect.left, boxRect.bottom, bgrect.right, bgrect.bottom), bgColor, pPaintData);
    drgui_draw_rect(pButton, drgui_make_rect(textPosX + textWidth, boxRect.top, bgrect.right, boxRect.bottom), bgColor, pPaintData);
}

void dred_colorbutton__on_mouse_enter(dred_colorbutton* pButton)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->isMouseOver = true;

    drgui_dirty(pButton, drgui_get_local_rect(pButton));
}

void dred_colorbutton__on_mouse_leave(dred_colorbutton* pButton)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->isMouseOver = false;

    drgui_dirty(pButton, drgui_get_local_rect(pButton));
}

void dred_colorbutton__on_mouse_move(dred_colorbutton* pButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    if (drgui_has_mouse_capture(pButton)) {
        pData->isMouseOver = (mousePosX >= 0 && mousePosX < dred_control_get_width(pButton)) && (mousePosY >= 0 && mousePosY < dred_control_get_height(pButton));
        drgui_dirty(pButton, drgui_get_local_rect(pButton));
    }
}

void dred_colorbutton__on_mouse_button_down(dred_colorbutton* pButton, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (!drgui_has_mouse_capture(pButton)) {
            drgui_capture_mouse(pButton);

            // Redraw to show the pressed state.
            drgui_dirty(pButton, drgui_get_local_rect(pButton));
        }
    }
}

void dred_colorbutton__on_mouse_button_up(dred_colorbutton* pButton, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_window* pOwnerWindow = dred_get_element_window(pButton);
    assert(pOwnerWindow != NULL);

    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (drgui_has_mouse_capture(pButton)) {
            drgui_release_mouse(pButton->pContext);
            
            // Show a color picker.
            dred_color newColor;
            if (dred_show_color_picker_dialog(pOwnerWindow->pDred, pOwnerWindow, pData->color, &newColor)) {
                dred_colorbutton_set_color(pButton, newColor, false);
                drgui_dirty(pButton, drgui_get_local_rect(pButton));
            }
        }
    }
}

void dred_colorbutton__on_release_mouse(dred_colorbutton* pButton)
{
    drgui_dirty(pButton, drgui_get_local_rect(pButton));
}

void dred_colorbutton__refresh_layout(dred_colorbutton* pButton)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    if (pData->isAutoSizeEnabled) {
        float textWidth;
        float textHeight;
        drgui_measure_string(pData->pSubFont, pData->text, strlen(pData->text), &textWidth, &textHeight);

        dred_rect boxRect = dred_colorbutton__get_box_rect(pButton);
        float boxWidth = (boxRect.right - boxRect.left);

        dred_control_set_size(pButton, textWidth + boxWidth + pData->padding, textHeight);
    }


    // Redraw.
    drgui_dirty(pButton, drgui_get_local_rect(pButton));
}

dred_colorbutton* dred_colorbutton_create(dred_context* pDred, dred_control* pParent, const char* text, dred_color color)
{
    dred_colorbutton* pButton = drgui_create_element(pDred, pParent, DRED_CONTROL_TYPE_BUTTON, sizeof(dred_colorbutton_data));
    if (pButton == NULL) {
        return NULL;
    }

    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    assert(pData != NULL);

    strncpy_s(pData->text, sizeof(pData->text), text, _TRUNCATE);
    pData->pFont = pDred->config.pUIFont;
    pData->pSubFont = dred_font_acquire_subfont(pData->pFont, pDred->uiScale);
    pData->textColor = drgui_rgb(0, 0, 0);
    pData->bgColor = drgui_rgb(255, 255, 255);
    pData->bgColorHovered = drgui_rgb(224, 240, 255);
    pData->bgColorPressed = drgui_rgb(200, 224, 240);
    pData->boxBorderColor = drgui_rgb(0, 0, 0);
    pData->borderWidth = 1;
    pData->padding = 4;
    pData->isAutoSizeEnabled = true;
    pData->color = color;

    // Events.
    pButton->onPaint = dred_colorbutton__on_paint;
    pButton->onMouseEnter = dred_colorbutton__on_mouse_enter;
    pButton->onMouseLeave = dred_colorbutton__on_mouse_leave;
    pButton->onMouseMove = dred_colorbutton__on_mouse_move;
    pButton->onMouseButtonDown = dred_colorbutton__on_mouse_button_down;
    pButton->onMouseButtonUp = dred_colorbutton__on_mouse_button_up;
    pButton->onReleaseMouse = dred_colorbutton__on_release_mouse;

    dred_colorbutton__refresh_layout(pButton);

    return pButton;
}

void dred_colorbutton_delete(dred_colorbutton* pButton)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData != NULL) {
        dred_font_release_subfont(pData->pFont, pData->pSubFont);
    }

    drgui_delete_element(pButton);
}


void dred_colorbutton_set_text(dred_colorbutton* pButton, const char* text)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    strncpy_s(pData->text, sizeof(pData->text), text, _TRUNCATE);


    // Redraw.
    drgui_dirty(pButton, drgui_get_local_rect(pButton));
}

void dred_colorbutton_enable_auto_size(dred_colorbutton* pButton)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->isAutoSizeEnabled = true;

    dred_colorbutton__refresh_layout(pButton);
}

void dred_colorbutton_disable_auto_size(dred_colorbutton* pButton)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->isAutoSizeEnabled = false;

    dred_colorbutton__refresh_layout(pButton);
}


void dred_colorbutton_set_font(dred_colorbutton* pButton, dred_font* pFont)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    if (pData->pFont == pFont) {
        return;
    }

    dred_font_release_subfont(pData->pFont, pData->pSubFont);

    pData->pFont = pFont;
    pData->pSubFont = dred_font_acquire_subfont(pData->pFont, dred_control_get_context(pButton)->uiScale);

    dred_colorbutton__refresh_layout(pButton);
}

void dred_colorbutton_set_background_color(dred_colorbutton* pButton, dred_color color)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->bgColor = color;

    // Redraw.
    drgui_dirty(pButton, drgui_get_local_rect(pButton));
}

void dred_colorbutton_set_border_color(dred_colorbutton* pButton, dred_color color)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->boxBorderColor = color;

    // Redraw.
    drgui_dirty(pButton, drgui_get_local_rect(pButton));
}

void dred_colorbutton_set_border_width(dred_colorbutton* pButton, float width)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->borderWidth = width;

    // Redraw.
    drgui_dirty(pButton, drgui_get_local_rect(pButton));
}

void dred_colorbutton_set_padding(dred_colorbutton* pButton, float padding)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->padding = padding;

    dred_colorbutton__refresh_layout(pButton);
}


void dred_colorbutton_set_color(dred_colorbutton* pButton, dred_color color, bool blockEvent)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->color = color;

    if (blockEvent) {
        return;
    }

    if (pData->varBinding[0] != '\0') {
        char colorStr[256];
        snprintf(colorStr, sizeof(colorStr), "%d %d %d %d", color.r, color.g, color.b, color.a);
        dred_config_set(&dred_control_get_context(pButton)->config, pData->varBinding, colorStr);
    }

    if (pData->onColorChanged) {
        pData->onColorChanged(pButton, color);
    }
}

dred_color dred_colorbutton_get_color(dred_colorbutton* pButton)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pData->color;
}


void dred_colorbutton_set_bind_to_config_var(dred_colorbutton* pButton, const char* varName)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    if (varName != NULL) {
        strcpy_s(pData->varBinding, sizeof(pData->varBinding), varName);
    } else {
        pData->varBinding[0] = '\0';
    }
}

void dred_colorbutton_set_on_color_changed(dred_colorbutton* pButton, dred_colorbutton_on_color_changed_proc proc)
{
    dred_colorbutton_data* pData = (dred_colorbutton_data*)drgui_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->onColorChanged = proc;
}
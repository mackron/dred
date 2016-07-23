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
    dred_color borderColor;
    float borderWidth;
    float paddingHorz;
    float paddingVert;
    bool isMouseOver;
    bool isAutoSizeEnabled;
    dred_button_on_pressed_proc onPressed;
} dred_button_data;

void dred_button__on_paint(dred_button* pButton, dred_rect rect, void* pPaintData)
{
    (void)rect;

    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_gui(pButton);
    if (pDred == NULL) {
        return;
    }


    dred_color bgColor = pData->bgColor;
    if (dred_control_has_mouse_capture(pButton)) {
        bgColor = pData->bgColorHovered;
        if (pData->isMouseOver) {
            bgColor = pData->bgColorPressed;
        }
    } else if (pData->isMouseOver) {
        bgColor = pData->bgColorHovered;
    }


    // Draw the border first.
    drgui_draw_rect_outline(pButton, dred_control_get_local_rect(pButton), pData->borderColor, pData->borderWidth, pPaintData);

    // Text and background. The text is centered.
    dred_rect bgrect = drgui_grow_rect(dred_control_get_local_rect(pButton), -pData->borderWidth);

    float textWidth;
    float textHeight;
    drgui_measure_string(pData->pSubFont, pData->text, strlen(pData->text), &textWidth, &textHeight);

    float textPosX = roundf(((bgrect.right - bgrect.left) - textWidth) / 2);
    float textPosY = roundf(((bgrect.bottom - bgrect.top) - textHeight) / 2);
    drgui_draw_text(pButton, pData->pSubFont, pData->text, (int)strlen(pData->text), textPosX, textPosY, pData->textColor, bgColor, pPaintData);
    
    // Make sure the background does not overdraw the text.
    drgui_draw_rect(pButton, drgui_make_rect(bgrect.left, bgrect.top, textPosX, bgrect.bottom), bgColor, pPaintData);
    drgui_draw_rect(pButton, drgui_make_rect(textPosX + textWidth, bgrect.top, bgrect.right, bgrect.bottom), bgColor, pPaintData);
    drgui_draw_rect(pButton, drgui_make_rect(textPosX, bgrect.top, textPosX + textWidth, textPosY), bgColor, pPaintData);
    drgui_draw_rect(pButton, drgui_make_rect(textPosX, textPosY + textHeight, textPosX + textWidth, bgrect.bottom), bgColor, pPaintData);
}

void dred_button__on_mouse_enter(dred_button* pButton)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->isMouseOver = true;

    drgui_dirty(pButton, dred_control_get_local_rect(pButton));
}

void dred_button__on_mouse_leave(dred_button* pButton)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->isMouseOver = false;

    drgui_dirty(pButton, dred_control_get_local_rect(pButton));
}

void dred_button__on_mouse_move(dred_button* pButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    if (dred_control_has_mouse_capture(pButton)) {
        pData->isMouseOver = (mousePosX >= 0 && mousePosX < dred_control_get_width(pButton)) && (mousePosY >= 0 && mousePosY < dred_control_get_height(pButton));
        drgui_dirty(pButton, dred_control_get_local_rect(pButton));
    }
}

void dred_button__on_mouse_button_down(dred_button* pButton, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (!dred_control_has_mouse_capture(pButton)) {
            dred_gui_capture_mouse(pButton);

            // Redraw to show the pressed state.
            drgui_dirty(pButton, dred_control_get_local_rect(pButton));
        }
    }
}

void dred_button__on_mouse_button_up(dred_button* pButton, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    if (mouseButton == DRED_GUI_MOUSE_BUTTON_LEFT) {
        if (dred_control_has_mouse_capture(pButton)) {
            dred_gui_release_mouse(pButton->pGUI);

            if (pData->onPressed && dred_control_is_under_mouse(pButton)) {
                pData->onPressed(pButton);
            }
        }
    }
}

void dred_button__on_release_mouse(dred_button* pButton)
{
    drgui_dirty(pButton, dred_control_get_local_rect(pButton));
}

void dred_button__refresh_layout(dred_button* pButton)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    if (pData->isAutoSizeEnabled) {
        float textWidth;
        float textHeight;
        drgui_measure_string(pData->pSubFont, pData->text, strlen(pData->text), &textWidth, &textHeight);

        dred_control_set_size(pButton, textWidth + (pData->paddingHorz*2), textHeight + (pData->paddingVert*2));
    }


    // Redraw.
    drgui_dirty(pButton, dred_control_get_local_rect(pButton));
}

dred_button* dred_button_create(dred_context* pDred, dred_control* pParent, const char* text)
{
    dred_button* pButton = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_BUTTON, sizeof(dred_button_data));
    if (pButton == NULL) {
        return NULL;
    }

    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    assert(pData != NULL);

    strncpy_s(pData->text, sizeof(pData->text), text, _TRUNCATE);
    pData->pFont = pDred->config.pUIFont;
    pData->pSubFont = dred_font_acquire_subfont(pData->pFont, pDred->uiScale);
    pData->textColor = drgui_rgb(32, 32, 32);
    pData->bgColor = drgui_rgb(224, 224, 224);
    pData->bgColorHovered = drgui_rgb(224, 240, 255);
    pData->bgColorPressed = drgui_rgb(200, 224, 240);
    pData->borderColor = drgui_rgb(32, 64, 160);
    pData->borderWidth = 1;
    pData->paddingHorz = 16;
    pData->paddingVert = 4;
    pData->isAutoSizeEnabled = true;

    // Events.
    pButton->onPaint = dred_button__on_paint;
    pButton->onMouseEnter = dred_button__on_mouse_enter;
    pButton->onMouseLeave = dred_button__on_mouse_leave;
    pButton->onMouseMove = dred_button__on_mouse_move;
    pButton->onMouseButtonDown = dred_button__on_mouse_button_down;
    pButton->onMouseButtonUp = dred_button__on_mouse_button_up;
    pButton->onReleaseMouse = dred_button__on_release_mouse;

    dred_button__refresh_layout(pButton);

    return pButton;
}

void dred_button_delete(dred_button* pButton)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData != NULL) {
        dred_font_release_subfont(pData->pFont, pData->pSubFont);
    }

    dred_control_delete(pButton);
}


void dred_button_set_text(dred_button* pButton, const char* text)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    strncpy_s(pData->text, sizeof(pData->text), text, _TRUNCATE);


    // Redraw.
    drgui_dirty(pButton, dred_control_get_local_rect(pButton));
}

void dred_button_enable_auto_size(dred_button* pButton)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->isAutoSizeEnabled = true;

    dred_button__refresh_layout(pButton);
}

void dred_button_disable_auto_size(dred_button* pButton)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->isAutoSizeEnabled = false;

    dred_button__refresh_layout(pButton);
}


void dred_button_set_font(dred_button* pButton, dred_font* pFont)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    if (pData->pFont == pFont) {
        return;
    }

    dred_font_release_subfont(pData->pFont, pData->pSubFont);

    pData->pFont = pFont;
    pData->pSubFont = dred_font_acquire_subfont(pData->pFont, dred_control_get_gui(pButton)->uiScale);

    dred_button__refresh_layout(pButton);
}

void dred_button_set_background_color(dred_button* pButton, dred_color color)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->bgColor = color;

    // Redraw.
    drgui_dirty(pButton, dred_control_get_local_rect(pButton));
}

void dred_button_set_border_color(dred_button* pButton, dred_color color)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->borderColor = color;

    // Redraw.
    drgui_dirty(pButton, dred_control_get_local_rect(pButton));
}

void dred_button_set_border_width(dred_button* pButton, float width)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->borderWidth = width;

    // Redraw.
    drgui_dirty(pButton, dred_control_get_local_rect(pButton));
}

void dred_button_set_padding(dred_button* pButton, float paddingHorz, float paddingVert)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->paddingHorz = paddingHorz;
    pData->paddingVert = paddingVert;

    dred_button__refresh_layout(pButton);
}


void dred_button_set_on_pressed(dred_button* pButton, dred_button_on_pressed_proc proc)
{
    dred_button_data* pData = (dred_button_data*)dred_control_get_extra_data(pButton);
    if (pData == NULL) {
        return;
    }

    pData->onPressed = proc;
}
// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRGUI_MIN_SCROLLBAR_THUMB_SIZE    16

typedef struct
{
    /// The orientation.
    dred_scrollbar_orientation orientation;

    /// The minimum scroll range.
    int rangeMin;

    /// The maximum scroll range.
    int rangeMax;

    /// The page size.
    int pageSize;

    /// The current scroll position.
    int scrollPos;

    /// Whether or not to auto-hide the thumb.
    bool autoHideThumb;

    /// The mouse wheel scale.
    int mouseWheelScale;

    /// The color of the track.
    drgui_color trackColor;

    /// The color of the thumb while not hovered or pressed.
    drgui_color thumbColor;

    /// The color of the thumb while hovered.
    drgui_color thumbColorHovered;

    /// The color of the thumb while pressed.
    drgui_color thumbColorPressed;

    /// The function to call when the scroll position changes.
    dred_scrollbar_on_scroll_proc onScroll;


    /// The current size of the thumb.
    float thumbSize;

    /// The current position of the thumb.
    float thumbPos;

    /// The amount of padding between the edge of the scrollbar and the thumb.
    float thumbPadding;

    /// Whether or not we are hovered over the thumb.
    bool thumbHovered;

    /// Whether or not the thumb is pressed.
    bool thumbPressed;

    /// The relative position of the mouse on the x axis at the time the thumb was pressed with the mouse.
    float thumbClickPosX;

    /// The relative position of the mouse on the y axis at the time the thumb was pressed with the mouse.
    float thumbClickPosY;


    /// The size of the extra data.
    size_t extraDataSize;

    /// A pointer to the extra data.
    char pExtraData[1];

} drgui_scrollbar;


/// Refreshes the given scrollbar's thumb layout and redraws it.
DRGUI_PRIVATE void dred_scrollbar_refresh_thumb(drgui_element* pSBElement);

/// Calculates the size of the thumb. This does not change the state of the thumb.
DRGUI_PRIVATE float dred_scrollbar_calculate_thumb_size(drgui_element* pSBElement);

/// Calculates the position of the thumb. This does not change the state of the thumb.
DRGUI_PRIVATE float dred_scrollbar_calculate_thumb_position(drgui_element* pSBElement);

/// Retrieves the size of the given scrollbar's track. For vertical alignments, it's the height of the element, otherwise it's the width.
DRGUI_PRIVATE float dred_scrollbar_get_track_size(drgui_element* pSBElement);

/// Makes the given point that's relative to the given scrollbar relative to it's thumb.
DRGUI_PRIVATE void dred_scrollbar_make_relative_to_thumb(drgui_element* pSBElement, float* pPosX, float* pPosY);

/// Calculates the scroll position based on the current position of the thumb. This is used for scrolling while dragging the thumb.
DRGUI_PRIVATE int dred_scrollbar_calculate_scroll_pos_from_thumb_pos(drgui_element* pScrollba, float thumbPosr);

/// Simple clamp function.
DRGUI_PRIVATE float dred_scrollbar_clampf(float n, float lower, float upper)
{
    return n <= lower ? lower : n >= upper ? upper : n;
}

/// Simple clamp function.
DRGUI_PRIVATE int dred_scrollbar_clampi(int n, int lower, int upper)
{
    return n <= lower ? lower : n >= upper ? upper : n;
}

/// Simple max function.
DRGUI_PRIVATE int dred_scrollbar_maxi(int x, int y)
{
    return (x > y) ? x : y;
}


drgui_element* drgui_create_scrollbar(drgui_context* pContext, drgui_element* pParent, dred_scrollbar_orientation orientation, size_t extraDataSize, const void* pExtraData)
{
    if (pContext == NULL || orientation == dred_scrollbar_orientation_none) {
        return NULL;
    }

    drgui_element* pSBElement = drgui_create_element(pContext, pParent, sizeof(drgui_scrollbar) + extraDataSize, NULL);
    if (pSBElement == NULL) {
        return NULL;
    }

    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    assert(pSB != NULL);

    pSB->orientation       = orientation;
    pSB->rangeMin          = 0;
    pSB->rangeMax          = 0;
    pSB->pageSize          = 0;
    pSB->scrollPos         = 0;
    pSB->autoHideThumb     = true;
    pSB->mouseWheelScale   = 1;
    pSB->trackColor        = drgui_rgb(80, 80, 80);
    pSB->thumbColor        = drgui_rgb(112, 112, 112);
    pSB->thumbColorHovered = drgui_rgb(144, 144, 144);
    pSB->thumbColorPressed = drgui_rgb(180, 180, 180);
    pSB->onScroll          = NULL;

    pSB->thumbSize         = DRGUI_MIN_SCROLLBAR_THUMB_SIZE;
    pSB->thumbPos          = 0;
    pSB->thumbPadding      = 2;
    pSB->thumbHovered      = false;
    pSB->thumbPressed      = false;
    pSB->thumbClickPosX    = 0;
    pSB->thumbClickPosY    = 0;

    pSB->extraDataSize = extraDataSize;
    if (pExtraData != NULL) {
        memcpy(pSB->pExtraData, pExtraData, extraDataSize);
    }


    // Default event handlers.
    drgui_set_on_size(pSBElement, dred_scrollbar_on_size);
    drgui_set_on_mouse_leave(pSBElement, dred_scrollbar_on_mouse_leave);
    drgui_set_on_mouse_move(pSBElement, dred_scrollbar_on_mouse_move);
    drgui_set_on_mouse_button_down(pSBElement, dred_scrollbar_on_mouse_button_down);
    drgui_set_on_mouse_button_up(pSBElement, dred_scrollbar_on_mouse_button_up);
    drgui_set_on_mouse_wheel(pSBElement, dred_scrollbar_on_mouse_wheel);
    drgui_set_on_paint(pSBElement, dred_scrollbar_on_paint);


    return pSBElement;
}

void drgui_delete_scrollbar(drgui_element* pSBElement)
{
    if (pSBElement == NULL) {
        return;
    }

    drgui_delete_element(pSBElement);
}


size_t dred_scrollbar_get_extra_data_size(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return 0;
    }

    return pSB->extraDataSize;
}

void* dred_scrollbar_get_extra_data(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return NULL;
    }

    return pSB->pExtraData;
}


dred_scrollbar_orientation dred_scrollbar_get_orientation(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return dred_scrollbar_orientation_none;
    }

    return pSB->orientation;
}


void dred_scrollbar_set_range(drgui_element* pSBElement, int rangeMin, int rangeMax)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    pSB->rangeMin = rangeMin;
    pSB->rangeMax = rangeMax;


    // Make sure the scroll position is still valid.
    dred_scrollbar_scroll_to(pSBElement, dred_scrollbar_get_scroll_position(pSBElement));

    // The thumb may have changed, so refresh it.
    dred_scrollbar_refresh_thumb(pSBElement);
}

void dred_scrollbar_get_range(drgui_element* pSBElement, int* pRangeMinOut, int* pRangeMaxOut)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    if (pRangeMinOut != NULL) {
        *pRangeMinOut = pSB->rangeMin;
    }

    if (pRangeMaxOut != NULL) {
        *pRangeMaxOut = pSB->rangeMax;
    }
}


void dred_scrollbar_set_page_size(drgui_element* pSBElement, int pageSize)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    pSB->pageSize = pageSize;


    // Make sure the scroll position is still valid.
    dred_scrollbar_scroll_to(pSBElement, dred_scrollbar_get_scroll_position(pSBElement));

    // The thumb may have changed, so refresh it.
    dred_scrollbar_refresh_thumb(pSBElement);
}

int dred_scrollbar_get_page_size(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return 0;
    }

    return pSB->pageSize;
}


void dred_scrollbar_set_range_and_page_size(drgui_element* pSBElement, int rangeMin, int rangeMax, int pageSize)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    pSB->rangeMin = rangeMin;
    pSB->rangeMax = rangeMax;
    pSB->pageSize = pageSize;


    // Make sure the scroll position is still valid.
    dred_scrollbar_scroll_to(pSBElement, dred_scrollbar_get_scroll_position(pSBElement));

    // The thumb may have changed, so refresh it.
    dred_scrollbar_refresh_thumb(pSBElement);
}


void dred_scrollbar_set_scroll_position(drgui_element* pSBElement, int position)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    int newScrollPos = dred_scrollbar_clampi(position, pSB->rangeMin, dred_scrollbar_maxi(0, pSB->rangeMax - pSB->pageSize + 1));
    if (newScrollPos != pSB->scrollPos)
    {
        pSB->scrollPos = newScrollPos;

        // The position of the thumb has changed, so refresh it.
        dred_scrollbar_refresh_thumb(pSBElement);
    }
}

int dred_scrollbar_get_scroll_position(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return 0;
    }

    return pSB->scrollPos;
}


void dred_scrollbar_scroll(drgui_element* pSBElement, int offset)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    dred_scrollbar_scroll_to(pSBElement, pSB->scrollPos + offset);
}

void dred_scrollbar_scroll_to(drgui_element* pSBElement, int newScrollPos)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    int oldScrollPos = pSB->scrollPos;
    dred_scrollbar_set_scroll_position(pSBElement, newScrollPos);

    if (oldScrollPos != pSB->scrollPos)
    {
        if (pSB->onScroll) {
            pSB->onScroll(pSBElement, pSB->scrollPos);
        }
    }
}


void dred_scrollbar_enable_thumb_auto_hide(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    if (pSB->autoHideThumb != true)
    {
        pSB->autoHideThumb = true;

        // The thumb needs to be refreshed in order to show the correct state.
        dred_scrollbar_refresh_thumb(pSBElement);
    }
}

void dred_scrollbar_disable_thumb_auto_hide(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    if (pSB->autoHideThumb != false)
    {
        pSB->autoHideThumb = false;

        // The thumb needs to be refreshed in order to show the correct state.
        dred_scrollbar_refresh_thumb(pSBElement);
    }
}

bool dred_scrollbar_is_thumb_auto_hide_enabled(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return false;
    }

    return pSB->autoHideThumb;
}

bool dred_scrollbar_is_thumb_visible(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return false;
    }

    // Always visible if auto-hiding is disabled.
    if (!pSB->autoHideThumb) {
        return true;
    }

    return pSB->pageSize < (pSB->rangeMax - pSB->rangeMin + 1) && pSB->pageSize > 0;
}


void dred_scrollbar_set_mouse_wheel_scele(drgui_element* pSBElement, int scale)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    pSB->mouseWheelScale = scale;
}

int dred_scrollbar_get_mouse_wheel_scale(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return 1;
    }

    return pSB->mouseWheelScale;
}


void dred_scrollbar_set_track_color(drgui_element* pSBElement, drgui_color color)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    pSB->trackColor = color;
}

void dred_scrollbar_set_default_thumb_color(drgui_element* pSBElement, drgui_color color)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    pSB->thumbColor = color;
}

void dred_scrollbar_set_hovered_thumb_color(drgui_element* pSBElement, drgui_color color)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    pSB->thumbColorHovered = color;
}

void dred_scrollbar_set_pressed_thumb_color(drgui_element* pSBElement, drgui_color color)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    pSB->thumbColorPressed = color;
}


void dred_scrollbar_set_on_scroll(drgui_element* pSBElement, dred_scrollbar_on_scroll_proc onScroll)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    pSB->onScroll = onScroll;
}

dred_scrollbar_on_scroll_proc dred_scrollbar_get_on_scroll(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return NULL;
    }

    return pSB->onScroll;
}


drgui_rect dred_scrollbar_get_thumb_rect(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return drgui_make_rect(0, 0, 0, 0);
    }

    drgui_rect rect = {0, 0, 0, 0};
    rect.left = pSB->thumbPadding;
    rect.top  = pSB->thumbPadding;

    if (pSB->orientation == dred_scrollbar_orientation_vertical)
    {
        // Vertical.
        rect.left   = pSB->thumbPadding;
        rect.right  = drgui_get_width(pSBElement) - pSB->thumbPadding;
        rect.top    = pSB->thumbPadding + pSB->thumbPos;
        rect.bottom = rect.top + pSB->thumbSize;
    }
    else
    {
        // Horizontal.
        rect.left   = pSB->thumbPadding + pSB->thumbPos;
        rect.right  = rect.left + pSB->thumbSize;
        rect.top    = pSB->thumbPadding;
        rect.bottom = drgui_get_height(pSBElement) - pSB->thumbPadding;
    }

    return rect;
}


void dred_scrollbar_on_size(drgui_element* pSBElement, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    dred_scrollbar_refresh_thumb(pSBElement);
}

void dred_scrollbar_on_mouse_leave(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    bool needsRedraw = false;
    if (pSB->thumbHovered)
    {
        needsRedraw = true;
        pSB->thumbHovered = false;
    }

    if (pSB->thumbPressed)
    {
        needsRedraw = true;
        pSB->thumbPressed = false;
    }

    if (needsRedraw) {
        drgui_dirty(pSBElement, dred_scrollbar_get_thumb_rect(pSBElement));
    }
}

void dred_scrollbar_on_mouse_move(drgui_element* pSBElement, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    if (pSB->thumbPressed)
    {
        // The thumb is pressed. Drag it.
        float thumbRelativeMousePosX = (float)relativeMousePosX;
        float thumbRelativeMousePosY = (float)relativeMousePosY;
        dred_scrollbar_make_relative_to_thumb(pSBElement, &thumbRelativeMousePosX, &thumbRelativeMousePosY);

        float dragOffsetX = thumbRelativeMousePosX - pSB->thumbClickPosX;
        float dragOffsetY = thumbRelativeMousePosY - pSB->thumbClickPosY;

        float destTrackPos = pSB->thumbPos;
        if (pSB->orientation == dred_scrollbar_orientation_vertical) {
            destTrackPos += dragOffsetY;
        } else {
            destTrackPos += dragOffsetX;
        }

        int destScrollPos = dred_scrollbar_calculate_scroll_pos_from_thumb_pos(pSBElement, destTrackPos);
        if (destScrollPos != pSB->scrollPos)
        {
            dred_scrollbar_scroll_to(pSBElement, destScrollPos);
        }
    }
    else
    {
        // The thumb is not pressed. We just need to check if the hovered state has change and redraw if required.
        if (dred_scrollbar_is_thumb_visible(pSBElement))
        {
            bool wasThumbHovered = pSB->thumbHovered;

            drgui_rect thumbRect = dred_scrollbar_get_thumb_rect(pSBElement);
            pSB->thumbHovered = drgui_rect_contains_point(thumbRect, (float)relativeMousePosX, (float)relativeMousePosY);

            if (wasThumbHovered != pSB->thumbHovered) {
                drgui_dirty(pSBElement, thumbRect);
            }
        }
    }
}

void dred_scrollbar_on_mouse_button_down(drgui_element* pSBElement, int button, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    if (button == DRGUI_MOUSE_BUTTON_LEFT)
    {
        if (dred_scrollbar_is_thumb_visible(pSBElement))
        {
            drgui_rect thumbRect = dred_scrollbar_get_thumb_rect(pSBElement);
            if (drgui_rect_contains_point(thumbRect, (float)relativeMousePosX, (float)relativeMousePosY))
            {
                if (!pSB->thumbPressed)
                {
                    drgui_capture_mouse(pSBElement);
                    pSB->thumbPressed = true;

                    pSB->thumbClickPosX = (float)relativeMousePosX;
                    pSB->thumbClickPosY = (float)relativeMousePosY;
                    dred_scrollbar_make_relative_to_thumb(pSBElement, &pSB->thumbClickPosX, &pSB->thumbClickPosY);

                    drgui_dirty(pSBElement, dred_scrollbar_get_thumb_rect(pSBElement));
                }
            }
            else
            {
                // If the click position is above the thumb we want to scroll up by a page. If it's below the thumb, we scroll down by a page.
                if (relativeMousePosY < thumbRect.top) {
                    dred_scrollbar_scroll(pSBElement, -dred_scrollbar_get_page_size(pSBElement));
                } else if (relativeMousePosY >= thumbRect.bottom) {
                    dred_scrollbar_scroll(pSBElement,  dred_scrollbar_get_page_size(pSBElement));
                }
            }
        }
    }
}

void dred_scrollbar_on_mouse_button_up(drgui_element* pSBElement, int button, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    if (button == DRGUI_MOUSE_BUTTON_LEFT)
    {
        if (pSB->thumbPressed && drgui_get_element_with_mouse_capture(pSBElement->pContext) == pSBElement)
        {
            drgui_release_mouse(pSBElement->pContext);
            pSB->thumbPressed = false;

            drgui_dirty(pSBElement, dred_scrollbar_get_thumb_rect(pSBElement));
        }
    }
}

void dred_scrollbar_on_mouse_wheel(drgui_element* pSBElement, int delta, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    dred_scrollbar_scroll(pSBElement, -delta * dred_scrollbar_get_mouse_wheel_scale(pSBElement));
}

void dred_scrollbar_on_paint(drgui_element* pSBElement, drgui_rect relativeClippingRect, void* pPaintData)
{
    (void)relativeClippingRect;

    const drgui_scrollbar* pSB = (const drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    if (pSB == NULL) {
        return;
    }

    drgui_rect thumbRect = dred_scrollbar_get_thumb_rect(pSBElement);

    if (dred_scrollbar_is_thumb_visible(pSBElement))
    {
        // The thumb is visible.

        // Track. We draw this in 4 seperate pieces so we can avoid overdraw with the thumb.
        drgui_draw_rect(pSBElement, drgui_make_rect(0, 0, drgui_get_width(pSBElement), thumbRect.top), pSB->trackColor, pPaintData);  // Top
        drgui_draw_rect(pSBElement, drgui_make_rect(0, thumbRect.bottom, drgui_get_width(pSBElement), drgui_get_height(pSBElement)), pSB->trackColor, pPaintData);  // Bottom
        drgui_draw_rect(pSBElement, drgui_make_rect(0, thumbRect.top, thumbRect.left, thumbRect.bottom), pSB->trackColor, pPaintData);  // Left
        drgui_draw_rect(pSBElement, drgui_make_rect(thumbRect.right, thumbRect.top, drgui_get_width(pSBElement), thumbRect.bottom), pSB->trackColor, pPaintData); // Right

        // Thumb.
        drgui_color thumbColor;
        if (pSB->thumbPressed) {
            thumbColor = pSB->thumbColorPressed;
        } else if (pSB->thumbHovered) {
            thumbColor = pSB->thumbColorHovered;
        } else {
            thumbColor = pSB->thumbColor;
        }

        drgui_draw_rect(pSBElement, thumbRect, thumbColor, pPaintData);
    }
    else
    {
        // The thumb is not visible - just draw the track as one quad.
        drgui_draw_rect(pSBElement, drgui_get_local_rect(pSBElement), pSB->trackColor, pPaintData);
    }
}



DRGUI_PRIVATE void dred_scrollbar_refresh_thumb(drgui_element* pSBElement)
{
    drgui_scrollbar* pSB = (drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    assert(pSB != NULL);

    drgui_rect oldThumbRect = dred_scrollbar_get_thumb_rect(pSBElement);

    pSB->thumbSize = dred_scrollbar_calculate_thumb_size(pSBElement);
    pSB->thumbPos  = dred_scrollbar_calculate_thumb_position(pSBElement);

    drgui_rect newThumbRect = dred_scrollbar_get_thumb_rect(pSBElement);
    if (!drgui_rect_equal(oldThumbRect, newThumbRect))
    {
        drgui_dirty(pSBElement, drgui_rect_union(oldThumbRect, newThumbRect));
    }
}

DRGUI_PRIVATE float dred_scrollbar_calculate_thumb_size(drgui_element* pSBElement)
{
    const drgui_scrollbar* pSB = (const drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    assert(pSB != NULL);

    float trackSize = dred_scrollbar_get_track_size(pSBElement);
    float range = (float)(pSB->rangeMax - pSB->rangeMin + 1);

    float thumbSize = DRGUI_MIN_SCROLLBAR_THUMB_SIZE;
    if (range > 0)
    {
        thumbSize = roundf((trackSize / range) * pSB->pageSize);
        thumbSize = dred_scrollbar_clampf(thumbSize, DRGUI_MIN_SCROLLBAR_THUMB_SIZE, trackSize);
    }

    return thumbSize;
}

DRGUI_PRIVATE float dred_scrollbar_calculate_thumb_position(drgui_element* pSBElement)
{
    const drgui_scrollbar* pSB = (const drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    assert(pSB != NULL);

    float trackSize = dred_scrollbar_get_track_size(pSBElement);
    float thumbSize = dred_scrollbar_calculate_thumb_size(pSBElement);
    float range = (float)(pSB->rangeMax - pSB->rangeMin + 1);

    float thumbPos = 0;
    if (range > pSB->pageSize)
    {
        thumbPos = roundf((trackSize / range) * pSB->scrollPos);
        thumbPos = dred_scrollbar_clampf(thumbPos, 0, trackSize - thumbSize);
    }

    return thumbPos;
}

DRGUI_PRIVATE float dred_scrollbar_get_track_size(drgui_element* pSBElement)
{
    const drgui_scrollbar* pSB = (const drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    assert(pSB != NULL);

    if (pSB->orientation == dred_scrollbar_orientation_vertical) {
        return drgui_get_height(pSBElement) - (pSB->thumbPadding*2);
    } else {
        return drgui_get_width(pSBElement) - (pSB->thumbPadding*2);
    }
}

DRGUI_PRIVATE void dred_scrollbar_make_relative_to_thumb(drgui_element* pSBElement, float* pPosX, float* pPosY)
{
    drgui_rect thumbRect = dred_scrollbar_get_thumb_rect(pSBElement);

    if (pPosX != NULL) {
        *pPosX -= thumbRect.left;
    }

    if (pPosY != NULL) {
        *pPosY -= thumbRect.top;
    }
}

DRGUI_PRIVATE int dred_scrollbar_calculate_scroll_pos_from_thumb_pos(drgui_element* pSBElement, float thumbPos)
{
    const drgui_scrollbar* pSB = (const drgui_scrollbar*)drgui_get_extra_data(pSBElement);
    assert(pSB != NULL);

    float trackSize = dred_scrollbar_get_track_size(pSBElement);
    float range     = (float)(pSB->rangeMax - pSB->rangeMin + 1);

    return (int)roundf(thumbPos / (trackSize / range));
}
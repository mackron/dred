// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_GUI_MIN_SCROLLBAR_THUMB_SIZE    16

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
    dred_color trackColor;

    /// The color of the thumb while not hovered or pressed.
    dred_color thumbColor;

    /// The color of the thumb while hovered.
    dred_color thumbColorHovered;

    /// The color of the thumb while pressed.
    dred_color thumbColorPressed;

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

} dred_scrollbar_data;


/// Refreshes the given scrollbar's thumb layout and redraws it.
DRED_GUI_PRIVATE void dred_scrollbar_refresh_thumb(dred_scrollbar* pScrollbar);

/// Calculates the size of the thumb. This does not change the state of the thumb.
DRED_GUI_PRIVATE float dred_scrollbar_calculate_thumb_size(dred_scrollbar* pScrollbar);

/// Calculates the position of the thumb. This does not change the state of the thumb.
DRED_GUI_PRIVATE float dred_scrollbar_calculate_thumb_position(dred_scrollbar* pScrollbar);

/// Retrieves the size of the given scrollbar's track. For vertical alignments, it's the height of the element, otherwise it's the width.
DRED_GUI_PRIVATE float dred_scrollbar_get_track_size(dred_scrollbar* pScrollbar);

/// Makes the given point that's relative to the given scrollbar relative to it's thumb.
DRED_GUI_PRIVATE void dred_scrollbar_make_relative_to_thumb(dred_scrollbar* pScrollbar, float* pPosX, float* pPosY);

/// Calculates the scroll position based on the current position of the thumb. This is used for scrolling while dragging the thumb.
DRED_GUI_PRIVATE int dred_scrollbar_calculate_scroll_pos_from_thumb_pos(dred_scrollbar* pScrollbar, float thumbPosr);

/// Simple clamp function.
DRED_GUI_PRIVATE float dred_scrollbar_clampf(float n, float lower, float upper)
{
    return n <= lower ? lower : n >= upper ? upper : n;
}

/// Simple clamp function.
DRED_GUI_PRIVATE int dred_scrollbar_clampi(int n, int lower, int upper)
{
    return n <= lower ? lower : n >= upper ? upper : n;
}

/// Simple max function.
DRED_GUI_PRIVATE int dred_scrollbar_maxi(int x, int y)
{
    return (x > y) ? x : y;
}


dred_scrollbar* dred_scrollbar_create(dred_context* pDred, dred_element* pParent, dred_scrollbar_orientation orientation, size_t extraDataSize, const void* pExtraData)
{
    if (orientation == dred_scrollbar_orientation_none) {
        return NULL;
    }

    dred_scrollbar* pScrollbar = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_SCROLLBAR, sizeof(dred_scrollbar_data) + extraDataSize);
    if (pScrollbar == NULL) {
        return NULL;
    }

    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
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

    pSB->thumbSize         = DRED_GUI_MIN_SCROLLBAR_THUMB_SIZE;
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
    drgui_set_on_size(pScrollbar, dred_scrollbar_on_size);
    drgui_set_on_mouse_leave(pScrollbar, dred_scrollbar_on_mouse_leave);
    drgui_set_on_mouse_move(pScrollbar, dred_scrollbar_on_mouse_move);
    drgui_set_on_mouse_button_down(pScrollbar, dred_scrollbar_on_mouse_button_down);
    drgui_set_on_mouse_button_up(pScrollbar, dred_scrollbar_on_mouse_button_up);
    drgui_set_on_mouse_wheel(pScrollbar, dred_scrollbar_on_mouse_wheel);
    drgui_set_on_paint(pScrollbar, dred_scrollbar_on_paint);


    return pScrollbar;
}

void dred_scrollbar_delete(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return;
    }

    dred_control_delete(pScrollbar);
}


size_t dred_scrollbar_get_extra_data_size(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return 0;
    }

    return pSB->extraDataSize;
}

void* dred_scrollbar_get_extra_data(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return NULL;
    }

    return pSB->pExtraData;
}


dred_scrollbar_orientation dred_scrollbar_get_orientation(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return dred_scrollbar_orientation_none;
    }

    return pSB->orientation;
}


void dred_scrollbar_set_range(dred_scrollbar* pScrollbar, int rangeMin, int rangeMax)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    pSB->rangeMin = rangeMin;
    pSB->rangeMax = rangeMax;


    // Make sure the scroll position is still valid.
    dred_scrollbar_scroll_to(pScrollbar, dred_scrollbar_get_scroll_position(pScrollbar));

    // The thumb may have changed, so refresh it.
    dred_scrollbar_refresh_thumb(pScrollbar);
}

void dred_scrollbar_get_range(dred_scrollbar* pScrollbar, int* pRangeMinOut, int* pRangeMaxOut)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
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


void dred_scrollbar_set_page_size(dred_scrollbar* pScrollbar, int pageSize)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    pSB->pageSize = pageSize;


    // Make sure the scroll position is still valid.
    dred_scrollbar_scroll_to(pScrollbar, dred_scrollbar_get_scroll_position(pScrollbar));

    // The thumb may have changed, so refresh it.
    dred_scrollbar_refresh_thumb(pScrollbar);
}

int dred_scrollbar_get_page_size(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return 0;
    }

    return pSB->pageSize;
}


void dred_scrollbar_set_range_and_page_size(dred_scrollbar* pScrollbar, int rangeMin, int rangeMax, int pageSize)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    pSB->rangeMin = rangeMin;
    pSB->rangeMax = rangeMax;
    pSB->pageSize = pageSize;


    // Make sure the scroll position is still valid.
    dred_scrollbar_scroll_to(pScrollbar, dred_scrollbar_get_scroll_position(pScrollbar));

    // The thumb may have changed, so refresh it.
    dred_scrollbar_refresh_thumb(pScrollbar);
}


void dred_scrollbar_set_scroll_position(dred_scrollbar* pScrollbar, int position)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    int newScrollPos = dred_scrollbar_clampi(position, pSB->rangeMin, dred_scrollbar_maxi(0, pSB->rangeMax - pSB->pageSize + 1));
    if (newScrollPos != pSB->scrollPos)
    {
        pSB->scrollPos = newScrollPos;

        // The position of the thumb has changed, so refresh it.
        dred_scrollbar_refresh_thumb(pScrollbar);
    }
}

int dred_scrollbar_get_scroll_position(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return 0;
    }

    return pSB->scrollPos;
}


void dred_scrollbar_scroll(dred_scrollbar* pScrollbar, int offset)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    dred_scrollbar_scroll_to(pScrollbar, pSB->scrollPos + offset);
}

void dred_scrollbar_scroll_to(dred_scrollbar* pScrollbar, int newScrollPos)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    int oldScrollPos = pSB->scrollPos;
    dred_scrollbar_set_scroll_position(pScrollbar, newScrollPos);

    if (oldScrollPos != pSB->scrollPos)
    {
        if (pSB->onScroll) {
            pSB->onScroll(pScrollbar, pSB->scrollPos);
        }
    }
}


void dred_scrollbar_enable_thumb_auto_hide(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    if (pSB->autoHideThumb != true)
    {
        pSB->autoHideThumb = true;

        // The thumb needs to be refreshed in order to show the correct state.
        dred_scrollbar_refresh_thumb(pScrollbar);
    }
}

void dred_scrollbar_disable_thumb_auto_hide(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    if (pSB->autoHideThumb != false)
    {
        pSB->autoHideThumb = false;

        // The thumb needs to be refreshed in order to show the correct state.
        dred_scrollbar_refresh_thumb(pScrollbar);
    }
}

bool dred_scrollbar_is_thumb_auto_hide_enabled(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return false;
    }

    return pSB->autoHideThumb;
}

bool dred_scrollbar_is_thumb_visible(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return false;
    }

    // Always visible if auto-hiding is disabled.
    if (!pSB->autoHideThumb) {
        return true;
    }

    return pSB->pageSize < (pSB->rangeMax - pSB->rangeMin + 1) && pSB->pageSize > 0;
}


void dred_scrollbar_set_mouse_wheel_scele(dred_scrollbar* pScrollbar, int scale)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    pSB->mouseWheelScale = scale;
}

int dred_scrollbar_get_mouse_wheel_scale(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return 1;
    }

    return pSB->mouseWheelScale;
}


void dred_scrollbar_set_track_color(dred_scrollbar* pScrollbar, dred_color color)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    pSB->trackColor = color;
}

void dred_scrollbar_set_default_thumb_color(dred_scrollbar* pScrollbar, dred_color color)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    pSB->thumbColor = color;
}

void dred_scrollbar_set_hovered_thumb_color(dred_scrollbar* pScrollbar, dred_color color)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    pSB->thumbColorHovered = color;
}

void dred_scrollbar_set_pressed_thumb_color(dred_scrollbar* pScrollbar, dred_color color)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    pSB->thumbColorPressed = color;
}


void dred_scrollbar_set_on_scroll(dred_scrollbar* pScrollbar, dred_scrollbar_on_scroll_proc onScroll)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    pSB->onScroll = onScroll;
}

dred_scrollbar_on_scroll_proc dred_scrollbar_get_on_scroll(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return NULL;
    }

    return pSB->onScroll;
}


dred_rect dred_scrollbar_get_thumb_rect(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return drgui_make_rect(0, 0, 0, 0);
    }

    dred_rect rect = {0, 0, 0, 0};
    rect.left = pSB->thumbPadding;
    rect.top  = pSB->thumbPadding;

    if (pSB->orientation == dred_scrollbar_orientation_vertical)
    {
        // Vertical.
        rect.left   = pSB->thumbPadding;
        rect.right  = drgui_get_width(pScrollbar) - pSB->thumbPadding;
        rect.top    = pSB->thumbPadding + pSB->thumbPos;
        rect.bottom = rect.top + pSB->thumbSize;
    }
    else
    {
        // Horizontal.
        rect.left   = pSB->thumbPadding + pSB->thumbPos;
        rect.right  = rect.left + pSB->thumbSize;
        rect.top    = pSB->thumbPadding;
        rect.bottom = drgui_get_height(pScrollbar) - pSB->thumbPadding;
    }

    return rect;
}


void dred_scrollbar_on_size(dred_scrollbar* pScrollbar, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    dred_scrollbar_refresh_thumb(pScrollbar);
}

void dred_scrollbar_on_mouse_leave(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
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
        drgui_dirty(pScrollbar, dred_scrollbar_get_thumb_rect(pScrollbar));
    }
}

void dred_scrollbar_on_mouse_move(dred_scrollbar* pScrollbar, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    if (pSB->thumbPressed)
    {
        // The thumb is pressed. Drag it.
        float thumbRelativeMousePosX = (float)relativeMousePosX;
        float thumbRelativeMousePosY = (float)relativeMousePosY;
        dred_scrollbar_make_relative_to_thumb(pScrollbar, &thumbRelativeMousePosX, &thumbRelativeMousePosY);

        float dragOffsetX = thumbRelativeMousePosX - pSB->thumbClickPosX;
        float dragOffsetY = thumbRelativeMousePosY - pSB->thumbClickPosY;

        float destTrackPos = pSB->thumbPos;
        if (pSB->orientation == dred_scrollbar_orientation_vertical) {
            destTrackPos += dragOffsetY;
        } else {
            destTrackPos += dragOffsetX;
        }

        int destScrollPos = dred_scrollbar_calculate_scroll_pos_from_thumb_pos(pScrollbar, destTrackPos);
        if (destScrollPos != pSB->scrollPos)
        {
            dred_scrollbar_scroll_to(pScrollbar, destScrollPos);
        }
    }
    else
    {
        // The thumb is not pressed. We just need to check if the hovered state has change and redraw if required.
        if (dred_scrollbar_is_thumb_visible(pScrollbar))
        {
            bool wasThumbHovered = pSB->thumbHovered;

            dred_rect thumbRect = dred_scrollbar_get_thumb_rect(pScrollbar);
            pSB->thumbHovered = dred_rect_contains_point(thumbRect, (float)relativeMousePosX, (float)relativeMousePosY);

            if (wasThumbHovered != pSB->thumbHovered) {
                drgui_dirty(pScrollbar, thumbRect);
            }
        }
    }
}

void dred_scrollbar_on_mouse_button_down(dred_scrollbar* pScrollbar, int button, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    if (button == DRED_GUI_MOUSE_BUTTON_LEFT)
    {
        if (dred_scrollbar_is_thumb_visible(pScrollbar))
        {
            dred_rect thumbRect = dred_scrollbar_get_thumb_rect(pScrollbar);
            if (dred_rect_contains_point(thumbRect, (float)relativeMousePosX, (float)relativeMousePosY))
            {
                if (!pSB->thumbPressed)
                {
                    drgui_capture_mouse(pScrollbar);
                    pSB->thumbPressed = true;

                    pSB->thumbClickPosX = (float)relativeMousePosX;
                    pSB->thumbClickPosY = (float)relativeMousePosY;
                    dred_scrollbar_make_relative_to_thumb(pScrollbar, &pSB->thumbClickPosX, &pSB->thumbClickPosY);

                    drgui_dirty(pScrollbar, dred_scrollbar_get_thumb_rect(pScrollbar));
                }
            }
            else
            {
                // If the click position is above the thumb we want to scroll up by a page. If it's below the thumb, we scroll down by a page.
                if (relativeMousePosY < thumbRect.top) {
                    dred_scrollbar_scroll(pScrollbar, -dred_scrollbar_get_page_size(pScrollbar));
                } else if (relativeMousePosY >= thumbRect.bottom) {
                    dred_scrollbar_scroll(pScrollbar,  dred_scrollbar_get_page_size(pScrollbar));
                }
            }
        }
    }
}

void dred_scrollbar_on_mouse_button_up(dred_scrollbar* pScrollbar, int button, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    if (button == DRED_GUI_MOUSE_BUTTON_LEFT)
    {
        if (pSB->thumbPressed && drgui_get_element_with_mouse_capture(pScrollbar->pContext) == pScrollbar)
        {
            drgui_release_mouse(pScrollbar->pContext);
            pSB->thumbPressed = false;

            drgui_dirty(pScrollbar, dred_scrollbar_get_thumb_rect(pScrollbar));
        }
    }
}

void dred_scrollbar_on_mouse_wheel(dred_scrollbar* pScrollbar, int delta, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    dred_scrollbar_scroll(pScrollbar, -delta * dred_scrollbar_get_mouse_wheel_scale(pScrollbar));
}

void dred_scrollbar_on_paint(dred_scrollbar* pScrollbar, dred_rect relativeClippingRect, void* pPaintData)
{
    (void)relativeClippingRect;

    const dred_scrollbar_data* pSB = (const dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    if (pSB == NULL) {
        return;
    }

    dred_rect thumbRect = dred_scrollbar_get_thumb_rect(pScrollbar);

    if (dred_scrollbar_is_thumb_visible(pScrollbar))
    {
        // The thumb is visible.

        // Track. We draw this in 4 seperate pieces so we can avoid overdraw with the thumb.
        drgui_draw_rect(pScrollbar, drgui_make_rect(0, 0, drgui_get_width(pScrollbar), thumbRect.top), pSB->trackColor, pPaintData);  // Top
        drgui_draw_rect(pScrollbar, drgui_make_rect(0, thumbRect.bottom, drgui_get_width(pScrollbar), drgui_get_height(pScrollbar)), pSB->trackColor, pPaintData);  // Bottom
        drgui_draw_rect(pScrollbar, drgui_make_rect(0, thumbRect.top, thumbRect.left, thumbRect.bottom), pSB->trackColor, pPaintData);  // Left
        drgui_draw_rect(pScrollbar, drgui_make_rect(thumbRect.right, thumbRect.top, drgui_get_width(pScrollbar), thumbRect.bottom), pSB->trackColor, pPaintData); // Right

        // Thumb.
        dred_color thumbColor;
        if (pSB->thumbPressed) {
            thumbColor = pSB->thumbColorPressed;
        } else if (pSB->thumbHovered) {
            thumbColor = pSB->thumbColorHovered;
        } else {
            thumbColor = pSB->thumbColor;
        }

        drgui_draw_rect(pScrollbar, thumbRect, thumbColor, pPaintData);
    }
    else
    {
        // The thumb is not visible - just draw the track as one quad.
        drgui_draw_rect(pScrollbar, drgui_get_local_rect(pScrollbar), pSB->trackColor, pPaintData);
    }
}



DRED_GUI_PRIVATE void dred_scrollbar_refresh_thumb(dred_scrollbar* pScrollbar)
{
    dred_scrollbar_data* pSB = (dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    assert(pSB != NULL);

    dred_rect oldThumbRect = dred_scrollbar_get_thumb_rect(pScrollbar);

    pSB->thumbSize = dred_scrollbar_calculate_thumb_size(pScrollbar);
    pSB->thumbPos  = dred_scrollbar_calculate_thumb_position(pScrollbar);

    dred_rect newThumbRect = dred_scrollbar_get_thumb_rect(pScrollbar);
    if (!dred_rect_equal(oldThumbRect, newThumbRect))
    {
        drgui_dirty(pScrollbar, dred_rect_union(oldThumbRect, newThumbRect));
    }
}

DRED_GUI_PRIVATE float dred_scrollbar_calculate_thumb_size(dred_scrollbar* pScrollbar)
{
    const dred_scrollbar_data* pSB = (const dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    assert(pSB != NULL);

    float trackSize = dred_scrollbar_get_track_size(pScrollbar);
    float range = (float)(pSB->rangeMax - pSB->rangeMin + 1);

    float thumbSize = DRED_GUI_MIN_SCROLLBAR_THUMB_SIZE;
    if (range > 0)
    {
        thumbSize = roundf((trackSize / range) * pSB->pageSize);
        thumbSize = dred_scrollbar_clampf(thumbSize, DRED_GUI_MIN_SCROLLBAR_THUMB_SIZE, trackSize);
    }

    return thumbSize;
}

DRED_GUI_PRIVATE float dred_scrollbar_calculate_thumb_position(dred_scrollbar* pScrollbar)
{
    const dred_scrollbar_data* pSB = (const dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    assert(pSB != NULL);

    float trackSize = dred_scrollbar_get_track_size(pScrollbar);
    float thumbSize = dred_scrollbar_calculate_thumb_size(pScrollbar);
    float range = (float)(pSB->rangeMax - pSB->rangeMin + 1);

    float thumbPos = 0;
    if (range > pSB->pageSize)
    {
        thumbPos = roundf((trackSize / range) * pSB->scrollPos);
        thumbPos = dred_scrollbar_clampf(thumbPos, 0, trackSize - thumbSize);
    }

    return thumbPos;
}

DRED_GUI_PRIVATE float dred_scrollbar_get_track_size(dred_scrollbar* pScrollbar)
{
    const dred_scrollbar_data* pSB = (const dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    assert(pSB != NULL);

    if (pSB->orientation == dred_scrollbar_orientation_vertical) {
        return drgui_get_height(pScrollbar) - (pSB->thumbPadding*2);
    } else {
        return drgui_get_width(pScrollbar) - (pSB->thumbPadding*2);
    }
}

DRED_GUI_PRIVATE void dred_scrollbar_make_relative_to_thumb(dred_scrollbar* pScrollbar, float* pPosX, float* pPosY)
{
    dred_rect thumbRect = dred_scrollbar_get_thumb_rect(pScrollbar);

    if (pPosX != NULL) {
        *pPosX -= thumbRect.left;
    }

    if (pPosY != NULL) {
        *pPosY -= thumbRect.top;
    }
}

DRED_GUI_PRIVATE int dred_scrollbar_calculate_scroll_pos_from_thumb_pos(dred_scrollbar* pScrollbar, float thumbPos)
{
    const dred_scrollbar_data* pSB = (const dred_scrollbar_data*)dred_control_get_extra_data(pScrollbar);
    assert(pSB != NULL);

    float trackSize = dred_scrollbar_get_track_size(pScrollbar);
    float range     = (float)(pSB->rangeMax - pSB->rangeMin + 1);

    return (int)roundf(thumbPos / (trackSize / range));
}
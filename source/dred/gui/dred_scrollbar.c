// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_GUI_MIN_SCROLLBAR_THUMB_SIZE    16

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


dr_bool32 dred_scrollbar_init(dred_scrollbar* pScrollbar, dred_context* pDred, dred_control* pParent, dred_scrollbar_orientation orientation)
{
    if (pScrollbar == NULL || orientation == dred_scrollbar_orientation_none) {
        return DR_FALSE;
    }

    if (!dred_control_init(DRED_CONTROL(pScrollbar), pDred, pParent, DRED_CONTROL_TYPE_SCROLLBAR)) {
        return DR_FALSE;
    }

    pScrollbar->orientation       = orientation;
    pScrollbar->rangeMin          = 0;
    pScrollbar->rangeMax          = 0;
    pScrollbar->pageSize          = 0;
    pScrollbar->scrollPos         = 0;
    pScrollbar->autoHideThumb     = DR_TRUE;
    pScrollbar->mouseWheelScale   = 1;
    pScrollbar->trackColor        = dred_rgb(80, 80, 80);
    pScrollbar->thumbColor        = dred_rgb(112, 112, 112);
    pScrollbar->thumbColorHovered = dred_rgb(144, 144, 144);
    pScrollbar->thumbColorPressed = dred_rgb(180, 180, 180);
    pScrollbar->onScroll          = NULL;

    pScrollbar->thumbSize         = DRED_GUI_MIN_SCROLLBAR_THUMB_SIZE;
    pScrollbar->thumbPos          = 0;
    pScrollbar->thumbPadding      = 2;
    pScrollbar->thumbHovered      = DR_FALSE;
    pScrollbar->thumbPressed      = DR_FALSE;
    pScrollbar->thumbClickPosX    = 0;
    pScrollbar->thumbClickPosY    = 0;


    // Default event handlers.
    dred_control_set_on_size(DRED_CONTROL(pScrollbar), dred_scrollbar_on_size);
    dred_control_set_on_mouse_leave(DRED_CONTROL(pScrollbar), dred_scrollbar_on_mouse_leave);
    dred_control_set_on_mouse_move(DRED_CONTROL(pScrollbar), dred_scrollbar_on_mouse_move);
    dred_control_set_on_mouse_button_down(DRED_CONTROL(pScrollbar), dred_scrollbar_on_mouse_button_down);
    dred_control_set_on_mouse_button_up(DRED_CONTROL(pScrollbar), dred_scrollbar_on_mouse_button_up);
    dred_control_set_on_mouse_wheel(DRED_CONTROL(pScrollbar), dred_scrollbar_on_mouse_wheel);
    dred_control_set_on_paint(DRED_CONTROL(pScrollbar), dred_scrollbar_on_paint);


    return DR_TRUE;
}

void dred_scrollbar_uninit(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return;
    }

    dred_control_uninit(DRED_CONTROL(pScrollbar));
}


dred_scrollbar_orientation dred_scrollbar_get_orientation(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return dred_scrollbar_orientation_none;
    }

    return pScrollbar->orientation;
}


void dred_scrollbar_set_range(dred_scrollbar* pScrollbar, int rangeMin, int rangeMax)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->rangeMin = rangeMin;
    pScrollbar->rangeMax = rangeMax;


    // Make sure the scroll position is still valid.
    dred_scrollbar_scroll_to(pScrollbar, dred_scrollbar_get_scroll_position(pScrollbar));

    // The thumb may have changed, so refresh it.
    dred_scrollbar_refresh_thumb(pScrollbar);
}

void dred_scrollbar_get_range(dred_scrollbar* pScrollbar, int* pRangeMinOut, int* pRangeMaxOut)
{
    if (pScrollbar == NULL) {
        return;
    }

    if (pRangeMinOut != NULL) {
        *pRangeMinOut = pScrollbar->rangeMin;
    }

    if (pRangeMaxOut != NULL) {
        *pRangeMaxOut = pScrollbar->rangeMax;
    }
}


void dred_scrollbar_set_page_size(dred_scrollbar* pScrollbar, int pageSize)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->pageSize = pageSize;


    // Make sure the scroll position is still valid.
    dred_scrollbar_scroll_to(pScrollbar, dred_scrollbar_get_scroll_position(pScrollbar));

    // The thumb may have changed, so refresh it.
    dred_scrollbar_refresh_thumb(pScrollbar);
}

int dred_scrollbar_get_page_size(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return 0;
    }

    return pScrollbar->pageSize;
}


void dred_scrollbar_set_range_and_page_size(dred_scrollbar* pScrollbar, int rangeMin, int rangeMax, int pageSize)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->rangeMin = rangeMin;
    pScrollbar->rangeMax = rangeMax;
    pScrollbar->pageSize = pageSize;


    // Make sure the scroll position is still valid.
    dred_scrollbar_scroll_to(pScrollbar, dred_scrollbar_get_scroll_position(pScrollbar));

    // The thumb may have changed, so refresh it.
    dred_scrollbar_refresh_thumb(pScrollbar);
}


void dred_scrollbar_set_scroll_position(dred_scrollbar* pScrollbar, int position)
{
    if (pScrollbar == NULL) {
        return;
    }

    int newScrollPos = dred_scrollbar_clampi(position, pScrollbar->rangeMin, dred_scrollbar_maxi(0, pScrollbar->rangeMax - pScrollbar->pageSize + 1));
    if (newScrollPos != pScrollbar->scrollPos)
    {
        pScrollbar->scrollPos = newScrollPos;

        // The position of the thumb has changed, so refresh it.
        dred_scrollbar_refresh_thumb(pScrollbar);
    }
}

int dred_scrollbar_get_scroll_position(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return 0;
    }

    return pScrollbar->scrollPos;
}


void dred_scrollbar_scroll(dred_scrollbar* pScrollbar, int offset)
{
    if (pScrollbar == NULL) {
        return;
    }

    dred_scrollbar_scroll_to(pScrollbar, pScrollbar->scrollPos + offset);
}

void dred_scrollbar_scroll_to(dred_scrollbar* pScrollbar, int newScrollPos)
{
    if (pScrollbar == NULL) {
        return;
    }

    int oldScrollPos = pScrollbar->scrollPos;
    dred_scrollbar_set_scroll_position(pScrollbar, newScrollPos);

    if (oldScrollPos != pScrollbar->scrollPos)
    {
        if (pScrollbar->onScroll) {
            pScrollbar->onScroll(pScrollbar, pScrollbar->scrollPos);
        }
    }
}


void dred_scrollbar_enable_thumb_auto_hide(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return;
    }

    if (pScrollbar->autoHideThumb != DR_TRUE)
    {
        pScrollbar->autoHideThumb = DR_TRUE;

        // The thumb needs to be refreshed in order to show the correct state.
        dred_scrollbar_refresh_thumb(pScrollbar);
    }
}

void dred_scrollbar_disable_thumb_auto_hide(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return;
    }

    if (pScrollbar->autoHideThumb != DR_FALSE)
    {
        pScrollbar->autoHideThumb = DR_FALSE;

        // The thumb needs to be refreshed in order to show the correct state.
        dred_scrollbar_refresh_thumb(pScrollbar);
    }
}

dr_bool32 dred_scrollbar_is_thumb_auto_hide_enabled(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return DR_FALSE;
    }

    return pScrollbar->autoHideThumb;
}

dr_bool32 dred_scrollbar_is_thumb_visible(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return DR_FALSE;
    }

    // Always visible if auto-hiding is disabled.
    if (!pScrollbar->autoHideThumb) {
        return DR_TRUE;
    }

    return pScrollbar->pageSize < (pScrollbar->rangeMax - pScrollbar->rangeMin + 1) && pScrollbar->pageSize > 0;
}


void dred_scrollbar_set_mouse_wheel_scele(dred_scrollbar* pScrollbar, int scale)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->mouseWheelScale = scale;
}

int dred_scrollbar_get_mouse_wheel_scale(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return 1;
    }

    return pScrollbar->mouseWheelScale;
}


void dred_scrollbar_set_track_color(dred_scrollbar* pScrollbar, dred_color color)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->trackColor = color;
}

void dred_scrollbar_set_default_thumb_color(dred_scrollbar* pScrollbar, dred_color color)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->thumbColor = color;
}

void dred_scrollbar_set_hovered_thumb_color(dred_scrollbar* pScrollbar, dred_color color)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->thumbColorHovered = color;
}

void dred_scrollbar_set_pressed_thumb_color(dred_scrollbar* pScrollbar, dred_color color)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->thumbColorPressed = color;
}


void dred_scrollbar_set_on_scroll(dred_scrollbar* pScrollbar, dred_scrollbar_on_scroll_proc onScroll)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->onScroll = onScroll;
}

dred_scrollbar_on_scroll_proc dred_scrollbar_get_on_scroll(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return NULL;
    }

    return pScrollbar->onScroll;
}


dred_rect dred_scrollbar_get_thumb_rect(dred_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return dred_make_rect(0, 0, 0, 0);
    }

    dred_rect rect = {0, 0, 0, 0};
    rect.left = pScrollbar->thumbPadding;
    rect.top  = pScrollbar->thumbPadding;

    if (pScrollbar->orientation == dred_scrollbar_orientation_vertical)
    {
        // Vertical.
        rect.left   = pScrollbar->thumbPadding;
        rect.right  = dred_control_get_width(DRED_CONTROL(pScrollbar)) - pScrollbar->thumbPadding;
        rect.top    = pScrollbar->thumbPadding + pScrollbar->thumbPos;
        rect.bottom = rect.top + pScrollbar->thumbSize;
    }
    else
    {
        // Horizontal.
        rect.left   = pScrollbar->thumbPadding + pScrollbar->thumbPos;
        rect.right  = rect.left + pScrollbar->thumbSize;
        rect.top    = pScrollbar->thumbPadding;
        rect.bottom = dred_control_get_height(DRED_CONTROL(pScrollbar)) - pScrollbar->thumbPadding;
    }

    return rect;
}


void dred_scrollbar_on_size(dred_control* pControl, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    dred_scrollbar* pScrollbar = DRED_SCROLLBAR(pControl);
    if (pScrollbar == NULL) {
        return;
    }

    dred_scrollbar_refresh_thumb(pScrollbar);
}

void dred_scrollbar_on_mouse_leave(dred_control* pControl)
{
    dred_scrollbar* pScrollbar = DRED_SCROLLBAR(pControl);
    if (pScrollbar == NULL) {
        return;
    }

    dr_bool32 needsRedraw = DR_FALSE;
    if (pScrollbar->thumbHovered)
    {
        needsRedraw = DR_TRUE;
        pScrollbar->thumbHovered = DR_FALSE;
    }

    if (pScrollbar->thumbPressed)
    {
        needsRedraw = DR_TRUE;
        pScrollbar->thumbPressed = DR_FALSE;
    }

    if (needsRedraw) {
        dred_control_dirty(DRED_CONTROL(pScrollbar), dred_scrollbar_get_thumb_rect(pScrollbar));
    }
}

void dred_scrollbar_on_mouse_move(dred_control* pControl, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_scrollbar* pScrollbar = DRED_SCROLLBAR(pControl);
    if (pScrollbar == NULL) {
        return;
    }

    if (pScrollbar->thumbPressed)
    {
        // The thumb is pressed. Drag it.
        float thumbRelativeMousePosX = (float)relativeMousePosX;
        float thumbRelativeMousePosY = (float)relativeMousePosY;
        dred_scrollbar_make_relative_to_thumb(pScrollbar, &thumbRelativeMousePosX, &thumbRelativeMousePosY);

        float dragOffsetX = thumbRelativeMousePosX - pScrollbar->thumbClickPosX;
        float dragOffsetY = thumbRelativeMousePosY - pScrollbar->thumbClickPosY;

        float destTrackPos = pScrollbar->thumbPos;
        if (pScrollbar->orientation == dred_scrollbar_orientation_vertical) {
            destTrackPos += dragOffsetY;
        } else {
            destTrackPos += dragOffsetX;
        }

        int destScrollPos = dred_scrollbar_calculate_scroll_pos_from_thumb_pos(pScrollbar, destTrackPos);
        if (destScrollPos != pScrollbar->scrollPos)
        {
            dred_scrollbar_scroll_to(pScrollbar, destScrollPos);
        }
    }
    else
    {
        // The thumb is not pressed. We just need to check if the hovered state has change and redraw if required.
        if (dred_scrollbar_is_thumb_visible(pScrollbar))
        {
            dr_bool32 wasThumbHovered = pScrollbar->thumbHovered;

            dred_rect thumbRect = dred_scrollbar_get_thumb_rect(pScrollbar);
            pScrollbar->thumbHovered = dred_rect_contains_point(thumbRect, (float)relativeMousePosX, (float)relativeMousePosY);

            if (wasThumbHovered != pScrollbar->thumbHovered) {
                dred_control_dirty(DRED_CONTROL(pScrollbar), thumbRect);
            }
        }
    }
}

void dred_scrollbar_on_mouse_button_down(dred_control* pControl, int button, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_scrollbar* pScrollbar = DRED_SCROLLBAR(pControl);
    if (pScrollbar == NULL) {
        return;
    }

    if (button == DRED_GUI_MOUSE_BUTTON_LEFT)
    {
        if (dred_scrollbar_is_thumb_visible(pScrollbar))
        {
            dred_rect thumbRect = dred_scrollbar_get_thumb_rect(pScrollbar);
            if (dred_rect_contains_point(thumbRect, (float)relativeMousePosX, (float)relativeMousePosY))
            {
                if (!pScrollbar->thumbPressed)
                {
                    dred_gui_capture_mouse(DRED_CONTROL(pScrollbar));
                    pScrollbar->thumbPressed = DR_TRUE;

                    pScrollbar->thumbClickPosX = (float)relativeMousePosX;
                    pScrollbar->thumbClickPosY = (float)relativeMousePosY;
                    dred_scrollbar_make_relative_to_thumb(pScrollbar, &pScrollbar->thumbClickPosX, &pScrollbar->thumbClickPosY);

                    dred_control_dirty(DRED_CONTROL(pScrollbar), dred_scrollbar_get_thumb_rect(pScrollbar));
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

void dred_scrollbar_on_mouse_button_up(dred_control* pControl, int button, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_scrollbar* pScrollbar = DRED_SCROLLBAR(pControl);
    if (pScrollbar == NULL) {
        return;
    }

    if (button == DRED_GUI_MOUSE_BUTTON_LEFT)
    {
        if (pScrollbar->thumbPressed && dred_gui_get_element_with_mouse_capture(dred_control_get_gui(pControl)) == pControl)
        {
            dred_gui_release_mouse(dred_control_get_gui(pControl));
            pScrollbar->thumbPressed = DR_FALSE;

            dred_control_dirty(pControl, dred_scrollbar_get_thumb_rect(pScrollbar));
        }
    }
}

void dred_scrollbar_on_mouse_wheel(dred_control* pControl, int delta, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dred_scrollbar* pScrollbar = DRED_SCROLLBAR(pControl);
    if (pScrollbar == NULL) {
        return;
    }

    dred_scrollbar_scroll(pScrollbar, -delta * dred_scrollbar_get_mouse_wheel_scale(pScrollbar));
}

void dred_scrollbar_on_paint(dred_control* pControl, dred_rect relativeClippingRect, dtk_surface* pSurface)
{
    (void)relativeClippingRect;

    dred_scrollbar* pScrollbar = DRED_SCROLLBAR(pControl);
    if (pScrollbar == NULL) {
        return;
    }

    dred_rect thumbRect = dred_scrollbar_get_thumb_rect(pScrollbar);

    if (dred_scrollbar_is_thumb_visible(pScrollbar))
    {
        // The thumb is visible.

        // Track. We draw this in 4 seperate pieces so we can avoid overdraw with the thumb.
        dred_control_draw_rect(pControl, dred_make_rect(0, 0, dred_control_get_width(pControl), thumbRect.top), pScrollbar->trackColor, pSurface);  // Top
        dred_control_draw_rect(pControl, dred_make_rect(0, thumbRect.bottom, dred_control_get_width(pControl), dred_control_get_height(pControl)), pScrollbar->trackColor, pSurface);  // Bottom
        dred_control_draw_rect(pControl, dred_make_rect(0, thumbRect.top, thumbRect.left, thumbRect.bottom), pScrollbar->trackColor, pSurface);  // Left
        dred_control_draw_rect(pControl, dred_make_rect(thumbRect.right, thumbRect.top, dred_control_get_width(pControl), thumbRect.bottom), pScrollbar->trackColor, pSurface); // Right

        // Thumb.
        dred_color thumbColor;
        if (pScrollbar->thumbPressed) {
            thumbColor = pScrollbar->thumbColorPressed;
        } else if (pScrollbar->thumbHovered) {
            thumbColor = pScrollbar->thumbColorHovered;
        } else {
            thumbColor = pScrollbar->thumbColor;
        }

        dred_control_draw_rect(pControl, thumbRect, thumbColor, pSurface);
    }
    else
    {
        // The thumb is not visible - just draw the track as one quad.
        dred_control_draw_rect(pControl, dred_control_get_local_rect(pControl), pScrollbar->trackColor, pSurface);
    }
}



DRED_GUI_PRIVATE void dred_scrollbar_refresh_thumb(dred_scrollbar* pScrollbar)
{
    assert(pScrollbar != NULL);

    dred_rect oldThumbRect = dred_scrollbar_get_thumb_rect(pScrollbar);

    pScrollbar->thumbSize = dred_scrollbar_calculate_thumb_size(pScrollbar);
    pScrollbar->thumbPos  = dred_scrollbar_calculate_thumb_position(pScrollbar);

    dred_rect newThumbRect = dred_scrollbar_get_thumb_rect(pScrollbar);
    if (!dred_rect_equal(oldThumbRect, newThumbRect))
    {
        dred_control_dirty(DRED_CONTROL(pScrollbar), dred_rect_union(oldThumbRect, newThumbRect));
    }
}

DRED_GUI_PRIVATE float dred_scrollbar_calculate_thumb_size(dred_scrollbar* pScrollbar)
{
    assert(pScrollbar != NULL);

    float trackSize = dred_scrollbar_get_track_size(pScrollbar);
    float range = (float)(pScrollbar->rangeMax - pScrollbar->rangeMin + 1);

    float thumbSize = DRED_GUI_MIN_SCROLLBAR_THUMB_SIZE;
    if (range > 0)
    {
        thumbSize = roundf((trackSize / range) * pScrollbar->pageSize);
        thumbSize = dred_scrollbar_clampf(thumbSize, DRED_GUI_MIN_SCROLLBAR_THUMB_SIZE, trackSize);
    }

    return thumbSize;
}

DRED_GUI_PRIVATE float dred_scrollbar_calculate_thumb_position(dred_scrollbar* pScrollbar)
{
    assert(pScrollbar != NULL);

    float trackSize = dred_scrollbar_get_track_size(pScrollbar);
    float thumbSize = dred_scrollbar_calculate_thumb_size(pScrollbar);
    float range = (float)(pScrollbar->rangeMax - pScrollbar->rangeMin + 1);

    float thumbPos = 0;
    if (range > pScrollbar->pageSize)
    {
        thumbPos = roundf((trackSize / range) * pScrollbar->scrollPos);
        thumbPos = dred_scrollbar_clampf(thumbPos, 0, trackSize - thumbSize);
    }

    return thumbPos;
}

DRED_GUI_PRIVATE float dred_scrollbar_get_track_size(dred_scrollbar* pScrollbar)
{
    assert(pScrollbar != NULL);

    if (pScrollbar->orientation == dred_scrollbar_orientation_vertical) {
        return dred_control_get_height(DRED_CONTROL(pScrollbar)) - (pScrollbar->thumbPadding*2);
    } else {
        return dred_control_get_width(DRED_CONTROL(pScrollbar)) - (pScrollbar->thumbPadding*2);
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
    assert(pScrollbar != NULL);

    float trackSize = dred_scrollbar_get_track_size(pScrollbar);
    float range     = (float)(pScrollbar->rangeMax - pScrollbar->rangeMin + 1);

    return (int)roundf(thumbPos / (trackSize / range));
}
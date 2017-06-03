// Copyright (C) 2017 David Reid. See included LICENSE file.

#define DTK_SCROLLBAR_MIN_THUMB_SIZE    16

// Refreshes the given scrollbar's thumb layout and redraws it.
static void dtk_scrollbar_refresh_thumb(dtk_scrollbar* pScrollbar);

// Calculates the size of the thumb. This does not change the state of the thumb.
static dtk_int32 dtk_scrollbar_calculate_thumb_size(dtk_scrollbar* pScrollbar);

// Calculates the position of the thumb. This does not change the state of the thumb.
static dtk_int32 dtk_scrollbar_calculate_thumb_position(dtk_scrollbar* pScrollbar);

// Retrieves the size of the given scrollbar's track. For vertical alignments, it's the height of the element, otherwise it's the width.
static dtk_int32 dtk_scrollbar_get_track_size(dtk_scrollbar* pScrollbar);

// Makes the given point that's relative to the given scrollbar relative to it's thumb.
static void dtk_scrollbar_make_relative_to_thumb(dtk_scrollbar* pScrollbar, dtk_int32* pPosX, dtk_int32* pPosY);

// Calculates the scroll position based on the current position of the thumb. This is used for scrolling while dragging the thumb.
static dtk_int32 dtk_scrollbar_calculate_scroll_pos_from_thumb_pos(dtk_scrollbar* pScrollbar, dtk_int32 thumbPosr);


void dtk_scrollbar__on_size(dtk_scrollbar* pScrollbar, dtk_uint32 newWidth, dtk_uint32 newHeight)
{
    (void)newWidth;
    (void)newHeight;

    dtk_scrollbar_refresh_thumb(pScrollbar);
}

void dtk_scrollbar__on_mouse_leave(dtk_scrollbar* pScrollbar)
{
    dtk_bool32 needsRedraw = DTK_FALSE;
    if (pScrollbar->thumbHovered) {
        needsRedraw = DTK_TRUE;
        pScrollbar->thumbHovered = DTK_FALSE;
    }

    if (pScrollbar->thumbPressed) {
        needsRedraw = DTK_TRUE;
        pScrollbar->thumbPressed = DTK_FALSE;
    }

    if (needsRedraw) {
        dtk_control_scheduled_redraw(DTK_CONTROL(pScrollbar), dtk_scrollbar_get_thumb_rect(pScrollbar));
    }
}

void dtk_scrollbar__on_mouse_move(dtk_scrollbar* pScrollbar, dtk_int32 relativeMousePosX, dtk_int32 relativeMousePosY, dtk_int32 stateFlags)
{
    (void)stateFlags;

    if (pScrollbar->thumbPressed) {
        // The thumb is pressed. Drag it.
        dtk_int32 thumbRelativeMousePosX = relativeMousePosX;
        dtk_int32 thumbRelativeMousePosY = relativeMousePosY;
        dtk_scrollbar_make_relative_to_thumb(pScrollbar, &thumbRelativeMousePosX, &thumbRelativeMousePosY);

        dtk_int32 dragOffsetX = thumbRelativeMousePosX - pScrollbar->thumbClickPosX;
        dtk_int32 dragOffsetY = thumbRelativeMousePosY - pScrollbar->thumbClickPosY;

        dtk_int32 destTrackPos = pScrollbar->thumbPos;
        if (pScrollbar->orientation == dtk_scrollbar_orientation_vertical) {
            destTrackPos += dragOffsetY;
        } else {
            destTrackPos += dragOffsetX;
        }

        dtk_int32 destScrollPos = dtk_scrollbar_calculate_scroll_pos_from_thumb_pos(pScrollbar, destTrackPos);
        if (destScrollPos != pScrollbar->scrollPos) {
            dtk_scrollbar_scroll_to(pScrollbar, destScrollPos);
        }
    } else {
        // The thumb is not pressed. We just need to check if the hovered state has change and redraw if required.
        if (dtk_scrollbar_is_thumb_visible(pScrollbar)) {
            dtk_bool32 wasThumbHovered = pScrollbar->thumbHovered;

            dtk_rect thumbRect = dtk_scrollbar_get_thumb_rect(pScrollbar);
            pScrollbar->thumbHovered = dtk_rect_contains_point(thumbRect, relativeMousePosX, relativeMousePosY);

            if (wasThumbHovered != pScrollbar->thumbHovered) {
                dtk_control_scheduled_redraw(DTK_CONTROL(pScrollbar), thumbRect);
            }
        }
    }
}

void dtk_scrollbar__on_mouse_button_down(dtk_scrollbar* pScrollbar, dtk_int32 button, dtk_int32 relativeMousePosX, dtk_int32 relativeMousePosY, dtk_int32 stateFlags)
{
    (void)stateFlags;

    if (button == DTK_MOUSE_BUTTON_LEFT) {
        if (dtk_scrollbar_is_thumb_visible(pScrollbar)) {
            dtk_rect thumbRect = dtk_scrollbar_get_thumb_rect(pScrollbar);
            if (dtk_rect_contains_point(thumbRect, relativeMousePosX, relativeMousePosY)) {
                if (!pScrollbar->thumbPressed) {
                    dtk_control_capture_mouse(DTK_CONTROL(pScrollbar));
                    pScrollbar->thumbPressed = DTK_TRUE;

                    pScrollbar->thumbClickPosX = relativeMousePosX;
                    pScrollbar->thumbClickPosY = relativeMousePosY;
                    dtk_scrollbar_make_relative_to_thumb(pScrollbar, &pScrollbar->thumbClickPosX, &pScrollbar->thumbClickPosY);

                    dtk_control_scheduled_redraw(DTK_CONTROL(pScrollbar), dtk_scrollbar_get_thumb_rect(pScrollbar));
                }
            } else {
                // If the click position is above the thumb we want to scroll up by a page. If it's below the thumb, we scroll down by a page.
                if (relativeMousePosY < thumbRect.top) {
                    dtk_scrollbar_scroll(pScrollbar, -dtk_scrollbar_get_page_size(pScrollbar));
                } else if (relativeMousePosY >= thumbRect.bottom) {
                    dtk_scrollbar_scroll(pScrollbar,  dtk_scrollbar_get_page_size(pScrollbar));
                }
            }
        }
    }
}

void dtk_scrollbar__on_mouse_button_up(dtk_scrollbar* pScrollbar, dtk_int32 button, dtk_int32 relativeMousePosX, dtk_int32 relativeMousePosY, dtk_int32 stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dtk_context* pTK = DTK_CONTROL(pScrollbar)->pTK;

    if (button == DTK_MOUSE_BUTTON_LEFT) {
        if (pScrollbar->thumbPressed && dtk_get_control_with_mouse_capture(pTK) == DTK_CONTROL(pScrollbar)) {
            dtk_release_mouse(DTK_CONTROL(pScrollbar)->pTK);
            pScrollbar->thumbPressed = DTK_FALSE;

            dtk_control_scheduled_redraw(DTK_CONTROL(pScrollbar), dtk_scrollbar_get_thumb_rect(pScrollbar));
        }
    }
}

void dtk_scrollbar__on_mouse_wheel(dtk_scrollbar* pScrollbar, dtk_int32 delta, dtk_int32 relativeMousePosX, dtk_int32 relativeMousePosY, dtk_int32 stateFlags)
{
    (void)relativeMousePosX;
    (void)relativeMousePosY;
    (void)stateFlags;

    dtk_scrollbar_scroll(pScrollbar, -delta * dtk_scrollbar_get_mouse_wheel_scale(pScrollbar));
}

void dtk_scrollbar__on_paint(dtk_scrollbar* pScrollbar, dtk_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dtk_rect thumbRect = dtk_scrollbar_get_thumb_rect(pScrollbar);

    if (dtk_scrollbar_is_thumb_visible(pScrollbar)) {
        // The thumb is visible.

        // Track. We draw this in 4 seperate pieces so we can avoid overdraw with the thumb.
        dtk_surface_draw_rect(pSurface, dtk_rect_init(0,                0,                dtk_control_get_width(DTK_CONTROL(pScrollbar)), thumbRect.top),                                   pScrollbar->trackColor);  // Top
        dtk_surface_draw_rect(pSurface, dtk_rect_init(0,                thumbRect.bottom, dtk_control_get_width(DTK_CONTROL(pScrollbar)), dtk_control_get_height(DTK_CONTROL(pScrollbar))), pScrollbar->trackColor);  // Bottom
        dtk_surface_draw_rect(pSurface, dtk_rect_init(0,                thumbRect.top,    thumbRect.left,                                 thumbRect.bottom),                                pScrollbar->trackColor);  // Left
        dtk_surface_draw_rect(pSurface, dtk_rect_init(thumbRect.right,  thumbRect.top,    dtk_control_get_width(DTK_CONTROL(pScrollbar)), thumbRect.bottom),                                pScrollbar->trackColor); // Right

        // Thumb.
        dtk_color thumbColor;
        if (pScrollbar->thumbPressed) {
            thumbColor = pScrollbar->thumbColorPressed;
        } else if (pScrollbar->thumbHovered) {
            thumbColor = pScrollbar->thumbColorHovered;
        } else {
            thumbColor = pScrollbar->thumbColor;
        }

        dtk_surface_draw_rect(pSurface, thumbRect, thumbColor);
    } else {
        // The thumb is not visible - just draw the track as one quad.
        dtk_surface_draw_rect(pSurface, dtk_control_get_local_rect(DTK_CONTROL(pScrollbar)), pScrollbar->trackColor);
    }
}



dtk_result dtk_scrollbar_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_scrollbar_orientation orientation, dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL || orientation == dtk_scrollbar_orientation_none) return DTK_INVALID_ARGS;
    dtk_zero_object(pScrollbar);

    dtk_result result = dtk_control_init(pTK, pParent, DTK_CONTROL_TYPE_SCROLLBAR, (onEvent != NULL) ? onEvent : dtk_scrollbar_default_event_handler, &pScrollbar->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    pScrollbar->orientation       = orientation;
    pScrollbar->rangeMin          = 0;
    pScrollbar->rangeMax          = 0;
    pScrollbar->pageSize          = 0;
    pScrollbar->scrollPos         = 0;
    pScrollbar->autoHideThumb     = DTK_TRUE;
    pScrollbar->mouseWheelScale   = 1;
    pScrollbar->trackColor        = dtk_rgb(80, 80, 80);
    pScrollbar->thumbColor        = dtk_rgb(112, 112, 112);
    pScrollbar->thumbColorHovered = dtk_rgb(144, 144, 144);
    pScrollbar->thumbColorPressed = dtk_rgb(180, 180, 180);
    pScrollbar->onScroll          = NULL;

    pScrollbar->thumbSize         = DTK_SCROLLBAR_MIN_THUMB_SIZE;
    pScrollbar->thumbPos          = 0;
    pScrollbar->thumbPadding      = 2;
    pScrollbar->thumbHovered      = DTK_FALSE;
    pScrollbar->thumbPressed      = DTK_FALSE;
    pScrollbar->thumbClickPosX    = 0;
    pScrollbar->thumbClickPosY    = 0;

    return DTK_SUCCESS;
}

dtk_result dtk_scrollbar_uninit(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) return DTK_INVALID_ARGS;

    dtk_control_uninit(DTK_CONTROL(pScrollbar));
    return DTK_SUCCESS;
}


dtk_bool32 dtk_scrollbar_default_event_handler(dtk_event* pEvent)
{
    dtk_scrollbar* pScrollbar = DTK_SCROLLBAR(pEvent->pControl);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dtk_scrollbar__on_paint(pScrollbar, pEvent->paint.rect, pEvent->paint.pSurface);
        } break;

        case DTK_EVENT_SIZE:
        {
            dtk_scrollbar__on_size(pScrollbar, pEvent->size.width, pEvent->size.height);
        } break;

        case DTK_EVENT_MOUSE_LEAVE:
        {
            dtk_scrollbar__on_mouse_leave(pScrollbar);
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            dtk_scrollbar__on_mouse_move(pScrollbar, pEvent->mouseMove.x, pEvent->mouseMove.y, pEvent->mouseMove.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        {
            dtk_scrollbar__on_mouse_button_down(pScrollbar, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_UP:
        {
            dtk_scrollbar__on_mouse_button_up(pScrollbar, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_WHEEL:
        {
            dtk_scrollbar__on_mouse_wheel(pScrollbar, pEvent->mouseWheel.delta, pEvent->mouseWheel.x, pEvent->mouseWheel.y, pEvent->mouseWheel.state);
        } break;

        case DTK_EVENT_REFRESH_LAYOUT:
        {
            dtk_control_scheduled_redraw(pEvent->pControl, dtk_control_get_local_rect(pEvent->pControl));
        } break;

        case DTK_EVENT_SCROLLBAR_SCROLL:
        {
            if (pScrollbar->onScroll) {
                pScrollbar->onScroll(pScrollbar, pEvent->scrollbar.scrollPos);
            }
        } break;

        default: break;
    }

    return dtk_control_default_event_handler(pEvent);
}


dtk_scrollbar_orientation dtk_scrollbar_get_orientation(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return dtk_scrollbar_orientation_none;
    }

    return pScrollbar->orientation;
}


void dtk_scrollbar_set_range(dtk_scrollbar* pScrollbar, dtk_int32 rangeMin, dtk_int32 rangeMax)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->rangeMin = rangeMin;
    pScrollbar->rangeMax = rangeMax;


    // Make sure the scroll position is still valid.
    dtk_scrollbar_scroll_to(pScrollbar, dtk_scrollbar_get_scroll_position(pScrollbar));

    // The thumb may have changed, so refresh it.
    dtk_scrollbar_refresh_thumb(pScrollbar);
}

void dtk_scrollbar_get_range(dtk_scrollbar* pScrollbar, dtk_int32* pRangeMinOut, dtk_int32* pRangeMaxOut)
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


void dtk_scrollbar_set_page_size(dtk_scrollbar* pScrollbar, dtk_int32 pageSize)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->pageSize = pageSize;


    // Make sure the scroll position is still valid.
    dtk_scrollbar_scroll_to(pScrollbar, dtk_scrollbar_get_scroll_position(pScrollbar));

    // The thumb may have changed, so refresh it.
    dtk_scrollbar_refresh_thumb(pScrollbar);
}

dtk_int32 dtk_scrollbar_get_page_size(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return 0;
    }

    return pScrollbar->pageSize;
}


void dtk_scrollbar_set_range_and_page_size(dtk_scrollbar* pScrollbar, dtk_int32 rangeMin, dtk_int32 rangeMax, dtk_int32 pageSize)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->rangeMin = rangeMin;
    pScrollbar->rangeMax = rangeMax;
    pScrollbar->pageSize = pageSize;


    // Make sure the scroll position is still valid.
    dtk_scrollbar_scroll_to(pScrollbar, dtk_scrollbar_get_scroll_position(pScrollbar));

    // The thumb may have changed, so refresh it.
    dtk_scrollbar_refresh_thumb(pScrollbar);
}


void dtk_scrollbar_set_scroll_position(dtk_scrollbar* pScrollbar, dtk_int32 position)
{
    if (pScrollbar == NULL) {
        return;
    }

    dtk_int32 newScrollPos = dtk_clamp(position, pScrollbar->rangeMin, dtk_max(0, pScrollbar->rangeMax - pScrollbar->pageSize + 1));
    if (newScrollPos != pScrollbar->scrollPos) {
        pScrollbar->scrollPos = newScrollPos;
        dtk_scrollbar_refresh_thumb(pScrollbar);
    }
}

dtk_int32 dtk_scrollbar_get_scroll_position(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return 0;
    }

    return pScrollbar->scrollPos;
}


void dtk_scrollbar_scroll(dtk_scrollbar* pScrollbar, dtk_int32 offset)
{
    if (pScrollbar == NULL) {
        return;
    }

    dtk_scrollbar_scroll_to(pScrollbar, pScrollbar->scrollPos + offset);
}

void dtk_scrollbar_scroll_to(dtk_scrollbar* pScrollbar, dtk_int32 newScrollPos)
{
    if (pScrollbar == NULL) {
        return;
    }

    dtk_int32 oldScrollPos = pScrollbar->scrollPos;
    dtk_scrollbar_set_scroll_position(pScrollbar, newScrollPos);

    if (oldScrollPos != pScrollbar->scrollPos) {
        if (pScrollbar->onScroll) {
            pScrollbar->onScroll(pScrollbar, pScrollbar->scrollPos);
        }

        dtk_event e = dtk_event_init(DTK_CONTROL(pScrollbar)->pTK, DTK_EVENT_SCROLLBAR_SCROLL, DTK_CONTROL(pScrollbar));
        e.scrollbar.scrollPos = pScrollbar->scrollPos;
        dtk_post_local_event(e.pTK, &e);
    }
}


void dtk_scrollbar_enable_thumb_auto_hide(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return;
    }

    if (pScrollbar->autoHideThumb != DTK_TRUE) {
        pScrollbar->autoHideThumb = DTK_TRUE;
        dtk_scrollbar_refresh_thumb(pScrollbar);
    }
}

void dtk_scrollbar_disable_thumb_auto_hide(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return;
    }

    if (pScrollbar->autoHideThumb != DTK_FALSE) {
        pScrollbar->autoHideThumb = DTK_FALSE;
        dtk_scrollbar_refresh_thumb(pScrollbar);
    }
}

dtk_bool32 dtk_scrollbar_is_thumb_auto_hide_enabled(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return DTK_FALSE;
    }

    return pScrollbar->autoHideThumb;
}

dtk_bool32 dtk_scrollbar_is_thumb_visible(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return DTK_FALSE;
    }

    // Always visible if auto-hiding is disabled.
    if (!pScrollbar->autoHideThumb) {
        return DTK_TRUE;
    }

    return pScrollbar->pageSize < (pScrollbar->rangeMax - pScrollbar->rangeMin + 1) && pScrollbar->pageSize > 0;
}


void dtk_scrollbar_set_mouse_wheel_scale(dtk_scrollbar* pScrollbar, dtk_int32 scale)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->mouseWheelScale = scale;
}

dtk_int32 dtk_scrollbar_get_mouse_wheel_scale(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return 1;
    }

    return pScrollbar->mouseWheelScale;
}


void dtk_scrollbar_set_track_color(dtk_scrollbar* pScrollbar, dtk_color color)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->trackColor = color;
}

void dtk_scrollbar_set_default_thumb_color(dtk_scrollbar* pScrollbar, dtk_color color)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->thumbColor = color;
}

void dtk_scrollbar_set_hovered_thumb_color(dtk_scrollbar* pScrollbar, dtk_color color)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->thumbColorHovered = color;
}

void dtk_scrollbar_set_pressed_thumb_color(dtk_scrollbar* pScrollbar, dtk_color color)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->thumbColorPressed = color;
}


void dtk_scrollbar_set_on_scroll(dtk_scrollbar* pScrollbar, dtk_scrollbar_on_scroll_proc onScroll)
{
    if (pScrollbar == NULL) {
        return;
    }

    pScrollbar->onScroll = onScroll;
}

dtk_scrollbar_on_scroll_proc dtk_scrollbar_get_on_scroll(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return NULL;
    }

    return pScrollbar->onScroll;
}


dtk_rect dtk_scrollbar_get_thumb_rect(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) {
        return dtk_rect_init(0, 0, 0, 0);
    }

    float scale = dtk_control_get_scaling_factor(DTK_CONTROL(pScrollbar));
    dtk_int32 thumbPadding = (dtk_int32)(pScrollbar->thumbPadding * scale);

    dtk_rect rect = {0, 0, 0, 0};
    rect.left = thumbPadding;
    rect.top  = thumbPadding;

    if (pScrollbar->orientation == dtk_scrollbar_orientation_vertical) {
        // Vertical.
        rect.left   = thumbPadding;
        rect.right  = dtk_control_get_width(DTK_CONTROL(pScrollbar)) - thumbPadding;
        rect.top    = thumbPadding + pScrollbar->thumbPos;
        rect.bottom = rect.top + pScrollbar->thumbSize;
    } else {
        // Horizontal.
        rect.left   = thumbPadding + pScrollbar->thumbPos;
        rect.right  = rect.left + pScrollbar->thumbSize;
        rect.top    = thumbPadding;
        rect.bottom = dtk_control_get_height(DTK_CONTROL(pScrollbar)) - thumbPadding;
    }

    return rect;
}



static void dtk_scrollbar_refresh_thumb(dtk_scrollbar* pScrollbar)
{
    dtk_assert(pScrollbar != NULL);

    dtk_rect oldThumbRect = dtk_scrollbar_get_thumb_rect(pScrollbar);

    pScrollbar->thumbSize = dtk_scrollbar_calculate_thumb_size(pScrollbar);
    pScrollbar->thumbPos  = dtk_scrollbar_calculate_thumb_position(pScrollbar);

    dtk_rect newThumbRect = dtk_scrollbar_get_thumb_rect(pScrollbar);
    if (!dtk_rect_equal(oldThumbRect, newThumbRect)) {
        dtk_control_scheduled_redraw(DTK_CONTROL(pScrollbar), dtk_rect_union(oldThumbRect, newThumbRect));
    }
}

static dtk_int32 dtk_scrollbar_calculate_thumb_size(dtk_scrollbar* pScrollbar)
{
    dtk_assert(pScrollbar != NULL);

    dtk_int32 trackSize = dtk_scrollbar_get_track_size(pScrollbar);
    dtk_int32 range = pScrollbar->rangeMax - pScrollbar->rangeMin + 1;

    dtk_int32 thumbSize = DTK_SCROLLBAR_MIN_THUMB_SIZE;
    if (range > 0) {
        thumbSize = (dtk_int32)((trackSize / (float)range) * pScrollbar->pageSize);
        thumbSize = dtk_clamp(thumbSize, DTK_SCROLLBAR_MIN_THUMB_SIZE, trackSize);
    }

    return thumbSize;
}

static dtk_int32 dtk_scrollbar_calculate_thumb_position(dtk_scrollbar* pScrollbar)
{
    dtk_assert(pScrollbar != NULL);

    dtk_int32 trackSize = dtk_scrollbar_get_track_size(pScrollbar);
    dtk_int32 thumbSize = dtk_scrollbar_calculate_thumb_size(pScrollbar);
    dtk_int32 range = pScrollbar->rangeMax - pScrollbar->rangeMin + 1;

    dtk_int32 thumbPos = 0;
    if (range > pScrollbar->pageSize) {
        thumbPos = (dtk_int32)((trackSize / (float)range) * pScrollbar->scrollPos);
        thumbPos = dtk_clamp(thumbPos, 0, trackSize - thumbSize);
    }

    return thumbPos;
}

static dtk_int32 dtk_scrollbar_get_track_size(dtk_scrollbar* pScrollbar)
{
    dtk_assert(pScrollbar != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pScrollbar));

    if (pScrollbar->orientation == dtk_scrollbar_orientation_vertical) {
        return dtk_control_get_height(DTK_CONTROL(pScrollbar)) - ((dtk_int32)(pScrollbar->thumbPadding*uiScale) * 2);
    } else {
        return dtk_control_get_width(DTK_CONTROL(pScrollbar))  - ((dtk_int32)(pScrollbar->thumbPadding*uiScale) * 2);
    }
}

static void dtk_scrollbar_make_relative_to_thumb(dtk_scrollbar* pScrollbar, dtk_int32* pPosX, dtk_int32* pPosY)
{
    dtk_rect thumbRect = dtk_scrollbar_get_thumb_rect(pScrollbar);

    if (pPosX != NULL) *pPosX -= thumbRect.left;
    if (pPosY != NULL) *pPosY -= thumbRect.top;
}

static dtk_int32 dtk_scrollbar_calculate_scroll_pos_from_thumb_pos(dtk_scrollbar* pScrollbar, dtk_int32 thumbPos)
{
    dtk_assert(pScrollbar != NULL);

    dtk_int32 trackSize = dtk_scrollbar_get_track_size(pScrollbar);
    dtk_int32 range     = pScrollbar->rangeMax - pScrollbar->rangeMin + 1;

    return (dtk_int32)(thumbPos / (trackSize / (float)range));
}
// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    dtk_int32 left;
    dtk_int32 top;
    dtk_int32 right;
    dtk_int32 bottom;
} dtk_rect;

DTK_INLINE dtk_rect dtk_rect_init(dtk_int32 left, dtk_int32 top, dtk_int32 right, dtk_int32 bottom)
{
    dtk_rect rect;
    rect.left   = left;
    rect.top    = top;
    rect.right  = right;
    rect.bottom = bottom;
    return rect;
}

DTK_INLINE dtk_rect dtk_rect_inside_out()
{
    return dtk_rect_init(
        INT32_MAX,
        INT32_MAX,
        INT32_MIN,
        INT32_MIN
    );
}

DTK_INLINE dtk_bool32 dtk_rect_has_volume(dtk_rect rect)
{
    return rect.left < rect.right && rect.top < rect.bottom;
}

DTK_INLINE dtk_rect dtk_rect_grow(dtk_rect rect, dtk_int32 amount)
{
    return dtk_rect_init(
        rect.left   - amount,
        rect.top    - amount,
        rect.right  + amount,
        rect.bottom + amount);
}

DTK_INLINE dtk_rect dtk_rect_union(dtk_rect rect0, dtk_rect rect1)
{
    return dtk_rect_init(
        (rect0.left   < rect1.left)   ? rect0.left   : rect1.left,
        (rect0.top    < rect1.top)    ? rect0.top    : rect1.top,
        (rect0.right  > rect1.right)  ? rect0.right  : rect1.right,
        (rect0.bottom > rect1.bottom) ? rect0.bottom : rect1.bottom);
}

DTK_INLINE dtk_rect dtk_rect_move_to_center(dtk_rect rect, dtk_rect bounds)
{
    dtk_int32 rectSizeX = rect.right - rect.left;
    dtk_int32 rectSizeY = rect.bottom - rect.top;

    dtk_int32 boundsSizeX = bounds.right - bounds.left;
    dtk_int32 boundsSizeY = bounds.bottom - bounds.top;

    dtk_rect centered;
    centered.left   = bounds.left + (boundsSizeX - rectSizeX)/2;
    centered.top    = bounds.top  + (boundsSizeY - rectSizeY)/2;
    centered.right  = centered.left + rectSizeX;
    centered.bottom = centered.top  + rectSizeY;
    return centered;
}

DTK_INLINE dtk_bool32 dtk_rect_contains_point(dtk_rect rect, dtk_int32 posX, dtk_int32 posY)
{
    if (posX < rect.left || posY < rect.top) {
        return DTK_FALSE;
    }

    if (posX >= rect.right || posY >= rect.bottom) {
        return DTK_FALSE;
    }

    return DTK_TRUE;
}

DTK_INLINE dtk_bool32 dtk_rect_equal(dtk_rect rect0, dtk_rect rect1)
{
    return
        rect0.left   == rect1.left  &&
        rect0.top    == rect1.top   &&
        rect0.right  == rect1.right &&
        rect0.bottom == rect1.bottom;
}
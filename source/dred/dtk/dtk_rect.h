// Copyright (C) 2016 David Reid. See included LICENSE file.

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
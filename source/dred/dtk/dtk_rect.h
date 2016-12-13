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
    dtk_rect rect;
    rect.left   = INT32_MAX;
    rect.top    = INT32_MAX;
    rect.right  = INT32_MIN;
    rect.bottom = INT32_MIN;
    return rect;
}

DTK_INLINE dtk_bool32 dtk_rect_has_volume(dtk_rect rect)
{
    return rect.left < rect.right && rect.top < rect.bottom;
}
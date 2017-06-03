// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct dtk_scrollbar dtk_scrollbar;
typedef void (* dtk_scrollbar_on_scroll_proc)(dtk_scrollbar* pScrollbar, dtk_int32 scrollPos);

typedef enum
{
    dtk_scrollbar_orientation_none,
    dtk_scrollbar_orientation_vertical,
    dtk_scrollbar_orientation_horizontal
} dtk_scrollbar_orientation;

#define DTK_SCROLLBAR(a) ((dtk_scrollbar*)(a))
struct dtk_scrollbar
{
    dtk_control control;
    dtk_scrollbar_orientation orientation;
    dtk_int32 rangeMin;
    dtk_int32 rangeMax;
    dtk_int32 pageSize;
    dtk_int32 scrollPos;
    dtk_bool32 autoHideThumb;
    dtk_int32 mouseWheelScale;
    dtk_color trackColor;
    dtk_color thumbColor;
    dtk_color thumbColorHovered;
    dtk_color thumbColorPressed;
    dtk_scrollbar_on_scroll_proc onScroll;
    dtk_int32 thumbSize;
    dtk_int32 thumbPos;
    dtk_int32 thumbPadding;
    dtk_bool32 thumbHovered;
    dtk_bool32 thumbPressed;
    dtk_int32 thumbClickPosX;
    dtk_int32 thumbClickPosY;
};


// Creates a scrollbar element.
dtk_result dtk_scrollbar_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, dtk_scrollbar_orientation orientation, dtk_scrollbar* pScrollbar);

// Deletes the given scrollbar element.
dtk_result dtk_scrollbar_uninit(dtk_scrollbar* pScrollbar);

// The default event handler for the scrollbar control.
dtk_bool32 dtk_scrollbar_default_event_handler(dtk_event* pEvent);


// Retrieves the orientation of the given scrollbar.
dtk_scrollbar_orientation dtk_scrollbar_get_orientation(dtk_scrollbar* pScrollbar);


// Sets the given scrollbar's range.
void dtk_scrollbar_set_range(dtk_scrollbar* pScrollbar, dtk_int32 rangeMin, dtk_int32 rangeMax);

// Retrieves the given scrollbar's range.
void dtk_scrollbar_get_range(dtk_scrollbar* pScrollbar, dtk_int32* pRangeMinOut, dtk_int32* pRangeMaxOut);


// Sets the page size of the given scrollbar's page.
void dtk_scrollbar_set_page_size(dtk_scrollbar* pScrollbar, dtk_int32 pageSize);

// Retrieves the page size of the given scrollbar's page.
dtk_int32 dtk_scrollbar_get_page_size(dtk_scrollbar* pScrollbar);


// Sets the range and page size.
//
// Use this when both the range and page size need to be updated at the same time.
void dtk_scrollbar_set_range_and_page_size(dtk_scrollbar* pScrollbar, dtk_int32 rangeMin, dtk_int32 rangeMax, dtk_int32 pageSize);


// Explicitly sets the scroll position.
//
// This will move the thumb, but not post the on_scroll event.
//
// The scroll position will be clamped to the current range, minus the page size.
void dtk_scrollbar_set_scroll_position(dtk_scrollbar* pScrollbar, dtk_int32 position);

// Retrieves the scroll position.
dtk_int32 dtk_scrollbar_get_scroll_position(dtk_scrollbar* pScrollbar);


// Scrolls by the given amount.
//
// If the resulting scroll position differs from the old one, the on on_scroll event will be posted.
void dtk_scrollbar_scroll(dtk_scrollbar* pScrollbar, dtk_int32 offset);

// Scrolls to the given position.
//
// This differs from dtk_scrollbar_set_scroll_position in that it will post the on_scroll event.
//
// Note that the actual maximum scrollable position is equal to the maximum range value minus the page size.
void dtk_scrollbar_scroll_to(dtk_scrollbar* pScrollbar, dtk_int32 newScrollPos);


// Enables auto-hiding of the thumb.
void dtk_scrollbar_enable_thumb_auto_hide(dtk_scrollbar* pScrollbar);

// Disables auto-hiding of the thumb.
void dtk_scrollbar_disable_thumb_auto_hide(dtk_scrollbar* pScrollbar);

// Determines whether or not thumb auto-hiding is enabled.
dtk_bool32 dtk_scrollbar_is_thumb_auto_hide_enabled(dtk_scrollbar* pScrollbar);

// Determines whether or not the thumb is visible.
//
// @remarks
//     This is determined by whether or not the thumb is set to auto-hide and the current range and page size.
dtk_bool32 dtk_scrollbar_is_thumb_visible(dtk_scrollbar* pScrollbar);


// Sets the mouse wheel scale.
//
// Set this to a negative value to reverse the direction.
void dtk_scrollbar_set_mouse_wheel_scale(dtk_scrollbar* pScrollbar, dtk_int32 scale);

// Retrieves the mouse wheel scale.
dtk_int32 dtk_scrollbar_get_mouse_wheel_scale(dtk_scrollbar* pScrollbar);


// Sets the color of the track.
void dtk_scrollbar_set_track_color(dtk_scrollbar* pScrollbar, dtk_color color);

// Sets the default color of the thumb.
void dtk_scrollbar_set_default_thumb_color(dtk_scrollbar* pScrollbar, dtk_color color);

// Sets the hovered color of the thumb.
void dtk_scrollbar_set_hovered_thumb_color(dtk_scrollbar* pScrollbar, dtk_color color);

// Sets the pressed color of the thumb.
void dtk_scrollbar_set_pressed_thumb_color(dtk_scrollbar* pScrollbar, dtk_color color);


// Sets the function to call when the given scrollbar is scrolled.
void dtk_scrollbar_set_on_scroll(dtk_scrollbar* pScrollbar, dtk_scrollbar_on_scroll_proc onScroll);

// Retrieves the function call when the given scrollbar is scrolled.
dtk_scrollbar_on_scroll_proc dtk_scrollbar_get_on_scroll(dtk_scrollbar* pScrollbar);


// Calculates the relative rectangle of the given scrollbar's thumb.
dtk_rect dtk_scrollbar_get_thumb_rect(dtk_scrollbar* pScrollbar);


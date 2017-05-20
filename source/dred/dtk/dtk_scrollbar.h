// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef enum
{
    dtk_scrollbar_type_vertical,
    dtk_scrollbar_type_horizontal
} dtk_scrollbar_type;

#define DTK_SCROLLBAR(p) ((dtk_scrollbar*)(p))
struct dtk_scrollbar
{
    dtk_control control;
    dtk_scrollbar_type type;
    dtk_uint32 range;
    dtk_uint32 page;
    dtk_uint32 scrollPos;

    dtk_bool32 isMouseOverThumb : 1;
};

dtk_result dtk_scrollbar_init(dtk_context* pTK, dtk_control* pParent, dtk_scrollbar_type type, dtk_event_proc onEvent, dtk_scrollbar* pScrollbar);
dtk_result dtk_scrollbar_uninit(dtk_scrollbar* pScrollbar);
dtk_bool32 dtk_scrollbar_default_event_handler(dtk_event* pEvent);
dtk_result dtk_scrollbar_set_range(dtk_scrollbar* pScrollbar, dtk_uint32 range);
dtk_result dtk_scrollbar_set_page(dtk_scrollbar* pScrollbar, dtk_uint32 page);
dtk_result dtk_scrollbar_scroll_to(dtk_scrollbar* pScrollbar, dtk_uint32 scrollPos);
dtk_result dtk_scrollbar_scroll(dtk_scrollbar* pScrollbar, dtk_int32 offset);
dtk_rect dtk_scrollbar_get_thumb_rect(dtk_scrollbar* pScrollbar);

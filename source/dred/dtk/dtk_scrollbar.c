// Copyright (C) 2016 David Reid. See included LICENSE file.

dtk_result dtk_scrollbar_init(dtk_context* pTK, dtk_control* pParent, dtk_scrollbar_type type, dtk_event_proc onEvent, dtk_scrollbar* pScrollbar)
{
    if (pTK == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pScrollbar);

    dtk_result result = dtk_control_init(pTK, pParent, DTK_CONTROL_TYPE_SCROLLBAR, onEvent, &pScrollbar->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    pScrollbar->type = type;

    return DTK_SUCCESS;
}

dtk_result dtk_scrollbar_uninit(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) return DTK_INVALID_ARGS;
    return dtk_control_uninit(&pScrollbar->control);
}


dtk_result dtk_scrollbar_set_range(dtk_scrollbar* pScrollbar, dtk_uint32 range)
{
    if (pScrollbar == NULL) return DTK_INVALID_ARGS;

    pScrollbar->range = range;
    return DTK_SUCCESS;
}

dtk_result dtk_scrollbar_set_page(dtk_scrollbar* pScrollbar, dtk_uint32 page)
{
    if (pScrollbar == NULL) return DTK_INVALID_ARGS;

    pScrollbar->page = page;
    return DTK_SUCCESS;
}

dtk_result dtk_scrollbar_scroll_to(dtk_scrollbar* pScrollbar, dtk_uint32 scrollPos)
{
    if (pScrollbar == NULL) return DTK_INVALID_ARGS;

    pScrollbar->scrollPos = dtk_clamp(scrollPos, 0, pScrollbar->range);
    return DTK_SUCCESS;
}

dtk_result dtk_scrollbar_scroll(dtk_scrollbar* pScrollbar, dtk_int32 offset)
{
    if (pScrollbar == NULL) return DTK_INVALID_ARGS;
    return dtk_scrollbar_scroll_to(pScrollbar, pScrollbar->scrollPos + offset);
}

dtk_bool32 dtk_scrollbar_default_event_handler(dtk_event* pEvent)
{
    if (pEvent == NULL) return DTK_FALSE;
    dtk_assert(pEvent->pControl != NULL);
    dtk_assert(pEvent->pControl->type == DTK_CONTROL_TYPE_SCROLLBAR);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
        } break;

        case DTK_EVENT_SCROLLBAR_SCROLL:
        {
        } break;
    }

    return DTK_TRUE;
}
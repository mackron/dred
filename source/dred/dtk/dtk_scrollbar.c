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


dtk_bool32 dtk_scrollbar_default_event_handler(dtk_event* pEvent)
{
    if (pEvent == NULL) return DTK_FALSE;
    dtk_assert(pEvent->pControl != NULL);
    dtk_assert(pEvent->pControl->type == DTK_CONTROL_TYPE_SCROLLBAR);

    dtk_scrollbar* pScrollbar = DTK_SCROLLBAR(pEvent->pControl);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dtk_surface_draw_rect(pEvent->paint.pSurface, dtk_control_get_local_rect(pEvent->pControl), dtk_rgb(64, 64, 64));
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            printf("MOVE %d %d\n", pEvent->mouseMove.x, pEvent->mouseMove.y);
            pScrollbar->isMouseOverThumb = dtk_rect_contains_point(dtk_scrollbar_get_thumb_rect(pScrollbar), pEvent->mouseMove.x, pEvent->mouseMove.y);
        } break;

        case DTK_EVENT_MOUSE_ENTER:
        {
            printf("ENTER\n");
        } break;

        case DTK_EVENT_MOUSE_LEAVE:
        {
            printf("LEAVE\n");
            pScrollbar->isMouseOverThumb = DTK_FALSE;
        } break;

        case DTK_EVENT_SCROLLBAR_SCROLL:
        {
        } break;
    }

    return DTK_TRUE;
}


dtk_result dtk_scrollbar_set_range(dtk_scrollbar* pScrollbar, dtk_uint32 range)
{
    if (pScrollbar == NULL) return DTK_INVALID_ARGS;

    if (pScrollbar->range == range) {
        return DTK_SUCCESS;
    }

    if (pScrollbar->scrollPos > range) {
        dtk_scrollbar_scroll_to(pScrollbar, range);
    }

    pScrollbar->range = range;

    return DTK_SUCCESS;
}

dtk_result dtk_scrollbar_set_page(dtk_scrollbar* pScrollbar, dtk_uint32 page)
{
    if (pScrollbar == NULL) return DTK_INVALID_ARGS;

    if (pScrollbar->page == page) {
        return DTK_SUCCESS;
    }

    pScrollbar->page = page;
    return DTK_SUCCESS;
}

dtk_result dtk_scrollbar_scroll_to(dtk_scrollbar* pScrollbar, dtk_uint32 scrollPos)
{
    if (pScrollbar == NULL) return DTK_INVALID_ARGS;

    if (pScrollbar->scrollPos == scrollPos) {
        return DTK_SUCCESS;
    }

    pScrollbar->scrollPos = dtk_clamp(scrollPos, 0, pScrollbar->range);
    return DTK_SUCCESS;
}

dtk_result dtk_scrollbar_scroll(dtk_scrollbar* pScrollbar, dtk_int32 offset)
{
    if (pScrollbar == NULL) return DTK_INVALID_ARGS;
    return dtk_scrollbar_scroll_to(pScrollbar, pScrollbar->scrollPos + offset);
}

dtk_rect dtk_scrollbar_get_thumb_rect(dtk_scrollbar* pScrollbar)
{
    if (pScrollbar == NULL) return dtk_rect_init(0, 0, 0, 0);
    
    // TODO: Implement me.
    return dtk_rect_init(0, 0, 0, 0);
}

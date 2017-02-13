// Copyright (C) 2016 David Reid. See included LICENSE file.

dtk_result dtk_tabbar_init(dtk_context* pTK, dtk_control* pParent, dtk_tabbar_orientation orientation, dtk_event_proc onEvent, dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pTabBar);

    dtk_result result = dtk_control_init(pTK, pParent, DTK_CONTROL_TYPE_TABBAR, onEvent, &pTabBar->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    pTabBar->orientation = orientation;

    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_uninit(dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    return dtk_control_uninit(&pTabBar->control);
}


dtk_bool32 dtk_tabbar_default_event_handler(dtk_event* pEvent)
{
    if (pEvent == NULL) return DTK_FALSE;

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            
        } break;

        case DTK_EVENT_MOUSE_LEAVE:
        {
            // TODO: Clear the hovered tab.
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            // TODO: Perform a hit test and redraw if applicable.
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        {
        } break;

        default: break;
    }

    return DTK_TRUE;
}


dtk_result dtk_tabbar_set_font(dtk_tabbar* pTabBar, dtk_font* pFont)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    pTabBar->pFont = pFont;

    if (pTabBar->isAutoResizeEnabled) {
        //dtk_tabbar_auto_resize_if_enabled(pTabBar);
    }
    
    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_set_text_fg_color(dtk_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    pTabBar->textFGColor = color;
    
    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_set_text_bg_color(dtk_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    pTabBar->textBGColor = color;
    
    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}



dtk_result dtk_tabbar_append_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage, dtk_tabbar_tab** ppTabOut)
{
	// TODO: Implement me.
	return DTK_SUCCESS;
}

dtk_result dtk_tabbar_prepend_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage, dtk_tabbar_tab** ppTabOut)
{
	// TODO: Implement me.
	return DTK_SUCCESS;
}

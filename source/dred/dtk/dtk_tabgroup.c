// Copyright (C) 2017 David Reid. See included LICENSE file.

void dtk_tabgroup__calculate_tabbar_position(dtk_tabgroup* pTabGroup, dtk_int32* pRelativePosX, dtk_int32* pRelativePosY)
{
    dtk_assert(pTabGroup != NULL);
    dtk_assert(pRelativePosX != NULL);
    dtk_assert(pRelativePosY != NULL);

    // Depends on the edge.
    switch (pTabGroup->tabbarEdge)
    {
        case dtk_tabgroup_tabbar_edge_top:
        case dtk_tabgroup_tabbar_edge_left:
        {
            *pRelativePosX = 0;
            *pRelativePosY = 0;
        } break;

        case dtk_tabgroup_tabbar_edge_right:
        {
            *pRelativePosX = dtk_control_get_width(DTK_CONTROL(pTabGroup)) - dtk_control_get_width(DTK_CONTROL(&pTabGroup->tabbar));
            *pRelativePosY = 0;
        } break;

        case dtk_tabgroup_tabbar_edge_bottom:
        {
            *pRelativePosX = 0;
            *pRelativePosY = dtk_control_get_height(DTK_CONTROL(pTabGroup)) - dtk_control_get_height(DTK_CONTROL(&pTabGroup->tabbar));
        } break;
    }
}

dtk_rect dtk_tabgroup__calculate_container_rect(dtk_tabgroup* pTabGroup)
{
    dtk_assert(pTabGroup != NULL);

    // Depends on the edge.
    dtk_rect tabbarRect = dtk_control_get_relative_rect(DTK_CONTROL(&pTabGroup->tabbar));
    dtk_rect containerRect = dtk_rect_init(0, 0, 0, 0);

    dtk_int32 tabgroupWidth;
    dtk_int32 tabgroupHeight;
    dtk_control_get_size(DTK_CONTROL(pTabGroup), &tabgroupWidth, &tabgroupHeight);

    switch (pTabGroup->tabbarEdge)
    {
        case dtk_tabgroup_tabbar_edge_top:
        {
            containerRect.left = 0;
            containerRect.top = tabbarRect.bottom;
            containerRect.right = tabgroupWidth;
            containerRect.bottom = tabgroupHeight;
        } break;

        case dtk_tabgroup_tabbar_edge_left:
        {
            containerRect.left = tabbarRect.right;
            containerRect.top = 0;
            containerRect.right = tabgroupWidth;
            containerRect.bottom = tabgroupHeight;
        } break;

        case dtk_tabgroup_tabbar_edge_right:
        {
            containerRect.left = 0;
            containerRect.top = 0;
            containerRect.right = tabbarRect.left;
            containerRect.bottom = tabgroupHeight;
        } break;

        case dtk_tabgroup_tabbar_edge_bottom:
        {
            containerRect.left = 0;
            containerRect.top = 0;
            containerRect.right = tabgroupWidth;
            containerRect.bottom = tabbarRect.top;
        } break;
    }

    return containerRect;
}

void dtk_tabgroup__refresh_container_layout(dtk_tabgroup* pTabGroup)
{
    dtk_rect containerRect = dtk_tabgroup__calculate_container_rect(pTabGroup);
    dtk_control_set_size(&pTabGroup->container, containerRect.right-containerRect.left, containerRect.bottom-containerRect.top);
    dtk_control_set_relative_position(&pTabGroup->container, containerRect.left, containerRect.top);
}

void dtk_tabgroup__refresh_tabbar_layout(dtk_tabgroup* pTabGroup)
{
    dtk_uint32 tabbarWidth = 0;
    dtk_uint32 tabbarHeight = 0;

    dtk_int32 tabgroupWidth;
    dtk_int32 tabgroupHeight;
    dtk_control_get_size(DTK_CONTROL(pTabGroup), &tabgroupWidth, &tabgroupHeight);

    switch (pTabGroup->tabbarEdge)
    {
        case dtk_tabgroup_tabbar_edge_top:
        {
            tabbarWidth = tabgroupWidth;
            tabbarHeight = 32;
        } break;

        case dtk_tabgroup_tabbar_edge_left:
        {
            tabbarWidth = 32;
            tabbarHeight = tabgroupHeight;
        } break;

        case dtk_tabgroup_tabbar_edge_right:
        {
            tabbarWidth = 32;
            tabbarHeight = tabgroupHeight;
        } break;

        case dtk_tabgroup_tabbar_edge_bottom:
        {
            tabbarWidth = tabgroupWidth;
            tabbarHeight = 32;
        } break;
    }

    dtk_control_set_size(DTK_CONTROL(&pTabGroup->tabbar), tabbarWidth, tabbarHeight);


    // The position depends on the size, so set the position after setting the size.
    dtk_int32 tabbarPosX;
    dtk_int32 tabbarPosY;
    dtk_tabgroup__calculate_tabbar_position(pTabGroup, &tabbarPosX, &tabbarPosY);
    dtk_control_set_relative_position(DTK_CONTROL(&pTabGroup->tabbar), tabbarPosX, tabbarPosY);
}

void dtk_tabgroup__refresh_layout(dtk_tabgroup* pTabGroup)
{
    dtk_tabgroup__refresh_tabbar_layout(pTabGroup);
    dtk_tabgroup__refresh_container_layout(pTabGroup);
}


dtk_bool32 dtk_tabgroup_tabbar_event_handler(dtk_event* pEvent)
{
    dtk_tabgroup* pTabGroup = DTK_TABGROUP(pEvent->pControl->pParent);

    switch (pEvent->type)
    {
        case DTK_EVENT_TABBAR_CHANGE_TAB:
        {
            // Route the equivalent event to the tab group control. The application should never need to know any of the details about
            // the tab bar.
            dtk_event e = *pEvent;
            e.pControl = pEvent->pControl->pParent;
            e.type = DTK_EVENT_TABGROUP_CHANGE_TAB;
            e.tabgroup.oldTabIndex = e.tabbar.oldTabIndex;
            e.tabgroup.newTabIndex = e.tabbar.newTabIndex;
            if (!dtk_handle_local_event(pEvent->pTK, &e)) {
                return DTK_FALSE;
            }
        } break;

        case DTK_EVENT_SIZE:
        {
            // The position and/or size of the container will have changed in response to the tabbar changing size.
            dtk_control_refresh_layout(DTK_CONTROL(pTabGroup));
        } break;

        case DTK_EVENT_MOVE:
        {
            // The postiion and/or size of the container will have changed.
            dtk_control_refresh_layout(DTK_CONTROL(pTabGroup));
        } break;

        case DTK_EVENT_REFRESH_LAYOUT:
        {
            dtk_tabgroup__refresh_container_layout(pTabGroup);
        } break;

        default: break;
    }

    return dtk_tabbar_default_event_handler(pEvent);
}

dtk_bool32 dtk_tabgroup_container_event_handler(dtk_event* pEvent)
{
    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dtk_surface_draw_rect(pEvent->paint.pSurface, dtk_control_get_local_rect(pEvent->pControl), dtk_rgb(255, 220, 220));
        } break;
    }

    return dtk_control_default_event_handler(pEvent);
}

dtk_result dtk_tabgroup_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, dtk_tabgroup_tabbar_edge tabbarEdge, dtk_tabbar_flow tabbarFlow, dtk_tabbar_text_direction tabbarTextDirection, dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pTabGroup);
    pTabGroup->tabbarEdge = tabbarEdge;

    dtk_result result = dtk_control_init(pTK, DTK_CONTROL_TYPE_TABGROUP, (onEvent != NULL) ? onEvent : dtk_tabgroup_default_event_handler, pParent, &pTabGroup->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    result = dtk_tabbar_init(pTK, dtk_tabgroup_tabbar_event_handler, DTK_CONTROL(pTabGroup), tabbarFlow, tabbarTextDirection, &pTabGroup->tabbar);
    if (result != DTK_SUCCESS) {
        dtk_control_uninit(&pTabGroup->control);
        return result;
    }

    result = dtk_control_init(pTK, DTK_CONTROL_TYPE_EMPTY, dtk_tabgroup_container_event_handler, pParent, &pTabGroup->container);
    if (result != DTK_SUCCESS) {
        dtk_tabbar_uninit(&pTabGroup->tabbar);
        dtk_control_uninit(&pTabGroup->control);
        return result;
    }


    dtk_tabgroup__refresh_tabbar_layout(pTabGroup);
    dtk_tabgroup__refresh_container_layout(pTabGroup);

    return DTK_SUCCESS;
}

dtk_result dtk_tabgroup_uninit(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) return DTK_INVALID_ARGS;

    dtk_control_uninit(&pTabGroup->container);
    dtk_tabbar_uninit(&pTabGroup->tabbar);
    dtk_control_uninit(&pTabGroup->control);

    return DTK_SUCCESS;
}


dtk_bool32 dtk_tabgroup_default_event_handler(dtk_event* pEvent)
{
    if (pEvent == NULL) return DTK_FALSE;

    dtk_tabgroup* pTabGroup = DTK_TABGROUP(pEvent->pControl);
    switch (pEvent->type)
    {
        case DTK_EVENT_SIZE:
        {
            dtk_control_refresh_layout(pEvent->pControl);
        } break;

        case DTK_EVENT_REFRESH_LAYOUT:
        {
            dtk_tabgroup__refresh_layout(pTabGroup);
        } break;
    }

    return dtk_control_default_event_handler(pEvent);
}

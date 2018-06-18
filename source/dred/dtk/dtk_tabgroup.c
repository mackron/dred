// Copyright (C) 2018 David Reid. See included LICENSE file.

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
    dtk_rect tabbarRect = dtk_rect_init(0, 0, 0, 0);
    if (dtk_tabgroup_is_showing_tabbar(pTabGroup)) {
        tabbarRect = dtk_control_get_relative_rect(DTK_CONTROL(&pTabGroup->tabbar));
    }
    
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
        case dtk_tabgroup_tabbar_edge_bottom:
        {
            tabbarWidth = tabgroupWidth;
        } break;

        case dtk_tabgroup_tabbar_edge_left:
        case dtk_tabgroup_tabbar_edge_right:
        {
            tabbarHeight = tabgroupHeight;
        } break;
    }

    // The position depends on the size, so set the position after setting the size.
    dtk_int32 tabbarPosX;
    dtk_int32 tabbarPosY;
    dtk_tabgroup__calculate_tabbar_position(pTabGroup, &tabbarPosX, &tabbarPosY);
    dtk_control_set_relative_position(DTK_CONTROL(&pTabGroup->tabbar), tabbarPosX, tabbarPosY);
}

void dtk_tabgroup__refresh_layout(dtk_tabgroup* pTabGroup)
{
    // We want to make sure the tab bar and the container are laid out properly before notifying the application.
    dtk_tabgroup__refresh_tabbar_layout(pTabGroup);
    dtk_tabgroup__refresh_container_layout(pTabGroup);
    
    // Now notify the application.
    dtk_control_refresh_layout(DTK_CONTROL(pTabGroup));
}


dtk_bool32 dtk_tabgroup_tabbar_event_handler(dtk_event* pEvent)
{
    dtk_tabgroup* pTabGroup = DTK_TABGROUP(pEvent->pControl->pParent);

    switch (pEvent->type)
    {
        // Tab bar events should be routed to the tab group. The application should never need to know any of the details about
        // the tab bar.
        case DTK_EVENT_TABBAR_MOUSE_BUTTON_DOWN_TAB:
        {
            dtk_event e = *pEvent;
            e.pControl = pEvent->pControl->pParent;
            e.type = DTK_EVENT_TABGROUP_MOUSE_BUTTON_DOWN_TAB;
            e.tabgroup.tabIndex = e.tabbar.tabIndex;
            e.tabgroup.mouseButton.x = e.tabbar.mouseButton.x;
            e.tabgroup.mouseButton.y = e.tabbar.mouseButton.y;
            e.tabgroup.mouseButton.button = e.tabbar.mouseButton.button;
            e.tabgroup.mouseButton.state = e.tabbar.mouseButton.state;
            if (!dtk_handle_local_event(&e)) {
                return DTK_FALSE;
            }
        } break;

        case DTK_EVENT_TABBAR_MOUSE_BUTTON_UP_TAB:
        {
            dtk_event e = *pEvent;
            e.pControl = pEvent->pControl->pParent;
            e.type = DTK_EVENT_TABGROUP_MOUSE_BUTTON_UP_TAB;
            e.tabgroup.tabIndex = e.tabbar.tabIndex;
            e.tabgroup.mouseButton.x = e.tabbar.mouseButton.x;
            e.tabgroup.mouseButton.y = e.tabbar.mouseButton.y;
            e.tabgroup.mouseButton.button = e.tabbar.mouseButton.button;
            e.tabgroup.mouseButton.state = e.tabbar.mouseButton.state;
            if (!dtk_handle_local_event(&e)) {
                return DTK_FALSE;
            }
        } break;

        case DTK_EVENT_TABBAR_MOUSE_BUTTON_DBLCLICK_TAB:
        {
            dtk_event e = *pEvent;
            e.pControl = pEvent->pControl->pParent;
            e.type = DTK_EVENT_TABGROUP_MOUSE_BUTTON_DBLCLICK_TAB;
            e.tabgroup.tabIndex = e.tabbar.tabIndex;
            e.tabgroup.mouseButton.x = e.tabbar.mouseButton.x;
            e.tabgroup.mouseButton.y = e.tabbar.mouseButton.y;
            e.tabgroup.mouseButton.button = e.tabbar.mouseButton.button;
            e.tabgroup.mouseButton.state = e.tabbar.mouseButton.state;
            if (!dtk_handle_local_event(&e)) {
                return DTK_FALSE;
            }
        } break;

        case DTK_EVENT_TABBAR_CHANGE_TAB:
        {
            dtk_event e = *pEvent;
            e.pControl = pEvent->pControl->pParent;
            e.type = DTK_EVENT_TABGROUP_CHANGE_TAB;
            e.tabgroup.oldTabIndex = e.tabbar.oldTabIndex;
            e.tabgroup.newTabIndex = e.tabbar.newTabIndex;
            if (!dtk_handle_local_event(&e)) {
                return DTK_FALSE;
            }
        } break;

        case DTK_EVENT_TABBAR_CLOSE_TAB:
        {
            dtk_event e = *pEvent;
            e.pControl = pEvent->pControl->pParent;
            e.type = DTK_EVENT_TABGROUP_CLOSE_TAB;
            e.tabgroup.tabIndex = e.tabbar.tabIndex;
            if (!dtk_handle_local_event(&e)) {
                return DTK_FALSE;
            }
        } break;



        case DTK_EVENT_SIZE:
        {
            dtk_tabgroup__refresh_layout(pTabGroup);
        } break;

        case DTK_EVENT_MOVE:
        {
            dtk_tabgroup__refresh_layout(pTabGroup);
        } break;

        case DTK_EVENT_REFRESH_LAYOUT:
        {
            dtk_tabgroup__refresh_container_layout(pTabGroup);

            // Now notify the application.
            dtk_control_refresh_layout(DTK_CONTROL(pTabGroup));
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
            //dtk_surface_draw_rect(pEvent->paint.pSurface, dtk_control_get_local_rect(pEvent->pControl), dtk_rgb(255, 220, 220));
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

    dtk_tabbar_enable_auto_resize(&pTabGroup->tabbar);
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
        case DTK_EVENT_TABGROUP_CHANGE_TAB:
        {
            // Hide the old tab page.
            if (pEvent->tabgroup.oldTabIndex != -1) {
                dtk_control_hide(dtk_tabgroup_get_tab_page(pTabGroup, pEvent->tabgroup.oldTabIndex));
            }

            // Show the new tab page.
            if (pEvent->tabgroup.newTabIndex != -1) {
                dtk_control_show(dtk_tabgroup_get_tab_page(pTabGroup, pEvent->tabgroup.newTabIndex));
            }
        } break;

        case DTK_EVENT_SIZE:
        {
            // We want to make sure the tab bar and the container are laid out properly before notifying the application.
            dtk_tabgroup__refresh_layout(pTabGroup);
        } break;

        case DTK_EVENT_REFRESH_LAYOUT:
        {
        } break;
    }

    return dtk_control_default_event_handler(pEvent);
}


dtk_control* dtk_tabgroup_get_tab_page_container(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return NULL;
    }

    return &pTabGroup->container;
}


dtk_result dtk_tabgroup_append_tab(dtk_tabgroup* pTabGroup, const char* text, dtk_control* pTabPage, dtk_uint32* pTabIndexOut)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_append_tab(&pTabGroup->tabbar, text, pTabPage, pTabIndexOut);
}

dtk_result dtk_tabgroup_prepend_tab(dtk_tabgroup* pTabGroup, const char* text, dtk_control* pTabPage, dtk_uint32* pTabIndexOut)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_prepend_tab(&pTabGroup->tabbar, text, pTabPage, pTabIndexOut);
}

dtk_result dtk_tabgroup_remove_tab_by_index(dtk_tabgroup* pTabGroup, dtk_uint32 tabIndex)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_remove_tab_by_index(&pTabGroup->tabbar, tabIndex);
}

dtk_uint32 dtk_tabgroup_get_tab_count(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return 0;
    }

    return dtk_tabbar_get_tab_count(&pTabGroup->tabbar);
}



dtk_control* dtk_tabgroup_get_tab_page(dtk_tabgroup* pTabGroup, dtk_uint32 tabIndex)
{
    if (pTabGroup == NULL) {
        return 0;
    }

    return dtk_tabbar_get_tab_page(&pTabGroup->tabbar, tabIndex);
}

dtk_uint32 dtk_tabgroup_get_active_tab_index(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return (dtk_uint32)-1;
    }

    return dtk_tabbar_get_active_tab_index(&pTabGroup->tabbar);
}

dtk_result dtk_tabgroup_activate_tab(dtk_tabgroup* pTabGroup, dtk_uint32 tabIndex)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_activate_tab(&pTabGroup->tabbar, tabIndex);
}

dtk_result dtk_tabgroup_activate_next_tab(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_activate_next_tab(&pTabGroup->tabbar);
}

dtk_result dtk_tabgroup_activate_prev_tab(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_activate_prev_tab(&pTabGroup->tabbar);
}


dtk_result dtk_tabgroup_set_tab_text(dtk_tabgroup* pTabGroup, dtk_uint32 tabIndex, const char* text)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_tab_text(&pTabGroup->tabbar, tabIndex, text);
}

dtk_result dtk_tabgroup_set_tab_tooltip(dtk_tabgroup* pTabGroup, dtk_uint32 tabIndex, const char* pTooltipText)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_tab_tooltip(&pTabGroup->tabbar, tabIndex, pTooltipText);
}


dtk_result dtk_tabgroup_get_container_size(dtk_tabgroup* pTabGroup, dtk_int32* pWidth, dtk_int32* pHeight)
{
    return dtk_control_get_size(&pTabGroup->container, pWidth, pHeight);
}


dtk_result dtk_tabgroup_show_tabbar(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    if (dtk_tabgroup_is_showing_tabbar(pTabGroup)) {
        return DTK_SUCCESS; // Already showing.
    }

    dtk_result result = dtk_control_show(DTK_CONTROL(&pTabGroup->tabbar));
    if (result != DTK_SUCCESS) {
        return result;
    }

    dtk_tabgroup__refresh_layout(pTabGroup);  // The size of the body has changed which requires a layout refresh.
    return result;
}

dtk_result dtk_tabgroup_hide_tabbar(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    if (!dtk_tabgroup_is_showing_tabbar(pTabGroup)) {
        return DTK_SUCCESS; // Already hidden.
    }

    dtk_result result = dtk_control_hide(DTK_CONTROL(&pTabGroup->tabbar));
    if (result != DTK_SUCCESS) {
        return result;
    }

    dtk_tabgroup__refresh_layout(pTabGroup);  // The size of the body has changed which requires a layout refresh.
    return result;
}

dtk_bool32 dtk_tabgroup_is_showing_tabbar(const dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return DTK_FALSE;
    }

    return dtk_control_is_visible(DTK_CONTROL(&pTabGroup->tabbar));
}


dtk_result dtk_tabgroup_enable_close_on_middle_click(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return DTK_FALSE;
    }

    return dtk_tabbar_enable_close_on_middle_click(&pTabGroup->tabbar);
}

dtk_result dtk_tabgroup_disable_close_on_middle_click(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return DTK_FALSE;
    }

    return dtk_tabbar_disable_close_on_middle_click(&pTabGroup->tabbar);
}

dtk_bool32 dtk_tabgroup_is_close_on_middle_click_enabled(const dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return DTK_FALSE;
    }

    return dtk_tabbar_is_close_on_middle_click_enabled(&pTabGroup->tabbar);
}


dtk_result dtk_tabgroup_transform_point_from_tab(const dtk_tabgroup* pTabGroup, dtk_uint32 tabIndex, dtk_int32* pX, dtk_int32* pY)
{
    if (pTabGroup == NULL) {
        return DTK_FALSE;
    }

    // The first thing to do is to get the position relative to the tab bar. Then we transform that position to make it relative
    // to the tab group.
    dtk_result result = dtk_tabbar_transform_point_from_tab(&pTabGroup->tabbar, tabIndex, pX, pY);
    if (result != DTK_SUCCESS) {
        return result;
    }

    // At this point the position is relative to the tab bar. So now we just make it relative to the tab group.
    dtk_int32 tabbarRelativePosX;
    dtk_int32 tabbarRelativePosY;
    result = dtk_control_get_relative_position(DTK_CONTROL(&pTabGroup->tabbar), &tabbarRelativePosX, &tabbarRelativePosY);
    if (result != DTK_SUCCESS) {
        return result;
    }

    if (pX) *pX += tabbarRelativePosX;
    if (pY) *pY += tabbarRelativePosY;

    return result;
}


// Styling
dtk_result dtk_tabgroup_set_tabbar_background_color(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_bg_color(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_padding(dtk_tabgroup* pTabGroup, dtk_uint32 padding)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_tab_padding(&pTabGroup->tabbar, padding, padding, padding, padding);
}

dtk_result dtk_tabgroup_set_tab_background_color(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_tab_bg_color(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_background_color_active(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_tab_bg_color_active(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_background_color_hovered(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_tab_bg_color_hovered(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_font(dtk_tabgroup* pTabGroup, dtk_font* pFont)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_font(&pTabGroup->tabbar, pFont);
}

dtk_result dtk_tabgroup_set_tab_text_color(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_text_color(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_text_color_active(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_text_color_active(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_text_color_hovered(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_text_color_hovered(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_close_button_padding(dtk_tabgroup* pTabGroup, dtk_uint32 padding)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_close_button_padding(&pTabGroup->tabbar, padding, padding, padding, padding);
}

dtk_result dtk_tabgroup_set_tab_close_button_left_padding(dtk_tabgroup* pTabGroup, dtk_uint32 padding)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_close_button_padding(&pTabGroup->tabbar, padding, pTabGroup->tabbar.closeButtonPaddingTop, pTabGroup->tabbar.closeButtonPaddingRight, pTabGroup->tabbar.closeButtonPaddingBottom);
}

dtk_result dtk_tabgroup_set_tab_close_button_image(dtk_tabgroup* pTabGroup, dtk_image* pImage)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_close_button_image(&pTabGroup->tabbar, pImage);
}

dtk_result dtk_tabgroup_set_tab_close_button_color(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_close_button_color(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_close_button_color_tab_hovered(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_close_button_color_tab_hovered(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_close_button_color_tab_active(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_close_button_color_tab_active(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_close_button_color_hovered(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_close_button_color_hovered(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_close_button_color_pressed(dtk_tabgroup* pTabGroup, dtk_color color)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_close_button_color_pressed(&pTabGroup->tabbar, color);
}

dtk_result dtk_tabgroup_set_tab_close_button_size(dtk_tabgroup* pTabGroup, dtk_uint32 width, dtk_uint32 height)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_set_close_button_size(&pTabGroup->tabbar, width, height);
}

dtk_result dtk_tabgroup_show_tab_close_buttons(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_show_close_button(&pTabGroup->tabbar);
}

dtk_result dtk_tabgroup_hide_tab_close_buttons(dtk_tabgroup* pTabGroup)
{
    if (pTabGroup == NULL) {
        return DTK_INVALID_ARGS;
    }

    return dtk_tabbar_hide_close_button(&pTabGroup->tabbar);
}

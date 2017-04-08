// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef struct
{
    dtk_int32 posX;
    dtk_int32 posY;
    dtk_uint32 width;
    dtk_uint32 height;
    dtk_tabbar_tab* pTab;
    dtk_bool32 _isLast;     // Internal use only. Used to indicate whether or not this iterator represents the last tab.
    dtk_int32 _nextIndex;   // Internal use only.
} dtk_tabbar__iterator;

dtk_bool32 dtk_tabbar__next_tab(dtk_tabbar* pTabBar, dtk_tabbar__iterator* pIterator)
{
    dtk_assert(pTabBar != NULL);
    dtk_assert(pIterator != NULL);

    if (pIterator->_isLast) {
        return DTK_FALSE;   // Reached the end of iteration.
    }

    pIterator->_nextIndex += 1;
    if ((dtk_uint32)pIterator->_nextIndex+1 == pTabBar->tabCount) {
        pIterator->_isLast = DTK_TRUE;
    }

    pIterator->pTab = &pTabBar->pTabs[pIterator->_nextIndex];

    
    dtk_uint32 prevTabWidth = pIterator->width;
    dtk_uint32 prevTabHeight = pIterator->height;

    // The size and position of each tab depends on the flow and text direction of the tabbar.
    float textWidth;
    float textHeight;
    dtk_font_measure_string(dtk_tabbar_get_font(pTabBar), 1, pIterator->pTab->text, strlen(pIterator->pTab->text), &textWidth, &textHeight);

    dtk_uint32 nextTabWidth = 0;
    dtk_uint32 nextTabHeight = 0;
    switch (pTabBar->textDirection)
    {
        case dtk_tabbar_text_direction_horizontal:
        {
            nextTabWidth  = (dtk_uint32)textWidth;
            nextTabHeight = (dtk_uint32)textHeight;
        } break;
        case dtk_tabbar_text_direction_vertical:
        {
            nextTabWidth  = (dtk_uint32)textHeight;
            nextTabHeight = (dtk_uint32)textWidth;
        } break;
        default: break; // Will never hit this.
    }

    switch (pTabBar->flow)
    {
        case dtk_tabbar_flow_left_to_right:
        {
            pIterator->posX = pIterator->posX + prevTabWidth;
        } break;
        case dtk_tabbar_flow_top_to_bottom:
        {
            pIterator->posY = pIterator->posY + prevTabHeight;
        } break;
        case dtk_tabbar_flow_right_to_left:
        {
            pIterator->posX = pIterator->posX - nextTabWidth;
        } break;
        case dtk_tabbar_flow_bottom_to_top:
        {
            pIterator->posY = pIterator->posY - nextTabHeight;
        } break;
        default: break; // Will never hit this.
    }

    pIterator->width  = nextTabWidth;
    pIterator->height = nextTabHeight;

    return DTK_TRUE;
}

dtk_bool32 dtk_tabbar__first_tab(dtk_tabbar* pTabBar, dtk_tabbar__iterator* pIterator)
{
    dtk_assert(pTabBar != NULL);
    dtk_assert(pIterator != NULL);

    if (pTabBar->tabCount == 0) {
        return DTK_FALSE;   // No tabs.
    }

    dtk_zero_object(pIterator);
    pIterator->_nextIndex = -1;

    // The initial position depends on the tab flow.
    if (pTabBar->flow == dtk_tabbar_flow_right_to_left) {
        pIterator->posX = dtk_control_get_width(DTK_CONTROL(pTabBar));
    } else if (pTabBar->flow == dtk_tabbar_flow_bottom_to_top) {
        pIterator->posY = dtk_control_get_height(DTK_CONTROL(pTabBar));
    }

    return dtk_tabbar__next_tab(pTabBar, pIterator);
}




dtk_result dtk_tabbar_init(dtk_context* pTK, dtk_control* pParent, dtk_tabbar_flow flow, dtk_tabbar_text_direction textDirection, dtk_event_proc onEvent, dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pTabBar);

    dtk_result result = dtk_control_init(pTK, pParent, DTK_CONTROL_TYPE_TABBAR, onEvent, &pTabBar->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    pTabBar->flow = flow;
    pTabBar->textDirection = textDirection;

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

    dtk_tabbar* pTabBar = DTK_TABBAR(pEvent->pControl);
    dtk_assert(pTabBar != NULL);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dtk_font* pFont = dtk_tabbar_get_font(pTabBar);

            dtk_font_metrics fontMetrics;
            dtk_font_get_metrics(pFont, 1, &fontMetrics);

            dtk_tabbar__iterator iterator;
            if (dtk_tabbar__first_tab(pTabBar, &iterator)) {
                do
                {
                    if (pTabBar->textDirection == dtk_tabbar_text_direction_horizontal) {
                        // Horizontal text.
                        dtk_surface_draw_text(pEvent->paint.pSurface, pFont, 1, iterator.pTab->text, strlen(iterator.pTab->text), iterator.posX, iterator.posY, pTabBar->textFGColor, pTabBar->textBGColor);
                    } else {
                        // Vertical text.
                        dtk_surface_push(pEvent->paint.pSurface);
                        {
                            dtk_surface_translate(pEvent->paint.pSurface, iterator.posX + fontMetrics.lineHeight, iterator.posY);
                            dtk_surface_rotate(pEvent->paint.pSurface, 90);
                            dtk_surface_draw_text(pEvent->paint.pSurface, pFont, 1, iterator.pTab->text, strlen(iterator.pTab->text), 0, 0, pTabBar->textFGColor, pTabBar->textBGColor);
                        }
                        dtk_surface_pop(pEvent->paint.pSurface);
                    }
                } while (dtk_tabbar__next_tab(pTabBar, &iterator));
            }
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
        //dtk_tabbar_auto_resize(pTabBar);
    }
    
    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}

dtk_font* dtk_tabbar_get_font(dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return NULL;
    return (pTabBar->pFont != NULL) ? pTabBar->pFont : dtk_get_default_font(pTabBar->control.pTK);
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



dtk_result dtk_tabbar_tab_init(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage, dtk_tabbar_tab* pTab)
{
    (void)pTabBar;  // Not used for now, but will probably be used later when more efficient memory management is implemented.

    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    pTab->text = dtk_make_string(text);
    pTab->pPage = pTabPage;

    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_tab_uninit(dtk_tabbar_tab* pTab)
{
    if (pTab == NULL) return DTK_INVALID_ARGS;
    
    dtk_free_string(pTab->text);
    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_append_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    if (pTabBar->tabCount == pTabBar->tabCapacity) {
        size_t newTabCapacity = (pTabBar->tabCapacity == 0) ? 1 : pTabBar->tabCapacity * 2;
        dtk_tabbar_tab* pNewTabs = (dtk_tabbar_tab*)realloc(pTabBar->pTabs, sizeof(*pNewTabs) * newTabCapacity);
        if (pNewTabs == NULL) {
            return DTK_OUT_OF_MEMORY;
        }

        pTabBar->pTabs = pNewTabs;
        pTabBar->tabCapacity = newTabCapacity;
    }

    dtk_assert(pTabBar->tabCapacity > pTabBar->tabCount);

    dtk_tabbar_tab_init(pTabBar, text, pTabPage, &pTabBar->pTabs[pTabBar->tabCount]);
    pTabBar->tabCount += 1;

	return DTK_SUCCESS;
}

dtk_result dtk_tabbar_prepend_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage)
{
	if (pTabBar == NULL) return DTK_INVALID_ARGS;

    if (pTabBar->tabCount == pTabBar->tabCapacity) {
        size_t newTabCapacity = (pTabBar->tabCapacity == 0) ? 1 : pTabBar->tabCapacity * 2;
        dtk_tabbar_tab* pNewTabs = (dtk_tabbar_tab*)malloc(sizeof(*pNewTabs) * newTabCapacity);
        if (pNewTabs == NULL) {
            return DTK_OUT_OF_MEMORY;
        }

        // Make sure there's room at the front of the buffer.
        memcpy(pNewTabs + 1, pTabBar->pTabs, sizeof(*pTabBar->pTabs) * pTabBar->tabCount);

        pTabBar->pTabs = pNewTabs;
        pTabBar->tabCapacity = newTabCapacity;
    }

    dtk_assert(pTabBar->tabCapacity > pTabBar->tabCount);
    
    dtk_tabbar_tab_init(pTabBar, text, pTabPage, &pTabBar->pTabs[0]);
    pTabBar->tabCount += 1;

	return DTK_SUCCESS;
}

dtk_result dtk_tabbar_remove_tab_by_index(dtk_tabbar* pTabBar, size_t tabIndex)
{
    if (pTabBar == NULL || pTabBar->tabCount <= tabIndex) return DTK_INVALID_ARGS;

    dtk_tabbar_tab_uninit(&pTabBar->pTabs[tabIndex]);
    for (size_t i = tabIndex; i < pTabBar->tabCount-1; ++i) {
        pTabBar->pTabs[i] = pTabBar->pTabs[i+1];
    }

    pTabBar->tabCount -= 1;
    return DTK_SUCCESS;
}

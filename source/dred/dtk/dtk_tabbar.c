// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    dtk_int32 posX;
    dtk_int32 posY;
    dtk_uint32 width;
    dtk_uint32 height;
    float textWidth;
    float textHeight;
    dtk_tabbar_tab* pTab;
    dtk_bool32 _isLast;         // Internal use only. Used to indicate whether or not this iterator represents the last tab.
    dtk_int32 _nextIndex;       // Internal use only.
    float _longestTextWidth;    // Internal use only.
} dtk_tabbar__iterator;

float dtk_tabbar__find_longest_tab_text(dtk_tabbar* pTabBar)
{
    float longestWidth = 0;

    for (dtk_uint32 iTab = 0; iTab < pTabBar->tabCount; ++iTab) {
        dtk_tabbar_tab* pTab = &pTabBar->pTabs[iTab];

        float textWidth;
        float textHeight;
        dtk_font_measure_string(dtk_tabbar_get_font(pTabBar), 1, pTab->text, strlen(pTab->text), &textWidth, &textHeight);

        if (longestWidth < textWidth) {
            longestWidth = textWidth;
        }
    }

    return longestWidth;
}

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
    pIterator->textWidth  = textWidth;       // <-- Set these before the adjustment below. Want this to measure the actual text and not the excess from stretching.
    pIterator->textHeight = textHeight;

    if ((pTabBar->textDirection == dtk_tabbar_text_direction_horizontal && (pTabBar->flow == dtk_tabbar_flow_top_to_bottom || pTabBar->flow == dtk_tabbar_flow_bottom_to_top)) ||
        (pTabBar->textDirection == dtk_tabbar_text_direction_vertical   && (pTabBar->flow == dtk_tabbar_flow_left_to_right || pTabBar->flow == dtk_tabbar_flow_right_to_left))) {
        textWidth = pIterator->_longestTextWidth;
    }

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

    nextTabWidth += pTabBar->paddingLeft + pTabBar->paddingRight;
    nextTabHeight += pTabBar->paddingTop + pTabBar->paddingBottom;

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

    if ((pTabBar->textDirection == dtk_tabbar_text_direction_horizontal && (pTabBar->flow == dtk_tabbar_flow_top_to_bottom || pTabBar->flow == dtk_tabbar_flow_bottom_to_top)) ||
        (pTabBar->textDirection == dtk_tabbar_text_direction_vertical   && (pTabBar->flow == dtk_tabbar_flow_left_to_right || pTabBar->flow == dtk_tabbar_flow_right_to_left))) {
        pIterator->_longestTextWidth = dtk_tabbar__find_longest_tab_text(pTabBar);
    }

    return dtk_tabbar__next_tab(pTabBar, pIterator);
}


void dtk_tabbar__set_hovered_tab(dtk_tabbar* pTabBar, dtk_int32 tabIndex)
{
    dtk_assert(pTabBar != NULL);
    dtk_assert(tabIndex < (dtk_int32)pTabBar->tabCount);

    if (pTabBar->hoveredTabIndex != tabIndex) {
        pTabBar->hoveredTabIndex = tabIndex;
        dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));   // <-- Redraw the entire control for now, but can optimize this later if necessary, which it probably isn't.
    }
}

void dtk_tabbar__unset_hovered_tab(dtk_tabbar* pTabBar)
{
    dtk_tabbar__set_hovered_tab(pTabBar, -1);
}

void dtk_tabbar__set_active_tab(dtk_tabbar* pTabBar, dtk_int32 tabIndex)
{
    dtk_assert(pTabBar != NULL);
    dtk_assert(tabIndex < (dtk_int32)pTabBar->tabCount);

    dtk_int32 oldTabIndex = pTabBar->activeTabIndex;
    dtk_int32 newTabIndex = tabIndex;
    if (oldTabIndex != newTabIndex) {
        dtk_event e = dtk_event_init(DTK_CONTROL(pTabBar)->pTK, DTK_EVENT_TABBAR_CHANGE_TAB, DTK_CONTROL(pTabBar));
        e.tabbar.newTabIndex = newTabIndex;
        e.tabbar.oldTabIndex = oldTabIndex;
        dtk_control_post_event(DTK_CONTROL(pTabBar), &e);

        // The actual hiding/showing of the tab pages and redrawing is done in the default event handler. The reason for this is that
        // it allows a custom event handler to cancel the tab change by simply not posting the event to the default event handler.
    }
}

void dtk_tabbar__unset_active_tab(dtk_tabbar* pTabBar)
{
    dtk_tabbar__set_active_tab(pTabBar, -1);
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
    pTabBar->hoveredTabIndex = -1;
    pTabBar->activeTabIndex = -1;

    // Default style.
    pTabBar->bgColor           = dtk_rgb(192, 192, 192);
    pTabBar->bgColorTab        = dtk_rgb(192, 192, 192);
    pTabBar->bgColorActiveTab  = dtk_rgb(128, 128, 128);
    pTabBar->bgColorHoveredTab = dtk_rgb(160, 160, 160);
    pTabBar->textColor         = dtk_rgb(0, 0, 0);
    pTabBar->paddingLeft       = 4;
    pTabBar->paddingTop        = 4;
    pTabBar->paddingRight      = 4;
    pTabBar->paddingBottom     = 4;

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

            dtk_rect tabGroupRect = dtk_rect_inside_out();

            dtk_int32 tabIndex = 0;
            dtk_tabbar__iterator iterator;
            if (dtk_tabbar__first_tab(pTabBar, &iterator)) {
                do
                {
                    dtk_color bgColor = pTabBar->bgColorTab;
                    dtk_color fgColor = pTabBar->textColor;
                    if (tabIndex == pTabBar->hoveredTabIndex) {
                        bgColor = pTabBar->bgColorHoveredTab;
                    }
                    if (tabIndex == pTabBar->activeTabIndex) {
                        bgColor = pTabBar->bgColorActiveTab;
                    }

                    dtk_rect tabRect = dtk_rect_init(iterator.posX, iterator.posY, iterator.posX + iterator.width, iterator.posY + iterator.height);
                    tabGroupRect = dtk_rect_union(tabGroupRect, tabRect);


                    // Padding area.
                    dtk_rect paddingRectLeft   = dtk_rect_init(tabRect.left,                          tabRect.top,                             tabRect.left  + pTabBar->paddingLeft,  tabRect.bottom);
                    dtk_rect paddingRectRight  = dtk_rect_init(tabRect.right - pTabBar->paddingRight, tabRect.top,                             tabRect.right,                         tabRect.bottom);
                    dtk_rect paddingRectTop    = dtk_rect_init(tabRect.left  + pTabBar->paddingLeft,  tabRect.top,                             tabRect.right - pTabBar->paddingRight, tabRect.top + pTabBar->paddingTop);
                    dtk_rect paddingRectBottom = dtk_rect_init(tabRect.left  + pTabBar->paddingLeft,  tabRect.bottom - pTabBar->paddingBottom, tabRect.right - pTabBar->paddingRight, tabRect.bottom);
                    dtk_surface_draw_rect(pEvent->paint.pSurface, paddingRectLeft,   bgColor);
                    dtk_surface_draw_rect(pEvent->paint.pSurface, paddingRectRight,  bgColor);
                    dtk_surface_draw_rect(pEvent->paint.pSurface, paddingRectTop,    bgColor);
                    dtk_surface_draw_rect(pEvent->paint.pSurface, paddingRectBottom, bgColor);

                    if (pTabBar->textDirection == dtk_tabbar_text_direction_horizontal) {
                        // Horizontal text.
                        dtk_surface_draw_text(pEvent->paint.pSurface, pFont, 1, iterator.pTab->text, strlen(iterator.pTab->text), iterator.posX + pTabBar->paddingLeft, iterator.posY + pTabBar->paddingTop, fgColor, bgColor);

                        dtk_rect excessRect = dtk_rect_init(
                            iterator.posX + pTabBar->paddingLeft + (dtk_int32)iterator.textWidth,
                            iterator.posY + pTabBar->paddingTop,
                            iterator.posX + iterator.width  - pTabBar->paddingRight,
                            iterator.posY + iterator.height - pTabBar->paddingBottom);
                        dtk_surface_draw_rect(pEvent->paint.pSurface, excessRect, bgColor);
                    } else {
                        // Vertical text.
                        dtk_surface_push(pEvent->paint.pSurface);
                        {
                            dtk_surface_translate(pEvent->paint.pSurface, iterator.posX + fontMetrics.lineHeight + pTabBar->paddingLeft, iterator.posY + pTabBar->paddingTop);
                            dtk_surface_rotate(pEvent->paint.pSurface, 90);
                            dtk_surface_draw_text(pEvent->paint.pSurface, pFont, 1, iterator.pTab->text, strlen(iterator.pTab->text), 0, 0, fgColor, bgColor);
                        }
                        dtk_surface_pop(pEvent->paint.pSurface);

                        dtk_rect excessRect = dtk_rect_init(
                            iterator.posX + pTabBar->paddingLeft,
                            iterator.posY + pTabBar->paddingTop + (dtk_int32)iterator.textWidth,
                            iterator.posX + iterator.width  - pTabBar->paddingRight,
                            iterator.posY + iterator.height - pTabBar->paddingBottom);
                        dtk_surface_draw_rect(pEvent->paint.pSurface, excessRect, bgColor);
                    }

                    tabIndex += 1;
                } while (dtk_tabbar__next_tab(pTabBar, &iterator));
            }

            // Now we need to draw the background of the main control.
            dtk_rect tabbarRect = dtk_control_get_local_rect(DTK_CONTROL(pTabBar));
            dtk_rect bgRectLeft   = dtk_rect_init(tabbarRect.left,                      tabbarRect.top,                       tabbarRect.left + tabGroupRect.left,  tabbarRect.bottom);
            dtk_rect bgRectRight  = dtk_rect_init(tabbarRect.left + tabGroupRect.right, tabbarRect.top,                       tabbarRect.right,                     tabbarRect.bottom);
            dtk_rect bgRectTop    = dtk_rect_init(tabbarRect.left + tabGroupRect.left,  tabbarRect.top,                       tabbarRect.left + tabGroupRect.right, tabbarRect.top + tabGroupRect.top);
            dtk_rect bgRectBottom = dtk_rect_init(tabbarRect.left + tabGroupRect.left,  tabbarRect.top + tabGroupRect.bottom, tabbarRect.left + tabGroupRect.right, tabbarRect.bottom);
            dtk_surface_draw_rect(pEvent->paint.pSurface, bgRectLeft,   pTabBar->bgColor);
            dtk_surface_draw_rect(pEvent->paint.pSurface, bgRectRight,  pTabBar->bgColor);
            dtk_surface_draw_rect(pEvent->paint.pSurface, bgRectTop,    pTabBar->bgColor);
            dtk_surface_draw_rect(pEvent->paint.pSurface, bgRectBottom, pTabBar->bgColor);
        } break;

        case DTK_EVENT_MOUSE_LEAVE:
        {
            dtk_tabbar__unset_hovered_tab(pTabBar);
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            dtk_tabbar_hit_test_result hit;
            if (dtk_tabbar_hit_test(pTabBar, pEvent->mouseMove.x, pEvent->mouseMove.y, &hit)) {
                // It's over a tab.
                dtk_tabbar__set_hovered_tab(pTabBar, hit.tabIndex);
            } else {
                // It's not over a tab.
                dtk_tabbar__unset_hovered_tab(pTabBar);
            }
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        {
            dtk_tabbar_hit_test_result hit;
            if (dtk_tabbar_hit_test(pTabBar, pEvent->mouseMove.x, pEvent->mouseMove.y, &hit)) {
                dtk_tabbar__set_active_tab(pTabBar, hit.tabIndex);
            }
        } break;

        case DTK_EVENT_TABBAR_CHANGE_TAB:
        {
            if (pEvent->tabbar.oldTabIndex != -1) {
                dtk_control_hide(pTabBar->pTabs[pEvent->tabbar.oldTabIndex].pPage);
            }
            if (pEvent->tabbar.newTabIndex != -1) {
                dtk_control_show(pTabBar->pTabs[pEvent->tabbar.newTabIndex].pPage);
            }

            pTabBar->activeTabIndex = pEvent->tabbar.newTabIndex;
            dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));   // <-- Redraw the entire control for now, but can optimize this later if necessary, which it probably isn't.
        } break;

        default: break;
    }

    return dtk_control_default_event_handler(pEvent);
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

dtk_result dtk_tabbar_set_text_color(dtk_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    pTabBar->textColor = color;
    
    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_set_bg_color(dtk_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    pTabBar->bgColorTab = color;
    
    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_set_bg_color_active(dtk_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    pTabBar->bgColorActiveTab = color;
    
    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_set_bg_color_hovered(dtk_tabbar* pTabBar, dtk_color color)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    pTabBar->bgColorHoveredTab = color;
    
    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_set_padding(dtk_tabbar* pTabBar, dtk_uint32 paddingLeft, dtk_uint32 paddingTop, dtk_uint32 paddingRight, dtk_uint32 paddingBottom)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    pTabBar->paddingLeft   = paddingLeft;
    pTabBar->paddingTop    = paddingTop;
    pTabBar->paddingRight  = paddingRight;
    pTabBar->paddingBottom = paddingBottom;
    
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


dtk_bool32 dtk_tabbar_hit_test(dtk_tabbar* pTabBar, dtk_int32 x, dtk_int32 y, dtk_tabbar_hit_test_result* pResult)
{
    if (pTabBar == NULL) return DTK_FALSE;

    pResult->tabIndex = -1;
    pResult->relativePosX = 0;
    pResult->relativePosY = 0;
    pResult->isOverCloseButton = DTK_FALSE;

    dtk_int32 tabIndex = 0;
    dtk_tabbar__iterator iterator;
    if (dtk_tabbar__first_tab(pTabBar, &iterator)) {
        do
        {
            dtk_rect tabRect = dtk_rect_init(iterator.posX, iterator.posY, iterator.posX + iterator.width, iterator.posY + iterator.height);
            if (dtk_rect_contains_point(tabRect, x, y)) {
                pResult->tabIndex = tabIndex;
                pResult->relativePosX = x - iterator.posX;
                pResult->relativePosY = y - iterator.posY;

                // TODO: Check if the point is over the close button.
                pResult->isOverCloseButton = DTK_FALSE;

                return DTK_TRUE;
            }

            tabIndex += 1;
        } while (dtk_tabbar__next_tab(pTabBar, &iterator));
    }

    return DTK_FALSE;
}

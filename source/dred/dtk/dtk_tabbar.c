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

    dtk_tabbar* pTabBar = DTK_TABBAR(pEvent->pControl);
    dtk_assert(pTabBar != NULL);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dtk_font* pFont = (pTabBar->pFont != NULL) ? pTabBar->pFont : dtk_get_default_font(pTabBar->control.pTK);

            float penPosX = 20;
            float penPosY = 40;
            for (size_t iTab = 0; iTab < pTabBar->tabCount; ++iTab) {
                dtk_tabbar_tab* pTab = &pTabBar->pTabs[iTab];

                float textWidth;
                float textHeight;
                dtk_font_measure_string(pFont, 1, pTab->text, strlen(pTab->text), &textWidth, &textHeight);
                dtk_surface_draw_text(pEvent->paint.pSurface, pFont, 1, pTab->text, strlen(pTab->text), (dtk_int32)penPosX, (dtk_int32)penPosY, pTabBar->textFGColor, pTabBar->textBGColor);

                // Tabs are drawn differently depending on orientation.
                if (pTabBar->orientation == dtk_tabbar_orientation_top || pTabBar->orientation == dtk_tabbar_orientation_bottom) {
                    // Horizontal tabs.
                    penPosX += textWidth;
                } else {
                    // Vertical tabs.
                    penPosY += textWidth;
                }
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

// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    dtk_int32 posX;
    dtk_int32 posY;
    dtk_uint32 width;
    dtk_uint32 height;
    dtk_rect textRect;
    dtk_rect closeButtonRect;
    dtk_rect pinButtonRect;
    dtk_tabbar_tab* pTab;
    dtk_bool32 _isLast;             // Internal use only. Used to indicate whether or not this iterator represents the last tab.
    dtk_int32 _nextIndex;           // Internal use only.
    dtk_int32 _longestTextWidth;    // Internal use only.
} dtk_tabbar__iterator;

dtk_int32 dtk_tabbar__find_longest_tab_text(dtk_tabbar* pTabBar)
{
    dtk_int32 longestWidth = 0;

    for (dtk_uint32 iTab = 0; iTab < pTabBar->tabCount; ++iTab) {
        dtk_tabbar_tab* pTab = &pTabBar->pTabs[iTab];

        dtk_int32 textWidth;
        dtk_int32 textHeight;
        dtk_font_measure_string(dtk_tabbar_get_font(pTabBar), 1, pTab->pText, strlen(pTab->pText), &textWidth, &textHeight);

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


    // The close button also affects the size of each tab.
    dtk_uint32 closeButtonImageWidth = 0;
    dtk_uint32 closeButtonImageHeight = 0;
    if (dtk_tabbar_is_showing_close_button(pTabBar)) {
        dtk_image* pCloseButtonImage = dtk_tabbar_get_close_button_image(pTabBar);
        if (pCloseButtonImage != NULL) {
            dtk_image_get_size(pCloseButtonImage, &closeButtonImageWidth, &closeButtonImageHeight);
        }

        closeButtonImageWidth += pTabBar->closeButtonPaddingLeft + pTabBar->closeButtonPaddingRight;
        closeButtonImageHeight += pTabBar->closeButtonPaddingTop + pTabBar->closeButtonPaddingBottom;
    }


    dtk_uint32 prevTabWidth = pIterator->width;
    dtk_uint32 prevTabHeight = pIterator->height;

    // The size and position of each tab depends on the flow and text direction of the tabbar.
    dtk_int32 actualTextWidth;
    dtk_int32 actualTextHeight;
    dtk_font_measure_string(dtk_tabbar_get_font(pTabBar), 1, pIterator->pTab->pText, strlen(pIterator->pTab->pText), &actualTextWidth, &actualTextHeight);

    dtk_int32 longestTextWidth = actualTextWidth;
    dtk_int32 longestTextHeight = actualTextHeight;
    if ((pTabBar->textDirection == dtk_tabbar_text_direction_horizontal && (pTabBar->flow == dtk_tabbar_flow_top_to_bottom || pTabBar->flow == dtk_tabbar_flow_bottom_to_top)) ||
        (pTabBar->textDirection == dtk_tabbar_text_direction_vertical   && (pTabBar->flow == dtk_tabbar_flow_left_to_right || pTabBar->flow == dtk_tabbar_flow_right_to_left))) {
        longestTextWidth = pIterator->_longestTextWidth;
    }

    // These are set in the switch below.
    dtk_int32 orientedTextWidth = 0;
    dtk_int32 orientedTextHeight = 0;
    dtk_uint32 orientedCloseButtonWidth = 0;
    dtk_uint32 orientedCloseButtonHeight = 0;


    dtk_uint32 nextTabWidth = 0;
    dtk_uint32 nextTabHeight = 0;
    switch (pTabBar->textDirection)
    {
        case dtk_tabbar_text_direction_horizontal:
        {
            orientedTextWidth = actualTextWidth;
            orientedTextHeight = actualTextHeight;
            orientedCloseButtonWidth = closeButtonImageWidth;
            orientedCloseButtonHeight = closeButtonImageHeight;
            nextTabWidth  = (dtk_uint32)longestTextWidth + orientedCloseButtonWidth;
            nextTabHeight = dtk_max((dtk_uint32)longestTextHeight, orientedCloseButtonHeight);
        } break;
        case dtk_tabbar_text_direction_vertical:
        {
            orientedTextWidth = actualTextHeight;
            orientedTextHeight = actualTextWidth;
            orientedCloseButtonWidth = closeButtonImageHeight;
            orientedCloseButtonHeight = closeButtonImageWidth;
            nextTabWidth  = dtk_max((dtk_uint32)longestTextHeight, orientedCloseButtonWidth);
            nextTabHeight = (dtk_uint32)longestTextWidth + orientedCloseButtonHeight;
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


    // At this point we have all the information we need to calculate the positions for each of the different elements.
    switch (pTabBar->textDirection)
    {
        case dtk_tabbar_text_direction_horizontal:
        {
            pIterator->textRect.left   = pTabBar->paddingLeft;
            pIterator->textRect.top    = pTabBar->paddingTop + (((dtk_int32)(nextTabHeight - pTabBar->paddingBottom - pTabBar->paddingTop) - (dtk_int32)orientedTextHeight)/2);
            pIterator->textRect.right  = pIterator->textRect.left + orientedTextWidth;
            pIterator->textRect.bottom = pIterator->textRect.top + orientedTextHeight;

            if (dtk_tabbar_is_showing_close_button(pTabBar)) {
                dtk_uint32 imageWidth;
                dtk_uint32 imageHeight;
                dtk_tabbar_get_close_button_size(pTabBar, &imageWidth, &imageHeight);

                pIterator->closeButtonRect.left   = pTabBar->paddingLeft + longestTextWidth + pTabBar->closeButtonPaddingLeft;
                pIterator->closeButtonRect.top    = pTabBar->paddingTop + pTabBar->closeButtonPaddingTop + (((dtk_int32)(nextTabHeight - pTabBar->paddingBottom - pTabBar->paddingTop) - (dtk_int32)imageHeight)/2);
                pIterator->closeButtonRect.right  = pIterator->closeButtonRect.left + imageWidth;
                pIterator->closeButtonRect.bottom = pIterator->closeButtonRect.top + imageHeight;
            }
        } break;
        case dtk_tabbar_text_direction_vertical:
        {
            pIterator->textRect.left   = pTabBar->paddingLeft + (((dtk_int32)(nextTabWidth - pTabBar->paddingRight - pTabBar->paddingLeft) - (dtk_int32)orientedTextWidth)/2);
            pIterator->textRect.top    = pTabBar->paddingTop;
            pIterator->textRect.right  = pIterator->textRect.left + orientedTextWidth;
            pIterator->textRect.bottom = pIterator->textRect.top + orientedTextHeight;

            if (dtk_tabbar_is_showing_close_button(pTabBar)) {
                dtk_uint32 imageWidth;
                dtk_uint32 imageHeight;
                dtk_tabbar_get_close_button_size(pTabBar, &imageWidth, &imageHeight);

                pIterator->closeButtonRect.left   = pTabBar->paddingLeft + pTabBar->closeButtonPaddingTop + (((dtk_int32)(nextTabWidth - pTabBar->paddingRight - pTabBar->paddingLeft) - (dtk_int32)imageWidth)/2);
                pIterator->closeButtonRect.top    = pTabBar->paddingTop + longestTextWidth + pTabBar->closeButtonPaddingLeft;
                pIterator->closeButtonRect.right  = pIterator->closeButtonRect.left + imageHeight;
                pIterator->closeButtonRect.bottom = pIterator->closeButtonRect.top + imageWidth;
            }
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

        // If the tooltip is visible, update it.
        if (pTabBar->isTooltipVisible) {
            dtk_do_tooltip(pTabBar->control.pTK);
        }
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



dtk_result dtk_tabbar_init(dtk_context* pTK, dtk_event_proc onEvent, dtk_control* pParent, dtk_tabbar_flow flow, dtk_tabbar_text_direction textDirection, dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pTabBar);

    dtk_result result = dtk_control_init(pTK, DTK_CONTROL_TYPE_TABBAR, (onEvent != NULL) ? onEvent : dtk_tabbar_default_event_handler, pParent, &pTabBar->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    pTabBar->flow = flow;
    pTabBar->textDirection = textDirection;
    pTabBar->hoveredTabIndex = -1;
    pTabBar->activeTabIndex = -1;

    // Default style.
    pTabBar->bgColor                  = dtk_rgb(192, 192, 192);
    pTabBar->bgColorTab               = dtk_rgb(192, 192, 192);
    pTabBar->bgColorActiveTab         = dtk_rgb(128, 128, 128);
    pTabBar->bgColorHoveredTab        = dtk_rgb(160, 160, 160);
    pTabBar->textColor                = dtk_rgb(0, 0, 0);
    pTabBar->paddingLeft              = 4;
    pTabBar->paddingTop               = 4;
    pTabBar->paddingRight             = 4;
    pTabBar->paddingBottom            = 4;
    pTabBar->closeButtonPaddingLeft   = 4;
    pTabBar->closeButtonPaddingTop    = 0;
    pTabBar->closeButtonPaddingRight  = 0;
    pTabBar->closeButtonPaddingBottom = 0;
    pTabBar->closeButtonColor         = dtk_rgb(224, 224, 224);
    pTabBar->closeButtonColorHovered  = dtk_rgb(255, 192, 192);
    pTabBar->closeButtonColorPressed  = dtk_rgb(192, 128, 128);

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

                    dtk_color closeButtonColor = pTabBar->closeButtonColor;
                    // TODO: Change the close button color based on hit testing.

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
                        dtk_surface_draw_text(pEvent->paint.pSurface, pFont, 1, iterator.pTab->pText, strlen(iterator.pTab->pText), iterator.posX + iterator.textRect.left, iterator.posY + iterator.textRect.top, fgColor, bgColor);

                        // Spacing above and below the text.
                        {
                            dtk_rect spacingRect;

                            // Above the text.
                            spacingRect.left   = iterator.posX + iterator.textRect.left;
                            spacingRect.top    = iterator.posY + pTabBar->paddingTop;
                            spacingRect.right  = iterator.posX + iterator.textRect.right;
                            spacingRect.bottom = iterator.posY + iterator.textRect.top;
                            dtk_surface_draw_rect(pEvent->paint.pSurface, spacingRect, bgColor);

                            // Below the text.
                            spacingRect.left   = iterator.posX + iterator.textRect.left;
                            spacingRect.top    = iterator.posY + iterator.textRect.bottom;
                            spacingRect.right  = iterator.posX + iterator.textRect.right;
                            spacingRect.bottom = iterator.posY + iterator.height - pTabBar->paddingBottom;
                            dtk_surface_draw_rect(pEvent->paint.pSurface, spacingRect, bgColor);
                        }

                        dtk_int32 cursorPosX = iterator.posX + iterator.textRect.right;

                        if (dtk_tabbar_is_showing_close_button(pTabBar)) {
                            // Spacing between text and the close button.
                            dtk_rect spacingRect;
                            spacingRect.left   = cursorPosX;
                            spacingRect.top    = iterator.posY + pTabBar->paddingTop;
                            spacingRect.right  = iterator.posX + iterator.closeButtonRect.left;
                            spacingRect.bottom = iterator.posY + iterator.height - pTabBar->paddingBottom;
                            dtk_surface_draw_rect(pEvent->paint.pSurface, spacingRect, bgColor);
                            
                            // The close button image.
                            dtk_draw_image_args drawImageArgs;
                            dtk_zero_object(&drawImageArgs);
                            drawImageArgs.dstX = iterator.posX + iterator.closeButtonRect.left;
                            drawImageArgs.dstY = iterator.posY + iterator.closeButtonRect.top;
                            drawImageArgs.dstWidth = dtk_rect_width(iterator.closeButtonRect);
                            drawImageArgs.dstHeight = dtk_rect_height(iterator.closeButtonRect);
                            drawImageArgs.srcX = 0;
                            drawImageArgs.srcY = 0;
                            drawImageArgs.srcWidth = dtk_image_get_width(pTabBar->pCloseButtonImage);
                            drawImageArgs.srcHeight = dtk_image_get_height(pTabBar->pCloseButtonImage);
                            drawImageArgs.foregroundColor = closeButtonColor;
                            drawImageArgs.backgroundColor = bgColor;
                            dtk_surface_draw_image(pEvent->paint.pSurface, pTabBar->pCloseButtonImage, &drawImageArgs);

                            // Spacing above and below the image.
                            {
                                // Above the text.
                                spacingRect.left   = iterator.posX + iterator.closeButtonRect.left;
                                spacingRect.top    = iterator.posY + pTabBar->paddingTop;
                                spacingRect.right  = iterator.posX + iterator.closeButtonRect.right;
                                spacingRect.bottom = iterator.posY + iterator.closeButtonRect.top;
                                dtk_surface_draw_rect(pEvent->paint.pSurface, spacingRect, bgColor);

                                // Below the text.
                                spacingRect.left   = iterator.posX + iterator.closeButtonRect.left;
                                spacingRect.top    = iterator.posY + iterator.closeButtonRect.bottom;
                                spacingRect.right  = iterator.posX + iterator.closeButtonRect.right;
                                spacingRect.bottom = iterator.posY + iterator.height - pTabBar->paddingBottom;
                                dtk_surface_draw_rect(pEvent->paint.pSurface, spacingRect, bgColor);
                            }

                            // Update the cursor position for the next parts.
                            cursorPosX = iterator.posX + iterator.closeButtonRect.right;
                        }

                        dtk_rect excessRect = dtk_rect_init(
                            cursorPosX,
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
                            dtk_surface_draw_text(pEvent->paint.pSurface, pFont, 1, iterator.pTab->pText, strlen(iterator.pTab->pText), 0, 0, fgColor, bgColor);
                        }
                        dtk_surface_pop(pEvent->paint.pSurface);

                        // Spacing to the left and right the text.
                        {
                            dtk_rect spacingRect;

                            // Left of the text.
                            spacingRect.left   = iterator.posX + pTabBar->paddingLeft;
                            spacingRect.top    = iterator.posY + iterator.textRect.top;
                            spacingRect.right  = iterator.posX + iterator.textRect.left;
                            spacingRect.bottom = iterator.posY + iterator.textRect.bottom;
                            dtk_surface_draw_rect(pEvent->paint.pSurface, spacingRect, bgColor);
                            
                            // Right of the text.
                            spacingRect.left   = iterator.posX + iterator.textRect.right;
                            spacingRect.top    = iterator.posY + iterator.textRect.top;
                            spacingRect.right  = iterator.posX + iterator.width - pTabBar->paddingRight;
                            spacingRect.bottom = iterator.posY + iterator.height - pTabBar->paddingBottom;
                            dtk_surface_draw_rect(pEvent->paint.pSurface, spacingRect, bgColor);
                        }

                        dtk_int32 cursorPosY = iterator.posY + iterator.textRect.bottom;

                        if (dtk_tabbar_is_showing_close_button(pTabBar)) {
                            // Spacing between text and the close button.
                            dtk_rect spacingRect;
                            spacingRect.left   = iterator.posX + pTabBar->paddingLeft;
                            spacingRect.top    = cursorPosY;
                            spacingRect.right  = iterator.posX + iterator.width - pTabBar->paddingRight;
                            spacingRect.bottom = iterator.posY + iterator.closeButtonRect.top;
                            dtk_surface_draw_rect(pEvent->paint.pSurface, spacingRect, bgColor);
                            
                            // The close button image.
                            dtk_draw_image_args drawImageArgs;
                            dtk_zero_object(&drawImageArgs);
                            drawImageArgs.dstX = 0;
                            drawImageArgs.dstY = 0;
                            drawImageArgs.dstWidth = dtk_rect_width(iterator.closeButtonRect);
                            drawImageArgs.dstHeight = dtk_rect_height(iterator.closeButtonRect);
                            drawImageArgs.srcX = 0;
                            drawImageArgs.srcY = 0;
                            drawImageArgs.srcWidth = dtk_image_get_width(pTabBar->pCloseButtonImage);
                            drawImageArgs.srcHeight = dtk_image_get_height(pTabBar->pCloseButtonImage);
                            drawImageArgs.foregroundColor = closeButtonColor;
                            drawImageArgs.backgroundColor = bgColor;

                            dtk_surface_push(pEvent->paint.pSurface);
                            {
                                dtk_surface_translate(pEvent->paint.pSurface, iterator.posX + iterator.closeButtonRect.left + dtk_rect_height(iterator.closeButtonRect), iterator.posY + iterator.closeButtonRect.top);
                                dtk_surface_rotate(pEvent->paint.pSurface, 90);
                                dtk_surface_draw_image(pEvent->paint.pSurface, pTabBar->pCloseButtonImage, &drawImageArgs);
                            }
                            dtk_surface_pop(pEvent->paint.pSurface);

                            // Spacing to the left and right the image.
                            {
                                // Left of the text.
                                spacingRect.left   = iterator.posX + pTabBar->paddingLeft;
                                spacingRect.top    = iterator.posY + iterator.closeButtonRect.top;
                                spacingRect.right  = iterator.posX + iterator.closeButtonRect.left;
                                spacingRect.bottom = iterator.posY + iterator.closeButtonRect.bottom;
                                dtk_surface_draw_rect(pEvent->paint.pSurface, spacingRect, bgColor);
                            
                                // Right of the text.
                                spacingRect.left   = iterator.posX + iterator.closeButtonRect.right;
                                spacingRect.top    = iterator.posY + iterator.closeButtonRect.top;
                                spacingRect.right  = iterator.posX + iterator.width - pTabBar->paddingRight;
                                spacingRect.bottom = iterator.posY + iterator.height - pTabBar->paddingBottom;
                                dtk_surface_draw_rect(pEvent->paint.pSurface, spacingRect, bgColor);
                            }

                            // Update the cursor position for the next parts.
                            cursorPosY = iterator.posY + iterator.closeButtonRect.bottom;
                        }

                        dtk_rect excessRect = dtk_rect_init(
                            iterator.posX + pTabBar->paddingLeft,
                            iterator.posY + cursorPosY,
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
            pTabBar->isTooltipVisible = DTK_FALSE;
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

        case DTK_EVENT_TOOLTIP:
        {
            dtk_tabbar_hit_test_result hit;
            if (dtk_tabbar_hit_test(pTabBar, pEvent->tooltip.x, pEvent->tooltip.y, &hit)) {
                dtk_string tooltipText = pTabBar->pTabs[hit.tabIndex].pTooltipText;
                if (!dtk_string_is_null_or_empty(tooltipText)) {
                    dtk_tooltip_show(&pEvent->tooltip.tooltip, tooltipText, hit.tabRect.left + pTabBar->control.absolutePosX, hit.tabRect.bottom + 2 + pTabBar->control.absolutePosY);
                }
            }

            pTabBar->isTooltipVisible = dtk_tooltip_is_visible(&pEvent->tooltip.tooltip);
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
    return (pTabBar->pFont != NULL) ? pTabBar->pFont : dtk_get_ui_font(pTabBar->control.pTK);
}

dtk_result dtk_tabbar_set_close_button_image(dtk_tabbar* pTabBar, dtk_image* pImage)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    pTabBar->pCloseButtonImage = pImage;

    if (pTabBar->isAutoResizeEnabled) {
        //dtk_tabbar_auto_resize(pTabBar);
    }

    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}

dtk_image* dtk_tabbar_get_close_button_image(dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return NULL;
    return pTabBar->pCloseButtonImage;
    //return (pTabBar->pCloseButtonImage != NULL) ? pTabBar->pCloseButtonImage : dtk_get_stock_image(DTK_CONTROL(pTabBar)->pTK, DTK_STOCK_IMAGE_CROSS);
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

dtk_result dtk_tabbar_show_close_button(dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    if (pTabBar->isShowingCloseButton) {
        return DTK_SUCCESS;
    }

    pTabBar->isShowingCloseButton = DTK_TRUE;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_hide_close_button(dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    if (!pTabBar->isShowingCloseButton) {
        return DTK_SUCCESS;
    }

    pTabBar->isShowingCloseButton = DTK_FALSE;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}

dtk_bool32 dtk_tabbar_is_showing_close_button(dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return DTK_FALSE;

    return pTabBar->isShowingCloseButton && pTabBar->pCloseButtonImage;
}

dtk_result dtk_tabbar_set_close_button_size(dtk_tabbar* pTabBar, dtk_uint32 width, dtk_uint32 height)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    pTabBar->closeButtonWidth = width;
    pTabBar->closeButtonHeight = height;

    dtk_control_scheduled_redraw(DTK_CONTROL(pTabBar), dtk_control_get_local_rect(DTK_CONTROL(pTabBar)));
    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_get_close_button_size(dtk_tabbar* pTabBar, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    if (pWidth) {
        *pWidth = dtk_tabbar_get_close_button_width(pTabBar);
    }
    if (pHeight) {
        *pHeight = dtk_tabbar_get_close_button_height(pTabBar);
    }

    return DTK_SUCCESS;
}

dtk_uint32 dtk_tabbar_get_close_button_width(dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return 0;

    if (pTabBar->pCloseButtonImage == NULL) {
        return 0;
    }

    if (pTabBar->closeButtonWidth == 0) {
        return dtk_image_get_width(pTabBar->pCloseButtonImage);
    }

    return pTabBar->closeButtonWidth;
}

dtk_uint32 dtk_tabbar_get_close_button_height(dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return 0;

    if (pTabBar->pCloseButtonImage == NULL) {
        return 0;
    }

    if (pTabBar->closeButtonHeight == 0) {
        return dtk_image_get_height(pTabBar->pCloseButtonImage);
    }

    return pTabBar->closeButtonHeight;
}



dtk_result dtk_tabbar_tab_init(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage, dtk_tabbar_tab* pTab)
{
    (void)pTabBar;  // Not used for now, but will probably be used later when more efficient memory management is implemented.

    if (pTabBar == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pTab);

    pTab->pText = dtk_make_string(text);
    pTab->pPage = pTabPage;

    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_tab_uninit(dtk_tabbar_tab* pTab)
{
    if (pTab == NULL) return DTK_INVALID_ARGS;
    
    dtk_free_string(pTab->pText);
    dtk_free_string(pTab->pTooltipText);
    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_append_tab(dtk_tabbar* pTabBar, const char* text, dtk_control* pTabPage)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    if (pTabBar->tabCount == pTabBar->tabCapacity) {
        dtk_uint32 newTabCapacity = (pTabBar->tabCapacity == 0) ? 1 : pTabBar->tabCapacity * 2;
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
        dtk_uint32 newTabCapacity = (pTabBar->tabCapacity == 0) ? 1 : pTabBar->tabCapacity * 2;
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

dtk_result dtk_tabbar_remove_tab_by_index(dtk_tabbar* pTabBar, dtk_uint32 tabIndex)
{
    if (pTabBar == NULL || pTabBar->tabCount <= tabIndex) return DTK_INVALID_ARGS;

    dtk_tabbar_tab_uninit(&pTabBar->pTabs[tabIndex]);
    for (dtk_uint32 i = tabIndex; i < pTabBar->tabCount-1; ++i) {
        pTabBar->pTabs[i] = pTabBar->pTabs[i+1];
    }

    pTabBar->tabCount -= 1;
    return DTK_SUCCESS;
}


dtk_result dtk_tabbar_set_tab_tooltip(dtk_tabbar* pTabBar, dtk_uint32 tabIndex, const char* pTooltipText)
{
    if (pTabBar == NULL || pTabBar->tabCount <= tabIndex) return DTK_INVALID_ARGS;

    pTabBar->pTabs[tabIndex].pTooltipText = dtk_set_string(pTabBar->pTabs[tabIndex].pTooltipText, pTooltipText);
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
                pResult->tabRect = tabRect;

                // TODO: Check if the point is over the close button.
                pResult->isOverCloseButton = DTK_FALSE;

                return DTK_TRUE;
            }

            tabIndex += 1;
        } while (dtk_tabbar__next_tab(pTabBar, &iterator));
    }

    return DTK_FALSE;
}

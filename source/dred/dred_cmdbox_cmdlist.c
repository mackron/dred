// Copyright (C) 2016 David Reid. See included LICENSE file.

dtk_rect dred_cmdbox_cmdlist__calculate_content_rect(dred_cmdbox_cmdlist* pCmdList)
{
    dtk_rect rect = dtk_control_get_local_rect(DTK_CONTROL(pCmdList));
    if (dtk_control_is_visible(DTK_CONTROL(&pCmdList->scrollbar))) {
        rect.right -= dtk_control_get_width(DTK_CONTROL(&pCmdList->scrollbar));
    }

    return rect;
}


void dred_cmdbox_cmdlist__update_scrollbar(dred_cmdbox_cmdlist* pCmdList)
{
    dtk_assert(pCmdList != NULL);

    // The page size is based on the number of line items that can fit on the main content area.
    dtk_rect contentRect = dred_cmdbox_cmdlist__calculate_content_rect(pCmdList);
    dtk_uint32 contentHeight = contentRect.bottom - contentRect.top;

    dtk_font_metrics fontMetrics;
    dtk_font_get_metrics(dtk_get_default_font(&pCmdList->pDred->tk), 1, &fontMetrics);

    dtk_uint32 pageSize = contentHeight / fontMetrics.lineHeight;
    dred_scrollbar_set_range_and_page_size(&pCmdList->scrollbar, 0, pCmdList->commandIndexCount, pageSize);
}

void dred_cmdbox_cmdlist__refresh_layout(dred_cmdbox_cmdlist* pCmdList)
{
    dtk_assert(pCmdList != NULL);

    // Update the size of the scrollbar first.
    dtk_uint32 scrollbarSizeX = 16;     // TODO: Make this a variable. Need some kind of standard scrollbar size.
    dtk_uint32 scrollbarSizeY = dtk_control_get_height(DTK_CONTROL(pCmdList));
    dtk_control_set_size(DTK_CONTROL(&pCmdList->scrollbar), scrollbarSizeX, scrollbarSizeY);

    // The scrollbar needs to be positioned on the right hand side.
    dtk_int32 scrollbarPosX = dtk_control_get_width(DTK_CONTROL(pCmdList)) - dtk_control_get_width(DTK_CONTROL(&pCmdList->scrollbar));
    dtk_int32 scrollbarPosY = 0;
    dtk_control_set_relative_position(DTK_CONTROL(&pCmdList->scrollbar), scrollbarPosX, scrollbarPosY);
}

void dred_cmdbox_cmdlist__on_scroll(dred_scrollbar* pScrollbar, int scrollPos)
{
    (void)scrollPos;

    // When the scrollbar is scrolled we need to redraw.
    dred_cmdbox_cmdlist* pCmdList = (dred_cmdbox_cmdlist*)dtk_control_get_parent(DTK_CONTROL(pScrollbar));
    dtk_control_scheduled_redraw(DTK_CONTROL(pCmdList), dtk_control_get_local_rect(DTK_CONTROL(pCmdList)));
}

dtk_bool32 dred_cmbox_cmdlist_event_handler(dtk_event* pEvent)
{
    dred_cmdbox_cmdlist* pCmdList = (dred_cmdbox_cmdlist*)pEvent->pControl;

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dred_context* pDred = pCmdList->pDred;
            float uiScale = pDred->uiScale;

            // The rectangle we draw inside of depends on whether or not the scrollbar is visible.
            dtk_rect contentRect = dred_cmdbox_cmdlist__calculate_content_rect(pCmdList);

            dtk_rect innerRect = contentRect;
            dtk_surface_set_clip(pEvent->paint.pSurface, innerRect);

            dtk_font_metrics fontMetrics;
            dtk_font_get_metrics(dtk_get_default_font(&pCmdList->pDred->tk), uiScale, &fontMetrics);

            dtk_int32 penPosX = 0;
            dtk_int32 penPosY = -dred_scrollbar_get_scroll_position(&pCmdList->scrollbar) * fontMetrics.lineHeight;
            for (size_t i = 0; i < pCmdList->commandIndexCount; ++i) {
                dtk_font* pFont = dtk_get_default_font(&pDred->tk);

                const char* text = g_CommandNames[pCmdList->pCommandIndices[i]];
                size_t textLength = strlen(text);

                float stringSizeX;
                float stringSizeY;
                dtk_font_measure_string(pFont, uiScale, text, textLength, &stringSizeX, &stringSizeY);

                dtk_surface_draw_text(pEvent->paint.pSurface, dtk_get_default_font(&pDred->tk), uiScale, text, textLength, penPosX, penPosY, dtk_rgb(0, 0, 0), pDred->config.cmdbarPopupBGColor);
                dtk_surface_draw_rect(pEvent->paint.pSurface, dtk_rect_init((dtk_int32)stringSizeX, penPosY, innerRect.right, penPosY + fontMetrics.lineHeight), pDred->config.cmdbarPopupBGColor);

                penPosY += fontMetrics.lineHeight;

                if (pCmdList->showOnlyFirstCommand) {
                    break;
                }
            }

            // The part of the background between the bottom of the text and the bottom of the main control.
            dtk_rect bgRect = innerRect;
            bgRect.top = penPosY;
            dtk_surface_draw_rect(pEvent->paint.pSurface, bgRect, pDred->config.cmdbarPopupBGColor);
        } break;

        case DTK_EVENT_SIZE:
        {
            dred_cmdbox_cmdlist__update_scrollbar(pCmdList);
            dred_cmdbox_cmdlist__refresh_layout(pCmdList);
        } break;

        case DTK_EVENT_MOUSE_WHEEL:
        {
            dred_scrollbar_scroll(&pCmdList->scrollbar, -pEvent->mouseWheel.delta);
        } break;

        default: break;
    }

    return dtk_control_default_event_handler(pEvent);
}

dred_result dred_cmdbox_cmdlist_init(dred_context* pDred, dtk_control* pParent, dred_cmdbox_cmdlist* pCmdList)
{
    if (pCmdList == NULL) return DRED_INVALID_ARGS;
    pCmdList->pDred = pDred;

    dtk_result result = dtk_control_init(&pDred->tk, pParent, DTK_CONTROL_TYPE_EMPTY, dred_cmbox_cmdlist_event_handler, DTK_CONTROL(pCmdList));
    if (result != DTK_SUCCESS) {
        return result;
    }

    if (!dred_scrollbar_init(&pCmdList->scrollbar, pDred, DTK_CONTROL(pCmdList), dred_scrollbar_orientation_vertical)) {
        return DTK_ERROR;
    }

    dred_scrollbar_set_on_scroll(&pCmdList->scrollbar, dred_cmdbox_cmdlist__on_scroll);

    return DTK_SUCCESS;
}

dred_result dred_cmdbox_cmdlist_uninit(dred_cmdbox_cmdlist* pCmdList)
{
    dtk_free(pCmdList->pCommandIndices);
    return dtk_control_uninit(DTK_CONTROL(pCmdList));
}

dred_result dred_cmdbox_cmdlist_update_list(dred_cmdbox_cmdlist* pCmdList, const char* runningText)
{
    if (pCmdList == NULL) return DRED_INVALID_ARGS;

    if (runningText == NULL) {
        runningText = "";
    }


    char commandName[256];
    const char* params = dr_next_token(runningText, commandName, sizeof(commandName));

    // If the user has typed the whole command we only want to show the first one.
    pCmdList->showOnlyFirstCommand = DR_FALSE;
    if (params != NULL && dr_is_whitespace(params[0])) {
        pCmdList->showOnlyFirstCommand = DR_TRUE;
    }

    size_t commandCount = dred_find_commands_starting_with(NULL, 0, commandName);
    if (commandCount > pCmdList->commandIndexCapacity) {
        size_t* pNewCommandIndices = (size_t*)dtk_realloc(pCmdList->pCommandIndices, commandCount * sizeof(*pNewCommandIndices));
        if (pNewCommandIndices == NULL) {
            return DTK_OUT_OF_MEMORY;
        }

        pCmdList->pCommandIndices = pNewCommandIndices;
        pCmdList->commandIndexCapacity = commandCount;
    }

    pCmdList->commandIndexCount = commandCount;
    dred_find_commands_starting_with(pCmdList->pCommandIndices, pCmdList->commandIndexCapacity, commandName);


    // A change in commands will change the structure of the scrollbar.
    dred_cmdbox_cmdlist__update_scrollbar(pCmdList);

    dtk_control_scheduled_redraw(DTK_CONTROL(pCmdList), dtk_control_get_local_rect(DTK_CONTROL(pCmdList)));
    return DTK_SUCCESS;
}
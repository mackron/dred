// Copyright (C) 2017 David Reid. See included LICENSE file.

dtk_rect dred_cmdbox_cmdlist__calculate_content_rect(dred_cmdbox_cmdlist* pCmdList)
{
    dtk_rect rect = dtk_control_get_local_rect(DTK_CONTROL(pCmdList));
    if (dtk_control_is_visible(DTK_CONTROL(&pCmdList->scrollbar))) {
        rect.right -= dtk_control_get_width(DTK_CONTROL(&pCmdList->scrollbar));
    }

    return rect;
}

dtk_bool32 dred_cmdbox_cmdlist__find_line_index_under_point(dred_cmdbox_cmdlist* pCmdList, dtk_int32 x, dtk_int32 y, dtk_uint32* pIndex)
{
    dtk_assert(pCmdList != NULL);
    dtk_assert(pIndex != NULL);

    dtk_rect contentRect = dred_cmdbox_cmdlist__calculate_content_rect(pCmdList);
    if (x < 0 || y < 0) {
        return DTK_FALSE;
    }
    if (x > contentRect.right || y > contentRect.bottom) {
        return DTK_FALSE;
    }

    float uiScale = dred_get_control_ui_scale(pCmdList->pDred, DTK_CONTROL(pCmdList));

    dtk_font_metrics fontMetrics;
    dtk_font_get_metrics(dtk_get_default_font(&pCmdList->pDred->tk), uiScale, &fontMetrics);

    dtk_uint32 lineIndexUnscrolled = y / fontMetrics.lineHeight;
    dtk_uint32 lineIndex = lineIndexUnscrolled + dred_scrollbar_get_scroll_position(&pCmdList->scrollbar);

    if (lineIndex >= pCmdList->commandIndexCount) {
        return DTK_FALSE;
    }

    *pIndex = lineIndex;
    return DTK_TRUE;
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
    dred_scrollbar_set_range_and_page_size(&pCmdList->scrollbar, 0, (dtk_int32)pCmdList->commandIndexCount, (dtk_int32)pageSize);
}

void dred_cmdbox_cmdlist__refresh_layout(dred_cmdbox_cmdlist* pCmdList)
{
    dtk_assert(pCmdList != NULL);

    float uiScale = dred_get_control_ui_scale(pCmdList->pDred, DTK_CONTROL(pCmdList));

    // Update the size of the scrollbar first.
    dtk_uint32 scrollbarSizeX = (dtk_uint32)(pCmdList->pDred->config.textEditorSBSize * uiScale);
    dtk_uint32 scrollbarSizeY = dtk_control_get_height(DTK_CONTROL(pCmdList));
    dtk_control_set_size(DTK_CONTROL(&pCmdList->scrollbar), scrollbarSizeX, scrollbarSizeY);

    // The scrollbar needs to be positioned on the right hand side.
    dtk_int32 scrollbarPosX = dtk_control_get_width(DTK_CONTROL(pCmdList)) - dtk_control_get_width(DTK_CONTROL(&pCmdList->scrollbar));
    dtk_int32 scrollbarPosY = 0;
    dtk_control_set_relative_position(DTK_CONTROL(&pCmdList->scrollbar), scrollbarPosX, scrollbarPosY);
}

void dred_cmdbox_cmdlist__scroll_to_highlighted_item(dred_cmdbox_cmdlist* pCmdList)
{
    dtk_assert(pCmdList != NULL);

    dtk_uint32 scrollPos = (dtk_uint32)dred_scrollbar_get_scroll_position(&pCmdList->scrollbar);
    dtk_uint32 pageSize = (dtk_uint32)dred_scrollbar_get_page_size(&pCmdList->scrollbar);

    if (pCmdList->selectedItemIndex < scrollPos) {
        dred_scrollbar_scroll_to(&pCmdList->scrollbar, pCmdList->selectedItemIndex);
    } else if (pCmdList->selectedItemIndex > scrollPos + (pageSize - 1)) {
        dred_scrollbar_scroll_to(&pCmdList->scrollbar, pCmdList->selectedItemIndex - pageSize + 1);
    }
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
            float uiScale = dred_get_control_ui_scale(pCmdList->pDred, DTK_CONTROL(pCmdList));;

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

                dtk_color bgColor = pDred->config.cmdbarPopupBGColor;
                if (pCmdList->selectedItemIndex == i) {
                    bgColor = dtk_rgb(192, 192, 192);   // TODO: Replace this with a config variable.
                }

                dtk_color fgColor = dtk_rgb(0, 0, 0);   // TODO: Replace this with a config variable.

                dtk_surface_draw_text(pEvent->paint.pSurface, dtk_get_default_font(&pDred->tk), uiScale, text, textLength, penPosX, penPosY, fgColor, bgColor);
                dtk_surface_draw_rect(pEvent->paint.pSurface, dtk_rect_init((dtk_int32)stringSizeX, penPosY, innerRect.right, penPosY + fontMetrics.lineHeight), bgColor);

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

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        {
            dtk_uint32 itemIndex;
            if (dred_cmdbox_cmdlist__find_line_index_under_point(pCmdList, pEvent->mouseButton.x, pEvent->mouseButton.y, &itemIndex)) {
                pCmdList->selectedItemIndex = itemIndex;
                dtk_control_scheduled_redraw(DTK_CONTROL(pCmdList), dtk_control_get_local_rect(DTK_CONTROL(pCmdList)));
            }
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DBLCLICK:
        {
            dtk_uint32 itemIndex;
            if (dred_cmdbox_cmdlist__find_line_index_under_point(pCmdList, pEvent->mouseButton.x, pEvent->mouseButton.y, &itemIndex)) {
                char* newText = dtk_make_stringf("%s ", g_CommandNames[pCmdList->pCommandIndices[itemIndex]]);
                dred_set_command_bar_text(pCmdList->pDred, newText);
                dred_cmdbox_cmdlist_update_list(pCmdList, newText);
                dtk_free_string(newText);

                dtk_control_scheduled_redraw(DTK_CONTROL(pCmdList), dtk_control_get_local_rect(DTK_CONTROL(pCmdList)));
            }
        } break;

        case DTK_EVENT_MOUSE_WHEEL:
        {
            dred_scrollbar_scroll(&pCmdList->scrollbar, -pEvent->mouseWheel.delta * dred_scrollbar_get_mouse_wheel_scale(&pCmdList->scrollbar));
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

    size_t oldSelectedCommandIndex = (size_t)-1;
    if (pCmdList->commandIndexCount > pCmdList->selectedItemIndex) {
        oldSelectedCommandIndex = pCmdList->pCommandIndices[pCmdList->selectedItemIndex];
    }


    char commandName[256];
    const char* params = dr_next_token(runningText, commandName, sizeof(commandName));

    // If the user has typed the whole command we only want to show the first one.
    pCmdList->showOnlyFirstCommand = DR_FALSE;
    if (params != NULL && dr_is_whitespace(params[0])) {
        pCmdList->showOnlyFirstCommand = DR_TRUE;
    }

    dtk_uint32 commandCount = (dtk_uint32)dred_find_commands_starting_with(NULL, 0, commandName);
    if (commandCount > pCmdList->commandIndexCapacity) {
        dtk_uint32* pNewCommandIndices = (dtk_uint32*)dtk_realloc(pCmdList->pCommandIndices, commandCount * sizeof(*pNewCommandIndices));
        if (pNewCommandIndices == NULL) {
            return DTK_OUT_OF_MEMORY;
        }

        pCmdList->pCommandIndices = pNewCommandIndices;
        pCmdList->commandIndexCapacity = commandCount;
    }

    pCmdList->commandIndexCount = commandCount;
    dred_find_commands_starting_with(pCmdList->pCommandIndices, pCmdList->commandIndexCapacity, commandName);

    pCmdList->selectedItemIndex = 0;
    if (oldSelectedCommandIndex != (size_t)-1 && pCmdList->commandIndexCount > 0) {
        for (dtk_uint32 i = 0; i < pCmdList->commandIndexCount; ++i) {
            if (pCmdList->pCommandIndices[i] == oldSelectedCommandIndex) {
                pCmdList->selectedItemIndex = i;
                break;
            }
        }
    }


    // A change in commands will change the structure of the scrollbar.
    dred_cmdbox_cmdlist__update_scrollbar(pCmdList);
    dred_cmdbox_cmdlist__scroll_to_highlighted_item(pCmdList);

    dtk_control_scheduled_redraw(DTK_CONTROL(pCmdList), dtk_control_get_local_rect(DTK_CONTROL(pCmdList)));
    return DTK_SUCCESS;
}

dred_result dred_cmdbox_cmdlist_highlight_next_item(dred_cmdbox_cmdlist* pCmdList)
{
    if (pCmdList == NULL) return DRED_INVALID_ARGS;

    pCmdList->selectedItemIndex = (pCmdList->selectedItemIndex + 1) % pCmdList->commandIndexCount;
    dred_cmdbox_cmdlist__scroll_to_highlighted_item(pCmdList);

    dtk_control_scheduled_redraw(DTK_CONTROL(pCmdList), dtk_control_get_local_rect(DTK_CONTROL(pCmdList)));
    return DTK_SUCCESS;
}

dred_result dred_cmdbox_cmdlist_highlight_prev_item(dred_cmdbox_cmdlist* pCmdList)
{
    if (pCmdList == NULL) return DRED_INVALID_ARGS;

    // Loop back to the start if necessary.
    if (pCmdList->selectedItemIndex > 0) {
        pCmdList->selectedItemIndex -= 1;
    } else {
        if (pCmdList->commandIndexCount > 0) {
            pCmdList->selectedItemIndex = pCmdList->commandIndexCount-1;
        } else {
            pCmdList->selectedItemIndex = 0;
        }
    }

    dred_cmdbox_cmdlist__scroll_to_highlighted_item(pCmdList);

    dtk_control_scheduled_redraw(DTK_CONTROL(pCmdList), dtk_control_get_local_rect(DTK_CONTROL(pCmdList)));
    return DTK_SUCCESS;
}

const char* dred_cmdbox_cmdlist_get_highlighted_command_name(dred_cmdbox_cmdlist* pCmdList)
{
    if (pCmdList == NULL) return NULL;

    if (pCmdList->selectedItemIndex < pCmdList->commandIndexCount) {
        return g_CommandNames[pCmdList->pCommandIndices[pCmdList->selectedItemIndex]];
    }

    return NULL;
}
// Copyright (C) 2017 David Reid. See included LICENSE file.

void dred_cmdbar_popup__refresh_cmdbox_layout(dred_cmdbar_popup* pCmdBarPopup, dtk_int32 popupSizeX, dtk_int32 popupSizeY)
{
    dred_context* pDred = pCmdBarPopup->pDred;
    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pCmdBarPopup));

    dtk_int32 padding     = (dtk_int32)(pDred->config.cmdbarPopupPadding     * uiScale);
    dtk_int32 borderWidth = (dtk_int32)(pDred->config.cmdbarPopupBorderWidth * uiScale);

    dtk_int32 cmdboxPosX = borderWidth + padding;
    dtk_int32 cmdboxPosY = borderWidth + padding;
    dtk_control_set_relative_position(DTK_CONTROL(&pCmdBarPopup->cmdlist), cmdboxPosX, cmdboxPosY);

    dtk_int32 cmdboxSizeX = (popupSizeX - (borderWidth + padding)*2);
    dtk_int32 cmdboxSizeY = (popupSizeY - (borderWidth + padding)*2);
    dtk_control_set_size(DTK_CONTROL(&pCmdBarPopup->cmdlist), cmdboxSizeX, cmdboxSizeY);
}

void dred_cmdbar_popup__on_size(dred_cmdbar_popup* pCmdBarPopup, dtk_int32 newWidth, dtk_int32 newHeight)
{
    dred_cmdbar_popup__refresh_cmdbox_layout(pCmdBarPopup, newWidth, newHeight);
}

void dred_cmdbar_popup__on_paint(dred_cmdbar_popup* pCmdBarPopup, dtk_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dred_context* pDred = pCmdBarPopup->pDred;
    assert(pDred != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pCmdBarPopup));

    dtk_rect popupRect = dtk_control_get_local_rect(DTK_CONTROL(pCmdBarPopup));
    dtk_surface_draw_rect_outline(pSurface, popupRect, pDred->config.cmdbarBGColorActive, (dtk_int32)(pDred->config.cmdbarPopupBorderWidth*uiScale));

    popupRect = dtk_rect_grow(popupRect, -(dtk_int32)(pDred->config.cmdbarPopupBorderWidth*uiScale));
    dtk_surface_draw_rect_outline(pSurface, popupRect, pDred->config.cmdbarPopupBGColor, (dtk_int32)(pDred->config.cmdbarPopupPadding*uiScale));
}

dtk_bool32 dred_cmdbar_popup_event_handler(dtk_event* pEvent)
{
    dred_cmdbar_popup* pCmdBarPopup = (dred_cmdbar_popup*)pEvent->pControl;

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dred_cmdbar_popup__on_paint(pCmdBarPopup, pEvent->paint.rect, pEvent->paint.pSurface);
        } break;

        case DTK_EVENT_SIZE:
        {
            dred_cmdbar_popup__on_size(pCmdBarPopup, pEvent->size.width, pEvent->size.height);
        } break;

        case DTK_EVENT_REFRESH_LAYOUT:
        {
            dtk_int32 sizeX;
            dtk_int32 sizeY;
            dtk_window_get_client_size(DTK_WINDOW(pCmdBarPopup), &sizeX, &sizeY);
            dred_cmdbar_popup__refresh_cmdbox_layout(pCmdBarPopup, sizeX, sizeY);
        } break;

        default: break;
    }

    return dtk_window_default_event_handler(pEvent);
}

dtk_result dred_cmdbar_popup_init(dred_context* pDred, dred_cmdbar_popup* pCmdBarPopup)
{
    if (pDred == NULL || pCmdBarPopup == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pCmdBarPopup);

    pCmdBarPopup->pDred = pDred;

    dtk_result result = dtk_window_init(&pDred->tk, dred_cmdbar_popup_event_handler, DTK_CONTROL(&pDred->mainWindow), dtk_window_type_popup, "", 0, 0, &pCmdBarPopup->window);
    if (result != DTK_SUCCESS) {
        return result;
    }

    result = dred_cmdbox_cmdlist_init(pDred, DTK_CONTROL(pCmdBarPopup), &pCmdBarPopup->cmdlist);
    if (result != DTK_SUCCESS) {
        dtk_window_uninit(&pCmdBarPopup->window);
        return result;
    }


    pCmdBarPopup->pFont = &pDred->config.cmdbarPopupFont->fontDTK;
    
    //pCmdBarPopup->pWindow->pUserData = pCmdBarPopup;
    //dred_control_set_on_size(pCmdBarPopup->pWindow->pRootGUIControl, dred_cmdbar_popup__on_size);
    //dred_control_set_on_paint(pCmdBarPopup->pWindow->pRootGUIControl, dred_cmdbar_popup__on_paint);

    return DTK_SUCCESS;
}

dtk_result dred_cmdbar_popup_uninit(dred_cmdbar_popup* pCmdBarPopup)
{
    if (pCmdBarPopup == NULL) return DTK_INVALID_ARGS;

    dtk_window_uninit(&pCmdBarPopup->window);
    return DTK_SUCCESS;
}

void dred_cmdbar_popup_show(dred_cmdbar_popup* pCmdBarPopup)
{
    dred_cmdbar* pCmdBar = &pCmdBarPopup->pDred->cmdBar;
    if (pCmdBar->manualTextEntry != NULL) {
        dred_cmdbar_popup_refresh_autocomplete(pCmdBarPopup, pCmdBar->manualTextEntry);
    } else {
        char* pCmdBarText = dred_cmdbar_get_text_malloc(&pCmdBarPopup->pDred->cmdBar);
        dred_cmdbar_popup_refresh_autocomplete(pCmdBarPopup, pCmdBarText);
        free(pCmdBarText);
    }

    dred_cmdbar_popup_refresh_position(pCmdBarPopup);
    dtk_window_show(&pCmdBarPopup->window, DTK_SHOW_NORMAL);
}

void dred_cmdbar_popup_hide(dred_cmdbar_popup* pCmdBarPopup)
{
    dtk_window_hide(&pCmdBarPopup->window);
}

void dred_cmdbar_popup_refresh_position(dred_cmdbar_popup* pCmdBarPopup)
{
    int mainWindowPosX;
    int mainWindowPosY;
    dtk_window_get_client_absolute_position(&pCmdBarPopup->pDred->mainWindow, &mainWindowPosX, &mainWindowPosY);

    dtk_int32 cmdbarPosX;
    dtk_int32 cmdbarPosY;
    dtk_control_get_absolute_position(DTK_CONTROL(&pCmdBarPopup->pDred->cmdBar), &cmdbarPosX, &cmdbarPosY);

    dtk_int32 popupSizeX;
    dtk_int32 popupSizeY;
    dtk_window_get_size(DTK_WINDOW(pCmdBarPopup), &popupSizeX, &popupSizeY);
    dtk_window_set_relative_position(DTK_WINDOW(pCmdBarPopup), (/*mainWindowPosX + */cmdbarPosX), (/*mainWindowPosY + */cmdbarPosY - popupSizeY));
}


void dred_cmdbar_popup_refresh_styling(dred_cmdbar_popup* pCmdBarPopup)
{
    if (pCmdBarPopup == NULL) return;

    dred_context* pDred = pCmdBarPopup->pDred;
    if (pDred == NULL) {
        return;
    }

    if (pCmdBarPopup->pFont) {
        pCmdBarPopup->pFont = NULL;
    }

    pCmdBarPopup->pFont = &pDred->config.cmdbarPopupFont->fontDTK;
    

    // The cmdbox control needs to be repositioned and resized based on the new padding and border sizes.
    dtk_int32 popupSizeX;
    dtk_int32 popupSizeY;
    dtk_control_get_size(DTK_CONTROL(pCmdBarPopup), &popupSizeX, &popupSizeY);
    dred_cmdbar_popup__refresh_cmdbox_layout(pCmdBarPopup, popupSizeX, popupSizeY);

    dtk_control_scheduled_redraw(DTK_CONTROL(pCmdBarPopup), dtk_control_get_local_rect(DTK_CONTROL(pCmdBarPopup)));
}


void dred_cmdbar_popup_refresh_autocomplete(dred_cmdbar_popup* pCmdBarPopup, const char* runningText)
{
    if (pCmdBarPopup == NULL) return;
    dred_cmdbox_cmdlist_update_list(&pCmdBarPopup->cmdlist, runningText);

    // Redraw?
    dtk_control_scheduled_redraw(DTK_CONTROL(pCmdBarPopup), dtk_control_get_local_rect(DTK_CONTROL(pCmdBarPopup)));
}
// Copyright (C) 2017 David Reid. See included LICENSE file.

void dred_cmdbar_popup__refresh_cmdbox_layout(dred_cmdbar_popup* pCmdBarPopup, dtk_uint32 popupSizeX, dtk_uint32 popupSizeY)
{
    dred_context* pDred = pCmdBarPopup->pDred;
    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pCmdBarPopup->pWindow));

    dtk_int32 padding     = (dtk_int32)(pDred->config.cmdbarPopupPadding     * uiScale);
    dtk_int32 borderWidth = (dtk_int32)(pDred->config.cmdbarPopupBorderWidth * uiScale);

    dtk_int32 cmdboxPosX = borderWidth + padding;
    dtk_int32 cmdboxPosY = borderWidth + padding;
    dtk_control_set_relative_position(DTK_CONTROL(&pCmdBarPopup->cmdlist), cmdboxPosX, cmdboxPosY);

    dtk_uint32 cmdboxSizeX = (dtk_uint32)((dtk_int32)popupSizeX - (borderWidth + padding)*2);
    dtk_uint32 cmdboxSizeY = (dtk_uint32)((dtk_int32)popupSizeY - (borderWidth + padding)*2);
    dtk_control_set_size(DTK_CONTROL(&pCmdBarPopup->cmdlist), cmdboxSizeX, cmdboxSizeY);
}

void dred_cmdbar_popup__on_size(dred_control* pControl, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    dred_window* pWindow = dred_get_control_window(pControl);
    if (pWindow == NULL) {
        return;
    }

    dred_cmdbar_popup* pCmdBarPopup = (dred_cmdbar_popup*)pWindow->pUserData;
    assert(pCmdBarPopup != NULL);

    dred_cmdbar_popup__refresh_cmdbox_layout(pCmdBarPopup, (dtk_uint32)newWidth, (dtk_uint32)newHeight);
}

void dred_cmdbar_popup__on_paint(dred_control* pControl, dred_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dred_window* pWindow = dred_get_control_window(pControl);
    if (pWindow == NULL) {
        return;
    }

    dred_context* pDred = pWindow->pDred;
    assert(pDred != NULL);

    //dred_cmdbar_popup* pCmdBarPopup = (dred_cmdbar_popup*)pWindow->pUserData;
    //assert(pCmdBarPopup != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pControl));

    dred_rect popupRect = dred_control_get_local_rect(pControl);
    dred_control_draw_rect_outline(pControl, popupRect, pDred->config.cmdbarBGColorActive, pDred->config.cmdbarPopupBorderWidth*uiScale, pSurface);

    popupRect = dred_grow_rect(popupRect, -pDred->config.cmdbarPopupBorderWidth*uiScale);
    dred_control_draw_rect_outline(pControl, popupRect, pDred->config.cmdbarPopupBGColor, pDred->config.cmdbarPopupPadding*uiScale, pSurface);

#if 0
    dred_rect innerRect = popupRect;
    innerRect = dred_grow_rect(innerRect, -pDred->config.cmdbarPopupBorderWidth*uiScale);
    innerRect = dred_grow_rect(innerRect, -pDred->config.cmdbarPopupPadding*uiScale);
    dred_control_set_clip(pControl, innerRect, pSurface);

    dtk_font_metrics fontMetrics;
    dred_gui_get_font_metrics(pCmdBarPopup->pFont, &fontMetrics);

    // TODO: Implement scrolling.
    float penPosX = (pDred->config.cmdbarPopupBorderWidth+pDred->config.cmdbarPopupPadding)*uiScale;
    float penPosY = (pDred->config.cmdbarPopupBorderWidth+pDred->config.cmdbarPopupPadding)*uiScale;
    for (size_t i = 0; i < pCmdBarPopup->commandIndexCount; ++i) {
        dred_control_draw_text(pControl, pCmdBarPopup->pFont, g_CommandNames[pCmdBarPopup->pCommandIndices[i]], -1, penPosX, penPosY, dtk_rgb(0, 0, 0), pDred->config.cmdbarPopupBGColor, pSurface);
        penPosY += fontMetrics.lineHeight;

        if (pCmdBarPopup->showOnlyFirstCommand) {
            break;
        }
    }
#endif
}

dred_cmdbar_popup* dred_cmdbar_popup_create(dred_context* pDred)
{
    if (pDred == NULL) return NULL;

    dred_cmdbar_popup* pCmdBarPopup = (dred_cmdbar_popup*)calloc(1, sizeof(*pCmdBarPopup));
    if (pCmdBarPopup == NULL) {
        return NULL;
    }

    pCmdBarPopup->pDred = pDred;

    pCmdBarPopup->pWindow = dred_window_create_popup(pDred->pMainWindow, 0, 0);
    if (pCmdBarPopup->pWindow == NULL) {
        free(pCmdBarPopup);
        return NULL;
    }

    dred_result result = dred_cmdbox_cmdlist_init(pDred, DTK_CONTROL(pCmdBarPopup->pWindow->pRootGUIControl), &pCmdBarPopup->cmdlist);
    if (result != DTK_SUCCESS) {
        dred_window_delete(pCmdBarPopup->pWindow);
        free(pCmdBarPopup);
        return NULL;
    }


    pCmdBarPopup->pFont = &pDred->config.cmdbarPopupFont->fontDTK;
    
    pCmdBarPopup->pWindow->pUserData = pCmdBarPopup;
    dred_control_set_on_size(pCmdBarPopup->pWindow->pRootGUIControl, dred_cmdbar_popup__on_size);
    dred_control_set_on_paint(pCmdBarPopup->pWindow->pRootGUIControl, dred_cmdbar_popup__on_paint);

    return pCmdBarPopup;
}

void dred_cmdbar_popup_delete(dred_cmdbar_popup* pCmdBarPopup)
{
    free(pCmdBarPopup);
}

void dred_cmdbar_popup_show(dred_cmdbar_popup* pCmdBarPopup)
{
    dred_cmdbar* pCmdBar = pCmdBarPopup->pDred->pCmdBar;
    if (pCmdBar->manualTextEntry != NULL) {
        dred_cmdbar_popup_refresh_autocomplete(pCmdBarPopup, pCmdBar->manualTextEntry);
    } else {
        char* pCmdBarText = dred_cmdbar_get_text_malloc(pCmdBarPopup->pDred->pCmdBar);
        dred_cmdbar_popup_refresh_autocomplete(pCmdBarPopup, pCmdBarText);
        free(pCmdBarText);
    }

    dred_cmdbar_popup_refresh_position(pCmdBarPopup);
    dred_window_show(pCmdBarPopup->pWindow);
}

void dred_cmdbar_popup_hide(dred_cmdbar_popup* pCmdBarPopup)
{
    dred_window_hide(pCmdBarPopup->pWindow);
}

void dred_cmdbar_popup_refresh_position(dred_cmdbar_popup* pCmdBarPopup)
{
    int mainWindowPosX;
    int mainWindowPosY;
    dred_window_get_client_position(pCmdBarPopup->pDred->pMainWindow, &mainWindowPosX, &mainWindowPosY);

    float cmdbarPosX;
    float cmdbarPosY;
    dred_control_get_absolute_position(DRED_CONTROL(pCmdBarPopup->pDred->pCmdBar), &cmdbarPosX, &cmdbarPosY);

    unsigned int popupSizeX;
    unsigned int popupSizeY;
    dtk_window_get_size(&pCmdBarPopup->pWindow->windowDTK, &popupSizeX, &popupSizeY);
    dtk_window_set_relative_position(&pCmdBarPopup->pWindow->windowDTK, (int)(/*mainWindowPosX + */cmdbarPosX), (int)(/*mainWindowPosY + */cmdbarPosY - popupSizeY));
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
    dtk_uint32 popupSizeX;
    dtk_uint32 popupSizeY;
    dtk_control_get_size(DTK_CONTROL(&pCmdBarPopup->pWindow->windowDTK), &popupSizeX, &popupSizeY);
    dred_cmdbar_popup__refresh_cmdbox_layout(pCmdBarPopup, popupSizeX, popupSizeY);

    // Redraw.
    dred_control_dirty(pCmdBarPopup->pWindow->pRootGUIControl, dred_control_get_local_rect(pCmdBarPopup->pWindow->pRootGUIControl));
}


void dred_cmdbar_popup_refresh_autocomplete(dred_cmdbar_popup* pCmdBarPopup, const char* runningText)
{
    if (pCmdBarPopup == NULL) return;
    dred_cmdbox_cmdlist_update_list(&pCmdBarPopup->cmdlist, runningText);


#if 0
    dred_context* pDred = pCmdBarPopup->pDred;
    if (pDred == NULL) {
        return;
    }

    char commandName[256];
    const char* params = dr_next_token(runningText, commandName, sizeof(commandName));

    // If the user has typed the whole command we only want to show the first one.
    pCmdBarPopup->showOnlyFirstCommand = DR_FALSE;
    if (params != NULL && dr_is_whitespace(params[0])) {
        pCmdBarPopup->showOnlyFirstCommand = DR_TRUE;
    }

    size_t commandCount = dred_find_commands_starting_with(NULL, 0, commandName);
    if (commandCount > pCmdBarPopup->commandIndexCapacity) {
        size_t* pNewCommandIndices = (size_t*)realloc(pCmdBarPopup->pCommandIndices, commandCount * sizeof(*pNewCommandIndices));
        if (pNewCommandIndices == NULL) {
            return; // Out of memory.
        }

        pCmdBarPopup->pCommandIndices = pNewCommandIndices;
        pCmdBarPopup->commandIndexCapacity = commandCount;
    }

    pCmdBarPopup->commandIndexCount = commandCount;
    dred_find_commands_starting_with(pCmdBarPopup->pCommandIndices, pCmdBarPopup->commandIndexCapacity, commandName);
#endif

    // Redraw?
    dred_control_dirty(pCmdBarPopup->pWindow->pRootGUIControl, dred_control_get_local_rect(pCmdBarPopup->pWindow->pRootGUIControl));
}
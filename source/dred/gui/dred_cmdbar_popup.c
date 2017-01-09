// Copyright (C) 2016 David Reid. See included LICENSE file.

void dred_cmdbar_popup__on_size(dred_control* pControl, float newWidth, float newHeight)
{
    dred_window* pWindow = dred_get_control_window(pControl);
    if (pWindow == NULL) {
        return;
    }

    // Do something...
}

void dred_cmdbar_popup__on_paint(dred_control* pControl, dred_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dred_window* pWindow = dred_get_control_window(pControl);
    if (pWindow == NULL) {
        return;
    }

    dred_cmdbar_popup* pCmdBarPopup = (dred_cmdbar_popup*)pWindow->pUserData;
    assert(pCmdBarPopup != NULL);

    float uiScale = (float)pWindow->pDred->uiScale;

    dred_rect popupRect = dred_control_get_local_rect(pControl);

    dred_control_draw_rect_with_outline(pControl, popupRect, dred_rgb(128, 128, 128), 2, pWindow->pDred->config.cmdbarBGColorActive, pSurface);
}

dred_cmdbar_popup* dred_cmdbar_popup_create(dred_context* pDred)
{
    if (pDred == NULL) return NULL;

    dred_cmdbar_popup* pCmdBarPopup = (dred_cmdbar_popup*)calloc(1, sizeof(*pCmdBarPopup));
    if (pCmdBarPopup == NULL) {
        return NULL;
    }

    pCmdBarPopup->pDred = pDred;

    unsigned int windowWidth = (unsigned int)(480*pDred->uiScale);
    unsigned int windowHeight = (unsigned int)(360*pDred->uiScale);

    pCmdBarPopup->pWindow = dred_window_create_popup(pDred->pMainWindow, windowWidth, windowHeight);
    if (pCmdBarPopup->pWindow == NULL) {
        free(pCmdBarPopup);
        return NULL;
    }
    
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
    dred_window_get_position(pCmdBarPopup->pDred->pMainWindow, &mainWindowPosX, &mainWindowPosY);

    float cmdbarPosX;
    float cmdbarPosY;
    dred_control_get_absolute_position(DRED_CONTROL(pCmdBarPopup->pDred->pCmdBar), &cmdbarPosX, &cmdbarPosY);

    unsigned int popupSizeX;
    unsigned int popupSizeY;
    dred_window_get_size(pCmdBarPopup->pWindow, &popupSizeX, &popupSizeY);
    dred_window_set_position(pCmdBarPopup->pWindow, mainWindowPosX + cmdbarPosX, mainWindowPosY + cmdbarPosY - popupSizeY);
}
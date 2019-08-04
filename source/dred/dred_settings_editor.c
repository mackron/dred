// Copyright (C) 2019 David Reid. See included LICENSE file.

#define DRED_SETTINGS_EDITOR_PAGE_GENERAL       0
#define DRED_SETTINGS_EDITOR_PAGE_THEME         1
#define DRED_SETTINGS_EDITOR_PAGE_TEXT_EDITOR   2

dtk_rect dred_settings_editor__get_side_panel_rect(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    return dtk_rect_init(0, 0, pSettingsEditor->sidePanelWidth, dtk_control_get_height(DTK_CONTROL(pSettingsEditor)));
}

dtk_rect dred_settings_editor__get_action_area_rect(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

    dtk_int32 posX = pSettingsEditor->sidePanelWidth;
    dtk_int32 posY = (dtk_int32)(dtk_control_get_height(DTK_CONTROL(pSettingsEditor)) - (dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->closeButton))) - (8*uiScale * 2));

    return dtk_rect_init(posX, posY, dtk_control_get_width(DTK_CONTROL(pSettingsEditor)), dtk_control_get_height(DTK_CONTROL(pSettingsEditor)));
}

int dred_settings_editor__get_side_panel_btn_index_under_point(dred_settings_editor* pSettingsEditor, float posX, float posY)
{
    assert(pSettingsEditor != NULL);

    if (posX < 0 || posX > pSettingsEditor->sidePanelWidth) {
        return -1;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

    dtk_font_metrics metrics;
    dtk_font_get_metrics(pSettingsEditor->pFont, uiScale, &metrics);

    dtk_int32 paddingY  = (dtk_int32)(metrics.lineHeight*pSettingsEditor->sidePanelBtnPaddingYRatio);
    dtk_int32 btnHeight = (dtk_int32)(paddingY*2 + metrics.lineHeight);
    dtk_int32 penPosY   = (dtk_int32)(pSettingsEditor->sidePanelBtnOffsetY);

    int sideButtonsCount = (int)(sizeof(pSettingsEditor->pages) / sizeof(pSettingsEditor->pages[0]));
    for (int i = 0; i < sideButtonsCount; ++i) {
        if (posY >= penPosY && posY < penPosY + btnHeight) {
            return i;
            break;
        }

        penPosY += btnHeight;
    }

    return -1;
}

void dred_settings_editor__select_page_by_index(dred_settings_editor* pSettingsEditor, int newPageIndex)
{
    assert(pSettingsEditor != NULL);

    int oldPageIndex = pSettingsEditor->selectedPageIndex;
    if (newPageIndex != oldPageIndex) {
        if (oldPageIndex != -1) {
            dtk_control_hide(&pSettingsEditor->pages[oldPageIndex].control);
        }
        if (newPageIndex != -1) {
            dtk_control_show(&pSettingsEditor->pages[newPageIndex].control);
        }

        pSettingsEditor->selectedPageIndex = newPageIndex;
        dtk_control_scheduled_redraw(DTK_CONTROL(pSettingsEditor), dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}


void dred_settings_editor__refresh_layout(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    // The close button needs to be auto-sized.
    dtk_control_refresh_layout(DTK_CONTROL(&pSettingsEditor->closeButton));

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

    dtk_int32 posX = pSettingsEditor->sidePanelWidth;
    dtk_int32 posY = 0;
    dtk_int32 sizeX = (dtk_int32)(dtk_control_get_width( DTK_CONTROL(pSettingsEditor)) - posX);
    dtk_int32 sizeY = (dtk_int32)(dtk_control_get_height(DTK_CONTROL(pSettingsEditor)) - (dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->closeButton))) - (8*uiScale * 2));

    // Every page needs to be resized. Every page will be the same size.
    int sideButtonsCount = (int)(sizeof(pSettingsEditor->pages) / sizeof(pSettingsEditor->pages[0]));
    for (int i = 0; i < sideButtonsCount; ++i) {
        dtk_control_set_relative_position(&pSettingsEditor->pages[i].control, posX, posY);
        dtk_control_set_size(&pSettingsEditor->pages[i].control, sizeX, sizeY);
        dtk_control_refresh_layout(&pSettingsEditor->pages[i].control);
    }

    dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->closeButton),
        (dtk_int32)(dtk_control_get_width (DTK_CONTROL(pSettingsEditor)) - dtk_control_get_width (DTK_CONTROL(&pSettingsEditor->closeButton)) - 8*uiScale),
        (dtk_int32)(dtk_control_get_height(DTK_CONTROL(pSettingsEditor)) - dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->closeButton)) - 8*uiScale));
}


void dred_settings_editor__on_size(dtk_control* pControl, dtk_int32 newWidth, dtk_int32 newHeight)
{
    (void)pControl;
    (void)newWidth;
    (void)newHeight;

    dred_settings_editor* pSettingsEditor = DRED_SETTINGS_EDITOR(pControl);
    assert(pSettingsEditor != NULL);

    //float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

    dred_settings_editor__refresh_layout(pSettingsEditor);

    //dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->closeButton),
    //    dred_control_get_width(DRED_CONTROL(pSettingsEditor)) - dred_control_get_width(DRED_CONTROL(&pSettingsEditor->closeButton)) - 8*uiScale,
    //    dred_control_get_height(DRED_CONTROL(pSettingsEditor)) - dred_control_get_height(DRED_CONTROL(&pSettingsEditor->closeButton)) - 8*uiScale);
    //
    
}

void dred_settings_editor__on_capture_keyboard(dtk_control* pControl, dtk_control* pPrevCapturedControl)
{
    (void)pControl;
    (void)pPrevCapturedControl;
}

void dred_settings_editor__on_mouse_button_down(dtk_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mouseButton;
    (void)stateFlags;

    dred_settings_editor* pSettingsEditor = DRED_SETTINGS_EDITOR(pControl);
    assert(pSettingsEditor != NULL);

    int newPageIndex = dred_settings_editor__get_side_panel_btn_index_under_point(pSettingsEditor, (float)mousePosX, (float)mousePosY);
    if (newPageIndex == -1) {
        return; // Don't change the selection if nothing was picked.
    }

    dred_settings_editor__select_page_by_index(pSettingsEditor, newPageIndex);
}

void dred_settings_editor__on_mouse_move(dtk_control* pControl, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_settings_editor* pSettingsEditor = DRED_SETTINGS_EDITOR(pControl);
    assert(pSettingsEditor != NULL);

    int hoveredPageIndex = dred_settings_editor__get_side_panel_btn_index_under_point(pSettingsEditor, (float)mousePosX, (float)mousePosY);
    if (hoveredPageIndex != pSettingsEditor->hoveredPageIndex) {
        pSettingsEditor->hoveredPageIndex = hoveredPageIndex;
        dtk_control_scheduled_redraw(DTK_CONTROL(pSettingsEditor), dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}

void dred_settings_editor__on_mouse_leave(dtk_control* pControl)
{
    dred_settings_editor* pSettingsEditor = DRED_SETTINGS_EDITOR(pControl);
    assert(pSettingsEditor != NULL);

    // None of the buttons will be hovered if the mouse leaves the window.
    if (pSettingsEditor->hoveredPageIndex != -1) {
        pSettingsEditor->hoveredPageIndex = -1;
        dtk_control_scheduled_redraw(DTK_CONTROL(pSettingsEditor), dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}

void dred_settings_editor__on_paint(dtk_control* pControl, dtk_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dred_settings_editor* pSettingsEditor = DRED_SETTINGS_EDITOR(pControl);
    assert(pSettingsEditor != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

    dtk_rect sideRect = dred_settings_editor__get_side_panel_rect(pSettingsEditor);

    // Side buttons.
    dtk_int32 penPosX = 0;
    dtk_int32 penPosY = pSettingsEditor->sidePanelBtnOffsetY;

    dtk_font_metrics metrics;
    dtk_font_get_metrics(pSettingsEditor->pFont, uiScale, &metrics);

    dtk_int32 paddingY = (dtk_int32)(metrics.lineHeight*pSettingsEditor->sidePanelBtnPaddingYRatio);
    dtk_int32 btnHeight = paddingY*2 + metrics.lineHeight;
    dtk_int32 selectedBtnPosY = 0;
    dtk_int32 borderWidth = (dtk_int32)(1*uiScale);

    int sideButtonsCount = (int)(sizeof(pSettingsEditor->pages) / sizeof(pSettingsEditor->pages[0]));
    for (int i = 0; i < sideButtonsCount; ++i) {
        dtk_color bgColor = dtk_rgb(255, 255, 255); //dtk_rgb(128, 32, 32);
        dtk_color borderColor = bgColor;
        if (pSettingsEditor->selectedPageIndex == i) {
            selectedBtnPosY = penPosY;
            borderColor = pSettingsEditor->sidePanelBtnBorderColor;
        } else if (pSettingsEditor->hoveredPageIndex == i) {
            bgColor = pSettingsEditor->sidePanelBtnGBColorHovered;
            borderColor = pSettingsEditor->sidePanelBtnGBColorHovered;
        }

        dtk_int32 textSizeX;
        dtk_int32 textSizeY;
        dtk_font_measure_string(pSettingsEditor->pFont, uiScale, pSettingsEditor->pages[i].title, strlen(pSettingsEditor->pages[i].title), &textSizeX, &textSizeY);

        dtk_int32 textPosX = (dtk_int32)(penPosX + 8*uiScale);
        dtk_int32 textPosY = (dtk_int32)(penPosY + paddingY);
        dtk_surface_draw_text(pSurface, pSettingsEditor->pFont, uiScale, pSettingsEditor->pages[i].title, (int)strlen(pSettingsEditor->pages[i].title), (dtk_int32)textPosX, (dtk_int32)(textPosY), pSettingsEditor->sidePanelBtnTextColor, bgColor);

        // Border.
        dtk_surface_draw_rect(pSurface, dtk_rect_init(penPosX, penPosY, pSettingsEditor->sidePanelWidth - borderWidth, penPosY + borderWidth), borderColor);
        dtk_surface_draw_rect(pSurface, dtk_rect_init(penPosX, penPosY + btnHeight - borderWidth, pSettingsEditor->sidePanelWidth - borderWidth, penPosY + btnHeight), borderColor);

        // Background.
        dtk_surface_draw_rect(pSurface, dtk_rect_init(0, penPosY + borderWidth, textPosX, penPosY + btnHeight - borderWidth), bgColor);
        dtk_surface_draw_rect(pSurface, dtk_rect_init(textPosX + textSizeX, penPosY + borderWidth, pSettingsEditor->sidePanelWidth - borderWidth, penPosY + btnHeight - borderWidth), bgColor);
        dtk_surface_draw_rect(pSurface, dtk_rect_init(textPosX, penPosY + borderWidth, textPosX + textSizeX, textPosY), bgColor);
        dtk_surface_draw_rect(pSurface, dtk_rect_init(textPosX, textPosY + textSizeY, textPosX + textSizeX, penPosY + btnHeight - borderWidth), bgColor);

        penPosY += btnHeight;
    }

    // Side border. This is done in two parts so that we can leave a little gap for the selected item.
    if (pSettingsEditor->selectedPageIndex != -1) {
        dtk_int32 gapPosY0 = (dtk_int32)(selectedBtnPosY + (1*uiScale));
        dtk_int32 gapPosY1 = (dtk_int32)(selectedBtnPosY - (1*uiScale) + btnHeight);

        dtk_rect sideBorderRect0 = dtk_rect_init(sideRect.right - borderWidth, sideRect.top, sideRect.right, gapPosY0);
        dtk_rect sideBorderRect1 = dtk_rect_init(sideRect.right - borderWidth, gapPosY0, sideRect.right, gapPosY1);
        dtk_rect sideBorderRect2 = dtk_rect_init(sideRect.right - borderWidth, gapPosY1, sideRect.right, sideRect.bottom);

        dtk_surface_draw_rect(pSurface, sideBorderRect0, pSettingsEditor->sidePanelBtnBorderColor);
        dtk_surface_draw_rect(pSurface, sideBorderRect1, dtk_rgb(255, 255, 255));
        dtk_surface_draw_rect(pSurface, sideBorderRect2, pSettingsEditor->sidePanelBtnBorderColor);
    } else {
        dtk_rect sideBorderRect0 = dtk_rect_init(sideRect.right - borderWidth, sideRect.top, sideRect.right, sideRect.bottom);
        dtk_surface_draw_rect(pSurface, sideBorderRect0, pSettingsEditor->sidePanelBtnBorderColor);
    }

    // Top of side panel.
    dtk_surface_draw_rect(pSurface, dtk_rect_init(0, 0, sideRect.right - borderWidth, pSettingsEditor->sidePanelBtnOffsetY), dtk_rgb(255, 255, 255));

    // Bottom of side panel.
    dtk_surface_draw_rect(pSurface, dtk_rect_init(0, penPosY, sideRect.right - borderWidth, sideRect.bottom), dtk_rgb(255, 255, 255));


    // Action area.
    dtk_rect actionRect = dred_settings_editor__get_action_area_rect(pSettingsEditor);
    dtk_surface_draw_rect(pSurface, actionRect, dtk_rgb(255, 255, 255));
}


void dred_settings__btn_close__on_pressed(dtk_button* pButton)
{
    dred_settings_dialog_hide(&dred_get_context_from_control(DTK_CONTROL(pButton))->settingsDialog);
}

void dred_settings__btn_choose_font__on_pressed(dtk_button* pButton)
{
    dtk_window* pWindow = dtk_control_get_window(DTK_CONTROL(pButton));
    if (pWindow == NULL) {
        return; // Should never hit this, but leaving here for sanity.
    }

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pButton));

    dred_font_desc fontDesc;
    if (dred_show_font_picker_dialog(pDred, pWindow, &pDred->config.pTextEditorFont->desc, &fontDesc) != DTK_DIALOG_RESULT_OK) {
        return;
    }

    char fontDescStr[256];
    dred_font_desc_to_string(&fontDesc, fontDescStr, sizeof(fontDescStr));
    
    dred_config_set(&pDred->config, "texteditor-font", fontDescStr, 0);
}


void dred_settings_editor_page__on_mouse_enter(dtk_control* pPageControl)
{
    dred_settings_editor* pSettingsEditor = DRED_SETTINGS_EDITOR(dtk_control_get_parent(pPageControl));

    assert(pSettingsEditor != NULL);

    // Clear the hovered state of any buttons on the side panel.
    if (pSettingsEditor->hoveredPageIndex != -1) {
        pSettingsEditor->hoveredPageIndex = -1;
        dtk_control_scheduled_redraw(DTK_CONTROL(pSettingsEditor), dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}

void dred_settings_editor_page__on_paint(dtk_control* pPageControl, dtk_rect rect, dtk_surface* pSurface)
{
    (void)rect;
    dtk_surface_draw_rect(pSurface, dtk_control_get_local_rect(pPageControl), dtk_rgb(255, 255, 255));
}

dtk_bool32 dred_settings_editor_page_event_handler(dtk_event* pEvent)
{
    switch (pEvent->type)
    {
        case DTK_EVENT_MOUSE_ENTER:
        {
            dred_settings_editor_page__on_mouse_enter(pEvent->pControl);
        } break;

        case DTK_EVENT_PAINT:
        {
            dred_settings_editor_page__on_paint(pEvent->pControl, pEvent->paint.rect, pEvent->paint.pSurface);
        } break;

        default: break;
    }

    return dtk_control_default_event_handler(pEvent);
}

dtk_bool32 dred_settings_editor__init_page(dred_settings_editor_page* pPage, dred_context* pDred, dtk_control* pParent, dtk_event_proc onEvent, const char* title)
{
    assert(pPage != NULL);
    assert(pDred != NULL);
    assert(pParent != NULL);
    assert(title != NULL);

    strcpy_s(pPage->title, sizeof(pPage->title), title);
    if (dtk_control_init(&pDred->tk, DTK_CONTROL_TYPE_EMPTY, (onEvent != NULL) ? onEvent : dred_settings_editor_page_event_handler, pParent, &pPage->control) != DTK_SUCCESS) {
        return DTK_FALSE;
    }

    dtk_control_hide(&pPage->control);

    return DTK_TRUE;
}


dtk_bool32 dred_settings_editor_general_page_event_handler(dtk_event* pEvent)
{
    dred_settings_editor* pSettingsEditor = (dred_settings_editor*)pEvent->pControl->pParent;

    switch (pEvent->type)
    {
        case DTK_EVENT_REFRESH_LAYOUT:
        {
            float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

            dtk_font_metrics fontMetrics;
            dtk_font_get_metrics(pSettingsEditor->pFont, uiScale, &fontMetrics);

            dtk_int32 penPosX = (dtk_int32)(8*uiScale);
            dtk_int32 penPosY = (dtk_int32)(8*uiScale);

            dtk_checkbox_set_padding(DTK_CHECKBOX(&pSettingsEditor->cbShowTabBar), (dtk_int32)(4*uiScale));
            dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->cbShowTabBar), penPosX, penPosY);
            penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->cbShowTabBar)) + (6*uiScale));
            dtk_checkbox_auto_size(DTK_CHECKBOX(&pSettingsEditor->cbShowTabBar));

            dtk_checkbox_set_padding(DTK_CHECKBOX(&pSettingsEditor->cbShowMenuBar), (dtk_int32)(4*uiScale));
            dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->cbShowMenuBar), penPosX, penPosY);
            penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->cbShowMenuBar)) + (6*uiScale));
            dtk_checkbox_auto_size(DTK_CHECKBOX(&pSettingsEditor->cbShowMenuBar));

            dtk_checkbox_set_padding(DTK_CHECKBOX(&pSettingsEditor->cbAutoHideCmdBar), (dtk_int32)(4*uiScale));
            dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->cbAutoHideCmdBar), penPosX, penPosY);
            penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->cbAutoHideCmdBar)) + (6*uiScale));
            dtk_checkbox_auto_size(DTK_CHECKBOX(&pSettingsEditor->cbAutoHideCmdBar));
        } break;
    }

    return dred_settings_editor_page_event_handler(pEvent);
}

dtk_bool32 dred_settings_editor__init_page__general(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pSettingsEditor));
    assert(pSettingsEditor != NULL);

    dred_settings_editor_page* pPage = &pSettingsEditor->pages[DRED_SETTINGS_EDITOR_PAGE_GENERAL];

    if (!dred_settings_editor__init_page(pPage, pDred, DTK_CONTROL(pSettingsEditor), dred_settings_editor_general_page_event_handler, "General")) {
        return DTK_FALSE;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

    dtk_font_metrics fontMetrics;
    dtk_font_get_metrics(pSettingsEditor->pFont, uiScale, &fontMetrics);

    float penPosX = 8*uiScale;
    float penPosY = 8*uiScale;

    dtk_checkbox_init(&pDred->tk, NULL, &pPage->control, "Show Tab Bar", pDred->config.showTabBar, &pSettingsEditor->cbShowTabBar);
    dtk_control_bind(DTK_CONTROL(&pSettingsEditor->cbShowTabBar), DTK_BIND_TARGET_CHECKED, "config.show-tab-bar");
    dtk_checkbox_set_padding(DTK_CHECKBOX(&pSettingsEditor->cbShowTabBar), (dtk_int32)(4*uiScale));
    dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->cbShowTabBar), (dtk_int32)penPosX, (dtk_int32)penPosY);
    penPosY += dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->cbShowTabBar)) + (6*uiScale);

    dtk_checkbox_init(&pDred->tk, NULL, &pPage->control, "Show Menu Bar", pDred->config.showMenuBar, &pSettingsEditor->cbShowMenuBar);
    dtk_control_bind(DTK_CONTROL(&pSettingsEditor->cbShowMenuBar), DTK_BIND_TARGET_CHECKED, "config.show-menu-bar");
    dtk_checkbox_set_padding(DTK_CHECKBOX(&pSettingsEditor->cbShowMenuBar), (dtk_int32)(4*uiScale));
    dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->cbShowMenuBar), (dtk_int32)penPosX, (dtk_int32)penPosY);
    penPosY += dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->cbShowMenuBar)) + (6*uiScale);

    dtk_checkbox_init(&pDred->tk, NULL, &pPage->control, "Auto-hide Command Bar", pDred->config.autoHideCmdBar, &pSettingsEditor->cbAutoHideCmdBar);
    dtk_control_bind(DTK_CONTROL(&pSettingsEditor->cbAutoHideCmdBar), DTK_BIND_TARGET_CHECKED, "config.auto-hide-cmd-bar");
    dtk_checkbox_set_padding(DTK_CHECKBOX(&pSettingsEditor->cbAutoHideCmdBar), (dtk_int32)(4*uiScale));
    dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->cbAutoHideCmdBar), (dtk_int32)penPosX, (dtk_int32)penPosY);
    penPosY += dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->cbAutoHideCmdBar)) + (6*uiScale);

    return DTK_TRUE;
}


dtk_bool32 dred_settings_editor_theme_page_event_handler(dtk_event* pEvent)
{
    dred_settings_editor* pSettingsEditor = (dred_settings_editor*)pEvent->pControl->pParent;

    switch (pEvent->type)
    {
        case DTK_EVENT_REFRESH_LAYOUT:
        {
            float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

            dtk_font_metrics fontMetrics;
            dtk_font_get_metrics(pSettingsEditor->pFont, uiScale, &fontMetrics);

            dtk_int32 penPosX = (dtk_int32)(8*uiScale);
            dtk_int32 penPosY = (dtk_int32)(8*uiScale);

            dtk_button_set_padding(&pSettingsEditor->fontButton, (dtk_int32)(16*uiScale), (dtk_int32)(6*uiScale));
            dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->fontButton), penPosX, penPosY);
            penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->fontButton)) + (6*uiScale));

            dtk_color_button_set_padding(DTK_COLOR_BUTTON(&pSettingsEditor->textColorButton), (dtk_int32)(4*uiScale));
            dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->textColorButton), penPosX, penPosY);
            penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->textColorButton)) + (6*uiScale));
            dtk_color_button_auto_size(DTK_COLOR_BUTTON(&pSettingsEditor->textColorButton));

            dtk_color_button_set_padding(DTK_COLOR_BUTTON(&pSettingsEditor->bgColorButton), (dtk_int32)(4*uiScale));
            dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->bgColorButton), penPosX, penPosY);
            penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->bgColorButton)) + (6*uiScale));
            dtk_color_button_auto_size(DTK_COLOR_BUTTON(&pSettingsEditor->bgColorButton));

            dtk_color_button_set_padding(DTK_COLOR_BUTTON(&pSettingsEditor->lineColorButton), (dtk_int32)(4*uiScale));
            dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->lineColorButton), penPosX, penPosY);
            penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->lineColorButton)) + (6*uiScale));
            dtk_color_button_auto_size(DTK_COLOR_BUTTON(&pSettingsEditor->lineColorButton));
        } break;
    }

    return dred_settings_editor_page_event_handler(pEvent);
}

dtk_bool32 dred_settings_editor__init_page__theme(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pSettingsEditor));
    assert(pSettingsEditor != NULL);

    dred_settings_editor_page* pPage = &pSettingsEditor->pages[DRED_SETTINGS_EDITOR_PAGE_THEME];

    if (!dred_settings_editor__init_page(pPage, pDred, DTK_CONTROL(pSettingsEditor), dred_settings_editor_theme_page_event_handler, "Theme")) {
        return DTK_FALSE;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

    dtk_font_metrics fontMetrics;
    dtk_font_get_metrics(pSettingsEditor->pFont, uiScale, &fontMetrics);

    dtk_int32 penPosX = (dtk_int32)(8*uiScale);
    dtk_int32 penPosY = (dtk_int32)(8*uiScale);

    if (dtk_button_init(&pDred->tk, NULL, &pPage->control, "Choose Font...", &pSettingsEditor->fontButton) != DTK_SUCCESS) {
        dtk_control_uninit(&pPage->control);
        return DTK_FALSE;
    }
    dtk_button_set_on_pressed(&pSettingsEditor->fontButton, dred_settings__btn_choose_font__on_pressed);
    dtk_button_set_padding(&pSettingsEditor->fontButton, (dtk_int32)(16*uiScale), (dtk_int32)(6*uiScale));
    dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->fontButton), (dtk_int32)penPosX, (dtk_int32)penPosY);
    penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->fontButton)) + (6*uiScale));


    if (dtk_color_button_init(&pDred->tk, NULL, &pPage->control, "Text color", pDred->config.textEditorTextColor, &pSettingsEditor->textColorButton) != DTK_SUCCESS) {
        dtk_control_uninit(&pPage->control);
        return DTK_FALSE;
    }
    dtk_control_bind(DTK_CONTROL(&pSettingsEditor->textColorButton), DTK_BIND_TARGET_VALUE, "config.texteditor-text-color");
    dtk_color_button_set_padding(DTK_COLOR_BUTTON(&pSettingsEditor->textColorButton), (dtk_int32)(4*uiScale));
    dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->textColorButton), (dtk_int32)penPosX, (dtk_int32)penPosY);
    penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->textColorButton)) + (6*uiScale));


    if (dtk_color_button_init(&pDred->tk, NULL, &pPage->control, "Background color", pDred->config.textEditorBGColor, &pSettingsEditor->bgColorButton) != DTK_SUCCESS) {
        dtk_control_uninit(&pPage->control);
        return DTK_FALSE;
    }
    dtk_control_bind(DTK_CONTROL(&pSettingsEditor->bgColorButton), DTK_BIND_TARGET_VALUE, "config.texteditor-bg-color");
    dtk_color_button_set_padding(DTK_COLOR_BUTTON(&pSettingsEditor->bgColorButton), (dtk_int32)(4*uiScale));
    dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->bgColorButton), (dtk_int32)penPosX, (dtk_int32)penPosY);
    penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->textColorButton)) + (6*uiScale));


    if (dtk_color_button_init(&pDred->tk, NULL, &pPage->control, "Active line color", pDred->config.textEditorActiveLineColor, &pSettingsEditor->lineColorButton) != DTK_SUCCESS) {
        dtk_control_uninit(&pPage->control);
        return DTK_FALSE;
    }
    dtk_control_bind(DTK_CONTROL(&pSettingsEditor->lineColorButton), DTK_BIND_TARGET_VALUE, "config.texteditor-active-line-color");
    dtk_color_button_set_padding(DTK_COLOR_BUTTON(&pSettingsEditor->lineColorButton), (dtk_int32)(4*uiScale));
    dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->lineColorButton), (dtk_int32)penPosX, (dtk_int32)penPosY);
    penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->textColorButton)) + (6*uiScale));

    return DTK_TRUE;
}


dtk_bool32 dred_settings_editor_text_editor_page_event_handler(dtk_event* pEvent)
{
    dred_settings_editor* pSettingsEditor = (dred_settings_editor*)pEvent->pControl->pParent;

    switch (pEvent->type)
    {
        case DTK_EVENT_REFRESH_LAYOUT:
        {
            float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

            dtk_font_metrics fontMetrics;
            dtk_font_get_metrics(pSettingsEditor->pFont, uiScale, &fontMetrics);

            dtk_int32 penPosX = (dtk_int32)(8*uiScale);
            dtk_int32 penPosY = (dtk_int32)(8*uiScale);

            dtk_checkbox_set_padding(DTK_CHECKBOX(&pSettingsEditor->cbTabsToSpaces), (dtk_int32)(4*uiScale));
            dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->cbTabsToSpaces), penPosX, penPosY);
            penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->cbTabsToSpaces)) + (6*uiScale));
            dtk_checkbox_auto_size(DTK_CHECKBOX(&pSettingsEditor->cbTabsToSpaces));

            dtk_checkbox_set_padding(DTK_CHECKBOX(&pSettingsEditor->cbShowLineNumbers), (dtk_int32)(4*uiScale));
            dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->cbShowLineNumbers), penPosX, penPosY);
            penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->cbShowLineNumbers)) + (6*uiScale));
            dtk_checkbox_auto_size(DTK_CHECKBOX(&pSettingsEditor->cbShowLineNumbers));
        } break;
    }

    return dred_settings_editor_page_event_handler(pEvent);
}

dtk_bool32 dred_settings_editor__init_page__text_editor(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pSettingsEditor));
    assert(pSettingsEditor != NULL);

    dred_settings_editor_page* pPage = &pSettingsEditor->pages[DRED_SETTINGS_EDITOR_PAGE_TEXT_EDITOR];

    if (!dred_settings_editor__init_page(pPage, pDred, DTK_CONTROL(pSettingsEditor), dred_settings_editor_text_editor_page_event_handler, "Text Editor")) {
        return DTK_FALSE;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

    dtk_font_metrics fontMetrics;
    dtk_font_get_metrics(pSettingsEditor->pFont, uiScale, &fontMetrics);

    dtk_int32 penPosX = (dtk_int32)(8*uiScale);
    dtk_int32 penPosY = (dtk_int32)(8*uiScale);

    dtk_checkbox_init(&pDred->tk, NULL, &pPage->control, "Convert tabs to spaces", pDred->config.textEditorTabsToSpacesEnabled, &pSettingsEditor->cbTabsToSpaces);
    dtk_control_bind(DTK_CONTROL(&pSettingsEditor->cbTabsToSpaces), DTK_BIND_TARGET_CHECKED, "config.texteditor-enable-tabs-to-spaces");
    dtk_checkbox_set_padding(DTK_CHECKBOX(&pSettingsEditor->cbTabsToSpaces), (dtk_int32)(4*uiScale));
    dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->cbTabsToSpaces), (dtk_int32)penPosX, (dtk_int32)penPosY);
    penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->cbTabsToSpaces)) + (6*uiScale));

    dtk_checkbox_init(&pDred->tk, NULL, &pPage->control, "Show line numbers", pDred->config.textEditorShowLineNumbers, &pSettingsEditor->cbShowLineNumbers);
    dtk_control_bind(DTK_CONTROL(&pSettingsEditor->cbShowLineNumbers), DTK_BIND_TARGET_CHECKED, "config.texteditor-show-line-numbers");
    dtk_checkbox_set_padding(DTK_CHECKBOX(&pSettingsEditor->cbShowLineNumbers), (dtk_int32)(4*uiScale));
    dtk_control_set_relative_position(DTK_CONTROL(&pSettingsEditor->cbShowLineNumbers), (dtk_int32)penPosX, (dtk_int32)penPosY);
    penPosY += (dtk_int32)(dtk_control_get_height(DTK_CONTROL(&pSettingsEditor->cbShowLineNumbers)) + (6*uiScale));
    
    return DTK_TRUE;
}

dtk_bool32 dred_settings_editor_default_event_handler(dtk_event* pEvent)
{
    switch (pEvent->type)
    {
        case DTK_EVENT_SIZE:
        {
            dred_settings_editor__on_size(pEvent->pControl, pEvent->size.width, pEvent->size.height);
        } break;

        case DTK_EVENT_CAPTURE_KEYBOARD:
        {
            dred_settings_editor__on_capture_keyboard(pEvent->pControl, pEvent->captureKeyboard.pOldCapturedControl);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        {
            dred_settings_editor__on_mouse_button_down(pEvent->pControl, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            dred_settings_editor__on_mouse_move(pEvent->pControl, pEvent->mouseMove.x, pEvent->mouseMove.y, pEvent->mouseMove.state);
        } break;

        case DTK_EVENT_MOUSE_LEAVE:
        {
            dred_settings_editor__on_mouse_leave(pEvent->pControl);
        } break;

        case DTK_EVENT_PAINT:
        {
            dred_settings_editor__on_paint(pEvent->pControl, pEvent->paint.rect, pEvent->paint.pSurface);
        } break;

        default: break;
    }

    return dtk_control_default_event_handler(pEvent);
}

dred_settings_editor* dred_settings_editor_create(dred_context* pDred, dtk_control* pParent, const char* filePathAbsolute)
{
    dred_settings_editor* pSettingsEditor = (dred_settings_editor*)calloc(1, sizeof(*pSettingsEditor));
    if (pSettingsEditor == NULL) {
        return NULL;
    }

    if (!dred_editor_init(pDred, DRED_CONTROL_TYPE_SETTINGS_EDITOR, dred_settings_editor_default_event_handler, pParent, 0, 0, filePathAbsolute, DRED_EDITOR(pSettingsEditor))) {
        free(pSettingsEditor);
        return NULL;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));

    pSettingsEditor->pFont = &pDred->config.pUIFont->fontDTK;
    pSettingsEditor->sidePanelWidth = (dtk_int32)(200*uiScale);
    pSettingsEditor->sidePanelBtnOffsetY = (dtk_int32)(8.0f*uiScale);
    pSettingsEditor->sidePanelBtnPaddingYRatio = 0.75f;
    pSettingsEditor->sidePanelBtnTextColor = dtk_rgb(0, 0, 0);
    pSettingsEditor->sidePanelBtnBorderColor = dtk_rgb(160, 160, 160);
    pSettingsEditor->sidePanelBtnGBColorHovered = dtk_rgb(200, 224, 255);
    pSettingsEditor->selectedPageIndex = -1;
    pSettingsEditor->hoveredPageIndex = -1;

    dred_settings_editor__init_page__general(pSettingsEditor);
    dred_settings_editor__init_page__theme(pSettingsEditor);
    dred_settings_editor__init_page__text_editor(pSettingsEditor);

    if (dtk_button_init(&pDred->tk, NULL, DTK_CONTROL(pSettingsEditor), "Close", &pSettingsEditor->closeButton) != DTK_SUCCESS) {
        dred_editor_uninit(DRED_EDITOR(pSettingsEditor));
        return NULL;
    }

    dtk_button_set_on_pressed(&pSettingsEditor->closeButton, dred_settings__btn_close__on_pressed);
    dtk_button_set_padding(&pSettingsEditor->closeButton, 32, 6);

    // Select the General tab by default.
    dred_settings_editor__select_page_by_index(pSettingsEditor, DRED_SETTINGS_EDITOR_PAGE_GENERAL);


    return pSettingsEditor;
}

void dred_settings_editor_delete(dred_settings_editor* pSettingsEditor)
{
    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pSettingsEditor));
    if (pDred == NULL) {
        return;
    }

    dred_editor_uninit(DRED_EDITOR(pSettingsEditor));
}


void dred_settings_editor_refresh_styling(dred_settings_editor* pSettingsEditor)
{
    if (pSettingsEditor == NULL) {
        return;
    }

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pSettingsEditor));
    if (pDred == NULL) {
        return;
    }

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pSettingsEditor));


    pSettingsEditor->pFont = &pDred->config.pUIFont->fontDTK;

    pSettingsEditor->sidePanelWidth = (dtk_int32)(200*uiScale);
    pSettingsEditor->sidePanelBtnOffsetY = (dtk_int32)(8.0f*uiScale);

    dtk_control_scheduled_redraw(DTK_CONTROL(pSettingsEditor), dtk_control_get_local_rect(DTK_CONTROL(pSettingsEditor)));
}
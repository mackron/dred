// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_SETTINGS_EDITOR_PAGE_GENERAL       0
#define DRED_SETTINGS_EDITOR_PAGE_THEME         1
#define DRED_SETTINGS_EDITOR_PAGE_TEXT_EDITOR   2

dred_rect dred_settings_editor__get_side_panel_rect(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    return dred_make_rect(0, 0, pSettingsEditor->sidePanelWidth, dred_control_get_height(DRED_CONTROL(pSettingsEditor)));
}

dred_rect dred_settings_editor__get_action_area_rect(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pSettingsEditor));
    assert(pSettingsEditor != NULL);

    float posX = pSettingsEditor->sidePanelWidth;
    float posY = dred_control_get_height(DRED_CONTROL(pSettingsEditor)) - (dred_control_get_height(DRED_CONTROL(&pSettingsEditor->closeButton))) - (8*pDred->uiScale * 2);

    return dred_make_rect(posX, posY, dred_control_get_width(DRED_CONTROL(pSettingsEditor)), dred_control_get_height(DRED_CONTROL(pSettingsEditor)));
}

int dred_settings_editor__get_side_panel_btn_index_under_point(dred_settings_editor* pSettingsEditor, float posX, float posY)
{
    assert(pSettingsEditor != NULL);

    if (posX < 0 || posX > pSettingsEditor->sidePanelWidth) {
        return -1;
    }


    dred_gui_font_metrics metrics;
    dred_gui_get_font_metrics(pSettingsEditor->pFont, &metrics);

    float paddingY = metrics.lineHeight*pSettingsEditor->sidePanelBtnPaddingYRatio;
    float btnHeight = paddingY*2 + metrics.lineHeight;
    float penPosY = pSettingsEditor->sidePanelBtnOffsetY;    

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
            dred_control_hide(pSettingsEditor->pages[oldPageIndex].pGUIControl);
        }
        if (newPageIndex != -1) {
            dred_control_show(pSettingsEditor->pages[newPageIndex].pGUIControl);
        }

        pSettingsEditor->selectedPageIndex = newPageIndex;
        dred_control_dirty(DRED_CONTROL(pSettingsEditor), dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}


void dred_settings_editor__refresh_layout(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pSettingsEditor));
    assert(pSettingsEditor != NULL);

    float posX = pSettingsEditor->sidePanelWidth;
    float posY = 0;
    float sizeX = dred_control_get_width(DRED_CONTROL(pSettingsEditor)) - posX;
    float sizeY = dred_control_get_height(DRED_CONTROL(pSettingsEditor)) - (dred_control_get_height(DRED_CONTROL(&pSettingsEditor->closeButton))) - (8*pDred->uiScale * 2);

    // Every page needs to be resized. Every page will be the same size.
    int sideButtonsCount = (int)(sizeof(pSettingsEditor->pages) / sizeof(pSettingsEditor->pages[0]));
    for (int i = 0; i < sideButtonsCount; ++i) {
        dred_control_set_relative_position(pSettingsEditor->pages[i].pGUIControl, posX, posY);
        dred_control_set_size(pSettingsEditor->pages[i].pGUIControl, sizeX, sizeY);
    }
}


void dred_settings_editor__on_size(dred_control* pControl, float newWidth, float newHeight)
{
    (void)pControl;
    (void)newWidth;
    (void)newHeight;

    dred_settings_editor* pSettingsEditor = DRED_SETTINGS_EDITOR(pControl);
    assert(pSettingsEditor != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pSettingsEditor));

    dred_settings_editor__refresh_layout(pSettingsEditor);

    dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->closeButton),
        dred_control_get_width(DRED_CONTROL(pSettingsEditor)) - dred_control_get_width(DRED_CONTROL(&pSettingsEditor->closeButton)) - 8*pDred->uiScale,
        dred_control_get_height(DRED_CONTROL(pSettingsEditor)) - dred_control_get_height(DRED_CONTROL(&pSettingsEditor->closeButton)) - 8*pDred->uiScale);

    
}

void dred_settings_editor__on_capture_keyboard(dred_control* pControl, dred_control* pPrevCapturedControl)
{
    (void)pControl;
    (void)pPrevCapturedControl;
}

void dred_settings_editor__on_mouse_button_down(dred_control* pControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
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

void dred_settings_editor__on_mouse_move(dred_control* pControl, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_settings_editor* pSettingsEditor = DRED_SETTINGS_EDITOR(pControl);
    assert(pSettingsEditor != NULL);

    int hoveredPageIndex = dred_settings_editor__get_side_panel_btn_index_under_point(pSettingsEditor, (float)mousePosX, (float)mousePosY);
    if (hoveredPageIndex != pSettingsEditor->hoveredPageIndex) {
        pSettingsEditor->hoveredPageIndex = hoveredPageIndex;
        dred_control_dirty(DRED_CONTROL(pSettingsEditor), dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}

void dred_settings_editor__on_mouse_leave(dred_control* pControl)
{
    dred_settings_editor* pSettingsEditor = DRED_SETTINGS_EDITOR(pControl);
    assert(pSettingsEditor != NULL);

    // None of the buttons will be hovered if the mouse leaves the window.
    if (pSettingsEditor->hoveredPageIndex != -1) {
        pSettingsEditor->hoveredPageIndex = -1;
        dred_control_dirty(DRED_CONTROL(pSettingsEditor), dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}

void dred_settings_editor__on_paint(dred_control* pControl, dred_rect rect, void* pPaintData)
{
    (void)rect;

    dred_settings_editor* pSettingsEditor = DRED_SETTINGS_EDITOR(pControl);
    assert(pSettingsEditor != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pSettingsEditor));
    assert(pDred != NULL);

    dred_rect sideRect = dred_settings_editor__get_side_panel_rect(pSettingsEditor);

    // Side buttons.
    float penPosX = 0;
    float penPosY = pSettingsEditor->sidePanelBtnOffsetY;

    dred_gui_font_metrics metrics;
    dred_gui_get_font_metrics(pSettingsEditor->pFont, &metrics);

    float paddingY = metrics.lineHeight*pSettingsEditor->sidePanelBtnPaddingYRatio;
    float btnHeight = paddingY*2 + metrics.lineHeight;
    float selectedBtnPosY = 0;
    float borderWidth = 1*pDred->uiScale;

    int sideButtonsCount = (int)(sizeof(pSettingsEditor->pages) / sizeof(pSettingsEditor->pages[0]));
    for (int i = 0; i < sideButtonsCount; ++i) {
        dred_color bgColor = dred_rgb(255, 255, 255); //dred_rgb(128, 32, 32);
        dred_color borderColor = bgColor;
        if (pSettingsEditor->selectedPageIndex == i) {
            selectedBtnPosY = penPosY;
            borderColor = pSettingsEditor->sidePanelBtnBorderColor;
        } else if (pSettingsEditor->hoveredPageIndex == i) {
            bgColor = pSettingsEditor->sidePanelBtnGBColorHovered;
            borderColor = pSettingsEditor->sidePanelBtnGBColorHovered;
        }

        float textSizeX;
        float textSizeY;
        dred_gui_measure_string(pSettingsEditor->pFont, pSettingsEditor->pages[i].title, strlen(pSettingsEditor->pages[i].title), &textSizeX, &textSizeY);

        float textPosX = penPosX + 8*pDred->uiScale;
        float textPosY = penPosY + paddingY;
        dred_control_draw_text(DRED_CONTROL(pSettingsEditor), pSettingsEditor->pFont, pSettingsEditor->pages[i].title, (int)strlen(pSettingsEditor->pages[i].title), textPosX, textPosY, pSettingsEditor->sidePanelBtnTextColor, bgColor, pPaintData);

        // Border.
        dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), dred_make_rect(penPosX, penPosY, pSettingsEditor->sidePanelWidth - borderWidth, penPosY + borderWidth), borderColor, pPaintData);
        dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), dred_make_rect(penPosX, penPosY + btnHeight - borderWidth, pSettingsEditor->sidePanelWidth - borderWidth, penPosY + btnHeight), borderColor, pPaintData);

        // Background.
        dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), dred_make_rect(0, penPosY + borderWidth, textPosX, penPosY + btnHeight - borderWidth), bgColor, pPaintData);
        dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), dred_make_rect(textPosX + textSizeX, penPosY + borderWidth, pSettingsEditor->sidePanelWidth - borderWidth, penPosY + btnHeight - borderWidth), bgColor, pPaintData);
        dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), dred_make_rect(textPosX, penPosY + borderWidth, textPosX + textSizeX, textPosY), bgColor, pPaintData);
        dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), dred_make_rect(textPosX, textPosY + textSizeY, textPosX + textSizeX, penPosY + btnHeight - borderWidth), bgColor, pPaintData);

        penPosY += btnHeight;
    }

    // Side border. This is done in two parts so that we can leave a little gap for the selected item.
    if (pSettingsEditor->selectedPageIndex != -1) {
        float gapPosY0 = selectedBtnPosY + (1*pDred->uiScale);
        float gapPosY1 = selectedBtnPosY - (1*pDred->uiScale) + btnHeight;

        dred_rect sideBorderRect0 = dred_make_rect(sideRect.right - borderWidth, sideRect.top, sideRect.right, gapPosY0);
        dred_rect sideBorderRect1 = dred_make_rect(sideRect.right - borderWidth, gapPosY0, sideRect.right, gapPosY1);
        dred_rect sideBorderRect2 = dred_make_rect(sideRect.right - borderWidth, gapPosY1, sideRect.right, sideRect.bottom);

        dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), sideBorderRect0, pSettingsEditor->sidePanelBtnBorderColor, pPaintData);
        dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), sideBorderRect1, dred_rgb(255, 255, 255), pPaintData);
        dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), sideBorderRect2, pSettingsEditor->sidePanelBtnBorderColor, pPaintData);
    } else {
        dred_rect sideBorderRect0 = dred_make_rect(sideRect.right - borderWidth, sideRect.top, sideRect.right, sideRect.bottom);
        dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), sideBorderRect0, pSettingsEditor->sidePanelBtnBorderColor, pPaintData);
    }

    // Top of side panel.
    dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), dred_make_rect(0, 0, sideRect.right - borderWidth, pSettingsEditor->sidePanelBtnOffsetY), dred_rgb(255, 255, 255), pPaintData);

    // Bottom of side panel.
    dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), dred_make_rect(0, penPosY, sideRect.right - borderWidth, sideRect.bottom), dred_rgb(255, 255, 255), pPaintData);


    // Action area.
    dred_rect actionRect = dred_settings_editor__get_action_area_rect(pSettingsEditor);
    dred_control_draw_rect(DRED_CONTROL(pSettingsEditor), actionRect, dred_rgb(255, 255, 255), pPaintData);
}


void dred_settings__btn_close__on_pressed(dred_button* pButton)
{
    dred_settings_dialog_hide(dred_control_get_context(DRED_CONTROL(pButton))->pSettingsDialog);
}

void dred_settings__btn_choose_font__on_pressed(dred_button* pButton)
{
    dred_window* pWindow = dred_get_control_window(DRED_CONTROL(pButton));
    if (pWindow == NULL) {
        return; // Should never hit this, but leaving here for sanity.
    }

    dred_context* pDred = pWindow->pDred;

    dred_font_desc fontDesc;
    if (!dred_show_font_picker_dialog(pDred, pWindow, &pDred->config.pTextEditorFont->desc, &fontDesc)) {
        return;
    }

    char fontDescStr[256];
    dred_font_desc_to_string(&fontDesc, fontDescStr, sizeof(fontDescStr));
    
    dred_config_set(&pDred->config, "texteditor-font", fontDescStr);
}


void dred_settings_editor_page__on_mouse_enter(dred_control* pPageControl)
{
    dred_settings_editor* pSettingsEditor = DRED_SETTINGS_EDITOR(dred_control_get_parent(pPageControl));

    assert(pSettingsEditor != NULL);

    // Clear the hovered state of any buttons on the side panel.
    if (pSettingsEditor->hoveredPageIndex != -1) {
        pSettingsEditor->hoveredPageIndex = -1;
        dred_control_dirty(DRED_CONTROL(pSettingsEditor), dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}

void dred_settings_editor_page__on_paint(dred_control* pPageControl, dred_rect rect, void* pPaintData)
{
    (void)rect;
    dred_control_draw_rect(pPageControl, dred_control_get_local_rect(pPageControl), dred_rgb(255, 255, 255), pPaintData);
}

dr_bool32 dred_settings_editor__init_page(dred_settings_editor_page* pPage, dred_context* pDred, dred_control* pParent, const char* title)
{
    assert(pPage != NULL);
    assert(pDred != NULL);
    assert(pParent != NULL);
    assert(title != NULL);

    strcpy_s(pPage->title, sizeof(pPage->title), title);
    pPage->pGUIControl = &pPage->control;
    if (!dred_control_init(pPage->pGUIControl, pDred, pParent, "dred.settings.page")) {
        return DR_FALSE;
    }

    dred_control_hide(pPage->pGUIControl);
    dred_control_set_on_mouse_enter(pPage->pGUIControl, dred_settings_editor_page__on_mouse_enter);
    dred_control_set_on_paint(pPage->pGUIControl, dred_settings_editor_page__on_paint);

    return DR_TRUE;
}

dr_bool32 dred_settings_editor__init_page__general(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pSettingsEditor));
    assert(pSettingsEditor != NULL);

    dred_settings_editor_page* pPage = &pSettingsEditor->pages[DRED_SETTINGS_EDITOR_PAGE_GENERAL];

    if (!dred_settings_editor__init_page(pPage, pDred, DRED_CONTROL(pSettingsEditor), "General")) {
        return DR_FALSE;
    }

    dred_gui_font_metrics fontMetrics;
    dred_gui_get_font_metrics(pSettingsEditor->pFont, &fontMetrics);

    float penPosX = 8*pDred->uiScale;
    float penPosY = 8*pDred->uiScale;

    dred_checkbox_init(&pSettingsEditor->cbShowTabBar, pDred, pPage->pGUIControl, "Show Tab Bar", pDred->config.showTabBar);
    dred_checkbox_set_bind_to_config_var(&pSettingsEditor->cbShowTabBar, "show-tab-bar");
    dred_checkbox_set_padding(&pSettingsEditor->cbShowTabBar, 4*pDred->uiScale);
    dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->cbShowTabBar), penPosX, penPosY);
    penPosY += dred_control_get_height(DRED_CONTROL(&pSettingsEditor->cbShowTabBar)) + (6*pDred->uiScale);

    dred_checkbox_init(&pSettingsEditor->cbShowMenuBar, pDred, pPage->pGUIControl, "Show Menu Bar", pDred->config.showMenuBar);
    dred_checkbox_set_bind_to_config_var(&pSettingsEditor->cbShowMenuBar, "show-menu-bar");
    dred_checkbox_set_padding(&pSettingsEditor->cbShowMenuBar, 4*pDred->uiScale);
    dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->cbShowMenuBar), penPosX, penPosY);
    penPosY += dred_control_get_height(DRED_CONTROL(&pSettingsEditor->cbShowMenuBar)) + (6*pDred->uiScale);

    dred_checkbox_init(&pSettingsEditor->cbAutoHideCmdBar, pDred, pPage->pGUIControl, "Auto-hide Command Bar", pDred->config.autoHideCmdBar);
    dred_checkbox_set_bind_to_config_var(&pSettingsEditor->cbAutoHideCmdBar, "auto-hide-cmd-bar");
    dred_checkbox_set_padding(&pSettingsEditor->cbAutoHideCmdBar, 4*pDred->uiScale);
    dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->cbAutoHideCmdBar), penPosX, penPosY);
    penPosY += dred_control_get_height(DRED_CONTROL(&pSettingsEditor->cbAutoHideCmdBar)) + (6*pDred->uiScale);

    return DR_TRUE;
}

dr_bool32 dred_settings_editor__init_page__theme(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pSettingsEditor));
    assert(pSettingsEditor != NULL);

    dred_settings_editor_page* pPage = &pSettingsEditor->pages[DRED_SETTINGS_EDITOR_PAGE_THEME];

    if (!dred_settings_editor__init_page(pPage, pDred, DRED_CONTROL(pSettingsEditor), "Theme")) {
        return DR_FALSE;
    }

    dred_gui_font_metrics fontMetrics;
    dred_gui_get_font_metrics(pSettingsEditor->pFont, &fontMetrics);

    float penPosX = 8*pDred->uiScale;
    float penPosY = 8*pDred->uiScale;

    if (!dred_button_init(&pSettingsEditor->fontButton, pDred, pPage->pGUIControl, "Choose Font...")) {
        dred_control_uninit(pPage->pGUIControl);
        return DR_FALSE;
    }

    dred_button_set_on_pressed(&pSettingsEditor->fontButton, dred_settings__btn_choose_font__on_pressed);
    dred_button_set_padding(&pSettingsEditor->fontButton, 16*pDred->uiScale, 6*pDred->uiScale);
    dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->fontButton), penPosX, penPosY);

    penPosY += dred_control_get_height(DRED_CONTROL(&pSettingsEditor->fontButton)) + (6*pDred->uiScale);


    if (!dred_colorbutton_init(&pSettingsEditor->textColorButton, pDred, pPage->pGUIControl, "Text color", pDred->config.textEditorTextColor)) {
        dred_control_uninit(pPage->pGUIControl);
        return DR_FALSE;
    }

    dred_colorbutton_set_bind_to_config_var(&pSettingsEditor->textColorButton, "texteditor-text-color");
    dred_colorbutton_set_padding(&pSettingsEditor->textColorButton, 4*pDred->uiScale);
    dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->textColorButton), penPosX, penPosY);

    penPosY += dred_control_get_height(DRED_CONTROL(&pSettingsEditor->textColorButton)) + (6*pDred->uiScale);



    if (!dred_colorbutton_init(&pSettingsEditor->bgColorButton, pDred, pPage->pGUIControl, "Background color", pDred->config.textEditorBGColor)) {
        dred_control_uninit(pPage->pGUIControl);
        return DR_FALSE;
    }

    dred_colorbutton_set_bind_to_config_var(&pSettingsEditor->bgColorButton, "texteditor-bg-color");
    dred_colorbutton_set_padding(&pSettingsEditor->bgColorButton, 4*pDred->uiScale);
    dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->bgColorButton), penPosX, penPosY);

    penPosY += dred_control_get_height(DRED_CONTROL(&pSettingsEditor->textColorButton)) + (6*pDred->uiScale);


    if (!dred_colorbutton_init(&pSettingsEditor->lineColorButton, pDred, pPage->pGUIControl, "Active line color", pDred->config.textEditorActiveLineColor)) {
        dred_control_uninit(pPage->pGUIControl);
        return DR_FALSE;
    }

    dred_colorbutton_set_bind_to_config_var(&pSettingsEditor->lineColorButton, "texteditor-active-line-color");
    dred_colorbutton_set_padding(&pSettingsEditor->lineColorButton, 4*pDred->uiScale);
    dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->lineColorButton), penPosX, penPosY);

    penPosY += dred_control_get_height(DRED_CONTROL(&pSettingsEditor->textColorButton)) + (6*pDred->uiScale);

    return DR_TRUE;
}

dr_bool32 dred_settings_editor__init_page__text_editor(dred_settings_editor* pSettingsEditor)
{
    assert(pSettingsEditor != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pSettingsEditor));
    assert(pSettingsEditor != NULL);

    dred_settings_editor_page* pPage = &pSettingsEditor->pages[DRED_SETTINGS_EDITOR_PAGE_TEXT_EDITOR];

    if (!dred_settings_editor__init_page(pPage, pDred, DRED_CONTROL(pSettingsEditor), "Text Editor")) {
        return DR_FALSE;
    }

    dred_gui_font_metrics fontMetrics;
    dred_gui_get_font_metrics(pSettingsEditor->pFont, &fontMetrics);

    float penPosX = 8*pDred->uiScale;
    float penPosY = 8*pDred->uiScale;

    dred_checkbox_init(&pSettingsEditor->cbTabsToSpaces, pDred, pPage->pGUIControl, "Convert tabs to spaces", pDred->config.textEditorTabsToSpacesEnabled);
    dred_checkbox_set_bind_to_config_var(&pSettingsEditor->cbTabsToSpaces, "texteditor-enable-tabs-to-spaces");
    dred_checkbox_set_padding(&pSettingsEditor->cbTabsToSpaces, 4*pDred->uiScale);
    dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->cbTabsToSpaces), penPosX, penPosY);
    penPosY += dred_control_get_height(DRED_CONTROL(&pSettingsEditor->cbTabsToSpaces)) + (6*pDred->uiScale);

    dred_checkbox_init(&pSettingsEditor->cbShowLineNumbers, pDred, pPage->pGUIControl, "Show line numbers", pDred->config.textEditorShowLineNumbers);
    dred_checkbox_set_bind_to_config_var(&pSettingsEditor->cbShowLineNumbers, "texteditor-show-line-numbers");
    dred_checkbox_set_padding(&pSettingsEditor->cbShowLineNumbers, 4*pDred->uiScale);
    dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->cbShowLineNumbers), penPosX, penPosY);
    penPosY += dred_control_get_height(DRED_CONTROL(&pSettingsEditor->cbShowLineNumbers)) + (6*pDred->uiScale);
    
    return DR_TRUE;
}


dred_settings_editor* dred_settings_editor_create(dred_context* pDred, dred_control* pParent, const char* filePathAbsolute)
{
    dred_settings_editor* pSettingsEditor = (dred_settings_editor*)calloc(1, sizeof(*pSettingsEditor));
    if (pSettingsEditor == NULL) {
        return NULL;
    }

    if (!dred_editor_init(DRED_EDITOR(pSettingsEditor), pDred, pParent, DRED_CONTROL_TYPE_SETTINGS_EDITOR, 0, 0, filePathAbsolute)) {
        free(pSettingsEditor);
        return NULL;
    }

    pSettingsEditor->pFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);
    pSettingsEditor->sidePanelWidth = 200*pDred->uiScale;
    pSettingsEditor->sidePanelBtnOffsetY = 8.0f*pDred->uiScale;
    pSettingsEditor->sidePanelBtnPaddingYRatio = 0.75f;
    pSettingsEditor->sidePanelBtnTextColor = dred_rgb(0, 0, 0);
    pSettingsEditor->sidePanelBtnBorderColor = dred_rgb(160, 160, 160);
    pSettingsEditor->sidePanelBtnGBColorHovered = dred_rgb(200, 224, 255);
    pSettingsEditor->selectedPageIndex = -1;
    pSettingsEditor->hoveredPageIndex = -1;

    dred_settings_editor__init_page__general(pSettingsEditor);
    dred_settings_editor__init_page__theme(pSettingsEditor);
    dred_settings_editor__init_page__text_editor(pSettingsEditor);

    if (!dred_button_init(&pSettingsEditor->closeButton, pDred, DRED_CONTROL(pSettingsEditor), "Close")) {
        dred_editor_uninit(DRED_EDITOR(pSettingsEditor));
        return NULL;
    }

    dred_button_set_on_pressed(&pSettingsEditor->closeButton, dred_settings__btn_close__on_pressed);
    dred_button_set_padding(&pSettingsEditor->closeButton, 32*pDred->uiScale, 6*pDred->uiScale);
    dred_control_set_relative_position(DRED_CONTROL(&pSettingsEditor->closeButton),
        dred_control_get_width(DRED_CONTROL(pSettingsEditor)) - dred_control_get_width(DRED_CONTROL(&pSettingsEditor->closeButton)) - 8*pDred->uiScale,
        dred_control_get_height(DRED_CONTROL(pSettingsEditor)) - dred_control_get_height(DRED_CONTROL(&pSettingsEditor->closeButton)) - 8*pDred->uiScale);


    // Events.
    dred_control_set_on_size(DRED_CONTROL(pSettingsEditor), dred_settings_editor__on_size);
    dred_control_set_on_capture_keyboard(DRED_CONTROL(pSettingsEditor), dred_settings_editor__on_capture_keyboard);
    dred_control_set_on_mouse_button_down(DRED_CONTROL(pSettingsEditor), dred_settings_editor__on_mouse_button_down);
    dred_control_set_on_mouse_move(DRED_CONTROL(pSettingsEditor), dred_settings_editor__on_mouse_move);
    dred_control_set_on_mouse_leave(DRED_CONTROL(pSettingsEditor), dred_settings_editor__on_mouse_leave);
    dred_control_set_on_paint(DRED_CONTROL(pSettingsEditor), dred_settings_editor__on_paint);


    // Select the General tab by default.
    dred_settings_editor__select_page_by_index(pSettingsEditor, DRED_SETTINGS_EDITOR_PAGE_GENERAL);


    return pSettingsEditor;
}

void dred_settings_editor_delete(dred_settings_editor* pSettingsEditor)
{
    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pSettingsEditor));
    if (pDred == NULL) {
        return;
    }

    dred_font_release_subfont(pDred->config.pUIFont, pSettingsEditor->pFont);
    dred_editor_uninit(DRED_EDITOR(pSettingsEditor));
}


void dred_settings_editor_refresh_styling(dred_settings_editor* pSettingsEditor)
{
    if (pSettingsEditor == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pSettingsEditor));
    if (pDred == NULL) {
        return;
    }

    pSettingsEditor->pFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);

    dred_control_dirty(DRED_CONTROL(pSettingsEditor), dred_control_get_local_rect(DRED_CONTROL(pSettingsEditor)));
}
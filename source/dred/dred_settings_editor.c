// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_SETTINGS_EDITOR_PAGE_GENERAL       0
#define DRED_SETTINGS_EDITOR_PAGE_THEME         1
#define DRED_SETTINGS_EDITOR_PAGE_TEXT_EDITOR   2

typedef struct
{
    char title[64];
    drgui_element* pGUIElement;
} dred_settings_editor_page;

typedef struct
{
    dred_control* pCloseButton;

    // General
    dred_checkbox* pCBShowTabBar;
    dred_checkbox* pCBShowMenuBar;
    dred_checkbox* pCBAutoHideCmdBar;

    // Theme
    dred_control* pFontButton;
    dred_control* pTextColorButton;
    dred_control* pBGColorButton;
    dred_control* pLineColorButton;

    // Text Editor
    dred_checkbox* pCBTabsToSpaces;
    dred_checkbox* pCBShowLineNumbers;
    

    // The font to use for everything.
    dred_gui_font* pFont;

    // The width of the side panel.
    float sidePanelWidth;

    // The offset to apply to the buttons on the side panel on the Y axis.
    float sidePanelBtnOffsetY;

    // The vertical padding to apply to the buttons on the side panel, as a ratio of the height of the current font.
    float sidePanelBtnPaddingYRatio;

    // The color of the text of side panels.
    dred_color sidePanelBtnTextColor;

    // The color of the border of side panel buttons.
    dred_color sidePanelBtnBorderColor;

    // The background color of side panel buttons while hovered.
    dred_color sidePanelBtnGBColorHovered;

    // Everything in the settings editor is grouped into "pages". This array holds those pages, with specific pages
    // being accessable with a 0 based index DRED_SETTINGS_PAGE_*.
    dred_settings_editor_page pages[3];

    // The index of the page that is currently selected. Set to -1 if no page is selected.
    int selectedPageIndex;

    // The index of the page that the mouse is currently sitting on top of. Set to -1 if no page is hovered.
    int hoveredPageIndex;
} dred_settings_editor_data;

dred_rect dred_settings_editor__get_side_panel_rect(dred_settings_editor* pSettingsEditor)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    return drgui_make_rect(0, 0, pData->sidePanelWidth, drgui_get_height(pSettingsEditor));
}

dred_rect dred_settings_editor__get_action_area_rect(dred_settings_editor* pSettingsEditor)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    dred_context* pDred = dred_control_get_context(pSettingsEditor);
    assert(pData != NULL);

    float posX = pData->sidePanelWidth;
    float posY = drgui_get_height(pSettingsEditor) - (dred_control_get_height(pData->pCloseButton)) - (8*pDred->uiScale * 2);

    return drgui_make_rect(posX, posY, drgui_get_width(pSettingsEditor), drgui_get_height(pSettingsEditor));
}

int dred_settings_editor__get_side_panel_btn_index_under_point(dred_settings_editor* pSettingsEditor, float posX, float posY)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    if (posX < 0 || posX > pData->sidePanelWidth) {
        return -1;
    }


    dred_gui_font_metrics metrics;
    drgui_get_font_metrics(pData->pFont, &metrics);

    float paddingY = metrics.lineHeight*pData->sidePanelBtnPaddingYRatio;
    float btnHeight = paddingY*2 + metrics.lineHeight;
    float penPosY = pData->sidePanelBtnOffsetY;    

    int sideButtonsCount = (int)(sizeof(pData->pages) / sizeof(pData->pages[0]));
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
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    int oldPageIndex = pData->selectedPageIndex;
    if (newPageIndex != oldPageIndex) {
        if (oldPageIndex != -1) {
            drgui_hide(pData->pages[oldPageIndex].pGUIElement);
        }
        if (newPageIndex != -1) {
            drgui_show(pData->pages[newPageIndex].pGUIElement);
        }

        pData->selectedPageIndex = newPageIndex;
        drgui_dirty(pSettingsEditor, dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}


void dred_settings_editor__refresh_layout(dred_settings_editor* pSettingsEditor)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    dred_context* pDred = dred_control_get_context(pSettingsEditor);
    assert(pData != NULL);

    float posX = pData->sidePanelWidth;
    float posY = 0;
    float sizeX = drgui_get_width(pSettingsEditor) - posX;
    float sizeY = drgui_get_height(pSettingsEditor) - (dred_control_get_height(pData->pCloseButton)) - (8*pDred->uiScale * 2);

    // Every page needs to be resized. Every page will be the same size.
    int sideButtonsCount = (int)(sizeof(pData->pages) / sizeof(pData->pages[0]));
    for (int i = 0; i < sideButtonsCount; ++i) {
        drgui_set_relative_position(pData->pages[i].pGUIElement, posX, posY);
        drgui_set_size(pData->pages[i].pGUIElement, sizeX, sizeY);
    }
}


void dred_settings_editor__on_size(dred_settings_editor* pSettingsEditor, float newWidth, float newHeight)
{
    (void)pSettingsEditor;
    (void)newWidth;
    (void)newHeight;

    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    dred_context* pDred = dred_control_get_context(pSettingsEditor);

    dred_settings_editor__refresh_layout(pSettingsEditor);

    dred_control_set_relative_position(pData->pCloseButton,
        dred_control_get_width(pSettingsEditor) - dred_control_get_width(pData->pCloseButton) - 8*pDred->uiScale,
        dred_control_get_height(pSettingsEditor) - dred_control_get_height(pData->pCloseButton) - 8*pDred->uiScale);

    
}

void dred_settings_editor__on_capture_keyboard(dred_settings_editor* pSettingsEditor, drgui_element* pPrevCapturedElement)
{
    (void)pSettingsEditor;
    (void)pPrevCapturedElement;
}

void dred_settings_editor__on_mouse_button_down(dred_settings_editor* pSettingsEditor, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mouseButton;
    (void)stateFlags;

    int newPageIndex = dred_settings_editor__get_side_panel_btn_index_under_point(pSettingsEditor, (float)mousePosX, (float)mousePosY);
    if (newPageIndex == -1) {
        return; // Don't change the selection if nothing was picked.
    }

    dred_settings_editor__select_page_by_index(pSettingsEditor, newPageIndex);
}

void dred_settings_editor__on_mouse_move(dred_settings_editor* pSettingsEditor, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    int hoveredPageIndex = dred_settings_editor__get_side_panel_btn_index_under_point(pSettingsEditor, (float)mousePosX, (float)mousePosY);
    if (hoveredPageIndex != pData->hoveredPageIndex) {
        pData->hoveredPageIndex = hoveredPageIndex;
        drgui_dirty(pSettingsEditor, dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}

void dred_settings_editor__on_mouse_leave(dred_settings_editor* pSettingsEditor)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    // None of the buttons will be hovered if the mouse leaves the window.
    if (pData->hoveredPageIndex != -1) {
        pData->hoveredPageIndex = -1;
        drgui_dirty(pSettingsEditor, dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}

void dred_settings_editor__on_paint(dred_settings_editor* pSettingsEditor, dred_rect rect, void* pPaintData)
{
    (void)rect;

    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    dred_context* pDred = dred_control_get_context(pSettingsEditor);
    assert(pDred != NULL);

    dred_rect sideRect = dred_settings_editor__get_side_panel_rect(pSettingsEditor);

    // Side buttons.
    float penPosX = 0;
    float penPosY = pData->sidePanelBtnOffsetY;

    dred_gui_font_metrics metrics;
    drgui_get_font_metrics(pData->pFont, &metrics);

    float paddingY = metrics.lineHeight*pData->sidePanelBtnPaddingYRatio;
    float btnHeight = paddingY*2 + metrics.lineHeight;
    float selectedBtnPosY = 0;
    float borderWidth = 1*pDred->uiScale;

    int sideButtonsCount = (int)(sizeof(pData->pages) / sizeof(pData->pages[0]));
    for (int i = 0; i < sideButtonsCount; ++i) {
        dred_color bgColor = drgui_rgb(255, 255, 255); //drgui_rgb(128, 32, 32);
        dred_color borderColor = bgColor;
        if (pData->selectedPageIndex == i) {
            selectedBtnPosY = penPosY;
            borderColor = pData->sidePanelBtnBorderColor;
        } else if (pData->hoveredPageIndex == i) {
            bgColor = pData->sidePanelBtnGBColorHovered;
            borderColor = pData->sidePanelBtnGBColorHovered;
        }

        float textSizeX;
        float textSizeY;
        drgui_measure_string(pData->pFont, pData->pages[i].title, strlen(pData->pages[i].title), &textSizeX, &textSizeY);

        float textPosX = penPosX + 8*pDred->uiScale;
        float textPosY = penPosY + paddingY;
        drgui_draw_text(pSettingsEditor, pData->pFont, pData->pages[i].title, (int)strlen(pData->pages[i].title), textPosX, textPosY, pData->sidePanelBtnTextColor, bgColor, pPaintData);

        // Border.
        drgui_draw_rect(pSettingsEditor, drgui_make_rect(penPosX, penPosY, pData->sidePanelWidth - borderWidth, penPosY + borderWidth), borderColor, pPaintData);
        drgui_draw_rect(pSettingsEditor, drgui_make_rect(penPosX, penPosY + btnHeight - borderWidth, pData->sidePanelWidth - borderWidth, penPosY + btnHeight), borderColor, pPaintData);

        // Background.
        drgui_draw_rect(pSettingsEditor, drgui_make_rect(0, penPosY + borderWidth, textPosX, penPosY + btnHeight - borderWidth), bgColor, pPaintData);
        drgui_draw_rect(pSettingsEditor, drgui_make_rect(textPosX + textSizeX, penPosY + borderWidth, pData->sidePanelWidth - borderWidth, penPosY + btnHeight - borderWidth), bgColor, pPaintData);
        drgui_draw_rect(pSettingsEditor, drgui_make_rect(textPosX, penPosY + borderWidth, textPosX + textSizeX, textPosY), bgColor, pPaintData);
        drgui_draw_rect(pSettingsEditor, drgui_make_rect(textPosX, textPosY + textSizeY, textPosX + textSizeX, penPosY + btnHeight - borderWidth), bgColor, pPaintData);

        penPosY += btnHeight;
    }

    // Side border. This is done in two parts so that we can leave a little gap for the selected item.
    if (pData->selectedPageIndex != -1) {
        float gapPosY0 = selectedBtnPosY + (1*pDred->uiScale);
        float gapPosY1 = selectedBtnPosY - (1*pDred->uiScale) + btnHeight;

        dred_rect sideBorderRect0 = drgui_make_rect(sideRect.right - borderWidth, sideRect.top, sideRect.right, gapPosY0);
        dred_rect sideBorderRect1 = drgui_make_rect(sideRect.right - borderWidth, gapPosY0, sideRect.right, gapPosY1);
        dred_rect sideBorderRect2 = drgui_make_rect(sideRect.right - borderWidth, gapPosY1, sideRect.right, sideRect.bottom);

        drgui_draw_rect(pSettingsEditor, sideBorderRect0, pData->sidePanelBtnBorderColor, pPaintData);
        drgui_draw_rect(pSettingsEditor, sideBorderRect1, drgui_rgb(255, 255, 255), pPaintData);
        drgui_draw_rect(pSettingsEditor, sideBorderRect2, pData->sidePanelBtnBorderColor, pPaintData);
    } else {
        dred_rect sideBorderRect0 = drgui_make_rect(sideRect.right - borderWidth, sideRect.top, sideRect.right, sideRect.bottom);
        drgui_draw_rect(pSettingsEditor, sideBorderRect0, pData->sidePanelBtnBorderColor, pPaintData);
    }

    // Top of side panel.
    drgui_draw_rect(pSettingsEditor, drgui_make_rect(0, 0, sideRect.right - borderWidth, pData->sidePanelBtnOffsetY), drgui_rgb(255, 255, 255), pPaintData);

    // Bottom of side panel.
    drgui_draw_rect(pSettingsEditor, drgui_make_rect(0, penPosY, sideRect.right - borderWidth, sideRect.bottom), drgui_rgb(255, 255, 255), pPaintData);


    // Action area.
    dred_rect actionRect = dred_settings_editor__get_action_area_rect(pSettingsEditor);
    drgui_draw_rect(pSettingsEditor, actionRect, drgui_rgb(255, 255, 255), pPaintData);
}


void dred_settings__btn_close__on_pressed(dred_button* pButton)
{
    dred_settings_dialog_hide(dred_control_get_context(pButton)->pSettingsDialog);
}

void dred_settings__btn_choose_font__on_pressed(dred_button* pButton)
{
    dred_window* pWindow = dred_get_element_window(pButton);
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


void dred_settings_editor_page__on_mouse_enter(drgui_element* pPageElement)
{
    dred_settings_editor* pSettingsEditor = drgui_get_parent(pPageElement);

    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    // Clear the hovered state of any buttons on the side panel.
    if (pData->hoveredPageIndex != -1) {
        pData->hoveredPageIndex = -1;
        drgui_dirty(pSettingsEditor, dred_settings_editor__get_side_panel_rect(pSettingsEditor));
    }
}

void dred_settings_editor_page__on_paint(drgui_element* pPageElement, dred_rect rect, void* pPaintData)
{
    (void)rect;
    drgui_draw_rect(pPageElement, drgui_get_local_rect(pPageElement), drgui_rgb(255, 255, 255), pPaintData);
}

bool dred_settings_editor__init_page(dred_settings_editor_page* pPage, dred_context* pDred, dred_control* pParent, const char* title)
{
    assert(pPage != NULL);
    assert(pDred != NULL);
    assert(pParent != NULL);
    assert(title != NULL);

    strcpy_s(pPage->title, sizeof(pPage->title), title);
    pPage->pGUIElement = dred_control_create(pDred, pParent, "dred.settings.page", 0);
    if (pPage->pGUIElement == NULL) {
        return false;
    }

    drgui_hide(pPage->pGUIElement);
    drgui_set_on_mouse_enter(pPage->pGUIElement, dred_settings_editor_page__on_mouse_enter);
    drgui_set_on_paint(pPage->pGUIElement, dred_settings_editor_page__on_paint);

    return true;
}

bool dred_settings_editor__init_page__general(dred_settings_editor* pSettingsEditor)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    dred_context* pDred = dred_control_get_context(pSettingsEditor);
    assert(pData != NULL);

    dred_settings_editor_page* pPage = &pData->pages[DRED_SETTINGS_EDITOR_PAGE_GENERAL];

    if (!dred_settings_editor__init_page(pPage, pDred, pSettingsEditor, "General")) {
        return false;
    }

    dred_gui_font_metrics fontMetrics;
    drgui_get_font_metrics(pData->pFont, &fontMetrics);

    float penPosX = 8*pDred->uiScale;
    float penPosY = 8*pDred->uiScale;

    pData->pCBShowTabBar = dred_checkbox_create(pDred, pPage->pGUIElement, "Show Tab Bar", pDred->config.showTabBar);
    dred_checkbox_set_bind_to_config_var(pData->pCBShowTabBar, "show-tab-bar");
    dred_checkbox_set_padding(pData->pCBShowTabBar, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pCBShowTabBar, penPosX, penPosY);
    penPosY += dred_control_get_height(pData->pCBShowTabBar) + (6*pDred->uiScale);

    pData->pCBShowMenuBar = dred_checkbox_create(pDred, pPage->pGUIElement, "Show Menu Bar", pDred->config.showMenuBar);
    dred_checkbox_set_bind_to_config_var(pData->pCBShowMenuBar, "show-menu-bar");
    dred_checkbox_set_padding(pData->pCBShowMenuBar, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pCBShowMenuBar, penPosX, penPosY);
    penPosY += dred_control_get_height(pData->pCBShowMenuBar) + (6*pDred->uiScale);

    pData->pCBShowMenuBar = dred_checkbox_create(pDred, pPage->pGUIElement, "Auto-hide Command Bar", pDred->config.autoHideCmdBar);
    dred_checkbox_set_bind_to_config_var(pData->pCBShowMenuBar, "auto-hide-cmd-bar");
    dred_checkbox_set_padding(pData->pCBShowMenuBar, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pCBShowMenuBar, penPosX, penPosY);
    penPosY += dred_control_get_height(pData->pCBShowMenuBar) + (6*pDred->uiScale);

    return true;
}

bool dred_settings_editor__init_page__theme(dred_settings_editor* pSettingsEditor)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    dred_context* pDred = dred_control_get_context(pSettingsEditor);
    assert(pData != NULL);

    dred_settings_editor_page* pPage = &pData->pages[DRED_SETTINGS_EDITOR_PAGE_THEME];

    if (!dred_settings_editor__init_page(pPage, pDred, pSettingsEditor, "Theme")) {
        return false;
    }

    dred_gui_font_metrics fontMetrics;
    drgui_get_font_metrics(pData->pFont, &fontMetrics);

    float penPosX = 8*pDred->uiScale;
    float penPosY = 8*pDred->uiScale;

    pData->pFontButton = dred_button_create(pDred, pPage->pGUIElement, "Choose Font...");
    if (pData->pFontButton == NULL) {
        dred_editor_delete(pPage->pGUIElement);
        return false;
    }

    dred_button_set_on_pressed(pData->pFontButton, dred_settings__btn_choose_font__on_pressed);
    dred_button_set_padding(pData->pFontButton, 16*pDred->uiScale, 6*pDred->uiScale);
    dred_control_set_relative_position(pData->pFontButton, penPosX, penPosY);

    penPosY += dred_control_get_height(pData->pFontButton) + (6*pDred->uiScale);


    pData->pTextColorButton = dred_colorbutton_create(pDred, pPage->pGUIElement, "Text color", pDred->config.textEditorTextColor);
    if (pData->pTextColorButton == NULL) {
        dred_editor_delete(pPage->pGUIElement);
        return false;
    }

    dred_colorbutton_set_bind_to_config_var(pData->pTextColorButton, "texteditor-text-color");
    dred_colorbutton_set_padding(pData->pTextColorButton, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pTextColorButton, penPosX, penPosY);

    penPosY += dred_control_get_height(pData->pTextColorButton) + (6*pDred->uiScale);



    pData->pBGColorButton = dred_colorbutton_create(pDred, pPage->pGUIElement, "Background color", pDred->config.textEditorBGColor);
    if (pData->pBGColorButton == NULL) {
        dred_editor_delete(pPage->pGUIElement);
        return false;
    }

    dred_colorbutton_set_bind_to_config_var(pData->pBGColorButton, "texteditor-bg-color");
    dred_colorbutton_set_padding(pData->pBGColorButton, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pBGColorButton, penPosX, penPosY);

    penPosY += dred_control_get_height(pData->pTextColorButton) + (6*pDred->uiScale);


    pData->pLineColorButton = dred_colorbutton_create(pDred, pPage->pGUIElement, "Active line color", pDred->config.textEditorActiveLineColor);
    if (pData->pLineColorButton == NULL) {
        dred_editor_delete(pPage->pGUIElement);
        return false;
    }

    dred_colorbutton_set_bind_to_config_var(pData->pLineColorButton, "texteditor-active-line-color");
    dred_colorbutton_set_padding(pData->pLineColorButton, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pLineColorButton, penPosX, penPosY);

    penPosY += dred_control_get_height(pData->pTextColorButton) + (6*pDred->uiScale);

    return true;
}

bool dred_settings_editor__init_page__text_editor(dred_settings_editor* pSettingsEditor)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    dred_context* pDred = dred_control_get_context(pSettingsEditor);
    assert(pData != NULL);

    dred_settings_editor_page* pPage = &pData->pages[DRED_SETTINGS_EDITOR_PAGE_TEXT_EDITOR];

    if (!dred_settings_editor__init_page(pPage, pDred, pSettingsEditor, "Text Editor")) {
        return false;
    }

    dred_gui_font_metrics fontMetrics;
    drgui_get_font_metrics(pData->pFont, &fontMetrics);

    float penPosX = 8*pDred->uiScale;
    float penPosY = 8*pDred->uiScale;

    pData->pCBTabsToSpaces = dred_checkbox_create(pDred, pPage->pGUIElement, "Convert tabs to spaces", pDred->config.textEditorTabsToSpacesEnabled);
    dred_checkbox_set_bind_to_config_var(pData->pCBTabsToSpaces, "texteditor-enable-tabs-to-spaces");
    dred_checkbox_set_padding(pData->pCBTabsToSpaces, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pCBTabsToSpaces, penPosX, penPosY);
    penPosY += dred_control_get_height(pData->pCBTabsToSpaces) + (6*pDred->uiScale);

    pData->pCBShowLineNumbers = dred_checkbox_create(pDred, pPage->pGUIElement, "Show line numbers", pDred->config.textEditorShowLineNumbers);
    dred_checkbox_set_bind_to_config_var(pData->pCBShowLineNumbers, "texteditor-show-line-numbers");
    dred_checkbox_set_padding(pData->pCBShowLineNumbers, 4*pDred->uiScale);
    dred_control_set_relative_position(pData->pCBShowLineNumbers, penPosX, penPosY);
    penPosY += dred_control_get_height(pData->pCBShowLineNumbers) + (6*pDred->uiScale);
    
    return true;
}


dred_settings_editor* dred_settings_editor_create(dred_context* pDred, dred_control* pParent, const char* filePathAbsolute)
{
    dred_settings_editor* pSettingsEditor = dred_editor_create(pDred, pParent, DRED_CONTROL_TYPE_SETTINGS_EDITOR, 0, 0, filePathAbsolute, sizeof(dred_settings_editor_data));
    if (pSettingsEditor == NULL) {
        return NULL;
    }

    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    assert(pData != NULL);

    pData->pFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);
    pData->sidePanelWidth = 200*pDred->uiScale;
    pData->sidePanelBtnOffsetY = 8.0f*pDred->uiScale;
    pData->sidePanelBtnPaddingYRatio = 0.75f;
    pData->sidePanelBtnTextColor = drgui_rgb(0, 0, 0);
    pData->sidePanelBtnBorderColor = drgui_rgb(160, 160, 160);
    pData->sidePanelBtnGBColorHovered = drgui_rgb(200, 224, 255);
    pData->selectedPageIndex = -1;
    pData->hoveredPageIndex = -1;

    dred_settings_editor__init_page__general(pSettingsEditor);
    dred_settings_editor__init_page__theme(pSettingsEditor);
    dred_settings_editor__init_page__text_editor(pSettingsEditor);

    pData->pCloseButton = dred_button_create(pDred, pSettingsEditor, "Close");
    if (pData->pCloseButton == NULL) {
        dred_editor_delete(pSettingsEditor);
        return NULL;
    }

    dred_button_set_on_pressed(pData->pCloseButton, dred_settings__btn_close__on_pressed);
    dred_button_set_padding(pData->pCloseButton, 32*pDred->uiScale, 6*pDred->uiScale);
    dred_control_set_relative_position(pData->pCloseButton,
        dred_control_get_width(pSettingsEditor) - dred_control_get_width(pData->pCloseButton) - 8*pDred->uiScale,
        dred_control_get_height(pSettingsEditor) - dred_control_get_height(pData->pCloseButton) - 8*pDred->uiScale);


    // Events.
    dred_control_set_on_size(pSettingsEditor, dred_settings_editor__on_size);
    dred_control_set_on_capture_keyboard(pSettingsEditor, dred_settings_editor__on_capture_keyboard);
    dred_control_set_on_mouse_button_down(pSettingsEditor, dred_settings_editor__on_mouse_button_down);
    dred_control_set_on_mouse_move(pSettingsEditor, dred_settings_editor__on_mouse_move);
    dred_control_set_on_mouse_leave(pSettingsEditor, dred_settings_editor__on_mouse_leave);
    dred_control_set_on_paint(pSettingsEditor, dred_settings_editor__on_paint);


    // Select the General tab by default.
    dred_settings_editor__select_page_by_index(pSettingsEditor, DRED_SETTINGS_EDITOR_PAGE_GENERAL);


    return pSettingsEditor;
}

void dred_settings_editor_delete(dred_settings_editor* pSettingsEditor)
{
    dred_context* pDred = dred_control_get_context(pSettingsEditor);
    if (pDred == NULL) {
        return;
    }

    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    if (pData != NULL) {
        dred_font_release_subfont(pDred->config.pUIFont, pData->pFont);
    }

    dred_editor_delete(pSettingsEditor);
}


void dred_settings_editor_refresh_styling(dred_settings_editor* pSettingsEditor)
{
    dred_settings_editor_data* pData = (dred_settings_editor_data*)dred_editor_get_extra_data(pSettingsEditor);
    if (pData == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pSettingsEditor);
    if (pDred == NULL) {
        return;
    }

    pData->pFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);

    drgui_dirty(pSettingsEditor, drgui_get_local_rect(pSettingsEditor));
}
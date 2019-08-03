// Copyright (C) 2019 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_SETTINGS_EDITOR  "dred.editor.settings"

typedef struct dred_settings_editor dred_settings_editor;
#define DRED_SETTINGS_EDITOR(a) ((dred_settings_editor*)(a))

typedef struct
{
    char title[64];
    dtk_control control;
} dred_settings_editor_page;

struct dred_settings_editor
{
    // The base editor.
    dred_editor editor;


    dtk_button closeButton;

    // General
    dtk_checkbox cbShowTabBar;
    dtk_checkbox cbShowMenuBar;
    dtk_checkbox cbAutoHideCmdBar;

    // Theme
    dtk_button fontButton;
    dtk_color_button textColorButton;
    dtk_color_button bgColorButton;
    dtk_color_button lineColorButton;

    // Text Editor
    dtk_checkbox cbTabsToSpaces;
    dtk_checkbox cbShowLineNumbers;
    

    // The font to use for everything.
    dtk_font* pFont;

    // The width of the side panel.
    dtk_int32 sidePanelWidth;

    // The offset to apply to the buttons on the side panel on the Y axis.
    dtk_int32 sidePanelBtnOffsetY;

    // The vertical padding to apply to the buttons on the side panel, as a ratio of the height of the current font.
    float sidePanelBtnPaddingYRatio;

    // The color of the text of side panels.
    dtk_color sidePanelBtnTextColor;

    // The color of the border of side panel buttons.
    dtk_color sidePanelBtnBorderColor;

    // The background color of side panel buttons while hovered.
    dtk_color sidePanelBtnGBColorHovered;

    // Everything in the settings editor is grouped into "pages". This array holds those pages, with specific pages
    // being accessable with a 0 based index DRED_SETTINGS_PAGE_*.
    dred_settings_editor_page pages[3];

    // The index of the page that is currently selected. Set to -1 if no page is selected.
    int selectedPageIndex;

    // The index of the page that the mouse is currently sitting on top of. Set to -1 if no page is hovered.
    int hoveredPageIndex;
};


dred_settings_editor* dred_settings_editor_create(dred_context* pDred, dtk_control* pParent, const char* filePathAbsolute);
void dred_settings_editor_delete(dred_settings_editor* pSettingsEditor);

// Refreshes the styling of the given settings editor.
void dred_settings_editor_refresh_styling(dred_settings_editor* pSettingsEditor);
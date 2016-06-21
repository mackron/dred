
// All config variables are defined in the section below. The pre-build tool will look at these variables and construct the main
// config struct based on their names and types.

// BEGIN CONFIG VARS: <name> <C variable name> <type> <on_set callback> <default value>
//
// show-tab-bar showTabBar bool dred_config_on_set__show_tab_bar true
//   Whether or not to show the tab bar.
//
// show-menu-bar showMenuBar bool dred_config_on_set__show_menu_bar true
//   Whether or not to show the menu bar.
//
// auto-hide-cmd-bar autoHideCmdBar bool dred_config_on_set__auto_hide_cmd_bar false
//   Whether or not to show the command bar at all times. If this is set to false it will be shown only when it has keyboard focus.
//
//
// window-width windowWidth int none 640
//   The width of the window
//
// window-height windowHeight int none 480
//   The height of the window.
//
// window-maximized windowMaximized bool none false
//   Whether or not the window is maximized.
//
//
// ui-scale uiScale float none 1
//   The scale of the UI. The total scale of UI elements is equal to the DPI scaling multiplied by this value.
//
// ui-font pUIFont font dred_config_on_set__ui_font system-font-ui
//   The default font to use for general UI elements. This will act as the default font in the event that a font fails to load.
//
//
// cmdbar-bg-color cmdbarBGColor color none 64 64 64 
//   The background color of the command bar.
//
// cmdbar-bg-color-active cmdbarBGColorActive color none 128 51 0
//   The background color of the command bar while it's got the keyboard focus.
//
// cmdbar-tb-font pCmdbarTBFont font none system-font-mono
//   The font to use in the text box of the command bar.
//
// cmdbar-tb-text-color cmdbarTBTextColor color none 224 224 224
//   The color of the text in the text box of the command bar.
//
// cmdbar-msg-font pCmdbarMessageFont font none system-font-ui
//   The font to use for the message box of the command bar.
//
// cmdbar-padding-horz cmdbarPaddingX float none 2
//   The amount of padding to apply to the command bar on the left and right sides.
//
// cmdbar-padding-vert cmdbarPaddingY float none 2
//   The amount of padding to apply to the command bar on the top and bottom sides.
//
//
// tabgroup-bg-color tabgroupBGColor color none 48 48 48
//   The background color of tab groups. This will be shown when there are no tabs open in the group.
//
// tab-bg-color-inactive tabBGColorInvactive color none 58 58 58
//   The background color of tabs while inactive.
//
// tab-bg-color-active tabBGColorActive color none 16 92 160
//   The background color of tabs while active.
//
// tab-bg-color-hovered tabBGColorHovered color none 32 128 192
//   The background color of tabs while hovered.
//
// tab-font tabFont font none system-font-ui
//   The font to use on tabs.
//
// tab-text-color tabTextColor color none 224 224 224
//   The color of the text on tabs.
//
// tab-padding tabPadding float none 4
//   The amount of padding to apply to all sides of a tab.
//
// tab-show-close-button tabShowCloseButton bool none true
//   Whether or not to show the close button on tabs.
//
//
// texteditor-font pTextEditorFont font dred_config_on_set__texteditor_font system-font-mono
//   The font to use in text editors.
//
// texteditor-text-color textEditorTextColor color none 224 224 224
//   The base color of the text in a text editor. Note that syntax highlighting is independant of this variable.
//
// texteditor-bg-color textEditorBGColor color none 48 48 48
//   The background color to use in text editors.
//
// texteditor-cursor-color textEditorCursorColor color none 224 224 224
//   The color of the caret/cursor in text editors.
//
// texteditor-cursor-width textEditorCursorWidth float none 1
//   The thickness of the caret/cursor in text editors.
//
// texteditor-selection-bg-color textEditorSelectionBGColor color none 64 128 192
//   The background color to use for selected text.
//
// texteditor-active-line-color textEditorActiveLineColor color none 40 40 40
//   The background color of the line that the caret is currently sitting on in a text editor.
//
// texteditor-show-line-numbers textEditorShowLineNumbers bool none false
//   Whether or not to show line numbers.
//
// texteditor-line-numbers-color textEditorLineNumbersColor color none 80 160 192
//   The color of the line numbers text.
//
// texteditor-line-numbers-bg-color textEditorLineNumbersBGColor color none 48 48 48
//   The color of the background of the line numbers.
//
// texteditor-line-numbers-padding textEditorLineNumbersPadding float none 16
//   The padding between the line numbers and the text in the text editor.
//
// texteditor-enable-tabs-to-spaces textEditorTabsToSpacesEnabled bool none false
//   Whether or not tabs are converted to spaces.
//
// texteditor-tab-size-in-spaces textEditorTabSizeInSpaces int none 4
//   The size of tabs in spaces.
//
// texteditor-scale textEditorScale float none 1
//   The scale/zoom to use in text editors.
//
// END CONFIG VARS

typedef struct
{
    // The main context that owns this config. This is set to the context that initialized the config object.
    dred_context* pDred;

    // System fonts.
    dred_font* pSystemFontUI;
    dred_font* pSystemFontMono;

    // Recent files.
    char* recentFiles[DRED_MAX_RECENT_FILES];
    size_t recentFileCount;

    // The cross image.
    dred_image* pImageCross;


    // The auto-generated variable declarations. You can find these in dred_autogenerated.h
    DRED_CONFIG_VARIABLE_DECLARATIONS

} dred_config;

typedef void (* dred_config_on_error_proc)(dred_config* pConfig, const char* configPath, const char* message, unsigned int line, void* pUserData);

bool dred_config_init(dred_config* pConfig, dred_context* pDred);
void dred_config_uninit(dred_config* pConfig);

// The pUserData argument of onError will be set to pConfig.
bool dred_config_load_file(dred_config* pConfig, const char* filePath, dred_config_on_error_proc onError, void* pUserData);

// Sets a variable from a name/value string pair.
void dred_config_set(dred_config* pConfig, const char* name, const char* value);

// Pushes a new recent file to the end of the list.
void dred_config_push_recent_file(dred_config* pConfig, const char* fileAbsolutePath);



// Set handlers.

// show-tab-bar
void dred_config_on_set__show_tab_bar(dred_context* pDred);

// show-menu-bar
void dred_config_on_set__show_menu_bar(dred_context* pDred);

// auto-hide-cmd-bar
void dred_config_on_set__auto_hide_cmd_bar(dred_context* pDred);


// ui-font
void dred_config_on_set__ui_font(dred_context* pDred);

// texteditor-font
void dred_config_on_set__texteditor_font(dred_context* pDred);


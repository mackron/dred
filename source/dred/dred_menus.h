// Copyright (C) 2016 David Reid. See included LICENSE file.

#if 0
// BEGIN MENUS
//
// MENU FileMenu
//   MENUITEM "&New" DRED_MENU_ITEM_ID_FILE_NEW "new" DRED_ACCELERATOR_ID_NEW 
// /MENU
//
// MENUBAR Default
//   MENUITEM "&File" SUBMNEU(FileMenu)
//   MENUITEM "&Edit" SUBMENU(EditMenu)
// /MENUBAR
//
// MENUBAR TextEditor
//   MENUITEM "&File" SUBMENU(FileMenu)
//   MENUITEM "&Edit" SUBMENU(EditMenu_TextEditor)
//   MENUITEM "&View" SUBMENU(ViewMenu_TextEditor)
//   MENUITEM "&Find" SUBMENU(FindMenu_TextEditor)
//   MENUITEM "&Help" SUBMENU(HelpMenu)
// /MENUBAR
//
// END MENUS

#define DRED_MENU_ITEM_ID_NONE                      0
#define DRED_MENU_ITEM_ID_FILE_NEW                  1
#define DRED_MENU_ITEM_ID_FILE_OPEN                 2
#define DRED_MENU_ITEM_ID_FILE_SAVE                 3
#define DRED_MENU_ITEM_ID_FILE_SAVE_AS              4
#define DRED_MENU_ITEM_ID_FILE_SAVE_ALL             5
#define DRED_MENU_ITEM_ID_FILE_CLOSE                6
#define DRED_MENU_ITEM_ID_FILE_CLOSE_ALL            7
#define DRED_MENU_ITEM_ID_FILE_PRINT_PREVIEW        8
#define DRED_MENU_ITEM_ID_FILE_PRINT                9
#define DRED_MENU_ITEM_ID_FILE_EXIT                 10

#define DRED_MENU_ITEM_ID_EDIT_UNDO                 11
#define DRED_MENU_ITEM_ID_EDIT_REDO                 12
#define DRED_MENU_ITEM_ID_EDIT_CUT                  13
#define DRED_MENU_ITEM_ID_EDIT_COPY                 14
#define DRED_MENU_ITEM_ID_EDIT_PASTE                15
#define DRED_MENU_ITEM_ID_EDIT_DELETE               16
#define DRED_MENU_ITEM_ID_EDIT_SELECT_ALL           17

#define DRED_MENU_ITEM_ID_VIEW_TAB_BARS             18
#define DRED_MENU_ITEM_ID_VIEW_CMD_BAR              19
#define DRED_MENU_ITEM_ID_VIEW_MENU_BAR             20
#define DRED_MENU_ITEM_ID_VIEW_LINE_NUMBERS         21
#define DRED_MENU_ITEM_ID_VIEW_RESET_ZOOM           22
#define DRED_MENU_ITEM_ID_VIEW_WORD_WRAP            23

#define DRED_MENU_ITEM_ID_FIND_FIND                 24
#define DRED_MENU_ITEM_ID_FIND_REPLACE              25
#define DRED_MENU_ITEM_ID_FIND_GOTO                 26

#define DRED_MENU_ITEM_ID_SETTINGS_SETTINGS         27

#define DRED_MENU_ITEM_ID_HELP_HELP                 28
#define DRED_MENU_ITEM_ID_HELP_ABOUT                29

#define DRED_MENU_ITEM_ID_ADD_TO_FAVOURITES         30
#define DRED_MENU_ITEM_ID_REMOVE_FROM_FAVOURITES    31

struct dred_menu_library
{
    // The main context that initialized the library.
    dred_context* pDred;


    // The recent files menu.
    dred_menu* pMenu_RecentFiles;
    size_t recentFileCount;

    // The favourite files menu.
    dred_menu* pMenu_FavouriteFiles;
    size_t favouriteFileCount;

    // The themes menu.
    dred_menu* pMenu_Themes;


    // The menu to use when nothing is opened.
    dred_menu* pMenu_Default;

    // The menu to use for text editors.
    dred_menu* pMenu_TextEditor;


    // The right-click popup menu for text editors.
    dred_menu* pPopupMenu_TextEditor;

    // The right-click popup menu for tabs.
    dred_menu* pPopupMenu_Tab;
    dred_menu_item* pMenuItem_AddToFavourites;
    dred_menu_item* pMenuItem_RemoveFromFavourites;


    // The Open Recent menu items. We need to keep track of these so we can dynamically enable and disable them.
    dred_menu_item* pOpenRecentItem_Default;
    dred_menu_item* pOpenRecentItem_TextEditor;

    // The Open Favourite menu items. We need to keep track of these so we can dynamically enable and disable them.
    dred_menu_item* pOpenFavouriteItem_Default;
    dred_menu_item* pOpenFavouriteItem_TextEditor;

    // The Theme menu items.
    dred_menu_item* pThemesItem_Default;
    dred_menu_item* pThemesItem_TextEditor;


    dred_menu_item* pMenuItem_ToggleTabBars;
    dred_menu_item* pMenuItem_ToggleCmdBarAutoHide;
    dred_menu_item* pMenuItem_ToggleLineNumbers;
    dred_menu_item* pMenuItem_ToggleWordWrap;
};

dr_bool32 dred_menu_library_init(dred_menu_library* pLibrary, dred_context* pDred);
void dred_menu_library_uninit(dred_menu_library* pLibrary);

void dred_menu_library_update_recent_files_menu(dred_menu_library* pLibrary);
void dred_menu_library_update_favourite_files_menu(dred_menu_library* pLibrary);
void dred_menu_library_update_themes_menu(dred_menu_library* pLibrary);
#endif

// Refreshes the Recent Files menu.
void dred_refresh_recent_files_menu(dred_context* pDred);

// Refreshes the Favourite Files menu.
void dred_refresh_favourite_files_menu(dred_context* pDred);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    dtk_uint32 id;
    size_t commandStrOffset;
    size_t shortcutStrOffset;
} dred_menu_item_data;

typedef struct
{
    dred_context* pDred;
    size_t count;
    size_t capacity;
    dred_menu_item_data* pItems;
} dred_menu_item_table;

dred_result dred_menu_item_table_init(dred_context* pDred, dred_menu_item_table* pTable);
dred_result dred_menu_item_table_uninit(dred_menu_item_table* pTable);

dred_result dred_menu_item_table_bind(dred_menu_item_table* pTable, dtk_uint32 id, const char* commandStr, const char* shortcutName);
dred_result dred_menu_item_table_unbind(dred_menu_item_table* pTable, dtk_uint32 id);

// When retrieving the command string. If the menu item does not have it's own command string, an empty
// string will be returned. Returns NULL if any of the inputs are invalid.
const char* dred_menu_item_table_get_command(dred_menu_item_table* pTable, dtk_uint32 id);

// Retrieves the name of the shortcut the menu item is associated with.
const char* dred_menu_item_table_get_shortcut(dred_menu_item_table* pTable, dtk_uint32 id);

// Finds the index of the item with the given ID.
dr_bool32 dred_menu_item_table_find(dred_menu_item_table* pTable, dtk_uint32 id, size_t* pIndex);
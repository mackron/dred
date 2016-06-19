
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

#define DRED_MENU_ITEM_ID_NONE              0
#define DRED_MENU_ITEM_ID_FILE_NEW          1
#define DRED_MENU_ITEM_ID_FILE_OPEN         2
#define DRED_MENU_ITEM_ID_FILE_SAVE         3
#define DRED_MENU_ITEM_ID_FILE_SAVE_AS      4
#define DRED_MENU_ITEM_ID_FILE_SAVE_ALL     5
#define DRED_MENU_ITEM_ID_FILE_CLOSE        6
#define DRED_MENU_ITEM_ID_FILE_CLOSE_ALL    7
#define DRED_MENU_ITEM_ID_FILE_EXIT         8

#define DRED_MENU_ITEM_ID_EDIT_UNDO         9
#define DRED_MENU_ITEM_ID_EDIT_REDO         10
#define DRED_MENU_ITEM_ID_EDIT_CUT          11
#define DRED_MENU_ITEM_ID_EDIT_COPY         12
#define DRED_MENU_ITEM_ID_EDIT_PASTE        13
#define DRED_MENU_ITEM_ID_EDIT_DELETE       14
#define DRED_MENU_ITEM_ID_EDIT_SELECT_ALL   15

#define DRED_MENU_ITEM_ID_VIEW_TAB_BARS     16
#define DRED_MENU_ITEM_ID_VIEW_CMD_BAR      17
#define DRED_MENU_ITEM_ID_VIEW_MENU_BAR     18
#define DRED_MENU_ITEM_ID_VIEW_LINE_NUMBERS 19
#define DRED_MENU_ITEM_ID_VIEW_RESET_ZOOM   20

#define DRED_MENU_ITEM_ID_FIND_FIND         21
#define DRED_MENU_ITEM_ID_FIND_REPLACE      22
#define DRED_MENU_ITEM_ID_FIND_GOTO         23

#define DRED_MENU_ITEM_ID_HELP_HELP         24
#define DRED_MENU_ITEM_ID_HELP_ABOUT        25

#define DRED_MENU_ITEM_ID_RECENT_FILE_0     32768

struct dred_menu_library
{
    // The main context that initialized the library.
    dred_context* pDred;


    // The recent files menu.
    dred_menu* pMenu_RecentFiles;
    size_t recentFileCount;


    // The menu to use when nothing is opened.
    dred_menu* pMenu_Default;

    // The menu to use for text editors.
    dred_menu* pMenu_TextEditor;


    // The right-click popup menu for text editors.
    dred_menu* pPopupMenu_TextEditor;

    // The right-click popup menu for tabs.
    dred_menu* pPopupMenu_Tab;


    // The Open Recent menu items. We need to keep track of these so we can dynamically enable and disable them.
    dred_menu_item* pOpenRecentItem_Default;
    dred_menu_item* pOpenRecentItem_TextEditor;
};

bool dred_menu_library_init(dred_menu_library* pLibrary, dred_context* pDred);
void dred_menu_library_uninit(dred_menu_library* pLibrary);

void dred_menu_library_update_recent_files_menu(dred_menu_library* pLibrary);
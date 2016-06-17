
bool dred_menu_library_init(dred_menu_library* pLibrary, dred_context* pDred)
{
    if (pLibrary == NULL || pDred == NULL) {
        return false;
    }

    memset(pLibrary, 0, sizeof(*pLibrary));

    // The default menu.
    pLibrary->pMenu_Default = dred_menu_create(pDred, dred_menu_type_menubar);
    if (pLibrary->pMenu_Default == NULL) {
        return false;
    }

    dred_menu* pFileMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append(pFileMenu_Default, "&New", DRED_MENU_ITEM_ID_FILE_NEW, "new", dred_shortcut_create_single(dred_accelerator_create('N', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pFileMenu_Default, "&Open...", DRED_MENU_ITEM_ID_FILE_OPEN, "open", dred_shortcut_create_single(dred_accelerator_create('O', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append_separator(pFileMenu_Default);
    dred_menu_item_create_and_append(pFileMenu_Default, "&Save", DRED_MENU_ITEM_ID_FILE_SAVE,  "save",  dred_shortcut_create_single(dred_accelerator_create('S', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pFileMenu_Default, "Save &As...", DRED_MENU_ITEM_ID_FILE_SAVE_AS,  "save-as",  dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append(pFileMenu_Default, "Save A&ll", DRED_MENU_ITEM_ID_FILE_SAVE_ALL,  "save-all",  dred_shortcut_create_single(dred_accelerator_create('S', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), NULL);
    dred_menu_item_create_and_append_separator(pFileMenu_Default);
    dred_menu_item_create_and_append(pFileMenu_Default, "&Close", DRED_MENU_ITEM_ID_FILE_CLOSE, "close", dred_shortcut_create_single(dred_accelerator_create('W', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pFileMenu_Default, "Clos&e All", DRED_MENU_ITEM_ID_FILE_CLOSE_ALL, "close-all", dred_shortcut_create_single(dred_accelerator_create('W', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), NULL);
    dred_menu_item_create_and_append_separator(pFileMenu_Default);
    dred_menu_item_create_and_append(pFileMenu_Default, "E&xit", DRED_MENU_ITEM_ID_FILE_EXIT, "exit", dred_shortcut_create_single(dred_accelerator_create(DRGUI_F4, DRED_KEY_STATE_ALT_DOWN)), NULL);

    dred_menu* pHelpMenu_Default = dred_menu_create(pDred, dred_menu_type_popup);
    //dred_menu_item_create_and_append(pHelpMenu_Default, "View &Help (Opens Web Browser)...", DRED_MENU_ITEM_ID_HELP_HELP, "help", dred_shortcut_create_single(dred_accelerator_create(DRGUI_F1, DRED_KEY_STATE_CTRL_DOWN), NULL);
    //dred_menu_item_create_and_append_separator(pHelpMenu_Default);
    dred_menu_item_create_and_append(pHelpMenu_Default, "&About...", DRED_MENU_ITEM_ID_HELP_ABOUT, "about", dred_shortcut_none(), NULL);

    dred_menu_item_create_and_append(pLibrary->pMenu_Default, "&File", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pFileMenu_Default);
    dred_menu_item_create_and_append(pLibrary->pMenu_Default, "&Help", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pHelpMenu_Default);




    // The text editor menu.
    pLibrary->pMenu_TextEditor = dred_menu_create(pDred, dred_menu_type_menubar);
    if (pLibrary->pMenu_TextEditor == NULL) {
        return false;
    }

    //dred_menu_item_create_and_append_with_shortcut(pFileMenu, "&New", DRED_MENU_ITEM_ID_FILE_NEW, DRED_SHORTCUT_ID_NEW);

    dred_menu* pFileMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append(pFileMenu, "&New", DRED_MENU_ITEM_ID_FILE_NEW, "new", dred_shortcut_create_single(dred_accelerator_create('N', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pFileMenu, "&Open...", DRED_MENU_ITEM_ID_FILE_OPEN, "open", dred_shortcut_create_single(dred_accelerator_create('O', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append(pFileMenu, "&Save", DRED_MENU_ITEM_ID_FILE_SAVE,  "save",  dred_shortcut_create_single(dred_accelerator_create('S', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pFileMenu, "Save &As...", DRED_MENU_ITEM_ID_FILE_SAVE_AS,  "save-as",  dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append(pFileMenu, "Save A&ll", DRED_MENU_ITEM_ID_FILE_SAVE_ALL,  "save-all",  dred_shortcut_create_single(dred_accelerator_create('S', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), NULL);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append(pFileMenu, "&Close", DRED_MENU_ITEM_ID_FILE_CLOSE, "close", dred_shortcut_create_single(dred_accelerator_create('W', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pFileMenu, "Clos&e All", DRED_MENU_ITEM_ID_FILE_CLOSE_ALL, "close-all", dred_shortcut_create_single(dred_accelerator_create('W', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), NULL);
    dred_menu_item_create_and_append_separator(pFileMenu);
    dred_menu_item_create_and_append(pFileMenu, "E&xit", DRED_MENU_ITEM_ID_FILE_EXIT, "exit", dred_shortcut_create_single(dred_accelerator_create(DRGUI_F4, DRED_KEY_STATE_ALT_DOWN)), NULL);

    dred_menu* pEditMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append(pEditMenu, "&Undo", DRED_MENU_ITEM_ID_EDIT_UNDO, "undo", dred_shortcut_create_single(dred_accelerator_create('Z', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pEditMenu, "&Redo", DRED_MENU_ITEM_ID_EDIT_REDO, "redo", dred_shortcut_create_single(dred_accelerator_create('Y', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append_separator(pEditMenu);
    dred_menu_item_create_and_append(pEditMenu, "Cu&t", DRED_MENU_ITEM_ID_EDIT_CUT, "cut", dred_shortcut_create_single(dred_accelerator_create('X', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pEditMenu, "&Copy", DRED_MENU_ITEM_ID_EDIT_COPY, "copy", dred_shortcut_create_single(dred_accelerator_create('C', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pEditMenu, "&Paste", DRED_MENU_ITEM_ID_EDIT_PASTE, "paste", dred_shortcut_create_single(dred_accelerator_create('V', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pEditMenu, "&Delete", DRED_MENU_ITEM_ID_EDIT_DELETE, "delete", dred_shortcut_create_single(dred_accelerator_create(DRGUI_DELETE, 0)), NULL);
    dred_menu_item_create_and_append_separator(pEditMenu);
    dred_menu_item_create_and_append(pEditMenu, "Select &All", DRED_MENU_ITEM_ID_EDIT_SELECT_ALL, "select-all", dred_shortcut_create_single(dred_accelerator_create('A', DRED_KEY_STATE_CTRL_DOWN)), NULL);

    dred_menu* pViewMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append(pViewMenu, "Show &Line Numbers", DRED_MENU_ITEM_ID_VIEW_LINE_NUMBERS, "toggle-line-numbers", dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append_separator(pViewMenu);
    dred_menu_item_create_and_append(pViewMenu, "Reset &Zoom", DRED_MENU_ITEM_ID_VIEW_RESET_ZOOM, "zoom 1", dred_shortcut_none(), NULL);

    dred_menu* pFindMenu = dred_menu_create(pDred, dred_menu_type_popup);
    dred_menu_item_create_and_append(pFindMenu, "&Find...", DRED_MENU_ITEM_ID_FIND_FIND, "cmdbar find-next ", dred_shortcut_create_single(dred_accelerator_create('F', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pFindMenu, "Find and &Replace...", DRED_MENU_ITEM_ID_FIND_REPLACE, "cmdbar replace-all ", dred_shortcut_create_single(dred_accelerator_create('F', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), NULL);
    dred_menu_item_create_and_append_separator(pFindMenu);
    dred_menu_item_create_and_append(pFindMenu, "&Go To...", DRED_MENU_ITEM_ID_FIND_GOTO, "cmdbar goto ", dred_shortcut_create_single(dred_accelerator_create('G', DRED_KEY_STATE_CTRL_DOWN)), NULL);

    dred_menu* pHelpMenu = dred_menu_create(pDred, dred_menu_type_popup);
    //dred_menu_item_create_and_append(pHelpMenu, "View &Help (Opens Web Browser)...", DRED_MENU_ITEM_ID_HELP_HELP, "help", dred_shortcut_create_single(dred_accelerator_create(DRGUI_F1, DRED_KEY_STATE_CTRL_DOWN), NULL);
    //dred_menu_item_create_and_append_separator(pHelpMenu);
    dred_menu_item_create_and_append(pHelpMenu, "&About...", DRED_MENU_ITEM_ID_HELP_ABOUT, "about", dred_shortcut_none(), NULL);

    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&File", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pFileMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&Edit", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pEditMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&View", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pViewMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "F&ind", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pFindMenu);
    dred_menu_item_create_and_append(pLibrary->pMenu_TextEditor, "&Help", DRED_MENU_ITEM_ID_NONE, NULL, dred_shortcut_none(), pHelpMenu);


    // The text editor right-click popup menu.
    pLibrary->pPopupMenu_TextEditor = dred_menu_create(pDred, dred_menu_type_popup);
    if (pLibrary->pPopupMenu_TextEditor == NULL) {
        return false;
    }

    dred_menu_item_create_and_append(pLibrary->pPopupMenu_TextEditor, "&Undo", DRED_MENU_ITEM_ID_EDIT_UNDO, "undo", dred_shortcut_create_single(dred_accelerator_create('Z', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_TextEditor, "&Redo", DRED_MENU_ITEM_ID_EDIT_REDO, "redo", dred_shortcut_create_single(dred_accelerator_create('Y', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_TextEditor);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_TextEditor, "Cu&t", DRED_MENU_ITEM_ID_EDIT_CUT, "cut", dred_shortcut_create_single(dred_accelerator_create('X', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_TextEditor, "&Copy", DRED_MENU_ITEM_ID_EDIT_COPY, "copy", dred_shortcut_create_single(dred_accelerator_create('C', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_TextEditor, "&Paste", DRED_MENU_ITEM_ID_EDIT_PASTE, "paste", dred_shortcut_create_single(dred_accelerator_create('V', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_TextEditor, "&Delete", DRED_MENU_ITEM_ID_EDIT_DELETE, "delete", dred_shortcut_create_single(dred_accelerator_create(DRGUI_DELETE, 0)), NULL);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_TextEditor);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_TextEditor, "Select &All", DRED_MENU_ITEM_ID_EDIT_SELECT_ALL, "select-all", dred_shortcut_create_single(dred_accelerator_create('A', DRED_KEY_STATE_CTRL_DOWN)), NULL);


    pLibrary->pPopupMenu_Tab = dred_menu_create(pDred, dred_menu_type_popup);
    if (pLibrary->pPopupMenu_Tab == NULL) {
        return false;
    }

    dred_menu_item_create_and_append(pLibrary->pPopupMenu_Tab, "&Save", DRED_MENU_ITEM_ID_FILE_SAVE,  "save",  dred_shortcut_create_single(dred_accelerator_create('S', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_Tab, "Save &As...", DRED_MENU_ITEM_ID_FILE_SAVE_AS,  "save-as",  dred_shortcut_none(), NULL);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_Tab, "Save A&ll", DRED_MENU_ITEM_ID_FILE_SAVE_ALL,  "save-all",  dred_shortcut_create_single(dred_accelerator_create('S', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), NULL);
    dred_menu_item_create_and_append_separator(pLibrary->pPopupMenu_Tab);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_Tab, "&Close", DRED_MENU_ITEM_ID_FILE_CLOSE, "close", dred_shortcut_create_single(dred_accelerator_create('W', DRED_KEY_STATE_CTRL_DOWN)), NULL);
    dred_menu_item_create_and_append(pLibrary->pPopupMenu_Tab, "Clos&e All", DRED_MENU_ITEM_ID_FILE_CLOSE_ALL, "close-all", dred_shortcut_create_single(dred_accelerator_create('W', DRED_KEY_STATE_CTRL_DOWN | DRED_KEY_STATE_SHIFT_DOWN)), NULL);

    return true;
}

void dred_menu_library_uninit(dred_menu_library* pLibrary)
{
    if (pLibrary == NULL) {
        return;
    }

    // TODO: Clean up sub-menus. Consider making menus delete their menu items to keep things simple to manage.


    dred_menu_delete(pLibrary->pMenu_TextEditor);
    dred_menu_delete(pLibrary->pMenu_Default);
}
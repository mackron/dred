
bool dred_menu_library_init(dred_menu_library* pLibrary, dred_context* pDred)
{
    if (pLibrary == NULL || pDred == NULL) {
        return false;
    }

    memset(pLibrary, 0, sizeof(*pLibrary));

    // The text editor menu.
    pLibrary->pMenu_TextEditor = dred_menu_create(pDred, dred_menu_type_menubar);
    if (pLibrary->pMenu_TextEditor == NULL) {
        return false;
    }

    return true;
}

void dred_menu_library_uninit(dred_menu_library* pLibrary)
{
    if (pLibrary == NULL) {
        return;
    }
}
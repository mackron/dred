// Copyright (C) 2016 David Reid. See included LICENSE file.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// WIN32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
dtk_result dtk_menu_init__win32(dtk_context* pTK, dtk_menu_type type, dtk_menu* pMenu)
{
    HMENU hMenu = NULL;
    if (type == dred_menu_type_popup) {
        hMenu = CreatePopupMenu();
    } else {
        hMenu = CreateMenu();
    }

    if (hMenu == NULL) {
        return DTK_ERROR;
    }

    pMenu->win32.hMenu = (dtk_handle)hMenu;

    // We want to receive notifications via the WM_MENUCOMMAND event. We need to do this because we want to handle menu-item events based on their
    // position rather than their ID.
    MENUINFO mi;
    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    mi.fMask = MIM_STYLE;
    GetMenuInfo(hMenu, &mi);
    mi.fMask = MIM_STYLE;
    mi.dwStyle |= MNS_NOTIFYBYPOS;      // <-- This is how we make Windows post WM_MENUCOMMAND events instead of WM_COMMAND.
    SetMenuInfo(hMenu, &mi);

    dtk__track_menu(pTK, pMenu);
    return DTK_SUCCESS;
}

dtk_result dtk_menu_uninit__win32(dtk_menu* pMenu)
{
    dtk__untrack_menu(pMenu->pTK, pMenu);
    if (!DestroyMenu(pMenu->win32.hMenu)) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}
#endif  // DTK_WIN32


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// GTK
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_GTK
#endif  // DTK_GTK

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

dtk_result dtk_menu_init(dtk_context* pTK, dtk_menu_type type, dtk_menu* pMenu)
{
    if (pMenu == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pMenu);

    if (pTK == NULL) return DTK_INVALID_ARGS;
    pMenu->pTK = pTK;
    pMenu->type = type;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_menu_init__win32(pTK, type, pMenu);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_menu_init__gtk(pTK, type, pMenu);
    }
#endif
    if (result != DTK_SUCCESS) {
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_menu_uninit(dtk_menu* pMenu)
{
    if (pMenu == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pMenu->pTK->platform == dtk_platform_win32) {
        result = dtk_menu_uninit__win32(pMenu);
    }
#endif
#ifdef DTK_GTK
    if (pMenu->pTK->platform == dtk_platform_gtk) {
        result = dtk_menu_uninit__gtk(pMenu);
    }
#endif

    return result;
}
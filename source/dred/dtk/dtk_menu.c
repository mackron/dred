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
    (void)pTK;

    HMENU hMenu = NULL;
    if (type == dtk_menu_type_popup) {
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
    mi.fMask = MIM_STYLE | MIM_MENUDATA;
    mi.dwStyle |= MNS_NOTIFYBYPOS;      // <-- This is how we make Windows post WM_MENUCOMMAND events instead of WM_COMMAND.
    mi.dwMenuData = (ULONG_PTR)pMenu;
    SetMenuInfo(hMenu, &mi);

    return DTK_SUCCESS;
}

dtk_result dtk_menu_uninit__win32(dtk_menu* pMenu)
{
    if (!DestroyMenu(pMenu->win32.hMenu)) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_menu_get_item_count__win32(dtk_menu* pMenu, dtk_uint32* pCount)
{
    int count = GetMenuItemCount((HMENU)pMenu->win32.hMenu);
    if (count == -1) {
        return DTK_ERROR;
    }

    *pCount = (dtk_uint32)count;
    return DTK_SUCCESS;
}

dtk_result dtk_menu_insert_item__win32(dtk_menu* pMenu, dtk_uint32 index, dtk_menu_item_info* pInfo)
{
    MENUITEMINFOA mii;
    dtk_zero_object(&mii);
    mii.cbSize = sizeof(mii);

    if (pInfo->pSubMenu != NULL) {
        mii.fMask |= MIIM_SUBMENU;
        mii.hSubMenu = (HMENU)pInfo->pSubMenu->win32.hMenu;
    }

    mii.fMask |= MIIM_ID;
    mii.wID = pInfo->id;

    mii.fMask |= MIIM_FTYPE;
    if (pInfo->type == dtk_menu_item_type_separator) {
        mii.fType = MFT_SEPARATOR;
    } else {
        mii.fMask |= MIIM_STRING;
        mii.fType = MFT_STRING;
        mii.cch = strlen(pInfo->text);
        mii.dwTypeData = (LPSTR)pInfo->text;
    }

    mii.fMask |= MIIM_STATE;
    if ((pInfo->stateFlags & DTK_MENU_ITEM_STATE_CHECKED) && pInfo->type == dtk_menu_item_type_check) {
        mii.fState |= MFS_CHECKED;
    }
    if ((pInfo->stateFlags & DTK_MENU_ITEM_STATE_DISABLED)) {
        mii.fState |= MFS_DISABLED;
    }

    BOOL wasSuccessful = InsertMenuItemA((HMENU)pMenu->win32.hMenu, (UINT)index, TRUE, &mii);
    if (!wasSuccessful) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_menu_remove_item__win32(dtk_menu* pMenu, dtk_uint32 index)
{
    if (!RemoveMenu((HMENU)pMenu->win32.hMenu, index, MF_BYPOSITION)) {
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

dtk_result dtk_menu_get_item_count(dtk_menu* pMenu, dtk_uint32* pCount)
{
    if (pCount == NULL) return DTK_INVALID_ARGS;
    *pCount = 0;

    if (pMenu == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pMenu->pTK->platform == dtk_platform_win32) {
        result = dtk_menu_get_item_count__win32(pMenu, pCount);
    }
#endif
#ifdef DTK_GTK
    if (pMenu->pTK->platform == dtk_platform_gtk) {
        result = dtk_menu_get_item_count__gtk(pMenu, pCount);
    }
#endif

    return result;
}


dtk_result dtk_menu_insert_item(dtk_menu* pMenu, dtk_uint32 index, dtk_menu_item_info* pInfo)
{
    if (pMenu == NULL) return DTK_INVALID_ARGS;

    dtk_uint32 itemCount;
    dtk_menu_get_item_count(pMenu, &itemCount);
    if (index > itemCount) {
        return DTK_INVALID_ARGS;
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pMenu->pTK->platform == dtk_platform_win32) {
        result = dtk_menu_insert_item__win32(pMenu, index, pInfo);
    }
#endif
#ifdef DTK_GTK
    if (pMenu->pTK->platform == dtk_platform_gtk) {
        result = dtk_menu_insert_item__gtk(pMenu, index, pInfo);
    }
#endif

    return result;
}

dtk_result dtk_menu_append_item(dtk_menu* pMenu, dtk_menu_item_info* pInfo)
{
    dtk_uint32 itemCount;
    dtk_menu_get_item_count(pMenu, &itemCount);

    return dtk_menu_insert_item(pMenu, itemCount, pInfo);
}

dtk_result dtk_menu_prepend_item(dtk_menu* pMenu, dtk_menu_item_info* pInfo)
{
    return dtk_menu_insert_item(pMenu, 0, pInfo);
}

dtk_result dtk_menu_remove_item(dtk_menu* pMenu, dtk_uint32 index)
{
    if (pMenu == NULL) return DTK_INVALID_ARGS;

    dtk_uint32 itemCount;
    dtk_menu_get_item_count(pMenu, &itemCount);
    if (index >= itemCount) {
        return DTK_INVALID_ARGS;
    }

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pMenu->pTK->platform == dtk_platform_win32) {
        result = dtk_menu_remove_item__win32(pMenu, index);
    }
#endif
#ifdef DTK_GTK
    if (pMenu->pTK->platform == dtk_platform_gtk) {
        result = dtk_menu_remove_item__gtk(pMenu, index);
    }
#endif

    return result;
}

dtk_result dtk_menu_remove_all_items(dtk_menu* pMenu)
{
    if (pMenu == NULL) return DTK_INVALID_ARGS;

    for (;;) {
        dtk_uint32 itemCount;
        dtk_result result = dtk_menu_get_item_count(pMenu, &itemCount);
        if (result != DTK_SUCCESS) {
            return result;
        }

        if (itemCount == 0) {
            break;
        }

        result = dtk_menu_remove_item(pMenu, 0);
        if (result != DTK_SUCCESS) {
            return result;
        }
    }

    return DTK_SUCCESS;
}

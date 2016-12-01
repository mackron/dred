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
    if (!GetMenuInfo(hMenu, &mi)) {
        return DTK_ERROR;
    }

    mi.fMask = MIM_STYLE | MIM_MENUDATA;
    mi.dwStyle |= MNS_NOTIFYBYPOS;      // <-- This is how we make Windows post WM_MENUCOMMAND events instead of WM_COMMAND.
    mi.dwMenuData = (ULONG_PTR)pMenu;
    if (!SetMenuInfo(hMenu, &mi)) {
        return DTK_ERROR;
    }

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

    mii.fMask |= MIIM_DATA;
    mii.dwItemData = (ULONG_PTR)pInfo->pUserData;

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

dtk_result dtk_menu_get_item_user_data__win32(dtk_menu* pMenu, dtk_uint32 index, void** ppUserData)
{
    MENUITEMINFOA mii;
    dtk_zero_object(&mii);
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_DATA;
    BOOL wasSuccessful = GetMenuItemInfoA((HMENU)pMenu->win32.hMenu, index, TRUE, &mii);
    if (!wasSuccessful) {
        return DTK_ERROR;
    }

    *ppUserData = (void*)mii.dwItemData;
    return DTK_SUCCESS;
}

dtk_result dtk_menu_get_item_id__win32(dtk_menu* pMenu, dtk_uint32 index, dtk_uint32* pID)
{
    MENUITEMINFOA mii;
    dtk_zero_object(&mii);
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_ID;
    BOOL wasSuccessful = GetMenuItemInfoA((HMENU)pMenu->win32.hMenu, index, TRUE, &mii);
    if (!wasSuccessful) {
        return DTK_ERROR;
    }

    *pID = mii.wID;
    return DTK_SUCCESS;
}


dtk_result dtk_menu_enable_item__win32(dtk_menu* pMenu, dtk_uint32 index)
{
    MENUITEMINFOA mii;
    dtk_zero_object(&mii);
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fState = MFS_ENABLED;
    BOOL wasSuccessful = SetMenuItemInfoA((HMENU)pMenu->win32.hMenu, index, TRUE, &mii);
    if (!wasSuccessful) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_menu_disable_item__win32(dtk_menu* pMenu, dtk_uint32 index)
{
    MENUITEMINFOA mii;
    dtk_zero_object(&mii);
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fState = MFS_DISABLED;
    BOOL wasSuccessful = SetMenuItemInfoA((HMENU)pMenu->win32.hMenu, index, TRUE, &mii);
    if (!wasSuccessful) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}


dtk_result dtk_menu_set_item_checked__win32(dtk_menu* pMenu, dtk_uint32 index, dtk_bool32 checked)
{
    MENUITEMINFOA mii;
    dtk_zero_object(&mii);
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fState = checked ? MFS_CHECKED : MFS_UNCHECKED;
    BOOL wasSuccessful = SetMenuItemInfoA((HMENU)pMenu->win32.hMenu, index, TRUE, &mii);
    if (!wasSuccessful) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_bool32 dtk_menu_is_item_checked__win32(dtk_menu* pMenu, dtk_uint32 index)
{
    MENUITEMINFOA mii;
    dtk_zero_object(&mii);
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    BOOL wasSuccessful = GetMenuItemInfoA((HMENU)pMenu->win32.hMenu, index, TRUE, &mii);
    if (!wasSuccessful) {
        return DTK_FALSE;
    }

    return (mii.fState & MFS_CHECKED) != 0;
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

// Helper for checking if the given menu item index is valid.
dtk_bool32 dtk_menu__is_item_index_valid(dtk_menu* pMenu, dtk_uint32 index)
{
    dtk_uint32 itemCount;
    dtk_menu_get_item_count(pMenu, &itemCount);

    return index < itemCount;
}

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
    if (pMenu == NULL || !dtk_menu__is_item_index_valid(pMenu, index)) return DTK_INVALID_ARGS;

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


dtk_result dtk_menu_get_item_user_data(dtk_menu* pMenu, dtk_uint32 index, void** ppUserData)
{
    if (ppUserData == NULL) return DTK_INVALID_ARGS;
    *ppUserData = 0;

    if (pMenu == NULL || !dtk_menu__is_item_index_valid(pMenu, index)) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pMenu->pTK->platform == dtk_platform_win32) {
        result = dtk_menu_get_item_user_data__win32(pMenu, index, ppUserData);
    }
#endif
#ifdef DTK_GTK
    if (pMenu->pTK->platform == dtk_platform_gtk) {
        result = dtk_menu_get_item_user_data__gtk(pMenu, index, ppUserData);
    }
#endif

    return result;
}

dtk_result dtk_menu_get_item_id(dtk_menu* pMenu, dtk_uint32 index, dtk_uint32* pID)
{
    if (pID == NULL) return DTK_INVALID_ARGS;
    *pID = 0;

    if (pMenu == NULL || !dtk_menu__is_item_index_valid(pMenu, index)) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pMenu->pTK->platform == dtk_platform_win32) {
        result = dtk_menu_get_item_id__win32(pMenu, index, pID);
    }
#endif
#ifdef DTK_GTK
    if (pMenu->pTK->platform == dtk_platform_gtk) {
        result = dtk_menu_get_item_id__gtk(pMenu, index, pID);
    }
#endif

    return result;
}

dtk_uint32 dtk_menu_find_item_by_id(dtk_menu* pMenu, dtk_uint32 id)
{
    dtk_uint32 itemCount;
    if (dtk_menu_get_item_count(pMenu, &itemCount) != DTK_SUCCESS) {
        return (dtk_uint32)-1;
    }

    for (dtk_uint32 i = 0; i < itemCount; ++i) {
        dtk_uint32 iID;
        if (dtk_menu_get_item_id(pMenu, i, &iID) == DTK_SUCCESS) {
            if (iID == id) {
                return i;
            }
        }
    }

    return (dtk_uint32)-1;
}


dtk_result dtk_menu_enable_item(dtk_menu* pMenu, dtk_uint32 index)
{
    if (pMenu == NULL || !dtk_menu__is_item_index_valid(pMenu, index)) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pMenu->pTK->platform == dtk_platform_win32) {
        result = dtk_menu_enable_item__win32(pMenu, index);
    }
#endif
#ifdef DTK_GTK
    if (pMenu->pTK->platform == dtk_platform_gtk) {
        result = dtk_menu_enable_item__gtk(pMenu, index);
    }
#endif

    return result;
}

dtk_result dtk_menu_disable_item(dtk_menu* pMenu, dtk_uint32 index)
{
    if (pMenu == NULL || !dtk_menu__is_item_index_valid(pMenu, index)) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pMenu->pTK->platform == dtk_platform_win32) {
        result = dtk_menu_disable_item__win32(pMenu, index);
    }
#endif
#ifdef DTK_GTK
    if (pMenu->pTK->platform == dtk_platform_gtk) {
        result = dtk_menu_disable_item__gtk(pMenu, index);
    }
#endif

    return result;
}


dtk_result dtk_menu_check_item(dtk_menu* pMenu, dtk_uint32 index)
{
    return dtk_menu_set_item_checked(pMenu, index, DTK_TRUE);
}

dtk_result dtk_menu_uncheck_item(dtk_menu* pMenu, dtk_uint32 index)
{
    return dtk_menu_set_item_checked(pMenu, index, DTK_FALSE);
}

dtk_result dtk_menu_set_item_checked(dtk_menu* pMenu, dtk_uint32 index, dtk_bool32 checked)
{
    if (pMenu == NULL || !dtk_menu__is_item_index_valid(pMenu, index)) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pMenu->pTK->platform == dtk_platform_win32) {
        result = dtk_menu_set_item_checked__win32(pMenu, index, checked);
    }
#endif
#ifdef DTK_GTK
    if (pMenu->pTK->platform == dtk_platform_gtk) {
        result = dtk_menu_set_item_checked__gtk(pMenu, index, checked);
    }
#endif

    return result;
}

dtk_bool32 dtk_menu_is_item_checked(dtk_menu* pMenu, dtk_uint32 index)
{
    if (pMenu == NULL || !dtk_menu__is_item_index_valid(pMenu, index)) return DTK_INVALID_ARGS;
    
    dtk_bool32 result = DTK_FALSE;
#ifdef DTK_WIN32
    if (pMenu->pTK->platform == dtk_platform_win32) {
        result = dtk_menu_is_item_checked__win32(pMenu, index);
    }
#endif
#ifdef DTK_GTK
    if (pMenu->pTK->platform == dtk_platform_gtk) {
        result = dtk_menu_is_item_checked__gtk(pMenu, index);
    }
#endif

    return result;
}

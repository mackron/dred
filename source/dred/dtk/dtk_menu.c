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
#define DTK_MENU_ITEM_DATA_KEY  "dtk.data"

typedef struct
{
    void* pUserData;
    dtk_menu* pMenu;    // The dtk_menu that owns this item.
    dtk_uint32 id;
    dtk_menu_item_type type;

    // When a check menu item is checked programatically with gtk_check_menu_item_set_active() it results in the "activate" event getting
    // fired which is inconsistent with Win32. This variable is used to block events in these situations.
    dtk_bool32 blockNextActivateSignal;
} dtk_menu_item_extra_data__gtk;

typedef struct
{
    dtk_uint32 index;
    dtk_uint32 count;
    GtkWidget* pItem;
} dtk_menu_item_iterator_data__gtk;

static gboolean dtk_menu__get_item_widget_by_index_cb__gtk(GtkWidget* pItem, dtk_menu_item_iterator_data__gtk* pData)
{
    if (pData->count == pData->index) {
        pData->pItem = pItem;
        return FALSE;
    }

    pData->count += 1;
    return TRUE;
}

static GtkWidget* dtk_menu__get_item_widget_by_index__gtk(dtk_menu* pMenu, dtk_uint32 index)
{
    dtk_menu_item_iterator_data__gtk data;
    data.index = index;
    data.count = 0;
    data.pItem = NULL;
    gtk_container_foreach(GTK_CONTAINER(pMenu->gtk.pWidget), (GtkCallback)dtk_menu__get_item_widget_by_index_cb__gtk, (gpointer)&data);

    return data.pItem;
}

static gboolean dtk_menu__get_item_index_cb__gtk(GtkWidget* pItem, dtk_menu_item_iterator_data__gtk* pData)
{
    if (pData->pItem == pItem) {
        return FALSE;
    }

    pData->index += 1;
    return TRUE;
}

static dtk_uint32 dtk_menu__get_item_index__gtk(dtk_menu* pMenu, GtkWidget* pItem)
{
    dtk_menu_item_iterator_data__gtk data;
    data.index = 0;
    data.count = 0;
    data.pItem = pItem;
    gtk_container_foreach(GTK_CONTAINER(pMenu->gtk.pWidget), (GtkCallback)dtk_menu__get_item_index_cb__gtk, (gpointer)&data);

    return data.index;
}

static gboolean dtk_menu__on_mouse_enter__gtk(GtkWidget* pGTKMenu, GdkEventCrossing* pEvent, gpointer pUserData)
{
    (void)pEvent;
    
    dtk_menu* pMenu = (dtk_menu*)pUserData;
    dtk_assert(pMenu != NULL);

    gdk_window_set_cursor(gtk_widget_get_window(pGTKMenu), pMenu->pTK->gtk.Cursor_Default);
    return DTK_FALSE;
}

dtk_result dtk_menu_init__gtk(dtk_context* pTK, dtk_menu_type type, dtk_menu* pMenu)
{
    GtkWidget* pGTKMenu = NULL;
    if (type == dtk_menu_type_popup) {
        pGTKMenu = gtk_menu_new();
    } else {
        pGTKMenu = gtk_menu_bar_new();
    }

    if (pGTKMenu == NULL) {
        return DTK_ERROR;
    }

    pMenu->gtk.pWidget = (dtk_ptr)pGTKMenu;

    // We want to make sure the cursor is set to the arrow while over the menu bar.
    g_signal_connect(pGTKMenu, "enter-notify-event", G_CALLBACK(dtk_menu__on_mouse_enter__gtk), pMenu);

    // When switching out menus we use the gtk_container_remove() function which decrements the reference counter
    // and may delete the widget. We don't want this behaviour so we just grab a reference to it from here.
    g_object_ref(pGTKMenu);

    return DTK_SUCCESS;
}

dtk_result dtk_menu_uninit__gtk(dtk_menu* pMenu)
{
    gtk_widget_destroy(GTK_WIDGET(pMenu->gtk.pWidget));
    return DTK_SUCCESS;
}

dtk_result dtk_menu_get_item_count__gtk(dtk_menu* pMenu, dtk_uint32* pCount)
{
    *pCount = pMenu->gtk.itemCount;
    return DTK_SUCCESS;
}


static gboolean dtk_menu_item__on_check_menu_item_toggled__gtk(GtkCheckMenuItem *pItem, gpointer pUserData)
{
    (void)pItem;
    (void)pUserData;
    return DTK_FALSE;
}

static void dtk__on_menu_item_activate__gtk(GtkWidget *pItem, gpointer pUserData)
{
    (void)pUserData;
    
    dtk_menu_item_extra_data__gtk* pItemData = g_object_get_data(G_OBJECT(pItem), DTK_MENU_ITEM_DATA_KEY);
    if (pItemData->blockNextActivateSignal) {
        return;
    }
    
    dtk_event e;
    e.pTK = pItemData->pMenu->pTK;
    e.pControl = NULL;
    e.type = DTK_EVENT_MENU;
    e.menu.pMenu = pItemData->pMenu;
    e.menu.itemIndex = dtk_menu__get_item_index__gtk(pItemData->pMenu, pItem);
    dtk__handle_event(&e);
}

dtk_result dtk_menu_insert_item__gtk(dtk_menu* pMenu, dtk_uint32 index, dtk_menu_item_info* pInfo)
{
    GtkWidget* pItem = NULL;

    if (pInfo->type == dtk_menu_item_type_separator) {
        pItem = gtk_separator_menu_item_new();
        if (pItem == NULL) {
            return DTK_ERROR;
        }
    } else {
        // The input string will have "&" characters for the mnemonic symbol, but GTK expects "_".
        char transformedText[256];
        dtk_strncpy_s(transformedText, sizeof(transformedText), pInfo->text, _TRUNCATE);

        dtk_bool32 useMnemonic = DTK_TRUE;
        for (char* c = transformedText; c[0] != '\0'; c += 1) {
            // If the text has an underscore, don't use mnemonics. Otherwise we'll end up with incorrect text.
            if (c[0] == '_') {
                useMnemonic = DTK_FALSE;
                break;
            }

            if (c[0] == '&') {
                c[0] = '_';
                break;
            }
        }

        if (useMnemonic) {
            if (pInfo->type == dtk_menu_item_type_check) {
                pItem = gtk_check_menu_item_new_with_mnemonic(transformedText);
            } else {
                pItem = gtk_menu_item_new_with_mnemonic(transformedText);
            }
        } else {
            if (pInfo->type == dtk_menu_item_type_check) {
                pItem = gtk_check_menu_item_new_with_label(transformedText);
            } else {
                pItem = gtk_menu_item_new_with_label(transformedText);
            }
        }

        if (pItem == NULL) {
            return DTK_ERROR;
        }

        // TODO: Set the accelerator label. See dred_platform_layer.c
    }
    
    if (pInfo->type == dtk_menu_item_type_check) {
        g_signal_connect(pItem, "toggled", G_CALLBACK(dtk_menu_item__on_check_menu_item_toggled__gtk), NULL);
    }

    g_signal_connect(pItem, "activate", G_CALLBACK(dtk__on_menu_item_activate__gtk), pItem);
    gtk_menu_shell_insert(GTK_MENU_SHELL(pMenu->gtk.pWidget), pItem, index);
    gtk_widget_show(pItem);

    if (pInfo->pSubMenu != NULL) {
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(pItem), pInfo->pSubMenu->gtk.pWidget);
    }


    // For now just calloc() the item's extra data, but consider optimizing this later. 
    dtk_menu_item_extra_data__gtk* pItemData = (dtk_menu_item_extra_data__gtk*)calloc(1, sizeof(*pItemData));
    if (pItemData == NULL) {
        gtk_widget_destroy(pItem);
        return DTK_OUT_OF_MEMORY;
    }

    pItemData->pMenu = pMenu;
    pItemData->pUserData = pInfo->pUserData;
    pItemData->id = pInfo->id;
    pItemData->type = pInfo->type;
    g_object_set_data(G_OBJECT(pItem), DTK_MENU_ITEM_DATA_KEY, pItemData);

    pMenu->gtk.itemCount += 1;
    return DTK_SUCCESS;
}

dtk_result dtk_menu_remove_item__gtk(dtk_menu* pMenu, dtk_uint32 index)
{
    GtkWidget* pItem = dtk_menu__get_item_widget_by_index__gtk(pMenu, index);
    if (pItem == NULL) {
        return DTK_ERROR;
    }

    gtk_widget_destroy(pItem);
    free(g_object_get_data(G_OBJECT(pItem), DTK_MENU_ITEM_DATA_KEY));

    dtk_assert(pMenu->gtk.itemCount > 0);
    pMenu->gtk.itemCount -= 1;
    return DTK_SUCCESS;
}

dtk_result dtk_menu_get_item_user_data__gtk(dtk_menu* pMenu, dtk_uint32 index, void** ppUserData)
{
    GtkWidget* pItem = dtk_menu__get_item_widget_by_index__gtk(pMenu, index);
    if (pItem == NULL) {
        return DTK_ERROR;
    }

    *ppUserData = ((dtk_menu_item_extra_data__gtk*)g_object_get_data(G_OBJECT(pItem), DTK_MENU_ITEM_DATA_KEY))->pUserData;
    return DTK_SUCCESS;
}

dtk_result dtk_menu_get_item_id__gtk(dtk_menu* pMenu, dtk_uint32 index, dtk_uint32* pID)
{
    GtkWidget* pItem = dtk_menu__get_item_widget_by_index__gtk(pMenu, index);
    if (pItem == NULL) {
        return DTK_ERROR;
    }

    *pID = ((dtk_menu_item_extra_data__gtk*)g_object_get_data(G_OBJECT(pItem), DTK_MENU_ITEM_DATA_KEY))->id;
    return DTK_SUCCESS;
}


dtk_result dtk_menu_enable_item__gtk(dtk_menu* pMenu, dtk_uint32 index)
{
    GtkWidget* pItem = dtk_menu__get_item_widget_by_index__gtk(pMenu, index);
    if (pItem == NULL) {
        return DTK_ERROR;
    }

    gtk_widget_set_sensitive(pItem, TRUE);
    return DTK_SUCCESS;
}

dtk_result dtk_menu_disable_item__gtk(dtk_menu* pMenu, dtk_uint32 index)
{
    GtkWidget* pItem = dtk_menu__get_item_widget_by_index__gtk(pMenu, index);
    if (pItem == NULL) {
        return DTK_ERROR;
    }

    gtk_widget_set_sensitive(pItem, FALSE);
    return DTK_SUCCESS;
}


dtk_result dtk_menu_set_item_checked__gtk(dtk_menu* pMenu, dtk_uint32 index, dtk_bool32 checked)
{
    GtkWidget* pItem = dtk_menu__get_item_widget_by_index__gtk(pMenu, index);
    if (pItem == NULL) {
        return DTK_ERROR;
    }

    dtk_menu_item_extra_data__gtk* pItemData = g_object_get_data(G_OBJECT(pItem), DTK_MENU_ITEM_DATA_KEY);
    if (pItemData->type != dtk_menu_item_type_check) {
        return DTK_INVALID_ARGS;
    }

    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(pItem)) != checked) {
        pItemData->blockNextActivateSignal = DTK_TRUE;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pItem), checked);
        pItemData->blockNextActivateSignal = DTK_FALSE;
    }

    return DTK_SUCCESS;
}

dtk_bool32 dtk_menu_is_item_checked__gtk(dtk_menu* pMenu, dtk_uint32 index)
{
    GtkWidget* pItem = dtk_menu__get_item_widget_by_index__gtk(pMenu, index);
    if (pItem == NULL) {
        return DTK_FALSE;
    }

    dtk_menu_item_extra_data__gtk* pItemData = g_object_get_data(G_OBJECT(pItem), DTK_MENU_ITEM_DATA_KEY);
    if (pItemData->type != dtk_menu_item_type_check) {
        return DTK_FALSE;
    }

    return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(pItem));
}
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

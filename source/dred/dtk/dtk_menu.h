// Copyright (C) 2018 David Reid. See included LICENSE file.

#define DTK_MENU_ITEM_STATE_CHECKED     (1 << 0)
#define DTK_MENU_ITEM_STATE_DISABLED    (1 << 1)

typedef enum
{
    dtk_menu_type_popup,
    dtk_menu_type_menubar
} dtk_menu_type;

typedef enum
{
    dtk_menu_item_type_normal,
    dtk_menu_item_type_check,
    dtk_menu_item_type_separator
} dtk_menu_item_type;

typedef struct
{
    dtk_uint32 id;
    dtk_menu_item_type type;
    dtk_uint32 stateFlags;      // DTK_MENU_ITEM_STATE_*
    dtk_menu* pSubMenu;
    const char* text;
    const char* shortcut;
    void* pUserData;
} dtk_menu_item_info;

struct dtk_menu
{
    dtk_context* pTK;
    dtk_menu_type type;
    void* pUserData;    // For use by the application.

    union
    {
    #ifdef DTK_WIN32
        struct
        {
            /*HMENU*/ dtk_handle hMenu;
        } win32;
    #endif
    #ifdef DTK_GTK
        struct
        {
            /*GtkWidget**/ dtk_ptr pWidget;
            dtk_uint32 itemCount;
        } gtk;
    #endif
    #ifdef DTK_X11
        struct
        {
        } x11;
    #endif
    };
};

// Initializes a menu.
//
// Note that the location in memory the menu object cannot change because a pointer to the object is
// used for referencing. If this becomes an issue, consider allocating the object with malloc().
dtk_result dtk_menu_init(dtk_context* pTK, dtk_menu_type type, dtk_menu* pMenu);

// Uninitializes a menu.
dtk_result dtk_menu_uninit(dtk_menu* pMenu);


// Retrieves the ID of the item at the given index.
dtk_result dtk_menu_get_item_id(dtk_menu* pMenu, dtk_uint32 index, dtk_uint32* pID);

// Retrieves the index of the first item with the given ID. If the item could not be found, -1 will
// be returned. This runs in linear time.
dtk_uint32 dtk_menu_find_item_by_id(dtk_menu* pMenu, dtk_uint32 id);


// Retrieves the number of menu items making up the given menu.
dtk_result dtk_menu_get_item_count(dtk_menu* pMenu, dtk_uint32* pCount);

// Creates a menu item and inserts it into the given position.
dtk_result dtk_menu_insert_item(dtk_menu* pMenu, dtk_uint32 index, dtk_menu_item_info* pInfo);
dtk_result dtk_menu_append_item(dtk_menu* pMenu, dtk_menu_item_info* pInfo);
dtk_result dtk_menu_prepend_item(dtk_menu* pMenu, dtk_menu_item_info* pInfo);

// Removes a menu item.
dtk_result dtk_menu_remove_item(dtk_menu* pMenu, dtk_uint32 index);
DTK_INLINE dtk_result dtk_menu_remove_item_by_id(dtk_menu* pMenu, dtk_uint32 id) { return dtk_menu_remove_item(pMenu, dtk_menu_find_item_by_id(pMenu, id)); }

// Removes every item from the menu.
dtk_result dtk_menu_remove_all_items(dtk_menu* pMenu);


// Sets the text of a menu item.
dtk_result dtk_menu_set_item_text(dtk_menu* pMenu, dtk_uint32 index, const char* text);
DTK_INLINE dtk_result dtk_menu_set_item_text_by_id(dtk_menu* pMenu, dtk_uint32 id, const char* text) { return dtk_menu_set_item_text(pMenu, dtk_menu_find_item_by_id(pMenu, id), text); }

// Sets the shortcut of a menu item.
dtk_result dtk_menu_set_item_shortcut(dtk_menu* pMenu, dtk_uint32 index, const char* shortcut);
DTK_INLINE dtk_result dtk_menu_set_item_shortcut_by_id(dtk_menu* pMenu, dtk_uint32 id, const char* shortcut) { return dtk_menu_set_item_shortcut(pMenu, dtk_menu_find_item_by_id(pMenu, id), shortcut); }

// Retrieves the user data of the menu item at the given index.
dtk_result dtk_menu_get_item_user_data(dtk_menu* pMenu, dtk_uint32 index, void** ppUserData);
DTK_INLINE dtk_result dtk_menu_get_item_user_data_by_id(dtk_menu* pMenu, dtk_uint32 id, void** ppUserData) { return dtk_menu_get_item_user_data(pMenu, dtk_menu_find_item_by_id(pMenu, id), ppUserData); }



// Enables the menu item at the given index.
dtk_result dtk_menu_enable_item(dtk_menu* pMenu, dtk_uint32 index);
DTK_INLINE dtk_result dtk_menu_enable_item_by_id(dtk_menu* pMenu, dtk_uint32 id) { return dtk_menu_enable_item(pMenu, dtk_menu_find_item_by_id(pMenu, id)); }

// Disables the menu item at the given index.
dtk_result dtk_menu_disable_item(dtk_menu* pMenu, dtk_uint32 index);
DTK_INLINE dtk_result dtk_menu_disable_item_by_id(dtk_menu* pMenu, dtk_uint32 id) { return dtk_menu_disable_item(pMenu, dtk_menu_find_item_by_id(pMenu, id)); }


// Checks the given menu item.
dtk_result dtk_menu_check_item(dtk_menu* pMenu, dtk_uint32 index);
DTK_INLINE dtk_result dtk_menu_check_item_by_id(dtk_menu* pMenu, dtk_uint32 id) { return dtk_menu_check_item(pMenu, dtk_menu_find_item_by_id(pMenu, id)); }

// Unchecks the given menu item.
dtk_result dtk_menu_uncheck_item(dtk_menu* pMenu, dtk_uint32 index);
DTK_INLINE dtk_result dtk_menu_uncheck_item_by_id(dtk_menu* pMenu, dtk_uint32 id) { return dtk_menu_uncheck_item(pMenu, dtk_menu_find_item_by_id(pMenu, id)); }

// Sets whether or not the menu item is checked.
dtk_result dtk_menu_set_item_checked(dtk_menu* pMenu, dtk_uint32 index, dtk_bool32 checked);
DTK_INLINE dtk_result dtk_menu_set_item_checked_by_id(dtk_menu* pMenu, dtk_uint32 id, dtk_bool32 checked) { return dtk_menu_set_item_checked(pMenu, dtk_menu_find_item_by_id(pMenu, id), checked); }

// Determines whether or not the given menu item is checked.
dtk_bool32 dtk_menu_is_item_checked(dtk_menu* pMenu, dtk_uint32 index);
DTK_INLINE dtk_bool32 dtk_menu_is_item_checked_by_id(dtk_menu* pMenu, dtk_uint32 id) { return dtk_menu_is_item_checked(pMenu, dtk_menu_find_item_by_id(pMenu, id)); }



//// Menu Item Info Helpers ////

// Initializes a menu item info structure.
dtk_result dtk_menu_item_info_init(dtk_uint32 id, dtk_menu_item_type type, dtk_uint32 stateFlags, dtk_menu* pSubMenu, const char* text, const char* shortcut, void* pUserData, dtk_menu_item_info* pMII);
dtk_result dtk_menu_item_info_init_simple(dtk_uint32 id, dtk_menu* pSubMenu, const char* text, const char* shortcut, void* pUserData, dtk_menu_item_info* pMII);
dtk_result dtk_menu_item_info_init_separator(dtk_menu_item_info* pMII);
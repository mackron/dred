// Copyright (C) 2016 David Reid. See included LICENSE file.

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
    void* pUserData;
} dtk_menu_item_info;

#if 0
struct dtk_menu_item
{
    dtk_menu* pMenu;        // The menu that owns this item.
    dtk_menu* pSubMenu;     // The sub-menu, if any.
    dtk_uint32 id;          // An application-defined identifier for the menu item.
    dtk_uint32 index;       // The index of the item in the menu that owns it. This includes separators.
    dtk_menu_item_type type;

    union
    {
    #ifdef DTK_WIN32
        struct
        {
            int unused;
        } win32;
    #endif
    #ifdef DTK_GTK
        struct
        {
            /*GtkWidget**/ dtk_ptr pWidget;
        } gtk;
    #endif
    #ifdef DTK_X11
        struct
        {
        } x11;
    #endif
    };
};
#endif

struct dtk_menu
{
    dtk_context* pTK;
    dtk_menu_type type;

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


// Retrieves the number of menu items making up the given menu.
dtk_result dtk_menu_get_item_count(dtk_menu* pMenu, dtk_uint32* pCount);

// Creates a menu item and inserts it into the given position.
dtk_result dtk_menu_insert_item(dtk_menu* pMenu, dtk_uint32 index, dtk_menu_item_info* pInfo);
dtk_result dtk_menu_append_item(dtk_menu* pMenu, dtk_menu_item_info* pInfo);
dtk_result dtk_menu_prepend_item(dtk_menu* pMenu, dtk_menu_item_info* pInfo);

// Removes a menu item.
dtk_result dtk_menu_remove_item(dtk_menu* pMenu, dtk_uint32 index);
dtk_result dtk_menu_remove_all_items(dtk_menu* pMenu);


// Retrieves the user data of the menu item at the given index.
dtk_result dtk_menu_get_item_user_data(dtk_menu* pMenu, dtk_uint32 index, void** ppUserData);

// Retrieves the ID of the item at the given index.
dtk_result dtk_menu_get_item_id(dtk_menu* pMenu, dtk_uint32 index, dtk_uint32* pID);

// Retrieves the index of the first item with the given ID. If the item could not be found, -1 will
// be returned. This runs in linear time.
dtk_uint32 dtk_menu_find_item_by_id(dtk_menu* pMenu, dtk_uint32 id);


// Enables the menu item at the given index.
dtk_result dtk_menu_enable_item(dtk_menu* pMenu, dtk_uint32 index);

// Disables the menu item at the given index.
dtk_result dtk_menu_disable_item(dtk_menu* pMenu, dtk_uint32 index);


// Checks the given menu item.
dtk_result dtk_menu_check_item(dtk_menu* pMenu, dtk_uint32 index);

// Unchecks the given menu item.
dtk_result dtk_menu_uncheck_item(dtk_menu* pMenu, dtk_uint32 index);

// Sets whether or not the menu item is checked.
dtk_result dtk_menu_set_item_checked(dtk_menu* pMenu, dtk_uint32 index, dtk_bool32 checked);

// Determines whether or not the given menu item is checked.
dtk_bool32 dtk_menu_is_item_checked(dtk_menu* pMenu, dtk_uint32 index);
// Copyright (C) 2017 David Reid. See included LICENSE file.

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

dred_result dred_menu_item_table_update_bindings_by_shortcut_name(dred_menu_item_table* pTable, const char* shortcutName, const char* commandStr);

// When retrieving the command string. If the menu item does not have it's own command string, an empty
// string will be returned. Returns NULL if any of the inputs are invalid.
const char* dred_menu_item_table_get_command(dred_menu_item_table* pTable, dtk_uint32 id);

// Retrieves the name of the shortcut the menu item is associated with.
const char* dred_menu_item_table_get_shortcut(dred_menu_item_table* pTable, dtk_uint32 id);

// Finds the index of the item with the given ID.
dtk_bool32 dred_menu_item_table_find(dred_menu_item_table* pTable, dtk_uint32 id, size_t* pIndex);

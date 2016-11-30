// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef enum
{
    dtk_menu_type_popup,
    dtk_menu_type_menubar
} dtk_menu_type;

typedef struct
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
        } gtk;
    #endif
    #ifdef DTK_X11
        struct
        {
        } x11;
    #endif
    };
} dtk_menu;

// Initializes a menu.
//
// Note that the location in memory the menu object cannot change because a pointer to the object is
// used for referencing. If this becomes an issue, consider allocating the object with malloc().
dtk_result dtk_menu_init(dtk_context* pTK, dtk_menu_type type, dtk_menu* pMenu);

// Uninitializes a menu.
dtk_result dtk_menu_uninit(dtk_menu* pMenu);

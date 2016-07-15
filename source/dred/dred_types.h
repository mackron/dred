
typedef struct dred_context dred_context;
typedef struct dred_window dred_window;
typedef struct dred_menu dred_menu;
typedef struct dred_menu_item dred_menu_item;
typedef struct dred_timer dred_timer;
typedef struct dred_accelerator dred_accelerator;
typedef struct dred_accelerator_table dred_accelerator_table;
typedef struct dred_shortcut dred_shortcut;
typedef struct dred_shortcut_table dred_shortcut_table;
typedef struct dred_font dred_font;
typedef struct dred_font_library dred_font_library;
typedef struct dred_image dred_image;
typedef struct dred_image_library dred_image_library;
typedef struct dred_menu_library dred_menu_library;
typedef struct dred_command dred_command;

typedef struct
{
    drgui_color bgColor;
    drgui_color fgColor;
    drgui_font* pFont;
} dred_text_style;
// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef dtk_int32 dtk_dialog_result;
#define DTK_DIALOG_RESULT_OK        1
#define DTK_DIALOG_RESULT_CANCEL    2
#define DTK_DIALOG_RESULT_YES       6
#define DTK_DIALOG_RESULT_NO        7

typedef dtk_uint32 dtk_dialog_buttons;
#define DTK_DIALOG_BUTTONS_OK           (1 << 0)
#define DTK_DIALOG_BUTTONS_OKCANCEL     (1 << 1)
#define DTK_DIALOG_BUTTONS_YESNO        (1 << 2)
#define DTK_DIALOG_BUTTONS_YESNOCANCEL  (1 << 3)

typedef struct
{
    dtk_bool32 fileMustExist;
    dtk_bool32 multiSelect;
    const char* pDefaultPath;
    const char** ppExtensionFilters;    // An array of null-terminated string pairs: {"Name of Filter", "Extension"}. Example: {"All, "", "Text Files", "txt,cfg,ini", "Images", "jpg,jpeg,png,tga", NULL}
} dtk_open_file_dialog_options;

typedef struct
{
    const char* pDefaultPath;
    const char** ppExtensionFilters;    // See note in the dtk_open_file_dialog_options struct.
} dtk_save_file_dialog_options;


// Shows a message box.
dtk_dialog_result dtk_message_box(dtk_window* pParentWindow, const char* text, const char* title, dtk_dialog_buttons buttons);

// Shows the color picker dialog. Returns DTK_DIALOG_RESULT_OK if the user chose a color, DTK_DIALOG_RESULT_CANCEL if they hit the cancel button or an error occurs.
dtk_dialog_result dtk_show_color_picker_dialog(dtk_context* pTK, dtk_window* pOwnerWindow, dtk_color initialColor, dtk_color* pColorOut);


// Shows an open file dialog.
//
// The returned list of strings is null terminated.
dtk_dialog_result dtk_show_open_file_dialog(dtk_window* pParentWindow, dtk_open_file_dialog_options* pOptions, char*** pppSelectedFilePaths);

// Shows a save file dialog.
dtk_dialog_result dtk_show_save_file_dialog(dtk_window* pParentWindow, dtk_save_file_dialog_options* pOptions, char** ppSelectedFilePath);
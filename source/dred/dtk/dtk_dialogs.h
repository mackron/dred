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

// Shows a message box.
dtk_dialog_result dtk_message_box(dtk_window* pParentWindow, const char* text, const char* title, dtk_dialog_buttons buttons);
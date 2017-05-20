// Copyright (C) 2017 David Reid. See included LICENSE file.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// WIN32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
dtk_dialog_result dtk_convert_dialog_result_from_win32(int dialogResultWin32)
{
    switch (dialogResultWin32)
    {
        case IDOK:      return DTK_DIALOG_RESULT_OK;
        case IDCANCEL:  return DTK_DIALOG_RESULT_CANCEL;
        case IDYES:     return DTK_DIALOG_RESULT_YES;
        case IDNO:      return DTK_DIALOG_RESULT_NO;

        case IDABORT:
        case IDRETRY:
        case IDIGNORE:
        case IDTRYAGAIN:
        case IDCONTINUE:
        default: break;
    }

    return 0;
}

UINT dtk_convert_dialog_buttons_to_win32(dtk_dialog_buttons buttons)
{
    switch (buttons)
    {
        case DTK_DIALOG_BUTTONS_OK:          return MB_OK;
        case DTK_DIALOG_BUTTONS_OKCANCEL:    return MB_OKCANCEL;
        case DTK_DIALOG_BUTTONS_YESNO:       return MB_YESNO;
        case DTK_DIALOG_BUTTONS_YESNOCANCEL: return MB_YESNOCANCEL;
        default: break;
    }

    return 0;
}

dtk_dialog_result dtk_message_box__win32(dtk_window* pParentWindow, const char* text, const char* title, dtk_dialog_buttons buttons)
{
    return dtk_convert_dialog_result_from_win32(MessageBoxA((pParentWindow == NULL) ? NULL : (HWND)pParentWindow->win32.hWnd, text, title, dtk_convert_dialog_buttons_to_win32(buttons)));
}
#endif



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// GTK
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_GTK
dtk_dialog_result dtk_message_box__gtk(dtk_window* pParentWindow, const char* text, const char* title, dtk_dialog_buttons buttons)
{
    GtkWidget* pDialog = gtk_message_dialog_new((pParentWindow == NULL) ? NULL : GTK_WINDOW(pParentWindow->gtk.pWidget), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_OTHER, GTK_BUTTONS_NONE, "%s", text);
    gtk_window_set_title(GTK_WINDOW(pDialog), title);

    // Buttons.
    switch (buttons)
    {
        case DTK_DIALOG_BUTTONS_OKCANCEL:
        {
            gtk_dialog_add_button(GTK_DIALOG(pDialog), "OK", DTK_DIALOG_RESULT_OK);
            gtk_dialog_add_button(GTK_DIALOG(pDialog), "Cancel", DTK_DIALOG_RESULT_CANCEL);
        } break;

        case DTK_DIALOG_BUTTONS_YESNO:
        {
            gtk_dialog_add_button(GTK_DIALOG(pDialog), "Yes", DTK_DIALOG_RESULT_YES);
            gtk_dialog_add_button(GTK_DIALOG(pDialog), "No", DTK_DIALOG_RESULT_NO);
        } break;

        case DTK_DIALOG_BUTTONS_YESNOCANCEL:
        {
            gtk_dialog_add_button(GTK_DIALOG(pDialog), "Yes", DTK_DIALOG_RESULT_YES);
            gtk_dialog_add_button(GTK_DIALOG(pDialog), "No", DTK_DIALOG_RESULT_NO);
            gtk_dialog_add_button(GTK_DIALOG(pDialog), "Cancel", DTK_DIALOG_RESULT_CANCEL);
        } break;

        case DTK_DIALOG_BUTTONS_OK:
        default: 
        {
            gtk_dialog_add_button(GTK_DIALOG(pDialog), "OK", DTK_DIALOG_RESULT_OK);
        } break;
    }

    dtk_dialog_result dialogResult = (dtk_dialog_result)gtk_dialog_run(GTK_DIALOG(pDialog));
    gtk_widget_destroy(pDialog);

    return dialogResult;
}
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


dtk_dialog_result dtk_message_box(dtk_window* pParentWindow, const char* text, const char* title, dtk_dialog_buttons buttons)
{
    if (text == NULL) text = "";
    if (title == NULL) title = "";

#ifdef DTK_WIN32
    return dtk_message_box__win32(pParentWindow, text, title, buttons);
#endif
#ifdef DTK_GTK
    return dtk_message_box__gtk(pParentWindow, text, title, buttons);
#endif
}
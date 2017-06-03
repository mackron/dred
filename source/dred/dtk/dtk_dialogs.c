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

dtk_dialog_result dtk_show_color_picker_dialog__win32(dtk_context* pTK, dtk_window* pOwnerWindow, dtk_color initialColor, dtk_color* pColorOut)
{
    dtk_assert(pTK != NULL);

    static COLORREF prevcolors[16] = {
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
    };

    CHOOSECOLORA cc;
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = (HWND)pOwnerWindow->win32.hWnd;
    cc.rgbResult = RGB(initialColor.r, initialColor.g, initialColor.b);
    cc.lpCustColors = prevcolors;
    cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN;

    if (!ChooseColorA(&cc)) {
        return DTK_DIALOG_RESULT_CANCEL;
    }

    pColorOut->r = GetRValue(cc.rgbResult);
    pColorOut->g = GetGValue(cc.rgbResult);
    pColorOut->b = GetBValue(cc.rgbResult);
    pColorOut->a = 255;

    return DTK_DIALOG_RESULT_OK;
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

dtk_dialog_result dtk_show_color_picker_dialog__gtk(dtk_context* pTK, dtk_window* pOwnerWindow, dtk_color initialColor, dtk_color* pColorOut)
{
    GtkWidget* dialog = gtk_color_chooser_dialog_new(NULL, GTK_WINDOW(pOwnerWindow->gtk.pWidget));
    if (dialog == NULL) {
        return DTK_DIALOG_RESULT_CANCEL;
    }

    GdkRGBA rgba;
    rgba.red   = initialColor.r / 255.0;
    rgba.green = initialColor.g / 255.0;
    rgba.blue  = initialColor.b / 255.0;
    rgba.alpha = initialColor.a / 255.0;
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &rgba);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &rgba);
    pColorOut->r = (uint8_t)(rgba.red   * 255);
    pColorOut->g = (uint8_t)(rgba.green * 255);
    pColorOut->b = (uint8_t)(rgba.blue  * 255);
    pColorOut->a = (uint8_t)(rgba.alpha * 255);

    gtk_widget_destroy(dialog);

    if (result == GTK_RESPONSE_OK) {
        return DTK_DIALOG_RESULT_OK;
    } else {
        return DTK_DIALOG_RESULT_CANCEL;
    }
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

dtk_dialog_result dtk_show_color_picker_dialog(dtk_context* pTK, dtk_window* pOwnerWindow, dtk_color initialColor, dtk_color* pColorOut)
{
    if (pTK == NULL) return DTK_DIALOG_RESULT_CANCEL;
   
#ifdef DTK_WIN32
    return dtk_show_color_picker_dialog__win32(pTK, pOwnerWindow, initialColor, pColorOut);
#endif
#ifdef DTK_GTK
    return dtk_show_color_picker_dialog__gtk(pTK, pOwnerWindow, initialColor, pColorOut);
#endif
}
// Copyright (C) 2016 David Reid. See included LICENSE file.

// This is set to a pointer to the dtk_context object that was passed to dred_platform_init() and is only temporary
// while working on the integration phase. Later on this source file will be removed entirely.
dtk_context* g_pTK = NULL;


//////////////////////////////////////////////////////////////////
//
// Private Cross Platform
//
//////////////////////////////////////////////////////////////////

// Helper for creating the root GUI element of a window.
dr_bool32 dred_platform__init_root_gui_element(dred_control* pControl, dred_context* pDred, dred_window* pWindow)
{
    if (!dred_control_init(pControl, pDred, NULL, DTK_CONTROL(&pWindow->windowDTK), "RootGUIControl")) {
        return DR_FALSE;
    }

    pControl->pUserData = pWindow;

    unsigned int width;
    unsigned int height;
    dred_window_get_client_size(pWindow, &width, &height);
    dred_control_set_size(pControl, (float)width, (float)height);

    return DR_TRUE;
}

// Event handler for windows.
static dtk_bool32 dred_dtk_window_event_handler(dtk_event* pEvent)
{
    dred_window* pWindow = (dred_window*)pEvent->pControl->pUserData;
    if (pWindow == NULL) {
        return dtk_window_default_event_handler(pEvent);
    }

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            //dred_control_draw(pWindow->pRootGUIControl, dred_make_rect((float)pEvent->paint.rect.left, (float)pEvent->paint.rect.top, (float)pEvent->paint.rect.right, (float)pEvent->paint.rect.bottom), pEvent->paint.pSurface);
        } break;

        case DTK_EVENT_CLOSE:
        {
            dred_window_on_close(pWindow);
        } break;

        case DTK_EVENT_MOVE:
        {
            dred_window_on_move(pWindow, pEvent->move.x, pEvent->move.y);
        } break;

        case DTK_EVENT_SIZE:
        {
            dred_window_on_size(pWindow, pEvent->size.width, pEvent->size.height);
        } break;

        case DTK_EVENT_MOUSE_ENTER:
        {
            dred_window_on_mouse_enter(pWindow);
        } break;

        case DTK_EVENT_MOUSE_LEAVE:
        {
            dred_window_on_mouse_leave(pWindow);
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            dred_window_on_mouse_move(pWindow, pEvent->mouseMove.x, pEvent->mouseMove.y, pEvent->mouseMove.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        {
            dred_window_on_mouse_button_down(pWindow, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_UP:
        {
            dred_window_on_mouse_button_up(pWindow, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DBLCLICK:
        {
            dred_window_on_mouse_button_dblclick(pWindow, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_WHEEL:
        {
            dred_window_on_mouse_wheel(pWindow, pEvent->mouseWheel.delta, pEvent->mouseWheel.x, pEvent->mouseWheel.y, pEvent->mouseWheel.state);
        } break;

        case DTK_EVENT_KEY_DOWN:
        {
            dred_window_on_key_down(pWindow, pEvent->keyDown.key, pEvent->keyDown.state);
        } break;

        case DTK_EVENT_KEY_UP:
        {
            dred_window_on_key_up(pWindow, pEvent->keyUp.key, pEvent->keyUp.state);
        } break;

        case DTK_EVENT_PRINTABLE_KEY_DOWN:
        {
            dred_window_on_printable_key_down(pWindow, pEvent->printableKeyDown.utf32, pEvent->printableKeyDown.state);
        } break;

        case DTK_EVENT_CAPTURE_KEYBOARD:
        {
            dred_window_on_focus(pWindow);
        } break;

        case DTK_EVENT_RELEASE_KEYBOARD:
        {
            dred_window_on_unfocus(pWindow);
        } break;

        default: break;
    }

    return dtk_window_default_event_handler(pEvent);
}


//////////////////////////////////////////////////////////////////
//
// Win32
//
//////////////////////////////////////////////////////////////////

#ifdef DRED_WIN32
//// Drag and Drop ////
dr_bool32 dred_begin_drag_and_drop__win32(dred_data_type dataType, const void* pData, size_t dataSize)
{
#if 0
    IDataObjectVtbl dataObject;
    dataObject.QueryInterface = NULL;
	dataObject.AddRef = NULL;
	dataObject.Release = NULL;
	dataObject.GetData = NULL;
	dataObject.GetDataHere = NULL;
	dataObject.QueryGetData = NULL;
	dataObject.GetCanonicalFormatEtc = NULL;
	dataObject.SetData = NULL;
	dataObject.EnumFormatEtc = NULL;
	dataObject.DAdvise = NULL;
	dataObject.DUnadvise = NULL;
	dataObject.EnumDAdvise = NULL;
    DoDragDrop();
#endif

    (void)dataType;
    (void)pData;
    (void)dataSize;
    return DR_FALSE;
}
#endif




//////////////////////////////////////////////////////////////////
//
// GTK+ 3
//
//////////////////////////////////////////////////////////////////

#ifdef DRED_GTK
//// Drag and Drop ////
dr_bool32 dred_begin_drag_and_drop__gtk(dred_data_type dataType, const void* pData, size_t dataSize)
{
    (void)dataType;
    (void)pData;
    (void)dataSize;
    return DR_FALSE;
}
#endif




//////////////////////////////////////////////////////////////////
//
// Cross Platform
//
//////////////////////////////////////////////////////////////////

//static void dred_platform__on_global_capture_mouse(dred_control* pControl)
//{
//    dred_window* pWindow = dred_get_control_window(pControl);
//    if (pWindow != NULL) {
//        dtk__capture_mouse_window(&pWindow->pDred->tk, &pWindow->windowDTK);
//    }
//}

//static void dred_platform__on_global_release_mouse(dred_control* pControl)
//{
//    dred_window* pWindow = dred_get_control_window(pControl);
//    if (pWindow != NULL) {
//        dtk__release_mouse_window(&pWindow->pDred->tk);
//    }
//}

//static void dred_platform__on_global_capture_keyboard(dred_control* pControl, dred_control* pPrevCapturedControl)
//{
//    (void)pPrevCapturedControl;
//
//    dred_window* pWindow = dred_get_control_window(pControl);
//    if (pWindow != NULL) {
//        pWindow->pControlWithKeyboardCapture = pControl;
//        dtk__capture_keyboard_window(&pWindow->pDred->tk, &pWindow->windowDTK);
//    }
//}
//
//static void dred_platform__on_global_release_keyboard(dred_control* pControl, dred_control* pNewCapturedControl)
//{
//    dred_window* pWindow = dred_get_control_window(pControl);
//    if (pWindow != NULL) {
//        dred_window* pNewWindow = dred_get_control_window(pNewCapturedControl);
//        if (pWindow != pNewWindow) {
//            dtk__release_keyboard_window(&pWindow->pDred->tk);
//        }
//    }
//}

//static void dred_platform__on_global_dirty(dred_control* pControl, dred_rect relativeRect)
//{
//    dred_window* pWindow = dred_get_control_window(pControl);
//    if (pWindow == NULL) {
//        return;
//    }
//
//    dred_rect absoluteRect = relativeRect;
//    dred_make_rect_absolute(pControl, &absoluteRect);
//
//    dtk_rect absoluteRectDTK;
//    absoluteRectDTK.left   = (dtk_int32)absoluteRect.left;
//    absoluteRectDTK.top    = (dtk_int32)absoluteRect.top;
//    absoluteRectDTK.right  = (dtk_int32)absoluteRect.right;
//    absoluteRectDTK.bottom = (dtk_int32)absoluteRect.bottom;
//    dtk_window_immediate_redraw(&pWindow->windowDTK, absoluteRectDTK);
//}

//static void dred_platform__on_global_change_cursor(dred_control* pControl, dtk_system_cursor_type cursor)
//{
//    dred_window* pWindow = dred_get_control_window(pControl);
//    if (pWindow == NULL) {
//        return;
//    }
//
//    dred_window_set_cursor(pWindow, cursor);
//}

void dred_platform__on_delete_gui_element(dred_control* pControl)
{
    dred_window* pWindow = dred_get_control_window(pControl);
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->pControlWithKeyboardCapture == DTK_CONTROL(pControl)) {
        pWindow->pControlWithKeyboardCapture = NULL;
    }
}


dr_bool32 dred_platform_init(dtk_context* pTK)
{
    g_pTK = pTK;
    return DR_TRUE;
}

void dred_platform_uninit()
{
    g_pTK = NULL;
}

int dred_platform_run()
{
    int exitCode = 0;
    for (;;) {
        dtk_result result = dtk_next_event(g_pTK, DTK_TRUE, &exitCode);  // <-- DTK_TRUE = blocking.
        if (result != DTK_SUCCESS) {
            break;
        }
    }

    return exitCode;
}

void dred_platform_post_quit_message(int resultCode)
{
    dtk_post_quit_event(g_pTK, resultCode);
}

void dred_platform_bind_gui(dred_gui* pGUI)
{
    //dred_gui_set_global_on_capture_mouse(pGUI, dred_platform__on_global_capture_mouse);
    //dred_gui_set_global_on_release_mouse(pGUI, dred_platform__on_global_release_mouse);
    //dred_gui_set_global_on_capture_keyboard(pGUI, dred_platform__on_global_capture_keyboard);
    //dred_gui_set_global_on_release_keyboard(pGUI, dred_platform__on_global_release_keyboard);
    //dred_gui_set_global_on_dirty(pGUI, dred_platform__on_global_dirty);
    //dred_gui_set_global_on_change_cursor(pGUI, dred_platform__on_global_change_cursor);
    dred_gui_set_on_delete_element(pGUI, dred_platform__on_delete_gui_element);
}


void dred_dtk_log_callback(dtk_context* pTK, const char* message)
{
    dred_context* pDred = (dred_context*)pTK->pUserData;
    dred_log(pDred, message);
}

void dred_platform_bind_logging(dred_context* pDred)
{
    if (pDred == NULL) return;
    dtk_set_log_callback(&pDred->tk, dred_dtk_log_callback);
}


dr_bool32 dred_window_create__post_setup(dred_context* pDred, dred_window* pWindow)
{
    pWindow->pDred = pDred;
    pWindow->isShowingMenu = DR_TRUE;
    DTK_CONTROL(&pWindow->windowDTK)->pUserData = pWindow;

    pWindow->pRootGUIControl = &pWindow->rootGUIControl;
    if (!dred_platform__init_root_gui_element(pWindow->pRootGUIControl, pDred, pWindow)) {
        return DR_FALSE;
    }

    return DR_TRUE;
}

dred_window* dred_window_create(dred_context* pDred)
{
    if (pDred == NULL) return NULL;

    dred_window* pWindow = (dred_window*)calloc(1, sizeof(*pWindow));
    if (pWindow == NULL) {
        return NULL;
    }

    if (dtk_window_init(&pDred->tk, NULL, dtk_window_type_toplevel, "dred", 1280, 1024, dred_dtk_window_event_handler, &pWindow->windowDTK) != DTK_SUCCESS) {
        free(pWindow);
        return NULL;
    }

    if (!dred_window_create__post_setup(pDred, pWindow)) {
        dtk_window_uninit(&pWindow->windowDTK);
        free(pWindow);
        return NULL;
    }

    return pWindow;
}

dred_window* dred_window_create_dialog(dred_window* pParentWindow, const char* title, unsigned int width, unsigned int height)
{
    if (pParentWindow == NULL) return NULL; // All dialog windows must be tied to a parent.

    dred_window* pWindow = (dred_window*)calloc(1, sizeof(*pWindow));
    if (pWindow == NULL) {
        return NULL;
    }

    if (dtk_window_init(pParentWindow->windowDTK.control.pTK, DTK_CONTROL(&pParentWindow->windowDTK), dtk_window_type_dialog, title, width, height, dred_dtk_window_event_handler, &pWindow->windowDTK) != DTK_SUCCESS) {
        free(pWindow);
        return NULL;
    }

    if (!dred_window_create__post_setup(pParentWindow->pDred, pWindow)) {
        dtk_window_uninit(&pWindow->windowDTK);
        free(pWindow);
        return NULL;
    }

    return pWindow;
}

dred_window* dred_window_create_popup(dred_window* pParentWindow, unsigned int width, unsigned int height)
{
    if (pParentWindow == NULL) return NULL; // All popup windows must be tied to a parent.

    dred_window* pWindow = (dred_window*)calloc(1, sizeof(*pWindow));
    if (pWindow == NULL) {
        return NULL;
    }

    if (dtk_window_init(pParentWindow->windowDTK.control.pTK, DTK_CONTROL(&pParentWindow->windowDTK), dtk_window_type_popup, "", width, height, dred_dtk_window_event_handler, &pWindow->windowDTK) != DTK_SUCCESS) {
        free(pWindow);
        return NULL;
    }

    if (!dred_window_create__post_setup(pParentWindow->pDred, pWindow)) {
        dtk_window_uninit(&pWindow->windowDTK);
        free(pWindow);
        return NULL;
    }

    return pWindow;
}

void dred_window_delete(dred_window* pWindow)
{
    if (pWindow == NULL) return;

    if (pWindow->pRootGUIControl) {
        dred_control_uninit(pWindow->pRootGUIControl);
        pWindow->pRootGUIControl = NULL;
    }

    dtk_window_uninit(&pWindow->windowDTK);
    free(pWindow);
}


void dred_window_set_title(dred_window* pWindow, const char* title)
{
    if (pWindow == NULL) return;
    dtk_window_set_title(&pWindow->windowDTK, title);
}


void dred_window_set_size(dred_window* pWindow, unsigned int newWidth, unsigned int newHeight)
{
    if (pWindow == NULL);
    dtk_window_set_size(&pWindow->windowDTK, newWidth, newHeight);
}

void dred_window_get_size(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
    if (pWidthOut) *pWidthOut = 0;
    if (pHeightOut) *pHeightOut = 0;
    if (pWindow == NULL) return;
    dtk_window_get_size(&pWindow->windowDTK, pWidthOut, pHeightOut);
}


void dred_window_set_position(dred_window* pWindow, int posX, int posY)
{
    if (pWindow == NULL) return;
    dtk_window_set_absolute_position(&pWindow->windowDTK, posX, posY);
}

void dred_window_get_position(dred_window* pWindow, int* pPosXOut, int* pPosYOut)
{
    if (pPosXOut) *pPosXOut = 0;
    if (pPosYOut) *pPosYOut = 0;
    if (pWindow == NULL) return;
    dtk_window_get_absolute_position(&pWindow->windowDTK, pPosXOut, pPosYOut);
}


void dred_window_get_client_size(dred_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
    if (pWidthOut) *pWidthOut = 0;
    if (pHeightOut) *pHeightOut = 0;
    if (pWindow == NULL) return;
    dtk_window_get_client_size(&pWindow->windowDTK, pWidthOut, pHeightOut);
}

void dred_window_get_client_position(dred_window* pWindow, int* pPosXOut, int* pPosYOut)
{
    dtk_window_get_client_absolute_position(&pWindow->windowDTK, pPosXOut, pPosYOut);
}


void dred_window_move_to_center(dred_window* pWindow)
{
    if (pWindow == NULL) return;
    dtk_window_move_to_center(&pWindow->windowDTK);
}


void dred_window_show(dred_window* pWindow)
{
    if (pWindow == NULL) return;
    dtk_window_show(&pWindow->windowDTK, DTK_SHOW_NORMAL);
}

void dred_window_show_maximized(dred_window* pWindow)
{
    if (pWindow == NULL) return;
    dtk_window_show(&pWindow->windowDTK, DTK_SHOW_MAXIMIZED);
}

void dred_window_show_sized(dred_window* pWindow, unsigned int width, unsigned int height)
{
    dred_window_set_size(pWindow, width, height);
    dred_window_show(pWindow);
}

void dred_window_hide(dred_window* pWindow)
{
    if (pWindow == NULL) return;
    dtk_window_hide(&pWindow->windowDTK);
}

void dred_window_bring_to_top(dred_window* pWindow)
{
    if (pWindow == NULL) return;
    dtk_window_bring_to_top(&pWindow->windowDTK);
}

dr_bool32 dred_window_is_maximized(dred_window* pWindow)
{
    if (pWindow == NULL) return DR_FALSE;
    return dtk_window_is_maximized(&pWindow->windowDTK);
}


void dred_window_set_cursor(dred_window* pWindow, dtk_system_cursor_type cursor)
{
    if (pWindow == NULL) return;
    dtk_window_set_cursor(&pWindow->windowDTK, cursor);
}

dr_bool32 dred_window_is_cursor_over(dred_window* pWindow)
{
    if (pWindow == NULL) return DR_FALSE;
    return dtk_window_is_cursor_over(&pWindow->windowDTK);
}


void dred_window_set_menu(dred_window* pWindow, dtk_menu* pMenu)
{
    if (pWindow == NULL) return;
    if (pWindow->pMenu == pMenu) {
        return; // It's the same menu.
    }

    if (pWindow->isShowingMenu && dtk_window_set_menu(&pWindow->windowDTK, pMenu) != DTK_SUCCESS) {
        return; // Failed to set the menu.
    }

    pWindow->pMenu = pMenu;
}

void dred_window_hide_menu(dred_window* pWindow)
{
    if (pWindow == NULL) return;
    if (!dred_window_is_showing_menu(pWindow)) {
        return;
    }

    if (pWindow->pMenu != NULL) {
        dtk_window_set_menu(&pWindow->windowDTK, NULL);
    }

    pWindow->isShowingMenu = DR_FALSE;
}

void dred_window_show_menu(dred_window* pWindow)
{
    if (pWindow == NULL) return;
    if (dred_window_is_showing_menu(pWindow)) {
        return;
    }

    if (pWindow->pMenu != NULL) {
        dtk_window_set_menu(&pWindow->windowDTK, pWindow->pMenu);
    }

    pWindow->isShowingMenu = DR_TRUE;
}

dr_bool32 dred_window_is_showing_menu(dred_window* pWindow)
{
    if (pWindow == NULL) return DR_FALSE;
    return pWindow->isShowingMenu;
}

void dred_window_show_popup_menu(dred_window* pWindow, dtk_menu* pMenu, int posX, int posY)
{
    if (pWindow == NULL) return;
    dtk_window_show_popup_menu(&pWindow->windowDTK, pMenu, posX, posY);
}

void dred_window_send_ipc_message_event(dred_window* pWindow, unsigned int messageID, const void* pMessageData, size_t messageDataSize)
{
    if (pWindow == NULL) return;
    dtk_post_custom_event(DTK_CONTROL(&pWindow->windowDTK)->pTK, DTK_CONTROL(&pWindow->windowDTK), messageID, pMessageData, messageDataSize);
}


void dred_window_on_close(dred_window* pWindow)
{
    if (pWindow->onClose) {
        pWindow->onClose(pWindow);
    }
}

dr_bool32 dred_window_on_hide(dred_window* pWindow, unsigned int flags)
{
    if (pWindow->onHide) {
        return pWindow->onHide(pWindow, flags);
    }

    return DR_TRUE;    // Returning DR_TRUE means to process the message as per normal.
}

dr_bool32 dred_window_on_show(dred_window* pWindow)
{
    if (pWindow->onShow) {
        return pWindow->onShow(pWindow);
    }

    return DR_TRUE;    // Returning DR_TRUE means to process the message as per normal.
}

void dred_window_on_activate(dred_window* pWindow)
{
    if (pWindow->onActivate) {
        pWindow->onActivate(pWindow);
    }
}

void dred_window_on_deactivate(dred_window* pWindow)
{
    if (pWindow->onDeactivate) {
        pWindow->onDeactivate(pWindow);
    }
}

void dred_window_on_size(dred_window* pWindow, unsigned int newWidth, unsigned int newHeight)
{
    if (pWindow->onSize) {
        pWindow->onSize(pWindow, newWidth, newHeight);
    }

    // Always resize the root GUI element so that it's the exact same size as the window.
    dred_control_set_size(pWindow->pRootGUIControl, (float)newWidth, (float)newHeight);
}

void dred_window_on_move(dred_window* pWindow, int newPosX, int newPosY)
{
    if (pWindow->onMove) {
        pWindow->onMove(pWindow, newPosX, newPosY);
    }
}

void dred_window_on_mouse_enter(dred_window* pWindow)
{
    if (pWindow->onActivate) {
        pWindow->onActivate(pWindow);
    }
}

void dred_window_on_mouse_leave(dred_window* pWindow)
{
    if (pWindow->onMouseLeave) {
        pWindow->onMouseLeave(pWindow);
    }

    //dred_gui_post_inbound_event_mouse_leave(pWindow->pRootGUIControl);
}

void dred_window_on_mouse_move(dred_window* pWindow, int mousePosX, int mousePosY, unsigned int stateFlags)
{
    if (pWindow->onMouseMove) {
        pWindow->onMouseMove(pWindow, mousePosX, mousePosY, stateFlags);
    }

    //dred_gui_post_inbound_event_mouse_move(pWindow->pRootGUIControl, mousePosX, mousePosY, stateFlags);
}

void dred_window_on_mouse_button_down(dred_window* pWindow, int mouseButton, int mousePosX, int mousePosY, unsigned int stateFlags)
{
    if (pWindow->onMouseButtonDown) {
        pWindow->onMouseButtonDown(pWindow, mouseButton, mousePosX, mousePosY, stateFlags);
    }

    //dred_gui_post_inbound_event_mouse_button_down(pWindow->pRootGUIControl, mouseButton, mousePosX, mousePosY, stateFlags);
}

void dred_window_on_mouse_button_up(dred_window* pWindow, int mouseButton, int mousePosX, int mousePosY, unsigned int stateFlags)
{
    if (pWindow->onMouseButtonUp) {
        pWindow->onMouseButtonUp(pWindow, mouseButton, mousePosX, mousePosY, stateFlags);
    }

    //dred_gui_post_inbound_event_mouse_button_up(pWindow->pRootGUIControl, mouseButton, mousePosX, mousePosY, stateFlags);
}

void dred_window_on_mouse_button_dblclick(dred_window* pWindow, int mouseButton, int mousePosX, int mousePosY, unsigned int stateFlags)
{
    if (pWindow->onMouseButtonDblClick) {
        pWindow->onMouseButtonDblClick(pWindow, mouseButton, mousePosX, mousePosY, stateFlags);
    }

    //dred_gui_post_inbound_event_mouse_button_dblclick(pWindow->pRootGUIControl, mouseButton, mousePosX, mousePosY, stateFlags);
}

void dred_window_on_mouse_wheel(dred_window* pWindow, int delta, int mousePosX, int mousePosY, unsigned int stateFlags)
{
    if (pWindow->onMouseWheel) {
        pWindow->onMouseWheel(pWindow, delta, mousePosX, mousePosY, stateFlags);
    }

    //dred_gui_post_inbound_event_mouse_wheel(pWindow->pRootGUIControl, delta, mousePosX, mousePosY, stateFlags);
}

void dred_window_on_key_down(dred_window* pWindow, dtk_key key, unsigned int stateFlags)
{
    if (pWindow->onKeyDown) {
        pWindow->onKeyDown(pWindow, key, stateFlags);
    }

    //if (pWindow->pRootGUIControl) {
    //    dred_gui_post_inbound_event_key_down(pWindow->pRootGUIControl->pGUI, key, stateFlags);
    //}
}

void dred_window_on_key_up(dred_window* pWindow, dtk_key key, unsigned int stateFlags)
{
    if (pWindow->onKeyUp) {
        pWindow->onKeyUp(pWindow, key, stateFlags);
    }

    /*if (pWindow->pRootGUIControl) {
        dred_gui_post_inbound_event_key_up(pWindow->pRootGUIControl->pGUI, key, stateFlags);
    }*/
}

void dred_window_on_printable_key_down(dred_window* pWindow, unsigned int character, unsigned int stateFlags)
{
    if (pWindow->onPrintableKeyDown) {
        pWindow->onPrintableKeyDown(pWindow, character, stateFlags);
    }

    /*if (pWindow->pRootGUIControl) {
        dred_gui_post_inbound_event_printable_key_down(pWindow->pRootGUIControl->pGUI, character, stateFlags);
    }*/
}

void dred_window_on_focus(dred_window* pWindow)
{
    if (pWindow->onFocus) {
        pWindow->onFocus(pWindow);
    }

    // Make sure the GUI element is given the keyboard capture if it hasn't already got it.
    /*if (!dred_control_has_keyboard_capture(pWindow->pControlWithKeyboardCapture)) {
        dred_gui_capture_keyboard(pWindow->pControlWithKeyboardCapture);
    }*/
}

void dred_window_on_unfocus(dred_window* pWindow)
{
    if (pWindow->onUnfocus) {
        pWindow->onUnfocus(pWindow);
    }

    // Make sure the GUI element is released of the keyboard capture, but don't clear the variable.
    /*if (dred_control_has_keyboard_capture(pWindow->pControlWithKeyboardCapture)) {
        dred_gui_release_keyboard(pWindow->pDred->pGUI);
    }*/
}

void dred_window_on_ipc_message(dred_window* pWindow, unsigned int messageID, const void* pMessageData)
{
    if (pWindow->onIPCMessage) {
        pWindow->onIPCMessage(pWindow, messageID, pMessageData);
    }
}


void dred_window__stock_event__hide_on_close(dred_window* pWindow)
{
    assert(pWindow != NULL);
    dred_window_hide(pWindow);
}


dred_window* dred_get_control_window(dred_control* pControl)
{
    dtk_window* pWindow = dtk_control_get_window(DTK_CONTROL(pControl));
    if (pWindow == NULL) {
        return NULL;
    }

    return (dred_window*)pWindow;

    /*if (pControl == NULL) {
        return NULL;
    }

    dred_control* pRootGUIControl = dred_control_find_top_level_control(pControl);
    if (pRootGUIControl == NULL) {
        return NULL;
    }

    if (!dred_control_is_of_type(pRootGUIControl, "RootGUIControl")) {
        return NULL;
    }

    return (dred_window*)pRootGUIControl->pUserData;*/
}


//// Drag and Drop ////

dr_bool32 dred_begin_drag_and_drop(dred_data_type dataType, const void* pData, size_t dataSize)
{
    if (pData == NULL) return DR_FALSE;

#ifdef DRED_WIN32
    return dred_begin_drag_and_drop__win32(dataType, pData, dataSize);
#endif
#ifdef DRED_GTK
    return dred_begin_drag_and_drop__gtk(dataType, pData, dataSize);
#endif
}

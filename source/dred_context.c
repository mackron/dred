
static void dred_window_cb__on_main_window_close(dred_window* pWindow)
{
    dred_platform_post_quit_message(0);
}

static void dred_window_cb__on_main_window_paint_TEMP(drgui_element* pElement, drgui_rect rect, void* pPaintData)
{
    //drgui_draw_rect_with_outline(pElement, drgui_get_local_rect(pElement), drgui_rgb(255, 255, 255), 4, drgui_rgb(0, 0, 0), pPaintData);
    //drgui_draw_rect(pElement, drgui_get_local_rect(pElement), drgui_rgb(180, 255, 255), pPaintData);
    
    drgui_draw_rect(pElement, drgui_get_local_rect(pElement), drgui_rgb(180, 255, 255), pPaintData);
    drgui_draw_rect_outline(pElement, drgui_get_local_rect(pElement), drgui_rgb(255, 128, 128), 4, pPaintData);
}
static void dred_window_cb__on_main_window_size_TEMP(drgui_element* pElement, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;
    //drgui_dirty(pElement, drgui_get_local_rect(pElement));
    //printf("SIZING %d %d ", (int)pElement->width, (int)pElement->height);
}


bool dred_init(dred_context* pDred, dr_cmdline cmdline)
{
    if (pDred == NULL) {
        return false;
    }

    pDred->cmdline = cmdline;


    // The drawing context.
#ifdef DRED_WIN32
    pDred->pDrawingContext = dr2d_create_context_gdi();
#endif
#ifdef DRED_GTK
    pDred->pDrawingContext = dr2d_create_context_cairo();
#endif
    if (pDred->pDrawingContext == NULL) {
        return false;
    }


    // The GUI.
    pDred->pGUI = drgui_create_context_dr_2d(pDred->pDrawingContext);
    if (pDred->pGUI == NULL) {
        dr2d_delete_context(pDred->pDrawingContext);
        return false;
    }

    // The GUI needs to be linked to the window system.
    dred_platform_bind_gui(pDred->pGUI);



    // The main window.
    pDred->pMainWindow = dred_window_create(pDred);
    if (pDred->pMainWindow == NULL) {
        printf("Failed to create main window.");
        return false;
    }

    dred_window_set_size(pDred->pMainWindow, 200, 200);

    pDred->pMainWindow->onClose = dred_window_cb__on_main_window_close;
    drgui_set_on_paint(pDred->pMainWindow->pRootGUIElement, dred_window_cb__on_main_window_paint_TEMP);
    drgui_set_on_size(pDred->pMainWindow->pRootGUIElement, dred_window_cb__on_main_window_size_TEMP);

    // Show the window as soon as possible to give it the illusion of loading quickly.
    dred_window_show(pDred->pMainWindow);

    return true;
}

void dred_uninit(dred_context* pDred)
{
    if (pDred == NULL) {
        return;
    }

    dred_window_delete(pDred->pMainWindow);
}

int dred_run(dred_context* pDred)
{
    if (pDred == NULL) {
        return -1;
    }

    return dred_platform_run(pDred);
}
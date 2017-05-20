// Copyright (C) 2017 David Reid. See included LICENSE file.

// TODO: Move this to autogen.
static const struct {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned char	 pixel_data[23 * 7 * 4 + 1];
} g_LogoBannerImage = {
  23, 7, 4,
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0"
  "\0\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\0\0\0\0\377\0"
  "\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\0\0\0\0\377\0\0\0\377\0\0\0"
  "\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0"
  "\0\0\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0\377\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\377\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0\377\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0\0\377\0\0\0\377\0\0"
  "\0\377\0\0\0\377\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0"
  "\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\0\0\0\377\0\0\0\377\0\0"
  "\0\377\0\0\0\377\0\0\0\377\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\0\0"
  "\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377",
};

void dred_about_dialog__on_size(dred_control* pControl, float newWidth, float newHeight)
{
    dred_window* pWindow = dred_get_control_window(pControl);
    if (pWindow == NULL) {
        return;
    }

    dred_about_dialog* pDialog = (dred_about_dialog*)pWindow->pUserData;
    assert(pDialog != NULL);

    dred_context* pDred = pWindow->pDred;
    assert(pDialog != NULL);

    // The close button needs to be repositioned.
    dred_control_set_relative_position(DRED_CONTROL(&pDialog->closeButton),
        newWidth - dred_control_get_width(DRED_CONTROL(&pDialog->closeButton)) - 8*pDred->uiScale,
        newHeight - dred_control_get_height(DRED_CONTROL(&pDialog->closeButton)) - 8*pDred->uiScale);
}

void dred_about_dialog__on_paint(dred_control* pControl, dred_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dred_window* pWindow = dred_get_control_window(pControl);
    if (pWindow == NULL) {
        return;
    }

    dred_about_dialog* pDialog = (dred_about_dialog*)pWindow->pUserData;
    assert(pDialog != NULL);

    float uiScale = (float)pWindow->pDred->uiScale;

    dred_rect dialogRect = dred_control_get_local_rect(pControl);

    dred_control_draw_rect(pControl, dialogRect, dred_rgb(255, 255, 255), pSurface);

    unsigned int logoWidth;
    unsigned int logoHeight;
    dred_gui_get_image_size(pDialog->pLogo, &logoWidth, &logoHeight);

    dred_rect bannerRect = dialogRect;
    bannerRect.bottom = (logoHeight*10 + 80.0f) * uiScale;

    dred_gui_draw_image_args args;
    memset(&args, 0, sizeof(args));
    args.srcWidth = (float)logoWidth;
    args.srcHeight = (float)logoHeight;
    args.dstWidth = args.srcWidth*10 * uiScale;
    args.dstHeight = args.srcHeight*10 * uiScale;
    args.dstX = 32;
    args.dstY = 32;
    args.dstBoundsX = bannerRect.left;
    args.dstBoundsY = bannerRect.top;
    args.dstBoundsWidth = bannerRect.right - bannerRect.left;
    args.dstBoundsHeight = bannerRect.bottom - bannerRect.top;
    args.options = DRED_GUI_IMAGE_ALIGN_CENTER;
    args.foregroundTint = dred_rgb(255, 255, 255);
    args.backgroundColor = dred_rgb(255, 255, 255);
    dred_control_draw_image(pControl, pDialog->pLogo, &args, pSurface);



    dred_gui_font* pFont = dred_font_acquire_subfont(pWindow->pDred->config.pUIFont, uiScale);
    dtk_font_metrics fontMetrics;
    dred_gui_get_font_metrics(pFont, &fontMetrics);

    float penPosX = 0;
    float penPosY = bannerRect.bottom - (fontMetrics.lineHeight*1.5f);

    const char* linkStr = "http://dred.io";

    float linkWidth;
    dred_gui_measure_string(pFont, linkStr, strlen(linkStr), &linkWidth, NULL);
    dred_control_draw_text(pControl, pFont, linkStr, (int)strlen(linkStr), ((dialogRect.right - dialogRect.left) - linkWidth) / 2, penPosY, dred_rgb(0, 0, 0), dred_rgb(255, 255, 255), pSurface);


    const char* versionStr = "dred version " DRED_VERSION_STRING;
    const char* copyrightStr = "Copyright \xC2\xA9 2016 David Reid";
    dred_font_release_subfont(pWindow->pDred->config.pUIFont, pFont);

    penPosX = (8*uiScale);
    penPosY = bannerRect.bottom;

    dred_control_draw_rect(pControl, dred_make_rect(0, bannerRect.bottom, dialogRect.right, penPosY + (1 * uiScale)), dred_rgb(200, 200, 200), pSurface);
    penPosY += (9*uiScale);

    dred_control_draw_text(pControl, pFont, versionStr, (int)strlen(versionStr), penPosX, penPosY, dred_rgb(0, 0, 0), dred_rgb(255, 255, 255), pSurface);
    penPosY += fontMetrics.lineHeight;

    dred_control_draw_text(pControl, pFont, copyrightStr, (int)strlen(copyrightStr), penPosX, penPosY, dred_rgb(0, 0, 0), dred_rgb(255, 255, 255), pSurface);
    penPosY += fontMetrics.lineHeight;
    
    penPosY += (9*uiScale);
    dred_control_draw_rect(pControl, dred_make_rect(0, penPosY, dialogRect.right, penPosY + (1 * uiScale)), dred_rgb(200, 200, 200), pSurface);
    penPosY += 9*uiScale;

    const char* creditsTitle = "The following libraries are used internally by dred:";
    dred_control_draw_text(pControl, pFont, creditsTitle, (int)strlen(creditsTitle), penPosX, penPosY, dred_rgb(0, 0, 0), dred_rgb(255, 255, 255), pSurface);
    penPosY += 4*uiScale + fontMetrics.lineHeight;

    static const char* credits[] = {
        "    dr_libs (https://github.com/mackron/dr_libs)",
        "    gb_string (https://github.com/gingerBill/gb)",
        "    nanosvg (https://github.com/memononen/nanosvg)"
    };

    size_t creditsCount = sizeof(credits) / sizeof(credits[0]);

    for (size_t iCredit = 0; iCredit < creditsCount; ++iCredit) {
        dred_control_draw_text(pControl, pFont, credits[iCredit], (int)strlen(credits[iCredit]), penPosX, penPosY, dred_rgb(0, 0, 0), dred_rgb(255, 255, 255), pSurface);
        penPosY += fontMetrics.lineHeight;
    }
}

void dred_about_dialog__on_window_close(dred_window* pWindow)
{
    dred_about_dialog* pDialog = (dred_about_dialog*)pWindow->pUserData;
    assert(pDialog != NULL);
    (void)pDialog;

    dred_context* pDred = pWindow->pDred;
    assert(pDred != NULL);

    assert(pDred->pAboutDialog == pDialog);
    dred_about_dialog_delete(pDred->pAboutDialog);
    pDred->pAboutDialog = NULL;
}

/*void dred_about_dialog__on_key_down(dred_window* pWindow, dtk_key key, unsigned int stateFlags)
{
    (void)stateFlags;

    dred_about_dialog* pDialog = (dred_about_dialog*)pWindow->pUserData;
    assert(pDialog != NULL);

    if (key == DRED_GUI_ESCAPE) {
        dred_about_dialog__on_window_close(pWindow);
    }
}*/

void dred_about_dialog__btn_close__on_pressed(dred_button* pButton)
{
    dred_window* pWindow = dred_get_control_window(DRED_CONTROL(pButton));
    if (pWindow == NULL) {
        return; // Should never hit this, but leaving here for sanity.
    }

    dred_about_dialog__on_window_close(pWindow);
}

dred_about_dialog* dred_about_dialog_create(dred_context* pDred)
{
    if (pDred == NULL) {
        return NULL;
    }

    dred_about_dialog* pDialog = (dred_about_dialog*)calloc(1, sizeof(*pDialog));
    if (pDialog == NULL) {
        return NULL;
    }

    unsigned int windowWidth = (unsigned int)(480*pDred->uiScale);
    unsigned int windowHeight = (unsigned int)(360*pDred->uiScale);

    pDialog->pWindow = dred_window_create_dialog(pDred->pMainWindow, "About", windowWidth, windowHeight);
    if (pDialog->pWindow == NULL) {
        free(pDialog);
        return NULL;
    }
    
    pDialog->pWindow->pUserData = pDialog;
    pDialog->pWindow->onClose = dred_about_dialog__on_window_close;
    //pDialog->pWindow->onKeyDown = dred_about_dialog__on_key_down;
    dred_control_set_on_size(pDialog->pWindow->pRootGUIControl, dred_about_dialog__on_size);
    dred_control_set_on_paint(pDialog->pWindow->pRootGUIControl, dred_about_dialog__on_paint);
    


    pDialog->pLogo = dred_gui_create_image(pDred->pGUI, g_LogoBannerImage.width, g_LogoBannerImage.height, g_LogoBannerImage.width*4, g_LogoBannerImage.pixel_data);


    dred_window_get_client_size(pDialog->pWindow, &windowWidth, &windowHeight);


    dred_button_init(&pDialog->closeButton, pDred, pDialog->pWindow->pRootGUIControl, "Close");
    dred_button_set_on_pressed(&pDialog->closeButton, dred_about_dialog__btn_close__on_pressed);
    dred_button_set_padding(&pDialog->closeButton, 32*pDred->uiScale, 6*pDred->uiScale);
    dred_control_set_relative_position(DRED_CONTROL(&pDialog->closeButton),
        windowWidth - dred_control_get_width(DRED_CONTROL(&pDialog->closeButton)) - 8*pDred->uiScale,
        windowHeight - dred_control_get_height(DRED_CONTROL(&pDialog->closeButton)) - 8*pDred->uiScale);

    return pDialog;
}

void dred_about_dialog_delete(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_button_uninit(&pDialog->closeButton);

    dred_gui_delete_image(pDialog->pLogo);
    pDialog->pLogo = NULL;

    dred_window_delete(pDialog->pWindow);
    free(pDialog);
}


void dred_about_dialog_show(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_window_move_to_center(pDialog->pWindow);
    dred_window_show(pDialog->pWindow);
}

void dred_about_dialog_hide(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_window_hide(pDialog->pWindow);
}

dr_bool32 dred_about_dialog_is_showing(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) {
        return DR_FALSE;
    }

    return dtk_control_is_visible(DTK_CONTROL(&pDialog->pWindow->windowDTK));
}

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
  "\0\377\0\0\0\377\0\0\0\0\0\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377}\0\0"
  "\377\0\0\0\0\0\0\0\0\0\0\0\0}\0\0\377\0\0\0\0}\0\0\377\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0}\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0}"
  "\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0}\0\0\377}\0\0\377}\0\0\377}\0\0\377}\0\0"
  "\377}\0\0\377\0\0\0\0}\0\0\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0}\0"
  "\0\377}\0\0\377}\0\0\377}\0\0\377}\0\0\377\0\0\0\0}\0\0\377}\0\0\377}\0\0"
  "\377}\0\0\377}\0\0\377",
};

void dred_about_dialog__on_size(drgui_element* pElement, float newWidth, float newHeight)
{
    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow == NULL) {
        return;
    }

    dred_about_dialog* pDialog = (dred_about_dialog*)pWindow->pUserData;
    assert(pDialog != NULL);

    dred_context* pDred = pWindow->pDred;
    assert(pDialog != NULL);

    // The close button needs to be repositioned.
    dred_control_set_relative_position(pDialog->pCloseButton, newWidth - dred_control_get_width(pDialog->pCloseButton) - 8*pDred->uiScale, newHeight - dred_control_get_height(pDialog->pCloseButton) - 8*pDred->uiScale);
}

void dred_about_dialog__on_paint(drgui_element* pElement, drgui_rect rect, void* pPaintData)
{
    (void)rect;

    dred_window* pWindow = dred_get_element_window(pElement);
    if (pWindow == NULL) {
        return;
    }

    dred_about_dialog* pDialog = (dred_about_dialog*)pWindow->pUserData;
    assert(pDialog != NULL);

    float uiScale = (float)pWindow->pDred->uiScale;

    drgui_rect dialogRect = drgui_get_local_rect(pElement);

    drgui_draw_rect(pElement, dialogRect, drgui_rgb(255, 255, 255), pPaintData);

    unsigned int logoWidth;
    unsigned int logoHeight;
    drgui_get_image_size(pDialog->pLogo, &logoWidth, &logoHeight);

    drgui_rect bannerRect = dialogRect;
    bannerRect.bottom = (logoHeight*10 + 80.0f) * uiScale;

    drgui_draw_image_args args;
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
    args.options = DRGUI_IMAGE_ALIGN_CENTER;
    args.foregroundTint = drgui_rgb(255, 255, 255);
    args.backgroundColor = drgui_rgb(255, 255, 255);
    drgui_draw_image(pElement, pDialog->pLogo, &args, pPaintData);

    

    drgui_font* pFont = dred_font_acquire_subfont(pWindow->pDred->config.pUIFont, uiScale);
    drgui_font_metrics fontMetrics;
    drgui_get_font_metrics(pFont, &fontMetrics);

    float penPosX = 0;
    float penPosY = bannerRect.bottom - (fontMetrics.lineHeight*2.0f);

    const char* linkStr = "https://www.drsoftware.com.au";

    float linkWidth;
    drgui_measure_string(pFont, linkStr, strlen(linkStr), &linkWidth, NULL);
    drgui_draw_text(pElement, pFont, linkStr, (int)strlen(linkStr), ((dialogRect.right - dialogRect.left) - linkWidth) / 2, penPosY, drgui_rgb(0, 0, 0), drgui_rgb(255, 255, 255), pPaintData);


    const char* versionStr = "dred version " DRED_VERSION_STRING;
    const char* copyrightStr = "Copyright \xC2\xA9 2016 David Reid";
    dred_font_release_subfont(pWindow->pDred->config.pUIFont, pFont);

    penPosX = (8*uiScale);
    penPosY = bannerRect.bottom;

    drgui_draw_rect(pElement, drgui_make_rect(0, bannerRect.bottom, dialogRect.right, penPosY + (1 * uiScale)), drgui_rgb(200, 200, 200), pPaintData);
    penPosY += (9*uiScale);

    drgui_draw_text(pElement, pFont, versionStr, (int)strlen(versionStr), penPosX, penPosY, drgui_rgb(0, 0, 0), drgui_rgb(255, 255, 255), pPaintData);
    penPosY += fontMetrics.lineHeight;

    drgui_draw_text(pElement, pFont, copyrightStr, (int)strlen(copyrightStr), penPosX, penPosY, drgui_rgb(0, 0, 0), drgui_rgb(255, 255, 255), pPaintData);
    penPosY += fontMetrics.lineHeight;
    
    penPosY += (9*uiScale);
    drgui_draw_rect(pElement, drgui_make_rect(0, penPosY, dialogRect.right, penPosY + (1 * uiScale)), drgui_rgb(200, 200, 200), pPaintData);
    penPosY += 9*uiScale;

    const char* creditsTitle = "The following libraries are used internally by dred:";
    drgui_draw_text(pElement, pFont, creditsTitle, (int)strlen(creditsTitle), penPosX, penPosY, drgui_rgb(0, 0, 0), drgui_rgb(255, 255, 255), pPaintData);
    penPosY += 4*uiScale + fontMetrics.lineHeight;

    static const char* credits[] = {
        "    dr_libs (https://github.com/mackron/dr_libs)",
        "    gb_string (https://github.com/gingerBill/gb)",
        "    nanosvg (https://github.com/memononen/nanosvg)"
    };

    size_t creditsCount = sizeof(credits) / sizeof(credits[0]);

    for (size_t iCredit = 0; iCredit < creditsCount; ++iCredit) {
        drgui_draw_text(pElement, pFont, credits[iCredit], strlen(credits[iCredit]), penPosX, penPosY, drgui_rgb(0, 0, 0), drgui_rgb(255, 255, 255), pPaintData);
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

/*void dred_about_dialog__on_key_down(dred_window* pWindow, drgui_key key, unsigned int stateFlags)
{
    (void)stateFlags;

    dred_about_dialog* pDialog = (dred_about_dialog*)pWindow->pUserData;
    assert(pDialog != NULL);

    if (key == DRGUI_ESCAPE) {
        dred_about_dialog__on_window_close(pWindow);
    }
}*/

void dred_about_dialog__btn_close__on_pressed(dred_button* pButton)
{
    dred_window* pWindow = dred_get_element_window(pButton);
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
    drgui_set_on_size(pDialog->pWindow->pRootGUIElement, dred_about_dialog__on_size);
    drgui_set_on_paint(pDialog->pWindow->pRootGUIElement, dred_about_dialog__on_paint);
    


    pDialog->pLogo = drgui_create_image(pDred->pGUI, g_LogoBannerImage.width, g_LogoBannerImage.height, drgui_image_format_rgba8, g_LogoBannerImage.width*4, g_LogoBannerImage.pixel_data);


    dred_window_get_client_size(pDialog->pWindow, &windowWidth, &windowHeight);


    pDialog->pCloseButton = dred_button_create(pDred, pDialog->pWindow->pRootGUIElement, "Close");
    dred_button_set_on_pressed(pDialog->pCloseButton, dred_about_dialog__btn_close__on_pressed);
    dred_button_set_padding(pDialog->pCloseButton, 32*pDred->uiScale, 6*pDred->uiScale);
    dred_control_set_relative_position(pDialog->pCloseButton, windowWidth - dred_control_get_width(pDialog->pCloseButton) - 8*pDred->uiScale, windowHeight - dred_control_get_height(pDialog->pCloseButton) - 8*pDred->uiScale);

    return pDialog;
}

void dred_about_dialog_delete(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_button_delete(pDialog->pCloseButton);
    pDialog->pCloseButton = NULL;

    drgui_delete_image(pDialog->pLogo);
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
    pDialog->isShowing = true;
}

void dred_about_dialog_hide(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dred_window_hide(pDialog->pWindow, 0);
    pDialog->isShowing = false;
}

bool dred_about_dialog_is_showing(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) {
        return false;
    }

    return pDialog->isShowing;
}
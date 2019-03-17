// Copyright (C) 2019 David Reid. See included LICENSE file.

void dred_about_dialog__btn_close__on_pressed(dtk_button* pButton)
{
    dtk_window* pWindow = dtk_control_get_window(DTK_CONTROL(pButton));
    if (pWindow == NULL) {
        return; // Should never hit this, but leaving here for sanity.
    }

    dtk_window_close(pWindow);
}

static dtk_bool32 dred_about_dialog_event_handler(dtk_event* pEvent)
{
    dred_about_dialog* pDialog = (dred_about_dialog*)pEvent->pControl;
    dred_context* pDred = dred_get_context_from_control(pEvent->pControl);

    switch (pEvent->type)
    {
        case DTK_EVENT_DPI_CHANGED:
        {
            dtk_control_refresh_layout(pEvent->pControl);
        } break;

        case DTK_EVENT_CLOSE:
        {
            dtk_assert(pDred->pAboutDialog == pDialog);
            dred_about_dialog_uninit(pDred->pAboutDialog);
            pDred->pAboutDialog = NULL;
        } return DTK_FALSE;

        case DTK_EVENT_SIZE:
        {
            float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pDialog));

            // The close button needs to be repositioned.
            dred_control_set_relative_position(DRED_CONTROL(&pDialog->closeButton),
                pEvent->size.width  - dred_control_get_width (DRED_CONTROL(&pDialog->closeButton)) - 8*uiScale,
                pEvent->size.height - dred_control_get_height(DRED_CONTROL(&pDialog->closeButton)) - 8*uiScale);
        } break;

        case DTK_EVENT_PAINT:
        {
            float uiScale = dtk_control_get_scaling_factor(pEvent->pControl);

            dtk_rect dialogRect = dtk_control_get_local_rect(pEvent->pControl);

            dtk_surface_draw_rect(pEvent->paint.pSurface, dialogRect, dred_rgb(255, 255, 255));

            dtk_image* pLogo = dred_image_library_get_image_by_id(&pDred->imageLibrary, DRED_STOCK_IMAGE_ID_LOGO);

            unsigned int logoWidth;
            unsigned int logoHeight;
            dtk_image_get_size(pLogo, &logoWidth, &logoHeight);

            dtk_rect bannerRect = dialogRect;
            bannerRect.bottom = (dtk_int32)((logoHeight*10 + 80.0f) * uiScale);

            dtk_rect logoRect = dtk_rect_move_to_center(dtk_rect_init(0, 0, (dtk_int32)(logoWidth*10 * uiScale), (dtk_int32)(logoHeight*10 * uiScale)), bannerRect);

            dtk_draw_image_args args;
            memset(&args, 0, sizeof(args));
            args.srcWidth = logoWidth;
            args.srcHeight = logoHeight;
            args.dstWidth = (dtk_int32)(args.srcWidth*10 * uiScale);
            args.dstHeight = (dtk_int32)(args.srcHeight*10 * uiScale);
            args.dstX = logoRect.left;
            args.dstY = logoRect.top;
            args.foregroundColor = dred_rgb(0, 0, 0);
            args.backgroundColor = dred_rgb(255, 255, 255);
            dtk_surface_draw_image(pEvent->paint.pSurface, pLogo, &args);
            

            dtk_surface_draw_outer_rect(pEvent->paint.pSurface, bannerRect, logoRect, dtk_rgb(255, 255, 255));

            dtk_font* pFont = &pDred->config.pUIFont->fontDTK;
            dtk_font_metrics fontMetrics;
            dtk_font_get_metrics(pFont, uiScale, &fontMetrics);

            float penPosX = 0;
            float penPosY = bannerRect.bottom - (fontMetrics.lineHeight*1.5f);

            const char* linkStr = "http://dred.io";
            
            dtk_int32 linkWidth;
            dtk_font_measure_string(pFont, uiScale, linkStr, strlen(linkStr), &linkWidth, NULL);
            dtk_surface_draw_text(pEvent->paint.pSurface, pFont, uiScale, linkStr, (int)strlen(linkStr), (dtk_int32)((dialogRect.right - dialogRect.left) - linkWidth) / 2, (dtk_int32)penPosY, dred_rgb(0, 0, 0), dred_rgb(255, 255, 255));


            const char* versionStr = "dred version " DRED_VERSION_STRING;
            const char* copyrightStr = "Copyright \xC2\xA9 2019 David Reid";

            penPosX = (8*uiScale);
            penPosY = (float)bannerRect.bottom;

            dtk_surface_draw_rect(pEvent->paint.pSurface, dtk_rect_init(0, bannerRect.bottom, dialogRect.right, (dtk_int32)(penPosY + (1 * uiScale))), dred_rgb(200, 200, 200));
            penPosY += (9*uiScale);

            dtk_surface_draw_text(pEvent->paint.pSurface, pFont, uiScale, versionStr, (int)strlen(versionStr), (dtk_int32)penPosX, (dtk_int32)penPosY, dred_rgb(0, 0, 0), dred_rgb(255, 255, 255));
            penPosY += fontMetrics.lineHeight;

            dtk_surface_draw_text(pEvent->paint.pSurface, pFont, uiScale, copyrightStr, (int)strlen(copyrightStr), (dtk_int32)penPosX, (dtk_int32)penPosY, dred_rgb(0, 0, 0), dred_rgb(255, 255, 255));
            penPosY += fontMetrics.lineHeight;
    
            penPosY += (9*uiScale);
            dtk_surface_draw_rect(pEvent->paint.pSurface, dtk_rect_init(0, (dtk_int32)penPosY, dialogRect.right, (dtk_int32)(penPosY + (1 * uiScale))), dred_rgb(200, 200, 200));
            penPosY += 9*uiScale;
        } break;

        case DTK_EVENT_REFRESH_LAYOUT:
        {
            dred_about_dialog_refresh_layout(pDialog);
        } break;

        default: break;
    }

    return dtk_window_default_event_handler(pEvent);
}


dtk_result dred_about_dialog_init(dred_context* pDred, dred_about_dialog* pDialog)
{
    if (pDred == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pDialog);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(&pDred->mainWindow));

    dtk_int32 windowWidth  = (dtk_int32)(480*uiScale);
    dtk_int32 windowHeight = (dtk_int32)(360*uiScale);

    dtk_result result = dtk_window_init(&pDred->tk, dred_about_dialog_event_handler, DTK_CONTROL(&pDred->mainWindow), dtk_window_type_dialog, "About", windowWidth, windowHeight, &pDialog->window);
    if (result != DTK_SUCCESS) {
        return result;
    }

    dtk_window_get_client_size(DTK_WINDOW(pDialog), &windowWidth, &windowHeight);
    dtk_button_init(&pDred->tk, NULL, DTK_CONTROL(pDialog), "Close", &pDialog->closeButton);
    dtk_button_set_on_pressed(&pDialog->closeButton, dred_about_dialog__btn_close__on_pressed);
    dred_about_dialog_refresh_layout(pDialog);

    return DTK_SUCCESS;
}

dtk_result dred_about_dialog_uninit(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) return DTK_INVALID_ARGS;

    dtk_button_uninit(&pDialog->closeButton);

    dtk_window_uninit(DTK_WINDOW(pDialog));
    return DTK_SUCCESS;
}


void dred_about_dialog_show(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dtk_window_move_to_center(DTK_WINDOW(pDialog));
    dtk_window_show(DTK_WINDOW(pDialog), DTK_SHOW_NORMAL);
}

void dred_about_dialog_hide(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) {
        return;
    }

    dtk_window_hide(DTK_WINDOW(pDialog));
}

dtk_bool32 dred_about_dialog_is_showing(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) {
        return DTK_FALSE;
    }

    return dtk_control_is_visible(DTK_CONTROL(pDialog));
}


void dred_about_dialog_refresh_inner_layout(dred_about_dialog* pDialog)
{
    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pDialog));

    dtk_int32 windowWidth;
    dtk_int32 windowHeight;
    dtk_window_get_client_size(DTK_WINDOW(pDialog), &windowWidth, &windowHeight);

    dtk_button_set_padding(&pDialog->closeButton, 32, 6);
    dtk_control_set_relative_position(DTK_CONTROL(&pDialog->closeButton),
        (dtk_int32)((dtk_int32)windowWidth  - (dtk_int32)dtk_control_get_width (DTK_CONTROL(&pDialog->closeButton)) - 8*uiScale),
        (dtk_int32)((dtk_int32)windowHeight - (dtk_int32)dtk_control_get_height(DTK_CONTROL(&pDialog->closeButton)) - 8*uiScale));

    dtk_control_scheduled_redraw(DTK_CONTROL(pDialog), dtk_control_get_local_rect(DTK_CONTROL(pDialog)));
}

void dred_about_dialog_refresh_layout(dred_about_dialog* pDialog)
{
    if (pDialog == NULL) return;

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pDialog));

    dtk_uint32 windowWidth  = (dtk_uint32)(480*uiScale);
    dtk_uint32 windowHeight = (dtk_uint32)(360*uiScale);
    dtk_window_set_size(DTK_WINDOW(pDialog), windowWidth, windowHeight);
    dtk_window_move_to_center(DTK_WINDOW(pDialog));

    dred_about_dialog_refresh_inner_layout(pDialog);
}
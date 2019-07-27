// Copyright (C) 2019 David Reid. See included LICENSE file.

#define DRED_INFO_BAR_TYPE_NONE             0
#define DRED_INFO_BAR_TYPE_TEXT_EDITOR      1

dtk_color dred_info_bar__get_bg_color(dred_info_bar* pInfoBar)
{
    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pInfoBar));
    assert(pDred != NULL);

    if (dred_cmdbar_has_keyboard_focus(&pDred->cmdBar)) {
        return pDred->config.cmdbarBGColorActive;
    } else {
        return pDred->config.cmdbarBGColor;
    }
}

dtk_color dred_info_bar__get_text_color(dred_info_bar* pInfoBar)
{
    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pInfoBar));
    assert(pDred != NULL);

    if (dred_cmdbar_has_keyboard_focus(&pDred->cmdBar)) {
        return pDred->config.cmdbarTextColorActive;
    } else {
        return pDred->config.cmdbarTextColor;
    }
}

void dred_info_bar__on_paint__none(dred_info_bar* pInfoBar, dtk_surface* pSurface)
{
    dtk_surface_draw_rect(pSurface, dtk_control_get_local_rect(DTK_CONTROL(pInfoBar)), dred_info_bar__get_bg_color(pInfoBar));
}

void dred_info_bar__on_paint__text_editor(dred_info_bar* pInfoBar, dtk_surface* pSurface)
{
    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pInfoBar));
    assert(pDred != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pInfoBar));

    dtk_surface_draw_rect(pSurface, dtk_control_get_local_rect(DTK_CONTROL(pInfoBar)), dred_info_bar__get_bg_color(pInfoBar));


    dtk_int32 padding = (dtk_int32)(32*uiScale);
    dtk_int32 paddingRight = (dtk_int32)(pDred->config.cmdbarPaddingX*uiScale);

    dtk_font* pFont = &pDred->config.pUIFont->fontDTK;
    if (pFont != NULL) {
        dtk_font_metrics fontMetrics;
        dtk_font_get_metrics(pFont, uiScale, &fontMetrics);

        // The text info will be right-aligned so we need to measure first.
        dtk_int32 lineStrWidth;
        dtk_font_measure_string(pFont, uiScale, pInfoBar->lineStr, strlen(pInfoBar->lineStr), &lineStrWidth, NULL);

        dtk_int32 colStrWidth;
        dtk_font_measure_string(pFont, uiScale, pInfoBar->colStr, strlen(pInfoBar->colStr), &colStrWidth, NULL);

        dtk_int32 totalWidth = lineStrWidth + padding + colStrWidth + paddingRight;

        
        dtk_int32 textPosX =  dtk_control_get_width( DTK_CONTROL(pInfoBar)) - totalWidth;
        dtk_int32 textPosY = (dtk_control_get_height(DTK_CONTROL(pInfoBar)) - fontMetrics.lineHeight) / 2;
        dtk_surface_draw_text(pSurface, pFont, uiScale, pInfoBar->lineStr, (int)strlen(pInfoBar->lineStr), (dtk_int32)textPosX, (dtk_int32)textPosY, dred_info_bar__get_text_color(pInfoBar), dred_info_bar__get_bg_color(pInfoBar));

        textPosX += lineStrWidth + padding;
        dtk_surface_draw_text(pSurface, pFont, uiScale, pInfoBar->colStr, (int)strlen(pInfoBar->colStr), (dtk_int32)textPosX, (dtk_int32)textPosY, dred_info_bar__get_text_color(pInfoBar), dred_info_bar__get_bg_color(pInfoBar));
    }
}

dtk_bool32 dred_info_bar_event_handler(dtk_event* pEvent)
{
    dred_info_bar* pInfoBar = DRED_INFO_BAR(pEvent->pControl);
    assert(pInfoBar != NULL);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            if (pInfoBar->type == DRED_INFO_BAR_TYPE_NONE) {
                dred_info_bar__on_paint__none(pInfoBar, pEvent->paint.pSurface);
            } else if (pInfoBar->type == DRED_INFO_BAR_TYPE_TEXT_EDITOR) {
                dred_info_bar__on_paint__text_editor(pInfoBar, pEvent->paint.pSurface);
            }
        } break;

        default: break;
    }

    return dtk_control_default_event_handler(pEvent);
}

dtk_bool32 dred_info_bar_init(dred_context* pDred, dtk_control* pParent, dred_info_bar* pInfoBar)
{
    if (pInfoBar == NULL) {
        return DTK_FALSE;
    }

    memset(pInfoBar, 0, sizeof(*pInfoBar));
    if (!dtk_control_init(&pDred->tk, DRED_CONTROL_TYPE_INFO_BAR, dred_info_bar_event_handler, DTK_CONTROL(pParent), DTK_CONTROL(pInfoBar))) {
        return DTK_FALSE;
    }


    pInfoBar->pFont = &pDred->config.pUIFont->fontDTK;
    if (pInfoBar->pFont == NULL) {
        dred_control_uninit(DRED_CONTROL(pInfoBar));
        return DTK_FALSE;
    }

    pInfoBar->type = DRED_INFO_BAR_TYPE_NONE;
    pInfoBar->lineStr[0] = '\0';
    pInfoBar->colStr[0] = '\0';
    pInfoBar->zoomStr[0] = '\0';

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pInfoBar));

    // The height of the command bar is based on the size of the font.
    dtk_font_metrics fontMetrics;
    dtk_font_get_metrics(pInfoBar->pFont, uiScale, &fontMetrics);
    dtk_control_set_size(DTK_CONTROL(pInfoBar), 0, fontMetrics.lineHeight);

    return DTK_TRUE;
}

void dred_info_bar_uninit(dred_info_bar* pInfoBar)
{
    dred_control_uninit(DRED_CONTROL(pInfoBar));
}

void dred_info_bar_update(dred_info_bar* pInfoBar, dred_control* pControl)
{
    if (pInfoBar == NULL) {
        return;
    }

    pInfoBar->type = DRED_INFO_BAR_TYPE_NONE;

    if (pControl != NULL) {
        if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR) || dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXTBOX)) {
            pInfoBar->type = DRED_INFO_BAR_TYPE_TEXT_EDITOR;
            snprintf(pInfoBar->lineStr, sizeof(pInfoBar->lineStr), "Ln %d", (int)dred_text_editor_get_cursor_line(DRED_TEXT_EDITOR(pControl)) + 1);
            snprintf(pInfoBar->colStr,  sizeof(pInfoBar->colStr),  "Col %d", (int)dred_text_editor_get_cursor_column(DRED_TEXT_EDITOR(pControl)) + 1);
        }
    }


    // The bar needs to be redrawn.
    dtk_control_scheduled_redraw(DTK_CONTROL(pInfoBar), dtk_control_get_local_rect(DTK_CONTROL(pInfoBar)));
}

void dred_info_bar_refresh_styling(dred_info_bar* pInfoBar)
{
    if (pInfoBar == NULL) {
        return;
    }

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pInfoBar));
    assert(pDred != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pInfoBar));

    dtk_font* pNewFont = &pDred->config.pUIFont->fontDTK;
    if (pNewFont != NULL) {
        pInfoBar->pFont = pNewFont;

        // The height of the command bar is based on the size of the font.
        dtk_font_metrics fontMetrics;
        dtk_font_get_metrics(pInfoBar->pFont, uiScale, &fontMetrics);
        dtk_control_set_size(DTK_CONTROL(pInfoBar), dtk_control_get_width(DTK_CONTROL(pInfoBar)), fontMetrics.lineHeight);
    }

    dtk_control_scheduled_redraw(DTK_CONTROL(pInfoBar), dtk_control_get_local_rect(DTK_CONTROL(pInfoBar)));
}
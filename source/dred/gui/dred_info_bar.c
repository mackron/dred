// Copyright (C) 2019 David Reid. See included LICENSE file.

#define DRED_INFO_BAR_TYPE_NONE             0
#define DRED_INFO_BAR_TYPE_TEXT_EDITOR      1
#define DRED_INFO_BAR_TYPE_HEX_EDITOR       2
#define DRED_INFO_BAR_TYPE_IMAGE_EDITOR     3
#define DRED_INFO_BAR_TYPE_ICON_EDITOR      4
#define DRED_INFO_BAR_TYPE_PARTICLE_EDITOR  5

dtk_color dred_info_bar__get_bg_color(dred_info_bar* pInfoBar)
{
    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pInfoBar));
    assert(pDred != NULL);

    if (dred_cmdbar_has_keyboard_focus(&pDred->cmdBar)) {
        return pDred->config.cmdbarBGColorActive;
    } else {
        return pDred->config.cmdbarBGColor;
    }
}

dtk_color dred_info_bar__get_text_color(dred_info_bar* pInfoBar)
{
    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pInfoBar));
    assert(pDred != NULL);

    if (dred_cmdbar_has_keyboard_focus(&pDred->cmdBar)) {
        return pDred->config.cmdbarTextColorActive;
    } else {
        return pDred->config.cmdbarTextColor;
    }
}

void dred_info_bar__on_paint__none(dred_info_bar* pInfoBar, dtk_surface* pSurface)
{
    dred_control_draw_rect(DRED_CONTROL(pInfoBar), dred_control_get_local_rect(DRED_CONTROL(pInfoBar)), dred_info_bar__get_bg_color(pInfoBar), pSurface);
}

void dred_info_bar__on_paint__text_editor(dred_info_bar* pInfoBar, dtk_surface* pSurface)
{
    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pInfoBar));
    assert(pDred != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pInfoBar));

    dred_control_draw_rect(DRED_CONTROL(pInfoBar), dred_control_get_local_rect(DRED_CONTROL(pInfoBar)), dred_info_bar__get_bg_color(pInfoBar), pSurface);


    float padding = 32*uiScale;
    float paddingRight = pDred->config.cmdbarPaddingX*uiScale;

    dtk_font* pFont = &pDred->config.pUIFont->fontDTK;
    if (pFont != NULL) {
        dtk_font_metrics fontMetrics;
        dtk_font_get_metrics(pFont, uiScale, &fontMetrics);

        // The text info will be right-aligned so we need to measure first.
        dtk_int32 lineStrWidth;
        dtk_font_measure_string(pFont, uiScale, pInfoBar->lineStr, strlen(pInfoBar->lineStr), &lineStrWidth, NULL);

        dtk_int32 colStrWidth;
        dtk_font_measure_string(pFont, uiScale, pInfoBar->colStr, strlen(pInfoBar->colStr), &colStrWidth, NULL);

        float totalWidth = lineStrWidth + padding + colStrWidth + paddingRight;

        
        float textPosX = dred_control_get_width(DRED_CONTROL(pInfoBar)) - totalWidth;
        float textPosY = (dred_control_get_height(DRED_CONTROL(pInfoBar)) - fontMetrics.lineHeight) / 2;
        dtk_surface_draw_text(pSurface, pFont, uiScale, pInfoBar->lineStr, (int)strlen(pInfoBar->lineStr), (dtk_int32)textPosX, (dtk_int32)textPosY, dred_info_bar__get_text_color(pInfoBar), dred_info_bar__get_bg_color(pInfoBar));

        textPosX += lineStrWidth + padding;
        dtk_surface_draw_text(pSurface, pFont, uiScale, pInfoBar->colStr, (int)strlen(pInfoBar->colStr), (dtk_int32)textPosX, (dtk_int32)textPosY, dred_info_bar__get_text_color(pInfoBar), dred_info_bar__get_bg_color(pInfoBar));
    }
}


void dred_info_bar__on_paint(dred_control* pControl, dred_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dred_info_bar* pInfoBar = DRED_INFO_BAR(pControl);
    assert(pInfoBar != NULL);

    if (pInfoBar->type == DRED_INFO_BAR_TYPE_NONE) {
        dred_info_bar__on_paint__none(pInfoBar, pSurface);
    } else if (pInfoBar->type == DRED_INFO_BAR_TYPE_TEXT_EDITOR) {
        dred_info_bar__on_paint__text_editor(pInfoBar, pSurface);
    }
}

dtk_bool32 dred_info_bar_init(dred_info_bar* pInfoBar, dred_context* pDred, dred_control* pParent)
{
    if (pInfoBar == NULL) {
        return DTK_FALSE;
    }

    memset(pInfoBar, 0, sizeof(*pInfoBar));
    if (!dred_control_init(DRED_CONTROL(pInfoBar), pDred, pParent, NULL, DRED_CONTROL_TYPE_INFO_BAR, NULL)) {
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
    dred_control_set_size(DRED_CONTROL(pInfoBar), 0, (float)fontMetrics.lineHeight);


    // Events.
    dred_control_set_on_paint(DRED_CONTROL(pInfoBar), dred_info_bar__on_paint);

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
        if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR) || dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXTBOX))
        {
            pInfoBar->type = DRED_INFO_BAR_TYPE_TEXT_EDITOR;
            snprintf(pInfoBar->lineStr, sizeof(pInfoBar->lineStr), "Ln %d", (int)dred_text_editor_get_cursor_line(DRED_TEXT_EDITOR(pControl)) + 1);
            snprintf(pInfoBar->colStr,  sizeof(pInfoBar->colStr),  "Col %d", (int)dred_text_editor_get_cursor_column(DRED_TEXT_EDITOR(pControl)) + 1);
        }
    }


    // The bar needs to be redrawn.
    dred_control_dirty(DRED_CONTROL(pInfoBar), dred_control_get_local_rect(DRED_CONTROL(pInfoBar)));
}

void dred_info_bar_refresh_styling(dred_info_bar* pInfoBar)
{
    if (pInfoBar == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pInfoBar));
    assert(pDred != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pInfoBar));

    dtk_font* pNewFont = &pDred->config.pUIFont->fontDTK;
    if (pNewFont != NULL) {
        pInfoBar->pFont = pNewFont;

        // The height of the command bar is based on the size of the font.
        dtk_font_metrics fontMetrics;
        dtk_font_get_metrics(pInfoBar->pFont, uiScale, &fontMetrics);
        dred_control_set_size(DRED_CONTROL(pInfoBar), dred_control_get_width(DRED_CONTROL(pInfoBar)), (float)fontMetrics.lineHeight);
    }

    dred_control_dirty(DRED_CONTROL(pInfoBar), dred_control_get_local_rect(DRED_CONTROL(pInfoBar)));
}
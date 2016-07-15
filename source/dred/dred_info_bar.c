
#define DRED_INFO_BAR_TYPE_NONE             0
#define DRED_INFO_BAR_TYPE_TEXT_EDITOR      1
#define DRED_INFO_BAR_TYPE_HEX_EDITOR       2
#define DRED_INFO_BAR_TYPE_IMAGE_EDITOR     3
#define DRED_INFO_BAR_TYPE_ICON_EDITOR      4
#define DRED_INFO_BAR_TYPE_PARTICLE_EDITOR  5

typedef struct
{
    drgui_font* pFont;
    int type;
    char lineStr[32];
    char colStr[32];
    char zoomStr[32];
} dred_info_bar_data;

drgui_color dred_info_bar__get_bg_color(dred_info_bar* pInfoBar)
{
    dred_context* pDred = dred_control_get_context(pInfoBar);
    assert(pDred != NULL);

    if (dred_cmdbar_has_keyboard_focus(pDred->pCmdBar)) {
        return pDred->config.cmdbarBGColorActive;
    } else {
        return pDred->config.cmdbarBGColor;
    }
}

drgui_color dred_info_bar__get_text_color(dred_info_bar* pInfoBar)
{
    dred_context* pDred = dred_control_get_context(pInfoBar);
    assert(pDred != NULL);

    if (dred_cmdbar_has_keyboard_focus(pDred->pCmdBar)) {
        return pDred->config.cmdbarTextColorActive;
    } else {
        return pDred->config.cmdbarTextColor;
    }
}

void dred_info_bar__on_paint__none(dred_info_bar* pInfoBar, dred_info_bar_data* data, void* pPaintData)
{
    (void)data;

    drgui_draw_rect(pInfoBar, drgui_get_local_rect(pInfoBar), dred_info_bar__get_bg_color(pInfoBar), pPaintData);
}

void dred_info_bar__on_paint__text_editor(dred_info_bar* pInfoBar, dred_info_bar_data* data, void* pPaintData)
{
    (void)data;

    dred_context* pDred = dred_control_get_context(pInfoBar);
    assert(pDred != NULL);

    drgui_draw_rect(pInfoBar, drgui_get_local_rect(pInfoBar), dred_info_bar__get_bg_color(pInfoBar), pPaintData);


    float padding = 32*pDred->uiScale;
    float paddingRight = pDred->config.cmdbarPaddingX*pDred->uiScale;

    drgui_font* pFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);
    if (pFont != NULL)
    {
        drgui_font_metrics fontMetrics;
        drgui_get_font_metrics(pFont, &fontMetrics);

        // The text info will be right-aligned so we need to measure first.
        float lineStrWidth;
        drgui_measure_string(pFont, data->lineStr, strlen(data->lineStr), &lineStrWidth, NULL);

        float colStrWidth;
        drgui_measure_string(pFont, data->colStr, strlen(data->colStr), &colStrWidth, NULL);

        float totalWidth = lineStrWidth + padding + colStrWidth + paddingRight;

        
        float textPosX = dred_control_get_width(pInfoBar) - totalWidth;
        float textPosY = (dred_control_get_height(pInfoBar) - fontMetrics.lineHeight) / 2;
        drgui_draw_text(pInfoBar, pFont, data->lineStr, (int)strlen(data->lineStr), textPosX, textPosY, dred_info_bar__get_text_color(pInfoBar), dred_info_bar__get_bg_color(pInfoBar), pPaintData);

        textPosX += lineStrWidth + padding;
        drgui_draw_text(pInfoBar, pFont, data->colStr, (int)strlen(data->colStr), textPosX, textPosY, dred_info_bar__get_text_color(pInfoBar), dred_info_bar__get_bg_color(pInfoBar), pPaintData);

        dred_font_release_subfont(pDred->config.pUIFont, pFont);
    }
}

void dred_info_bar__on_paint__image_editor(dred_info_bar* pInfoBar, dred_info_bar_data* data, void* pPaintData)
{
    (void)data;

    dred_context* pDred = dred_control_get_context(pInfoBar);
    assert(pDred != NULL);

    drgui_draw_rect(pInfoBar, drgui_get_local_rect(pInfoBar), dred_info_bar__get_bg_color(pInfoBar), pPaintData);


    //float padding = 32*pDred->uiScale;
    float paddingRight = pDred->config.cmdbarPaddingX*pDred->uiScale;

    drgui_font* pFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);
    if (pFont != NULL)
    {
        drgui_font_metrics fontMetrics;
        drgui_get_font_metrics(pFont, &fontMetrics);

        // The text info will be right-aligned so we need to measure first.
        float zoomStrWidth;
        drgui_measure_string(pFont, data->zoomStr, (int)strlen(data->zoomStr), &zoomStrWidth, NULL);

        float totalWidth = zoomStrWidth + paddingRight;

        
        float textPosX = dred_control_get_width(pInfoBar) - totalWidth;
        float textPosY = (dred_control_get_height(pInfoBar) - fontMetrics.lineHeight) / 2;
        drgui_draw_text(pInfoBar, pFont, data->zoomStr, (int)strlen(data->zoomStr), textPosX, textPosY, dred_info_bar__get_text_color(pInfoBar), dred_info_bar__get_bg_color(pInfoBar), pPaintData);

        dred_font_release_subfont(pDred->config.pUIFont, pFont);
    }
}


void dred_info_bar__on_paint(dred_info_bar* pInfoBar, drgui_rect rect, void* pPaintData)
{
    (void)rect;

    dred_info_bar_data* data = (dred_info_bar_data*)dred_control_get_extra_data(pInfoBar);
    assert(data != NULL);

    if (data->type == DRED_INFO_BAR_TYPE_NONE) {
        dred_info_bar__on_paint__none(pInfoBar, data, pPaintData);
    } else if (data->type == DRED_INFO_BAR_TYPE_TEXT_EDITOR) {
        dred_info_bar__on_paint__text_editor(pInfoBar, data, pPaintData);
    } else if (data->type == DRED_INFO_BAR_TYPE_IMAGE_EDITOR) {
        dred_info_bar__on_paint__image_editor(pInfoBar, data, pPaintData);
    }
}

dred_info_bar* dred_info_bar_create(dred_context* pDred, dred_control* pParent)
{
    dred_info_bar* pInfoBar = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_INFO_BAR, sizeof(dred_info_bar_data));
    if (pInfoBar == NULL) {
        return NULL;
    }

    dred_info_bar_data* data = (dred_info_bar_data*)dred_control_get_extra_data(pInfoBar);
    assert(data != NULL);

    data->pFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);
    if (data->pFont == NULL) {
        dred_control_delete(pInfoBar);
        return NULL;
    }

    data->type = DRED_INFO_BAR_TYPE_NONE;
    data->lineStr[0] = '\0';
    data->colStr[0] = '\0';
    data->zoomStr[0] = '\0';


    // The height of the command bar is based on the size of the font.
    drgui_font_metrics fontMetrics;
    drgui_get_font_metrics(data->pFont, &fontMetrics);
    drgui_set_size(pInfoBar, 0, (float)fontMetrics.lineHeight);


    // Events.
    pInfoBar->onPaint = dred_info_bar__on_paint;

    return pInfoBar;
}

void dred_info_bar_delete(dred_info_bar* pInfoBar)
{
    dred_control_delete(pInfoBar);
}

void dred_info_bar_update(dred_info_bar* pInfoBar, dred_control* pControl)
{
    dred_info_bar_data* data = (dred_info_bar_data*)dred_control_get_extra_data(pInfoBar);
    if (data == NULL) {
        return;
    }

    data->type = DRED_INFO_BAR_TYPE_NONE;

    if (pControl != NULL) {
        if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR) || dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXTBOX))
        {
            data->type = DRED_INFO_BAR_TYPE_TEXT_EDITOR;
            snprintf(data->lineStr, sizeof(data->lineStr), "Ln %d", (int)dred_text_editor_get_cursor_line(pControl) + 1);
            snprintf(data->colStr,  sizeof(data->colStr),  "Col %d", (int)dred_text_editor_get_cursor_column(pControl) + 1);
        }
#ifndef DRED_NO_IMAGE_EDITOR
        else if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_IMAGE_EDITOR))
        {
            data->type = DRED_INFO_BAR_TYPE_IMAGE_EDITOR;
            snprintf(data->zoomStr, sizeof(data->zoomStr), "%d%%", (int)(dred_image_editor_get_image_scale(pControl) * 100));
        }
#endif
    }


    // The bar needs to be redrawn.
    drgui_dirty(pInfoBar, drgui_get_local_rect(pInfoBar));
}

void dred_info_bar_refresh_styling(dred_info_bar* pInfoBar)
{
    dred_info_bar_data* data = (dred_info_bar_data*)dred_control_get_extra_data(pInfoBar);
    if (data == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pInfoBar);
    assert(pDred != NULL);

    drgui_font* pNewFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);
    if (pNewFont != NULL) {
        data->pFont = pNewFont;

        // The height of the command bar is based on the size of the font.
        drgui_font_metrics fontMetrics;
        drgui_get_font_metrics(data->pFont, &fontMetrics);
        drgui_set_size(pInfoBar, drgui_get_width(pInfoBar), (float)fontMetrics.lineHeight);
    }

    drgui_dirty(pInfoBar, drgui_get_local_rect(pInfoBar));
}
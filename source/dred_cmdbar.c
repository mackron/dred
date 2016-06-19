
typedef struct
{
    dred_context* pDred;
    dred_textbox* pTextBox;
    char message[256];
    drgui_font* pMessageFont;

    dred_info_bar* pInfoBar;
} dred_cmdbar_data;

drgui_rect dred_cmdbar__get_inner_rect(dred_cmdbar* pCmdBar)
{
    dred_context* pDred = dred_control_get_context(pCmdBar);
    assert(pDred != NULL);

    float cmdbarWidth;
    float cmdbarHeight;
    dred_control_get_size(pCmdBar, &cmdbarWidth, &cmdbarHeight);

    float scaledPaddingX = pDred->config.cmdbarPaddingX*pDred->uiScale;
    float scaledPaddingY = pDred->config.cmdbarPaddingY*pDred->uiScale;
    return drgui_make_rect(scaledPaddingX, scaledPaddingY, cmdbarWidth - scaledPaddingX, cmdbarHeight - scaledPaddingY);
}

void dred_cmdbar__get_inner_size(dred_cmdbar* pCmdBar, float* pWidthOut, float* pHeightOut)
{
    drgui_rect innerRect = dred_cmdbar__get_inner_rect(pCmdBar);

    if (pWidthOut) *pWidthOut = innerRect.right - innerRect.left;
    if (pHeightOut) *pHeightOut = innerRect.bottom - innerRect.top;
}

void dred_cmdbar__get_segment_rects(dred_cmdbar* pCmdBar, drgui_rect* pLRect, drgui_rect* pMRect, drgui_rect* pRRect)
{
    drgui_rect innerRect = dred_cmdbar__get_inner_rect(pCmdBar);

    float segWidth = (innerRect.right - innerRect.left) / 3;
    drgui_rect lrect = drgui_make_rect(innerRect.left, innerRect.top, innerRect.left + segWidth, innerRect.bottom);
    drgui_rect mrect = drgui_make_rect(lrect.right, innerRect.top, lrect.right + segWidth, innerRect.bottom);
    drgui_rect rrect = drgui_make_rect(mrect.right, innerRect.top, innerRect.right, innerRect.bottom);

    if (pLRect) *pLRect = lrect;
    if (pMRect) *pMRect = mrect;
    if (pRRect) *pRRect = rrect;
}


void dred_cmdbar__on_size(dred_cmdbar* pCmdBar, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    dred_cmdbar_data* data = dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    // Controls need to be resized based on their rectangles.
    drgui_rect lrect;
    drgui_rect mrect;
    drgui_rect rrect;
    dred_cmdbar__get_segment_rects(pCmdBar, &lrect, &mrect, &rrect);

    dred_control_set_relative_position(data->pTextBox, lrect.left, lrect.top);
    dred_control_set_size(data->pTextBox, lrect.right - lrect.left, lrect.bottom - lrect.top);
    
    dred_control_set_relative_position(data->pInfoBar, rrect.left, rrect.top);
    dred_control_set_size(data->pInfoBar, rrect.right - rrect.left, rrect.bottom - rrect.top);
}

void dred_cmdbar__on_capture_keyboard(dred_cmdbar* pCmdBar, drgui_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;

    dred_cmdbar_data* data = dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    drgui_capture_keyboard(data->pTextBox);
}

void dred_cmdbar__on_paint(dred_cmdbar* pCmdBar, drgui_rect rect, void* pPaintData)
{
    (void)rect;

    dred_cmdbar_data* data = dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    dred_context* pDred = dred_control_get_context(pCmdBar);
    assert(pDred != NULL);

    drgui_rect localRect = drgui_get_local_rect(pCmdBar);

    float scaledPaddingX = pDred->config.cmdbarPaddingX*pDred->uiScale;
    float scaledPaddingY = pDred->config.cmdbarPaddingY*pDred->uiScale;
    drgui_draw_rect(pCmdBar, drgui_make_rect(0,                                0,                                 scaledPaddingX,                   localRect.bottom), pDred->config.cmdbarBGColor, pPaintData); // Left
    drgui_draw_rect(pCmdBar, drgui_make_rect(localRect.right - scaledPaddingX, 0,                                 localRect.right,                  localRect.bottom), pDred->config.cmdbarBGColor, pPaintData); // Right
    drgui_draw_rect(pCmdBar, drgui_make_rect(scaledPaddingX,                   0,                                 localRect.right - scaledPaddingX, scaledPaddingY),   pDred->config.cmdbarBGColor, pPaintData); // Top
    drgui_draw_rect(pCmdBar, drgui_make_rect(scaledPaddingX,                   localRect.bottom - scaledPaddingY, localRect.right - scaledPaddingX, localRect.bottom), pDred->config.cmdbarBGColor, pPaintData); // Bottom


    // Message.
    drgui_rect lrect;
    drgui_rect mrect;
    drgui_rect rrect;
    dred_cmdbar__get_segment_rects(pCmdBar, &lrect, &mrect, &rrect);
    drgui_draw_rect(pCmdBar, mrect, pDred->config.cmdbarBGColor, pPaintData);

    drgui_font_metrics messageFontMetrics;
    drgui_get_font_metrics(data->pMessageFont, &messageFontMetrics);

    float messageLeft = mrect.left + (4*pDred->uiScale);
    float messageTop  = (((mrect.bottom - mrect.top) - messageFontMetrics.lineHeight) / 2) + scaledPaddingY;
    drgui_draw_text(pCmdBar, data->pMessageFont, data->message, (int)strlen(data->message), messageLeft, messageTop, drgui_rgb(224, 224, 224), data->pDred->config.cmdbarBGColor, pPaintData);
}

void dred_cmdbar_tb__on_key_down(dred_textbox* pTextBox, drgui_key key, int stateFlags)
{
    (void)stateFlags;

    // The parent is the command bar.
    dred_cmdbar* pCmdBar = dred_control_get_parent(pTextBox);
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_control_get_context(pCmdBar);

    dred_cmdbar_clear_message(pCmdBar);

    if (key == DRGUI_ESCAPE) {
        dred_unfocus_command_bar(pDred);
    } else {
        dred_textbox_on_key_down(pTextBox, key, stateFlags);
    }
}

void dred_cmdbar_tb__on_printable_key_down(dred_textbox* pTextBox, uint32_t utf32, int stateFlags)
{
    (void)stateFlags;

    // The parent is the command bar.
    dred_cmdbar* pCmdBar = dred_control_get_parent(pTextBox);
    assert(pCmdBar != NULL);

    if (utf32 == '\r' || utf32 == '\n')
    {
        size_t cmdLen = dred_textbox_get_text(pTextBox, NULL, 0);
        char* cmd = malloc(cmdLen + 1);
        if (dred_textbox_get_text(pTextBox, cmd, cmdLen + 1) == cmdLen)
        {
            const char* value;
            dred_command command;
            if (dred_find_command(cmd, &command, &value)) {
                command.proc(dred_control_get_context(pCmdBar), value);
            }

            if ((command.flags & DRED_CMDBAR_NO_CLEAR) == 0) {
                dred_textbox_set_text(pTextBox, "");
            }

            if ((command.flags & DRED_CMDBAR_RELEASE_KEYBOARD) != 0) {
                dred_unfocus_command_bar(dred_control_get_context(pCmdBar));
            }
        }

        free(cmd);
    }
    else
    {
        dred_textbox_on_printable_key_down(pTextBox, utf32, stateFlags);
    }
}

dred_cmdbar* dred_cmdbar_create(dred_context* pDred, dred_control* pParent)
{
    dred_cmdbar* pCmdBar = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_CMDBAR, sizeof(dred_cmdbar_data));
    if (pCmdBar == NULL) {
        return NULL;
    }

    dred_cmdbar_data* data = (dred_cmdbar_data*)dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    data->pDred = pDred;

    data->pTextBox = dred_textbox_create(pDred, pCmdBar);
    if (data->pTextBox == NULL) {
        dred_control_delete(data->pTextBox);
        return NULL;
    }

    dred_textbox_set_font(data->pTextBox, dred_font_acquire_subfont(pDred->config.pCmdbarTBFont, (float)pDred->uiScale));
    dred_textbox_set_text_color(data->pTextBox, pDred->config.cmdbarTBTextColor);
    dred_textbox_set_cursor_color(data->pTextBox, pDred->config.cmdbarTBTextColor);
    dred_textbox_set_background_color(data->pTextBox, pDred->config.cmdbarBGColor);
    dred_textbox_set_active_line_background_color(data->pTextBox, pDred->config.cmdbarBGColor);
    dred_textbox_disable_horizontal_scrollbar(data->pTextBox);
    dred_textbox_disable_vertical_scrollbar(data->pTextBox);

    // Events.
    dred_control_set_on_size(pCmdBar, dred_cmdbar__on_size);
    dred_control_set_on_capture_keyboard(pCmdBar, dred_cmdbar__on_capture_keyboard);
    dred_control_set_on_paint(pCmdBar, dred_cmdbar__on_paint);

    // Text box event overrides.
    dred_control_set_on_key_down(data->pTextBox, dred_cmdbar_tb__on_key_down);
    dred_control_set_on_printable_key_down(data->pTextBox, dred_cmdbar_tb__on_printable_key_down);


    data->pMessageFont = dred_font_acquire_subfont(pDred->config.pCmdbarMessageFont, (float)pDred->uiScale);
    strcpy_s(data->message, sizeof(data->message), "");


    // The info bar.
    data->pInfoBar = dred_info_bar_create(pDred, pCmdBar);



    // Set the initial size.
    drgui_font_metrics fontMetricsTB;
    drgui_get_font_metrics(dred_textbox_get_font(data->pTextBox), &fontMetricsTB);

    drgui_font_metrics fontMetricsMsg;
    drgui_get_font_metrics(data->pMessageFont, &fontMetricsMsg);

    float textboxHeight = (float)fontMetricsTB.lineHeight + dred_textbox_get_padding_vert(data->pTextBox)*2;
    float messageHeight = (float)fontMetricsMsg.lineHeight;
    float infobarHeight = (float)drgui_get_height(data->pInfoBar);

    float cmdbarHeight = dr_max(textboxHeight, dr_max(messageHeight, infobarHeight)) + (pDred->config.cmdbarPaddingY*pDred->uiScale*2);
    float cmdbarWidth = 0;
    if (pParent != NULL) {
        cmdbarWidth = dred_control_get_width(pParent);
    }
    dred_control_set_size(pCmdBar, cmdbarWidth, cmdbarHeight);

    return pCmdBar;
}

void dred_cmdbar_delete(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) {
        return;
    }

    dred_cmdbar_data* data = (dred_cmdbar_data*)dred_control_get_extra_data(pCmdBar);
    if (data != NULL) {
        dred_textbox_delete(data->pTextBox);
        dred_font_release_subfont(data->pDred->config.pCmdbarMessageFont, data->pMessageFont);
    }

    dred_control_delete(pCmdBar);
}


void dred_cmdbar_set_text(dred_cmdbar* pCmdBar, const char* text)
{
    dred_cmdbar_data* data = (dred_cmdbar_data*)dred_control_get_extra_data(pCmdBar);
    if (data == NULL) {
        return;
    }

    if (text == NULL) {
        text = "";
    }

    dred_textbox_set_text(data->pTextBox, text);
    dred_textbox_move_cursor_to_end_of_text(data->pTextBox);
}


bool dred_cmdbar_has_keyboard_focus(dred_cmdbar* pCmdBar)
{
    dred_cmdbar_data* data = (dred_cmdbar_data*)dred_control_get_extra_data(pCmdBar);
    if (data == NULL) {
        return false;
    }

    return dred_textbox_has_keyboard_capture(data->pTextBox);
}


void dred_cmdbar_set_message(dred_cmdbar* pCmdBar, const char* text)
{
    dred_cmdbar_data* data = (dred_cmdbar_data*)dred_control_get_extra_data(pCmdBar);
    if (data == NULL) {
        return;
    }

    if (text == NULL) {
        text = "";
    }

    strncpy_s(data->message, sizeof(data->message), text, _TRUNCATE);
    drgui_dirty(pCmdBar, drgui_get_local_rect(pCmdBar));    // <-- Can optimize this to only draw the message region.
}

void dred_cmdbar_clear_message(dred_cmdbar* pCmdBar)
{
    dred_cmdbar_data* data = (dred_cmdbar_data*)dred_control_get_extra_data(pCmdBar);
    if (data == NULL) {
        return;
    }

    if (data->message[0] != '\0') {
        dred_cmdbar_set_message(pCmdBar, "");
    }
}

void dred_cmdbar_update_info_bar(dred_cmdbar* pCmdBar, dred_control* pControl)
{
    dred_cmdbar_data* data = (dred_cmdbar_data*)dred_control_get_extra_data(pCmdBar);
    if (data == NULL) {
        return;
    }

    dred_info_bar_update(data->pInfoBar, pControl);
}
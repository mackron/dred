// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef struct
{
    dred_context* pDred;
    dred_textbox* pTextBox;
    char message[256];
    dred_info_bar* pInfoBar;
    unsigned int iPrevCommand;
    char* workingCommand;
} dred_cmdbar_data;

dred_textbox* dred_cmdbar__get_textbox(dred_cmdbar* pCmdBar)
{
    dred_cmdbar_data* data = dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    return data->pTextBox;
}

dred_rect dred_cmdbar__get_inner_rect(dred_cmdbar* pCmdBar)
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
    dred_rect innerRect = dred_cmdbar__get_inner_rect(pCmdBar);

    if (pWidthOut) *pWidthOut = innerRect.right - innerRect.left;
    if (pHeightOut) *pHeightOut = innerRect.bottom - innerRect.top;
}

void dred_cmdbar__get_segment_rects(dred_cmdbar* pCmdBar, dred_rect* pLRect, dred_rect* pMRect, dred_rect* pRRect)
{
    dred_rect innerRect = dred_cmdbar__get_inner_rect(pCmdBar);

    float segWidth = (innerRect.right - innerRect.left) / 3;
    dred_rect lrect = drgui_make_rect(innerRect.left, innerRect.top, innerRect.left + segWidth, innerRect.bottom);
    dred_rect mrect = drgui_make_rect(lrect.right, innerRect.top, lrect.right + segWidth, innerRect.bottom);
    dred_rect rrect = drgui_make_rect(mrect.right, innerRect.top, innerRect.right, innerRect.bottom);

    if (pLRect) *pLRect = lrect;
    if (pMRect) *pMRect = mrect;
    if (pRRect) *pRRect = rrect;
}


void dred_cmdbar__update_layouts_of_inner_controls(dred_cmdbar* pCmdBar)
{
    dred_cmdbar_data* data = dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    // Controls need to be resized based on their rectangles.
    dred_rect lrect;
    dred_rect mrect;
    dred_rect rrect;
    dred_cmdbar__get_segment_rects(pCmdBar, &lrect, &mrect, &rrect);

    dred_control_set_relative_position(data->pTextBox, lrect.left, lrect.top);
    dred_control_set_size(data->pTextBox, lrect.right - lrect.left, lrect.bottom - lrect.top);
    
    dred_control_set_relative_position(data->pInfoBar, rrect.left, rrect.top);
    dred_control_set_size(data->pInfoBar, rrect.right - rrect.left, rrect.bottom - rrect.top);
}


void dred_cmdbar__on_size(dred_cmdbar* pCmdBar, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    dred_cmdbar__update_layouts_of_inner_controls(pCmdBar);
}

void dred_cmdbar__on_capture_keyboard(dred_cmdbar* pCmdBar, dred_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;

    dred_cmdbar_data* data = dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    drgui_capture_keyboard(data->pTextBox);
}

void dred_cmdbar__on_paint(dred_cmdbar* pCmdBar, dred_rect rect, void* pPaintData)
{
    (void)rect;

    dred_cmdbar_data* data = dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    dred_context* pDred = dred_control_get_context(pCmdBar);
    assert(pDred != NULL);

    dred_rect localRect = drgui_get_local_rect(pCmdBar);

    dred_color bgcolor = pDred->config.cmdbarBGColor;
    if (dred_cmdbar_has_keyboard_focus(pCmdBar)) {
        bgcolor = pDred->config.cmdbarBGColorActive;
    }

    float scaledPaddingX = pDred->config.cmdbarPaddingX*pDred->uiScale;
    float scaledPaddingY = pDred->config.cmdbarPaddingY*pDred->uiScale;
    drgui_draw_rect(pCmdBar, drgui_make_rect(0,                                0,                                 scaledPaddingX,                   localRect.bottom), bgcolor, pPaintData); // Left
    drgui_draw_rect(pCmdBar, drgui_make_rect(localRect.right - scaledPaddingX, 0,                                 localRect.right,                  localRect.bottom), bgcolor, pPaintData); // Right
    drgui_draw_rect(pCmdBar, drgui_make_rect(scaledPaddingX,                   0,                                 localRect.right - scaledPaddingX, scaledPaddingY),   bgcolor, pPaintData); // Top
    drgui_draw_rect(pCmdBar, drgui_make_rect(scaledPaddingX,                   localRect.bottom - scaledPaddingY, localRect.right - scaledPaddingX, localRect.bottom), bgcolor, pPaintData); // Bottom


    // Message.
    dred_rect lrect;
    dred_rect mrect;
    dred_rect rrect;
    dred_cmdbar__get_segment_rects(pCmdBar, &lrect, &mrect, &rrect);
    drgui_draw_rect(pCmdBar, mrect, bgcolor, pPaintData);

    dred_gui_font* pMessageFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);

    dred_gui_font_metrics messageFontMetrics;
    drgui_get_font_metrics(pMessageFont, &messageFontMetrics);

    float messageLeft = mrect.left + (4*pDred->uiScale);
    float messageTop  = (((mrect.bottom - mrect.top) - messageFontMetrics.lineHeight) / 2) + scaledPaddingY;
    drgui_draw_text(pCmdBar, pMessageFont, data->message, (int)strlen(data->message), messageLeft, messageTop, pDred->config.cmdbarTextColor, bgcolor, pPaintData);
}

void dred_cmdbar_tb__on_capture_keyboard(dred_textbox* pTextBox, dred_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;

    dred_cmdbar* pCmdBar = dred_control_get_parent(pTextBox);
    assert(pCmdBar != NULL);

    dred_cmdbar_data* data = dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    dred_context* pDred = dred_control_get_context(pCmdBar);
    assert(pDred != NULL);

    // Activate the focused styles.
    dred_textbox_set_text_color(data->pTextBox, pDred->config.cmdbarTextColorActive);
    dred_textbox_set_cursor_color(data->pTextBox, pDred->config.cmdbarTextColorActive);
    dred_textbox_set_background_color(data->pTextBox, pDred->config.cmdbarBGColorActive);
    dred_textbox_set_active_line_background_color(data->pTextBox, pDred->config.cmdbarBGColorActive);

    // If auto-hiding is enabled we need to show the command bar.
    if (pDred->config.autoHideCmdBar) {
        dred_show_command_bar(pDred);
    }

    // Hide any message that's showing.
    dred_cmdbar_set_message(pCmdBar, "");

    drgui_dirty(pCmdBar, drgui_get_local_rect(pCmdBar));


    // Fall through to the default handler.
    dred_textbox_on_capture_keyboard(pTextBox, pPrevCapturedElement);
}

void dred_cmdbar_tb__on_release_keyboard(dred_textbox* pTextBox, dred_element* pNextCapturedElement)
{
    (void)pNextCapturedElement;

    dred_cmdbar* pCmdBar = dred_control_get_parent(pTextBox);
    assert(pCmdBar != NULL);

    dred_cmdbar_data* data = dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    dred_context* pDred = dred_control_get_context(pCmdBar);
    assert(pDred != NULL);

    // If the element being captured is the inner text box, just ignore it and pretend that we're not actually losing focus.
    if (dred_control_is_descendant(pNextCapturedElement, dred_cmdbar__get_textbox(pCmdBar))) {
        return;
    }

    // Deactivate unfocused styles.
    dred_textbox_set_text_color(data->pTextBox, pDred->config.cmdbarTextColor);
    dred_textbox_set_cursor_color(data->pTextBox, pDred->config.cmdbarTextColor);
    dred_textbox_set_background_color(data->pTextBox, pDred->config.cmdbarBGColor);
    dred_textbox_set_active_line_background_color(data->pTextBox, pDred->config.cmdbarBGColor);
    

    // If auto-hiding is enabled we need to hide the command bar.
    if (pDred->config.autoHideCmdBar) {
        dred_hide_command_bar(pDred);
    }

    drgui_dirty(pCmdBar, drgui_get_local_rect(pCmdBar));


    // Fall through to the default handler.
    dred_textbox_on_release_keyboard(pTextBox, pNextCapturedElement);
}

void dred_cmdbar_tb__on_key_down(dred_textbox* pTextBox, dred_key key, int stateFlags)
{
    (void)stateFlags;

    // The parent is the command bar.
    dred_cmdbar* pCmdBar = dred_control_get_parent(pTextBox);
    assert(pCmdBar != NULL);

    dred_cmdbar_data* pData = dred_control_get_extra_data(pCmdBar);
    assert(pData != NULL);

    dred_context* pDred = dred_control_get_context(pCmdBar);
    assert(pDred != NULL);

    dred_cmdbar_clear_message(pCmdBar);

    switch (key)
    {
        case DRED_GUI_ESCAPE:
        {
            dred_unfocus_command_bar(pDred);
        } break;

        case DRED_GUI_ARROW_UP:
        {
            if (pData->iPrevCommand == 0) {
                // The working command needs to be saved so it can be restored later if the user pressed the down key.
                size_t cmdLen = dred_textbox_get_text(pTextBox, NULL, 0);
                if (cmdLen > 0) {
                    pData->workingCommand = malloc(cmdLen + 1);
                    if (dred_textbox_get_text(pTextBox, pData->workingCommand, cmdLen + 1) != cmdLen) {
                        free(pData->workingCommand);
                        pData->workingCommand = NULL;
                    }
                }
            }

            if (dred_cmdbar_set_text_to_previous_command(pCmdBar, pData->iPrevCommand)) {
                pData->iPrevCommand += 1;
            }
        } break;

        case DRED_GUI_ARROW_DOWN:
        {
            if (pData->iPrevCommand > 0) {
                if (dred_cmdbar_set_text_to_previous_command(pCmdBar, pData->iPrevCommand - 1)) {
                    pData->iPrevCommand -= 1;
                }
            }

            if (pData->iPrevCommand == 0) {
                dred_cmdbar_set_text(pCmdBar, pData->workingCommand);
            }
        } break;

        default: 
        {
            dred_textbox_on_key_down(pTextBox, key, stateFlags);
        } break;
    }
}

void dred_cmdbar_tb__on_printable_key_down(dred_textbox* pTextBox, uint32_t utf32, int stateFlags)
{
    (void)stateFlags;

    // The parent is the command bar.
    dred_cmdbar* pCmdBar = dred_control_get_parent(pTextBox);
    assert(pCmdBar != NULL);

    dred_cmdbar_data* pData = dred_control_get_extra_data(pCmdBar);
    assert(pData != NULL);

    dred_context* pDred = dred_control_get_context(pCmdBar);
    assert(pDred != NULL);

    if (utf32 == '\r' || utf32 == '\n')
    {
        size_t cmdLen = dred_textbox_get_text(pTextBox, NULL, 0);
        char* cmd = malloc(cmdLen + 1);
        if (dred_textbox_get_text(pTextBox, cmd, cmdLen + 1) == cmdLen)
        {
            dred_command command;
            dred_exec(pDred, cmd, &command);

            if ((command.flags & DRED_CMDBAR_NO_CLEAR) == 0) {
                dred_textbox_set_text(pTextBox, "");
            }

            if ((command.flags & DRED_CMDBAR_RELEASE_KEYBOARD) != 0) {
                dred_unfocus_command_bar(dred_control_get_context(pCmdBar));
            }

            dred_config_push_recent_cmd(&pDred->config, cmd);
            pData->iPrevCommand = 0;

            if (pData->workingCommand) {
                free(pData->workingCommand);
                pData->workingCommand = NULL;
            }
        }

        free(cmd);
    }
    else
    {
        dred_textbox_on_printable_key_down(pTextBox, utf32, stateFlags);
    }
}

void dred_cmdbar__update_size(dred_cmdbar* pCmdBar)
{
    dred_cmdbar_data* data = (dred_cmdbar_data*)dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    dred_context* pDred = dred_control_get_context(pCmdBar);
    assert(pDred != NULL);


    dred_gui_font_metrics fontMetricsTB;
    drgui_get_font_metrics(dred_textbox_get_font(data->pTextBox), &fontMetricsTB);

    dred_gui_font* pMessageFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);

    dred_gui_font_metrics fontMetricsMsg;
    drgui_get_font_metrics(pMessageFont, &fontMetricsMsg);

    float textboxHeight = (float)fontMetricsTB.lineHeight + dred_textbox_get_padding_vert(data->pTextBox)*2;
    float messageHeight = (float)fontMetricsMsg.lineHeight;
    float infobarHeight = (float)drgui_get_height(data->pInfoBar);

    float cmdbarHeight = dr_max(textboxHeight, dr_max(messageHeight, infobarHeight)) + (pDred->config.cmdbarPaddingY*pDred->uiScale*2);
    float cmdbarWidth = 0;
    if (pCmdBar->pParent != NULL) {
        cmdbarWidth = dred_control_get_width(pCmdBar->pParent);
    }
    dred_control_set_size(pCmdBar, cmdbarWidth, cmdbarHeight);


    // A change in size will require the inner controls to have their layouts updated for centering and whatnot.
    dred_cmdbar__update_layouts_of_inner_controls(pCmdBar);
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

    dred_textbox_disable_horizontal_scrollbar(data->pTextBox);
    dred_textbox_disable_vertical_scrollbar(data->pTextBox);

    // Events.
    dred_control_set_on_size(pCmdBar, dred_cmdbar__on_size);
    dred_control_set_on_capture_keyboard(pCmdBar, dred_cmdbar__on_capture_keyboard);
    dred_control_set_on_paint(pCmdBar, dred_cmdbar__on_paint);

    // Text box event overrides.
    dred_control_set_on_capture_keyboard(data->pTextBox, dred_cmdbar_tb__on_capture_keyboard);
    dred_control_set_on_release_keyboard(data->pTextBox, dred_cmdbar_tb__on_release_keyboard);
    dred_control_set_on_key_down(data->pTextBox, dred_cmdbar_tb__on_key_down);
    dred_control_set_on_printable_key_down(data->pTextBox, dred_cmdbar_tb__on_printable_key_down);


    strcpy_s(data->message, sizeof(data->message), "");


    // The info bar.
    data->pInfoBar = dred_info_bar_create(pDred, pCmdBar);



    dred_cmdbar_refresh_styling(pCmdBar);
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
        free(data->workingCommand);
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

    dred_textbox_deselect_all(data->pTextBox);
    dred_textbox_set_text(data->pTextBox, text);
    dred_textbox_move_cursor_to_end_of_text(data->pTextBox);
}

bool dred_cmdbar_set_text_to_previous_command(dred_cmdbar* pCmdBar, unsigned int iPrevCommand)
{
    dred_cmdbar_data* pData = (dred_cmdbar_data*)dred_control_get_extra_data(pCmdBar);
    if (pData == NULL) {
        return false;
    }

    if (iPrevCommand >= pData->pDred->config.recentCommandsCount) {
        return false;
    }

    dred_cmdbar_set_text(pCmdBar, pData->pDred->config.recentCommands[iPrevCommand]);
    return true;
}


bool dred_cmdbar_has_keyboard_focus(dred_cmdbar* pCmdBar)
{
    dred_cmdbar_data* data = (dred_cmdbar_data*)dred_control_get_extra_data(pCmdBar);
    if (data == NULL) {
        return false;
    }

    return dred_control_has_keyboard_capture(data->pTextBox);
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

void dred_cmdbar_refresh_styling(dred_cmdbar* pCmdBar)
{
    dred_cmdbar_data* data = (dred_cmdbar_data*)dred_control_get_extra_data(pCmdBar);
    if (data == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(pCmdBar);
    if (pDred == NULL) {
        return;
    }

    // Textbox.
    dred_textbox_set_font(data->pTextBox, dred_font_acquire_subfont(pDred->config.pCmdbarTBFont, pDred->uiScale));

    if (dred_cmdbar_has_keyboard_focus(pCmdBar)) {
        dred_textbox_set_text_color(data->pTextBox, pDred->config.cmdbarTextColorActive);
        dred_textbox_set_cursor_color(data->pTextBox, pDred->config.cmdbarTextColorActive);
        dred_textbox_set_background_color(data->pTextBox, pDred->config.cmdbarBGColorActive);
        dred_textbox_set_active_line_background_color(data->pTextBox, pDred->config.cmdbarBGColorActive);
    } else {
        dred_textbox_set_text_color(data->pTextBox, pDred->config.cmdbarTextColor);
        dred_textbox_set_cursor_color(data->pTextBox, pDred->config.cmdbarTextColor);
        dred_textbox_set_background_color(data->pTextBox, pDred->config.cmdbarBGColor);
        dred_textbox_set_active_line_background_color(data->pTextBox, pDred->config.cmdbarBGColor);
    }

    // Info bar.
    dred_info_bar_refresh_styling(data->pInfoBar);


    // The command bar may need to be resized.
    dred_cmdbar__update_size(pCmdBar);

    // Redraw.
    drgui_dirty(pCmdBar, drgui_get_local_rect(pCmdBar));
}

// Copyright (C) 2016 David Reid. See included LICENSE file.

dred_rect dred_cmdbar__get_inner_rect(dred_cmdbar* pCmdBar)
{
    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pCmdBar));
    assert(pDred != NULL);

    float cmdbarWidth;
    float cmdbarHeight;
    dred_control_get_size(DRED_CONTROL(pCmdBar), &cmdbarWidth, &cmdbarHeight);

    float scaledPaddingX = pDred->config.cmdbarPaddingX*pDred->uiScale;
    float scaledPaddingY = pDred->config.cmdbarPaddingY*pDred->uiScale;
    return dred_make_rect(scaledPaddingX, scaledPaddingY, cmdbarWidth - scaledPaddingX, cmdbarHeight - scaledPaddingY);
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
    dred_rect lrect = dred_make_rect(innerRect.left, innerRect.top, innerRect.left + segWidth, innerRect.bottom);
    dred_rect mrect = dred_make_rect(lrect.right, innerRect.top, lrect.right + segWidth, innerRect.bottom);
    dred_rect rrect = dred_make_rect(mrect.right, innerRect.top, innerRect.right, innerRect.bottom);

    if (pLRect) *pLRect = lrect;
    if (pMRect) *pMRect = mrect;
    if (pRRect) *pRRect = rrect;
}


void dred_cmdbar__update_layouts_of_inner_controls(dred_cmdbar* pCmdBar)
{
    assert(pCmdBar != NULL);

    // Controls need to be resized based on their rectangles.
    dred_rect lrect;
    dred_rect mrect;
    dred_rect rrect;
    dred_cmdbar__get_segment_rects(pCmdBar, &lrect, &mrect, &rrect);

    dred_control_set_relative_position(DRED_CONTROL(pCmdBar->pTextBox), lrect.left, lrect.top);
    dred_control_set_size(DRED_CONTROL(pCmdBar->pTextBox), lrect.right - lrect.left, lrect.bottom - lrect.top);
    
    dred_control_set_relative_position(DRED_CONTROL(pCmdBar->pInfoBar), rrect.left, rrect.top);
    dred_control_set_size(DRED_CONTROL(pCmdBar->pInfoBar), rrect.right - rrect.left, rrect.bottom - rrect.top);
}


void dred_cmdbar__on_size(dred_control* pControl, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    dred_cmdbar* pCmdBar = DRED_CMDBAR(pControl);
    assert(pCmdBar != NULL);

    dred_cmdbar__update_layouts_of_inner_controls(pCmdBar);
}

void dred_cmdbar__on_capture_keyboard(dred_control* pControl, dred_control* pPrevCapturedControl)
{
    (void)pPrevCapturedControl;

    dred_cmdbar* pCmdBar = DRED_CMDBAR(pControl);
    assert(pCmdBar != NULL);

    dred_gui_capture_keyboard(DRED_CONTROL(pCmdBar->pTextBox));
}

void dred_cmdbar__on_paint(dred_control* pControl, dred_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dred_cmdbar* pCmdBar = DRED_CMDBAR(pControl);
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pCmdBar));
    assert(pDred != NULL);

    dred_rect localRect = dred_control_get_local_rect(DRED_CONTROL(pCmdBar));

    dtk_color bgcolor = pDred->config.cmdbarBGColor;
    if (dred_cmdbar_has_keyboard_focus(pCmdBar)) {
        bgcolor = pDred->config.cmdbarBGColorActive;
    }

    float scaledPaddingX = pDred->config.cmdbarPaddingX*pDred->uiScale;
    float scaledPaddingY = pDred->config.cmdbarPaddingY*pDred->uiScale;
    dred_control_draw_rect(DRED_CONTROL(pCmdBar), dred_make_rect(0,                                0,                                 scaledPaddingX,                   localRect.bottom), bgcolor, pSurface); // Left
    dred_control_draw_rect(DRED_CONTROL(pCmdBar), dred_make_rect(localRect.right - scaledPaddingX, 0,                                 localRect.right,                  localRect.bottom), bgcolor, pSurface); // Right
    dred_control_draw_rect(DRED_CONTROL(pCmdBar), dred_make_rect(scaledPaddingX,                   0,                                 localRect.right - scaledPaddingX, scaledPaddingY),   bgcolor, pSurface); // Top
    dred_control_draw_rect(DRED_CONTROL(pCmdBar), dred_make_rect(scaledPaddingX,                   localRect.bottom - scaledPaddingY, localRect.right - scaledPaddingX, localRect.bottom), bgcolor, pSurface); // Bottom


    // Message.
    dred_rect lrect;
    dred_rect mrect;
    dred_rect rrect;
    dred_cmdbar__get_segment_rects(pCmdBar, &lrect, &mrect, &rrect);
    dred_control_draw_rect(DRED_CONTROL(pCmdBar), mrect, bgcolor, pSurface);

    dred_gui_font* pMessageFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);

    dtk_font_metrics messageFontMetrics;
    dred_gui_get_font_metrics(pMessageFont, &messageFontMetrics);

    float messageLeft = mrect.left + (4*pDred->uiScale);
    float messageTop  = (((mrect.bottom - mrect.top) - messageFontMetrics.lineHeight) / 2) + scaledPaddingY;
    dred_control_draw_text(DRED_CONTROL(pCmdBar), pMessageFont, pCmdBar->message, (int)strlen(pCmdBar->message), messageLeft, messageTop, pDred->config.cmdbarTextColor, bgcolor, pSurface);
}

void dred_cmdbar_tb__on_capture_keyboard(dred_control* pControl, dred_control* pPrevCapturedControl)
{
    (void)pPrevCapturedControl;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    dred_cmdbar* pCmdBar = DRED_CMDBAR(dred_control_get_parent(DRED_CONTROL(pTextBox)));
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pCmdBar));
    assert(pDred != NULL);

    // Activate the focused styles.
    dred_textbox_set_text_color(pCmdBar->pTextBox, pDred->config.cmdbarTextColorActive);
    dred_textbox_set_cursor_color(pCmdBar->pTextBox, pDred->config.cmdbarTextColorActive);
    dred_textbox_set_background_color(pCmdBar->pTextBox, pDred->config.cmdbarBGColorActive);
    dred_textbox_set_active_line_background_color(pCmdBar->pTextBox, pDred->config.cmdbarBGColorActive);

    // If auto-hiding is enabled we need to show the command bar.
    if (pDred->config.autoHideCmdBar) {
        dred_show_command_bar(pDred);
    }

    // Hide any message that's showing.
    dred_cmdbar_set_message(pCmdBar, "");

    dred_control_dirty(DRED_CONTROL(pCmdBar), dred_control_get_local_rect(DRED_CONTROL(pCmdBar)));


// TODO: Fix the command bar popup for GTK.
#ifdef DRED_WIN32
    // Show the popup window.
    dred_cmdbar_popup_show(pDred->pCmdBarPopup);
#endif

    // Fall through to the default handler.
    dred_textview_on_capture_keyboard(DRED_CONTROL(pTextBox), pPrevCapturedControl);
}

void dred_cmdbar_tb__on_release_keyboard(dred_control* pControl, dred_control* pNextCapturedControl)
{
    (void)pNextCapturedControl;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    dred_cmdbar* pCmdBar = DRED_CMDBAR(dred_control_get_parent(DRED_CONTROL(pTextBox)));
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pCmdBar));
    assert(pDred != NULL);

    

    // If the element being captured is the inner text box, just ignore it and pretend that we're not actually losing focus.
    if (dred_control_is_descendant(pNextCapturedControl, DRED_CONTROL(pCmdBar->pTextBox))) {
        return;
    }

// TODO: Fix the command bar popup for GTK.
#ifdef DRED_WIN32
    dred_cmdbar_popup_hide(pDred->pCmdBarPopup);
#endif

    // Deactivate unfocused styles.
    dred_textbox_set_text_color(pCmdBar->pTextBox, pDred->config.cmdbarTextColor);
    dred_textbox_set_cursor_color(pCmdBar->pTextBox, pDred->config.cmdbarTextColor);
    dred_textbox_set_background_color(pCmdBar->pTextBox, pDred->config.cmdbarBGColor);
    dred_textbox_set_active_line_background_color(pCmdBar->pTextBox, pDred->config.cmdbarBGColor);
    

    // If auto-hiding is enabled we need to hide the command bar.
    if (pDred->config.autoHideCmdBar) {
        dred_hide_command_bar(pDred);
    }

    dred_control_dirty(DRED_CONTROL(pCmdBar), dred_control_get_local_rect(DRED_CONTROL(pCmdBar)));


    // Fall through to the default handler.
    dred_textview_on_release_keyboard(DRED_CONTROL(pTextBox), pNextCapturedControl);
}

void dred_cmdbar_tb__on_key_down(dred_control* pControl, dtk_key key, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    // The parent is the command bar.
    dred_cmdbar* pCmdBar = DRED_CMDBAR(dred_control_get_parent(DRED_CONTROL(pTextBox)));
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pCmdBar));
    assert(pDred != NULL);

    dred_cmdbar_clear_message(pCmdBar);

    switch (key)
    {
        case DTK_KEY_ESCAPE:
        {
            dred_unfocus_command_bar(pDred);
        } break;

        case DTK_KEY_ARROW_UP:
        {
            if (pCmdBar->iPrevCommand == 0) {
                // The working command needs to be saved so it can be restored later if the user pressed the down key.
                size_t cmdLen = dred_textbox_get_text(pTextBox, NULL, 0);
                if (cmdLen > 0) {
                    pCmdBar->workingCommand = (char*)malloc(cmdLen + 1);
                    if (dred_textbox_get_text(pTextBox, pCmdBar->workingCommand, cmdLen + 1) != cmdLen) {
                        free(pCmdBar->workingCommand);
                        pCmdBar->workingCommand = NULL;
                    }
                }
            }

            if (dred_cmdbar_set_text_to_previous_command(pCmdBar, pCmdBar->iPrevCommand)) {
                pCmdBar->iPrevCommand += 1;
            }
        } break;

        case DTK_KEY_ARROW_DOWN:
        {
            if (pCmdBar->iPrevCommand > 0) {
                if (dred_cmdbar_set_text_to_previous_command(pCmdBar, pCmdBar->iPrevCommand - 1)) {
                    pCmdBar->iPrevCommand -= 1;
                }
            }

            if (pCmdBar->iPrevCommand == 0) {
                dred_cmdbar_set_text(pCmdBar, pCmdBar->workingCommand);
            }
        } break;

        default: 
        {
            dred_textview_on_key_down(DRED_CONTROL(pTextBox), key, stateFlags);
        } break;
    }
}

void dred_cmdbar_tb__on_printable_key_down(dred_control* pControl, uint32_t utf32, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    // The parent is the command bar.
    dred_cmdbar* pCmdBar = DRED_CMDBAR(dred_control_get_parent(DRED_CONTROL(pTextBox)));
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pCmdBar));
    assert(pDred != NULL);

    if (utf32 == '\r' || utf32 == '\n')
    {
        size_t cmdLen = dred_textbox_get_text(pTextBox, NULL, 0);
        char* cmd = (char*)malloc(cmdLen + 1);
        if (dred_textbox_get_text(pTextBox, cmd, cmdLen + 1) == cmdLen)
        {
            dred_command command;
            dred_exec(pDred, cmd, &command);

            if ((command.flags & DRED_CMDBAR_NO_CLEAR) == 0) {
                dred_textbox_set_text(pTextBox, "");
            }

            if ((command.flags & DRED_CMDBAR_RELEASE_KEYBOARD) != 0) {
                dred_unfocus_command_bar(dred_control_get_context(DRED_CONTROL(pCmdBar)));
            }

            dred_config_push_recent_cmd(&pDred->config, cmd);
            pCmdBar->iPrevCommand = 0;

            if (pCmdBar->workingCommand) {
                free(pCmdBar->workingCommand);
                pCmdBar->workingCommand = NULL;
            }
        }

        free(cmd);
    }
    else
    {
        dred_textview_on_printable_key_down(DRED_CONTROL(pTextBox), utf32, stateFlags);
    }
}

void dred_cmdbar_tb__on_text_changed(dred_textbox* pTextBox)
{
    // The parent is the command bar.
    dred_cmdbar* pCmdBar = DRED_CMDBAR(dred_control_get_parent(DRED_CONTROL(pTextBox)));
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pCmdBar));
    assert(pDred != NULL);


    char* pText = dred_textbox_get_text_malloc(pTextBox);
    if (pText == NULL) {
        return;
    }

    dred_cmdbar_popup_refresh_autocomplete(pDred->pCmdBarPopup, pText);
    free(pText);
}

void dred_cmdbar__update_size(dred_cmdbar* pCmdBar)
{
    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pCmdBar));
    assert(pDred != NULL);


    dtk_font_metrics fontMetricsTB;
    dred_gui_get_font_metrics(dred_textbox_get_font(pCmdBar->pTextBox), &fontMetricsTB);

    dred_gui_font* pMessageFont = dred_font_acquire_subfont(pDred->config.pUIFont, pDred->uiScale);

    dtk_font_metrics fontMetricsMsg;
    dred_gui_get_font_metrics(pMessageFont, &fontMetricsMsg);

    float textboxHeight = (float)fontMetricsTB.lineHeight + dred_textbox_get_padding_vert(pCmdBar->pTextBox)*2;
    float messageHeight = (float)fontMetricsMsg.lineHeight;
    float infobarHeight = (float)dred_control_get_height(DRED_CONTROL(pCmdBar->pInfoBar));

    float cmdbarHeight = dr_max(textboxHeight, dr_max(messageHeight, infobarHeight)) + (pDred->config.cmdbarPaddingY*pDred->uiScale*2);
    float cmdbarWidth = 0;
    if (dred_control_get_parent(DRED_CONTROL(pCmdBar)) != NULL) {
        cmdbarWidth = dred_control_get_width(dred_control_get_parent(DRED_CONTROL(pCmdBar)));
    }
    dred_control_set_size(DRED_CONTROL(pCmdBar), cmdbarWidth, cmdbarHeight);


    // A change in size will require the inner controls to have their layouts updated for centering and whatnot.
    dred_cmdbar__update_layouts_of_inner_controls(pCmdBar);
}

dr_bool32 dred_cmdbar_init(dred_cmdbar* pCmdBar, dred_context* pDred, dred_control* pParent)
{
    if (pCmdBar == NULL) {
        return DR_FALSE;
    }

    memset(pCmdBar, 0, sizeof(*pCmdBar));
    if (!dred_control_init(DRED_CONTROL(pCmdBar), pDred, pParent, DRED_CONTROL_TYPE_CMDBAR)) {
        return DR_FALSE;
    }


    pCmdBar->pDred = pDred;

    pCmdBar->pTextBox = &pCmdBar->textBox;
    if (!dred_textbox_init(pCmdBar->pTextBox, pDred, DRED_CONTROL(pCmdBar))) {
        dred_control_uninit(DRED_CONTROL(pCmdBar));
        return DR_FALSE;
    }

    dred_textbox_disable_horizontal_scrollbar(pCmdBar->pTextBox);
    dred_textbox_disable_vertical_scrollbar(pCmdBar->pTextBox);
    dred_textbox_set_on_text_changed(pCmdBar->pTextBox, dred_cmdbar_tb__on_text_changed);

    // Events.
    dred_control_set_on_size(DRED_CONTROL(pCmdBar), dred_cmdbar__on_size);
    dred_control_set_on_capture_keyboard(DRED_CONTROL(pCmdBar), dred_cmdbar__on_capture_keyboard);
    dred_control_set_on_paint(DRED_CONTROL(pCmdBar), dred_cmdbar__on_paint);

    // Text box event overrides.
    dred_control_set_on_capture_keyboard(DRED_CONTROL(pCmdBar->pTextBox), dred_cmdbar_tb__on_capture_keyboard);
    dred_control_set_on_release_keyboard(DRED_CONTROL(pCmdBar->pTextBox), dred_cmdbar_tb__on_release_keyboard);
    dred_control_set_on_key_down(DRED_CONTROL(pCmdBar->pTextBox), dred_cmdbar_tb__on_key_down);
    dred_control_set_on_printable_key_down(DRED_CONTROL(pCmdBar->pTextBox), dred_cmdbar_tb__on_printable_key_down);

    strcpy_s(pCmdBar->message, sizeof(pCmdBar->message), "");


    // The info bar.
    pCmdBar->pInfoBar = &pCmdBar->infoBar;
    dred_info_bar_init(pCmdBar->pInfoBar, pDred, DRED_CONTROL(pCmdBar));



    dred_cmdbar_refresh_styling(pCmdBar);
    return DR_TRUE;
}

void dred_cmdbar_uninit(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) {
        return;
    }

    dred_textbox_uninit(pCmdBar->pTextBox);
    free(pCmdBar->workingCommand);

    dred_control_uninit(DRED_CONTROL(pCmdBar));
}


void dred_cmdbar_set_text(dred_cmdbar* pCmdBar, const char* text)
{
    if (pCmdBar == NULL) {
        return;
    }

    if (text == NULL) {
        text = "";
    }

    dred_textbox_deselect_all(pCmdBar->pTextBox);
    dred_textbox_set_text(pCmdBar->pTextBox, text);
    dred_textbox_move_cursor_to_end_of_text(pCmdBar->pTextBox);
}

dr_bool32 dred_cmdbar_set_text_to_previous_command(dred_cmdbar* pCmdBar, unsigned int iPrevCommand)
{
    if (pCmdBar == NULL) {
        return DR_FALSE;
    }

    if (iPrevCommand >= pCmdBar->pDred->config.recentCommandsCount) {
        return DR_FALSE;
    }

    dred_cmdbar_set_text(pCmdBar, pCmdBar->pDred->config.recentCommands[iPrevCommand]);
    return DR_TRUE;
}

size_t dred_cmdbar_get_text(dred_cmdbar* pCmdBar, char* pTextOut, size_t textOutSize)
{
    if (pCmdBar == NULL) return 0;
    return dred_textbox_get_text(pCmdBar->pTextBox, pTextOut, textOutSize);
}

char* dred_cmdbar_get_text_malloc(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) return 0;
    return dred_textbox_get_text_malloc(pCmdBar->pTextBox);
}


dr_bool32 dred_cmdbar_has_keyboard_focus(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) {
        return DR_FALSE;
    }

    return dred_control_has_keyboard_capture(DRED_CONTROL(pCmdBar->pTextBox));
}


void dred_cmdbar_set_message(dred_cmdbar* pCmdBar, const char* text)
{
    if (pCmdBar == NULL) {
        return;
    }

    if (text == NULL) {
        text = "";
    }

    strncpy_s(pCmdBar->message, sizeof(pCmdBar->message), text, _TRUNCATE);
    dred_control_dirty(DRED_CONTROL(pCmdBar), dred_control_get_local_rect(DRED_CONTROL(pCmdBar)));    // <-- Can optimize this to only draw the message region.
}

void dred_cmdbar_clear_message(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) {
        return;
    }

    if (pCmdBar->message[0] != '\0') {
        dred_cmdbar_set_message(pCmdBar, "");
    }
}

void dred_cmdbar_update_info_bar(dred_cmdbar* pCmdBar, dred_control* pControl)
{
    if (pCmdBar == NULL) {
        return;
    }

    dred_info_bar_update(pCmdBar->pInfoBar, pControl);
}

void dred_cmdbar_refresh_styling(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) {
        return;
    }

    dred_context* pDred = dred_control_get_context(DRED_CONTROL(pCmdBar));
    if (pDred == NULL) {
        return;
    }

    // Textbox.
    dred_textbox_set_font(pCmdBar->pTextBox, dred_font_acquire_subfont(pDred->config.pCmdbarTBFont, pDred->uiScale));

    if (dred_cmdbar_has_keyboard_focus(pCmdBar)) {
        dred_textbox_set_text_color(pCmdBar->pTextBox, pDred->config.cmdbarTextColorActive);
        dred_textbox_set_cursor_color(pCmdBar->pTextBox, pDred->config.cmdbarTextColorActive);
        dred_textbox_set_background_color(pCmdBar->pTextBox, pDred->config.cmdbarBGColorActive);
        dred_textbox_set_active_line_background_color(pCmdBar->pTextBox, pDred->config.cmdbarBGColorActive);
    } else {
        dred_textbox_set_text_color(pCmdBar->pTextBox, pDred->config.cmdbarTextColor);
        dred_textbox_set_cursor_color(pCmdBar->pTextBox, pDred->config.cmdbarTextColor);
        dred_textbox_set_background_color(pCmdBar->pTextBox, pDred->config.cmdbarBGColor);
        dred_textbox_set_active_line_background_color(pCmdBar->pTextBox, pDred->config.cmdbarBGColor);
    }

    // Info bar.
    dred_info_bar_refresh_styling(pCmdBar->pInfoBar);


    // The command bar may need to be resized.
    dred_cmdbar__update_size(pCmdBar);

    // Redraw.
    dred_control_dirty(DRED_CONTROL(pCmdBar), dred_control_get_local_rect(DRED_CONTROL(pCmdBar)));
}

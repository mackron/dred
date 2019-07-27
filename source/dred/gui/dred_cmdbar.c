// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_rect dred_cmdbar__get_inner_rect(dred_cmdbar* pCmdBar)
{
    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pCmdBar));
    assert(pDred != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pCmdBar));

    dtk_int32 cmdbarWidth;
    dtk_int32 cmdbarHeight;
    dtk_control_get_size(DTK_CONTROL(pCmdBar), &cmdbarWidth, &cmdbarHeight);

    dtk_int32 scaledPaddingX = (dtk_int32)(pDred->config.cmdbarPaddingX*uiScale);
    dtk_int32 scaledPaddingY = (dtk_int32)(pDred->config.cmdbarPaddingY*uiScale);
    return dtk_rect_init(scaledPaddingX, scaledPaddingY, cmdbarWidth - scaledPaddingX, cmdbarHeight - scaledPaddingY);
}

void dred_cmdbar__get_inner_size(dred_cmdbar* pCmdBar, dtk_int32* pWidthOut, dtk_int32* pHeightOut)
{
    dtk_rect innerRect = dred_cmdbar__get_inner_rect(pCmdBar);

    if (pWidthOut) *pWidthOut = innerRect.right - innerRect.left;
    if (pHeightOut) *pHeightOut = innerRect.bottom - innerRect.top;
}

void dred_cmdbar__get_segment_rects(dred_cmdbar* pCmdBar, dtk_rect* pLRect, dtk_rect* pMRect, dtk_rect* pRRect)
{
    dtk_rect innerRect = dred_cmdbar__get_inner_rect(pCmdBar);

    dtk_int32 innerWidth = innerRect.right - innerRect.left;
    dtk_int32 lwidth = (dtk_int32)(innerWidth * 0.5f);
    dtk_int32 mwidth = (dtk_int32)(innerWidth * 0.25f);
    //float rwidth = innerWidth * 0.25f;

    dtk_rect lrect = dtk_rect_init(innerRect.left, innerRect.top, innerRect.left + lwidth, innerRect.bottom);
    dtk_rect mrect = dtk_rect_init(lrect.right, innerRect.top, lrect.right + mwidth, innerRect.bottom);
    dtk_rect rrect = dtk_rect_init(mrect.right, innerRect.top, innerRect.right, innerRect.bottom);

    if (pLRect) *pLRect = lrect;
    if (pMRect) *pMRect = mrect;
    if (pRRect) *pRRect = rrect;
}


void dred_cmdbar__update_layouts_of_inner_controls(dred_cmdbar* pCmdBar)
{
    assert(pCmdBar != NULL);

    // Controls need to be resized based on their rectangles.
    dtk_rect lrect;
    dtk_rect mrect;
    dtk_rect rrect;
    dred_cmdbar__get_segment_rects(pCmdBar, &lrect, &mrect, &rrect);

    dtk_control_set_relative_position(DTK_CONTROL(&pCmdBar->textBox), lrect.left, lrect.top);
    dtk_control_set_size(DTK_CONTROL(&pCmdBar->textBox), lrect.right - lrect.left, lrect.bottom - lrect.top);
    
    dtk_control_set_relative_position(DTK_CONTROL(&pCmdBar->infoBar), rrect.left, rrect.top);
    dtk_control_set_size(DTK_CONTROL(&pCmdBar->infoBar), rrect.right - rrect.left, rrect.bottom - rrect.top);
}

void dred_cmdbar__update_manual_text_entry(dred_cmdbar* pCmdBar)
{
    dred_textview* pTextView = DRED_TEXTVIEW(&pCmdBar->textBox);

    dtk_free(pCmdBar->manualTextEntry);
    pCmdBar->manualTextEntry = NULL;

    size_t textLen = dred_textview_get_text(pTextView, NULL, 0);
    pCmdBar->manualTextEntry = (char*)dtk_malloc(textLen + 1);
    if (pCmdBar->manualTextEntry == NULL) {
        return;     // Out of memory.
    }

    dred_textview_get_text(pTextView, pCmdBar->manualTextEntry, textLen+1);
}

dtk_bool32 dred_cmdbar__does_manual_text_entry_contain_whole_command_name(dred_cmdbar* pCmdBar)
{
    dtk_assert(pCmdBar != NULL);
    
    if (pCmdBar->manualTextEntry == NULL) {
        return DTK_FALSE;
    }

    const char* pSearchResult = dtk_first_whitespace(pCmdBar->manualTextEntry);
    if (pSearchResult == NULL || pSearchResult[0] == '\0') {
        return DTK_FALSE;
    }

    return DTK_TRUE;
}

void dred_cmdbar__update_text_based_on_autocomplete(dred_cmdbar* pCmdBar)
{
    if (!dred_cmdbar__does_manual_text_entry_contain_whole_command_name(pCmdBar)) {
        dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pCmdBar));
        dtk_assert(pDred != NULL);

        // The text of the command bar needs to be set to the whole command name, with the trailing section highlighted.
        dred_cmdbox_cmdlist* pCmdList = &pDred->cmdbarPopup.cmdlist;

        const char* wholeCommandText = dred_cmdbox_cmdlist_get_highlighted_command_name(pCmdList);
        if (wholeCommandText == NULL) {
            return;
        }

        // The call to dred_cmdbar_set_text() will reset the manual text entry, so it'll need to be restored.
        char* manualTextEntryCopy = dtk_make_string((pCmdBar->manualTextEntry != NULL) ? pCmdBar->manualTextEntry : "");
        dred_cmdbar_set_text(pCmdBar, wholeCommandText);
        if (manualTextEntryCopy != NULL) {
            pCmdBar->manualTextEntry = manualTextEntryCopy;
        }
    }
}


void dred_cmdbar__on_size(dred_control* pControl, dtk_int32 newWidth, dtk_int32 newHeight)
{
    (void)newWidth;
    (void)newHeight;

    dred_cmdbar* pCmdBar = DRED_CMDBAR(pControl);
    assert(pCmdBar != NULL);

    dred_cmdbar__update_layouts_of_inner_controls(pCmdBar);
}

void dred_cmdbar__on_capture_keyboard(dred_control* pControl, dtk_control* pPrevCapturedControl)
{
    (void)pPrevCapturedControl;

    dred_cmdbar* pCmdBar = DRED_CMDBAR(pControl);
    assert(pCmdBar != NULL);

    dtk_capture_keyboard(DTK_CONTROL(pCmdBar)->pTK, DTK_CONTROL(&pCmdBar->textBox));
}

void dred_cmdbar__on_paint(dred_control* pControl, dtk_rect rect, dtk_surface* pSurface)
{
    (void)rect;

    dred_cmdbar* pCmdBar = DRED_CMDBAR(pControl);
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pCmdBar));
    assert(pDred != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pCmdBar));
    dtk_rect localRect = dtk_control_get_local_rect(DTK_CONTROL(pCmdBar));

    dtk_color bgcolor = pDred->config.cmdbarBGColor;
    if (dred_cmdbar_has_keyboard_focus(pCmdBar)) {
        bgcolor = pDred->config.cmdbarBGColorActive;
    }

    dtk_int32 scaledPaddingX = (dtk_int32)(pDred->config.cmdbarPaddingX*uiScale);
    dtk_int32 scaledPaddingY = (dtk_int32)(pDred->config.cmdbarPaddingY*uiScale);
    dtk_surface_draw_rect(pSurface, dtk_rect_init(0,                                0,                                 scaledPaddingX,                   localRect.bottom), bgcolor); // Left
    dtk_surface_draw_rect(pSurface, dtk_rect_init(localRect.right - scaledPaddingX, 0,                                 localRect.right,                  localRect.bottom), bgcolor); // Right
    dtk_surface_draw_rect(pSurface, dtk_rect_init(scaledPaddingX,                   0,                                 localRect.right - scaledPaddingX, scaledPaddingY),   bgcolor); // Top
    dtk_surface_draw_rect(pSurface, dtk_rect_init(scaledPaddingX,                   localRect.bottom - scaledPaddingY, localRect.right - scaledPaddingX, localRect.bottom), bgcolor); // Bottom


    // Message.
    dtk_rect lrect;
    dtk_rect mrect;
    dtk_rect rrect;
    dred_cmdbar__get_segment_rects(pCmdBar, &lrect, &mrect, &rrect);
    dtk_surface_draw_rect(pSurface, mrect, bgcolor);

    dtk_font* pMessageFont = &pDred->config.pUIFont->fontDTK;

    dtk_font_metrics messageFontMetrics;
    dtk_font_get_metrics(pMessageFont, uiScale, &messageFontMetrics);

    dtk_int32 messageLeft =(dtk_int32)(mrect.left + (4*uiScale));
    dtk_int32 messageTop  = (((mrect.bottom - mrect.top) - messageFontMetrics.lineHeight) / 2) + scaledPaddingY;
    dtk_surface_draw_text(pSurface, pMessageFont, uiScale, pCmdBar->message, (int)strlen(pCmdBar->message), (dtk_int32)messageLeft, (dtk_int32)messageTop, pDred->config.cmdbarTextColor, bgcolor);
}

void dred_cmdbar_tb__on_capture_keyboard(dred_control* pControl, dtk_control* pPrevCapturedControl)
{
    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    dred_cmdbar* pCmdBar = DRED_CMDBAR(dtk_control_get_parent(DTK_CONTROL(pTextBox)));
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pCmdBar));
    assert(pDred != NULL);

    // Activate the focused styles.
    dred_textbox_set_text_color(&pCmdBar->textBox, pDred->config.cmdbarTextColorActive);
    dred_textbox_set_cursor_color(&pCmdBar->textBox, pDred->config.cmdbarTextColorActive);
    dred_textbox_set_background_color(&pCmdBar->textBox, pDred->config.cmdbarBGColorActive);
    dred_textbox_set_active_line_background_color(&pCmdBar->textBox, pDred->config.cmdbarBGColorActive);

    // If auto-hiding is enabled we need to show the command bar.
    if (pDred->config.autoHideCmdBar) {
        dred_show_command_bar(pDred);
    }

    // Hide any message that's showing.
    dred_cmdbar_set_message(pCmdBar, "");

    dtk_control_scheduled_redraw(DTK_CONTROL(pCmdBar), dtk_control_get_local_rect(DTK_CONTROL(pCmdBar)));


    // Show the popup window.
    dred_cmdbar_popup_show(&pDred->cmdbarPopup);

    // Fall through to the default handler.
    dred_textview_on_capture_keyboard(DRED_CONTROL(pTextBox), pPrevCapturedControl);
}

void dred_cmdbar_tb__on_release_keyboard(dred_control* pControl, dtk_control* pNextCapturedControl)
{
    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    dred_cmdbar* pCmdBar = DRED_CMDBAR(dtk_control_get_parent(DTK_CONTROL(pTextBox)));
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pCmdBar));
    assert(pDred != NULL);


    // If the element being captured is the inner text box, just ignore it and pretend that we're not actually losing focus.
    if (dtk_control_is_descendant(pNextCapturedControl, DTK_CONTROL(&pCmdBar->textBox))) {
        return;
    }

    dred_cmdbar_popup_hide(&pDred->cmdbarPopup);

    // Deactivate unfocused styles.
    dred_textbox_set_text_color(&pCmdBar->textBox, pDred->config.cmdbarTextColor);
    dred_textbox_set_cursor_color(&pCmdBar->textBox, pDred->config.cmdbarTextColor);
    dred_textbox_set_background_color(&pCmdBar->textBox, pDred->config.cmdbarBGColor);
    dred_textbox_set_active_line_background_color(&pCmdBar->textBox, pDred->config.cmdbarBGColor);
    

    // If auto-hiding is enabled we need to hide the command bar.
    if (pDred->config.autoHideCmdBar) {
        dred_hide_command_bar(pDred);
    }

    dtk_control_scheduled_redraw(DTK_CONTROL(pCmdBar), dtk_control_get_local_rect(DTK_CONTROL(pCmdBar)));


    // Fall through to the default handler.
    dred_textview_on_release_keyboard(DRED_CONTROL(pTextBox), pNextCapturedControl);
}

void dred_cmdbar_tb__on_key_down(dred_control* pControl, dtk_key key, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    // The parent is the command bar.
    dred_cmdbar* pCmdBar = DRED_CMDBAR(dtk_control_get_parent(DTK_CONTROL(pTextBox)));
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pCmdBar));
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
                pCmdBar->iPrevCommand -= 1;
                if (pCmdBar->iPrevCommand > 0) {
                    dred_cmdbar_set_text_to_previous_command(pCmdBar, pCmdBar->iPrevCommand - 1);
                }                
            }

            if (pCmdBar->iPrevCommand == 0) {
                dred_cmdbar_set_text(pCmdBar, pCmdBar->workingCommand);
            }
        } break;

        case DTK_KEY_TAB:
        {
            // We first need to determine what it is exactly we're tabbing through. If we have not yet completed the command name, we cycle through
            // the possible commands. Otherwise, we cycle through the arguments. To determine whether or not the user has completed the command name,
            // we just search for a whitespace character.
            if (pCmdBar->manualTextEntry != NULL && !dred_cmdbar__does_manual_text_entry_contain_whole_command_name(pCmdBar)) {
                // Cycle through commands.
                if (stateFlags & DTK_MODIFIER_SHIFT) {
                    dred_cmdbox_cmdlist_highlight_prev_item(&pDred->cmdbarPopup.cmdlist);
                } else {
                    dred_cmdbox_cmdlist_highlight_next_item(&pDred->cmdbarPopup.cmdlist);
                }

                dred_cmdbar__update_text_based_on_autocomplete(pCmdBar);
                dred_cmdbar_popup_refresh_autocomplete(&pDred->cmdbarPopup, pCmdBar->manualTextEntry);
            } else {
                // Cycle through parameters.
                if (stateFlags & DTK_MODIFIER_SHIFT) {
                    dred_cmdbar_highlight_prev_argument(pCmdBar);
                } else {
                    dred_cmdbar_highlight_next_argument(pCmdBar);
                }
            }
        } break;

        default: 
        {
            dred_textview_on_key_down(DRED_CONTROL(pTextBox), key, stateFlags);

            if (key == DTK_KEY_DELETE || key == DTK_KEY_BACKSPACE) {
                dred_cmdbar__update_manual_text_entry(pCmdBar);
                dred_cmdbar_popup_refresh_autocomplete(&pDred->cmdbarPopup, pCmdBar->manualTextEntry);
            }
        } break;
    }
}

void dred_cmdbar_tb__on_printable_key_down(dred_control* pControl, uint32_t utf32, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = DRED_TEXTBOX(pControl);
    assert(pTextBox != NULL);

    // The parent is the command bar.
    dred_cmdbar* pCmdBar = DRED_CMDBAR(dtk_control_get_parent(DTK_CONTROL(pTextBox)));
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pCmdBar));
    assert(pDred != NULL);

    // Tabs are handle in a special way for the command bar.
    if (utf32 == '\t') {
        return;
    }

    if (utf32 == '\r' || utf32 == '\n') {
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
                dred_unfocus_command_bar(dred_get_context_from_control(DTK_CONTROL(pCmdBar)));
            }

            dred_config_push_recent_cmd(&pDred->config, cmd);
            pCmdBar->iPrevCommand = 0;

            if (pCmdBar->workingCommand) {
                free(pCmdBar->workingCommand);
                pCmdBar->workingCommand = NULL;
            }
            if (pCmdBar->manualTextEntry) {
                free(pCmdBar->manualTextEntry);
                pCmdBar->manualTextEntry = NULL;
            }
        }

        free(cmd);
    } else {
        dred_textview_on_printable_key_down(DRED_CONTROL(pTextBox), utf32, stateFlags);
        dred_cmdbar__update_manual_text_entry(pCmdBar);
        dred_cmdbar_popup_refresh_autocomplete(&pDred->cmdbarPopup, pCmdBar->manualTextEntry);
    }
}

void dred_cmdbar_tb__on_text_changed(dred_textbox* pTextBox)
{
    // The parent is the command bar.
    dred_cmdbar* pCmdBar = DRED_CMDBAR(dtk_control_get_parent(DTK_CONTROL(pTextBox)));
    assert(pCmdBar != NULL);

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pCmdBar));
    assert(pDred != NULL);

    if (pCmdBar->manualTextEntry == NULL) {
        char* pText = dred_textbox_get_text_malloc(pTextBox);
        if (pText == NULL) {
            return;
        }

        dred_cmdbar_popup_refresh_autocomplete(&pDred->cmdbarPopup, pText);
        free(pText);
    } else {
        dred_cmdbar_popup_refresh_autocomplete(&pDred->cmdbarPopup, pCmdBar->manualTextEntry);
    }
}

void dred_cmdbar__update_size(dred_cmdbar* pCmdBar)
{
    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pCmdBar));
    assert(pDred != NULL);

    float uiScale = dtk_control_get_scaling_factor(DTK_CONTROL(pCmdBar));

    dtk_font_metrics fontMetricsTB;
    dtk_font_get_metrics(dred_textbox_get_font(&pCmdBar->textBox), uiScale, &fontMetricsTB);

    dtk_font* pMessageFont = &pDred->config.pUIFont->fontDTK;

    dtk_font_metrics fontMetricsMsg;
    dtk_font_get_metrics(pMessageFont, uiScale, &fontMetricsMsg);

    dtk_int32 textboxHeight = fontMetricsTB.lineHeight + dred_textbox_get_padding_vert(&pCmdBar->textBox)*2;
    dtk_int32 messageHeight = fontMetricsMsg.lineHeight;
    dtk_int32 infobarHeight = dtk_control_get_height(DTK_CONTROL(&pCmdBar->infoBar));

    dtk_int32 cmdbarHeight = (dtk_int32)(dtk_max(textboxHeight, dtk_max(messageHeight, infobarHeight)) + (pDred->config.cmdbarPaddingY*uiScale*2));
    dtk_int32 cmdbarWidth = 0;
    if (dtk_control_get_parent(DTK_CONTROL(pCmdBar)) != NULL) {
        cmdbarWidth = dtk_control_get_width(dtk_control_get_parent(DTK_CONTROL(pCmdBar)));
    }
    dtk_control_set_size(DTK_CONTROL(pCmdBar), cmdbarWidth, cmdbarHeight);


    // A change in size will require the inner controls to have their layouts updated for centering and whatnot.
    dred_cmdbar__update_layouts_of_inner_controls(pCmdBar);
}

dtk_bool32 dred_cmdbar_init(dred_cmdbar* pCmdBar, dred_context* pDred, dtk_control* pParent)
{
    if (pCmdBar == NULL) {
        return DTK_FALSE;
    }

    memset(pCmdBar, 0, sizeof(*pCmdBar));
    if (!dred_control_init(DRED_CONTROL(pCmdBar), pDred, NULL, pParent, DRED_CONTROL_TYPE_CMDBAR, NULL)) {
        return DTK_FALSE;
    }

    if (!dred_textbox_init(&pCmdBar->textBox, pDred, DRED_CONTROL(pCmdBar))) {
        dred_control_uninit(DRED_CONTROL(pCmdBar));
        return DTK_FALSE;
    }

    dred_textbox_disable_horizontal_scrollbar(&pCmdBar->textBox);
    dred_textbox_disable_vertical_scrollbar(&pCmdBar->textBox);
    dred_textbox_set_on_text_changed(&pCmdBar->textBox, dred_cmdbar_tb__on_text_changed);

    // Events.
    dred_control_set_on_size(DRED_CONTROL(pCmdBar), dred_cmdbar__on_size);
    dred_control_set_on_capture_keyboard(DRED_CONTROL(pCmdBar), dred_cmdbar__on_capture_keyboard);
    dred_control_set_on_paint(DRED_CONTROL(pCmdBar), dred_cmdbar__on_paint);

    // Text box event overrides.
    dred_control_set_on_capture_keyboard(DRED_CONTROL(&pCmdBar->textBox), dred_cmdbar_tb__on_capture_keyboard);
    dred_control_set_on_release_keyboard(DRED_CONTROL(&pCmdBar->textBox), dred_cmdbar_tb__on_release_keyboard);
    dred_control_set_on_key_down(DRED_CONTROL(&pCmdBar->textBox), dred_cmdbar_tb__on_key_down);
    dred_control_set_on_printable_key_down(DRED_CONTROL(&pCmdBar->textBox), dred_cmdbar_tb__on_printable_key_down);

    strcpy_s(pCmdBar->message, sizeof(pCmdBar->message), "");


    // The info bar.
    dred_info_bar_init(pDred, DTK_CONTROL(pCmdBar), &pCmdBar->infoBar);



    dred_cmdbar_refresh_styling(pCmdBar);
    return DTK_TRUE;
}

void dred_cmdbar_uninit(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) {
        return;
    }

    dred_textbox_uninit(&pCmdBar->textBox);
    free(pCmdBar->workingCommand);
    free(pCmdBar->manualTextEntry);

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

    dred_textbox_deselect_all(&pCmdBar->textBox);
    dred_textbox_set_text(&pCmdBar->textBox, text);
    dred_textbox_move_cursor_to_end_of_text(&pCmdBar->textBox);

    dtk_free(pCmdBar->manualTextEntry);
    pCmdBar->manualTextEntry = NULL;
}

dtk_bool32 dred_cmdbar_set_text_to_previous_command(dred_cmdbar* pCmdBar, unsigned int iPrevCommand)
{
    if (pCmdBar == NULL) {
        return DTK_FALSE;
    }

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pCmdBar));
    dtk_assert(pDred != NULL);

    if (iPrevCommand >= pDred->config.recentCommandsCount) {
        return DTK_FALSE;
    }

    dred_cmdbar_set_text(pCmdBar, pDred->config.recentCommands[iPrevCommand]);
    return DTK_TRUE;
}

size_t dred_cmdbar_get_text(dred_cmdbar* pCmdBar, char* pTextOut, size_t textOutSize)
{
    if (pCmdBar == NULL) return 0;
    return dred_textbox_get_text(&pCmdBar->textBox, pTextOut, textOutSize);
}

char* dred_cmdbar_get_text_malloc(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) return 0;
    return dred_textbox_get_text_malloc(&pCmdBar->textBox);
}


void dred_cmdbar_select_text(dred_cmdbar* pCmdBar, size_t firstCharacter, size_t lastCharacter)
{
    if (pCmdBar == NULL) return;
    dred_textbox_deselect_all(&pCmdBar->textBox);
    dred_textbox_select(&pCmdBar->textBox, firstCharacter, lastCharacter);
    dred_textbox_move_cursor_to_character(&pCmdBar->textBox, 0, lastCharacter);
}


void dred_cmdbar_highlight_next_argument(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) return;

    unsigned int argumentCount = dred_cmdbar_get_argument_count(pCmdBar);
    if (argumentCount == 0) {
        return;
    }
    
    unsigned int currentIndex = dred_cmdbar_get_argument_index_by_cursor(pCmdBar);
    if (currentIndex == (unsigned int)-1) {
        currentIndex = 0;
    } else {
        currentIndex = (currentIndex + 1) % argumentCount;
    }

    dred_cmdbar_highlight_argument_by_index(pCmdBar, currentIndex);
}

void dred_cmdbar_highlight_prev_argument(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) return;

    unsigned int argumentCount = dred_cmdbar_get_argument_count(pCmdBar);
    if (argumentCount == 0) {
        return;
    }
    
    unsigned int currentIndex = dred_cmdbar_get_argument_index_by_cursor(pCmdBar);
    if (currentIndex == (unsigned int)-1) {
        currentIndex = 0;
    } else {
        if (currentIndex > 0) {
            currentIndex -= 1;
        } else {
            currentIndex = argumentCount - 1;
        }
    }

    dred_cmdbar_highlight_argument_by_index(pCmdBar, currentIndex);
}

void dred_cmdbar_highlight_argument_by_index(dred_cmdbar* pCmdBar, unsigned int argumentIndex)
{
    if (pCmdBar == NULL) return;

    char* pCmdBarText = dred_cmdbar_get_text_malloc(pCmdBar);
    if (pCmdBarText == NULL) {
        return;
    }

    size_t charBeg;
    size_t charEnd;
    if (!dred_cmdbar_get_argument_character_range_by_index(pCmdBar, argumentIndex, &charBeg, &charEnd)) {
        return; // Didn't find the argument.
    }

    dred_textbox_deselect_all(&pCmdBar->textBox);

    // charBeg and charEnd will both include any double quotes. Since double-quotes are useful, we'll try to be smart and
    // pull the selection region inside the quotes, assuming they exist.
    size_t argumentLength = charEnd - charBeg;
    if (argumentLength > 0) {
        dred_cmdbar_select_text(pCmdBar, charBeg, charEnd);
    } else {
        dred_textbox_move_cursor_to_character(&pCmdBar->textBox, 0, charBeg);
    }
    
    free(pCmdBarText);
}

unsigned int dred_cmdbar_get_argument_index_by_cursor(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) return (unsigned int)-1;
    return dred_cmdbar_get_argument_index_by_character_index(pCmdBar, dred_textbox_get_cursor_character(&pCmdBar->textBox, 0));
}

unsigned int dred_cmdbar_get_argument_index_by_character_index(dred_cmdbar* pCmdBar, size_t characterIndex)
{
    if (pCmdBar == NULL) return (unsigned int)-1;
    
    char* pCmdBarText = dred_cmdbar_get_text_malloc(pCmdBar);
    if (pCmdBarText == NULL) {
        return (unsigned int)-1;
    }

    size_t prevTokenEnd = 0;

    unsigned int count = 0;
    const char* pNextStr = pCmdBarText;
    for (;;) {
        char token[4096];

        size_t thisTokenBeg = pNextStr - pCmdBarText;
        pNextStr = dtk_next_token(pNextStr, token, sizeof(token));
        if (pNextStr == NULL) {
            break;
        }

        prevTokenEnd = pNextStr - pCmdBarText;
        if (characterIndex >= thisTokenBeg && characterIndex <= prevTokenEnd) {
            break;
        }

        count += 1;
    }

    free(pCmdBarText);
    return count - 1;   // -1 because "count" will include the command name which is not a command. This also naturally returns -1 when the character index is not over an argument.
}

unsigned int dred_cmdbar_get_argument_count(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) return 0;

    char* pCmdBarText = dred_cmdbar_get_text_malloc(pCmdBar);
    if (pCmdBarText == NULL) {
        return 0;
    }

    unsigned int count = 0;
    const char* pNextStr = pCmdBarText;
    for (;;) {
        char token[4096];
        pNextStr = dtk_next_token(pNextStr, token, sizeof(token));
        if (pNextStr == NULL) {
            break;
        }

        count += 1;
    }

    free(pCmdBarText);

    // At this point the count contains the total number of tokens. However, one of those tokens is the command name, which is not an argument.
    if (count == 0) {
        return 0;
    }
    return count - 1;
}

dtk_bool32 dred_cmdbar_get_argument_character_range_by_index(dred_cmdbar* pCmdBar, unsigned int argumentIndex, size_t* pCharBeg, size_t* pCharEnd)
{
    if (pCmdBar == NULL) return DTK_FALSE;

    // Safety.
    if (pCharBeg != NULL) *pCharBeg = 0;
    if (pCharEnd != NULL) *pCharEnd = 0;

    char* pCmdBarText = dred_cmdbar_get_text_malloc(pCmdBar);
    if (pCmdBarText == NULL) {
        return DTK_FALSE;
    }

    dtk_bool32 foundArgument = DTK_FALSE;

    unsigned int count = 0;
    const char* pNextStr = pCmdBarText;
    for (;;) {
        size_t charBeg = dtk_ltrim(pNextStr) - pCmdBarText;
        if (pCmdBarText[charBeg] == '\"') {
            charBeg += 1;
        }

        char token[4096];
        pNextStr = dtk_next_token(pNextStr, token, sizeof(token));
        if (pNextStr == NULL) {
            break;
        }

        size_t charEnd = charBeg + strlen(token);
        if (count == argumentIndex + 1) {   // +1 because the first token will be the command name (not an argument).
            *pCharBeg = charBeg;
            *pCharEnd = charEnd;
            foundArgument = DTK_TRUE;
            break;
        }

        count += 1;
    }

    free(pCmdBarText);
    return foundArgument;
}


dtk_bool32 dred_cmdbar_has_keyboard_focus(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) {
        return DTK_FALSE;
    }

    return dtk_control_has_keyboard_capture(DTK_CONTROL(&pCmdBar->textBox));
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
    dtk_control_scheduled_redraw(DTK_CONTROL(pCmdBar), dtk_control_get_local_rect(DTK_CONTROL(pCmdBar)));    // <-- Can optimize this to only draw the message region.
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

    dred_info_bar_update(&pCmdBar->infoBar, pControl);
}

void dred_cmdbar_refresh_styling(dred_cmdbar* pCmdBar)
{
    if (pCmdBar == NULL) {
        return;
    }

    dred_context* pDred = dred_get_context_from_control(DTK_CONTROL(pCmdBar));
    if (pDred == NULL) {
        return;
    }

    // Textbox.
    dred_textbox_set_font(&pCmdBar->textBox, &pDred->config.pCmdbarTBFont->fontDTK);
    dred_textbox_set_font_scale(&pCmdBar->textBox, dtk_control_get_scaling_factor(DTK_CONTROL(pCmdBar)));

    if (dred_cmdbar_has_keyboard_focus(pCmdBar)) {
        dred_textbox_set_text_color(&pCmdBar->textBox, pDred->config.cmdbarTextColorActive);
        dred_textbox_set_cursor_color(&pCmdBar->textBox, pDred->config.cmdbarTextColorActive);
        dred_textbox_set_background_color(&pCmdBar->textBox, pDred->config.cmdbarBGColorActive);
        dred_textbox_set_active_line_background_color(&pCmdBar->textBox, pDred->config.cmdbarBGColorActive);
    } else {
        dred_textbox_set_text_color(&pCmdBar->textBox, pDred->config.cmdbarTextColor);
        dred_textbox_set_cursor_color(&pCmdBar->textBox, pDred->config.cmdbarTextColor);
        dred_textbox_set_background_color(&pCmdBar->textBox, pDred->config.cmdbarBGColor);
        dred_textbox_set_active_line_background_color(&pCmdBar->textBox, pDred->config.cmdbarBGColor);
    }

    // Info bar.
    dred_info_bar_refresh_styling(&pCmdBar->infoBar);


    // The command bar may need to be resized.
    dred_cmdbar__update_size(pCmdBar);

    // Redraw.
    dtk_control_scheduled_redraw(DTK_CONTROL(pCmdBar), dtk_control_get_local_rect(DTK_CONTROL(pCmdBar)));
}

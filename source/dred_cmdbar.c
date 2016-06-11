
typedef struct
{
    dred_context* pDred;
    dred_textbox* pTextBox;
} dred_cmdbar_data;

void dred_cmdbar__on_size(dred_cmdbar* pCmdBar, float newWidth, float newHeight)
{
    dred_cmdbar_data* data = dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    dred_control_set_size(data->pTextBox, newWidth, newHeight);
}

void dred_cmdbar__on_capture_keyboard(dred_cmdbar* pCmdBar, drgui_element* pPrevCapturedElement)
{
    dred_cmdbar_data* data = dred_control_get_extra_data(pCmdBar);
    assert(data != NULL);

    drgui_capture_keyboard(data->pTextBox);
}

void dred_cmdbar_tb__on_key_down(dred_textbox* pTextBox, drgui_key key, int stateFlags)
{
    (void)stateFlags;

    // The parent is the command bar.
    dred_cmdbar* pCmdBar = dred_control_get_parent(pTextBox);
    assert(pCmdBar != NULL);

    if (key == DRGUI_ESCAPE) {
        dred_release_keyboard(dred_control_get_context(pCmdBar));
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
                dred_release_keyboard(dred_control_get_context(pCmdBar));
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

    dred_textbox_set_font(data->pTextBox, dred_font_acquire_subfont(pDred->config.pCmdbarTBFont, 1));
    dred_textbox_set_background_color(data->pTextBox, pDred->config.cmdbarBGColor);
    dred_textbox_set_active_line_background_color(data->pTextBox, pDred->config.cmdbarBGColor);
    dred_textbox_disable_horizontal_scrollbar(data->pTextBox);
    dred_textbox_disable_vertical_scrollbar(data->pTextBox);


    // Events.
    dred_control_set_on_size(pCmdBar, dred_cmdbar__on_size);
    dred_control_set_on_capture_keyboard(pCmdBar, dred_cmdbar__on_capture_keyboard);


    // Text box event overrides.
    dred_control_set_on_key_down(data->pTextBox, dred_cmdbar_tb__on_key_down);
    dred_control_set_on_printable_key_down(data->pTextBox, dred_cmdbar_tb__on_printable_key_down);

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
    }

    dred_control_delete(pCmdBar);
}
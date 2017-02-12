// Copyright (C) 2016 David Reid. See included LICENSE file.

dtk_result dtk_textbox_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pTextBox);

    dtk_result result = dtk_control_init(pTK, pParent, DTK_CONTROL_TYPE_TEXTBOX, onEvent, &pTextBox->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_textbox_uninit(dtk_textbox* pTextBox)
{
    if (pTextBox == NULL) return DTK_INVALID_ARGS;

    return dtk_control_uninit(&pTextBox->control);
}


dtk_bool32 dtk_textbox_default_event_handler(dtk_event* pEvent)
{
    if (pEvent == NULL) return DTK_FALSE;

    return DTK_TRUE;
}
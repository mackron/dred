// Copyright (C) 2017 David Reid. See included LICENSE file.

dtk_result dtk_button_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_button* pButton)
{
    if (pButton == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pButton);

    dtk_result result = dtk_control_init(pTK, pParent, DTK_CONTROL_TYPE_BUTTON, onEvent, &pButton->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_button_uninit(dtk_button* pButton)
{
    if (pButton == NULL) return DTK_INVALID_ARGS;

    return dtk_control_uninit(&pButton->control);
}


dtk_bool32 dtk_button_default_event_handler(dtk_event* pEvent)
{
    if (pEvent == NULL) return DTK_FALSE;

    return DTK_TRUE;
}
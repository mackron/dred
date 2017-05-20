// Copyright (C) 2017 David Reid. See included LICENSE file.

dtk_result dtk_color_button_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_color_button* pColorButton)
{
    if (pColorButton == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pColorButton);

    dtk_result result = dtk_control_init(pTK, pParent, DTK_CONTROL_TYPE_COLOR_BUTTON, onEvent, &pColorButton->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_color_button_uninit(dtk_color_button* pColorButton)
{
    if (pColorButton == NULL) return DTK_INVALID_ARGS;

    return dtk_control_uninit(&pColorButton->control);
}


dtk_bool32 dtk_color_button_default_event_handler(dtk_event* pEvent)
{
    if (pEvent == NULL) return DTK_FALSE;

    return DTK_TRUE;
}
// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_bool32 dred_color_button_event_handler(dtk_event* pEvent)
{
    dred_color_button* pButton = DRED_COLOR_BUTTON(pEvent->pControl);
    
    switch (pEvent->type)
    {
        case DTK_EVENT_COLOR_BUTTON_COLOR_CHANGED:
        {
            if (pButton->varBinding[0] != '\0') {
                char colorStr[256];
                snprintf(colorStr, sizeof(colorStr), "%d %d %d %d", pEvent->colorButton.color.r, pEvent->colorButton.color.g, pEvent->colorButton.color.b, pEvent->colorButton.color.a);
                dred_config_set(&dred_get_context_from_control(DTK_CONTROL(pButton))->config, pButton->varBinding, colorStr, 0);
            }
        } break;

        default: break;
    }

    return dtk_color_button_default_event_handler(pEvent);
}

dtk_result dred_color_button_init(dred_context* pDred, dtk_control* pParent, const char* text, dtk_color color, dred_color_button* pButton)
{
    if (pButton == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pButton);

    dtk_result result = dtk_color_button_init(&pDred->tk, dred_color_button_event_handler, pParent, text, color, &pButton->buttonDTK);
    if (result != DTK_SUCCESS) {
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dred_color_button_uninit(dred_color_button* pButton)
{
    if (pButton == NULL) return DTK_INVALID_ARGS;

    dtk_color_button_uninit(&pButton->buttonDTK);
    return DTK_SUCCESS;
}


void dred_color_button_set_bind_to_config_var(dred_color_button* pButton, const char* varName)
{
    if (pButton == NULL) return;

    if (varName != NULL) {
        strcpy_s(pButton->varBinding, sizeof(pButton->varBinding), varName);
    } else {
        pButton->varBinding[0] = '\0';
    }
}

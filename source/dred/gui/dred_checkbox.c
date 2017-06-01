// Copyright (C) 2017 David Reid. See included LICENSE file.

dtk_bool32 dred_checkbox_event_handler(dtk_event* pEvent)
{
    dred_checkbox* pCheckbox = DRED_CHECKBOX(pEvent->pControl);
    
    switch (pEvent->type)
    {
        case DTK_EVENT_CHECKBOX_CHECK_CHANGED:
        {
            if (pCheckbox->varBinding[0] != '\0') {
                dred_config_set(&dred_get_context_from_control(DTK_CONTROL(pCheckbox))->config, pCheckbox->varBinding, dtk_checkbox_is_checked(DTK_CHECKBOX(pCheckbox)) ? "true" : "false");
            }
        } break;

        default: break;
    }

    return dtk_checkbox_default_event_handler(pEvent);
}

dtk_result dred_checkbox_init(dred_context* pDred, dtk_control* pParent, const char* text, dtk_bool32 checked, dred_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pCheckbox);

    dtk_result result = dtk_checkbox_init(&pDred->tk, pParent, dred_checkbox_event_handler, text, checked, &pCheckbox->checkboxDTK);
    if (result != DTK_SUCCESS) {
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dred_checkbox_uninit(dred_checkbox* pCheckbox)
{
    if (pCheckbox == NULL) return DTK_INVALID_ARGS;

    dtk_checkbox_uninit(&pCheckbox->checkboxDTK);
    return DTK_SUCCESS;
}


void dred_checkbox_set_bind_to_config_var(dred_checkbox* pCheckbox, const char* varName)
{
    if (pCheckbox == NULL) {
        return;
    }

    if (varName != NULL) {
        strcpy_s(pCheckbox->varBinding, sizeof(pCheckbox->varBinding), varName);
    } else {
        pCheckbox->varBinding[0] = '\0';
    }
}
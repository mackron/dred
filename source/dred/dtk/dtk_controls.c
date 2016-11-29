// Copyright (C) 2016 David Reid. See included LICENSE file.

dtk_result dtk_control_init(dtk_context* pTK, dtk_control_type type, dtk_event_proc onEvent, dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pControl);

    if (pTK == NULL) return DTK_INVALID_ARGS;
    pControl->pTK = pTK;
    pControl->type = type;
    pControl->onEvent = onEvent;

    dtk_result result = dtk_font_init(pTK, "Courier New", 24, dtk_font_weight_bold, dtk_font_slant_italic, 0, 0, &pControl->font);
    return DTK_SUCCESS;
}

dtk_result dtk_control_uninit(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;
    return DTK_SUCCESS;
}

dtk_result dtk_control_set_event_handler(dtk_control* pControl, dtk_event_proc onEvent)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    dtk_atomic_exchange_ptr(&pControl->onEvent, onEvent);
    return DTK_SUCCESS;
}

dtk_result dtk_control_show(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;
    
    // Special case for windows.
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_show(DTK_WINDOW(pControl), DTK_SHOW_NORMAL);
    }

    // TODO: Implement me.
    return DTK_SUCCESS;
}

dtk_result dtk_control_hide(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // Special case for windows.
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_hide(DTK_WINDOW(pControl));
    }

    // TODO: Implement me.
    return DTK_SUCCESS;
}
// Copyright (C) 2016 David Reid. See included LICENSE file.

dtk_result dtk_tabbar_init(dtk_context* pTK, dtk_control* pParent, dtk_event_proc onEvent, dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pTabBar);

    dtk_result result = dtk_control_init(pTK, pParent, DTK_CONTROL_TYPE_TABBAR, onEvent, &pTabBar->control);
    if (result != DTK_SUCCESS) {
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_tabbar_uninit(dtk_tabbar* pTabBar)
{
    if (pTabBar == NULL) return DTK_INVALID_ARGS;

    return dtk_control_uninit(&pTabBar->control);
}


dtk_bool32 dtk_tabbar_default_event_handler(dtk_event* pEvent)
{
    if (pEvent == NULL) return DTK_FALSE;

    return DTK_TRUE;
}
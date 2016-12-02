// Copyright (C) 2016 David Reid. See included LICENSE file.

dtk_result dtk_control_init(dtk_context* pTK, dtk_control* pParent, dtk_control_type type, dtk_event_proc onEvent, dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pControl);

    if (pTK == NULL || pParent == pControl) return DTK_INVALID_ARGS;
    pControl->pTK = pTK;
    pControl->type = type;
    pControl->onEvent = onEvent;
    pControl->pParent = pParent;

    dtk_font_init(pTK, "Courier New", 24, dtk_font_weight_bold, dtk_font_slant_italic, 0, 0, &pControl->font);
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


dtk_result dtk_control_set_size(dtk_control* pControl, dtk_uint32 width, dtk_uint32 height)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // Special case for windows.
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_set_size(DTK_WINDOW(pControl), width, height);
    } else {
        pControl->width = width;
        pControl->height = height;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_control_get_size(dtk_control* pControl, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    if (pWidth) *pWidth = 0;
    if (pHeight) *pHeight = 0;
    if (pControl == NULL) return DTK_INVALID_ARGS;

    if (pWidth) *pWidth = pControl->width;
    if (pHeight) *pHeight = pControl->height;
    return DTK_SUCCESS;
}

dtk_result dtk_control_set_absolute_position(dtk_control* pControl, dtk_int32 posX, dtk_int32 posY)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // Special case for windows.
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_set_absolute_position(DTK_WINDOW(pControl), posX, posY);
    } else {
        pControl->absolutePosX = posX;
        pControl->absolutePosY = posY;
        return DTK_SUCCESS; 
    }
}

dtk_result dtk_control_get_absolute_position(dtk_control* pControl, dtk_int32* pPosX, dtk_int32* pPosY)
{
    if (pPosX) *pPosX = 0;  // Safety.
    if (pPosY) *pPosY = 0;  // ^
    if (pControl == NULL) return DTK_INVALID_ARGS;

    if (pPosX) *pPosX = pControl->absolutePosX;
    if (pPosY) *pPosY = pControl->absolutePosY;
    return DTK_SUCCESS;
}

dtk_result dtk_control_set_relative_position(dtk_control* pControl, dtk_int32 posX, dtk_int32 posY)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;
    
    if (pControl->pParent == NULL) {
        return dtk_control_set_absolute_position(pControl, posX, posY);
    }

    dtk_control_relative_to_absolute(pControl->pParent, &posX, &posY);
    return dtk_control_set_absolute_position(pControl, posX, posY);
}

dtk_result dtk_control_get_relative_position(dtk_control* pControl, dtk_int32* pPosX, dtk_int32* pPosY)
{
    if (pPosX) *pPosX = 0;  // Safety.
    if (pPosY) *pPosY = 0;  // ^
    if (pControl == NULL) return DTK_INVALID_ARGS;

    dtk_result result = dtk_control_get_absolute_position(pControl, pPosX, pPosY);
    if (result != DTK_SUCCESS) {
        return result;
    }

    if (pControl->pParent != NULL) {
        return dtk_control_absolute_to_relative(pControl->pParent, pPosX, pPosY);
    }

    return DTK_SUCCESS;
}

dtk_result dtk_control_relative_to_absolute(dtk_control* pControl, dtk_int32* pPosX, dtk_int32* pPosY)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    dtk_int32 absolutePosX = 0;
    dtk_int32 absolutePosY = 0;
    dtk_control_get_absolute_position(pControl, &absolutePosX, &absolutePosY);

    if (pPosX) *pPosX += absolutePosX;
    if (pPosY) *pPosY += absolutePosY;
    return DTK_SUCCESS;
}

dtk_result dtk_control_absolute_to_relative(dtk_control* pControl, dtk_int32* pPosX, dtk_int32* pPosY)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    dtk_int32 absolutePosX;
    dtk_int32 absolutePosY;
    dtk_control_get_absolute_position(pControl, &absolutePosX, &absolutePosY);

    if (pPosX) *pPosX -= absolutePosX;
    if (pPosY) *pPosY -= absolutePosY;
    return DTK_SUCCESS;
}


dtk_control* dtk_control_find_top_level_control(dtk_control* pControl)
{
    while (pControl != NULL) {
        if (pControl->pParent == NULL) {
            return pControl;    // <-- This is the top-level control.
        }

        pControl = pControl->pParent;
    }

    return NULL;
}

dtk_window* dtk_control_get_window(dtk_control* pControl)
{
    while (pControl != NULL) {
        if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
            return DTK_WINDOW(pControl);
        }

        pControl = pControl->pParent;
    }

    return NULL;
}

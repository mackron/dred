// Copyright (C) 2016 David Reid. See included LICENSE file.

void dtk_control__link_child(dtk_control* pParent, dtk_control* pChild)
{
    dtk_assert(pParent != NULL);
    dtk_assert(pChild != NULL);
    dtk_assert(pChild->pParent == NULL);    // <-- The child should not already be attached to a parent.

    pChild->pParent = pChild;

    if (pParent->pFirstChild == NULL) {
        pParent->pFirstChild = pChild;
        dtk_assert(pParent->pLastChild == NULL);
    } else {
        pChild->pPrevSibling = pParent->pLastChild;
        pChild->pNextSibling = NULL;
        pChild->pPrevSibling->pNextSibling = pChild;
    }

    pParent->pLastChild = pChild;
}

void dtk_control__unlink_child(dtk_control* pParent, dtk_control* pChild)
{
    dtk_assert(pParent != NULL);
    dtk_assert(pChild != NULL);
    dtk_assert(pChild->pParent == pParent);

    if (pChild->pNextSibling) {
        pChild->pNextSibling->pPrevSibling = pChild->pPrevSibling;
    }
    if (pChild->pPrevSibling) {
        pChild->pPrevSibling->pNextSibling = pChild->pNextSibling;
    }

    if (pParent->pLastChild == pChild) {
        pParent->pLastChild = pChild->pPrevSibling;
    }
    if (pParent->pFirstChild == pChild) {
        pParent->pFirstChild = NULL;
    }

    pChild->pParent = NULL;
    pChild->pPrevSibling = NULL;
    pChild->pNextSibling = NULL;
}

dtk_result dtk_control_init(dtk_context* pTK, dtk_control* pParent, dtk_control_type type, dtk_event_proc onEvent, dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pControl);

    if (pTK == NULL || pParent == pControl) return DTK_INVALID_ARGS;
    pControl->pTK = pTK;
    pControl->type = type;
    pControl->onEvent = onEvent;

    if (pParent != NULL) {
        dtk_control__link_child(pParent, pControl);
    }

    return DTK_SUCCESS;
}

dtk_result dtk_control_uninit(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    if (pControl->pParent != NULL) {
        dtk_control__unlink_child(pControl->pParent, pControl);
    }

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

    // Don't do anything if it's already visible.
    if (dtk_control_is_visible(pControl)) {
        return DTK_SUCCESS;
    }
    
    // Special case for windows.
    dtk_result result = DTK_SUCCESS;
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        result = dtk_window_show(DTK_WINDOW(pControl), DTK_SHOW_NORMAL);
    } else {
        pControl->flags &= ~DTK_CONTROL_FLAG_HIDDEN;
    }

    return result;
}

dtk_result dtk_control_hide(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // Don't do anything if it's already visible.
    if (!dtk_control_is_visible(pControl)) {
        return DTK_SUCCESS;
    }

    // Special case for windows.
    dtk_result result = DTK_SUCCESS;
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        result = dtk_window_hide(DTK_WINDOW(pControl));
    } else {
        pControl->flags |= DTK_CONTROL_FLAG_HIDDEN;
    }

    return result;
}

dtk_bool32 dtk_control_is_visible(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_FALSE;
    return (pControl->flags & DTK_CONTROL_FLAG_HIDDEN) == 0;
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


dtk_rect dtk_control_get_absolute_rect(dtk_control* pControl)
{
    if (pControl == NULL) return dtk_rect_init(0, 0, 0, 0);
    
    dtk_int32 absolutePosX;
    dtk_int32 absolutePosY;
    dtk_control_get_absolute_position(pControl, &absolutePosX, &absolutePosY);

    dtk_uint32 sizeX;
    dtk_uint32 sizeY;
    dtk_control_get_size(pControl, &sizeX, &sizeY);

    return dtk_rect_init(absolutePosX, absolutePosY, absolutePosX + (dtk_int32)sizeX, absolutePosY + (dtk_int32)sizeY);
}

dtk_rect dtk_control_get_relative_rect(dtk_control* pControl)
{
    if (pControl == NULL) return dtk_rect_init(0, 0, 0, 0);
    
    dtk_int32 relativePosX;
    dtk_int32 relativePosY;
    dtk_control_get_relative_position(pControl, &relativePosX, &relativePosY);

    dtk_uint32 sizeX;
    dtk_uint32 sizeY;
    dtk_control_get_size(pControl, &sizeX, &sizeY);

    return dtk_rect_init(relativePosX, relativePosY, relativePosX + (dtk_int32)sizeX, relativePosY + (dtk_int32)sizeY);
}

dtk_rect dtk_control_get_local_rect(dtk_control* pControl)
{
    if (pControl == NULL) return dtk_rect_init(0, 0, 0, 0);
    return dtk_rect_init(0, 0, pControl->width, pControl->height);
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


dtk_result dtk_control_allow_keyboard_capture(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // If keyboard capture is already allowed just pretend it was successful.
    if ((pControl->flags & DTK_CONTROL_FLAG_FORBID_KEYBOARD_CAPTURE) == 0) {
        return DTK_SUCCESS;
    }

    pControl->flags &= ~DTK_CONTROL_FLAG_FORBID_KEYBOARD_CAPTURE;
    return DTK_SUCCESS;
}

dtk_result dtk_control_forbid_keyboard_capture(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // If keyboard capture is already forbidden just pretend it was successful.
    if ((pControl->flags & DTK_CONTROL_FLAG_FORBID_KEYBOARD_CAPTURE) != 0) {
        return DTK_SUCCESS;
    }

    if (dtk_control_has_keyboard_capture(pControl)) {
        dtk_release_keyboard(pControl->pTK);
    }

    pControl->flags |= DTK_CONTROL_FLAG_FORBID_KEYBOARD_CAPTURE;
    return DTK_SUCCESS;
}

dtk_bool32 dtk_control_is_keyboard_capture_allowed(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_FALSE;

    if ((pControl->flags & DTK_CONTROL_FLAG_FORBID_KEYBOARD_CAPTURE) != 0) {
        return DTK_FALSE;
    }

    if (pControl->pParent != NULL) {
        return dtk_control_is_keyboard_capture_allowed(pControl->pParent);
    }

    return DTK_TRUE;
}


dtk_result dtk_control_capture_keyboard(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;
    return dtk_capture_keyboard(pControl->pTK, pControl);
}

dtk_bool32 dtk_control_has_keyboard_capture(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_FALSE;
    return pControl == pControl->pTK->pControlWithKeyboardCapture;
}


dtk_result dtk_control_capture_mouse(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;
    return dtk_capture_mouse(pControl->pTK, pControl);
}

dtk_bool32 dtk_control_has_mouse_capture(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_FALSE;
    return pControl == pControl->pTK->pControlWithMouseCapture;
}

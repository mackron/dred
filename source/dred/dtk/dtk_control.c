// Copyright (C) 2017 David Reid. See included LICENSE file.

void dtk_control__link_child_append(dtk_control* pParent, dtk_control* pChild)
{
    dtk_assert(pParent != NULL);
    dtk_assert(pChild != NULL);
    dtk_assert(pChild->pParent == NULL);    // <-- The child should not already be attached to a parent.

    pChild->pParent = pParent;

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

void dtk_control__link_child_prepend(dtk_control* pParent, dtk_control* pChild)
{
    dtk_assert(pParent != NULL);
    dtk_assert(pChild != NULL);
    dtk_assert(pChild->pParent == NULL);    // <-- The child should not already be attached to a parent.

    pChild->pParent = pParent;

    if (pParent->pLastChild == NULL) {
        pParent->pLastChild = pChild;
        dtk_assert(pParent->pFirstChild == NULL);
    } else {
        pChild->pNextSibling = pParent->pFirstChild;
        pChild->pPrevSibling = NULL;
        pChild->pNextSibling->pPrevSibling = pChild;
    }

    pParent->pFirstChild = pChild;
}

void dtk_control__link_sibling_append(dtk_control* pControlToAppendTo, dtk_control* pControlToAppend)
{
    dtk_assert(pControlToAppendTo != NULL);
    dtk_assert(pControlToAppend != NULL);
    dtk_assert(pControlToAppend->pParent == NULL);

    pControlToAppend->pParent = pControlToAppendTo->pParent;
    if (pControlToAppend->pParent != NULL) {
        pControlToAppend->pNextSibling = pControlToAppendTo->pNextSibling;
        pControlToAppend->pPrevSibling = pControlToAppendTo;

        pControlToAppendTo->pNextSibling->pPrevSibling = pControlToAppend;
        pControlToAppendTo->pNextSibling = pControlToAppend;

        if (pControlToAppend->pParent->pLastChild == pControlToAppendTo) {
            pControlToAppend->pParent->pLastChild = pControlToAppend;
        }
    }
}

void dtk_control__link_sibling_prepend(dtk_control* pControlToPrependTo, dtk_control* pControlToPrepend)
{
    dtk_assert(pControlToPrependTo != NULL);
    dtk_assert(pControlToPrepend != NULL);
    dtk_assert(pControlToPrepend->pParent == NULL);

    pControlToPrepend->pParent = pControlToPrependTo->pParent;
    if (pControlToPrepend->pParent != NULL) {
        pControlToPrepend->pPrevSibling = pControlToPrependTo->pNextSibling;
        pControlToPrepend->pNextSibling = pControlToPrependTo;

        pControlToPrependTo->pPrevSibling->pNextSibling = pControlToPrepend;
        pControlToPrependTo->pNextSibling = pControlToPrepend;

        if (pControlToPrepend->pParent->pFirstChild == pControlToPrependTo) {
            pControlToPrepend->pParent->pFirstChild = pControlToPrepend;
        }
    }
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


void dtk_control__apply_position_offset_recursive(dtk_control* pControl, dtk_int32 offsetX, dtk_int32 offsetY, dtk_window* pWindow)
{
    if (pWindow == NULL) {
        pWindow = dtk_control_get_window(pControl);
    }

    dtk_rect oldRect = dtk_control_relative_to_absolute_rect(pControl, dtk_control_get_local_rect(pControl));
    pControl->absolutePosX += offsetX;
    pControl->absolutePosY += offsetY;
    dtk_rect newRect = dtk_control_relative_to_absolute_rect(pControl, dtk_control_get_local_rect(pControl));

    dtk_window_scheduled_redraw(pWindow, dtk_rect_union(oldRect, newRect));

    for (dtk_control* pChild = pControl->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling) {
        dtk_control__apply_position_offset_recursive(pChild, offsetX, offsetY, pWindow);
    }
}


dtk_result dtk_control_init(dtk_context* pTK, dtk_control_type type, dtk_event_proc onEvent, dtk_control* pParent, dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pControl);

    if (pTK == NULL || pParent == pControl) return DTK_INVALID_ARGS;
    pControl->pTK = pTK;
    pControl->type = type;
    pControl->onEvent = onEvent;
    pControl->cursor = dtk_system_cursor_type_default;

    if (pParent != NULL) {
        dtk_control__link_child_append(pParent, pControl);
    }

    return DTK_SUCCESS;
}

dtk_result dtk_control_uninit(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    dtk_context* pTK = pControl->pTK;

    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        if (pTK->pWindowUnderMouse == DTK_WINDOW(pControl)) {
            pTK->pWindowUnderMouse = NULL;
            pTK->pControlUnderMouse = NULL;
        }
        if (pTK->pWindowWithKeyboardCapture == DTK_WINDOW(pControl)) {
            pTK->pWindowWithKeyboardCapture = NULL;
            pTK->pControlWithKeyboardCapture = NULL;
        }
        if (pTK->pWindowWithMouseCapture == DTK_WINDOW(pControl)) {
            pTK->pWindowWithMouseCapture = NULL;
            pTK->pControlWithMouseCapture = NULL;
        }
    } else {
        dtk_window* pWindow = dtk_control_get_window(pControl);
        if (pWindow != NULL) {
            if (pTK->pWindowUnderMouse == pWindow) {
                dtk_control* pNewControlUnderMouse = dtk_window_refresh_mouse_enter_leave_state(pWindow, pTK->lastMousePosX, pTK->lastMousePosY);

                dtk_control* pEventReceiver = pTK->pControlWithMouseCapture;
                if (pEventReceiver == NULL) {
                    pEventReceiver = pNewControlUnderMouse;
                }

                if (pEventReceiver != NULL && pEventReceiver != DTK_CONTROL(pWindow)) {
                    // Mouse move.
                    dtk_event e = dtk_event_init(pTK, DTK_EVENT_MOUSE_MOVE, pEventReceiver);
                    e.mouseMove.x = pTK->lastMousePosX;
                    e.mouseMove.y = pTK->lastMousePosY;
                    dtk_control_absolute_to_relative(pEventReceiver, &e.mouseMove.x, &e.mouseMove.y);
                    dtk_handle_local_event(&e);
                }
            }

            if (pWindow->pLastDescendantWithKeyboardFocus == pControl) {
                pWindow->pLastDescendantWithKeyboardFocus = NULL;
            }
            if (pWindow->pLastDescendantWithMouseCapture == pControl) {
                pWindow->pLastDescendantWithMouseCapture = NULL;
            }
        }

        if (pTK->pControlUnderMouse == pControl) {
            pTK->pControlUnderMouse = NULL;
        }
        if (pTK->pControlWithKeyboardCapture == pControl) {
            pTK->pControlWithKeyboardCapture = NULL;
        }
        if (pTK->pControlWithMouseCapture == pControl) {
            pTK->pControlWithMouseCapture = NULL;
        }
    }

    if (pControl->pParent != NULL) {
        dtk_control__unlink_child(pControl->pParent, pControl);
    }


    pControl->isUninitialized = DTK_TRUE;   // <-- Make sure this is set before flushing the event queue.

    // Flush the event queue before returning. The reason this is required is to ensure there are no pending events in the
    // queue that refer to this control. If we don't do this, it's possible for the event to be handled after this control
    // has been uninitialized which probably means a crash.
    dtk_flush_event_queue(pControl->pTK);
    return DTK_SUCCESS;
}

dtk_bool32 dtk_control_default_event_handler(dtk_event* pEvent)
{
    if (pEvent == NULL) return DTK_FALSE;

    switch (pEvent->type)
    {
        case DTK_EVENT_MOUSE_ENTER:
        {
            dtk_window_set_cursor(dtk_control_get_window(pEvent->pControl), pEvent->pControl->cursor);
        } break;
    }

    return DTK_TRUE;
}


dtk_result dtk_control_post_event(dtk_control* pControl, dtk_event* pEvent)
{
    if (pControl == NULL || pEvent == NULL) return DTK_INVALID_ARGS;

    // pEvent->pControl can either be NULL or pControl.
    if (pEvent->pControl != NULL) {
        if (pEvent->pControl != pControl) {
            return DTK_INVALID_ARGS;
        }
    } else {
        pEvent->pControl = pControl;
    }
    
    return dtk_post_local_event(pEvent);
}

dtk_result dtk_control_set_event_handler(dtk_control* pControl, dtk_event_proc onEvent)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    dtk_atomic_exchange_ptr(&pControl->onEvent, onEvent);
    return DTK_SUCCESS;
}

dtk_result dtk_control_set_hit_test_proc(dtk_control* pControl, dtk_control_hit_test_proc onHitTest)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    dtk_atomic_exchange_ptr(&pControl->onHitTest, onHitTest);
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
        pControl->isHidden = DTK_FALSE;
        dtk_control_scheduled_redraw(pControl, dtk_control_get_local_rect(pControl));
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
        pControl->isHidden = DTK_TRUE;
        dtk_control_scheduled_redraw(pControl, dtk_control_get_local_rect(pControl));
    }

    return result;
}

dtk_bool32 dtk_control_is_visible(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_FALSE;
    return pControl->isHidden == DTK_FALSE;
}

dtk_bool32 dtk_control_is_visible_recursive(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_FALSE;

    if (!dtk_control_is_visible(pControl)) {
        return DTK_FALSE;
    }

    if (pControl->pParent) {
        return dtk_control_is_visible_recursive(pControl->pParent);
    }

    return DTK_TRUE;
}


void dtk_control_disable_clipping(dtk_control* pControl)
{
    if (pControl == NULL) return;
    pControl->isClippingDisabled = DTK_TRUE;
    dtk_control_scheduled_redraw(pControl, dtk_control_get_local_rect(pControl));
}

void dtk_control_enable_clipping(dtk_control* pControl)
{
    if (pControl == NULL) return;
    pControl->isClippingDisabled = DTK_FALSE;
    dtk_control_scheduled_redraw(pControl, dtk_control_get_local_rect(pControl));
}

dtk_bool32 dtk_control_is_clipping_enabled(const dtk_control* pControl)
{
    if (pControl == NULL) return DTK_FALSE;
    return !pControl->isClippingDisabled;
}


dtk_result dtk_control_set_size(dtk_control* pControl, dtk_int32 width, dtk_int32 height)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // Special case for windows.
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_set_size(DTK_WINDOW(pControl), width, height);
    } else {
        if (pControl->width != width || pControl->height != height) {
            dtk_rect oldRect = dtk_control_get_local_rect(pControl);
            pControl->width  = width;
            pControl->height = height;
            dtk_rect newRect = dtk_control_get_local_rect(pControl);

            dtk_control_scheduled_redraw(pControl, dtk_rect_union(oldRect, newRect));

            dtk_event e = dtk_event_init(pControl->pTK, DTK_EVENT_SIZE, pControl);
            e.size.width  = pControl->width;
            e.size.height = pControl->height;
            dtk_post_local_event(&e);
        }
    }

    return DTK_SUCCESS;
}

dtk_result dtk_control_get_size(dtk_control* pControl, dtk_int32* pWidth, dtk_int32* pHeight)
{
    if (pWidth)  *pWidth  = 0;
    if (pHeight) *pHeight = 0;
    if (pControl == NULL) return DTK_INVALID_ARGS;

    if (pWidth)  *pWidth  = pControl->width;
    if (pHeight) *pHeight = pControl->height;
    return DTK_SUCCESS;
}

dtk_int32 dtk_control_get_width(dtk_control* pControl)
{
    if (pControl == NULL) return 0;
    return pControl->width;
}

dtk_int32 dtk_control_get_height(dtk_control* pControl)
{
    if (pControl == NULL) return 0;
    return pControl->height;
}

dtk_result dtk_control_set_absolute_position(dtk_control* pControl, dtk_int32 posX, dtk_int32 posY)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // Special case for windows.
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_set_absolute_position(DTK_WINDOW(pControl), posX, posY);
    } else {
        if (pControl->absolutePosX != posX || pControl->absolutePosY != posY) {
            dtk_int32 oldRelativePosX;
            dtk_int32 oldRelativePosY;
            dtk_control_get_relative_position(pControl, &oldRelativePosX, &oldRelativePosY);

            // Controls use absolute coordinates to define their position so we'll need to recursively update the positions
            // of each relatively positioned child control. We do not post move events in this case.
            dtk_int32 offsetX = posX - pControl->absolutePosX;
            dtk_int32 offsetY = posY - pControl->absolutePosY;
            dtk_control__apply_position_offset_recursive(pControl, offsetX, offsetY, NULL);   // <-- This does not post move events, but _will_ post paint events.
            
            // Post an event _only_ for this control (not descendants, since their relative position will not have changed).
            dtk_int32 newRelativePosX;
            dtk_int32 newRelativePosY;
            dtk_control_get_relative_position(pControl, &newRelativePosX, &newRelativePosY);

            if (newRelativePosX != oldRelativePosX || newRelativePosY != oldRelativePosY) {
                dtk_event e = dtk_event_init(pControl->pTK, DTK_EVENT_MOVE, pControl);
                e.move.x = newRelativePosX;
                e.move.y = newRelativePosY;
                dtk_post_local_event(&e);
            }
        }

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

    // Window controls are handled differently.
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_set_relative_position(DTK_WINDOW(pControl), posX, posY);
    }
    
    if (pControl->pParent == NULL) {
        return dtk_control_set_absolute_position(pControl, posX, posY);
    }

    if (pControl->pParent->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_control_set_absolute_position(pControl, posX, posY);
    }

    dtk_control_relative_to_absolute(pControl->pParent, &posX, &posY);
    return dtk_control_set_absolute_position(pControl, posX, posY);
}

dtk_result dtk_control_get_relative_position(dtk_control* pControl, dtk_int32* pRelativePosX, dtk_int32* pRelativePosY)
{
    if (pRelativePosX) *pRelativePosX = 0;  // Safety.
    if (pRelativePosY) *pRelativePosY = 0;  // ^
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // Window controls are handled differently.
    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_get_relative_position(DTK_WINDOW(pControl), pRelativePosX, pRelativePosY);
    }

    dtk_result result = dtk_control_get_absolute_position(pControl, pRelativePosX, pRelativePosY);
    if (result != DTK_SUCCESS) {
        return result;
    }

    if (pControl->pParent != NULL && pControl->pParent->type != DTK_CONTROL_TYPE_WINDOW) {
        return dtk_control_absolute_to_relative(pControl->pParent, pRelativePosX, pRelativePosY);
    }

    return DTK_SUCCESS;
}

dtk_result dtk_control_get_screen_position(dtk_control* pControl, dtk_int32* pPosX, dtk_int32* pPosY)
{
    if (pPosX) *pPosX = 0;  // Safety.
    if (pPosY) *pPosY = 0;  // ^
    if (pControl == NULL) return DTK_INVALID_ARGS;

    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_get_absolute_position(DTK_WINDOW(pControl), pPosX, pPosY);
    }

    dtk_window* pWindow = dtk_control_get_window(pControl);
    if (pWindow == NULL) {
        return dtk_control_get_absolute_position(pControl, pPosX, pPosY);   // <-- The control is not attached to a window, so just fall back to it's absolute position.
    }

    dtk_int32 windowClientScreenPosX;
    dtk_int32 windowClientScreenPosY;
    dtk_result result = dtk_window_get_client_absolute_position(pWindow, &windowClientScreenPosX, &windowClientScreenPosY);
    if (result != DTK_SUCCESS) {
        return result;
    }

    if (pPosX) *pPosX = windowClientScreenPosX + pControl->absolutePosX;
    if (pPosY) *pPosY = windowClientScreenPosY + pControl->absolutePosY;
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

    dtk_int32 sizeX;
    dtk_int32 sizeY;
    dtk_control_get_size(pControl, &sizeX, &sizeY);

    return dtk_rect_init(absolutePosX, absolutePosY, absolutePosX + sizeX, absolutePosY + sizeY);
}

dtk_rect dtk_control_get_relative_rect(dtk_control* pControl)
{
    if (pControl == NULL) return dtk_rect_init(0, 0, 0, 0);
    
    dtk_int32 relativePosX;
    dtk_int32 relativePosY;
    dtk_control_get_relative_position(pControl, &relativePosX, &relativePosY);

    dtk_int32 sizeX;
    dtk_int32 sizeY;
    dtk_control_get_size(pControl, &sizeX, &sizeY);

    return dtk_rect_init(relativePosX, relativePosY, relativePosX + sizeX, relativePosY + sizeY);
}

dtk_rect dtk_control_get_local_rect(dtk_control* pControl)
{
    if (pControl == NULL) return dtk_rect_init(0, 0, 0, 0);
    return dtk_rect_init(0, 0, pControl->width, pControl->height);
}

dtk_rect dtk_control_relative_to_absolute_rect(dtk_control* pControl, dtk_rect relativeRect)
{
    if (pControl == NULL) return dtk_rect_init(0, 0, 0, 0);
    
    dtk_rect rect = relativeRect;
    rect.left   += pControl->absolutePosX;
    rect.top    += pControl->absolutePosY;
    rect.right  += pControl->absolutePosX;
    rect.bottom += pControl->absolutePosY;
    return rect;
}

dtk_rect dtk_control_absolute_to_relative_rect(dtk_control* pControl, dtk_rect absoluteRect)
{
    if (pControl == NULL) return dtk_rect_init(0, 0, 0, 0);
    
    dtk_rect rect = absoluteRect;
    rect.left   -= pControl->absolutePosX;
    rect.top    -= pControl->absolutePosY;
    rect.right  -= pControl->absolutePosX;
    rect.bottom -= pControl->absolutePosY;
    return rect;
}

dtk_bool32 dtk_control_clamp_rect(dtk_control* pControl, dtk_rect* pRelativeRect)
{
    if (pControl == NULL || pRelativeRect == NULL) return DTK_FALSE;

    if (pRelativeRect->left < 0) {
        pRelativeRect->left = 0;
    }
    if (pRelativeRect->top < 0) {
        pRelativeRect->top = 0;
    }

    if (pRelativeRect->right > (dtk_int32)pControl->width) {
        pRelativeRect->right = (dtk_int32)pControl->width;
    }
    if (pRelativeRect->bottom > (dtk_int32)pControl->height) {
        pRelativeRect->bottom = (dtk_int32)pControl->height;
    }

    return (pRelativeRect->right - pRelativeRect->left > 0) && (pRelativeRect->bottom - pRelativeRect->top > 0);
}


dtk_result dtk_control_set_cursor(dtk_control* pControl, dtk_system_cursor_type cursor)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    pControl->cursor = cursor;

    if (dtk_control_is_under_mouse(pControl)) {
        dtk_window_set_cursor(dtk_control_get_window(pControl), cursor);
    }

    return DTK_SUCCESS;
}

dtk_system_cursor_type dtk_control_get_cursor(dtk_control* pControl)
{
    if (pControl == NULL) return dtk_system_cursor_type_default;
    return pControl->cursor;
}

dtk_bool32 dtk_control_is_under_mouse(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_FALSE;

    dtk_window* pWindow = dtk_control_get_window(pControl);
    if (pWindow != pControl->pTK->pWindowUnderMouse) {
        return DTK_FALSE;
    }

    return dtk_window_find_control_under_point(pWindow, pControl->pTK->lastMousePosX, pControl->pTK->lastMousePosY) == pControl;
}


dtk_result dtk_control_detach(dtk_control* pChildControl)
{
    if (pChildControl == NULL) return DTK_INVALID_ARGS;

    dtk_control* pOldParent = pChildControl->pParent;
    if (pOldParent == NULL) {
        return DTK_INVALID_ARGS;
    }

    dtk_rect oldRect = dtk_control_get_relative_rect(pChildControl);
    dtk_control__unlink_child(pOldParent, pChildControl);
    dtk_control_scheduled_redraw(pOldParent, oldRect);

    return DTK_SUCCESS;
}

dtk_result dtk_control_append(dtk_control* pChildControl, dtk_control* pParentControl)
{
    if (pChildControl == NULL || pParentControl == NULL) return DTK_INVALID_ARGS;

    dtk_control_detach(pChildControl);
    dtk_control__link_child_append(pParentControl, pChildControl);
    dtk_control_scheduled_redraw(pChildControl, dtk_control_get_local_rect(pChildControl));

    return DTK_SUCCESS;
}

dtk_result dtk_control_prepend(dtk_control* pChildControl, dtk_control* pParentControl)
{
    if (pChildControl == NULL || pParentControl == NULL) return DTK_INVALID_ARGS;

    dtk_control_detach(pChildControl);
    dtk_control__link_child_prepend(pParentControl, pChildControl);
    dtk_control_scheduled_redraw(pChildControl, dtk_control_get_local_rect(pChildControl));

    return DTK_SUCCESS;
}

dtk_result dtk_control_append_sibling(dtk_control* pControlToAppend, dtk_control* pControlToAppendTo)
{
    if (pControlToAppend == NULL || pControlToAppendTo) return DTK_INVALID_ARGS;

    dtk_control_detach(pControlToAppend);
    dtk_control__link_sibling_append(pControlToAppendTo, pControlToAppend);
    dtk_control_scheduled_redraw(pControlToAppend, dtk_control_get_local_rect(pControlToAppend));

    return DTK_SUCCESS;
}

dtk_result dtk_control_prepend_sibling(dtk_control* pControlToPrepend, dtk_control* pControlToPrependTo)
{
    if (pControlToPrepend == NULL || pControlToPrependTo) return DTK_INVALID_ARGS;

    dtk_control_detach(pControlToPrepend);
    dtk_control__link_sibling_prepend(pControlToPrependTo, pControlToPrepend);
    dtk_control_scheduled_redraw(pControlToPrepend, dtk_control_get_local_rect(pControlToPrepend));

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

dtk_bool32 dtk_control_is_parent(dtk_control* pParentControl, dtk_control* pChildControl)
{
    if (pParentControl == NULL || pChildControl == NULL) return DTK_FALSE;
    return pParentControl == pChildControl->pParent;
}

dtk_bool32 dtk_control_is_child(dtk_control* pChildControl, dtk_control* pParentControl)
{
    if (pParentControl == NULL || pChildControl == NULL) return DTK_FALSE;
    return dtk_control_is_parent(pParentControl, pChildControl);
}

dtk_bool32 dtk_control_is_ancestor(dtk_control* pAncestorControl, dtk_control* pChildControl)
{
    if (pAncestorControl == NULL || pChildControl == NULL) return DTK_FALSE;

    dtk_control* pParent = pChildControl->pParent;
    while (pParent != NULL) {
        if (pParent == pAncestorControl) {
            return DTK_TRUE;
        }

        pParent = pParent->pParent;
    }

    return DTK_FALSE;
}

dtk_bool32 dtk_control_is_descendant(dtk_control* pChildControl, dtk_control* pAncestorControl)
{
    if (pChildControl == NULL || pAncestorControl == NULL) return DTK_FALSE;
    return dtk_control_is_ancestor(pAncestorControl, pChildControl);
}

dtk_bool32 dtk_control_is_self_or_ancestor(dtk_control* pAncestorControl, dtk_control* pChildControl)
{
    if (pAncestorControl == NULL || pChildControl == NULL) return DTK_FALSE;
    return pAncestorControl == pChildControl || dtk_control_is_ancestor(pAncestorControl, pChildControl);
}

dtk_bool32 dtk_control_is_self_or_descendant(dtk_control* pChildControl, dtk_control* pAncestorControl)
{
    if (pChildControl == NULL || pAncestorControl == NULL) return DTK_FALSE;
    return pChildControl == pAncestorControl || dtk_control_is_descendant(pChildControl, pAncestorControl);
}

dtk_control* dtk_control_get_parent(dtk_control* pControl)
{
    if (pControl == NULL) return NULL;
    return pControl->pParent;
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


dtk_bool32 dtk_control_iterate_visible_controls(dtk_control* pControl, dtk_rect relativeRect, dtk_control_visibility_iteration_proc callback, dtk_control_visibility_iteration_proc callbackPost, dtk_uint32 options, void* pUserData)
{
    if (pControl == NULL || callback == NULL) return DTK_FALSE;

    if (!dtk_control_is_visible(pControl)) {
        return DTK_TRUE;
    }

    dtk_bool32 isRootControlVisible = DTK_FALSE;

    dtk_rect clampedRelativeRect = relativeRect;
    if (dtk_control_clamp_rect(pControl, &clampedRelativeRect)) {
        isRootControlVisible = DTK_TRUE;
    }

    if (isRootControlVisible) {
        if (!callback(pControl, &clampedRelativeRect, pUserData)) {
            return DTK_FALSE;
        }
    }

    dtk_bool32 result = DTK_TRUE;
    for (dtk_control* pChild = pControl->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling) {
        if (pChild->type == DTK_CONTROL_TYPE_WINDOW && (options & DTK_CONTROL_ITERATION_SKIP_WINDOWS) != 0) {
            continue;
        }

        dtk_int32 childRelativePosX;
        dtk_int32 childRelativePosY;
        dtk_control_get_relative_position(pChild, &childRelativePosX, &childRelativePosY);

        dtk_rect childRect;
        if (dtk_control_is_clipping_enabled(pChild)) {
            childRect = clampedRelativeRect;
        } else {
            childRect = relativeRect;
        }

        childRect.left   -= childRelativePosX;
        childRect.top    -= childRelativePosY;
        childRect.right  -= childRelativePosX;
        childRect.bottom -= childRelativePosY;
        dtk_bool32 childResult = dtk_control_iterate_visible_controls(pChild, childRect, callback, callbackPost, options, pUserData);
        if (childResult == DTK_FALSE && (options & DTK_CONTROL_ITERATION_ALWAYS_INCLUDE_CHILDREN) == 0) {
            result = DTK_FALSE;
            break;
        }
    }

    if (isRootControlVisible) {
        if (callbackPost) {
            callbackPost(pControl, &clampedRelativeRect, pUserData);
        }
    }

    return result;
}


dtk_result dtk_control_scheduled_redraw(dtk_control* pControl, dtk_rect relativeRect)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // If the rectangle does not have any volume, just pretend the redraw was successful.
    if (!dtk_rect_has_volume(relativeRect)) {
        return DTK_SUCCESS;
    }

    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_scheduled_redraw(DTK_WINDOW(pControl), relativeRect);
    } else {
        return dtk_window_scheduled_redraw(dtk_control_get_window(pControl), dtk_control_relative_to_absolute_rect(pControl, relativeRect));
    }
}

dtk_result dtk_control_immediate_redraw(dtk_control* pControl, dtk_rect relativeRect)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // If the rectangle does not have any volume, just pretend the redraw was successful.
    if (!dtk_rect_has_volume(relativeRect)) {
        return DTK_SUCCESS;
    }

    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_immediate_redraw(DTK_WINDOW(pControl), relativeRect);
    } else {
        return dtk_window_immediate_redraw(dtk_control_get_window(pControl), dtk_control_relative_to_absolute_rect(pControl, relativeRect));
    }
}



dtk_result dtk_control_allow_keyboard_capture(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // If keyboard capture is already allowed just pretend it was successful.
    if (!pControl->isKeyboardCaptureForbidden) {
        return DTK_SUCCESS;
    }

    pControl->isKeyboardCaptureForbidden = DTK_FALSE;
    return DTK_SUCCESS;
}

dtk_result dtk_control_forbid_keyboard_capture(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // If keyboard capture is already forbidden just pretend it was successful.
    if (pControl->isKeyboardCaptureForbidden) {
        return DTK_SUCCESS;
    }

    if (dtk_control_has_keyboard_capture(pControl)) {
        dtk_release_keyboard(pControl->pTK);
    }

    pControl->isKeyboardCaptureForbidden = DTK_TRUE;
    return DTK_SUCCESS;
}

dtk_bool32 dtk_control_is_keyboard_capture_allowed(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_FALSE;

    if (pControl->isKeyboardCaptureForbidden) {
        return DTK_FALSE;
    }

    if (pControl->pParent != NULL) {
        return dtk_control_is_keyboard_capture_allowed(pControl->pParent);
    }

    return DTK_TRUE;
}

dtk_result dtk_control_allow_mouse_capture(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // If mouse capture is already allowed just pretend it was successful.
    if (!pControl->isMouseCaptureForbidden) {
        return DTK_SUCCESS;
    }

    pControl->isMouseCaptureForbidden = DTK_FALSE;
    return DTK_SUCCESS;
}

dtk_result dtk_control_forbid_mouse_capture(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    // If mouse capture is already forbidden just pretend it was successful.
    if (pControl->isMouseCaptureForbidden) {
        return DTK_SUCCESS;
    }

    if (dtk_control_has_mouse_capture(pControl)) {
        dtk_release_mouse(pControl->pTK);
    }

    pControl->isMouseCaptureForbidden = DTK_TRUE;
    return DTK_SUCCESS;
}

dtk_bool32 dtk_control_is_mouse_capture_allowed(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_FALSE;

    if (pControl->isMouseCaptureForbidden) {
        return DTK_FALSE;
    }

    if (pControl->pParent != NULL) {
        return dtk_control_is_mouse_capture_allowed(pControl->pParent);
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


dtk_bool32 dtk_control_is_point_inside_bounds(dtk_control* pControl, dtk_int32 absolutePosX, dtk_int32 absolutePosY)
{
    if (pControl == NULL) return DTK_FALSE;

    if (absolutePosX < pControl->absolutePosX ||
        absolutePosX < pControl->absolutePosY) {
        return DTK_FALSE;
    }

    if (absolutePosX >= pControl->absolutePosX + (dtk_int32)pControl->width ||
        absolutePosY >= pControl->absolutePosY + (dtk_int32)pControl->height) {
        return DTK_FALSE;
    }

    return DTK_TRUE;
}

dtk_bool32 dtk_control_is_point_inside(dtk_control* pControl, dtk_int32 absolutePosX, dtk_int32 absolutePosY)
{
    if (!dtk_control_is_point_inside_bounds(pControl, absolutePosX, absolutePosY)) {
        return DTK_FALSE;
    }

    // It is valid for onHitTest to be null, in which case we use the default hit test which assumes the element is just a rectangle
    // equal to the size of it's bounds. It's equivalent to onHitTest always returning DTK_TRUE.
    if (pControl->onHitTest) {
        return pControl->onHitTest(pControl, absolutePosX - pControl->absolutePosX, absolutePosY - pControl->absolutePosY);
    }

    return DTK_TRUE;
}


float dtk_control_get_scaling_factor(dtk_control* pControl)
{
    if (pControl == NULL) return 1;

    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_get_application_scaling_factor(pControl->pTK) * DTK_WINDOW(pControl)->dpiScale;
    }

    return dtk_control_get_scaling_factor(pControl->pParent);
}

dtk_result dtk_control_refresh_layout(dtk_control* pControl)
{
    if (pControl == NULL) return DTK_INVALID_ARGS;

    dtk_event e = dtk_event_init(pControl->pTK, DTK_EVENT_REFRESH_LAYOUT, pControl);
    return dtk_handle_local_event(&e);
}


dtk_result dtk_control_show_popup_menu(dtk_control* pControl, dtk_menu* pMenu, dtk_int32 relativePosX, dtk_int32 relativePosY)
{
    if (pControl == NULL || pMenu == NULL) return DTK_INVALID_ARGS;

    if (pControl->type == DTK_CONTROL_TYPE_WINDOW) {
        return dtk_window_show_popup_menu(DTK_WINDOW(pControl), pMenu, relativePosX, relativePosY);
    }

    dtk_window* pWindow = dtk_control_get_window(DTK_CONTROL(pControl));
    if (pWindow == NULL) {
        return DTK_INVALID_ARGS;    // The control is not owned by a window.
    }

    dtk_control_relative_to_absolute(pControl, &relativePosX, &relativePosY);
    return dtk_window_show_popup_menu(pWindow, pMenu, relativePosX, relativePosX);
}
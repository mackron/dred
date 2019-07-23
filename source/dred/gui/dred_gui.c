// Copyright (C) 2019 David Reid. See included LICENSE file.

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <float.h>
#include <math.h>

#ifndef DRED_GUI_PRIVATE
#define DRED_GUI_PRIVATE static
#endif

/////////////////////////////////////////////////////////////////
//
// PRIVATE CORE API
//
/////////////////////////////////////////////////////////////////

#define DTK_CONTROL_TYPE_DRED   DTK_CONTROL_TYPE_CUSTOM + 1     // <-- Temporary until dred_control is removed entirely.

DTK_INLINE dtk_rect dtk_rect_init_dred(dred_rect rect)
{
    return dtk_rect_init((dtk_int32)rect.left, (dtk_int32)rect.top, (dtk_int32)rect.right, (dtk_int32)rect.bottom);
}


void dred_control__post_outbound_event_move(dred_control* pControl, float newRelativePosX, float newRelativePosY)
{
    if (pControl->onMove) {
        pControl->onMove(pControl, newRelativePosX, newRelativePosY);
    }
}

void dred_control__post_outbound_event_size(dred_control* pControl, float newWidth, float newHeight)
{
    if (pControl->onSize) {
        pControl->onSize(pControl, newWidth, newHeight);
    }
}

void dred_control__post_outbound_event_mouse_enter(dred_control* pControl)
{
    if (pControl->onMouseEnter) {
        pControl->onMouseEnter(pControl);
    }
}

void dred_control__post_outbound_event_mouse_leave(dred_control* pControl)
{
    if (pControl->onMouseLeave) {
        pControl->onMouseLeave(pControl);
    }
}

void dred_control__post_outbound_event_mouse_move(dred_control* pControl, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    if (pControl->onMouseMove) {
        pControl->onMouseMove(pControl, relativeMousePosX, relativeMousePosY, stateFlags);
    }
}

void dred_control__post_outbound_event_mouse_button_down(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    if (pControl->onMouseButtonDown) {
        pControl->onMouseButtonDown(pControl, mouseButton, relativeMousePosX, relativeMousePosY, stateFlags);
    }
}

void dred_control__post_outbound_event_mouse_button_up(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    if (pControl->onMouseButtonUp) {
        pControl->onMouseButtonUp(pControl, mouseButton, relativeMousePosX, relativeMousePosY, stateFlags);
    }
}

void dred_control__post_outbound_event_mouse_button_dblclick(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    if (pControl->onMouseButtonDblClick) {
        pControl->onMouseButtonDblClick(pControl, mouseButton, relativeMousePosX, relativeMousePosY, stateFlags);
    }
}

void dred_control__post_outbound_event_mouse_wheel(dred_control* pControl, int delta, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    if (pControl->onMouseWheel) {
        pControl->onMouseWheel(pControl, delta, relativeMousePosX, relativeMousePosY, stateFlags);
    }
}

void dred_control__post_outbound_event_key_down(dred_control* pControl, dtk_key key, int stateFlags)
{
    if (pControl->onKeyDown) {
        pControl->onKeyDown(pControl, key, stateFlags);
    }
}

void dred_control__post_outbound_event_key_up(dred_control* pControl, dtk_key key, int stateFlags)
{
    if (pControl->onKeyUp) {
        pControl->onKeyUp(pControl, key, stateFlags);
    }
}

void dred_control__post_outbound_event_printable_key_down(dred_control* pControl, unsigned int character, int stateFlags)
{
    if (pControl->onPrintableKeyDown) {
        pControl->onPrintableKeyDown(pControl, character, stateFlags);
    }
}


void dred_control__post_outbound_event_dirty(dred_control* pControl, dred_rect relativeRect)
{
    if (pControl->onDirty) {
        pControl->onDirty(pControl, relativeRect);
    }
}

void dred_control__post_outbound_event_capture_mouse(dred_control* pControl)
{
    if (pControl->onCaptureMouse) {
        pControl->onCaptureMouse(pControl);
    }
}

void dred_control__post_outbound_event_release_mouse(dred_control* pControl)
{
    if (pControl->onReleaseMouse) {
        pControl->onReleaseMouse(pControl);
    }
}

void dred_control__post_outbound_event_capture_keyboard(dred_control* pControl, dtk_control* pPrevCapturedControl)
{
    if (pControl->onCaptureKeyboard) {
        pControl->onCaptureKeyboard(pControl, pPrevCapturedControl);
    }
}

void dred_control__post_outbound_event_release_keyboard(dred_control* pControl, dtk_control* pNewCapturedControl)
{
    if (pControl->onReleaseKeyboard) {
        pControl->onReleaseKeyboard(pControl, pNewCapturedControl);
    }
}


/////////////////////////////////////////////////////////////////
//
// CORE API
//
/////////////////////////////////////////////////////////////////

dtk_bool32 dred_gui_init(dred_gui* pGUI, dred_context* pDred)
{
    if (pGUI == NULL || pDred == NULL) {
        return DTK_FALSE;
    }

    memset(pGUI, 0, sizeof(*pGUI));
    pGUI->pDred = pDred;

    return DTK_TRUE;
}

void dred_gui_uninit(dred_gui* pGUI)
{
    if (pGUI == NULL) {
        return;
    }
}



/////////////////////////////////////////////////////////////////
// Controls

dtk_bool32 dred_control_event_handler(dtk_event* pEvent)
{
    dtk_control* pControl = pEvent->pControl;
    dred_control* pDredControl = DRED_CONTROL(pControl);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            dred_rect rect = dred_make_rect(
                (float)pEvent->paint.rect.left,
                (float)pEvent->paint.rect.top,
                (float)pEvent->paint.rect.right,
                (float)pEvent->paint.rect.bottom
            );
            
            if (pDredControl->onPaint) {
                pDredControl->onPaint(pDredControl, rect, pEvent->paint.pSurface);
            }
        } break;

        case DTK_EVENT_SIZE:
        {
            dred_control__post_outbound_event_size(pDredControl, (float)pEvent->size.width, (float)pEvent->size.height);
        } break;

        case DTK_EVENT_MOVE:
        {
            dred_control__post_outbound_event_move(pDredControl, (float)pEvent->move.x, (float)pEvent->move.y);
        } break;

        case DTK_EVENT_MOUSE_LEAVE:
        {
            dred_control__post_outbound_event_mouse_leave(pDredControl);
        } break;

        case DTK_EVENT_MOUSE_ENTER:
        {
            dred_control__post_outbound_event_mouse_enter(pDredControl);
        } break;

        case DTK_EVENT_MOUSE_MOVE:
        {
            dred_control__post_outbound_event_mouse_move(pDredControl, pEvent->mouseMove.x, pEvent->mouseMove.y, pEvent->mouseMove.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DOWN:
        {
            dred_control__post_outbound_event_mouse_button_down(pDredControl, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_UP:
        {
            dred_control__post_outbound_event_mouse_button_up(pDredControl, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_BUTTON_DBLCLICK:
        {
            dred_control__post_outbound_event_mouse_button_dblclick(pDredControl, pEvent->mouseButton.button, pEvent->mouseButton.x, pEvent->mouseButton.y, pEvent->mouseButton.state);
        } break;

        case DTK_EVENT_MOUSE_WHEEL:
        {
            dred_control__post_outbound_event_mouse_wheel(pDredControl, pEvent->mouseWheel.delta, pEvent->mouseWheel.x, pEvent->mouseWheel.y, pEvent->mouseWheel.state);
        } break;

        case DTK_EVENT_KEY_DOWN:
        {
            dred_control__post_outbound_event_key_down(pDredControl, pEvent->keyDown.key, pEvent->keyDown.state);
        } break;

        case DTK_EVENT_KEY_UP:
        {
            dred_control__post_outbound_event_key_up(pDredControl, pEvent->keyUp.key, pEvent->keyUp.state);
        } break;

        case DTK_EVENT_PRINTABLE_KEY_DOWN:
        {
            dred_control__post_outbound_event_printable_key_down(pDredControl, pEvent->printableKeyDown.utf32, pEvent->printableKeyDown.state);
        } break;

        case DTK_EVENT_CAPTURE_KEYBOARD:
        {
            dred_control__post_outbound_event_capture_keyboard(pDredControl, pEvent->captureKeyboard.pOldCapturedControl);
        } break;

        case DTK_EVENT_RELEASE_KEYBOARD:
        {
            dred_control__post_outbound_event_release_keyboard(pDredControl, pEvent->releaseKeyboard.pNewCapturedControl);
        } break;

        case DTK_EVENT_CAPTURE_MOUSE:
        {
            dred_control__post_outbound_event_capture_mouse(pDredControl);
        } break;

        case DTK_EVENT_RELEASE_MOUSE:
        {
            dred_control__post_outbound_event_release_mouse(pDredControl);
        } break;

        default: break;
    }

    return dtk_control_default_event_handler(pEvent);
}

dtk_bool32 dred_control_init(dred_control* pControl, dred_context* pDred, dred_control* pParent, dtk_control* pDTKParent, const char* type, dtk_event_proc onEvent)
{
    if (pControl == NULL || pDred == NULL) return DTK_FALSE;
    memset(pControl, 0, sizeof(*pControl));


    if (dtk_control_init(&pDred->tk, DTK_CONTROL_TYPE_DRED, (onEvent != NULL) ? onEvent : dred_control_event_handler, (pParent != NULL) ? &pParent->baseControl : pDTKParent, &pControl->baseControl) != DTK_SUCCESS) {
        return DTK_FALSE;
    }
    
    pControl->pGUI = &pDred->gui;

    dred_control_set_type(pControl, type);
    return DTK_TRUE;
}

void dred_control_uninit(dred_control* pControl)
{
    if (pControl == NULL) {
        return;
    }

    dred_gui* pGUI = pControl->pGUI;
    if (pGUI == NULL) {
        return;
    }

    dtk_control_uninit(&pControl->baseControl);
}


dred_context* dred_control_get_context(dred_control* pControl)
{
    if (pControl == NULL || pControl->pGUI == NULL) {
        return NULL;
    }

    return pControl->pGUI->pDred;
}


dtk_bool32 dred_control_set_type(dred_control* pControl, const char* type)
{
    if (pControl == NULL) {
        return DTK_FALSE;
    }

    return strcpy_s(pControl->type, sizeof(pControl->type), (type == NULL) ? "" : type) == 0;
}

const char* dred_control_get_type(dred_control* pControl)
{
    if (pControl == NULL) {
        return NULL;
    }

    return pControl->type;
}

dtk_bool32 dred_control_is_of_type(dred_control* pControl, const char* type)
{
    if (pControl == NULL || type == NULL) {
        return DTK_FALSE;
    }

    return dred_is_control_type_of_type(pControl->type, type);
}

dtk_bool32 dred_is_control_type_of_type(const char* type, const char* base)
{
    if (type == NULL || base == NULL) {
        return DTK_FALSE;
    }

    return strncmp(type, base, strlen(base)) == 0;
}


void dred_control_hide(dred_control* pControl)
{
    dtk_control_hide(DTK_CONTROL(pControl));
}

void dred_control_show(dred_control* pControl)
{
    dtk_control_show(DTK_CONTROL(pControl));
}

dtk_bool32 dred_control_is_visible(const dred_control* pControl)
{
    return dtk_control_is_visible(DTK_CONTROL(pControl));
}

dtk_bool32 dred_control_is_visible_recursive(const dred_control* pControl)
{
    return dtk_control_is_visible_recursive(DTK_CONTROL(pControl));
}


void dred_control_disable_clipping(dred_control* pControl)
{
    dtk_control_disable_clipping(DTK_CONTROL(pControl));
}

void dred_control_enable_clipping(dred_control* pControl)
{
    dtk_control_enable_clipping(DTK_CONTROL(pControl));
}

dtk_bool32 dred_control_is_clipping_enabled(const dred_control* pControl)
{
    return dtk_control_is_clipping_enabled(DTK_CONTROL(pControl));
}



void dred_gui_capture_mouse(dred_control* pControl)
{
    if (pControl == NULL) return;
    dtk_capture_mouse(&pControl->pGUI->pDred->tk, DTK_CONTROL(pControl));
}

void dred_gui_release_mouse(dred_gui* pGUI)
{
    if (pGUI == NULL) return;
    dtk_release_mouse(&pGUI->pDred->tk);
}

void dred_gui_capture_keyboard(dred_control* pControl)
{
    dtk_capture_keyboard(DTK_CONTROL(pControl)->pTK, DTK_CONTROL(pControl));
}

void dred_gui_release_keyboard(dred_gui* pGUI)
{
    dtk_release_keyboard(&pGUI->pDred->tk);
}

void dred_control_set_cursor(dred_control* pControl, dtk_system_cursor_type cursor)
{
    dtk_control_set_cursor(DTK_CONTROL(pControl), cursor);
}

dtk_system_cursor_type dred_control_get_cursor(dred_control* pControl)
{
    return dtk_control_get_cursor(DTK_CONTROL(pControl));
}

void dred_control_show_popup_menu(dred_control* pControl, dtk_menu* pMenu, int relativePosX, int relativePosY)
{
    dtk_control_show_popup_menu(DTK_CONTROL(pControl), pMenu, relativePosX, relativePosY);
}



//// Events ////

void dred_control_set_on_move(dred_control * pControl, dred_gui_on_move_proc callback)
{
    if (pControl != NULL) {
        pControl->onMove = callback;
    }
}

void dred_control_set_on_size(dred_control * pControl, dred_gui_on_size_proc callback)
{
    if (pControl != NULL) {
        pControl->onSize = callback;
    }
}

void dred_control_set_on_mouse_enter(dred_control* pControl, dred_gui_on_mouse_enter_proc callback)
{
    if (pControl != NULL) {
        pControl->onMouseEnter = callback;
    }
}

void dred_control_set_on_mouse_leave(dred_control* pControl, dred_gui_on_mouse_leave_proc callback)
{
    if (pControl != NULL) {
        pControl->onMouseLeave = callback;
    }
}

void dred_control_set_on_mouse_move(dred_control* pControl, dred_gui_on_mouse_move_proc callback)
{
    if (pControl != NULL) {
        pControl->onMouseMove = callback;
    }
}

void dred_control_set_on_mouse_button_down(dred_control* pControl, dred_gui_on_mouse_button_down_proc callback)
{
    if (pControl != NULL) {
        pControl->onMouseButtonDown = callback;
    }
}

void dred_control_set_on_mouse_button_up(dred_control* pControl, dred_gui_on_mouse_button_up_proc callback)
{
    if (pControl != NULL) {
        pControl->onMouseButtonUp = callback;
    }
}

void dred_control_set_on_mouse_button_dblclick(dred_control* pControl, dred_gui_on_mouse_button_dblclick_proc callback)
{
    if (pControl != NULL) {
        pControl->onMouseButtonDblClick = callback;
    }
}

void dred_control_set_on_mouse_wheel(dred_control* pControl, dred_gui_on_mouse_wheel_proc callback)
{
    if (pControl != NULL) {
        pControl->onMouseWheel = callback;
    }
}

void dred_control_set_on_key_down(dred_control* pControl, dred_gui_on_key_down_proc callback)
{
    if (pControl != NULL) {
        pControl->onKeyDown = callback;
    }
}

void dred_control_set_on_key_up(dred_control* pControl, dred_gui_on_key_up_proc callback)
{
    if (pControl != NULL) {
        pControl->onKeyUp = callback;
    }
}

void dred_control_set_on_printable_key_down(dred_control* pControl, dred_gui_on_printable_key_down_proc callback)
{
    if (pControl != NULL) {
        pControl->onPrintableKeyDown = callback;
    }
}

void dred_control_set_on_paint(dred_control* pControl, dred_gui_on_paint_proc callback)
{
    if (pControl != NULL) {
        pControl->onPaint = callback;
    }
}

void dred_control_set_on_dirty(dred_control * pControl, dred_gui_on_dirty_proc callback)
{
    if (pControl != NULL) {
        pControl->onDirty = callback;
    }
}

void dred_control_set_on_hittest(dred_control* pControl, dred_gui_on_hittest_proc callback)
{
    if (pControl != NULL) {
        pControl->onHitTest = callback;
    }
}

void dred_control_set_on_capture_mouse(dred_control* pControl, dred_gui_on_capture_mouse_proc callback)
{
    if (pControl != NULL) {
        pControl->onCaptureMouse = callback;
    }
}

void dred_control_set_on_release_mouse(dred_control* pControl, dred_gui_on_release_mouse_proc callback)
{
    if (pControl != NULL) {
        pControl->onReleaseMouse = callback;
    }
}

void dred_control_set_on_capture_keyboard(dred_control* pControl, dred_gui_on_capture_keyboard_proc callback)
{
    if (pControl != NULL) {
        pControl->onCaptureKeyboard = callback;
    }
}

void dred_control_set_on_release_keyboard(dred_control* pControl, dred_gui_on_release_keyboard_proc callback)
{
    if (pControl != NULL) {
        pControl->onReleaseKeyboard = callback;
    }
}



dtk_bool32 dred_control_is_point_inside_bounds(const dred_control* pControl, float absolutePosX, float absolutePosY)
{
    return dtk_control_is_point_inside_bounds(DTK_CONTROL(pControl), (dtk_int32)absolutePosX, (dtk_int32)absolutePosY);
}

dtk_bool32 dred_control_is_point_inside(dred_control* pControl, float absolutePosX, float absolutePosY)
{
    return dtk_control_is_point_inside(DTK_CONTROL(pControl), (dtk_int32)absolutePosX, (dtk_int32)absolutePosY);
}

dtk_bool32 dred_control_is_under_mouse(dred_control* pControl)
{
    return dtk_control_is_under_mouse(DTK_CONTROL(pControl));
}


void dred_control_detach(dred_control* pChildControl)
{
    dtk_control_detach(DTK_CONTROL(pChildControl));
}

void dred_control_append(dred_control* pChildControl, dred_control* pParentControl)
{
    dtk_control_append(DTK_CONTROL(pChildControl), DTK_CONTROL(pParentControl));
}

void dred_control_prepend(dred_control* pChildControl, dred_control* pParentControl)
{
    dtk_control_prepend(DTK_CONTROL(pChildControl), DTK_CONTROL(pParentControl));
}

void dred_control_append_sibling(dred_control* pControlToAppend, dred_control* pControlToAppendTo)
{
    dtk_control_append_sibling(DTK_CONTROL(pControlToAppend), DTK_CONTROL(pControlToAppendTo));
}

void dred_control_prepend_sibling(dred_control* pControlToPrepend, dred_control* pControlToPrependTo)
{
    dtk_control_prepend_sibling(DTK_CONTROL(pControlToPrepend), DTK_CONTROL(pControlToPrependTo));
}

dtk_control* dred_control_find_top_level_control(dred_control* pControl)
{
    return dtk_control_find_top_level_control(DTK_CONTROL(pControl));
}

dtk_bool32 dred_control_is_parent(dred_control* pParentControl, dred_control* pChildControl)
{
    return dtk_control_is_parent(DTK_CONTROL(pParentControl), DTK_CONTROL(pChildControl));
}

dtk_bool32 dred_control_is_child(dred_control* pChildControl, dred_control* pParentControl)
{
    return dred_control_is_parent(pParentControl, pChildControl);
}

dtk_bool32 dred_control_is_ancestor(dred_control* pAncestorControl, dred_control* pChildControl)
{
    return dtk_control_is_ancestor(DTK_CONTROL(pAncestorControl), DTK_CONTROL(pChildControl));
}

dtk_bool32 dred_control_is_descendant(dred_control* pChildControl, dred_control* pAncestorControl)
{
    return dred_control_is_ancestor(pAncestorControl, pChildControl);
}

dtk_bool32 dred_control_is_self_or_ancestor(dred_control* pAncestorControl, dred_control* pChildControl)
{
    return pAncestorControl == pChildControl || dred_control_is_ancestor(pAncestorControl, pChildControl);
}

dtk_bool32 dred_control_is_self_or_descendant(dred_control* pChildControl, dred_control* pAncestorControl)
{
    return pChildControl == pAncestorControl || dred_control_is_descendant(pChildControl, pAncestorControl);
}



//// Layout ////

void dred_control_set_absolute_position(dred_control* pControl, float positionX, float positionY)
{
    if (pControl == NULL) return;
    dtk_control_set_absolute_position(DTK_CONTROL(pControl), (dtk_int32)positionX, (dtk_int32)positionY);
}

void dred_control_get_absolute_position(const dred_control* pControl, float * positionXOut, float * positionYOut)
{
    dtk_int32 x;
    dtk_int32 y;
    dtk_control_get_absolute_position(DTK_CONTROL(pControl), &x, &y);

    if (positionXOut) *positionXOut = (float)x;
    if (positionYOut) *positionYOut = (float)y;
}

float dred_control_get_absolute_position_x(const dred_control* pControl)
{
    if (pControl == NULL) return 0.0f;
    return (float)pControl->baseControl.absolutePosX;
}

float dred_control_get_absolute_position_y(const dred_control* pControl)
{
    if (pControl == NULL) return 0.0f;
    return (float)pControl->baseControl.absolutePosY;
}


void dred_control_set_relative_position(dred_control* pControl, float relativePosX, float relativePosY)
{
    if (pControl == NULL) return;
    dtk_control_set_relative_position(DTK_CONTROL(pControl), (dtk_int32)relativePosX, (dtk_int32)relativePosY);
}

void dred_control_get_relative_position(const dred_control* pControl, float* positionXOut, float* positionYOut)
{
    dtk_int32 x;
    dtk_int32 y;
    dtk_control_get_relative_position(DTK_CONTROL(pControl), &x, &y);

    if (positionXOut) *positionXOut = (float)x;
    if (positionYOut) *positionYOut = (float)y;
}

float dred_control_get_relative_position_x(const dred_control* pControl)
{
    float x;
    dred_control_get_relative_position(pControl, &x, NULL);
    return x;
}

float dred_control_get_relative_position_y(const dred_control* pControl)
{
    float y;
    dred_control_get_relative_position(pControl, NULL, &y);
    return y;
}


void dred_control_set_size(dred_control* pControl, float width, float height)
{
    if (pControl == NULL) return;
    dtk_control_set_size(DTK_CONTROL(pControl), (dtk_uint32)width, (dtk_uint32)height);
}

void dred_control_get_size(const dred_control* pControl, float* widthOut, float* heightOut)
{
    dtk_int32 sizeX;
    dtk_int32 sizeY;
    dtk_control_get_size(DTK_CONTROL(pControl), &sizeX, &sizeY);

    if (widthOut)  *widthOut  = (float)sizeX;
    if (heightOut) *heightOut = (float)sizeY;
}

float dred_control_get_width(const dred_control * pControl)
{
    return (float)pControl->baseControl.width;
}

float dred_control_get_height(const dred_control * pControl)
{
    return (float)pControl->baseControl.height;
}


dred_rect dred_control_get_absolute_rect(const dred_control* pControl)
{
    dtk_rect rect = dtk_control_get_absolute_rect(DTK_CONTROL(pControl));
    return dred_make_rect(
        (float)rect.left,
        (float)rect.top,
        (float)rect.right,
        (float)rect.bottom
    );
}

dred_rect dred_control_get_relative_rect(const dred_control* pControl)
{
    dtk_rect rect = dtk_control_get_relative_rect(DTK_CONTROL(pControl));
    return dred_make_rect(
        (float)rect.left,
        (float)rect.top,
        (float)rect.right,
        (float)rect.bottom
    );
}

dred_rect dred_control_get_local_rect(const dred_control* pControl)
{
    dtk_rect rect = dtk_control_get_local_rect(DTK_CONTROL(pControl));
    return dred_make_rect(
        (float)rect.left,
        (float)rect.top,
        (float)rect.right,
        (float)rect.bottom
    );
}



//// Painting ////

dtk_bool32 dred_gui_register_painting_callbacks(dred_gui* pGUI, dred_gui_painting_callbacks callbacks)
{
    if (pGUI == NULL) {
        return DTK_FALSE;
    }

    pGUI->paintingCallbacks = callbacks;

    return DTK_TRUE;
}

void dred_control_dirty(dred_control* pControl, dred_rect relativeRect)
{
    if (pControl == NULL) {
        return;
    }

    dtk_control_scheduled_redraw(DTK_CONTROL(pControl), dtk_rect_init_dred(relativeRect));
}

void dred_control_draw_rect(dred_control* pControl, dred_rect relativeRect, dtk_color color, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    pControl->pGUI->paintingCallbacks.drawRect(relativeRect, color, pSurface);
}



/////////////////////////////////////////////////////////////////
//
// HIGH-LEVEL API
//
/////////////////////////////////////////////////////////////////

//// Hit Testing and Layout ////

void dred_control_on_size_fit_children_to_parent(dred_control* pControl, float newWidth, float newHeight)
{
    for (dtk_control* pChild = pControl->baseControl.pFirstChild; pChild != NULL; pChild = pChild->pNextSibling) {
        dtk_control_set_size(pChild, (dtk_int32)newWidth, (dtk_int32)newHeight);
    }
}

dtk_bool32 dred_control_pass_through_hit_test(dred_control* pControl, float mousePosX, float mousePosY)
{
    (void)pControl;
    (void)mousePosX;
    (void)mousePosY;

    return DTK_FALSE;
}



//// Painting ////

void dred_control_draw_border(dred_control* pControl, float borderWidth, dtk_color color, dtk_surface* pSurface)
{
    dtk_surface_draw_rect_outline(pSurface, dtk_rect_init_dred(dred_control_get_local_rect(pControl)), color, (dtk_int32)borderWidth);
}



/////////////////////////////////////////////////////////////////
//
// UTILITY API
//
/////////////////////////////////////////////////////////////////

dred_rect dred_clamp_rect(dred_rect rect, dred_rect other)
{
    dred_rect result;
    result.left   = (rect.left   >= other.left)   ? rect.left   : other.left;
    result.top    = (rect.top    >= other.top)    ? rect.top    : other.top;
    result.right  = (rect.right  <= other.right)  ? rect.right  : other.right;
    result.bottom = (rect.bottom <= other.bottom) ? rect.bottom : other.bottom;

    return result;
}

dtk_bool32 dred_clamp_rect_to_element(const dred_control* pControl, dred_rect* pRelativeRect)
{
    if (pControl == NULL || pRelativeRect == NULL) {
        return DTK_FALSE;
    }

    dtk_rect rect = dtk_rect_init_dred(*pRelativeRect);
    dtk_bool32 result = dtk_control_clamp_rect(DTK_CONTROL(pControl), &rect);
    
    pRelativeRect->left   = (float)rect.left;
    pRelativeRect->top    = (float)rect.top;
    pRelativeRect->right  = (float)rect.right;
    pRelativeRect->bottom = (float)rect.bottom;

    return result;
}

dred_rect dred_make_rect_relative(const dred_control* pControl, dred_rect* pRect)
{
    if (pControl == NULL || pRect == NULL) {
        return dred_make_rect(0, 0, 0, 0);
    }

    pRect->left   -= pControl->baseControl.absolutePosX;
    pRect->top    -= pControl->baseControl.absolutePosY;
    pRect->right  -= pControl->baseControl.absolutePosX;
    pRect->bottom -= pControl->baseControl.absolutePosY;

    return *pRect;
}

dred_rect dred_make_rect_absolute(const dred_control * pControl, dred_rect * pRect)
{
    if (pControl == NULL || pRect == NULL) {
        return dred_make_rect(0, 0, 0, 0);
    }

    pRect->left   += pControl->baseControl.absolutePosX;
    pRect->top    += pControl->baseControl.absolutePosY;
    pRect->right  += pControl->baseControl.absolutePosX;
    pRect->bottom += pControl->baseControl.absolutePosY;

    return *pRect;
}

void dred_make_point_relative(const dred_control* pControl, float* positionX, float* positionY)
{
    if (pControl != NULL)
    {
        if (positionX != NULL) {
            *positionX -= pControl->baseControl.absolutePosX;
        }

        if (positionY != NULL) {
            *positionY -= pControl->baseControl.absolutePosY;
        }
    }
}

void dred_make_point_absolute(const dred_control* pControl, float* positionX, float* positionY)
{
    if (pControl != NULL)
    {
        if (positionX != NULL) {
            *positionX += pControl->baseControl.absolutePosX;
        }

        if (positionY != NULL) {
            *positionY += pControl->baseControl.absolutePosY;
        }
    }
}

dred_rect dred_make_rect(float left, float top, float right, float bottom)
{
    dred_rect rect;
    rect.left   = left;
    rect.top    = top;
    rect.right  = right;
    rect.bottom = bottom;

    return rect;
}

dred_rect dred_make_inside_out_rect()
{
    dred_rect rect;
    rect.left   =  FLT_MAX;
    rect.top    =  FLT_MAX;
    rect.right  = -FLT_MAX;
    rect.bottom = -FLT_MAX;

    return rect;
}

dred_rect dred_grow_rect(dred_rect rect, float amount)
{
    dred_rect result = rect;
    result.left   -= amount;
    result.top    -= amount;
    result.right  += amount;
    result.bottom += amount;

    return result;
}

dred_rect dred_scale_rect(dred_rect rect, float scaleX, float scaleY)
{
    dred_rect result = rect;
    result.left   *= scaleX;
    result.top    *= scaleY;
    result.right  *= scaleX;
    result.bottom *= scaleY;

    return result;
}

dred_rect dred_offset_rect(dred_rect rect, float offsetX, float offsetY)
{
    return dred_make_rect(rect.left + offsetX, rect.top + offsetY, rect.right + offsetX, rect.bottom + offsetY);
}

dred_rect dred_rect_union(dred_rect rect0, dred_rect rect1)
{
    dred_rect result;
    result.left   = (rect0.left   < rect1.left)   ? rect0.left   : rect1.left;
    result.top    = (rect0.top    < rect1.top)    ? rect0.top    : rect1.top;
    result.right  = (rect0.right  > rect1.right)  ? rect0.right  : rect1.right;
    result.bottom = (rect0.bottom > rect1.bottom) ? rect0.bottom : rect1.bottom;

    return result;
}

dtk_bool32 dred_rect_contains_point(dred_rect rect, float posX, float posY)
{
    if (posX < rect.left || posY < rect.top) {
        return DTK_FALSE;
    }

    if (posX >= rect.right || posY >= rect.bottom) {
        return DTK_FALSE;
    }

    return DTK_TRUE;
}

dtk_bool32 dred_rect_equal(dred_rect rect0, dred_rect rect1)
{
    return
        rect0.left   == rect1.left  &&
        rect0.top    == rect1.top   &&
        rect0.right  == rect1.right &&
        rect0.bottom == rect1.bottom;
}

dtk_bool32 dred_rect_has_volume(dred_rect rect)
{
    return rect.right > rect.left && rect.bottom > rect.top;
}




/////////////////////////////////////////////////////////////////
//
// DTK-SPECIFIC API
//
/////////////////////////////////////////////////////////////////
void dred_control_draw_rect_dtk(dred_rect rect, dtk_color color, dtk_surface* pSurface);

dtk_bool32 dred_gui_init_dtk(dred_gui* pGUI, dred_context* pDred)
{
    if (!dred_gui_init(pGUI, pDred)) {
        return DTK_FALSE;
    }

    dred_gui_register_dtk_callbacks(pGUI);
    return DTK_TRUE;
}

void dred_gui_register_dtk_callbacks(dred_gui* pGUI)
{
    dred_gui_painting_callbacks callbacks;
    callbacks.drawRect = dred_control_draw_rect_dtk;

    dred_gui_register_painting_callbacks(pGUI, callbacks);
}

void dred_control_draw_rect_dtk(dred_rect rect, dtk_color color, dtk_surface* pSurface)
{
    dtk_surface_draw_rect(pSurface, dtk_rect_init_dred(rect), color);
}

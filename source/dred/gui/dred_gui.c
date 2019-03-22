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

void dred_control_get_clip(dred_control* pControl, dred_rect* pRelativeRect, dtk_surface* pSurface)
{
    if (pControl == NULL || pControl->pGUI == NULL) {
        return;
    }

    pControl->pGUI->paintingCallbacks.getClip(pRelativeRect, pSurface);
}

void dred_control_set_clip(dred_control* pControl, dred_rect relativeRect, dtk_surface* pSurface)
{
    if (pControl == NULL || pControl->pGUI == NULL) {
        return;
    }

    // Make sure the rectangle is not negative.
    if (relativeRect.right < relativeRect.left) {
        relativeRect.right = relativeRect.left;
    }

    if (relativeRect.bottom < relativeRect.top) {
        relativeRect.bottom = relativeRect.top;
    }

    pControl->pGUI->paintingCallbacks.setClip(relativeRect, pSurface);
}

void dred_control_draw_rect(dred_control* pControl, dred_rect relativeRect, dtk_color color, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    pControl->pGUI->paintingCallbacks.drawRect(relativeRect, color, pSurface);
}

void dred_control_draw_rect_outline(dred_control* pControl, dred_rect relativeRect, dtk_color color, float outlineWidth, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    pControl->pGUI->paintingCallbacks.drawRectOutline(relativeRect, color, outlineWidth, pSurface);
}

void dred_control_draw_rect_with_outline(dred_control * pControl, dred_rect relativeRect, dtk_color color, float outlineWidth, dtk_color outlineColor, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    pControl->pGUI->paintingCallbacks.drawRectWithOutline(relativeRect, color, outlineWidth, outlineColor, pSurface);
}

void dred_control_draw_round_rect(dred_control* pControl, dred_rect relativeRect, dtk_color color, float radius, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    pControl->pGUI->paintingCallbacks.drawRoundRect(relativeRect, color, radius, pSurface);
}

void dred_control_draw_round_rect_outline(dred_control* pControl, dred_rect relativeRect, dtk_color color, float radius, float outlineWidth, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    pControl->pGUI->paintingCallbacks.drawRoundRectOutline(relativeRect, color, radius, outlineWidth, pSurface);
}

void dred_control_draw_round_rect_with_outline(dred_control* pControl, dred_rect relativeRect, dtk_color color, float radius, float outlineWidth, dtk_color outlineColor, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    pControl->pGUI->paintingCallbacks.drawRoundRectWithOutline(relativeRect, color, radius, outlineWidth, outlineColor, pSurface);
}

void dred_control_draw_text(dred_control* pControl, dtk_font* pFont, float scale, const char* text, int textLengthInBytes, float posX, float posY, dtk_color color, dtk_color backgroundColor, dtk_surface* pSurface)
{
    if (pControl == NULL || pFont == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    pControl->pGUI->paintingCallbacks.drawText(pFont, scale, text, textLengthInBytes, posX, posY, color, backgroundColor, pSurface);
}

void dred_control_draw_image(dred_control* pControl, dred_gui_image* pImage, dred_gui_draw_image_args* pArgs, dtk_surface* pSurface)
{
    if (pControl == NULL || pImage == NULL || pArgs == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    if ((pArgs->options & DRED_GUI_IMAGE_ALIGN_CENTER) != 0)
    {
        pArgs->dstX = pArgs->dstBoundsX + (pArgs->dstBoundsWidth  - pArgs->dstWidth)  / 2;
        pArgs->dstY = pArgs->dstBoundsY + (pArgs->dstBoundsHeight - pArgs->dstHeight) / 2;
    }

    dred_rect prevClip;
    pControl->pGUI->paintingCallbacks.getClip(&prevClip, pSurface);

    dtk_bool32 restoreClip = DTK_FALSE;
    if ((pArgs->options & DRED_GUI_IMAGE_CLIP_BOUNDS) != 0)
    {
        // We only need to clip if part of the destination rectangle falls outside of the bounds.
        if (pArgs->dstX < pArgs->dstBoundsX || pArgs->dstX + pArgs->dstWidth  > pArgs->dstBoundsX + pArgs->dstBoundsWidth ||
            pArgs->dstY < pArgs->dstBoundsY || pArgs->dstY + pArgs->dstHeight > pArgs->dstBoundsY + pArgs->dstBoundsHeight)
        {
            restoreClip = DTK_TRUE;
            pControl->pGUI->paintingCallbacks.setClip(dred_make_rect(pArgs->dstBoundsX, pArgs->dstBoundsY, pArgs->dstBoundsX + pArgs->dstBoundsWidth, pArgs->dstBoundsY + pArgs->dstBoundsHeight), pSurface);
        }
    }

    if ((pArgs->options & DRED_GUI_IMAGE_DRAW_BOUNDS) != 0)
    {
        // The bounds is the area sitting around the outside of the destination rectangle.
        const float boundsLeft   = pArgs->dstBoundsX;
        const float boundsTop    = pArgs->dstBoundsY;
        const float boundsRight  = boundsLeft + pArgs->dstBoundsWidth;
        const float boundsBottom = boundsTop + pArgs->dstBoundsHeight;

        const float imageLeft   = pArgs->dstX;
        const float imageTop    = pArgs->dstY;
        const float imageRight  = imageLeft + pArgs->dstWidth;
        const float imageBottom = imageTop + pArgs->dstHeight;

        // Left.
        if (boundsLeft < imageLeft) {
            pControl->pGUI->paintingCallbacks.drawRect(dred_make_rect(boundsLeft, boundsTop, imageLeft, boundsBottom), pArgs->boundsColor, pSurface);
        }

        // Right.
        if (boundsRight > imageRight) {
            pControl->pGUI->paintingCallbacks.drawRect(dred_make_rect(imageRight, boundsTop, boundsRight, boundsBottom), pArgs->boundsColor, pSurface);
        }

        // Top.
        if (boundsTop < imageTop) {
            pControl->pGUI->paintingCallbacks.drawRect(dred_make_rect(imageLeft, boundsTop, imageRight, imageTop), pArgs->boundsColor, pSurface);
        }

        // Bottom.
        if (boundsBottom > imageBottom) {
            pControl->pGUI->paintingCallbacks.drawRect(dred_make_rect(imageLeft, imageBottom, imageRight, boundsBottom), pArgs->boundsColor, pSurface);
        }
    }

    dtk_draw_image_args argsDTK;
    argsDTK.dstX = (dtk_int32)pArgs->dstX;
    argsDTK.dstY = (dtk_int32)pArgs->dstY;
    argsDTK.dstWidth = (dtk_int32)pArgs->dstWidth;
    argsDTK.dstHeight = (dtk_int32)pArgs->dstHeight;
    argsDTK.srcX = (dtk_int32)pArgs->srcX;
    argsDTK.srcY = (dtk_int32)pArgs->srcY;
    argsDTK.srcWidth = (dtk_int32)pArgs->srcWidth;
    argsDTK.srcHeight = (dtk_int32)pArgs->srcHeight;
    argsDTK.foregroundColor = pArgs->foregroundTint;
    argsDTK.backgroundColor = pArgs->backgroundColor;
    argsDTK.options = pArgs->options;
    pControl->pGUI->paintingCallbacks.drawImage(pImage->pInternalImage, &argsDTK, pSurface);

    if (restoreClip) {
        pControl->pGUI->paintingCallbacks.setClip(prevClip, pSurface);
    }
}


dred_gui_font* dred_gui_create_font(dred_gui* pGUI, const char* family, unsigned int size, dtk_font_weight weight, dtk_font_slant slant, unsigned int flags)
{
    if (pGUI == NULL) {
        return NULL;
    }

    if (pGUI->paintingCallbacks.createFont == NULL) {
        return NULL;
    }


    dtk_font* pInternalFont = pGUI->paintingCallbacks.createFont(&pGUI->pDred->tk, family, size, weight, slant, flags);
    if (pInternalFont == NULL) {
        return NULL;
    }

    dred_gui_font* pFont = (dred_gui_font*)malloc(sizeof(dred_gui_font));
    if (pFont == NULL) {
        return NULL;
    }

    pFont->pGUI     = pGUI;
    pFont->family[0]    = '\0';
    pFont->size         = size;
    pFont->weight       = weight;
    pFont->slant        = slant;
    pFont->flags        = flags;
    pFont->pInternalFont = pInternalFont;

    if (family != NULL) {
        strcpy_s(pFont->family, sizeof(pFont->family), family);
    }

    return pFont;
}

void dred_gui_delete_font(dred_gui_font* pFont)
{
    if (pFont == NULL) {
        return;
    }

    assert(pFont->pGUI != NULL);

    // Delete the internal font objects first.
    if (pFont->pGUI->paintingCallbacks.deleteFont) {
        pFont->pGUI->paintingCallbacks.deleteFont(pFont->pInternalFont);
    }

    free(pFont);
}

dtk_bool32 dred_gui_get_font_metrics(dred_gui_font* pFont, float scale, dtk_font_metrics* pMetricsOut)
{
    if (pFont == NULL || pMetricsOut == NULL) {
        return DTK_FALSE;
    }

    assert(pFont->pGUI != NULL);

    if (pFont->pGUI->paintingCallbacks.getFontMetrics == NULL) {
        return DTK_FALSE;
    }

    return pFont->pGUI->paintingCallbacks.getFontMetrics(pFont->pInternalFont, scale, pMetricsOut);
}

dtk_bool32 dred_gui_get_glyph_metrics(dred_gui_font* pFont, float scale, unsigned int utf32, dtk_glyph_metrics* pMetricsOut)
{
    if (pFont == NULL || pMetricsOut == NULL) {
        return DTK_FALSE;
    }

    assert(pFont->pGUI != NULL);

    if (pFont->pGUI->paintingCallbacks.getGlyphMetrics == NULL) {
        return DTK_FALSE;
    }

    return pFont->pGUI->paintingCallbacks.getGlyphMetrics(pFont->pInternalFont, scale, utf32, pMetricsOut);
}

dtk_bool32 dred_gui_measure_string(dred_gui_font* pFont, float scale, const char* text, size_t textLengthInBytes, dtk_int32* pWidthOut, dtk_int32* pHeightOut)
{
    if (pFont == NULL) {
        return DTK_FALSE;
    }

    if (text == NULL || textLengthInBytes == 0)
    {
        dtk_font_metrics metrics;
        if (!dred_gui_get_font_metrics(pFont, scale, &metrics)) {
            return DTK_FALSE;
        }

        if (pWidthOut) {
            *pWidthOut = 0;
        }
        if (pHeightOut) {
            *pHeightOut = metrics.lineHeight;
        }

        return DTK_TRUE;
    }



    assert(pFont->pGUI != NULL);

    if (pFont->pGUI->paintingCallbacks.measureString == NULL) {
        return DTK_FALSE;
    }

    return pFont->pGUI->paintingCallbacks.measureString(pFont->pInternalFont, scale, text, textLengthInBytes, pWidthOut, pHeightOut);
}

dtk_bool32 dred_gui_get_text_cursor_position_from_point(dred_gui_font* pFont, float scale, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut)
{
    if (pFont == NULL) {
        return DTK_FALSE;
    }

    assert(pFont->pGUI != NULL);

    if (pFont->pGUI->paintingCallbacks.getTextCursorPositionFromPoint) {
        return pFont->pGUI->paintingCallbacks.getTextCursorPositionFromPoint(pFont->pInternalFont, scale, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosXOut, pCharacterIndexOut);
    }

    return DTK_FALSE;
}

dtk_bool32 dred_gui_get_text_cursor_position_from_char(dred_gui_font* pFont, float scale, const char* text, size_t characterIndex, float* pTextCursorPosXOut)
{
    if (pFont == NULL) {
        return DTK_FALSE;
    }

    assert(pFont->pGUI != NULL);

    if (pFont->pGUI->paintingCallbacks.getTextCursorPositionFromChar) {
        return pFont->pGUI->paintingCallbacks.getTextCursorPositionFromChar(pFont->pInternalFont, scale, text, characterIndex, pTextCursorPosXOut);
    }

    return DTK_FALSE;
}



dred_gui_image* dred_gui_create_image(dred_gui* pGUI, unsigned int width, unsigned int height, unsigned int stride, const void* pData)
{
    if (pGUI == NULL) {
        return NULL;
    }

    if (pGUI->paintingCallbacks.createImage == NULL) {
        return NULL;
    }


    // If the stride is 0, assume tightly packed.
    if (stride == 0) {
        stride = width * 4;
    }


    dtk_surface* pInternalImage = pGUI->paintingCallbacks.createImage(&pGUI->pDred->tk, width, height, stride, pData);
    if (pInternalImage == NULL) {
        return NULL;
    }

    dred_gui_image* pImage = (dred_gui_image*)malloc(sizeof(*pImage));
    if (pImage == NULL) {
        return NULL;
    }

    pImage->pGUI  = pGUI;
    pImage->pInternalImage = pInternalImage;


    return pImage;
}

void dred_gui_delete_image(dred_gui_image* pImage)
{
    if (pImage == NULL) {
        return;
    }

    assert(pImage->pGUI != NULL);

    // Delete the internal font object.
    if (pImage->pGUI->paintingCallbacks.deleteImage) {
        pImage->pGUI->paintingCallbacks.deleteImage(pImage->pInternalImage);
    }

    // Free the font object last.
    free(pImage);
}

void dred_gui_get_image_size(dred_gui_image* pImage, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
    if (pWidthOut) *pWidthOut = 0;
    if (pHeightOut) *pHeightOut = 0;

    if (pImage == NULL) {
        return;
    }

    assert(pImage->pGUI != NULL);

    if (pImage->pGUI->paintingCallbacks.getImageSize == NULL) {
        return;
    }

    pImage->pGUI->paintingCallbacks.getImageSize(pImage->pInternalImage, pWidthOut, pHeightOut);
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
    dred_control_draw_rect_outline(pControl, dred_control_get_local_rect(pControl), color, borderWidth, pSurface);
}



/////////////////////////////////////////////////////////////////
//
// UTILITY API
//
/////////////////////////////////////////////////////////////////

dtk_color dred_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    dtk_color color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;

    return color;
}

dtk_color dred_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    dtk_color color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = 255;

    return color;
}

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
void dred_control_set_clip_dtk(dred_rect rect, dtk_surface* pSurface);
void dred_control_get_clip_dtk(dred_rect* pRectOut, dtk_surface* pSurface);
void dred_control_draw_rect_dtk(dred_rect rect, dtk_color color, dtk_surface* pSurface);
void dred_control_draw_rect_outline_dtk(dred_rect, dtk_color, float, dtk_surface* pSurface);
void dred_control_draw_rect_with_outline_dtk(dred_rect, dtk_color, float, dtk_color, dtk_surface* pSurface);
void dred_control_draw_round_rect_dtk(dred_rect, dtk_color, float, dtk_surface* pSurface);
void dred_control_draw_round_rect_outline_dtk(dred_rect, dtk_color, float, float, dtk_surface* pSurface);
void dred_control_draw_round_rect_with_outline_dtk(dred_rect, dtk_color, float, float, dtk_color, dtk_surface* pSurface);
void dred_control_draw_text_dtk(dtk_font*, float scale, const char*, int, float, float, dtk_color, dtk_color, dtk_surface* pSurface);
void dred_control_draw_image_dtk(dtk_surface*, dtk_draw_image_args* pArgs, dtk_surface* pSurface);

dtk_font* dred_gui_create_font_dtk(dtk_context*, const char*, unsigned int, dtk_font_weight, dtk_font_slant, unsigned int flags);
void dred_gui_delete_font_dtk(dtk_font*);
unsigned int dred_gui_get_font_size_dtk(dtk_font*);
dtk_bool32 dred_gui_get_font_metrics_dtk(dtk_font*, float, dtk_font_metrics*);
dtk_bool32 dred_gui_get_glyph_metrics_dtk(dtk_font*, float, unsigned int, dtk_glyph_metrics*);
dtk_bool32 dred_gui_measure_string_dtk(dtk_font*, float, const char*, size_t, dtk_int32*, dtk_int32*);
dtk_bool32 dred_gui_get_text_cursor_position_from_point_dtk(dtk_font*, float, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut);
dtk_bool32 dred_gui_get_text_cursor_position_from_char_dtk(dtk_font*, float, const char* text, size_t characterIndex, float* pTextCursorPosXOut);

dtk_surface* dred_gui_create_image_dtk(dtk_context*, unsigned int width, unsigned int height, unsigned int stride, const void* pImageData);
void dred_gui_delete_image_dtk(dtk_surface*);
void dred_gui_get_image_size_dtk(dtk_surface*, unsigned int* pWidthOut, unsigned int* pHeightOut);

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
    callbacks.setClip                        = dred_control_set_clip_dtk;
    callbacks.getClip                        = dred_control_get_clip_dtk;
	
    callbacks.drawRect                       = dred_control_draw_rect_dtk;
    callbacks.drawRectOutline                = dred_control_draw_rect_outline_dtk;
    callbacks.drawRectWithOutline            = dred_control_draw_rect_with_outline_dtk;
    callbacks.drawRoundRect                  = dred_control_draw_round_rect_dtk;
    callbacks.drawRoundRectOutline           = dred_control_draw_round_rect_outline_dtk;
    callbacks.drawRoundRectWithOutline       = dred_control_draw_round_rect_with_outline_dtk;
    callbacks.drawText                       = dred_control_draw_text_dtk;
    callbacks.drawImage                      = dred_control_draw_image_dtk;

    callbacks.createFont                     = dred_gui_create_font_dtk;
    callbacks.deleteFont                     = dred_gui_delete_font_dtk;
    callbacks.getFontMetrics                 = dred_gui_get_font_metrics_dtk;
    callbacks.getGlyphMetrics                = dred_gui_get_glyph_metrics_dtk;
    callbacks.measureString                  = dred_gui_measure_string_dtk;

    callbacks.createImage                    = dred_gui_create_image_dtk;
    callbacks.deleteImage                    = dred_gui_delete_image_dtk;
    callbacks.getImageSize                   = dred_gui_get_image_size_dtk;

    callbacks.getTextCursorPositionFromPoint = dred_gui_get_text_cursor_position_from_point_dtk;
    callbacks.getTextCursorPositionFromChar  = dred_gui_get_text_cursor_position_from_char_dtk;

    dred_gui_register_painting_callbacks(pGUI, callbacks);
}

void dred_control_set_clip_dtk(dred_rect rect, dtk_surface* pSurface)
{
    dtk_surface_set_clip(pSurface, dtk_rect_init((dtk_int32)rect.left, (dtk_int32)rect.top, (dtk_int32)rect.right, (dtk_int32)rect.bottom));
}

void dred_control_get_clip_dtk(dred_rect* pRectOut, dtk_surface* pSurface)
{
    assert(pRectOut != NULL);

    dtk_rect rect;
    dtk_surface_get_clip(pSurface, &rect);

    pRectOut->left = (float)rect.left;
    pRectOut->top = (float)rect.top;
    pRectOut->right = (float)rect.right;
    pRectOut->bottom = (float)rect.bottom;
}

void dred_control_draw_rect_dtk(dred_rect rect, dtk_color color, dtk_surface* pSurface)
{
    dtk_surface_draw_rect(pSurface, dtk_rect_init_dred(rect), color);
}

void dred_control_draw_rect_outline_dtk(dred_rect rect, dtk_color color, float outlineWidth, dtk_surface* pSurface)
{
    dtk_surface_draw_rect_outline(pSurface, dtk_rect_init_dred(rect), color, (dtk_int32)outlineWidth);
}

void dred_control_draw_rect_with_outline_dtk(dred_rect rect, dtk_color color, float outlineWidth, dtk_color outlineColor, dtk_surface* pSurface)
{
    dtk_surface_draw_rect_with_outline(pSurface, dtk_rect_init_dred(rect), color, (dtk_int32)outlineWidth, outlineColor);
}

void dred_control_draw_round_rect_dtk(dred_rect rect, dtk_color color, float radius, dtk_surface* pSurface)
{
    // NOTE: Rounded rectangles not currently supported.
    (void)radius;
    dtk_surface_draw_rect(pSurface, dtk_rect_init_dred(rect), color);
}

void dred_control_draw_round_rect_outline_dtk(dred_rect rect, dtk_color color, float radius, float outlineWidth, dtk_surface* pSurface)
{
    // NOTE: Rounded rectangles not currently supported.
    (void)radius;
    dtk_surface_draw_rect_outline(pSurface, dtk_rect_init_dred(rect), color, (dtk_int32)outlineWidth);
}

void dred_control_draw_round_rect_with_outline_dtk(dred_rect rect, dtk_color color, float radius, float outlineWidth, dtk_color outlineColor, dtk_surface* pSurface)
{
    // NOTE: Rounded rectangles not currently supported.
    (void)radius;
    dtk_surface_draw_rect_with_outline(pSurface, dtk_rect_init_dred(rect), color, (dtk_int32)outlineWidth, outlineColor);
}

void dred_control_draw_text_dtk(dtk_font* pFont, float scale, const char* text, int textSizeInBytes, float posX, float posY, dtk_color color, dtk_color backgroundColor, dtk_surface* pSurface)
{
    dtk_surface_draw_text(pSurface, pFont, scale, text, textSizeInBytes, (dtk_int32)posX, (dtk_int32)posY, color, backgroundColor);
}

void dred_control_draw_image_dtk(dtk_surface* pImage, dtk_draw_image_args* pArgs, dtk_surface* pSurface)
{
    dtk_surface_draw_surface(pSurface, pImage, pArgs);
}


dtk_font* dred_gui_create_font_dtk(dtk_context* pTK, const char* family, unsigned int size, dtk_font_weight weight, dtk_font_slant slant, unsigned int flags)
{
    dtk_font* pFont = (dtk_font*)malloc(sizeof(*pFont));
    if (pFont == NULL) {
        return NULL;
    }

    if (dtk_font_init(pTK, family, (float)size, weight, slant, flags, pFont) != DTK_SUCCESS) {
        free(pFont);
        return NULL;
    }

    return pFont;
}

void dred_gui_delete_font_dtk(dtk_font* pFont)
{
    dtk_font_uninit(pFont);
    free(pFont);
}

unsigned int dred_gui_get_font_size_dtk(dtk_font* pFont)
{
    return (unsigned int)pFont->size;
}

dtk_bool32 dred_gui_get_font_metrics_dtk(dtk_font* pFont, float scale, dtk_font_metrics* pMetricsOut)
{
    assert(pMetricsOut != NULL);
    return dtk_font_get_metrics(pFont, scale, pMetricsOut) == DTK_SUCCESS;
}

dtk_bool32 dred_gui_get_glyph_metrics_dtk(dtk_font* pFont, float scale, unsigned int utf32, dtk_glyph_metrics* pMetricsOut)
{
    assert(pMetricsOut != NULL);
    return dtk_font_get_glyph_metrics(pFont, scale, utf32, pMetricsOut) == DTK_SUCCESS;
}

dtk_bool32 dred_gui_measure_string_dtk(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, dtk_int32* pWidthOut, dtk_int32* pHeightOut)
{
    return dtk_font_measure_string(pFont, scale, text, textSizeInBytes, pWidthOut, pHeightOut) == DTK_SUCCESS;
}

dtk_bool32 dred_gui_get_text_cursor_position_from_point_dtk(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut)
{
    return dtk_font_get_text_cursor_position_from_point(pFont, scale, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosXOut, pCharacterIndexOut) == DTK_SUCCESS;
}

dtk_bool32 dred_gui_get_text_cursor_position_from_char_dtk(dtk_font* pFont, float scale, const char* text, size_t characterIndex, float* pTextCursorPosXOut)
{
    return dtk_font_get_text_cursor_position_from_char(pFont, scale, text, characterIndex, pTextCursorPosXOut) == DTK_SUCCESS;
}


dtk_surface* dred_gui_create_image_dtk(dtk_context* pTK, unsigned int width, unsigned int height, unsigned int stride, const void* pImageData)
{
    dtk_surface* pSurface = (dtk_surface*)malloc(sizeof(*pSurface));
    if (pSurface == NULL) {
        return NULL;
    }

    if (dtk_surface_init_image(pTK, width, height, stride, pImageData, pSurface) != DTK_SUCCESS) {
        free(pSurface);
        return NULL;
    }

    return pSurface;
}

void dred_gui_delete_image_dtk(dtk_surface* pImage)
{
    dtk_surface_uninit(pImage);
    free(pImage);
}

void dred_gui_get_image_size_dtk(dtk_surface* pImage, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
    if (pWidthOut) *pWidthOut = pImage->width;
    if (pHeightOut) *pHeightOut = pImage->height;
}

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

void dred_control__post_outbound_event_move(dred_control* pControl, int newRelativePosX, int newRelativePosY)
{
    if (pControl->onMove) {
        pControl->onMove(pControl, newRelativePosX, newRelativePosY);
    }
}

void dred_control__post_outbound_event_size(dred_control* pControl, int newWidth, int newHeight)
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


void dred_control__post_outbound_event_dirty(dred_control* pControl, dtk_rect relativeRect)
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
// Controls

dtk_bool32 dred_control_event_handler(dtk_event* pEvent)
{
    dtk_control* pControl = pEvent->pControl;
    dred_control* pDredControl = DRED_CONTROL(pControl);

    switch (pEvent->type)
    {
        case DTK_EVENT_PAINT:
        {
            if (pDredControl->onPaint) {
                pDredControl->onPaint(pDredControl, pEvent->paint.rect, pEvent->paint.pSurface);
            }
        } break;

        case DTK_EVENT_SIZE:
        {
            dred_control__post_outbound_event_size(pDredControl, pEvent->size.width, pEvent->size.height);
        } break;

        case DTK_EVENT_MOVE:
        {
            dred_control__post_outbound_event_move(pDredControl, pEvent->move.x, pEvent->move.y);
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

dtk_bool32 dred_control_init(dred_control* pControl, dred_context* pDred, dtk_control* pParent, const char* type, dtk_event_proc onEvent)
{
    if (pControl == NULL || pDred == NULL) return DTK_FALSE;
    memset(pControl, 0, sizeof(*pControl));


    if (dtk_control_init(&pDred->tk, DTK_CONTROL_TYPE_DRED, (onEvent != NULL) ? onEvent : dred_control_event_handler, pParent, &pControl->baseControl) != DTK_SUCCESS) {
        return DTK_FALSE;
    }

    dred_control_set_type(pControl, type);
    return DTK_TRUE;
}

void dred_control_uninit(dred_control* pControl)
{
    if (pControl == NULL) {
        return;
    }

    dtk_control_uninit(&pControl->baseControl);
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

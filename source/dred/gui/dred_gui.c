// Copyright (C) 2016 David Reid. See included LICENSE file.

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

// Context Flags
#define IS_AUTO_DIRTY_DISABLED              (1U << 1)
#define IS_RELEASING_KEYBOARD               (1U << 2)

// Control Flags
#define IS_CONTROL_HIDDEN                   (1U << 0)
#define IS_CONTROL_CLIPPING_DISABLED        (1U << 1)


DTK_INLINE dtk_rect dtk_rect_init_dred(dred_rect rect)
{
    return dtk_rect_init((dtk_int32)rect.left, (dtk_int32)rect.top, (dtk_int32)rect.right, (dtk_int32)rect.bottom);
}

DTK_INLINE dtk_color dtk_color_init_dred(dtk_color color)
{
    return dtk_rgba(color.r, color.g, color.b, color.a);
}


/// Orphans the given element without triggering a redraw of the parent nor the child.
void dred_control__detach_without_redraw(dred_control* pChildControl);

/// Appends the given element without first detaching it from the old parent, nor does it post a redraw.
void dred_control__append_without_detach_or_redraw(dred_control* pChildControl, dred_control* pParentControl);

/// Appends the given element without first detaching it from the old parent.
void dred_control__append_without_detach(dred_control* pChildControl, dred_control* pParentControl);

/// Prepends the given element without first detaching it from the old parent, nor does it post a redraw.
void dred_control__prepend_without_detach_or_redraw(dred_control* pChildControl, dred_control* pParentControl);

/// Prepends the given element without first detaching it from the old parent.
void dred_control__prepend_without_detach(dred_control* pChildControl, dred_control* pParentControl);

/// Appends an element to another as it's sibling, but does not detach it from the previous parent nor trigger a redraw.
void dred_control__append_sibling_without_detach_or_redraw(dred_control* pControlToAppend, dred_control* pControlToAppendTo);

/// Appends an element to another as it's sibling, but does not detach it from the previous parent.
void dred_control__append_sibling_without_detach(dred_control* pControlToAppend, dred_control* pControlToAppendTo);

/// Prepends an element to another as it's sibling, but does not detach it from the previous parent nor trigger a redraw.
void dred_control__prepend_sibling_without_detach_or_redraw(dred_control* pControlToPrepend, dred_control* pControlToPrependTo);

/// Prepends an element to another as it's sibling, but does not detach it from the previous parent.
void dred_control__prepend_sibling_without_detach(dred_control* pControlToPrepend, dred_control* pControlToPrependTo);


/// Begins accumulating an invalidation rectangle.
//void dred_control__begin_auto_dirty(dred_control* pControl);

/// Ends accumulating the invalidation rectangle and posts on_dirty is auto-dirty is enabled.
//void dred_control__end_auto_dirty(dred_control* pControl);

/// Marks the given region of the given top level element as dirty, but only if automatic dirtying is enabled.
///
/// @remarks
///     This is equivalent to dred_control__begin_auto_dirty() immediately followed by dred_control__end_auto_dirty().
//void dred_control__auto_dirty(dred_control* pTopLevelControl, dred_rect rect);


/// Recursively applies the given offset to the absolute positions of the children of the given element.
///
/// @remarks
///     This is called when the absolute position of an element is changed.
//void dred_control__apply_offset_to_children_recursive(dred_control* pParentControl, float offsetX, float offsetY);


/// The function to call when the mouse may have entered into a new element.
//void dred_gui__update_mouse_enter_and_leave_state(dred_gui* pGUI, dred_control* pNewControlUnderMouse);


/// Functions for posting outbound events.
void dred_control__post_outbound_event_move(dred_control* pControl, float newRelativePosX, float newRelativePosY);
void dred_control__post_outbound_event_size(dred_control* pControl, float newWidth, float newHeight);
void dred_control__post_outbound_event_mouse_enter(dred_control* pControl);
void dred_control__post_outbound_event_mouse_leave(dred_control* pControl);
void dred_control__post_outbound_event_mouse_move(dred_control* pControl, int relativeMousePosX, int relativeMousePosY, int stateFlags);
void dred_control__post_outbound_event_mouse_button_down(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);
void dred_control__post_outbound_event_mouse_button_up(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);
void dred_control__post_outbound_event_mouse_button_dblclick(dred_control* pControl, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags);
void dred_control__post_outbound_event_mouse_wheel(dred_control* pControl, int delta, int relativeMousePosX, int relativeMousePosY, int stateFlags);
void dred_control__post_outbound_event_key_down(dred_control* pControl, dtk_key key, int stateFlags);
void dred_control__post_outbound_event_key_up(dred_control* pControl, dtk_key key, int stateFlags);
void dred_control__post_outbound_event_printable_key_down(dred_control* pControl, unsigned int character, int stateFlags);
void dred_control__post_outbound_event_dirty(dred_control* pControl, dred_rect relativeRect);
void dred_control__post_outbound_event_dirty_global(dred_control* pControl, dred_rect relativeRect);
void dred_control__post_outbound_event_capture_mouse(dred_control* pControl);
void dred_control__post_outbound_event_capture_mouse_global(dred_control* pControl);
void dred_control__post_outbound_event_release_mouse(dred_control* pControl);
void dred_control__post_outbound_event_release_mouse_global(dred_control* pControl);
void dred_control__post_outbound_event_capture_keyboard(dred_control* pControl, dred_control* pPrevCapturedControl);
void dred_control__post_outbound_event_capture_keyboard_global(dred_control* pControl, dred_control* pPrevCapturedControl);
void dred_control__post_outbound_event_release_keyboard(dred_control* pControl, dred_control* pNewCapturedControl);
void dred_control__post_outbound_event_release_keyboard_global(dred_control* pControl, dred_control* pNewCapturedControl);

/// Posts a log message.
void dred_gui__log(dred_gui* pGUI, const char* message);

#if 0
void dred_control__detach_without_redraw(dred_control* pControl)
{
    if (pControl->pParent != NULL) {
        if (pControl->pParent->pFirstChild == pControl) {
            pControl->pParent->pFirstChild = pControl->pNextSibling;
        }

        if (pControl->pParent->pLastChild == pControl) {
            pControl->pParent->pLastChild = pControl->pPrevSibling;
        }


        if (pControl->pPrevSibling != NULL) {
            pControl->pPrevSibling->pNextSibling = pControl->pNextSibling;
        }

        if (pControl->pNextSibling != NULL) {
            pControl->pNextSibling->pPrevSibling = pControl->pPrevSibling;
        }
    }

    pControl->pParent      = NULL;
    pControl->pPrevSibling = NULL;
    pControl->pNextSibling = NULL;
}

void dred_control__append_without_detach_or_redraw(dred_control* pChildControl, dred_control* pParentControl)
{
    pChildControl->pParent = pParentControl;
    if (pChildControl->pParent != NULL) {
        if (pChildControl->pParent->pLastChild != NULL) {
            pChildControl->pPrevSibling = pChildControl->pParent->pLastChild;
            pChildControl->pPrevSibling->pNextSibling = pChildControl;
        }

        if (pChildControl->pParent->pFirstChild == NULL) {
            pChildControl->pParent->pFirstChild = pChildControl;
        }

        pChildControl->pParent->pLastChild = pChildControl;
    }
}

void dred_control__append_without_detach(dred_control* pChildControl, dred_control* pParentControl)
{
    dred_control__append_without_detach_or_redraw(pChildControl, pParentControl);
    dred_control__auto_dirty(pChildControl, dred_make_rect(0, 0, pChildControl->width, pChildControl->height));
}

void dred_control__prepend_without_detach_or_redraw(dred_control* pChildControl, dred_control* pParentControl)
{
    pChildControl->pParent = pParentControl;
    if (pChildControl->pParent != NULL) {
        if (pChildControl->pParent->pFirstChild != NULL) {
            pChildControl->pNextSibling = pChildControl->pParent->pFirstChild;
            pChildControl->pNextSibling->pPrevSibling = pChildControl;
        }

        if (pChildControl->pParent->pLastChild == NULL) {
            pChildControl->pParent->pLastChild = pChildControl;
        }

        pChildControl->pParent->pFirstChild = pChildControl;
    }
}

void dred_control__prepend_without_detach(dred_control* pChildControl, dred_control* pParentControl)
{
    dred_control__prepend_without_detach_or_redraw(pChildControl, pParentControl);
    dred_control__auto_dirty(pChildControl, dred_make_rect(0, 0, pChildControl->width, pChildControl->height));
}

void dred_control__append_sibling_without_detach_or_redraw(dred_control* pControlToAppend, dred_control* pControlToAppendTo)
{
    assert(pControlToAppend   != NULL);
    assert(pControlToAppendTo != NULL);

    pControlToAppend->pParent = pControlToAppendTo->pParent;
    if (pControlToAppend->pParent != NULL)
    {
        pControlToAppend->pNextSibling = pControlToAppendTo->pNextSibling;
        pControlToAppend->pPrevSibling = pControlToAppendTo;

        pControlToAppendTo->pNextSibling->pPrevSibling = pControlToAppend;
        pControlToAppendTo->pNextSibling = pControlToAppend;

        if (pControlToAppend->pParent->pLastChild == pControlToAppendTo) {
            pControlToAppend->pParent->pLastChild = pControlToAppend;
        }
    }
}

void dred_control__append_sibling_without_detach(dred_control* pControlToAppend, dred_control* pControlToAppendTo)
{
    dred_control__append_sibling_without_detach_or_redraw(pControlToAppend, pControlToAppendTo);
    dred_control__auto_dirty(pControlToAppend, dred_make_rect(0, 0, pControlToAppend->width, pControlToAppend->height));
}

void dred_control__prepend_sibling_without_detach_or_redraw(dred_control* pControlToPrepend, dred_control* pControlToPrependTo)
{
    assert(pControlToPrepend   != NULL);
    assert(pControlToPrependTo != NULL);

    pControlToPrepend->pParent = pControlToPrependTo->pParent;
    if (pControlToPrepend->pParent != NULL)
    {
        pControlToPrepend->pPrevSibling = pControlToPrependTo->pNextSibling;
        pControlToPrepend->pNextSibling = pControlToPrependTo;

        pControlToPrependTo->pPrevSibling->pNextSibling = pControlToPrepend;
        pControlToPrependTo->pNextSibling = pControlToPrepend;

        if (pControlToPrepend->pParent->pFirstChild == pControlToPrependTo) {
            pControlToPrepend->pParent->pFirstChild = pControlToPrepend;
        }
    }
}

void dred_control__prepend_sibling_without_detach(dred_control* pControlToPrepend, dred_control* pControlToPrependTo)
{
    dred_control__prepend_sibling_without_detach_or_redraw(pControlToPrepend, pControlToPrependTo);
    dred_control__auto_dirty(pControlToPrepend, dred_make_rect(0, 0, pControlToPrepend->width, pControlToPrepend->height));
}
#endif

#if 0
void dred_control__begin_auto_dirty(dred_control* pControl)
{
    assert(pControl           != NULL);
    assert(pControl->pGUI != NULL);

    if (dred_gui_is_auto_dirty_enabled(pControl->pGUI)) {
        dred_control_begin_dirty(pControl);
    }
}

void dred_control__end_auto_dirty(dred_control* pControl)
{
    assert(pControl != NULL);

    dred_gui* pGUI = pControl->pGUI;
    assert(pGUI != NULL);

    if (dred_gui_is_auto_dirty_enabled(pGUI)) {
        dred_control_end_dirty(pControl);
    }
}

void dred_control__auto_dirty(dred_control* pControl, dred_rect relativeRect)
{
    assert(pControl != NULL);
    assert(pControl->pGUI != NULL);

    if (dred_gui_is_auto_dirty_enabled(pControl->pGUI)) {
        dred_control_dirty(pControl, relativeRect);
    }
}
#endif


//void dred_gui__change_cursor(dred_control* pControl, dtk_system_cursor_type cursor)
//{
//    if (pControl == NULL || pControl->pGUI == NULL) {
//        return;
//    }
//
//    pControl->pGUI->currentCursor = cursor;
//
//    if (pControl->pGUI->onChangeCursor) {
//        pControl->pGUI->onChangeCursor(pControl, cursor);
//    }
//}


#if 0
void dred_control__apply_offset_to_children_recursive(dred_control* pParentControl, float offsetX, float offsetY)
{
    assert(pParentControl != NULL);

    for (dred_control* pChild = pParentControl->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling)
    {
        dred_control__begin_auto_dirty(pParentControl);
        {
            dred_control__auto_dirty(pParentControl, dred_control_get_local_rect(pParentControl));
            pChild->absolutePosX += offsetX;
            pChild->absolutePosY += offsetY;

            dred_control__apply_offset_to_children_recursive(pChild, offsetX, offsetY);
        }
        dred_control__end_auto_dirty(pParentControl);
    }
}
#endif

#if 0
DRED_GUI_PRIVATE void dred_gui__post_on_mouse_leave_recursive(dred_gui* pGUI, dred_control* pNewControlUnderMouse, dred_control* pOldControlUnderMouse)
{
    (void)pGUI;

    dred_control* pOldAncestor = pOldControlUnderMouse;
    while (pOldAncestor != NULL)
    {
        dr_bool32 isOldControlUnderMouse = pNewControlUnderMouse == pOldAncestor || dred_control_is_ancestor(pOldAncestor, pNewControlUnderMouse);
        if (!isOldControlUnderMouse)
        {
            dred_control__post_outbound_event_mouse_leave(pOldAncestor);
        }

        pOldAncestor = pOldAncestor->pParent;
    }
}

DRED_GUI_PRIVATE void dred_gui__post_on_mouse_enter_recursive(dred_gui* pGUI, dred_control* pNewControlUnderMouse, dred_control* pOldControlUnderMouse)
{
    if (pNewControlUnderMouse == NULL) {
        return;
    }


    if (pNewControlUnderMouse->pParent != NULL) {
        dred_gui__post_on_mouse_enter_recursive(pGUI, pNewControlUnderMouse->pParent, pOldControlUnderMouse);
    }

    dr_bool32 wasNewControlUnderMouse = pOldControlUnderMouse == pNewControlUnderMouse || dred_control_is_ancestor(pNewControlUnderMouse, pOldControlUnderMouse);
    if (!wasNewControlUnderMouse)
    {
        dred_control__post_outbound_event_mouse_enter(pNewControlUnderMouse);
    }
}
#endif

//void dred_gui__update_mouse_enter_and_leave_state(dred_gui* pGUI, dred_control* pNewControlUnderMouse)
//{
//    if (pGUI == NULL) {
//        return;
//    }
//
//    dred_control* pOldControlUnderMouse = pGUI->pControlUnderMouse;
//    if (pOldControlUnderMouse != pNewControlUnderMouse)
//    {
//        // We don't change the enter and leave state if an element is capturing the mouse.
//        if (pGUI->pControlWithMouseCapture == NULL)
//        {
//            pGUI->pControlUnderMouse = pNewControlUnderMouse;
//
//            dtk_system_cursor_type newCursor = dtk_system_cursor_type_default;
//            if (pNewControlUnderMouse != NULL) {
//                newCursor = pNewControlUnderMouse->cursor;
//            }
//
//
//            // It's intuitive to check that the new cursor is different to the old one before trying to change it, but that is not actually
//            // what we want to do. We'll let the event handler manage it themselves because it's possible the window manager might do some
//            // window-specific cursor management and the old and new elements are on different windows.
//            dred_gui__change_cursor(pNewControlUnderMouse, newCursor);
//
//
//
//            // The the event handlers below, remember that ancestors are considered hovered if a descendant is the element under the mouse.
//
//            // on_mouse_leave
//            dred_gui__post_on_mouse_leave_recursive(pGUI, pNewControlUnderMouse, pOldControlUnderMouse);
//
//            // on_mouse_enter
//            dred_gui__post_on_mouse_enter_recursive(pGUI, pNewControlUnderMouse, pOldControlUnderMouse);
//        }
//    }
//}


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

//void dred_control__post_outbound_event_dirty_global(dred_control* pControl, dred_rect relativeRect)
//{
//    if (pControl->pGUI->onGlobalDirty) {
//        pControl->pGUI->onGlobalDirty(pControl, relativeRect);
//    }
//}

void dred_control__post_outbound_event_capture_mouse(dred_control* pControl)
{
    if (pControl->onCaptureMouse) {
        pControl->onCaptureMouse(pControl);
    }
}

//void dred_control__post_outbound_event_capture_mouse_global(dred_control* pControl)
//{
//    if (pControl->pGUI->onGlobalCaptureMouse) {
//        pControl->pGUI->onGlobalCaptureMouse(pControl);
//    }
//}

void dred_control__post_outbound_event_release_mouse(dred_control* pControl)
{
    if (pControl->onReleaseMouse) {
        pControl->onReleaseMouse(pControl);
    }
}

//void dred_control__post_outbound_event_release_mouse_global(dred_control* pControl)
//{
//    if (pControl->pGUI->onGlobalReleaseMouse) {
//        pControl->pGUI->onGlobalReleaseMouse(pControl);
//    }
//}


void dred_control__post_outbound_event_capture_keyboard(dred_control* pControl, dred_control* pPrevCapturedControl)
{
    if (pControl->onCaptureKeyboard) {
        pControl->onCaptureKeyboard(pControl, pPrevCapturedControl);
    }
}

//void dred_control__post_outbound_event_capture_keyboard_global(dred_control* pControl, dred_control* pPrevCapturedControl)
//{
//    if (pControl->pGUI->onGlobalCaptureKeyboard) {
//        pControl->pGUI->onGlobalCaptureKeyboard(pControl, pPrevCapturedControl);
//    }
//}

void dred_control__post_outbound_event_release_keyboard(dred_control* pControl, dred_control* pNewCapturedControl)
{
    if (pControl->onReleaseKeyboard) {
        pControl->onReleaseKeyboard(pControl, pNewCapturedControl);
    }
}

//void dred_control__post_outbound_event_release_keyboard_global(dred_control* pControl, dred_control* pNewCapturedControl)
//{
//    if (pControl->pGUI->onGlobalReleaseKeyboard) {
//        pControl->pGUI->onGlobalReleaseKeyboard(pControl, pNewCapturedControl);
//    }
//}


void dred_gui__log(dred_gui* pGUI, const char* message)
{
    if (pGUI->onLog) {
        pGUI->onLog(pGUI, message);
    }
}


/////////////////////////////////////////////////////////////////
//
// CORE API
//
/////////////////////////////////////////////////////////////////

dr_bool32 dred_gui_init(dred_gui* pGUI, dred_context* pDred)
{
    if (pGUI == NULL || pDred == NULL) {
        return DR_FALSE;
    }

    memset(pGUI, 0, sizeof(*pGUI));
    pGUI->pDred = pDred;
    //pGUI->currentCursor = dtk_system_cursor_type_default;

    return DR_TRUE;
}

void dred_gui_uninit(dred_gui* pGUI)
{
    if (pGUI == NULL) {
        return;
    }


    // Make sure the mouse capture is released.
    //if (pGUI->pControlWithMouseCapture != NULL)
    //{
    //    dred_gui__log(pGUI, "WARNING: Deleting the GUI context while an element still has the mouse capture.");
    //    dred_gui_release_mouse(pGUI);
    //}

    //// Make sure the keyboard capture is released.
    //if (pGUI->pControlWithKeyboardCapture != NULL)
    //{
    //    dred_gui__log(pGUI, "WARNING: Deleting the GUI context while an element still has the keyboard capture.");
    //    dred_gui_release_keyboard(pGUI);
    //}


    //if (pGUI->ppDirtyControls != NULL) {
    //    free(pGUI->ppDirtyControls);
    //}
}



/////////////////////////////////////////////////////////////////
// Events

//void dred_gui_post_inbound_event_mouse_leave(dred_control* pTopLevelControl)
//{
//    if (pTopLevelControl == NULL) {
//        return;
//    }
//
//    dred_gui* pGUI = pTopLevelControl->pGUI;
//    if (pGUI == NULL) {
//        return;
//    }
//
//    // We assume that the control that was previously under the mouse was either pTopLevelControl itself or one of it's descendants.
//    dred_gui__update_mouse_enter_and_leave_state(pGUI, NULL);
//}
//
//void dred_gui_post_inbound_event_mouse_move(dred_control* pTopLevelControl, int mousePosX, int mousePosY, int stateFlags)
//{
//    if (pTopLevelControl == NULL || pTopLevelControl->pGUI == NULL) {
//        return;
//    }
//
//
//    // A pointer to the top level element that was passed in from the last inbound mouse move event.
//    pTopLevelControl->pGUI->pLastMouseMoveTopLevelControl = pTopLevelControl;
//
//    // The position of the mouse that was passed in from the last inbound mouse move event.
//    pTopLevelControl->pGUI->lastMouseMovePosX = (float)mousePosX;
//    pTopLevelControl->pGUI->lastMouseMovePosY = (float)mousePosY;
//
//
//
//    // The first thing we need to do is find the new element that's sitting under the mouse.
//    dred_control* pNewControlUnderMouse = dred_gui_find_control_under_point(pTopLevelControl, (float)mousePosX, (float)mousePosY);
//
//    // Now that we know which element is sitting under the mouse we need to check if the mouse has entered into a new element.
//    dred_gui__update_mouse_enter_and_leave_state(pTopLevelControl->pGUI, pNewControlUnderMouse);
//
//
//    dred_control* pEventReceiver = pTopLevelControl->pGUI->pControlWithMouseCapture;
//    if (pEventReceiver == NULL)
//    {
//        pEventReceiver = pNewControlUnderMouse;
//    }
//
//    if (pEventReceiver != NULL)
//    {
//        float relativeMousePosX = (float)mousePosX;
//        float relativeMousePosY = (float)mousePosY;
//        dred_make_point_relative(pEventReceiver, &relativeMousePosX, &relativeMousePosY);
//
//        dred_control__post_outbound_event_mouse_move(pEventReceiver, (int)relativeMousePosX, (int)relativeMousePosY, stateFlags);
//    }
//}
//
//void dred_gui_post_inbound_event_mouse_button_down(dred_control* pTopLevelControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
//{
//    if (pTopLevelControl == NULL || pTopLevelControl->pGUI == NULL) {
//        return;
//    }
//
//    dred_gui* pGUI = pTopLevelControl->pGUI;
//
//    dred_control* pEventReceiver = pGUI->pControlWithMouseCapture;
//    if (pEventReceiver == NULL) {
//        pEventReceiver = pGUI->pControlUnderMouse;
//        if (pEventReceiver == NULL) {
//            // We'll get here if this message is posted without a prior mouse move event.
//            pEventReceiver = dred_gui_find_control_under_point(pTopLevelControl, (float)mousePosX, (float)mousePosY);
//        }
//    }
//
//    if (pEventReceiver != NULL) {
//        float relativeMousePosX = (float)mousePosX;
//        float relativeMousePosY = (float)mousePosY;
//        dred_make_point_relative(pEventReceiver, &relativeMousePosX, &relativeMousePosY);
//
//        dred_control__post_outbound_event_mouse_button_down(pEventReceiver, mouseButton, (int)relativeMousePosX, (int)relativeMousePosY, stateFlags);
//    }
//}
//
//void dred_gui_post_inbound_event_mouse_button_up(dred_control* pTopLevelControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
//{
//    if (pTopLevelControl == NULL || pTopLevelControl->pGUI == NULL) {
//        return;
//    }
//
//    dred_gui* pGUI = pTopLevelControl->pGUI;
//
//    dred_control* pEventReceiver = pGUI->pControlWithMouseCapture;
//    if (pEventReceiver == NULL) {
//        pEventReceiver = pGUI->pControlUnderMouse;
//        if (pEventReceiver == NULL) {
//            // We'll get here if this message is posted without a prior mouse move event.
//            pEventReceiver = dred_gui_find_control_under_point(pTopLevelControl, (float)mousePosX, (float)mousePosY);
//        }
//    }
//
//    if (pEventReceiver != NULL) {
//        float relativeMousePosX = (float)mousePosX;
//        float relativeMousePosY = (float)mousePosY;
//        dred_make_point_relative(pEventReceiver, &relativeMousePosX, &relativeMousePosY);
//
//        dred_control__post_outbound_event_mouse_button_up(pEventReceiver, mouseButton, (int)relativeMousePosX, (int)relativeMousePosY, stateFlags);
//    }
//}
//
//void dred_gui_post_inbound_event_mouse_button_dblclick(dred_control* pTopLevelControl, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
//{
//    if (pTopLevelControl == NULL || pTopLevelControl->pGUI == NULL) {
//        return;
//    }
//
//    dred_gui* pGUI = pTopLevelControl->pGUI;
//
//    dred_control* pEventReceiver = pGUI->pControlWithMouseCapture;
//    if (pEventReceiver == NULL) {
//        pEventReceiver = pGUI->pControlUnderMouse;
//        if (pEventReceiver == NULL) {
//            // We'll get here if this message is posted without a prior mouse move event.
//            pEventReceiver = dred_gui_find_control_under_point(pTopLevelControl, (float)mousePosX, (float)mousePosY);
//        }
//    }
//
//    if (pEventReceiver != NULL) {
//        float relativeMousePosX = (float)mousePosX;
//        float relativeMousePosY = (float)mousePosY;
//        dred_make_point_relative(pEventReceiver, &relativeMousePosX, &relativeMousePosY);
//
//        dred_control__post_outbound_event_mouse_button_dblclick(pEventReceiver, mouseButton, (int)relativeMousePosX, (int)relativeMousePosY, stateFlags);
//    }
//}
//
//void dred_gui_post_inbound_event_mouse_wheel(dred_control* pTopLevelControl, int delta, int mousePosX, int mousePosY, int stateFlags)
//{
//    if (pTopLevelControl == NULL || pTopLevelControl->pGUI == NULL) {
//        return;
//    }
//
//    dred_gui* pGUI = pTopLevelControl->pGUI;
//
//    dred_control* pEventReceiver = pGUI->pControlWithMouseCapture;
//    if (pEventReceiver == NULL) {
//        pEventReceiver = pGUI->pControlUnderMouse;
//        if (pEventReceiver == NULL) {
//            // We'll get here if this message is posted without a prior mouse move event.
//            pEventReceiver = dred_gui_find_control_under_point(pTopLevelControl, (float)mousePosX, (float)mousePosY);
//        }
//    }
//
//    if (pEventReceiver != NULL) {
//        float relativeMousePosX = (float)mousePosX;
//        float relativeMousePosY = (float)mousePosY;
//        dred_make_point_relative(pEventReceiver, &relativeMousePosX, &relativeMousePosY);
//
//        dred_control__post_outbound_event_mouse_wheel(pEventReceiver, delta, (int)relativeMousePosX, (int)relativeMousePosY, stateFlags);
//    }
//}
//
//void dred_gui_post_inbound_event_key_down(dred_gui* pGUI, dtk_key key, int stateFlags)
//{
//    if (pGUI == NULL) {
//        return;
//    }
//
//    if (pGUI->pControlWithKeyboardCapture != NULL) {
//        dred_control__post_outbound_event_key_down(pGUI->pControlWithKeyboardCapture, key, stateFlags);
//    }
//}
//
//void dred_gui_post_inbound_event_key_up(dred_gui* pGUI, dtk_key key, int stateFlags)
//{
//    if (pGUI == NULL) {
//        return;
//    }
//
//    if (pGUI->pControlWithKeyboardCapture != NULL) {
//        dred_control__post_outbound_event_key_up(pGUI->pControlWithKeyboardCapture, key, stateFlags);
//    }
//}
//
//void dred_gui_post_inbound_event_printable_key_down(dred_gui* pGUI, unsigned int character, int stateFlags)
//{
//    if (pGUI == NULL) {
//        return;
//    }
//
//    if (pGUI->pControlWithKeyboardCapture != NULL) {
//        dred_control__post_outbound_event_printable_key_down(pGUI->pControlWithKeyboardCapture, character, stateFlags);
//    }
//}



//void dred_gui_set_global_on_dirty(dred_gui * pGUI, dred_gui_on_dirty_proc onDirty)
//{
//    if (pGUI != NULL) {
//        pGUI->onGlobalDirty = onDirty;
//    }
//}

//void dred_gui_set_global_on_capture_mouse(dred_gui* pGUI, dred_gui_on_capture_mouse_proc onCaptureMouse)
//{
//    if (pGUI != NULL) {
//        pGUI->onGlobalCaptureMouse = onCaptureMouse;
//    }
//}

//void dred_gui_set_global_on_release_mouse(dred_gui* pGUI, dred_gui_on_release_mouse_proc onReleaseMouse)
//{
//    if (pGUI != NULL) {
//        pGUI->onGlobalReleaseMouse = onReleaseMouse;
//    }
//}

//void dred_gui_set_global_on_capture_keyboard(dred_gui* pGUI, dred_gui_on_capture_keyboard_proc onCaptureKeyboard)
//{
//    if (pGUI != NULL) {
//        pGUI->onGlobalCaptureKeyboard = onCaptureKeyboard;
//    }
//}
//
//void dred_gui_set_global_on_release_keyboard(dred_gui* pGUI, dred_gui_on_capture_keyboard_proc onReleaseKeyboard)
//{
//    if (pGUI != NULL) {
//        pGUI->onGlobalReleaseKeyboard = onReleaseKeyboard;
//    }
//}

//void dred_gui_set_global_on_change_cursor(dred_gui* pGUI, dred_gui_on_change_cursor_proc onChangeCursor)
//{
//    if (pGUI != NULL) {
//        pGUI->onChangeCursor = onChangeCursor;
//    }
//}

void dred_gui_set_on_delete_element(dred_gui* pGUI, dred_gui_on_delete_element_proc onDeleteControl)
{
    if (pGUI != NULL) {
        pGUI->onDeleteControl = onDeleteControl;
    }
}

void dred_gui_set_on_log(dred_gui* pGUI, dred_gui_on_log onLog)
{
    if (pGUI != NULL) {
        pGUI->onLog = onLog;
    }
}



/////////////////////////////////////////////////////////////////
// Controls

dtk_bool32 dred_control_event_handler(dtk_event* pEvent)
{
    dtk_control* pControl = pEvent->pControl;
    dred_control* pDredControl = (dred_control*)pControl->pUserData;

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

            //dred_control__post_outbound_event_paint(pDredControl, rect);
        } break;

        case DTK_EVENT_SIZE:
        {
            dred_control__post_outbound_event_size(pDredControl, (float)pEvent->size.width, (float)pEvent->size.height);
        } break;

        case DTK_EVENT_MOVE:
        {
            dred_control__post_outbound_event_move(pDredControl, (float)pEvent->move.x, (float)pEvent->move.y);
        } break;

        /*case DTK_EVENT_SHOW:
        {
            dred_control__post_outbound_event_show(pDredControl);
        } break;

        case DTK_EVENT_HIDE:
        {
            dred_control__post_outbound_event_hide(pDredControl);
        } break;*/

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
            dred_control__post_outbound_event_key_down(pDredControl, pEvent->keyUp.key, pEvent->keyUp.state);
        } break;

        case DTK_EVENT_PRINTABLE_KEY_DOWN:
        {
            dred_control__post_outbound_event_printable_key_down(pDredControl, pEvent->printableKeyDown.utf32, pEvent->printableKeyDown.state);
        } break;

        case DTK_EVENT_CAPTURE_KEYBOARD:
        {
            dred_window* pWindow = (dred_window*)dtk_control_get_window(pControl);
            pWindow->pControlWithKeyboardCapture = pControl;
            dred_control__post_outbound_event_capture_keyboard(pDredControl, NULL);
        } break;

        case DTK_EVENT_RELEASE_KEYBOARD:
        {
            dred_control__post_outbound_event_release_keyboard(pDredControl, NULL);
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

dr_bool32 dred_control_init(dred_control* pControl, dred_context* pDred, dred_control* pParent, dtk_control* pDTKParent, const char* type)
{
    if (pControl == NULL || pDred == NULL) return DR_FALSE;
    memset(pControl, 0, sizeof(*pControl));


    if (dtk_control_init(&pDred->tk, (pParent != NULL) ? &pParent->baseControl : pDTKParent, DTK_CONTROL_TYPE_DRED, dred_control_event_handler, &pControl->baseControl) != DTK_SUCCESS) {
        return DTK_FALSE;
    }
    pControl->baseControl.pUserData = pControl;

    
    pControl->pGUI = pDred->pGUI;
    //pControl->pParent = pParent;
    //pControl->cursor = dtk_system_cursor_type_default;
    //pControl->dirtyRect = dred_make_inside_out_rect();

    // Add to the the hierarchy.
    //dred_control__append_without_detach_or_redraw(pControl, pControl->pParent);


    // Have the element positioned at 0,0 relative to the parent by default.
    /*if (pParent != NULL) {
        pControl->absolutePosX = pParent->absolutePosX;
        pControl->absolutePosY = pParent->absolutePosY;
    }*/

    dred_control_set_type(pControl, type);
    return DR_TRUE;
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


    // Notify the application that the element is being deleted. Do this at the top so the event handler can access things like the hierarchy and
    // whatnot in case it needs it.
    if (pGUI->onDeleteControl) {
        pGUI->onDeleteControl(pControl);
    }

#if 0
    // If this was element is marked as the one that was last under the mouse it needs to be unset.
    dr_bool32 needsMouseUpdate = DR_FALSE;
    if (pGUI->pControlUnderMouse == pControl) {
        pGUI->pControlUnderMouse = NULL;
        needsMouseUpdate = DR_TRUE;
    }

    if (pGUI->pLastMouseMoveTopLevelControl == pControl) {
        pGUI->pLastMouseMoveTopLevelControl = NULL;
        pGUI->lastMouseMovePosX = 0;
        pGUI->lastMouseMovePosY = 0;
        needsMouseUpdate = DR_FALSE;       // It was a top-level element so the mouse enter/leave state doesn't need an update.
    }


    // If this element has the mouse capture it needs to be released.
    if (pGUI->pControlWithMouseCapture == pControl) {
        dred_gui__log(pGUI, "WARNING: Deleting an element while it still has the mouse capture.");
        dred_gui_release_mouse_no_local_notify(pGUI);
    }

    // If this element has the keyboard capture it needs to be released.
    if (pGUI->pControlWithKeyboardCapture == pControl) {
        dred_gui__log(pGUI, "WARNING: Deleting an element while it still has the keyboard capture.");
        dred_gui_release_keyboard_no_local_notify(pGUI);
    }

    // Is this element in the middle of being marked as dirty?
    for (size_t iDirtyControl = 0; iDirtyControl < pGUI->dirtyControlCount; ++iDirtyControl) {
        if (pGUI->ppDirtyControls[iDirtyControl] == pControl) {
            dred_gui__log(pGUI, "WARNING: Deleting an element while it is being marked as dirty.");
            for (size_t iDirtyControl2 = iDirtyControl; iDirtyControl2+1 < pGUI->dirtyControlCount; ++iDirtyControl2) {
                pGUI->ppDirtyControls[iDirtyControl2] = pGUI->ppDirtyControls[iDirtyControl2+1];
            }

            pGUI->dirtyControlCount -= 1;
            break;
        }
    }



    // Deleting this element may have resulted in the mouse entering a new element. Here is where we do a mouse enter/leave update.
    if (needsMouseUpdate) {
        pControl->onHitTest = dred_control_pass_through_hit_test;        // <-- This ensures we don't include this element when searching for the new element under the mouse.
        dred_gui__update_mouse_enter_and_leave_state(pGUI, dred_gui_find_control_under_point(pGUI->pLastMouseMoveTopLevelControl, pGUI->lastMouseMovePosX, pGUI->lastMouseMovePosY));
    }


    // Here is where we need to detach the element from the hierarchy. When doing this we want to ensure the element is not redrawn when
    // it's children are detached. To do this we simply detach the event handler.
    pControl->onPaint = NULL;

    // The parent needs to be redraw after detaching.
    dred_control* pParent = pControl->pParent;
    dred_rect relativeRect = dred_control_get_relative_rect(pControl);


    // Orphan the element first.
    dred_control__detach_without_redraw(pControl);

    // Children need to be deleted before deleting the element itself.
    while (pControl->pLastChild != NULL) {
        dred_control_uninit(pControl->pLastChild);
    }


    // The parent needs to be redrawn.
    if (pParent) {
        dred_control_dirty(pParent, relativeRect);
    }
#endif

    dtk_control_uninit(&pControl->baseControl);
}


dred_context* dred_control_get_context(dred_control* pControl)
{
    if (pControl == NULL || pControl->pGUI == NULL) {
        return NULL;
    }

    return pControl->pGUI->pDred;
}

dred_gui* dred_control_get_gui(dred_control* pControl)
{
    dred_context* pDred = dred_control_get_context(pControl);
    if (pDred == NULL) {
        return NULL;
    }

    return pDred->pGUI;
}


dr_bool32 dred_control_set_type(dred_control* pControl, const char* type)
{
    if (pControl == NULL) {
        return DR_FALSE;
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

dr_bool32 dred_control_is_of_type(dred_control* pControl, const char* type)
{
    if (pControl == NULL || type == NULL) {
        return DR_FALSE;
    }

    return dred_is_control_type_of_type(pControl->type, type);
}

dr_bool32 dred_is_control_type_of_type(const char* type, const char* base)
{
    if (type == NULL || base == NULL) {
        return DR_FALSE;
    }

    return strncmp(type, base, strlen(base)) == 0;
}


void dred_control_hide(dred_control* pControl)
{
#if 0
    if (pControl != NULL) {
        pControl->flags |= IS_CONTROL_HIDDEN;
        dred_control__auto_dirty(pControl, dred_control_get_local_rect(pControl));
    }
#endif

    dtk_control_hide(DTK_CONTROL(pControl));
}

void dred_control_show(dred_control* pControl)
{
#if 0
    if (pControl != NULL) {
        pControl->flags &= ~IS_CONTROL_HIDDEN;
        dred_control__auto_dirty(pControl, dred_control_get_local_rect(pControl));
    }
#endif

    dtk_control_show(DTK_CONTROL(pControl));
}

dr_bool32 dred_control_is_visible(const dred_control* pControl)
{
#if 0
    if (pControl != NULL) {
        return (pControl->flags & IS_CONTROL_HIDDEN) == 0;
    }

    return DR_FALSE;
#endif

    return dtk_control_is_visible(DTK_CONTROL(pControl));
}

dr_bool32 dred_control_is_visible_recursive(const dred_control* pControl)
{
#if 0
    if (dred_control_is_visible(pControl))
    {
        assert(pControl->pParent != NULL);

        if (pControl->pParent != NULL) {
            return dred_control_is_visible(pControl->pParent);
        }
    }

    return DR_FALSE;
#endif

    return dtk_control_is_visible_recursive(DTK_CONTROL(pControl));
}


void dred_control_disable_clipping(dred_control* pControl)
{
#if 0
    if (pControl != NULL) {
        pControl->flags |= IS_CONTROL_CLIPPING_DISABLED;
    }
#endif

    dtk_control_disable_clipping(DTK_CONTROL(pControl));
}

void dred_control_enable_clipping(dred_control* pControl)
{
#if 0
    if (pControl != NULL) {
        pControl->flags &= ~IS_CONTROL_CLIPPING_DISABLED;
    }
#endif

    dtk_control_enable_clipping(DTK_CONTROL(pControl));
}

dr_bool32 dred_control_is_clipping_enabled(const dred_control* pControl)
{
#if 0
    if (pControl != NULL) {
        return (pControl->flags & IS_CONTROL_CLIPPING_DISABLED) == 0;
    }

    return DR_TRUE;
#endif

    return dtk_control_is_clipping_enabled(DTK_CONTROL(pControl));
}



void dred_gui_capture_mouse(dred_control* pControl)
{
    if (pControl == NULL) return;
    dtk_capture_mouse(&pControl->pGUI->pDred->tk, DTK_CONTROL(pControl));

#if 0
    if (pControl == NULL) {
        return;
    }

    if (pControl->pGUI == NULL) {
        return;
    }


    if (pControl->pGUI->pControlWithMouseCapture != pControl)
    {
        // Release the previous capture first.
        if (pControl->pGUI->pControlWithMouseCapture != NULL) {
            dred_gui_release_mouse(pControl->pGUI);
        }

        assert(pControl->pGUI->pControlWithMouseCapture == NULL);

        pControl->pGUI->pControlWithMouseCapture = pControl;

        // Two events need to be posted - the global on_capture_mouse event and the local on_capture_mouse event.
        dred_control__post_outbound_event_capture_mouse(pControl);

        if (pControl == pControl->pGUI->pControlWithMouseCapture) {  // <-- Only post the global event handler if the element still has the capture.
            dred_control__post_outbound_event_capture_mouse_global(pControl);
        }
    }
#endif
}

void dred_gui_release_mouse(dred_gui* pGUI)
{
    if (pGUI == NULL) return;
    dtk_release_mouse(&pGUI->pDred->tk);

#if 0
    if (pGUI == NULL || pGUI->pControlWithMouseCapture == NULL) {
        return;
    }


    // Events need to be posted before setting the internal pointer.
    dred_control__post_outbound_event_release_mouse(pGUI->pControlWithMouseCapture);
    dred_control__post_outbound_event_release_mouse_global(pGUI->pControlWithMouseCapture);

    // We want to set the internal pointer to NULL after posting the events since that is when it has truly released the mouse.
    pGUI->pControlWithMouseCapture = NULL;


    // After releasing the mouse the cursor may be sitting on top of a different element - we want to recheck that.
    dred_gui__update_mouse_enter_and_leave_state(pGUI, dred_gui_find_control_under_point(pGUI->pLastMouseMoveTopLevelControl, pGUI->lastMouseMovePosX, pGUI->lastMouseMovePosY));
#endif
}

#if 0
void dred_gui_release_mouse_no_local_notify(dred_gui* pGUI)
{
    if (pGUI == NULL) {
        return;
    }

    dred_control* pControl = pGUI->pControlWithMouseCapture;
    if (pControl == NULL) {
        return;
    }

    dred_gui_on_release_mouse_proc prevProc = pControl->onReleaseMouse;
    pControl->onReleaseMouse = NULL;
    dred_gui_release_mouse(pGUI);
    pControl->onReleaseMouse = prevProc;
}

void dred_gui_release_mouse_no_global_notify(dred_gui* pGUI)
{
    if (pGUI == NULL) {
        return;
    }

    dred_gui_on_release_mouse_proc prevProc = pGUI->onGlobalReleaseMouse;
    pGUI->onGlobalReleaseMouse = NULL;
    dred_gui_release_mouse(pGUI);
    pGUI->onGlobalReleaseMouse = prevProc;
}
#endif

//dred_control* dred_gui_get_element_with_mouse_capture(dred_gui* pGUI)
//{
//    if (pGUI == NULL) {
//        return NULL;
//    }
//
//    return pGUI->pControlWithMouseCapture;
//}
//
//dr_bool32 dred_control_has_mouse_capture(dred_control* pControl)
//{
//    if (pControl == NULL) {
//        return DR_FALSE;
//    }
//
//    return dred_gui_get_element_with_mouse_capture(pControl->pGUI) == pControl;
//}


//DRED_GUI_PRIVATE void dred_gui_release_keyboard_private(dred_gui* pGUI, dred_control* pNewCapturedControl)
//{
//    assert(pGUI != NULL);
//
//    // It is reasonable to expect that an application will want to change keyboard focus from within the release_keyboard
//    // event handler. The problem with this is that is can cause a infinite dependency chain. We need to handle that case
//    // by setting a flag that keeps track of whether or not we are in the middle of a release_keyboard event. At the end
//    // we look at the element that want's the keyboard focuse and explicitly capture it at the end.
//
//    pGUI->flags |= IS_RELEASING_KEYBOARD;
//    {
//        dred_control* pPrevCapturedControl = pGUI->pControlWithKeyboardCapture;
//        pGUI->pControlWithKeyboardCapture = NULL;
//
//        dred_control__post_outbound_event_release_keyboard(pPrevCapturedControl, pNewCapturedControl);
//
//        //if (pNewCapturedControl != NULL) {
//            dred_control__post_outbound_event_release_keyboard_global(pPrevCapturedControl, pNewCapturedControl);
//        //}
//    }
//    pGUI->flags &= ~IS_RELEASING_KEYBOARD;
//
//    // Explicitly capture the keyboard.
//    dred_gui_capture_keyboard(pGUI->pControlWantingKeyboardCapture);
//    pGUI->pControlWantingKeyboardCapture = NULL;
//}

void dred_gui_capture_keyboard(dred_control* pControl)
{
    dtk_capture_keyboard(DTK_CONTROL(pControl)->pTK, DTK_CONTROL(pControl));

#if 0
    if (pControl == NULL) {
        return;
    }

    if (pControl->pGUI == NULL) {
        return;
    }


    if ((pControl->pGUI->flags & IS_RELEASING_KEYBOARD) != 0) {
        pControl->pGUI->pControlWantingKeyboardCapture = pControl;
        return;
    }


    if (pControl->pGUI->pControlWithKeyboardCapture != pControl)
    {
        // Release the previous capture first.
        dred_control* pPrevControlWithKeyboardCapture = pControl->pGUI->pControlWithKeyboardCapture;
        if (pPrevControlWithKeyboardCapture != NULL) {
            dred_gui_release_keyboard_private(pControl->pGUI, pControl);
        }

        //assert(pControl->pGUI->pControlWithKeyboardCapture == NULL);

        pControl->pGUI->pControlWithKeyboardCapture = pControl;
        pControl->pGUI->pControlWantingKeyboardCapture = NULL;

        // Two events need to be posted - the global on_capture event and the local on_capture event. The problem, however, is that the
        // local event handler may change the keyboard capture internally, such as if it wants to pass it's focus onto an internal child
        // element or whatnot. In this case we don't want to fire the global event handler because it will result in superfluous event
        // posting, and could also be posted with an incorrect element.
        dred_control__post_outbound_event_capture_keyboard(pControl, pPrevControlWithKeyboardCapture);

        if (pControl == pControl->pGUI->pControlWithKeyboardCapture) {  // <-- Only post the global event handler if the element still has the capture.
            dred_control__post_outbound_event_capture_keyboard_global(pControl, pPrevControlWithKeyboardCapture);
        }
    }
#endif
}

void dred_gui_release_keyboard(dred_gui* pGUI)
{
    dtk_release_keyboard(&pGUI->pDred->tk);

#if 0
    if (pGUI == NULL) {
        return;
    }

    dred_gui_release_keyboard_private(pGUI, NULL);
#endif
}

//void dred_gui_release_keyboard_no_local_notify(dred_gui* pGUI)
//{
//    if (pGUI == NULL) {
//        return;
//    }
//
//    dred_control* pControl = pGUI->pControlWithMouseCapture;
//    if (pControl == NULL) {
//        return;
//    }
//
//    dred_gui_on_release_keyboard_proc prevProc = pControl->onReleaseKeyboard;
//    pControl->onReleaseKeyboard = NULL;
//    dred_gui_release_keyboard(pGUI);
//    pControl->onReleaseKeyboard = prevProc;
//}
//
//void dred_gui_release_keyboard_no_global_notify(dred_gui* pGUI)
//{
//    if (pGUI == NULL) {
//        return;
//    }
//
//    dred_gui_on_release_keyboard_proc prevProc = pGUI->onGlobalReleaseKeyboard;
//    pGUI->onGlobalReleaseKeyboard = NULL;
//    dred_gui_release_keyboard(pGUI);
//    pGUI->onGlobalReleaseKeyboard = prevProc;
//}

//dred_control* dred_gui_get_element_with_keyboard_capture(dred_gui* pGUI)
//{
//    if (pGUI == NULL) {
//        return NULL;
//    }
//
//    return pGUI->pControlWithKeyboardCapture;
//}
//
//dr_bool32 dred_control_has_keyboard_capture(dred_control* pControl)
//{
//    if (pControl == NULL) {
//        return DR_FALSE;
//    }
//
//    return dred_gui_get_element_with_keyboard_capture(pControl->pGUI) == pControl;
//}


void dred_control_set_cursor(dred_control* pControl, dtk_system_cursor_type cursor)
{
    dtk_control_set_cursor(DTK_CONTROL(pControl), cursor);

#if 0
    if (pControl == NULL) {
        return;
    }

    pControl->cursor = cursor;

    if (dred_control_is_under_mouse(pControl) && pControl->pGUI->currentCursor != cursor) {
        dred_gui__change_cursor(pControl, cursor);
    }
#endif
}

dtk_system_cursor_type dred_control_get_cursor(dred_control* pControl)
{
    return dtk_control_get_cursor(DTK_CONTROL(pControl));

#if 0
    if (pControl == NULL) {
        return dtk_system_cursor_type_none;
    }

    return pControl->cursor;
#endif
}

void dred_control_show_popup_menu(dred_control* pControl, dtk_menu* pMenu, int relativePosX, int relativePosY)
{
    if (pControl == NULL || pMenu == NULL) {
        return;
    }

    dred_window* pWindow = dred_get_control_window(pControl);
    if (pWindow == NULL) {
        return;
    }

    int mousePosXWindow = relativePosX + (int)dred_control_get_absolute_position_x(pControl);
    int mousePosYWindow = relativePosY + (int)dred_control_get_absolute_position_y(pControl);
    dred_window_show_popup_menu(pWindow, pMenu, mousePosXWindow, mousePosYWindow);
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



dr_bool32 dred_control_is_point_inside_bounds(const dred_control* pControl, float absolutePosX, float absolutePosY)
{
    return dtk_control_is_point_inside_bounds(DTK_CONTROL(pControl), (dtk_int32)absolutePosX, (dtk_int32)absolutePosY);

#if 0
    if (absolutePosX < pControl->absolutePosX ||
        absolutePosX < pControl->absolutePosY)
    {
        return DR_FALSE;
    }

    if (absolutePosX >= pControl->absolutePosX + pControl->width ||
        absolutePosY >= pControl->absolutePosY + pControl->height)
    {
        return DR_FALSE;
    }

    return DR_TRUE;
#endif
}

dr_bool32 dred_control_is_point_inside(dred_control* pControl, float absolutePosX, float absolutePosY)
{
    return dtk_control_is_point_inside(DTK_CONTROL(pControl), (dtk_int32)absolutePosX, (dtk_int32)absolutePosY);
    
#if 0
    if (dred_control_is_point_inside_bounds(pControl, absolutePosX, absolutePosY))
    {
        // It is valid for onHitTest to be null, in which case we use the default hit test which assumes the element is just a rectangle
        // equal to the size of it's bounds. It's equivalent to onHitTest always returning DR_TRUE.

        if (pControl->onHitTest) {
            return pControl->onHitTest(pControl, absolutePosX - pControl->absolutePosX, absolutePosY - pControl->absolutePosY);
        }

        return DR_TRUE;
    }

    return DR_FALSE;
#endif
}


#if 0
typedef struct
{
    dred_control* pControlUnderPoint;
    float absolutePosX;
    float absolutePosY;
}dred_gui_find_control_under_point_data;

dr_bool32 dred_gui_find_control_under_point_iterator(dred_control* pControl, dred_rect* pRelativeVisibleRect, void* pUserData)
{
    assert(pControl             != NULL);
    assert(pRelativeVisibleRect != NULL);

    dred_gui_find_control_under_point_data* pData = (dred_gui_find_control_under_point_data*)pUserData;
    assert(pData != NULL);

    float relativePosX = pData->absolutePosX;
    float relativePosY = pData->absolutePosY;
    dred_make_point_relative(pControl, &relativePosX, &relativePosY);

    if (dred_rect_contains_point(*pRelativeVisibleRect, relativePosX, relativePosY))
    {
        if (pControl->onHitTest) {
            if (pControl->onHitTest(pControl, relativePosX, relativePosY)) {
                pData->pControlUnderPoint = pControl;
            }
        } else {
            pData->pControlUnderPoint = pControl;
        }
    }


    // Always return DR_TRUE to ensure the entire hierarchy is checked.
    return DR_TRUE;
}
#endif

//dtk_control* dred_gui_find_control_under_point(dred_control* pTopLevelControl, float absolutePosX, float absolutePosY)
//{
//    return dtk_window_find_control_under_point(dtk_control_get_window(DTK_CONTROL(pTopLevelControl)), (dtk_int32)absolutePosX, (dtk_int32)absolutePosY);
//
//#if 0
//    if (pTopLevelControl == NULL) {
//        return NULL;
//    }
//
//    dred_gui_find_control_under_point_data data;
//    data.pControlUnderPoint = NULL;
//    data.absolutePosX = absolutePosX;
//    data.absolutePosY = absolutePosY;
//    dred_control_iterate_visible_elements(pTopLevelControl, dred_control_get_absolute_rect(pTopLevelControl), dred_gui_find_control_under_point_iterator, &data);
//
//    return data.pControlUnderPoint;
//#endif
//}

dr_bool32 dred_control_is_under_mouse(dred_control* pControl)
{
    return dtk_control_is_under_mouse(DTK_CONTROL(pControl));

#if 0
    if (pControl == NULL) {
        return DR_FALSE;
    }

    return dred_gui_find_control_under_point(pControl->pGUI->pLastMouseMoveTopLevelControl, pControl->pGUI->lastMouseMovePosX, pControl->pGUI->lastMouseMovePosY) == pControl;
#endif
}



//// Hierarchy ////

//dred_control* dtk_control_get_parent(dred_control* pChildControl)
//{
//    if (pChildControl == NULL) {
//        return NULL;
//    }
//
//    return pChildControl->pParent;
//}

void dred_control_detach(dred_control* pChildControl)
{
#if 0
    if (pChildControl == NULL) {
        return;
    }

    dred_control* pOldParent = pChildControl->pParent;


    // We orphan the element using the private API. This will not mark the parent element as dirty so we need to do that afterwards.
    dred_control__detach_without_redraw(pChildControl);

    // The region of the old parent needs to be redrawn.
    if (pOldParent != NULL) {
        dred_control__auto_dirty(pOldParent, dred_control_get_relative_rect(pOldParent));
    }
#endif

    dtk_control_detach(DTK_CONTROL(pChildControl));
}

void dred_control_append(dred_control* pChildControl, dred_control* pParentControl)
{
#if 0
    if (pChildControl == NULL) {
        return;
    }

    // We first need to orphan the element. If the parent element is the new parent is the same as the old one, as in we
    // are just moving the child element to the end of the children list, we want to delay the repaint until the end. To
    // do this we use dred_control__detach_without_redraw() because that will not trigger a redraw.
    if (pChildControl->pParent != pParentControl) {
        dred_control_detach(pChildControl);
    } else {
        dred_control__detach_without_redraw(pChildControl);
    }


    // Now we attach it to the end of the new parent.
    if (pParentControl != NULL) {
        dred_control__append_without_detach(pChildControl, pParentControl);
    }
#endif

    dtk_control_append(DTK_CONTROL(pChildControl), DTK_CONTROL(pParentControl));
}

void dred_control_prepend(dred_control* pChildControl, dred_control* pParentControl)
{
    dtk_control_prepend(DTK_CONTROL(pChildControl), DTK_CONTROL(pParentControl));

#if 0
    if (pChildControl == NULL) {
        return;
    }

    // See comment in dred_control_append() for explanation on this.
    if (pChildControl->pParent != pParentControl) {
        dred_control_detach(pChildControl);
    } else {
        dred_control__detach_without_redraw(pChildControl);
    }


    // Now we need to attach the element to the beginning of the parent.
    if (pParentControl != NULL) {
        dred_control__prepend_without_detach(pChildControl, pParentControl);
    }
#endif
}

void dred_control_append_sibling(dred_control* pControlToAppend, dred_control* pControlToAppendTo)
{
    dtk_control_append_sibling(DTK_CONTROL(pControlToAppend), DTK_CONTROL(pControlToAppendTo));

#if 0
    if (pControlToAppend == NULL || pControlToAppendTo == NULL) {
        return;
    }

    // See comment in dred_control_append() for explanation on this.
    if (pControlToAppend->pParent != pControlToAppendTo->pParent) {
        dred_control_detach(pControlToAppend);
    } else {
        dred_control__detach_without_redraw(pControlToAppend);
    }


    // Now we need to attach the element such that it comes just after pControlToAppendTo
    dred_control__append_sibling_without_detach(pControlToAppend, pControlToAppendTo);
#endif
}

void dred_control_prepend_sibling(dred_control* pControlToPrepend, dred_control* pControlToPrependTo)
{
    dtk_control_prepend_sibling(DTK_CONTROL(pControlToPrepend), DTK_CONTROL(pControlToPrependTo));

#if 0
    if (pControlToPrepend == NULL || pControlToPrependTo == NULL) {
        return;
    }

    // See comment in dred_control_append() for explanation on this.
    if (pControlToPrepend->pParent != pControlToPrependTo->pParent) {
        dred_control_detach(pControlToPrepend);
    } else {
        dred_control__detach_without_redraw(pControlToPrepend);
    }


    // Now we need to attach the element such that it comes just after pControlToPrependTo
    dred_control__prepend_sibling_without_detach(pControlToPrepend, pControlToPrependTo);
#endif
}

dtk_control* dred_control_find_top_level_control(dred_control* pControl)
{
    return dtk_control_find_top_level_control(DTK_CONTROL(pControl));

#if 0
    if (pControl == NULL) {
        return NULL;
    }

    if (pControl->pParent != NULL) {
        return dred_control_find_top_level_control(pControl->pParent);
    }

    return pControl;
#endif
}

dr_bool32 dred_control_is_parent(dred_control* pParentControl, dred_control* pChildControl)
{
    return dtk_control_is_parent(DTK_CONTROL(pParentControl), DTK_CONTROL(pChildControl));

#if 0
    if (pParentControl == NULL || pChildControl == NULL) {
        return DR_FALSE;
    }

    return pParentControl == pChildControl->pParent;
#endif
}

dr_bool32 dred_control_is_child(dred_control* pChildControl, dred_control* pParentControl)
{
    return dred_control_is_parent(pParentControl, pChildControl);
}

dr_bool32 dred_control_is_ancestor(dred_control* pAncestorControl, dred_control* pChildControl)
{
    return dtk_control_is_ancestor(DTK_CONTROL(pAncestorControl), DTK_CONTROL(pChildControl));
    
#if 0
    if (pAncestorControl == NULL || pChildControl == NULL) {
        return DR_FALSE;
    }

    dred_control* pParent = pChildControl->pParent;
    while (pParent != NULL)
    {
        if (pParent == pAncestorControl) {
            return DR_TRUE;
        }

        pParent = pParent->pParent;
    }


    return DR_FALSE;
#endif
}

dr_bool32 dred_control_is_descendant(dred_control* pChildControl, dred_control* pAncestorControl)
{
    return dred_control_is_ancestor(pAncestorControl, pChildControl);
}

dr_bool32 dred_control_is_self_or_ancestor(dred_control* pAncestorControl, dred_control* pChildControl)
{
    return pAncestorControl == pChildControl || dred_control_is_ancestor(pAncestorControl, pChildControl);
}

dr_bool32 dred_control_is_self_or_descendant(dred_control* pChildControl, dred_control* pAncestorControl)
{
    return pChildControl == pAncestorControl || dred_control_is_descendant(pChildControl, pAncestorControl);
}



//// Layout ////

void dred_control_set_absolute_position(dred_control* pControl, float positionX, float positionY)
{
    if (pControl == NULL) return;
    dtk_control_set_absolute_position(DTK_CONTROL(pControl), (dtk_int32)positionX, (dtk_int32)positionY);

#if 0
    if (pControl->absolutePosX != positionX || pControl->absolutePosY != positionY)
    {
        float oldRelativePosX = dred_control_get_relative_position_x(pControl);
        float oldRelativePosY = dred_control_get_relative_position_y(pControl);

        dred_control__begin_auto_dirty(pControl);
        {
            dred_control__auto_dirty(pControl, dred_control_get_local_rect(pControl));     // <-- Previous rectangle.

            float offsetX = positionX - pControl->absolutePosX;
            float offsetY = positionY - pControl->absolutePosY;

            pControl->absolutePosX = positionX;
            pControl->absolutePosY = positionY;
            dred_control__auto_dirty(pControl, dred_control_get_local_rect(pControl));     // <-- New rectangle.


            float newRelativePosX = dred_control_get_relative_position_x(pControl);
            float newRelativePosY = dred_control_get_relative_position_y(pControl);

            if (newRelativePosX != oldRelativePosX || newRelativePosY != oldRelativePosY) {
                dred_control__post_outbound_event_move(pControl, newRelativePosX, newRelativePosY);
            }


            dred_control__apply_offset_to_children_recursive(pControl, offsetX, offsetY);
        }
        dred_control__end_auto_dirty(pControl);
    }
#endif
}

void dred_control_get_absolute_position(const dred_control* pControl, float * positionXOut, float * positionYOut)
{
    dtk_int32 x;
    dtk_int32 y;
    dtk_control_get_absolute_position(DTK_CONTROL(pControl), &x, &y);

    if (positionXOut) *positionXOut = (float)x;
    if (positionYOut) *positionYOut = (float)y;
    
#if 0
    if (positionXOut != NULL) *positionXOut = 0;
    if (positionYOut != NULL) *positionYOut = 0;
    if (pControl == NULL) return;

    if (positionXOut != NULL) {
        *positionXOut = pControl->absolutePosX;
    }

    if (positionYOut != NULL) {
        *positionYOut = pControl->absolutePosY;
    }
#endif
}

float dred_control_get_absolute_position_x(const dred_control* pControl)
{
    if (pControl == NULL) return 0.0f;
    return (float)pControl->baseControl.absolutePosX;

#if 0
    if (pControl != NULL) {
        return pControl->absolutePosX;
    }

    return 0.0f;
#endif
}

float dred_control_get_absolute_position_y(const dred_control* pControl)
{
    if (pControl == NULL) return 0.0f;
    return (float)pControl->baseControl.absolutePosY;

#if 0
    if (pControl != NULL) {
        return pControl->absolutePosY;
    }

    return 0.0f;
#endif
}


void dred_control_set_relative_position(dred_control* pControl, float relativePosX, float relativePosY)
{
    if (pControl == NULL) return;
    dtk_control_set_relative_position(DTK_CONTROL(pControl), (dtk_int32)relativePosX, (dtk_int32)relativePosY);

#if 0
    if (pControl->pParent != NULL) {
        dred_control_set_absolute_position(pControl, pControl->pParent->absolutePosX + relativePosX, pControl->pParent->absolutePosY + relativePosY);
    } else {
        dred_control_set_absolute_position(pControl, relativePosX, relativePosY);
    }
#endif
}

void dred_control_get_relative_position(const dred_control* pControl, float* positionXOut, float* positionYOut)
{
    dtk_int32 x;
    dtk_int32 y;
    dtk_control_get_relative_position(DTK_CONTROL(pControl), &x, &y);

    if (positionXOut) *positionXOut = (float)x;
    if (positionYOut) *positionYOut = (float)y;

#if 0
    if (pControl != NULL)
    {
        if (pControl->pParent != NULL)
        {
            if (positionXOut != NULL) {
                *positionXOut = pControl->absolutePosX - pControl->pParent->absolutePosX;
            }

            if (positionYOut != NULL) {
                *positionYOut = pControl->absolutePosY - pControl->pParent->absolutePosY;
            }
        }
        else
        {
            if (positionXOut != NULL) {
                *positionXOut = pControl->absolutePosX;
            }

            if (positionYOut != NULL) {
                *positionYOut = pControl->absolutePosY;
            }
        }
    }
#endif
}

float dred_control_get_relative_position_x(const dred_control* pControl)
{
    float x;
    dred_control_get_relative_position(pControl, &x, NULL);
    return x;

#if 0
    if (pControl != NULL) {
        if (pControl->pParent != NULL) {
            return pControl->absolutePosX - pControl->pParent->absolutePosX;
        } else {
            return pControl->absolutePosX;
        }
    }

    return 0;
#endif
}

float dred_control_get_relative_position_y(const dred_control* pControl)
{
    float y;
    dred_control_get_relative_position(pControl, NULL, &y);
    return y;

#if 0
    if (pControl != NULL) {
        if (pControl->pParent != NULL) {
            return pControl->absolutePosY - pControl->pParent->absolutePosY;
        } else {
            return pControl->absolutePosY;
        }
    }

    return 0;
#endif
}


void dred_control_set_size(dred_control* pControl, float width, float height)
{
    if (pControl == NULL) return;
    dtk_control_set_size(DTK_CONTROL(pControl), (dtk_uint32)width, (dtk_uint32)height);

#if 0
    if (pControl->width != width || pControl->height != height)
    {
        dred_control__begin_auto_dirty(pControl);
        {
            dred_control__auto_dirty(pControl, dred_control_get_local_rect(pControl));     // <-- Previous rectangle.

            pControl->width  = width;
            pControl->height = height;
            dred_control__auto_dirty(pControl, dred_control_get_local_rect(pControl));     // <-- New rectangle.

            dred_control__post_outbound_event_size(pControl, width, height);
        }
        dred_control__end_auto_dirty(pControl);
    }
#endif
}

void dred_control_get_size(const dred_control* pControl, float* widthOut, float* heightOut)
{
    dtk_uint32 sizeX;
    dtk_uint32 sizeY;
    dtk_control_get_size(DTK_CONTROL(pControl), &sizeX, &sizeY);

    if (widthOut)  *widthOut  = (float)sizeX;
    if (heightOut) *heightOut = (float)sizeY;

#if 0
    if (pControl != NULL) {
        if (widthOut) *widthOut = pControl->width;
        if (heightOut) *heightOut = pControl->height;
    } else {
        if (widthOut) *widthOut = 0;
        if (heightOut) *heightOut = 0;
    }
#endif
}

float dred_control_get_width(const dred_control * pControl)
{
    return (float)pControl->baseControl.width;

#if 0
    if (pControl != NULL) {
        return pControl->width;
    }

    return 0;
#endif
}

float dred_control_get_height(const dred_control * pControl)
{
    return (float)pControl->baseControl.height;

#if 0
    if (pControl != NULL) {
        return pControl->height;
    }

    return 0;
#endif
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

#if 0
    dred_rect rect;
    if (pControl != NULL)
    {
        rect.left   = pControl->absolutePosX;
        rect.top    = pControl->absolutePosY;
        rect.right  = rect.left + pControl->width;
        rect.bottom = rect.top + pControl->height;
    }
    else
    {
        rect.left   = 0;
        rect.top    = 0;
        rect.right  = 0;
        rect.bottom = 0;
    }

    return rect;
#endif
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

#if 0
    dred_rect rect;
    if (pControl != NULL)
    {
        rect.left   = dred_control_get_relative_position_x(pControl);
        rect.top    = dred_control_get_relative_position_y(pControl);
        rect.right  = rect.left + pControl->width;
        rect.bottom = rect.top  + pControl->height;
    }
    else
    {
        rect.left   = 0;
        rect.top    = 0;
        rect.right  = 0;
        rect.bottom = 0;
    }

    return rect;
#endif
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

#if 0
    dred_rect rect;
    rect.left = 0;
    rect.top  = 0;

    if (pControl != NULL)
    {
        rect.right  = pControl->width;
        rect.bottom = pControl->height;
    }
    else
    {
        rect.right  = 0;
        rect.bottom = 0;
    }

    return rect;
#endif
}



//// Painting ////

dr_bool32 dred_gui_register_painting_callbacks(dred_gui* pGUI, dred_gui_painting_callbacks callbacks)
{
    if (pGUI == NULL) {
        return DR_FALSE;
    }

    pGUI->paintingCallbacks = callbacks;

    return DR_TRUE;
}

#if 0
dr_bool32 dred_control_iterate_visible_elements(dred_control* pParentControl, dred_rect relativeRect, dred_gui_visible_iteration_proc callback, void* pUserData)
{
    if (pParentControl == NULL) {
        return DR_FALSE;
    }

    if (callback == NULL) {
        return DR_FALSE;
    }


    if (!dred_control_is_visible(pParentControl)) {
        return DR_TRUE;
    }

    dred_rect clampedRelativeRect = relativeRect;
    if (dred_clamp_rect_to_element(pParentControl, &clampedRelativeRect))
    {
        // We'll only get here if some part of the rectangle was inside the element.
        if (!callback(pParentControl, &clampedRelativeRect, pUserData)) {
            return DR_FALSE;
        }
    }

    for (dred_control* pChild = pParentControl->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling)
    {
        float childRelativePosX = dred_control_get_relative_position_x(pChild);
        float childRelativePosY = dred_control_get_relative_position_y(pChild);

        dred_rect childRect;
        if (dred_control_is_clipping_enabled(pChild)) {
            childRect = clampedRelativeRect;
        } else {
            childRect = relativeRect;
        }


        childRect.left   -= childRelativePosX;
        childRect.top    -= childRelativePosY;
        childRect.right  -= childRelativePosX;
        childRect.bottom -= childRelativePosY;

        if (!dred_control_iterate_visible_elements(pChild, childRect, callback, pUserData)) {
            return DR_FALSE;
        }
    }


    return DR_TRUE;
}
#endif

//void dred_gui_disable_auto_dirty(dred_gui* pGUI)
//{
//    if (pGUI != NULL) {
//        pGUI->flags |= IS_AUTO_DIRTY_DISABLED;
//    }
//}
//
//void dred_gui_enable_auto_dirty(dred_gui* pGUI)
//{
//    if (pGUI != NULL) {
//        pGUI->flags &= ~IS_AUTO_DIRTY_DISABLED;
//    }
//}
//
//dr_bool32 dred_gui_is_auto_dirty_enabled(dred_gui* pGUI)
//{
//    if (pGUI != NULL) {
//        return (pGUI->flags & IS_AUTO_DIRTY_DISABLED) == 0;
//    }
//
//    return DR_FALSE;
//}

#if 0
dred_control* dred_control_begin_dirty(dred_control* pControl)
{
    if (pControl == NULL) {
        return NULL;
    }

    dred_gui* pGUI = pControl->pGUI;
    assert(pGUI != NULL);

    dred_control* pTopLevelControl = dred_control_find_top_level_control(pControl);
    assert(pTopLevelControl != NULL);

    // The element needs to be added to the list of dirty elements if it doesn't exist already.
    dr_bool32 isAlreadyDirty = DR_FALSE;
    for (size_t iDirtyControlCount = 0; iDirtyControlCount < pGUI->dirtyControlCount; ++iDirtyControlCount) {
        if (pGUI->ppDirtyControls[iDirtyControlCount] == pTopLevelControl) {
            isAlreadyDirty = DR_TRUE;
            break;
        }
    }

    if (!isAlreadyDirty) {
        if (pGUI->dirtyControlCount == pGUI->dirtyControlBufferSize) {
            size_t newBufferSize = pGUI->dirtyControlBufferSize == 0 ? 1 : pGUI->dirtyControlBufferSize*2;
            dred_control** ppNewDirtyControls = (dred_control**)realloc(pGUI->ppDirtyControls, newBufferSize * sizeof(*ppNewDirtyControls));
            if (ppNewDirtyControls == NULL) {
                return NULL;
            }

            pGUI->ppDirtyControls = ppNewDirtyControls;
            pGUI->dirtyControlBufferSize = newBufferSize;
        }

        pGUI->ppDirtyControls[pGUI->dirtyCounter] = pTopLevelControl;
        pGUI->dirtyControlCount += 1;
    }


    pGUI->dirtyCounter += 1;
    return pTopLevelControl;
}

void dred_control_end_dirty(dred_control* pControl)
{
    if (pControl == NULL) {
        return;
    }

    dred_gui* pGUI = pControl->pGUI;
    assert(pGUI != NULL);

    assert(pGUI->dirtyControlCount > 0);
    assert(pGUI->dirtyCounter > 0);

    pGUI->dirtyCounter -= 1;
    if (pGUI->dirtyCounter == 0)
    {
        for (size_t i = 0; i < pGUI->dirtyControlCount; ++i) {
            dtk_control_immediate_redraw(DTK_CONTROL(pGUI->ppDirtyControls[i]), dtk_rect_init_dred(pGUI->ppDirtyControls[i]->dirtyRect));
            pGUI->ppDirtyControls[i]->dirtyRect = dred_make_inside_out_rect();
        }

        pGUI->dirtyControlCount = 0;
    }
}
#endif

void dred_control_dirty(dred_control* pControl, dred_rect relativeRect)
{
    if (pControl == NULL) {
        return;
    }

    dtk_control_scheduled_redraw(DTK_CONTROL(pControl), dtk_rect_init_dred(relativeRect));

    //dred_gui* pGUI = pControl->pGUI;
    //assert(pGUI != NULL);

#if 0
    dred_control* pTopLevelControl = dred_control_begin_dirty(pControl);
    if (pTopLevelControl == NULL) {
        return;
    }

    pTopLevelControl->dirtyRect = dred_rect_union(pTopLevelControl->dirtyRect, dred_make_rect_absolute(pControl, &relativeRect));
    dred_control_end_dirty(pControl);
#endif
}


#if 0
dr_bool32 dred_control_draw_iteration_callback(dred_control* pControl, dred_rect* pRelativeRect, void* pUserData)
{
    assert(pControl      != NULL);
    assert(pRelativeRect != NULL);

    if (pControl->onPaint != NULL)
    {
        // We want to set the initial clipping rectangle before drawing.
        dred_control_set_clip(pControl, *pRelativeRect, (dtk_surface*)pUserData);

        // We now call the painting function, but only after setting the clipping rectangle.
        pControl->onPaint(pControl, *pRelativeRect, (dtk_surface*)pUserData);

        // The on_paint event handler may have adjusted the clipping rectangle so we need to ensure it's restored.
        dred_control_set_clip(pControl, *pRelativeRect, (dtk_surface*)pUserData);
    }

    return DR_TRUE;
}

void dred_control_draw(dred_control* pControl, dred_rect relativeRect, dtk_surface* pSurface)
{
    (void)pSurface;
    dtk_control_immediate_redraw(DTK_CONTROL(pControl), dtk_rect_init_dred(relativeRect));

    /*if (pControl == NULL) {
        return;
    }

    dred_gui* pGUI = pControl->pGUI;
    if (pGUI == NULL) {
        return;
    }

    dred_control_iterate_visible_elements(pControl, relativeRect, dred_control_draw_iteration_callback, pSurface);*/
}
#endif

void dred_control_get_clip(dred_control* pControl, dred_rect* pRelativeRect, dtk_surface* pSurface)
{
    if (pControl == NULL || pControl->pGUI == NULL) {
        return;
    }

    pControl->pGUI->paintingCallbacks.getClip(pRelativeRect, pSurface);

    // The clip returned by the drawing callback will be absolute so we'll need to convert that to relative.
    //dred_make_rect_relative(pControl, pRelativeRect);
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

    //dred_rect absoluteRect = relativeRect;
    //dred_make_rect_absolute(pControl, &absoluteRect);

    pControl->pGUI->paintingCallbacks.setClip(relativeRect, pSurface);
}

void dred_control_draw_rect(dred_control* pControl, dred_rect relativeRect, dtk_color color, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    //dred_rect absoluteRect = relativeRect;
    //dred_make_rect_absolute(pControl, &absoluteRect);

    pControl->pGUI->paintingCallbacks.drawRect(relativeRect, color, pSurface);
}

void dred_control_draw_rect_outline(dred_control* pControl, dred_rect relativeRect, dtk_color color, float outlineWidth, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    //dred_rect absoluteRect = relativeRect;
    //dred_make_rect_absolute(pControl, &absoluteRect);

    pControl->pGUI->paintingCallbacks.drawRectOutline(relativeRect, color, outlineWidth, pSurface);
}

void dred_control_draw_rect_with_outline(dred_control * pControl, dred_rect relativeRect, dtk_color color, float outlineWidth, dtk_color outlineColor, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    //dred_rect absoluteRect = relativeRect;
    //dred_make_rect_absolute(pControl, &absoluteRect);

    pControl->pGUI->paintingCallbacks.drawRectWithOutline(relativeRect, color, outlineWidth, outlineColor, pSurface);
}

void dred_control_draw_round_rect(dred_control* pControl, dred_rect relativeRect, dtk_color color, float radius, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    //dred_rect absoluteRect = relativeRect;
    //dred_make_rect_absolute(pControl, &absoluteRect);

    pControl->pGUI->paintingCallbacks.drawRoundRect(relativeRect, color, radius, pSurface);
}

void dred_control_draw_round_rect_outline(dred_control* pControl, dred_rect relativeRect, dtk_color color, float radius, float outlineWidth, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    //dred_rect absoluteRect = relativeRect;
    //dred_make_rect_absolute(pControl, &absoluteRect);

    pControl->pGUI->paintingCallbacks.drawRoundRectOutline(relativeRect, color, radius, outlineWidth, pSurface);
}

void dred_control_draw_round_rect_with_outline(dred_control* pControl, dred_rect relativeRect, dtk_color color, float radius, float outlineWidth, dtk_color outlineColor, dtk_surface* pSurface)
{
    if (pControl == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    //dred_rect absoluteRect = relativeRect;
    //dred_make_rect_absolute(pControl, &absoluteRect);

    pControl->pGUI->paintingCallbacks.drawRoundRectWithOutline(relativeRect, color, radius, outlineWidth, outlineColor, pSurface);
}

void dred_control_draw_text(dred_control* pControl, dred_gui_font* pFont, const char* text, int textLengthInBytes, float posX, float posY, dtk_color color, dtk_color backgroundColor, dtk_surface* pSurface)
{
    if (pControl == NULL || pFont == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    //float absolutePosX = posX;
    //float absolutePosY = posY;
    //dred_make_point_absolute(pControl, &absolutePosX, &absolutePosY);

    pControl->pGUI->paintingCallbacks.drawText(pFont->pInternalFont, text, textLengthInBytes, posX, posY, color, backgroundColor, pSurface);
}

void dred_control_draw_image(dred_control* pControl, dred_gui_image* pImage, dred_gui_draw_image_args* pArgs, dtk_surface* pSurface)
{
    if (pControl == NULL || pImage == NULL || pArgs == NULL) {
        return;
    }

    assert(pControl->pGUI != NULL);

    //dred_make_point_absolute(pControl, &pArgs->dstX, &pArgs->dstY);
    //dred_make_point_absolute(pControl, &pArgs->dstBoundsX, &pArgs->dstBoundsY);

    if ((pArgs->options & DRED_GUI_IMAGE_ALIGN_CENTER) != 0)
    {
        pArgs->dstX = pArgs->dstBoundsX + (pArgs->dstBoundsWidth  - pArgs->dstWidth)  / 2;
        pArgs->dstY = pArgs->dstBoundsY + (pArgs->dstBoundsHeight - pArgs->dstHeight) / 2;
    }

    dred_rect prevClip;
    pControl->pGUI->paintingCallbacks.getClip(&prevClip, pSurface);

    dr_bool32 restoreClip = DR_FALSE;
    if ((pArgs->options & DRED_GUI_IMAGE_CLIP_BOUNDS) != 0)
    {
        // We only need to clip if part of the destination rectangle falls outside of the bounds.
        if (pArgs->dstX < pArgs->dstBoundsX || pArgs->dstX + pArgs->dstWidth  > pArgs->dstBoundsX + pArgs->dstBoundsWidth ||
            pArgs->dstY < pArgs->dstBoundsY || pArgs->dstY + pArgs->dstHeight > pArgs->dstBoundsY + pArgs->dstBoundsHeight)
        {
            restoreClip = DR_TRUE;
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

    dtk_draw_surface_args argsDTK;
    argsDTK.dstX = (dtk_int32)pArgs->dstX;
    argsDTK.dstY = (dtk_int32)pArgs->dstY;
    argsDTK.dstWidth = (dtk_int32)pArgs->dstWidth;
    argsDTK.dstHeight = (dtk_int32)pArgs->dstHeight;
    argsDTK.srcX = (dtk_int32)pArgs->srcX;
    argsDTK.srcY = (dtk_int32)pArgs->srcY;
    argsDTK.srcWidth = (dtk_int32)pArgs->srcWidth;
    argsDTK.srcHeight = (dtk_int32)pArgs->srcHeight;
    argsDTK.foregroundTint = dtk_color_init_dred(pArgs->foregroundTint);
    argsDTK.backgroundColor = dtk_color_init_dred(pArgs->backgroundColor);
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

dr_bool32 dred_gui_get_font_metrics(dred_gui_font* pFont, dtk_font_metrics* pMetricsOut)
{
    if (pFont == NULL || pMetricsOut == NULL) {
        return DR_FALSE;
    }

    assert(pFont->pGUI != NULL);

    if (pFont->pGUI->paintingCallbacks.getFontMetrics == NULL) {
        return DR_FALSE;
    }

    return pFont->pGUI->paintingCallbacks.getFontMetrics(pFont->pInternalFont, pMetricsOut);
}

dr_bool32 dred_gui_get_glyph_metrics(dred_gui_font* pFont, unsigned int utf32, dtk_glyph_metrics* pMetricsOut)
{
    if (pFont == NULL || pMetricsOut == NULL) {
        return DR_FALSE;
    }

    assert(pFont->pGUI != NULL);

    if (pFont->pGUI->paintingCallbacks.getGlyphMetrics == NULL) {
        return DR_FALSE;
    }

    return pFont->pGUI->paintingCallbacks.getGlyphMetrics(pFont->pInternalFont, utf32, pMetricsOut);
}

dr_bool32 dred_gui_measure_string(dred_gui_font* pFont, const char* text, size_t textLengthInBytes, float* pWidthOut, float* pHeightOut)
{
    if (pFont == NULL) {
        return DR_FALSE;
    }

    if (text == NULL || textLengthInBytes == 0)
    {
        dtk_font_metrics metrics;
        if (!dred_gui_get_font_metrics(pFont, &metrics)) {
            return DR_FALSE;
        }

        if (pWidthOut) {
            *pWidthOut = 0;
        }
        if (pHeightOut) {
            *pHeightOut = (float)metrics.lineHeight;
        }

        return DR_TRUE;
    }



    assert(pFont->pGUI != NULL);

    if (pFont->pGUI->paintingCallbacks.measureString == NULL) {
        return DR_FALSE;
    }

    return pFont->pGUI->paintingCallbacks.measureString(pFont->pInternalFont, text, textLengthInBytes, pWidthOut, pHeightOut);
}

dr_bool32 dred_gui_get_text_cursor_position_from_point(dred_gui_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut)
{
    if (pFont == NULL) {
        return DR_FALSE;
    }

    assert(pFont->pGUI != NULL);

    if (pFont->pGUI->paintingCallbacks.getTextCursorPositionFromPoint) {
        return pFont->pGUI->paintingCallbacks.getTextCursorPositionFromPoint(pFont->pInternalFont, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosXOut, pCharacterIndexOut);
    }

    return DR_FALSE;
}

dr_bool32 dred_gui_get_text_cursor_position_from_char(dred_gui_font* pFont, const char* text, size_t characterIndex, float* pTextCursorPosXOut)
{
    if (pFont == NULL) {
        return DR_FALSE;
    }

    assert(pFont->pGUI != NULL);

    if (pFont->pGUI->paintingCallbacks.getTextCursorPositionFromChar) {
        return pFont->pGUI->paintingCallbacks.getTextCursorPositionFromChar(pFont->pInternalFont, text, characterIndex, pTextCursorPosXOut);
    }

    return DR_FALSE;
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

dr_bool32 dred_control_pass_through_hit_test(dred_control* pControl, float mousePosX, float mousePosY)
{
    (void)pControl;
    (void)mousePosX;
    (void)mousePosY;

    return DR_FALSE;
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

dr_bool32 dred_clamp_rect_to_element(const dred_control* pControl, dred_rect* pRelativeRect)
{
    if (pControl == NULL || pRelativeRect == NULL) {
        return DR_FALSE;
    }

    dtk_rect rect = dtk_rect_init_dred(*pRelativeRect);
    dtk_bool32 result = dtk_control_clamp_rect(DTK_CONTROL(pControl), &rect);
    
    pRelativeRect->left   = (float)rect.left;
    pRelativeRect->top    = (float)rect.top;
    pRelativeRect->right  = (float)rect.right;
    pRelativeRect->bottom = (float)rect.bottom;

    return result;


#if 0
    if (pRelativeRect->left < 0) {
        pRelativeRect->left = 0;
    }
    if (pRelativeRect->top < 0) {
        pRelativeRect->top = 0;
    }

    if (pRelativeRect->right > pControl->width) {
        pRelativeRect->right = pControl->width;
    }
    if (pRelativeRect->bottom > pControl->height) {
        pRelativeRect->bottom = pControl->height;
    }


    return (pRelativeRect->right - pRelativeRect->left > 0) && (pRelativeRect->bottom - pRelativeRect->top > 0);
#endif
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

dr_bool32 dred_rect_contains_point(dred_rect rect, float posX, float posY)
{
    if (posX < rect.left || posY < rect.top) {
        return DR_FALSE;
    }

    if (posX >= rect.right || posY >= rect.bottom) {
        return DR_FALSE;
    }

    return DR_TRUE;
}

dr_bool32 dred_rect_equal(dred_rect rect0, dred_rect rect1)
{
    return
        rect0.left   == rect1.left  &&
        rect0.top    == rect1.top   &&
        rect0.right  == rect1.right &&
        rect0.bottom == rect1.bottom;
}

dr_bool32 dred_rect_has_volume(dred_rect rect)
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
void dred_control_draw_text_dtk(dtk_font*, const char*, int, float, float, dtk_color, dtk_color, dtk_surface* pSurface);
void dred_control_draw_image_dtk(dtk_surface*, dtk_draw_surface_args* pArgs, dtk_surface* pSurface);

dtk_font* dred_gui_create_font_dtk(dtk_context*, const char*, unsigned int, dtk_font_weight, dtk_font_slant, unsigned int flags);
void dred_gui_delete_font_dtk(dtk_font*);
unsigned int dred_gui_get_font_size_dtk(dtk_font*);
dr_bool32 dred_gui_get_font_metrics_dtk(dtk_font*, dtk_font_metrics*);
dr_bool32 dred_gui_get_glyph_metrics_dtk(dtk_font*, unsigned int, dtk_glyph_metrics*);
dr_bool32 dred_gui_measure_string_dtk(dtk_font*, const char*, size_t, float*, float*);
dr_bool32 dred_gui_get_text_cursor_position_from_point_dtk(dtk_font*, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut);
dr_bool32 dred_gui_get_text_cursor_position_from_char_dtk(dtk_font*, const char* text, size_t characterIndex, float* pTextCursorPosXOut);

dtk_surface* dred_gui_create_image_dtk(dtk_context*, unsigned int width, unsigned int height, unsigned int stride, const void* pImageData);
void dred_gui_delete_image_dtk(dtk_surface*);
void dred_gui_get_image_size_dtk(dtk_surface*, unsigned int* pWidthOut, unsigned int* pHeightOut);

dr_bool32 dred_gui_init_dtk(dred_gui* pGUI, dred_context* pDred)
{
    if (!dred_gui_init(pGUI, pDred)) {
        return DR_FALSE;
    }

    dred_gui_register_dtk_callbacks(pGUI);
    return DR_TRUE;
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
    callbacks.getFontSize                    = dred_gui_get_font_size_dtk;
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
    dtk_surface_draw_rect(pSurface, dtk_rect_init_dred(rect), dtk_color_init_dred(color));
}

void dred_control_draw_rect_outline_dtk(dred_rect rect, dtk_color color, float outlineWidth, dtk_surface* pSurface)
{
    dtk_surface_draw_rect_outline(pSurface, dtk_rect_init_dred(rect), dtk_color_init_dred(color), (dtk_int32)outlineWidth);
}

void dred_control_draw_rect_with_outline_dtk(dred_rect rect, dtk_color color, float outlineWidth, dtk_color outlineColor, dtk_surface* pSurface)
{
    dtk_surface_draw_rect_with_outline(pSurface, dtk_rect_init_dred(rect), dtk_color_init_dred(color), (dtk_int32)outlineWidth, dtk_color_init_dred(outlineColor));
}

void dred_control_draw_round_rect_dtk(dred_rect rect, dtk_color color, float radius, dtk_surface* pSurface)
{
    // NOTE: Rounded rectangles not currently supported.
    (void)radius;
    dtk_surface_draw_rect(pSurface, dtk_rect_init_dred(rect), dtk_color_init_dred(color));
}

void dred_control_draw_round_rect_outline_dtk(dred_rect rect, dtk_color color, float radius, float outlineWidth, dtk_surface* pSurface)
{
    // NOTE: Rounded rectangles not currently supported.
    (void)radius;
    dtk_surface_draw_rect_outline(pSurface, dtk_rect_init_dred(rect), dtk_color_init_dred(color), (dtk_int32)outlineWidth);
}

void dred_control_draw_round_rect_with_outline_dtk(dred_rect rect, dtk_color color, float radius, float outlineWidth, dtk_color outlineColor, dtk_surface* pSurface)
{
    // NOTE: Rounded rectangles not currently supported.
    (void)radius;
    dtk_surface_draw_rect_with_outline(pSurface, dtk_rect_init_dred(rect), dtk_color_init_dred(color), (dtk_int32)outlineWidth, dtk_color_init_dred(outlineColor));
}

void dred_control_draw_text_dtk(dtk_font* pFont, const char* text, int textSizeInBytes, float posX, float posY, dtk_color color, dtk_color backgroundColor, dtk_surface* pSurface)
{
    dtk_surface_draw_text(pSurface, pFont, 1, text, textSizeInBytes, (dtk_int32)posX, (dtk_int32)posY, dtk_color_init_dred(color), dtk_color_init_dred(backgroundColor));
}

void dred_control_draw_image_dtk(dtk_surface* pImage, dtk_draw_surface_args* pArgs, dtk_surface* pSurface)
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

dr_bool32 dred_gui_get_font_metrics_dtk(dtk_font* pFont, dtk_font_metrics* pMetricsOut)
{
    assert(pMetricsOut != NULL);
    return dtk_font_get_metrics(pFont, 1, pMetricsOut) == DTK_SUCCESS;
}

dr_bool32 dred_gui_get_glyph_metrics_dtk(dtk_font* pFont, unsigned int utf32, dtk_glyph_metrics* pMetricsOut)
{
    assert(pMetricsOut != NULL);
    return dtk_font_get_glyph_metrics(pFont, 1, utf32, pMetricsOut) == DTK_SUCCESS;
}

dr_bool32 dred_gui_measure_string_dtk(dtk_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut)
{
    return dtk_font_measure_string(pFont, 1, text, textSizeInBytes, pWidthOut, pHeightOut) == DTK_SUCCESS;
}

dr_bool32 dred_gui_get_text_cursor_position_from_point_dtk(dtk_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut)
{
    return dtk_font_get_text_cursor_position_from_point(pFont, 1, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosXOut, pCharacterIndexOut) == DTK_SUCCESS;
}

dr_bool32 dred_gui_get_text_cursor_position_from_char_dtk(dtk_font* pFont, const char* text, size_t characterIndex, float* pTextCursorPosXOut)
{
    return dtk_font_get_text_cursor_position_from_char(pFont, 1, text, characterIndex, pTextCursorPosXOut) == DTK_SUCCESS;
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

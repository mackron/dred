// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef struct
{
    dred_context* pDred;
} dred_control_data;

dred_control* dred_control_create(dred_context* pDred, dred_control* pParent, const char* type, size_t extraDataSize)
{
    drgui_element* pControl = drgui_create_element(pDred->pGUI, pParent, sizeof(dred_control_data) + extraDataSize, NULL);
    if (pControl == NULL) {
        return NULL;
    }

    dred_control_data* data = (dred_control_data*)drgui_get_extra_data(pControl);
    assert(data != NULL);

    data->pDred = pDred;


    drgui_set_type(pControl, type);
    return pControl;
}

void dred_control_delete(dred_control* pControl)
{
    if (pControl == NULL) {
        return;
    }

    drgui_delete_element(pControl);
}


dred_context* dred_control_get_context(dred_control* pControl)
{
    dred_control_data* data = (dred_control_data*)drgui_get_extra_data(pControl);
    assert(data != NULL);

    return data->pDred;
}

dred_control* dred_control_get_parent(dred_control* pControl)
{
    return pControl->pParent;
}


void* dred_control_get_extra_data(dred_control* pControl)
{
    return (void*)((uint8_t*)drgui_get_extra_data(pControl) + sizeof(dred_control_data));
}

size_t dred_control_get_extra_data_size(dred_control* pControl)
{
    return drgui_get_extra_data_size(pControl) - sizeof(dred_control_data);
}



void dred_control_hide(dred_control* pControl)
{
    drgui_hide(pControl);
}

void dred_control_show(dred_control* pControl)
{
    drgui_show(pControl);
}

bool dred_control_is_visible(const dred_control* pControl)
{
    return drgui_is_visible(pControl);
}

bool dred_control_is_visible_recursive(const dred_control* pControl)
{
    return drgui_is_visible_recursive(pControl);
}


bool dred_control_has_mouse_capture(dred_control* pControl)
{
    return drgui_has_mouse_capture(pControl);
}

bool dred_control_has_keyboard_capture(dred_control* pControl)
{
    return drgui_has_keyboard_capture(pControl);
}


void dred_control_detach(dred_control* pChildControl)
{
    drgui_detach(pChildControl);
}

void dred_control_append(dred_control* pChildControl, dred_control* pParentControl)
{
    drgui_append(pChildControl, pParentControl);
}

void dred_control_prepend(dred_control* pChildControl, dred_control* pParentControl)
{
    drgui_prepend(pChildControl, pParentControl);
}

void dred_control_append_sibling(dred_control* pControlToAppend, dred_control* pControlToAppendTo)
{
    drgui_append_sibling(pControlToAppend, pControlToAppendTo);
}

void dred_control_prepend_sibling(dred_control* pControlToPrepend, dred_control* pControlToPrependTo)
{
    drgui_prepend_sibling(pControlToPrepend, pControlToPrependTo);
}

dred_control* dred_control_find_top_level_element(dred_control* pControl)
{
    return drgui_find_top_level_element(pControl);
}

bool dred_control_is_parent(dred_control* pParentControl, dred_control* pChildControl)
{
    return drgui_is_parent(pParentControl, pChildControl);
}

bool dred_control_is_child(dred_control* pChildControl, dred_control* pParentControl)
{
    return drgui_is_child(pChildControl, pParentControl);
}

bool dred_control_is_ancestor(dred_control* pAncestorControl, dred_control* pChildControl)
{
    return drgui_is_ancestor(pAncestorControl, pChildControl);
}

bool dred_control_is_descendant(dred_control* pChildControl, dred_control* pAncestorControl)
{
    return drgui_is_descendant(pChildControl, pAncestorControl);
}

bool dred_control_is_self_or_ancestor(dred_control* pAncestorControl, dred_control* pChildControl)
{
    return drgui_is_self_or_ancestor(pAncestorControl, pChildControl);
}

bool dred_control_is_self_or_descendant(dred_control* pChildControl, dred_control* pAncestorControl)
{
    return drgui_is_self_or_descendant(pChildControl, pAncestorControl);
}




void dred_control_set_absolute_position(dred_control* pControl, float positionX, float positionY)
{
    drgui_set_absolute_position(pControl, positionX, positionY);
}

void dred_control_get_absolute_position(const dred_control* pControl, float* positionXOut, float* positionYOut)
{
    drgui_get_absolute_position(pControl, positionXOut, positionYOut);
}

float dred_control_get_absolute_position_x(const dred_control* pControl)
{
    return drgui_get_absolute_position_x(pControl);
}

float dred_control_get_absolute_position_y(const dred_control* pControl)
{
    return drgui_get_absolute_position_y(pControl);
}


void dred_control_set_relative_position(dred_control* pControl, float relativePosX, float relativePosY)
{
    drgui_set_relative_position(pControl, relativePosX, relativePosY);
}

void dred_control_get_relative_position(const dred_control* pControl, float* relativePosXOut, float* relativePosYOut)
{
    drgui_get_relative_position(pControl, relativePosXOut, relativePosYOut);
}

float dred_control_get_relative_position_x(const dred_control* pControl)
{
    return drgui_get_relative_position_x(pControl);
}

float dred_control_get_relative_position_y(const dred_control* pControl)
{
    return drgui_get_relative_position_y(pControl);
}


void dred_control_set_size(dred_control* pControl, float width, float height)
{
    drgui_set_size(pControl, width, height);
}

void dred_control_get_size(const dred_control* pControl, float* widthOut, float* heightOut)
{
    drgui_get_size(pControl, widthOut, heightOut);
}

float dred_control_get_width(const dred_control* pControl)
{
    return drgui_get_width(pControl);
}

float dred_control_get_height(const dred_control* pControl)
{
    return drgui_get_height(pControl);
}



void dred_control_set_on_move(dred_control* pControl, drgui_on_move_proc callback)
{
    drgui_set_on_move(pControl, callback);
}

void dred_control_set_on_size(dred_control* pControl, drgui_on_size_proc callback)
{
    drgui_set_on_size(pControl, callback);
}

void dred_control_set_on_mouse_enter(dred_control* pControl, drgui_on_mouse_enter_proc callback)
{
    drgui_set_on_mouse_enter(pControl, callback);
}

void dred_control_set_on_mouse_leave(dred_control* pControl, drgui_on_mouse_leave_proc callback)
{
    drgui_set_on_mouse_leave(pControl, callback);
}

void dred_control_set_on_mouse_move(dred_control* pControl, drgui_on_mouse_move_proc callback)
{
    drgui_set_on_mouse_move(pControl, callback);
}

void dred_control_set_on_mouse_button_down(dred_control* pControl, drgui_on_mouse_button_down_proc callback)
{
    drgui_set_on_mouse_button_down(pControl, callback);
}

void dred_control_set_on_mouse_button_up(dred_control* pControl, drgui_on_mouse_button_up_proc callback)
{
    drgui_set_on_mouse_button_up(pControl, callback);
}

void dred_control_set_on_mouse_button_dblclick(dred_control* pControl, drgui_on_mouse_button_dblclick_proc callback)
{
    drgui_set_on_mouse_button_dblclick(pControl, callback);
}

void dred_control_set_on_mouse_wheel(dred_control* pControl, drgui_on_mouse_wheel_proc callback)
{
    drgui_set_on_mouse_wheel(pControl, callback);
}

void dred_control_set_on_key_down(dred_control* pControl, drgui_on_key_down_proc callback)
{
    drgui_set_on_key_down(pControl, callback);
}

void dred_control_set_on_key_up(dred_control* pControl, drgui_on_key_up_proc callback)
{
    drgui_set_on_key_up(pControl, callback);
}

void dred_control_set_on_printable_key_down(dred_control* pControl, drgui_on_printable_key_down_proc callback)
{
    drgui_set_on_printable_key_down(pControl, callback);
}

void dred_control_set_on_paint(dred_control* pControl, drgui_on_paint_proc callback)
{
    drgui_set_on_paint(pControl, callback);
}

void dred_control_set_on_dirty(dred_control* pControl, drgui_on_dirty_proc callback)
{
    drgui_set_on_dirty(pControl, callback);
}

void dred_control_set_on_hittest(dred_control* pControl, drgui_on_hittest_proc callback)
{
    drgui_set_on_hittest(pControl, callback);
}

void dred_control_set_on_capture_mouse(dred_control* pControl, drgui_on_capture_mouse_proc callback)
{
    drgui_set_on_capture_mouse(pControl, callback);
}

void dred_control_set_on_release_mouse(dred_control* pControl, drgui_on_release_mouse_proc callback)
{
    drgui_set_on_release_mouse(pControl, callback);
}

void dred_control_set_on_capture_keyboard(dred_control* pControl, drgui_on_capture_keyboard_proc callback)
{
    drgui_set_on_capture_keyboard(pControl, callback);
}

void dred_control_set_on_release_keyboard(dred_control* pControl, drgui_on_release_keyboard_proc callback)
{
    drgui_set_on_release_keyboard(pControl, callback);
}



//// Misc stuff relating to controls ////

bool dred_control_is_of_type(dred_control* pControl, const char* type)
{
    return drgui_is_of_type(pControl, type);
}

bool dred_is_control_type_of_type(const char* type, const char* base)
{
    if (type == NULL || base == NULL) {
        return false;
    }

    return strncmp(type, base, strlen(base)) == 0;
}

void dred_control_show_popup_menu(dred_control* pControl, dred_menu* pMenu, int relativePosX, int relativePosY)
{
    if (pControl == NULL || pMenu == NULL) {
        return;
    }

    dred_window* pWindow = dred_get_element_window(pControl);
    if (pWindow == NULL) {
        return;
    }

    int mousePosXWindow = relativePosX + (int)drgui_get_absolute_position_x(pControl);
    int mousePosYWindow = relativePosY + (int)drgui_get_absolute_position_y(pControl);
    dred_window_show_popup_menu(pWindow, pMenu, mousePosXWindow, mousePosYWindow);
}
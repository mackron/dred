// Controls are the base GUI element used in dred. Whenever a GUI element is required, it'll be based
// off this object.
//
// A dred_control is a dred_control_element with dred-specific functionality.

typedef drgui_element dred_control;

// dred_control_create()
dred_control* dred_control_create(dred_context* pDred, dred_control* pParent, const char* typeName, size_t extraDataSize);

// dred_control_delete()
void dred_control_delete(dred_control* pControl);


// dred_control_get_context()
dred_context* dred_control_get_context(dred_control* pControl);

// dred_control_get_parent()
dred_control* dred_control_get_parent(dred_control* pControl);


// dred_control_get_data()
void* dred_control_get_data(dred_control* pControl);

// dred_control_get_data_size()
size_t dred_control_get_data_size(dred_control* pControl);



// Hides the given element.
void dred_control_hide(dred_control* pControl);

// Shows the given element.
void dred_control_show(dred_control* pControl);

// Determines whether or not the element is marked as visible.
//
// This is a direct accessor for the internal visible flag of the element and is not recursive. Thus, if this element is
// marked as visible, but it's parent is invisible, it will still return true. Use drgui_is_visible_recursive() to do
// a recursive visibility check.
bool dred_control_is_visible(const dred_control* pControl);

// Recursively determines whether or not the element is marked as visible.
bool dred_control_is_visible_recursive(const dred_control* pControl);



//// Hierarchy ////

void dred_control_detach(dred_control* pChildElement);
void dred_control_append(dred_control* pChildElement, dred_control* pParentElement);
void dred_control_prepend(dred_control* pChildElement, dred_control* pParentElement);
void dred_control_append_sibling(dred_control* pElementToAppend, dred_control* pElementToAppendTo);
void dred_control_prepend_sibling(dred_control* pElementToPrepend, dred_control* pElementToPrependTo);
dred_control* dred_control_find_top_level_element(dred_control* pElement);
bool dred_control_is_parent(dred_control* pParentElement, dred_control* pChildElement);
bool dred_control_is_child(dred_control* pChildElement, dred_control* pParentElement);
bool dred_control_is_ancestor(dred_control* pAncestorElement, dred_control* pChildElement);
bool dred_control_is_descendant(dred_control* pChildElement, dred_control* pAncestorElement);
bool dred_control_is_self_or_ancestor(dred_control* pAncestorElement, dred_control* pChildElement);
bool dred_control_is_self_or_descendant(dred_control* pChildElement, dred_control* pAncestorElement);



//// Layout ////

// dred_control_set_absolute_position()
void dred_control_set_absolute_position(dred_control* pControl, float positionX, float positionY);

// dred_control_get_absolute_position()
void dred_control_get_absolute_position(const dred_control* pControl, float* positionXOut, float* positionYOut);
float dred_control_get_absolute_position_x(const dred_control* pControl);
float dred_control_get_absolute_position_y(const dred_control* pControl);

// dred_control_set_relative_position()
void dred_control_set_relative_position(dred_control* pControl, float relativePosX, float relativePosY);

// dred_control_get_relative_position()
void dred_control_get_relative_position(const dred_control* pControl, float* relativePosXOut, float* relativePosYOut);
float dred_control_get_relative_position_x(const dred_control* pControl);
float dred_control_get_relative_position_y(const dred_control* pControl);


// dred_control_set_size()
void dred_control_set_size(dred_control* pControl, float width, float height);

// dred_control_get_size()
void dred_control_get_size(const dred_control* pControl, float* widthOut, float* heightOut);
float dred_control_get_width(const dred_control* pControl);
float dred_control_get_height(const dred_control* pControl);


// Event setters.
void dred_control_set_on_move(dred_control* pControl, drgui_on_move_proc callback);
void dred_control_set_on_size(dred_control* pControl, drgui_on_size_proc callback);
void dred_control_set_on_mouse_enter(dred_control* pControl, drgui_on_mouse_enter_proc callback);
void dred_control_set_on_mouse_leave(dred_control* pControl, drgui_on_mouse_leave_proc callback);
void dred_control_set_on_mouse_move(dred_control* pControl, drgui_on_mouse_move_proc callback);
void dred_control_set_on_mouse_button_down(dred_control* pControl, drgui_on_mouse_button_down_proc callback);
void dred_control_set_on_mouse_button_up(dred_control* pControl, drgui_on_mouse_button_up_proc callback);
void dred_control_set_on_mouse_button_dblclick(dred_control* pControl, drgui_on_mouse_button_dblclick_proc callback);
void dred_control_set_on_mouse_wheel(dred_control* pControl, drgui_on_mouse_wheel_proc callback);
void dred_control_set_on_key_down(dred_control* pControl, drgui_on_key_down_proc callback);
void dred_control_set_on_key_up(dred_control* pControl, drgui_on_key_up_proc callback);
void dred_control_set_on_printable_key_down(dred_control* pControl, drgui_on_printable_key_down_proc callback);
void dred_control_set_on_paint(dred_control* pControl, drgui_on_paint_proc callback);
void dred_control_set_on_dirty(dred_control* pControl, drgui_on_dirty_proc callback);
void dred_control_set_on_hittest(dred_control* pControl, drgui_on_hittest_proc callback);
void dred_control_set_on_capture_mouse(dred_control* pControl, drgui_on_capture_mouse_proc callback);
void dred_control_set_on_release_mouse(dred_control* pControl, drgui_on_release_mouse_proc callback);
void dred_control_set_on_capture_keyboard(dred_control* pControl, drgui_on_capture_keyboard_proc callback);
void dred_control_set_on_release_keyboard(dred_control* pControl, drgui_on_release_keyboard_proc callback);
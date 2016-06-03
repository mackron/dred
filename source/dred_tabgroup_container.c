
typedef struct
{
    dred_tabgroup_split_axis splitAxis;
} dred_tabgroup_container_data;

void dred_tabgroup_container__on_size(dred_tabgroup_container* pContainer, float newWidth, float newHeight)
{
    dred_tabgroup_container_data* data = (dred_tabgroup_container_data*)dred_control_get_data(pContainer);
    assert(data != NULL);

    if (data->splitAxis == dred_tabgroup_split_axis_none) {
        drgui_on_size_fit_children_to_parent(pContainer, newWidth, newHeight);  // <-- Need to rethink this when the resize bar is added. Maybe just make an assumption on the layout? Branch based on the types of each child?
    } else {
        // Reposition and size the two child panels depending on the split axis.
    }
}

dred_tabgroup_container* dred_tabgroup_container_create(dred_context* pDred, dred_control* pParent)
{
    dred_tabgroup* pContainer = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_TABGROUP_CONTAINER, sizeof(dred_tabgroup_container_data));
    if (pContainer == NULL) {
        return NULL;
    }

    dred_tabgroup_container_data* data = (dred_tabgroup_container_data*)dred_control_get_data(pContainer);
    assert(data != NULL);

    data->splitAxis = dred_tabgroup_split_axis_none;


    // Events.
    dred_control_set_on_size(pContainer, dred_tabgroup_container__on_size);

    return pContainer;
}

void dred_tabgroup_container_delete(dred_tabgroup_container* pContainer)
{
    if (pContainer == NULL) {
        return;
    }

    dred_tabgroup_container_data* data = (dred_tabgroup_container_data*)dred_control_get_data(pContainer);
    if (data != NULL) {
        // TODO: Delete children.
    }

    dred_control_delete(pContainer);
}

// Copyright (C) 2016 David Reid. See included LICENSE file.

void dred_tabgroup_container__on_size(dred_control* pControl, float newWidth, float newHeight)
{
    dred_tabgroup_container* pContainer = DRED_TABGROUP_CONTAINER(pControl);
    assert(pContainer != NULL);

    if (pContainer->splitAxis == dred_tabgroup_split_axis_none) {
        dred_control_on_size_fit_children_to_parent(pControl, newWidth, newHeight);  // <-- Need to rethink this when the resize bar is added. Maybe just make an assumption on the layout? Branch based on the types of each child?
    } else {
        // Reposition and size the two child panels depending on the split axis.
    }
}

dred_tabgroup_container* dred_tabgroup_container_create(dred_context* pDred, dred_control* pParent)
{
    dred_tabgroup_container* pContainer = (dred_tabgroup_container*)calloc(1, sizeof(*pContainer));
    if (pContainer == NULL) {
        return NULL;
    }

    if (!dred_control_init(DRED_CONTROL(pContainer), pDred, pParent, DRED_CONTROL_TYPE_TABGROUP_CONTAINER)) {
        free(pContainer);
        return NULL;
    }

    pContainer->splitAxis = dred_tabgroup_split_axis_none;


    // Events.
    dred_control_set_on_size(DRED_CONTROL(pContainer), dred_tabgroup_container__on_size);

    return pContainer;
}

void dred_tabgroup_container_delete(dred_tabgroup_container* pContainer)
{
    if (pContainer == NULL) {
        return;
    }

    dred_control_uninit(DRED_CONTROL(pContainer));
}

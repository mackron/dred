// Copyright (C) 2017 David Reid. See included LICENSE file.

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

dr_bool32 dred_tabgroup_container_init(dred_tabgroup_container* pContainer, dred_context* pDred, dred_control* pParent)
{
    if (pContainer == NULL) {
        return DR_FALSE;
    }

    memset(pContainer, 0, sizeof(*pContainer));

    if (!dred_control_init(DRED_CONTROL(pContainer), pDred, pParent, NULL, DRED_CONTROL_TYPE_TABGROUP_CONTAINER, NULL)) {
        return DR_FALSE;
    }

    pContainer->splitAxis = dred_tabgroup_split_axis_none;


    // Events.
    dred_control_set_on_size(DRED_CONTROL(pContainer), dred_tabgroup_container__on_size);

    return DR_TRUE;
}

void dred_tabgroup_container_uninit(dred_tabgroup_container* pContainer)
{
    if (pContainer == NULL) {
        return;
    }

    dred_control_uninit(DRED_CONTROL(pContainer));
}

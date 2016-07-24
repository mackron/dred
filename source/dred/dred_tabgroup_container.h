// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_TABGROUP_CONTAINER  "dred.tabgroup_container"

typedef dred_control dred_tabgroup_container;

typedef enum
{
    dred_tabgroup_split_axis_none,
    dred_tabgroup_split_axis_vertical,
    dred_tabgroup_split_axis_horizontal
} dred_tabgroup_split_axis;

// dred_tabgroup_container_create()
dred_tabgroup_container* dred_tabgroup_container_create(dred_context* pDred, dred_control* pParent);

// dred_tabgroup_container_delete()
void dred_tabgroup_container_delete(dred_tabgroup_container* pContainer);

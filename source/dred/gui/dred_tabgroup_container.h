// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_TABGROUP_CONTAINER  "dred.tabgroup_container"

typedef struct dred_tabgroup_container dred_tabgroup_container;
#define DRED_TABGROUP_CONTAINER(a) ((dred_tabgroup_container*)(a))

typedef enum
{
    dred_tabgroup_split_axis_none,
    dred_tabgroup_split_axis_vertical,
    dred_tabgroup_split_axis_horizontal
} dred_tabgroup_split_axis;

struct dred_tabgroup_container
{
    // The base control.
    dred_control control;

    dred_tabgroup_split_axis splitAxis;
};


// dred_tabgroup_container_create()
dr_bool32 dred_tabgroup_container_init(dred_tabgroup_container* pContainer, dred_context* pDred, dred_control* pParent);

// dred_tabgroup_container_delete()
void dred_tabgroup_container_uninit(dred_tabgroup_container* pContainer);

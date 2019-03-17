// Copyright (C) 2019 David Reid. See included LICENSE file.

//// Drag and Drop ////

typedef enum
{
    dred_data_type_generic,
    dred_data_type_text
} dred_data_type;

// Begins a drag and drop operation.
//
// This will make a copy of the data.
dtk_bool32 dred_begin_drag_and_drop(dred_data_type dataType, const void* pData, size_t dataSize);

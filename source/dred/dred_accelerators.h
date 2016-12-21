// Copyright (C) 2016 David Reid. See included LICENSE file.

#if 0
struct dred_accelerator_table
{
    dtk_accelerator* pAccelerators;
    size_t count;
    size_t bufferSize;
};

dr_bool32 dred_accelerator_table_init(dred_accelerator_table* pTable);
void dred_accelerator_table_uninit(dred_accelerator_table* pTable);

dr_bool32 dred_accelerator_table_add(dred_accelerator_table* pTable, dtk_accelerator accelerator);
dr_bool32 dred_accelerator_table_remove(dred_accelerator_table* pTable, dtk_accelerator accelerator);
dr_bool32 dred_accelerator_table_find(dred_accelerator_table* pTable, dtk_accelerator accelerator, size_t* pIndexOut);

#if 0
// Creates an empty accelerator object.
dred_accelerator dred_accelerator_none();
dred_accelerator dred_accelerator_create(dtk_key key, uint32_t modifiers);
dred_accelerator dred_accelerator_parse(const char* accelStr);
dr_bool32 dred_accelerator_equal(dred_accelerator a, dred_accelerator b);
size_t dred_accelerator_to_string(dred_accelerator accelerator, char* strOut, size_t strOutSize);
#endif

#endif

struct dred_accelerator
{
    drgui_key key;
    uint32_t modifiers;
};

struct dred_accelerator_table
{
    dred_accelerator* pAccelerators;
    size_t count;
    size_t bufferSize;
};

bool dred_accelerator_table_init(dred_accelerator_table* pTable);
void dred_accelerator_table_uninit(dred_accelerator_table* pTable);

bool dred_accelerator_table_bind(dred_accelerator_table* pTable, drgui_key key, uint32_t modifiers, const char* cmdStr);

bool dred_accelerator_table_find(dred_accelerator_table* pTable, drgui_key key, uint32_t modifiers, size_t* pIndexOut);
void dred_accelerator_table_replace(dred_accelerator_table* pTable, size_t acceleratorIndex, const char* cmdStr);
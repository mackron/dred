
struct dred_accelerator
{
    drgui_key key;
    uint32_t modifiers;
};

struct dred_accelerator_table
{
    dred_accelerator* pAccelerators;
    char** ppCmdStrings;
    size_t count;
    size_t bufferSize;
};

bool dred_accelerator_table_init(dred_accelerator_table* pTable);
void dred_accelerator_table_uninit(dred_accelerator_table* pTable);

bool dred_accelerator_table_bind(dred_accelerator_table* pTable, drgui_key key, uint32_t modifiers, const char* cmdStr);

bool dred_accelerator_table_find(dred_accelerator_table* pTable, drgui_key key, uint32_t modifiers, size_t* pIndexOut);
void dred_accelerator_table_replace(dred_accelerator_table* pTable, size_t acceleratorIndex, const char* cmdStr);

const char* dred_accelerator_table_get_command_string_by_index(dred_accelerator_table* pTable, size_t acceleratorIndex);

size_t dred_accelerator_to_string(dred_accelerator accelerator, char* strOut, size_t strOutSize);


// Creates an empty accelerator object.
dred_accelerator dred_accelerator_none();
dred_accelerator dred_accelerator_create(drgui_key key, uint32_t modifiers);
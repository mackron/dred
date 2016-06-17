
// BEGIN STOCK ACCELERATORS
//
// ACCELERATOR DRED_ACCELERATOR_ID_NEW      'N'     CTRL        "new"
// ACCELERATOR DRED_ACCELERATOR_ID_OPEN     'O'     CTRL        "open"
// ACCELERATOR DRED_ACCELERATOR_ID_SAVE     'S'     CTRL        "save"
// ACCELERATOR DRED_ACCELERATOR_ID_SAVE_AS  NONE    NONE        "save-as"
// ACCELERATOR DRED_ACCELERATOR_ID_SAVE_ALL 'S'     CTRL|SHIFT  "save-all"
//
// END STOCK ACCELERATORS

#define DRED_ACCELERATOR_ID_NONE        0
#define DRED_ACCELERATOR_ID_NEW         1
#define DRED_ACCELERATOR_ID_OPEN        2
#define DRED_ACCELERATOR_ID_SAVE        3
#define DRED_ACCELERATOR_ID_SAVE_AS     4

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




// Creates an empty accelerator object.
dred_accelerator dred_accelerator_none();
dred_accelerator dred_accelerator_create(drgui_key key, uint32_t modifiers);
bool dred_accelerator_equal(dred_accelerator a, dred_accelerator b);
size_t dred_accelerator_to_string(dred_accelerator accelerator, char* strOut, size_t strOutSize);
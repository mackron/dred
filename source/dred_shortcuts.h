
struct dred_shortcut
{
    dred_accelerator accelerators[2];
};

struct dred_shortcut_table
{
    dred_shortcut* pShortcuts;
    char** ppCmdStrings;
    size_t count;
    size_t bufferSize;
};

bool dred_shortcut_table_init(dred_shortcut_table* pTable);
void dred_shortcut_table_uninit(dred_shortcut_table* pTable);

bool dred_shortcut_table_bind(dred_shortcut_table* pTable, dred_shortcut shortcut, const char* cmdStr);

bool dred_shortcut_table_find(dred_shortcut_table* pTable, dred_shortcut shortcut, size_t* pIndexOut);
void dred_shortcut_table_replace(dred_shortcut_table* pTable, size_t shortcutIndex, const char* cmdStr);

const char* dred_shortcut_table_get_command_string_by_index(dred_shortcut_table* pTable, size_t shortcutIndex);




dred_shortcut dred_shortcut_create(dred_accelerator accel0, dred_accelerator accel1);
dred_shortcut dred_shortcut_create_single(dred_accelerator accel1);
dred_shortcut dred_shortcut_none();
bool dred_shortcut_equal(dred_shortcut a, dred_shortcut b);
size_t dred_shortcut_to_string(dred_shortcut shortcut, char* strOut, size_t strOutSize);
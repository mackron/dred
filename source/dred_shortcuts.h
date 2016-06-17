
// Stock shortcut names.
#define DRED_SHORTCUT_NAME_NEW          "New"
#define DRED_SHORTCUT_NAME_OPEN         "Open"
#define DRED_SHORTCUT_NAME_SAVE         "Save"
#define DRED_SHORTCUT_NAME_SAVE_ALL     "SaveAll"
#define DRED_SHORTCUT_NAME_CLOSE        "Close"
#define DRED_SHORTCUT_NAME_CLOSE_ALL    "CloseAll"
#define DRED_SHORTCUT_NAME_UNDO         "Undo"
#define DRED_SHORTCUT_NAME_REDO         "Redo"
#define DRED_SHORTCUT_NAME_CUT          "Cut"
#define DRED_SHORTCUT_NAME_COPY         "Copy"
#define DRED_SHORTCUT_NAME_PASTE        "Paste"
#define DRED_SHORTCUT_NAME_SELECT_ALL   "SelectAll"
#define DRED_SHORTCUT_NAME_GOTO         "GoTo"
#define DRED_SHORTCUT_NAME_FIND         "Find"
#define DRED_SHORTCUT_NAME_REPLACE      "FindAndReplace"

struct dred_shortcut
{
    dred_accelerator accelerators[2];
};

struct dred_shortcut_table
{
    dred_shortcut* pShortcuts;
    char** ppCmdStrings;
    char** ppNameStrings;
    size_t count;
    size_t bufferSize;

    // The table of accelerators used by every shortcut.
    dred_accelerator_table acceleratorTable;
};

bool dred_shortcut_table_init(dred_shortcut_table* pTable);
void dred_shortcut_table_uninit(dred_shortcut_table* pTable);

bool dred_shortcut_table_bind(dred_shortcut_table* pTable, const char* name, dred_shortcut shortcut, const char* cmdStr);
bool dred_shortcut_table_unbind(dred_shortcut_table* pTable, dred_shortcut shortcut);

bool dred_shortcut_table_find(dred_shortcut_table* pTable, dred_shortcut shortcut, size_t* pIndexOut);
bool dred_shortcut_table_find_by_name(dred_shortcut_table* pTable, const char* name, size_t* pIndexOut);
void dred_shortcut_table_replace(dred_shortcut_table* pTable, size_t shortcutIndex, const char* cmdStr);

bool dred_shortcut_table_get_shortcut_by_index(dred_shortcut_table* pTable, size_t shortcutIndex, dred_shortcut* pShortcutOut);
const char* dred_shortcut_table_get_command_string_by_index(dred_shortcut_table* pTable, size_t shortcutIndex);




dred_shortcut dred_shortcut_create(dred_accelerator accel0, dred_accelerator accel1);
dred_shortcut dred_shortcut_create_single(dred_accelerator accel1);
dred_shortcut dred_shortcut_none();
bool dred_shortcut_equal(dred_shortcut a, dred_shortcut b);
size_t dred_shortcut_to_string(dred_shortcut shortcut, char* strOut, size_t strOutSize);
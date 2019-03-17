// Copyright (C) 2019 David Reid. See included LICENSE file.

// Stock shortcut names.
/*
#define DRED_SHORTCUT_NAME_NEW          "New"
#define DRED_SHORTCUT_NAME_OPEN         "Open"
#define DRED_SHORTCUT_NAME_SAVE         "Save"
#define DRED_SHORTCUT_NAME_SAVE_ALL     "SaveAll"
#define DRED_SHORTCUT_NAME_CLOSE        "Close"
#define DRED_SHORTCUT_NAME_CLOSE_ALL    "CloseAll"
#define DRED_SHORTCUT_NAME_PRINT        "Print"
#define DRED_SHORTCUT_NAME_UNDO         "Undo"
#define DRED_SHORTCUT_NAME_REDO         "Redo"
#define DRED_SHORTCUT_NAME_CUT          "Cut"
#define DRED_SHORTCUT_NAME_COPY         "Copy"
#define DRED_SHORTCUT_NAME_PASTE        "Paste"
#define DRED_SHORTCUT_NAME_SELECT_ALL   "SelectAll"
#define DRED_SHORTCUT_NAME_UNINDENT     "Unindent"
#define DRED_SHORTCUT_NAME_GOTO         "GoTo"
#define DRED_SHORTCUT_NAME_FIND         "Find"
#define DRED_SHORTCUT_NAME_REPLACE      "FindAndReplace"
#define DRED_SHORTCUT_NAME_NEXT_TAB     "NextTab"
#define DRED_SHORTCUT_NAME_PREV_TAB     "PrevTab"
#define DRED_SHORTCUT_NAME_RELOAD       "Reload"
*/

struct dred_shortcut
{
    dtk_uint32 id;
    size_t nameOffset;  // Offset of the main string pool.
    size_t cmdOffset;   // Offset of the main string pool.
    dtk_uint32 acceleratorCount;
    dtk_accelerator accelerators[4];
};

struct dred_shortcut_table
{
    dred_context* pDred;
    dred_shortcut* pShortcuts;
    size_t count;
    size_t bufferSize;

    // The table of accelerators used by every shortcut.
    //dred_accelerator_table acceleratorTable;
};

dtk_bool32 dred_shortcut_table_init(dred_context* pDred, dred_shortcut_table* pTable, size_t initialCapacity);
void dred_shortcut_table_uninit(dred_shortcut_table* pTable);

dtk_bool32 dred_shortcut_table_bind(dred_shortcut_table* pTable, dtk_uint32 id, const char* name, const char* cmdStr, dtk_uint32 acceleratorCount, dtk_accelerator* pAccelerators);
dtk_bool32 dred_shortcut_table_unbind(dred_shortcut_table* pTable, dtk_uint32 id);

dtk_bool32 dred_shortcut_table_find(dred_shortcut_table* pTable, dtk_uint32 id, size_t* pIndexOut);
dtk_bool32 dred_shortcut_table_find_by_name(dred_shortcut_table* pTable, const char* name, size_t* pIndexOut);
void dred_shortcut_table_replace_by_index(dred_shortcut_table* pTable, size_t shortcutIndex, const char* name, const char* cmdStr, dtk_uint32 acceleratorCount, dtk_accelerator* pAccelerators);

dtk_bool32 dred_shortcut_table_get_shortcut_by_index(dred_shortcut_table* pTable, size_t shortcutIndex, dred_shortcut* pShortcutOut);
const char* dred_shortcut_table_get_command_string_by_index(dred_shortcut_table* pTable, size_t shortcutIndex);
const char* dred_shortcut_table_get_command_string_by_name(dred_shortcut_table* pTable, const char* name);




dred_shortcut dred_shortcut_create(dtk_accelerator accel0, dtk_accelerator accel1);
dred_shortcut dred_shortcut_create_single(dtk_accelerator accel1);
dred_shortcut dred_shortcut_none();
dred_shortcut dred_shortcut_parse(const char* shortcutStr);
dtk_bool32 dred_shortcut_equal(dred_shortcut a, dred_shortcut b);
size_t dred_shortcut_to_string(dred_shortcut shortcut, char* strOut, size_t strOutSize);
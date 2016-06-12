// Commands are not usable until they are added to the list below. The pre-build stage will look at this file and auto-generate an
// optimized data structure for finding and executing commands based on the list below.

// BEGIN COMMAND LIST : <command name> <proc> <flags>
//
// !            dred_command__system_command    DRED_CMDBAR_RELEASE_KEYBOARD
// new          dred_command__new               DRED_CMDBAR_RELEASE_KEYBOARD
// open         dred_command__open              DRED_CMDBAR_RELEASE_KEYBOARD
// save         dred_command__save              DRED_CMDBAR_RELEASE_KEYBOARD
// save-all     dred_command__save_all          DRED_CMDBAR_RELEASE_KEYBOARD
// close        dred_command__close             DRED_CMDBAR_RELEASE_KEYBOARD
// close-all    dred_command__close_all         DRED_CMDBAR_RELEASE_KEYBOARD
// undo         dred_command__undo              DRED_CMDBAR_NO_CLEAR
// redo         dred_command__redo              DRED_CMDBAR_NO_CLEAR
// goto         dred_command__goto              DRED_CMDBAR_RELEASE_KEYBOARD
// find         dred_command__find              DRED_CMDBAR_RELEASE_KEYBOARD
// find-next    dred_command__find_next         DRED_CMDBAR_NO_CLEAR
// replace      dred_command__replace           DRED_CMDBAR_RELEASE_KEYBOARD
// replace-next dred_command__replace_next      DRED_CMDBAR_NO_CLEAR
// replace-all  dred_command__replace_all       DRED_CMDBAR_RELEASE_KEYBOARD
//
// END COMMAND LIST

#define DRED_CMDBAR_RELEASE_KEYBOARD    1   // Releases the keyboard if the command was executed from the command bar.
#define DRED_CMDBAR_NO_CLEAR            2   // Prevents the command bar from being cleared if the command was executed from the command bar.

typedef void (* dred_command_proc) (dred_context* pDred, const char* value);

typedef struct
{
    dred_command_proc proc;
    unsigned int flags;
} dred_command;


//// General ////

// Handles the "!" command.
void dred_command__system_command(dred_context* pDred, const char* value);

// new
void dred_command__new(dred_context* pDred, const char* value);

// open
void dred_command__open(dred_context* pDred, const char* value);

// save
void dred_command__save(dred_context* pDred, const char* value);

// save-all
void dred_command__save_all(dred_context* pDred, const char* value);

// close
void dred_command__close(dred_context* pDred, const char* value);

// close-all
void dred_command__close_all(dred_context* pDred, const char* value);



//// General Editing ////

// undo
void dred_command__undo(dred_context* pDred, const char* value);

// redo
void dred_command__redo(dred_context* pDred, const char* value);



//// Text Editor ////

// goto
void dred_command__goto(dred_context* pDred, const char* value);

// find
void dred_command__find(dred_context* pDred, const char* value);

// find-next
void dred_command__find_next(dred_context* pDred, const char* value);

// replace
void dred_command__replace(dred_context* pDred, const char* value);

// replace-next
void dred_command__replace_next(dred_context* pDred, const char* value);

// replace-all
void dred_command__replace_all(dred_context* pDred, const char* value);




// Finds the index of a command.
bool dred_find_command(const char* cmdStr, dred_command* pCommandOut, const char** pValueOut);

// Finds the index of the given command function. Returns -1 if the command could not be found.
size_t dred_find_command_index(const char* cmdFunc);
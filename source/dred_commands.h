// Commands are not usable until they are added to the list below. The pre-build stage will look at this file and auto-generate an
// optimized data structure for finding and executing commands based on the list below.

// BEGIN COMMAND LIST : <command name> <proc> <flags>
//
// !            dred_command__platform_command  DRED_CMDBAR_RELEASE_KEYBOARD
// find         dred_command__find              DRED_CMDBAR_RELEASE_KEYBOARD
// find-next    dred_command__find_next         DRED_CMDBAR_NO_CLEAR
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
void dred_command__platform_command(dred_context* pDred, const char* value);


//// Text Editor ////

// find
void dred_command__find(dred_context* pDred, const char* value);

// find-next
void dred_command__find_next(dred_context* pDred, const char* value);




// Finds the index of a command.
bool dred_find_command(const char* cmdStr, dred_command* pCommandOut, const char** pValueOut);

// Finds the index of the given command function. Returns -1 if the command could not be found.
size_t dred_find_command_index(const char* cmdFunc);
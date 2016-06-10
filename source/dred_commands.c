
void dred_command__system_command(dred_context* pDred, const char* value)
{
    printf("exec: ! %s\n", value);
}


void dred_command__new(dred_context* pDred, const char* value)
{
    printf("exec: new %s\n", value);
}

void dred_command__open(dred_context* pDred, const char* value)
{
    printf("exec: open %s\n", value);

    char fileName[DRED_MAX_PATH];
    if (dr_next_token(value, fileName, sizeof(fileName)) == NULL) {
        dred_show_open_file_dialog(pDred);
    } else {
        dred_open_file(pDred, fileName);
    }
}

void dred_command__save(dred_context* pDred, const char* value)
{
    printf("exec: save %s\n", value);
}

void dred_command__save_all(dred_context* pDred, const char* value)
{
    printf("exec: save-all %s\n", value);
}

void dred_command__close(dred_context* pDred, const char* value)
{
    printf("exec: close %s\n", value);
}

void dred_command__close_all(dred_context* pDred, const char* value)
{
    printf("exec: close-all %s\n", value);
}


void dred_command__goto(dred_context* pDred, const char* value)
{
    printf("exec: goto %s\n", value);
}

void dred_command__find(dred_context* pDred, const char* value)
{
    printf("exec: find %s\n", value);
}

void dred_command__find_next(dred_context* pDred, const char* value)
{
    printf("exec: find-next %s\n", value);
}

void dred_command__replace(dred_context* pDred, const char* value)
{
    printf("exec: replace %s\n", value);
}

void dred_command__replace_next(dred_context* pDred, const char* value)
{
    printf("exec: replace-next %s\n", value);
}





bool dred_find_command(const char* cmdStr, dred_command* pCommandOut, const char** pValueOut)
{
    if (cmdStr == NULL || pCommandOut == NULL || pValueOut == NULL) {
        return false;
    }

    // Special case for "!".
    if (cmdStr[0] == '!') {
        *pCommandOut = g_Commands[0];
        *pValueOut   = dr_first_non_whitespace(cmdStr + 1);
        return true;
    }


    // For every other command the value will come after the first whitespace.
    char func[256];
    cmdStr = dr_next_token(cmdStr, func, sizeof(func));
    if (cmdStr == NULL) {
        return false;
    }

    size_t index = dred_find_command_index(func);
    if (index == (size_t)-1) {
        return false;
    }

    *pCommandOut = g_Commands[index];
    *pValueOut = dr_first_non_whitespace(cmdStr);
    return true;
}

size_t dred_find_command_index(const char* cmdFunc)
{
    // The command names are stored in a single pool of memory.
    if (cmdFunc[0] == '!') {
        return 0;
    }

    for (size_t i = 0; i < DRED_COMMAND_COUNT; ++i) {
        if (strcmp(cmdFunc, g_CommandNames[i]) == 0) {
            return i;
        }
    }

    return (size_t)-1;
}
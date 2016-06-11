
static bool dred__preprocess_system_command(dred_context* pDred, const char* cmd, char* cmdOut, size_t cmdOutSize)
{
    // Currently, the only symbols we're expanding is the enescaped "%" character, which is expanded to the relative
    // path of the currently focused file.
    dred_editor* pEditor = dred_get_focused_editor(pDred);
    if (pEditor == NULL) {
        return false;
    }

    const char* absolutePath = dred_editor_get_file_path(pEditor);
    if (absolutePath == NULL) {
        return false;
    }

    char currentDir[DRED_MAX_PATH];

    char relativePath[DRED_MAX_PATH];
    if (!drpath_to_relative(absolutePath, dr_get_current_directory(currentDir, sizeof(currentDir)), relativePath, sizeof(relativePath))) {
        return false;
    }

    size_t relativePathLength = strlen(relativePath);


    char prevC = '\0';
    while (cmdOutSize > 0 && *cmd != '\0') {
        if (*cmd == '%' && prevC != '\\') {
            if (strncpy_s(cmdOut, cmdOutSize, relativePath, relativePathLength) != 0) {
                return false;
            }

            cmdOut += relativePathLength;
            cmdOutSize -= relativePathLength;
        } else {
            *cmdOut = *cmd;
            cmdOut += 1;
            cmdOutSize -= 1;
        }

        prevC = *cmd;
        cmd += 1;
    }

    *cmdOut = '\0';
    return true;
}

void dred_command__system_command(dred_context* pDred, const char* value)
{
    (void)pDred;

    char valueExpanded[4096];
    if (dred__preprocess_system_command(pDred, value, valueExpanded, sizeof(valueExpanded))) {
        system(valueExpanded);
    } else {
        system(value);
    }
}


void dred_command__new(dred_context* pDred, const char* value)
{
    (void)pDred;
    (void)value;
}

void dred_command__open(dred_context* pDred, const char* value)
{
    char fileName[DRED_MAX_PATH];
    if (dr_next_token(value, fileName, sizeof(fileName)) == NULL) {
        dred_show_open_file_dialog(pDred);
    } else {
        dred_open_file(pDred, fileName);
    }
}

void dred_command__save(dred_context* pDred, const char* value)
{
    (void)pDred;
    (void)value;
}

void dred_command__save_all(dred_context* pDred, const char* value)
{
    (void)pDred;
    (void)value;
}

void dred_command__close(dred_context* pDred, const char* value)
{
    (void)value;

    dred_close_focused_file(pDred);
}

void dred_command__close_all(dred_context* pDred, const char* value)
{
    (void)value;

    dred_close_all_tabs(pDred);
}


void dred_command__goto(dred_context* pDred, const char* value)
{
    (void)pDred;
    (void)value;
}

void dred_command__find(dred_context* pDred, const char* value)
{
    (void)pDred;
    (void)value;
}

void dred_command__find_next(dred_context* pDred, const char* value)
{
    (void)pDred;
    (void)value;
}

void dred_command__replace(dred_context* pDred, const char* value)
{
    (void)pDred;
    (void)value;
}

void dred_command__replace_next(dred_context* pDred, const char* value)
{
    (void)pDred;
    (void)value;
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
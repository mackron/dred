// Copyright (C) 2019 David Reid. See included LICENSE file.

typedef struct
{
    // argv style.
    int argc;
    char** argv;

    // Win32 style
    const char* win32;
} dtk_cmdline;

typedef dtk_bool32 dtk_cmdline_parse_proc(const char* key, const char* value, void* pUserData);

typedef struct
{
    dtk_cmdline* pCmdLine;
    char* value;

    // Win32 style data.
    char* win32_payload;
    char* valueEnd;

    // argv style data.
    int iarg;   // <-- This starts at -1 so that the first call to next() increments it to 0.
} dtk_cmdline_iterator;

dtk_cmdline_iterator dtk_cmdline_begin(dtk_cmdline* pCmdLine)
{
    dtk_cmdline_iterator i;
    i.pCmdLine      = pCmdLine;
    i.value         = NULL;
    i.win32_payload = NULL;
    i.valueEnd      = NULL;
    i.iarg          = -1;

    if (pCmdLine != NULL && pCmdLine->win32 != NULL) {
        // Win32 style
        size_t length = strlen(pCmdLine->win32);
        i.win32_payload = (char*)malloc(length + 2);         // +2 for a double null terminator.
        dtk_strcpy_s(i.win32_payload, length + 2, pCmdLine->win32);
        i.win32_payload[length + 1] = '\0';

        i.valueEnd = i.win32_payload;
    }

    return i;
}

dtk_bool32 dtk_cmdline_next(dtk_cmdline_iterator* i)
{
    if (i != NULL && i->pCmdLine != NULL) {
        if (i->pCmdLine->win32 != NULL) {
            // Win32 style
            if (i->value == NULL) {
                i->value    = i->win32_payload;
                i->valueEnd = i->value;
            } else {
                i->value = i->valueEnd + 1;
            }


            // Move to the start of the next argument.
            while (i->value[0] == ' ') {
                i->value += 1;
            }


            // If at this point we are sitting on the null terminator it means we have finished iterating.
            if (i->value[0] == '\0')
            {
                free(i->win32_payload);
                i->win32_payload = NULL;
                i->pCmdLine      = NULL;
                i->value         = NULL;
                i->valueEnd      = NULL;

                return DTK_FALSE;
            }


            // Move to the end of the token. If the argument begins with a double quote, we iterate until we find
            // the next unescaped double-quote.
            if (i->value[0] == '\"') {
                // Go to the last unescaped double-quote.
                i->value += 1;
                i->valueEnd = i->value + 1;

                while (i->valueEnd[0] != '\0' && i->valueEnd[0] != '\"') {
                    if (i->valueEnd[0] == '\\') {
                        i->valueEnd += 1;

                        if (i->valueEnd[0] == '\0') {
                            break;
                        }
                    }

                    i->valueEnd += 1;
                }
                i->valueEnd[0] = '\0';
            } else {
                // Go to the next space.
                i->valueEnd = i->value + 1;

                while (i->valueEnd[0] != '\0' && i->valueEnd[0] != ' ') {
                    i->valueEnd += 1;
                }
                i->valueEnd[0] = '\0';
            }

            return DTK_TRUE;
        } else {
            // argv style
            i->iarg += 1;
            if (i->iarg < i->pCmdLine->argc) {
                i->value = i->pCmdLine->argv[i->iarg];
                return DTK_TRUE;
            } else {
                i->value = NULL;
                return DTK_FALSE;
            }
        }
    }

    return DTK_FALSE;
}


dtk_bool32 dtk_cmdline_init(int argc, char** argv, dtk_cmdline* pCmdLine)
{
    if (pCmdLine == NULL) {
        return DTK_FALSE;
    }

    pCmdLine->argc  = argc;
    pCmdLine->argv  = argv;
    pCmdLine->win32 = NULL;

    return DTK_TRUE;
}

dtk_bool32 dtk_cmdline_init_winmain(const char* args, dtk_cmdline* pCmdLine)
{
    if (pCmdLine == NULL) {
        return DTK_FALSE;
    }

    pCmdLine->argc  = 0;
    pCmdLine->argv  = NULL;
    pCmdLine->win32 = args;

    return DTK_TRUE;
}

void dtk_parse_cmdline(dtk_cmdline* pCmdLine, dtk_cmdline_parse_proc callback, void* pUserData)
{
    if (pCmdLine == NULL || callback == NULL) {
        return;
    }


    char pTemp[2] = {0};

    char* pKey = NULL;
    char* pVal = NULL;

    dtk_cmdline_iterator arg = dtk_cmdline_begin(pCmdLine);
    if (dtk_cmdline_next(&arg)) {
        if (!callback("[path]", arg.value, pUserData)) {
            return;
        }
    }

    while (dtk_cmdline_next(&arg)) {
        if (arg.value[0] == '-') {
            // key

            // If the key is non-null, but the value IS null, it means we hit a key with no value in which case it will not yet have been posted.
            if (pKey != NULL && pVal == NULL) {
                if (!callback(pKey, pVal, pUserData)) {
                    return;
                }

                pKey = NULL;
            } else {
                // Need to ensure the key and value are reset before doing any further processing.
                pKey = NULL;
                pVal = NULL;
            }


            if (arg.value[1] == '-') {
                // --argument style
                pKey = arg.value + 2;
            } else {
                // -a -b -c -d or -abcd style
                if (arg.value[1] != '\0') {
                    if (arg.value[2] == '\0') {
                        // -a -b -c -d style
                        pTemp[0] = arg.value[1];
                        pKey = pTemp;
                        pVal = NULL;
                    } else {
                        // -abcd style.
                        int i = 1;
                        while (arg.value[i] != '\0') {
                            pTemp[0] = arg.value[i];

                            if (!callback(pTemp, NULL, pUserData)) {
                                return;
                            }

                            pKey = NULL;
                            pVal = NULL;

                            i += 1;
                        }
                    }
                }
            }
        } else {
            // value

            pVal = arg.value;
            if (!callback(pKey, pVal, pUserData)) {
                return;
            }
        }
    }

    // There may be a key without a value that needs posting.
    if (pKey != NULL && pVal == NULL) {
        callback(pKey, pVal, pUserData);
    }
}

int dtk_cmdline_to_argv(dtk_cmdline* pCmdLine, char*** argvOut)
{
    if (argvOut == NULL) return 0;
    *argvOut = NULL;    // Safety.

    int argc = 0;
    char** argv = NULL;
    size_t cmdlineLen = 0;

    // The command line is parsed in 2 passes. The first pass simple calculates the required sizes of each buffer. The second
    // pass fills those buffers with actual data.

    // First pass.
    dtk_cmdline_iterator arg = dtk_cmdline_begin(pCmdLine);
    while (dtk_cmdline_next(&arg)) {
        cmdlineLen += strlen(arg.value) + 1;    // +1 for null terminator.
        argc += 1;
    }

    if (argc == 0) {
        return 0;
    }


    // The entire data for the command line is stored in a single buffer.
    char* data = (char*)dtk_malloc((argc * sizeof(char**)) + (cmdlineLen * sizeof(char)));
    if (data == NULL) {
        return 0;   // Ran out of memory.
    }

    argv = (char**)data;
    char* cmdlineStr = data + (argc * sizeof(char**));
    


    // Second pass.
    argc = 0;
    cmdlineLen = 0;

    arg = dtk_cmdline_begin(pCmdLine);
    while (dtk_cmdline_next(&arg)) {
        argv[argc] = cmdlineStr + cmdlineLen;
        
        int i = 0;
        while (arg.value[i] != '\0') {
            argv[argc][i] = arg.value[i];
            i += 1;
        }
        argv[argc][i] = '\0';


        cmdlineLen += strlen(arg.value) + 1;    // +1 for null terminator.
        argc += 1;
    }


    *argvOut = argv;
    return argc;
}

int dtk_winmain_to_argv(const char* cmdlineWinMain, char*** argvOut)
{
    dtk_cmdline cmdline;
    if (!dtk_cmdline_init_winmain(cmdlineWinMain, &cmdline)) {
        return 0;
    }

    return dtk_cmdline_to_argv(&cmdline, argvOut);
}

void dtk_free_argv(char** argv)
{
    if (argv == NULL) {
        return;
    }

    dtk_free(argv);
}

void dtk_argv_parse(int argc, char** argv, dtk_argv_parse_proc callback, void* pUserData)
{
    dtk_cmdline cmdline;
    if (!dtk_cmdline_init(argc, argv, &cmdline)) {
        return;
    }

    dtk_parse_cmdline(&cmdline, callback, pUserData);
}


typedef struct
{
    const char* key;
    dtk_bool32 exists;
} dtk_argv_exists_data;

dtk_bool32 dtk_argv_exists_callback(const char* key, const char* value, void* pUserData)
{
    (void)value;

    dtk_argv_exists_data* pData = (dtk_argv_exists_data*)pUserData;
    assert(pData != NULL);

    if (key != NULL && strcmp(pData->key, key) == 0) {
        pData->exists = DTK_TRUE;
        return DTK_FALSE;
    }

    return DTK_TRUE;
}

dtk_bool32 dtk_argv_exists(int argc, char** argv, const char* key)
{
    dtk_cmdline cmdline;
    dtk_cmdline_init(argc, argv, &cmdline);

    dtk_argv_exists_data data;
    data.key = key;
    data.exists = DTK_FALSE;
    dtk_parse_cmdline(&cmdline, dtk_argv_exists_callback, &data);

    return data.exists;
}


typedef struct
{
    const char* key;
    const char* value;
} dtk_argv_get_value_data;

dtk_bool32 dtk_argv_get_value_callback(const char* key, const char* value, void* pUserData)
{
    (void)value;

    dtk_argv_get_value_data* pData = (dtk_argv_get_value_data*)pUserData;
    assert(pData != NULL);

    if (key != NULL && strcmp(pData->key, key) == 0) {
        pData->value = value;
        return DTK_FALSE;
    }

    return DTK_TRUE;
}

const char* dtk_argv_get_value(int argc, char** argv, const char* key)
{
    dtk_cmdline cmdline;
    dtk_cmdline_init(argc, argv, &cmdline);

    dtk_argv_get_value_data data;
    data.key = key;
    data.value = NULL;
    dtk_parse_cmdline(&cmdline, dtk_argv_get_value_callback, &data);

    return data.value;
}

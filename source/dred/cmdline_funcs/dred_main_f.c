// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef int (* dred_cmdline_func_proc)(int argc, char** argv);

typedef struct
{
    const char* name;
    dred_cmdline_func_proc func;
} dred_cmdline_func_mapping;

static dred_cmdline_func_mapping g_BuiltInCmdLineFuncs[] = {
    {"file2chex",    dred_file2chex},
    {"file2cstring", dred_file2cstring}
};


int dred_main_f_exec(int argc, char** argv)
{
    // The first argument should be the name of the function. If it's a built-in function we just
    // handle it directly. Otherwise we need to check extensions.
    assert(argc > 0);

    char* funcName = argv[0];

    size_t count = sizeof(g_BuiltInCmdLineFuncs) / sizeof(g_BuiltInCmdLineFuncs[0]);
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(g_BuiltInCmdLineFuncs[i].name, funcName) == 0) {
            return g_BuiltInCmdLineFuncs[i].func(argc, argv);
        }
    }


    // If we get here it means the function is not built in and we need to check extensions.
    //
    // TODO: Implement extensions.
    return -4;
}

int dred_main_f(int argc, char** argv)
{
    // This function will be called when the -f command line option is specified. This option is used
    // for executing a command-line function.

    if (argc < 3) {
        return -2;  // Not enough arguments on the command line.
    }

    // The command line we pass to the handler needs to start at the arguments just after the "-f".
    int f_index;
    for (f_index = 0; f_index < argc; ++f_index) {
        if (strcmp(argv[f_index], "-f") == 0) {
            f_index += 1;
            break;
        }
    }

    if (f_index == argc) {
        return -3;  // Couldn't find the "-f" argument.
    }

    return dred_main_f_exec(argc - f_index, argv + f_index);
}


typedef int (* dred_cmdline_func_proc)(int argc, char** argv);

typedef struct
{
    const char* name;
    dred_cmdline_func_proc func;
} dred_cmdline_func_mapping;

static dred_cmdline_func_mapping g_BuiltInCmdLineFuncs[] = {
    {"file2chex", dred_file2chex}
};


int dred_main_f_argv(int argc, char** argv)
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

int dred_main_f(dr_cmdline cmdline)
{
    // This function will be called when the -f command line option is specified. This option is used
    // for executing a command-line function. It is possible for extensions to implemente custom
    // command line functions, so to make it easier for them we want to convert the command line to
    // argc/argv style, starting from the name of the function.

    char** argv;
    int argc = dr_cmdline_to_argv(&cmdline, &argv);
    if (argc == 0) {
        return -1;  // There was an error parsing the command line.
    }

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


    int result = dred_main_f_argv(argc - f_index, argv + f_index);


    // Free the command line arguments that were created with dr_cmdline_to_argv().
    dr_free_argv(argv);
    return result;
}

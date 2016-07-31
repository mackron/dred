// Copyright (C) 2016 David Reid. See included LICENSE file.

// Command: dred -f file2chex [Input File Name] [Options]
//    -n OutputVariableName : Specifies the name the name of the output variable. Defaults to "FileData"
//
// Implementation: dred_file2chex

// dred -f file2chex
int dred_file2chex(int argc, char** argv)
{
    for (int i = 0; i < argc; ++i) {
        printf("Command Line: %s\n", argv[i]);
    }

    return -1;
}

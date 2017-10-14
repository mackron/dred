// Copyright (C) 2017 David Reid. See included LICENSE file.

// BUILDING (WIN32)
//
// Libraries:
// - gdi32
// - comdlg32
//
// Notes:
// - When compiling with MSVC, make sure the [Manifest Tool -> Input and Output -> Embed Manifest] setting is set to "No". Not doing
//   so will result in a link error about it already being defined (it's defined manually in dred.rc).
// - Make sure resources/win32/dred.rc is included with the VC project and compiled with it.

// BUILDING (LINUX)
//
// cc source/dred/dred_main.c -o dred `pkg-config --cflags --libs gtk+-3.0` -lm -ldl

#include "dred.c"

dtk_bool32 dred_parse_cmdline__post_startup_files_to_server(const char* key, const char* value, void* pUserData)
{
    dtk_pipe client = (dtk_pipe)pUserData;

    if (key == NULL) {
        dred_ipc_post_message(client, DRED_IPC_MESSAGE_OPEN, value, strlen(value)+1);   // +1 for null terminator.
        return DTK_TRUE;
    }

    return DTK_TRUE;
}

dr_bool32 dred__try_opening_existing_process(int argc, char** argv)
{
    char pipeName[256];
    if (!dred_ipc_get_pipe_name(pipeName, sizeof(pipeName))) {
        return DR_FALSE;
    }

    dtk_pipe client;
    if (dtk_pipe_open_named_client(pipeName, DTK_IPC_WRITE, &client) == DTK_SUCCESS) {
        // If we get here it means there is a server instance already open and we want to use that one instead
        // of creating a new one. The first thing to do is notify the server that it should be activated.
        dred_ipc_post_message(client, DRED_IPC_MESSAGE_ACTIVATE, NULL, 0);

        // After activating the server we need to let it know which files to open.
        dtk_argv_parse(argc, argv, dred_parse_cmdline__post_startup_files_to_server, client);

        // The server should be notified of the file, so we just need to return now.
        dred_ipc_post_message(client, DRED_IPC_MESSAGE_TERMINATOR, NULL, 0);
        dtk_pipe_close(client);

        return DR_TRUE;
    }

    return DR_FALSE;
}


int main(int argc, char** argv)
{
    // Packages need to be loaded first.
    dred_package_library packages;
    dred_package_library_init(&packages);

    // Go down a different branch for command-line functions.
    if (dtk_argv_exists(argc, argv, "f")) {
        return dred_main_f(argc, argv);    // <-- Implemented in cmdline_funcs/dred_main_f.c
    }


    dr_bool32 tryUsingExistingInstance = DR_TRUE;
    if (dtk_argv_exists(argc, argv, "newinstance")) {
        tryUsingExistingInstance = DR_FALSE;
    }

    dr_bool32 disableIPC = DR_FALSE;
    if (dtk_argv_exists(argc, argv, "noipc")) {
        disableIPC = DR_TRUE;
    }

#ifndef _WIN32
    char lockFileName[256];
    dred_ipc_get_lock_name(lockFileName, sizeof(lockFileName));

    char pipeName[256];
    dred_ipc_get_pipe_name(pipeName, sizeof(pipeName));

    char pipeFileName[256];
    dtk_pipe_get_translated_name(pipeName, pipeFileName, sizeof(pipeFileName));

    int lockfd = -1;
#endif

    // If an instance of dred is already running, we may want to use that one instead. We can know this by trying
    // to create a client-side pipe.
    if (tryUsingExistingInstance) {
#ifdef _WIN32
        if (dred__try_opening_existing_process(argc, argv)) {
            return 0;
        }
#else
        dr_bool32 isOtherProcessRunning = DR_FALSE;
        int fd = open(lockFileName, O_RDONLY, 0666);
        if (fd != -1) {
            if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
                isOtherProcessRunning = DR_TRUE;
            } else {
                isOtherProcessRunning = DR_FALSE;
            }

            close(fd);

            // If the other process isn't running, remove both the lock and pipe files.
            if (!isOtherProcessRunning) {
                remove(lockFileName);
                remove(pipeFileName);
            }
        }

        if (isOtherProcessRunning) {
            if (dred__try_opening_existing_process(argc, argv)) {
                return 0;
            }
        } else {
            if (!disableIPC) {
                lockfd = open(lockFileName, O_WRONLY | O_CREAT, 0666);
                if (lockfd != -1) {
                    flock(lockfd, LOCK_EX);
                }
            }
        }
#endif
    }

    dred_context dred;
    if (!dred_init(&dred, argc, argv, &packages)) {
        return -1;
    }

    int result = dred_run(&dred);

#ifndef _WIN32
    if (lockfd != -1) {
        close(lockfd);
        remove(lockFileName);
    }
#endif

    dred_uninit(&dred);
    return result;
}

#ifdef _WIN32
#ifdef NDEBUG
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    char** argv;
    int argc = dtk_winmain_to_argv(lpCmdLine, &argv);

    int result = main(argc, argv);

    dtk_free_argv(argv);
    return result;
}
#endif
#endif

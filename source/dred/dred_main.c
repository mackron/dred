// Copyright (C) 2016 David Reid. See included LICENSE file.

// BUILDING (WIN32)
//
// Libraries:
// - gdi32
// - msimg32
// - comctl32
// - comdlg32
// - ole32
//
// Notes:
// - When compiling with MSVC, make sure the [Manifest Tool -> Input and Output -> Embed Manifest] setting is set to "No". Not doing
//   so will result in a link error about it already being defined (we defined it manually in dred.rc).
// - Make sure source/dred/win32/dred.rc is included with the VC project and compiled with it.

// BUILDING (LINUX)
//
// `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`
// -lpthread
// -lm
// -lrt

#include "dred.c"

bool dred_parse_cmdline__post_startup_files_to_server(const char* key, const char* value, void* pUserData)
{
    drpipe client = (drpipe)pUserData;

    if (key == NULL) {
        dred_ipc_post_message(client, DRED_IPC_MESSAGE_OPEN, value, strlen(value)+1);   // +1 for null terminator.
        return true;
    }

    return true;
}

bool dred__try_opening_existing_process(dr_cmdline cmdline)
{
    char pipeName[256];
    if (!dred_ipc_get_pipe_name(pipeName, sizeof(pipeName))) {
        return false;
    }

    drpipe client;
    if (drpipe_open_named_client(pipeName, DR_IPC_WRITE, &client) == dripc_result_success) {
        // If we get here it means there is a server instance already open and we want to use that one instead
        // of creating a new one. The first thing to do is notify the server that it should be activated.
        dred_ipc_post_message(client, DRED_IPC_MESSAGE_ACTIVATE, NULL, 0);

        // After activating the server we need to let it know which files to open.
        dr_parse_cmdline(&cmdline, dred_parse_cmdline__post_startup_files_to_server, client);

        // The server should be notified of the file, so we just need to return now.
        dred_ipc_post_message(client, DRED_IPC_MESSAGE_TERMINATOR, NULL, 0);
        drpipe_close(client);

        return true;
    }

    return false;
}

int dred_main(dr_cmdline cmdline)
{
    // Go down a different branch for command-line functions.
    if (dr_cmdline_key_exists(&cmdline, "f")) {
        return dred_main_f(cmdline);    // <-- Implemented in cmdline_funcs/dred_main_f.c
    }


    bool tryUsingExistingInstance = true;
    if (dr_cmdline_key_exists(&cmdline, "newinstance")) {
        tryUsingExistingInstance = false;
    }

    bool disableIPC = false;
    if (dr_cmdline_key_exists(&cmdline, "noipc")) {
        disableIPC = true;
    }

#ifndef _WIN32
    char lockFileName[256];
    dred_ipc_get_lock_name(lockFileName, sizeof(lockFileName));

    char pipeName[256];
    dred_ipc_get_pipe_name(pipeName, sizeof(pipeName));

    char pipeFileName[256];
    drpipe_get_translated_name(pipeName, pipeFileName, sizeof(pipeFileName));

    int lockfd = -1;
#endif

    // If an instance of dred is already running, we may want to use that one instead. We can know this by trying
    // to create a client-side pipe.
    if (tryUsingExistingInstance) {
#ifdef _WIN32
        if (dred__try_opening_existing_process(cmdline)) {
            return 0;
        }
#else
        bool isOtherProcessRunning = false;
        int fd = open(lockFileName, O_RDONLY, 0666);
        if (fd != -1) {
            if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
                isOtherProcessRunning = true;
            } else {
                isOtherProcessRunning = false;
            }

            close(fd);

            // If the other process isn't running, remove both the lock and pipe files.
            if (!isOtherProcessRunning) {
                remove(lockFileName);
                remove(pipeFileName);
            }
        }

        if (isOtherProcessRunning) {
            if (dred__try_opening_existing_process(cmdline)) {
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


    // The platform needs to be initialized first. In the case of Windows, this will register the window classes
    // and enable DPI awareness. Always make sure this is the first thing to be called.
    dred_platform_init();

    dred_context dred;
    if (!dred_init(&dred, cmdline)) {
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
    dred_platform_uninit();
    return result;
}

int main(int argc, char** argv)
{
    dr_cmdline cmdline;
    dr_init_cmdline(&cmdline, argc, argv);

    return dred_main(cmdline);
}

#ifdef _WIN32
#ifdef NDEBUG
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    dr_cmdline cmdline;
    dr_init_cmdline_win32(&cmdline, GetCommandLineA());

    return dred_main(cmdline);
}
#endif
#endif

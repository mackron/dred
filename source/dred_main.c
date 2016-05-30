
// These #defines enable us to load large files on Linux platforms. They need to be placed before including any headers.
#ifndef _WIN32
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#endif

// Standard headers.
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

// Platform headers.
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#endif

// Platform libraries, for simplifying MSVC builds.
#ifdef _WIN32
#if defined(_MSC_VER) || defined(__clang__)
#pragma comment(lib, "msimg32.lib")
#endif
#endif


// External libraries.
#define DR_UTIL_IMPLEMENTATION
#include "../../dr_libs/dr.h"

#define DR_PATH_IMPLEMENTATION
#include "../../dr_libs/dr_path.h"

#define DR_GUI_IMPLEMENTATION
#define DR_GUI_INCLUDE_WIP
#define DR_2D_IMPLEMENTATION
#include "../../dr_libs/dr_gui.h"

#define GB_STRING_IMPLEMENTATION
#include "gb_string.h"


// dred header files.
#include "dred_build_config.h"
#include "dred_types.h"
#include "dred_fs.h"
#include "dred_config.h"
#include "dred_context.h"
#include "dred_platform_layer.h"

// dred source files.
#include "dred_fs.c"
#include "dred_config.c"
#include "dred_context.c"
#include "dred_platform_layer.c"

int dred_main(dr_cmdline cmdline)
{
    // The platform needs to be initialized first. In the case of Windows, this will register the window classes
    // and enable DPI awareness. Always make sure this is the first thing to be called.
    dred_platform_init();

    dred_context dred;
    if (!dred_init(&dred, cmdline)) {
        return -1;
    }

    int result = dred_run(&dred);

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

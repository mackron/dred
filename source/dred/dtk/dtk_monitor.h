// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    union
    {
    #ifdef DTK_WIN32
        struct
        {
            /*HMONITOR*/ dtk_handle hMonitor;
        } win32;
    #endif
    #ifdef DTK_GTK
        struct
        {
#if GTK_CHECK_VERSION(3, 22, 0)
            /*GdkMonitor**/ dtk_ptr pMonitor;
#else
            int iMonitor;
#endif
        } gtk;
    #endif
    #ifdef DTK_X11
        struct
        {
            int unused;
        } x11;
    #endif
    };
} dtk_monitor;

// Retrieves the monitor count.
dtk_uint32 dtk_get_monitor_count(dtk_context* pTK);

// Retrieves the monitor at the given index.
dtk_monitor dtk_get_monitor_by_index(dtk_context* pTK, dtk_uint32 index);

// Retrieves the monitor that the given window is sitting in, or the closest to it.
dtk_monitor dtk_get_monitor_by_window(dtk_context* pTK, dtk_window* pWindow);

// Retrieves the monitor that the given point is sitting in, or the closest to it.
dtk_monitor dtk_get_monitor_by_point(dtk_context* pTK, dtk_int32 x, dtk_int32 y);

// Retrieves the DPI scale for the given monitor.
float dtk_monitor_get_dpi_scale(dtk_context* pTK, dtk_monitor monitor);
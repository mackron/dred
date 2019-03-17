// Copyright (C) 2019 David Reid. See included LICENSE file.

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
            union
            {
                /*GdkMonitor**/ dtk_ptr pMonitor;   // GTK 3.22+
                int iMonitor;
            };
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

// Returns a null monitor that is good for initialization purposes.
dtk_monitor dtk_monitor_null();

// Determines whether or not the given monitor is a null monitor.
dtk_bool32 dtk_monitor_is_null(dtk_monitor monitor);

// Retrieves the monitor count.
dtk_uint32 dtk_get_monitor_count(dtk_context* pTK);

// Retrieves the monitor at the given index.
dtk_monitor dtk_get_monitor_by_index(dtk_context* pTK, dtk_uint32 index);

// Retrieves the monitor that the given window is sitting in, or the closest to it.
dtk_monitor dtk_get_monitor_by_window(dtk_context* pTK, dtk_window* pWindow);

// Retrieves the monitor that the given point is sitting in, or the closest to it.
dtk_monitor dtk_get_monitor_by_point(dtk_context* pTK, dtk_int32 x, dtk_int32 y);

// Checks if two monitors are the same.
dtk_bool32 dtk_monitor_equal(dtk_monitor monitorA, dtk_monitor monitorB);

// Retrieves the rectangle of the given monitor relative to the whole screen area.
dtk_rect dtk_monitor_get_rect(dtk_context* pTK, dtk_monitor monitor);

// Retrieves the DPI scale for the given monitor.
float dtk_monitor_get_dpi_scale(dtk_context* pTK, dtk_monitor monitor);
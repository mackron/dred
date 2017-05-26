// Copyright (C) 2017 David Reid. See included LICENSE file.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Win32
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
dtk_monitor dtk_monitor__init_HMONITOR(HMONITOR hMonitor)
{
    dtk_monitor monitor;
    monitor.win32.hMonitor = hMonitor;

    return monitor;
}

BOOL CALLBACK dtk_get_monitor_count__win32__cb(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    (void)hMonitor;
    (void)hdcMonitor;
    (void)lprcMonitor;

    *((dtk_uint32*)dwData) += 1;
    return TRUE;
}
dtk_uint32 dtk_get_monitor_count__win32(dtk_context* pTK)
{
    (void)pTK;

    dtk_uint32 count = 0;
    EnumDisplayMonitors(NULL, NULL, dtk_get_monitor_count__win32__cb, (LPARAM)&count);

    return count;
}


typedef struct
{
    dtk_uint32 requestedIndex;
    dtk_uint32 currentIndex;
    HMONITOR hMonitor;
} dtk_get_monitor_by_index__win32__data;
BOOL CALLBACK dtk_get_monitor_by_index__win32__cb(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    (void)hdcMonitor;
    (void)lprcMonitor;

    dtk_get_monitor_by_index__win32__data* pData = (dtk_get_monitor_by_index__win32__data*)dwData;
    if (pData->requestedIndex == pData->currentIndex) {
        pData->hMonitor = hMonitor;
        return FALSE;   // Terminate early.
    }

    pData->currentIndex += 1;
    return TRUE;
}
dtk_monitor dtk_get_monitor_by_index__win32(dtk_context* pTK, dtk_uint32 index)
{
    (void)pTK;

    dtk_get_monitor_by_index__win32__data data;
    data.requestedIndex = index;
    data.currentIndex = 0;
    data.hMonitor = NULL;
    EnumDisplayMonitors(NULL, NULL, dtk_get_monitor_by_index__win32__cb, (LPARAM)&data);

    return dtk_monitor__init_HMONITOR(data.hMonitor);
}

dtk_monitor dtk_get_monitor_by_window__win32(dtk_context* pTK, dtk_window* pWindow)
{
    (void)pTK;
    return dtk_monitor__init_HMONITOR(MonitorFromWindow((HWND)pWindow->win32.hWnd, MONITOR_DEFAULTTONEAREST));
}

dtk_monitor dtk_get_monitor_by_point__win32(dtk_context* pTK, dtk_int32 x, dtk_int32 y)
{
    (void)pTK;

    POINT pt;
    pt.x = x;
    pt.y = y;
    return dtk_monitor__init_HMONITOR(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST));
}

dtk_rect dtk_monitor_get_rect__win32(dtk_context* pTK, dtk_monitor monitor)
{
    (void)pTK;

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW((HMONITOR)monitor.win32.hMonitor, &mi)) {
        return dtk_rect_init(0, 0, 0, 0);
    }

    return dtk_rect_init(mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom);
}

float dtk_monitor_get_dpi_scale__win32(dtk_context* pTK, dtk_monitor monitor)
{
    dtk_bool32 fallBackToSystemDPI = DTK_FALSE;

    HMODULE hSHCoreDLL = LoadLibraryW(L"shcore.dll");
    if (hSHCoreDLL != NULL) {
        DTK_PFN_GetDpiForMonitor _GetDpiForMonitor = (DTK_PFN_GetDpiForMonitor)GetProcAddress(hSHCoreDLL, "GetDpiForMonitor");
        if (_GetDpiForMonitor != NULL) {
            UINT dpiX;
            UINT dpiY;
            if (_GetDpiForMonitor((HMONITOR)monitor.win32.hMonitor, DTK_MDT_DEFAULT, &dpiX, &dpiY) == S_OK) {
                (void)dpiY;     // We aren't using the Y DPI for now. This is just for simplicity because it's so rare that the X and Y DPIs would differ.
                return dpiX / 96.0f;
            } else {
                fallBackToSystemDPI = DTK_TRUE;
            }
        } else {
            fallBackToSystemDPI = DTK_TRUE;
        }
    } else {
        fallBackToSystemDPI = DTK_TRUE;
    }

    FreeLibrary(hSHCoreDLL);

    if (fallBackToSystemDPI) {
        return dtk_get_dpi_scale(pTK);
    }

    // Should never get here...
    return 1;
}
#endif



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// GTK
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_GTK
dtk_uint32 dtk_get_monitor_count__gtk(dtk_context* pTK)
{
    (void)pTK;

#if GTK_CHECK_VERSION(3, 22, 0)
    return (dtk_uint32)gdk_display_get_n_monitors(gdk_display_get_default());
#else
    return (dtk_uint32)gdk_screen_get_n_monitors(gdk_display_get_default_screen(gdk_display_get_default()));
#endif
}

dtk_monitor dtk_get_monitor_by_index__gtk(dtk_context* pTK, dtk_uint32 index)
{
    (void)pTK;

    dtk_monitor monitor;
#if GTK_CHECK_VERSION(3, 22, 0)
    monitor.gtk.pMonitor = gdk_display_get_monitor(gdk_display_get_default(), (int)index);
#else
    monitor.gtk.iMonitor = (int)index;
#endif

    return monitor;
}

dtk_monitor dtk_get_monitor_by_window__gtk(dtk_context* pTK, dtk_window* pWindow)
{
    (void)pTK;
    dtk_assert(pWindow != NULL);

    dtk_monitor monitor = dtk_monitor_null();
#if GTK_CHECK_VERSION(3, 22, 0)
    monitor.gtk.pMonitor = gdk_display_get_monitor_at_window(gdk_display_get_default(), gtk_widget_get_window(pWindow->gtk.pWidget));
#else
    monitor.gtk.iMonitor = gdk_screen_get_monitor_at_window(gdk_display_get_default_screen(gdk_display_get_default()), gtk_widget_get_window(pWindow->gtk.pWidget));
#endif

    return monitor;
}

dtk_monitor dtk_get_monitor_by_point__gtk(dtk_context* pTK, dtk_int32 x, dtk_int32 y)
{
    (void)pTK;

    dtk_monitor monitor = dtk_monitor_null();
#if GTK_CHECK_VERSION(3, 22, 0)
    monitor.gtk.pMonitor = gdk_display_get_monitor_at_point(gdk_display_get_default(), x, y);
#else
    monitor.gtk.iMonitor = gdk_screen_get_monitor_at_point(gdk_display_get_default_screen(gdk_display_get_default()), x, y);
#endif

    return monitor;
}

dtk_rect dtk_monitor_get_rect__gtk(dtk_context* pTK, dtk_monitor monitor)
{
    (void)pTK;

#if GTK_CHECK_VERSION(3, 22, 0)
    GdkRectangle rect;
    gdk_monitor_get_geometry((GdkMonitor*)monitor.gtk.pMonitor, &rect);
    return dtk_rect_init(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
#else
    GdkRectangle rect;
    gdk_screen_get_monitor_geometry(gdk_display_get_default_screen(gdk_display_get_default()), monitor.gtk.iMonitor, &rect);
    return dtk_rect_init(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
#endif
}

float dtk_monitor_get_dpi_scale__gtk(dtk_context* pTK, dtk_monitor monitor)
{
    (void)pTK;

#if GTK_CHECK_VERSION(3, 22, 0)
    return (float)gdk_monitor_get_scale_factor((GdkMonitor*)monitor.gtk.pMonitor);
#else
    return (float)gdk_screen_get_monitor_scale_factor(gdk_display_get_default_screen(gdk_display_get_default()), monitor.gtk.iMonitor);
#endif
}
#endif



dtk_monitor dtk_monitor_null()
{
    dtk_monitor monitor;
    dtk_zero_object(&monitor);

#ifdef DTK_WIN32
    monitor.win32.hMonitor = NULL;
#endif
#ifdef DTK_GTK
    #if GTK_CHECK_VERSION(3, 22, 0)
        monitor.gtk.pMonitor = NULL;
    #else
        monitor.gtk.iMonitor = -1;
    #endif
#endif

    return monitor;
}

dtk_bool32 dtk_monitor_is_null(dtk_monitor monitor)
{
#ifdef DTK_WIN32
    return monitor.win32.hMonitor == NULL;
#endif
#ifdef DTK_GTK
    #if GTK_CHECK_VERSION(3, 22, 0)
        return monitor.gtk.pMonitor == NULL;
    #else
        return monitor.gtk.iMonitor == -1;
    #endif
#endif
}

dtk_uint32 dtk_get_monitor_count(dtk_context* pTK)
{
    if (pTK == NULL) return 0;

    dtk_uint32 count = 0;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        count = dtk_get_monitor_count__win32(pTK);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        count = dtk_get_monitor_count__gtk(pTK);
    }
#endif

    return count;
}

dtk_monitor dtk_get_monitor_by_index(dtk_context* pTK, dtk_uint32 index)
{
    if (pTK == NULL) return dtk_monitor_null();

    dtk_monitor monitor = dtk_monitor_null();
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        monitor = dtk_get_monitor_by_index__win32(pTK, index);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        monitor = dtk_get_monitor_by_index__gtk(pTK, index);
    }
#endif

    return monitor;
}

dtk_monitor dtk_get_monitor_by_window(dtk_context* pTK, dtk_window* pWindow)
{
    if (pTK == NULL) return dtk_monitor_null();

    dtk_monitor monitor = dtk_monitor_null();
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        monitor = dtk_get_monitor_by_window__win32(pTK, pWindow);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        monitor = dtk_get_monitor_by_window__gtk(pTK, pWindow);
    }
#endif

    return monitor;
}

dtk_monitor dtk_get_monitor_by_point(dtk_context* pTK, dtk_int32 x, dtk_int32 y)
{
    if (pTK == NULL) return dtk_monitor_null();

    dtk_monitor monitor = dtk_monitor_null();
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        monitor = dtk_get_monitor_by_point__win32(pTK, x, y);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        monitor = dtk_get_monitor_by_point__gtk(pTK, x, y);
    }
#endif

    return monitor;
}

dtk_bool32 dtk_monitor_equal(dtk_monitor monitorA, dtk_monitor monitorB)
{
    return memcmp(&monitorA, &monitorB, sizeof(dtk_monitor)) == 0;
}

dtk_rect dtk_monitor_get_rect(dtk_context* pTK, dtk_monitor monitor)
{
    if (pTK == NULL || dtk_monitor_is_null(monitor)) return dtk_rect_init(0, 0, 0, 0);

    dtk_rect rect = dtk_rect_init(0, 0, 0, 0);
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        rect = dtk_monitor_get_rect__win32(pTK, monitor);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        rect = dtk_monitor_get_rect__gtk(pTK, monitor);
    }
#endif

    return rect;
}

float dtk_monitor_get_dpi_scale(dtk_context* pTK, dtk_monitor monitor)
{
    if (pTK == NULL) return 1;

    if (dtk_monitor_is_null(monitor)) {
        return 1;
    }

    float scale = 0;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        scale = dtk_monitor_get_dpi_scale__win32(pTK, monitor);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        scale = dtk_monitor_get_dpi_scale__gtk(pTK, monitor);
    }
#endif

    return scale;
}
// Copyright (C) 2016 David Reid. See included LICENSE file.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// GDI
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
dtk_result dtk_surface_init_window__gdi(dtk_context* pTK, dtk_window* pWindow, dtk_surface* pSurface)
{
    (void)pTK;

    pSurface->gdi.hDC = (dtk_handle)GetDC((HWND)pWindow->win32.hWnd);
    if (pSurface->gdi.hDC == 0) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_surface_uninit__gdi(dtk_surface* pSurface)
{
    (void)pSurface;
    return DTK_SUCCESS;
}

void dtk_surface_draw_quad__gdi(dtk_surface* pSurface, dtk_int32 x, dtk_int32 y, dtk_uint32 width, dtk_uint32 height)
{
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(NULL_PEN));
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor((HDC)pSurface->gdi.hDC, RGB(0, 0, 0));
    Rectangle((HDC)pSurface->gdi.hDC, x, y, x + width + 1, y + height + 1);
}
#endif


dtk_result dtk_surface_init_window(dtk_context* pTK, dtk_window* pWindow, dtk_surface* pSurface)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pSurface);

    if (pTK == NULL || pWindow == NULL) return DTK_INVALID_ARGS;
    pSurface->pTK = pTK;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_surface_init_window__gdi(pTK, pWindow, pSurface);
    }
#endif

    return result;
}

dtk_result dtk_surface_uninit(dtk_surface* pSurface)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pSurface->pTK->platform == dtk_platform_win32) {
        result = dtk_surface_uninit__gdi(pSurface);
    }
#endif

    return result;
}

void dtk_surface_draw_quad(dtk_surface* pSurface, dtk_int32 x, dtk_int32 y, dtk_uint32 width, dtk_uint32 height)
{
    if (pSurface == NULL) return;

#ifdef DTK_WIN32
    if (pSurface->pTK->platform == dtk_platform_win32) {
        dtk_surface_draw_quad__gdi(pSurface, x, y, width, height);
    }
#endif
}
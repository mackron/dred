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

    pSurface->backend = dtk_graphics_backend_gdi;
    return DTK_SUCCESS;
}

dtk_result dtk_surface_uninit__gdi(dtk_surface* pSurface)
{
    (void)pSurface;
    return DTK_SUCCESS;
}

void dtk_surface_clear__gdi(dtk_surface* pSurface, dtk_color color)
{
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(NULL_PEN));
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor((HDC)pSurface->gdi.hDC, RGB(color.r, color.g, color.b));
    Rectangle((HDC)pSurface->gdi.hDC, 0, 0, (int)pSurface->width+1, (int)pSurface->height+1);
}

void dtk_surface_draw_quad__gdi(dtk_surface* pSurface, dtk_int32 x, dtk_int32 y, dtk_uint32 width, dtk_uint32 height)
{
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(NULL_PEN));
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor((HDC)pSurface->gdi.hDC, RGB(0, 0, 255));
    Rectangle((HDC)pSurface->gdi.hDC, x, y, x + width + 1, y + height + 1);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Cairo
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_GTK
dtk_result dtk_surface_init_window__cairo(dtk_context* pTK, dtk_window* pWindow, dtk_surface* pSurface)
{
    (void)pTK;
    
    int width;
    int height;
    gtk_window_get_size(GTK_WINDOW(pWindow->gtk.pWidget), &width, &height);
    
    cairo_surface_t* pCairoSurface = gdk_window_create_similar_surface(gtk_widget_get_window((GtkWidget*)pWindow->gtk.pWidget), CAIRO_CONTENT_COLOR, width, height); 
    if (pCairoSurface == NULL) {
        return DTK_ERROR;   // I don't think this is ever hit, but I like it for safety.
    }
    
    cairo_t* pCairoContext = cairo_create(pCairoSurface);
    if (pCairoContext == NULL) {
        cairo_surface_destroy(pCairoSurface);
        return DTK_ERROR;
    }
    
    pSurface->cairo.pSurface = pCairoSurface;
    pSurface->cairo.pContext = pCairoContext;
    
    pSurface->backend = dtk_graphics_backend_cairo;
    return DTK_SUCCESS;
}

dtk_result dtk_surface_uninit__cairo(dtk_surface* pSurface)
{
    cairo_destroy((cairo_t*)pSurface->cairo.pContext);
    cairo_surface_destroy((cairo_surface_t*)pSurface->cairo.pSurface);
    
    return DTK_SUCCESS;
}

void dtk_surface_clear__cairo(dtk_surface* pSurface, dtk_color color)
{
    cairo_set_source_rgba(pSurface->cairo.pContext, color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a/255.0f);
    cairo_paint(pSurface->cairo.pContext);
}

void dtk_surface_draw_quad__cairo(dtk_surface* pSurface, dtk_int32 x, dtk_int32 y, dtk_uint32 width, dtk_uint32 height)
{
    cairo_set_source_rgba(pSurface->cairo.pContext, 0, 0, 1, 1);
    cairo_rectangle(pSurface->cairo.pContext, x, y, width, height);
    cairo_fill(pSurface->cairo.pContext);
}
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

dtk_result dtk_surface_init_window(dtk_context* pTK, dtk_window* pWindow, dtk_surface* pSurface)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pSurface);

    if (pTK == NULL || pWindow == NULL) return DTK_INVALID_ARGS;
    pSurface->pTK = pTK;
    pSurface->width  = DTK_CONTROL(pWindow)->width;
    pSurface->height = DTK_CONTROL(pWindow)->height;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        result = dtk_surface_init_window__gdi(pTK, pWindow, pSurface);
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        result = dtk_surface_init_window__cairo(pTK, pWindow, pSurface);
    }
#endif

    return result;
}

dtk_result dtk_surface_uninit(dtk_surface* pSurface)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        result = dtk_surface_uninit__gdi(pSurface);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        result = dtk_surface_uninit__cairo(pSurface);
    }
#endif

    return result;
}

void dtk_surface_clear(dtk_surface* pSurface, dtk_color color)
{
    if (pSurface == NULL) return;
    
#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        dtk_surface_clear__gdi(pSurface, color);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        dtk_surface_clear__cairo(pSurface, color);
    }
#endif
}

void dtk_surface_draw_quad(dtk_surface* pSurface, dtk_int32 x, dtk_int32 y, dtk_uint32 width, dtk_uint32 height)
{
    if (pSurface == NULL) return;

#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        dtk_surface_draw_quad__gdi(pSurface, x, y, width, height);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        dtk_surface_draw_quad__cairo(pSurface, x, y, width, height);
    }
#endif
}
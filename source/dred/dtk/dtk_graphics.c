// Copyright (C) 2016 David Reid. See included LICENSE file.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// GDI
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_WIN32
// Fonts
// =====
dtk_result dtk_font_init__gdi(dtk_context* pTK, const char* family, float size, dtk_font_weight weight, dtk_font_slant slant, float rotation, dtk_uint32 optionFlags, dtk_font* pFont)
{
    (void)pTK;

    LONG weightGDI = FW_REGULAR;
    switch (weight)
    {
    case dtk_font_weight_medium:      weightGDI = FW_MEDIUM;     break;
    case dtk_font_weight_thin:        weightGDI = FW_THIN;       break;
    case dtk_font_weight_extra_light: weightGDI = FW_EXTRALIGHT; break;
    case dtk_font_weight_light:       weightGDI = FW_LIGHT;      break;
    case dtk_font_weight_semi_bold:   weightGDI = FW_SEMIBOLD;   break;
    case dtk_font_weight_bold:        weightGDI = FW_BOLD;       break;
    case dtk_font_weight_extra_bold:  weightGDI = FW_EXTRABOLD;  break;
    case dtk_font_weight_heavy:       weightGDI = FW_HEAVY;      break;
    default: break;
    }

	BYTE slantGDI = FALSE;
    if (slant == dtk_font_slant_italic || slant == dtk_font_slant_oblique) {
        slantGDI = TRUE;
    }

	LOGFONTA logfont;
	memset(&logfont, 0, sizeof(logfont));
    logfont.lfHeight      = -(LONG)size;
	logfont.lfWeight      = weightGDI;
	logfont.lfItalic      = slantGDI;
	logfont.lfCharSet     = DEFAULT_CHARSET;
    logfont.lfQuality     = (optionFlags & DTK_FONT_FLAG_NO_CLEARTYPE) ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY;
    logfont.lfEscapement  = (LONG)rotation * 10;
    logfont.lfOrientation = (LONG)rotation * 10;
    dtk_strncpy_s(logfont.lfFaceName, sizeof(logfont.lfFaceName), family, _TRUNCATE);

    pFont->gdi.hFont = (dtk_handle)CreateFontIndirectA(&logfont);
    if (pFont->gdi.hFont == NULL) {
        return DTK_ERROR;
    }

    pFont->backend = dtk_graphics_backend_gdi;
    return DTK_SUCCESS;
}

dtk_result dtk_font_uninit__gdi(dtk_font* pFont)
{
    DeleteObject((HGDIOBJ)pFont->gdi.hFont);
    return DTK_SUCCESS;
}


// Surfaces
// ========
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
    if (pSurface->gdi.pScratchBuffer) {
        dtk_free(pSurface->gdi.pScratchBuffer);
    }

    return DTK_SUCCESS;
}

void dtk_surface_clear__gdi(dtk_surface* pSurface, dtk_color color)
{
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(NULL_PEN));
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor((HDC)pSurface->gdi.hDC, RGB(color.r, color.g, color.b));
    Rectangle((HDC)pSurface->gdi.hDC, 0, 0, (int)pSurface->width+1, (int)pSurface->height+1);
}

void dtk_surface_draw_rect__gdi(dtk_surface* pSurface, dtk_int32 x, dtk_int32 y, dtk_uint32 width, dtk_uint32 height)
{
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(NULL_PEN));
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor((HDC)pSurface->gdi.hDC, RGB(0, 0, 255));
    Rectangle((HDC)pSurface->gdi.hDC, x, y, x + width + 1, y + height + 1);
}

wchar_t* dtk_mb_to_wchar__gdi(dtk_surface* pSurface, const char* text, size_t textSizeInBytes, size_t* pCharacterCount)
{
    int wcharCount = 0;

    // We first try to copy the string into the already-allocated buffer. If it fails we fall back to the slow path which requires
    // two conversions.
    if (pSurface->gdi.pScratchBuffer == NULL) {
        goto fallback;
    }

    wcharCount = MultiByteToWideChar(CP_UTF8, 0, text, (int)textSizeInBytes, (wchar_t*)pSurface->gdi.pScratchBuffer, pSurface->gdi.scratchBufferSize);
    if (wcharCount != 0) {
        if (pCharacterCount) *pCharacterCount = (size_t)wcharCount;
        return (wchar_t*)pSurface->gdi.pScratchBuffer;
    }


fallback:;
    wcharCount = MultiByteToWideChar(CP_UTF8, 0, text, (int)textSizeInBytes, NULL, 0);
    if (wcharCount == 0) {
        return NULL;
    }

    if (pSurface->gdi.scratchBufferSize < (size_t)(wcharCount+1) * sizeof(wchar_t)) {
        dtk_free(pSurface->gdi.pScratchBuffer);
        pSurface->gdi.pScratchBuffer    = dtk_malloc((wcharCount+1) * sizeof(wchar_t));
        pSurface->gdi.scratchBufferSize = wcharCount + 1;
    }

    wcharCount = MultiByteToWideChar(CP_UTF8, 0, text, (int)textSizeInBytes, (wchar_t*)pSurface->gdi.pScratchBuffer, pSurface->gdi.scratchBufferSize);
    if (wcharCount == 0) {
        return NULL;
    }

    if (pCharacterCount != NULL) *pCharacterCount = wcharCount;
    return pSurface->gdi.pScratchBuffer;
}

void dtk_surface_draw_text__gdi(dtk_surface* pSurface, dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float posX, float posY, dtk_color fgColor, dtk_color bgColor)
{
    // TODO: Select the closest sub-font based on the scale.
    (void)scale;

    HDC hDC = (HDC)pSurface->gdi.hDC;    // For ease of use.

    size_t textWLength;
    wchar_t* textW = dtk_mb_to_wchar__gdi(pSurface, text, textSizeInBytes, &textWLength);
    if (textW != NULL)
    {
        SelectObject(hDC, (HFONT)pFont->gdi.hFont);

        UINT options = 0;
        RECT rect = {0, 0, 0, 0};

        if (bgColor.a == 0) {
            SetBkMode(hDC, TRANSPARENT);
        } else {
            SetBkMode(hDC, OPAQUE);
            SetBkColor(hDC, RGB(bgColor.r, bgColor.g, bgColor.b));

            // There is an issue with the way GDI draws the background of a string of text. When ClearType is enabled, the rectangle appears
            // to be wider than it is supposed to be. As a result, drawing text right next to each other results in the most recent one being
            // drawn slightly on top of the previous one. To fix this we need to use ExtTextOut() with the ETO_CLIPPED parameter enabled.
            options |= ETO_CLIPPED;

            SIZE textSize = {0, 0};
            GetTextExtentPoint32W(hDC, textW, textWLength, &textSize);
            rect.left   = (LONG)posX;
            rect.top    = (LONG)posY;
            rect.right  = (LONG)(posX + textSize.cx);
            rect.bottom = (LONG)(posY + textSize.cy);
        }

        SetTextColor(hDC, RGB(fgColor.r, fgColor.g, fgColor.b));
        ExtTextOutW(hDC, (int)posX, (int)posY, options, &rect, textW, textWLength, NULL);
    }
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
// Fonts
// =====
dtk_result dtk_font_init__cairo(dtk_context* pTK, const char* family, float size, dtk_font_weight weight, dtk_font_slant slant, float rotation, dtk_uint32 optionFlags, dtk_font* pFont)
{
    return DTK_SUCCESS;
}

dtk_result dtk_font_uninit__cairo(dtk_font* pFont)
{
    return DTK_SUCCESS;
}


// Surfaces
// ========
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

void dtk_surface_draw_rect__cairo(dtk_surface* pSurface, dtk_int32 x, dtk_int32 y, dtk_uint32 width, dtk_uint32 height)
{
    cairo_set_source_rgba(pSurface->cairo.pContext, 0, 0, 1, 1);
    cairo_rectangle(pSurface->cairo.pContext, x, y, width, height);
    cairo_fill(pSurface->cairo.pContext);
}
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Fonts
// =====
dtk_result dtk_font_init(dtk_context* pTK, const char* family, float size, dtk_font_weight weight, dtk_font_slant slant, float rotation, dtk_uint32 optionFlags, dtk_font* pFont)
{
    if (pFont == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pFont);

    if (pTK == NULL || family == NULL || size == 0) return DTK_INVALID_ARGS;
    pFont->pTK = pTK;
    pFont->size = size;
    pFont->weight = weight;
    pFont->slant = slant;
    pFont->rotation = rotation;
    pFont->optionFlags = optionFlags;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        if (result != DTK_SUCCESS) {
            result = dtk_font_init__gdi(pTK, family, size, weight, slant, rotation, optionFlags, pFont);
        }
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        if (result != DTK_SUCCESS) {
            result = dtk_font_init__cairo(pTK, family, size, weight, slant, rotation, optionFlags, pFont);
        }
    }
#endif

    return result;
}

dtk_result dtk_font_uninit(dtk_font* pFont)
{
    if (pFont == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pFont->backend == dtk_graphics_backend_gdi) {
        result = dtk_font_uninit__gdi(pFont);
    }
#endif
#ifdef DTK_GTK
    if (pFont->backend == dtk_graphics_backend_cairo) {
        result = dtk_font_uninit__cairo(pFont);
    }
#endif

    return result;
}


// Surfaces
// ========
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
        if (result != DTK_SUCCESS) {
            result = dtk_surface_init_window__gdi(pTK, pWindow, pSurface);
        }
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        if (result != DTK_SUCCESS) {
            result = dtk_surface_init_window__cairo(pTK, pWindow, pSurface);
        }
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

void dtk_surface_draw_rect(dtk_surface* pSurface, dtk_int32 x, dtk_int32 y, dtk_uint32 width, dtk_uint32 height)
{
    if (pSurface == NULL) return;

#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        dtk_surface_draw_rect__gdi(pSurface, x, y, width, height);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        dtk_surface_draw_rect__cairo(pSurface, x, y, width, height);
    }
#endif
}

void dtk_surface_draw_text(dtk_surface* pSurface, dtk_font* pFont, float scale, const char* text, size_t textLength, float posX, float posY, dtk_color fgColor, dtk_color bgColor)
{
    if (pSurface == NULL) return;

#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        dtk_surface_draw_text__gdi(pSurface, pFont, scale, text, textLength, posX, posY, fgColor, bgColor);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        dtk_surface_draw_text__cairo(pSurface, pFont, scale, text, textLength, posX, posY, fgColor, bgColor);
    }
#endif
}
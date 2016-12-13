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


    // Retrieving font metrics is quite slow with GDI so we'll cache it.
    HGDIOBJ hPrevFont = SelectObject((HDC)pTK->win32.hGraphicsDC, (HFONT)pFont->gdi.hFont);
    {
        TEXTMETRIC metrics;
        GetTextMetrics((HDC)pTK->win32.hGraphicsDC, &metrics);
        pFont->gdi.metrics.ascent     = metrics.tmAscent;
        pFont->gdi.metrics.descent    = metrics.tmDescent;
        pFont->gdi.metrics.lineHeight = metrics.tmHeight;

        const MAT2 transform = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};        // <-- Identity matrix

        GLYPHMETRICS spaceMetrics;
        DWORD bitmapBufferSize = GetGlyphOutlineW((HDC)pTK->win32.hGraphicsDC, ' ', GGO_NATIVE, &spaceMetrics, 0, NULL, &transform);
        if (bitmapBufferSize == GDI_ERROR) {
			pFont->gdi.metrics.spaceWidth = 4;
        } else {
            pFont->gdi.metrics.spaceWidth = spaceMetrics.gmCellIncX;
        }
    }
    SelectObject((HDC)pTK->win32.hGraphicsDC, hPrevFont);


    pFont->backend = dtk_graphics_backend_gdi;
    return DTK_SUCCESS;
}

dtk_result dtk_font_uninit__gdi(dtk_font* pFont)
{
    DeleteObject((HGDIOBJ)pFont->gdi.hFont);
    return DTK_SUCCESS;
}

dtk_result dtk_font_get_metrics__gdi(dtk_font* pFont, float scale, dtk_font_metrics* pMetrics)
{
    // TODO: Select the sub-font from the scale.
    (void)scale;

    *pMetrics = pFont->gdi.metrics;
    return DTK_SUCCESS;
}

dtk_result dtk_font_get_glyph_metrics__gdi(dtk_font* pFont, float scale, dtk_uint32 utf32, dtk_glyph_metrics* pMetrics)
{
    // TODO: Select the sub-font from the scale.
    (void)scale;

    dtk_result result = DTK_ERROR;
    HGDIOBJ hPrevFont = SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, (HFONT)pFont->gdi.hFont);
    {
        const MAT2 transform = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};        // <-- Identity matrix

        dtk_uint16 utf16[2];
        dtk_uint32 utf16Len = dtk_utf32_to_utf16_ch(utf32, utf16);

        WCHAR glyphIndices[2];

        GCP_RESULTSW glyphResults;
        ZeroMemory(&glyphResults, sizeof(glyphResults));
        glyphResults.lStructSize = sizeof(glyphResults);
        glyphResults.lpGlyphs = glyphIndices;
        glyphResults.nGlyphs  = 2;
        if (GetCharacterPlacementW((HDC)pFont->pTK->win32.hGraphicsDC, (LPCWSTR)utf16, utf16Len, 0, &glyphResults, 0) != 0) {
            GLYPHMETRICS metrics;
            DWORD bitmapBufferSize = GetGlyphOutlineW((HDC)pFont->pTK->win32.hGraphicsDC, glyphIndices[0], GGO_NATIVE | GGO_GLYPH_INDEX, &metrics, 0, NULL, &transform);
            if (bitmapBufferSize != GDI_ERROR) {
                pMetrics->width    = metrics.gmBlackBoxX;
                pMetrics->height   = metrics.gmBlackBoxY;
                pMetrics->originX  = metrics.gmptGlyphOrigin.x;
                pMetrics->originY  = metrics.gmptGlyphOrigin.y;
                pMetrics->advanceX = metrics.gmCellIncX;
                pMetrics->advanceY = metrics.gmCellIncY;
                result = DTK_SUCCESS;
            }
        }
    }
    SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, hPrevFont);

    return result;
}

dtk_result dtk_font_measure_string__gdi(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float* pWidth, float* pHeight)
{
    // TODO: Select the sub-font from the scale.
    (void)scale;

    dtk_result result = DTK_ERROR;
    HGDIOBJ hPrevFont = SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, (HFONT)pFont->gdi.hFont);
    {
        unsigned int textWLength;
        wchar_t* textW = dtk__mb_to_wchar__win32(pFont->pTK, text, textSizeInBytes, &textWLength);
        if (textW != NULL) {
            SIZE sizeWin32;
            if (GetTextExtentPoint32W((HDC)pFont->pTK->win32.hGraphicsDC, textW, textWLength, &sizeWin32)) {
                if (pWidth)  *pWidth  = (float)sizeWin32.cx;
                if (pHeight) *pHeight = (float)sizeWin32.cy;
                result = DTK_SUCCESS;
            }
        }
    }
    SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, hPrevFont);

    return result;
}

dtk_result dtk_font_get_text_cursor_position_from_point__gdi(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosX, size_t* pCharacterIndex)
{
    // TODO: Select the sub-font from the scale.
    (void)scale;

    dtk_result result = DTK_ERROR;
    HGDIOBJ hPrevFont = SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, (HFONT)pFont->gdi.hFont);
    {
        GCP_RESULTSW results;
        ZeroMemory(&results, sizeof(results));
        results.lStructSize = sizeof(results);
        results.nGlyphs     = (UINT)textSizeInBytes;

        unsigned int textWLength;
        wchar_t* textW = dtk__mb_to_wchar__win32(pFont->pTK, text, textSizeInBytes, &textWLength);
        if (textW != NULL) {
            if (results.nGlyphs > pFont->pTK->win32.glyphCacheSize) {
                pFont->pTK->win32.pGlyphCache = (dtk_int32*)dtk_realloc(pFont->pTK->win32.pCharConvBuffer, results.nGlyphs);
                if (pFont->pTK->win32.pGlyphCache == NULL) {
                    pFont->pTK->win32.glyphCacheSize = 0;
                } else {
                    pFont->pTK->win32.glyphCacheSize = results.nGlyphs;
                }
            }

            results.lpCaretPos = pFont->pTK->win32.pGlyphCache;
            if (results.lpCaretPos != NULL) {
                if (GetCharacterPlacementW((HDC)pFont->pTK->win32.hGraphicsDC, textW, results.nGlyphs, (int)maxWidth, &results, GCP_MAXEXTENT | GCP_USEKERNING) != 0) {
                    float textCursorPosX = 0;
                    unsigned int iChar;
                    for (iChar = 0; iChar < results.nGlyphs; ++iChar) {
                        float charBoundsLeft  = (float)results.lpCaretPos[iChar];
                        float charBoundsRight = 0;
                        if (iChar < results.nGlyphs - 1) {
                            charBoundsRight = (float)results.lpCaretPos[iChar + 1];
                        } else {
                            charBoundsRight = maxWidth;
                        }

                        if (inputPosX >= charBoundsLeft && inputPosX <= charBoundsRight) {
                            // The input position is somewhere on top of this character. If it's positioned on the left side of the character, set the output
                            // value to the character at iChar. Otherwise it should be set to the character at iChar + 1.
                            float charBoundsRightHalf = charBoundsLeft + ceilf(((charBoundsRight - charBoundsLeft) / 2.0f));
                            if (inputPosX <= charBoundsRightHalf) {
                                break;
                            } else {
                                textCursorPosX = charBoundsRight;
                                iChar += 1;
                                break;
                            }
                        }

                        textCursorPosX = charBoundsRight;
                    }

                    // Make sure the character index is in UTF-8 characters.
                    iChar = WideCharToMultiByte(CP_UTF8, 0, textW, (int)iChar, NULL, 0, NULL, FALSE);

                    if (pTextCursorPosX) *pTextCursorPosX = textCursorPosX;
                    if (pCharacterIndex) *pCharacterIndex = iChar;
                    result = DTK_SUCCESS;
                }
            }
        }
    }
    SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, hPrevFont);

    return result;
}

dtk_result dtk_font_get_text_cursor_position_from_char__gdi(dtk_font* pFont, float scale, const char* text, size_t characterIndex, float* pTextCursorPosX)
{
    // TODO: Select the sub-font from the scale.
    (void)scale;

    dtk_result result = DTK_ERROR;
    HGDIOBJ hPrevFont = SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, (HFONT)pFont->gdi.hFont);
    {
        GCP_RESULTSW results;
        ZeroMemory(&results, sizeof(results));
        results.lStructSize = sizeof(results);
        results.nGlyphs     = (DWORD)(characterIndex + 1);

        unsigned int textWLength;
        wchar_t* textW = dtk__mb_to_wchar__win32(pFont->pTK, text, (int)results.nGlyphs, &textWLength);
        if (textW != NULL) {
            if (results.nGlyphs > pFont->pTK->win32.glyphCacheSize) {
                pFont->pTK->win32.pGlyphCache = (dtk_int32*)dtk_realloc(pFont->pTK->win32.pCharConvBuffer, results.nGlyphs);
                if (pFont->pTK->win32.pGlyphCache == NULL) {
                    pFont->pTK->win32.glyphCacheSize = 0;
                } else {
                    pFont->pTK->win32.glyphCacheSize = results.nGlyphs;
                }
            }

            results.lpCaretPos = pFont->pTK->win32.pGlyphCache;
            if (results.lpCaretPos != NULL) {
                if (GetCharacterPlacementW((HDC)pFont->pTK->win32.hGraphicsDC, textW, results.nGlyphs, 0, &results, GCP_USEKERNING) != 0) {
                    if (pTextCursorPosX) *pTextCursorPosX = (float)results.lpCaretPos[characterIndex];
                    result = DTK_SUCCESS;
                }
            }
        }
    }
    SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, hPrevFont);

    return result;
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

void dtk_surface_draw_rect__gdi(dtk_surface* pSurface, dtk_int32 x, dtk_int32 y, dtk_uint32 width, dtk_uint32 height)
{
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(NULL_PEN));
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor((HDC)pSurface->gdi.hDC, RGB(0, 0, 255));
    Rectangle((HDC)pSurface->gdi.hDC, x, y, x + width + 1, y + height + 1);
}

void dtk_surface_draw_text__gdi(dtk_surface* pSurface, dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, dtk_int32 posX, dtk_int32 posY, dtk_color fgColor, dtk_color bgColor)
{
    // TODO: Select the closest sub-font based on the scale.
    (void)scale;

    HDC hDC = (HDC)pSurface->gdi.hDC;    // For ease of use.

    size_t textWLength;
    wchar_t* textW = dtk__mb_to_wchar__win32(pSurface->pTK, text, textSizeInBytes, &textWLength);
    if (textW != NULL) {
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

dtk_result dtk_font_get_metrics__gtk(dtk_font* pFont, float scale, dtk_font_metrics* pMetrics)
{
    // TODO: Implement me.
    return DTK_ERROR;
}

dtk_result dtk_font_get_glyph_metrics__gtk(dtk_font* pFont, float scale, dtk_uint32 utf32, dtk_glyph_metrics* pMetrics)
{
    // TODO: Implement me.
    return DTK_ERROR;
}

dtk_result dtk_font_measure_string__gtk(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float* pWidth, float* pHeight)
{
    // TODO: Implement me.
    return DTK_ERROR;
}

dtk_result dtk_font_get_text_cursor_position_from_point__gtk(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosX, size_t* pCharacterIndex)
{
    // TODO: Implement me.
    return DTK_ERROR;
}

dtk_result dtk_font_get_text_cursor_position_from_char__gtk(dtk_font* pFont, float scale, const char* text, size_t characterIndex, float* pTextCursorPosX)
{
    // TODO: Implement me.
    return DTK_ERROR;
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

void dtk_surface_draw_text__cairo(dtk_surface* pSurface, dtk_font* pFont, float scale, const char* text, size_t textLength, dtk_int32 posX, dtk_int32 posY, dtk_color fgColor, dtk_color bgColor)
{
    (void)pSurface;
    (void)pFont;
    (void)scale;
    (void)text;
    (void)textLength;
    (void)posX;
    (void)posY;
    (void)fgColor;
    (void)bgColor;
    
    // TODO: Implement me.
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

dtk_result dtk_font_get_metrics(dtk_font* pFont, float scale, dtk_font_metrics* pMetrics)
{
    if (pMetrics == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pMetrics);

    if (pFont == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pFont->backend == dtk_graphics_backend_gdi) {
        result = dtk_font_get_metrics__gdi(pFont, scale, pMetrics);
    }
#endif
#ifdef DTK_GTK
    if (pFont->backend == dtk_graphics_backend_cairo) {
        result = dtk_font_get_metrics__cairo(pFont, scale, pMetrics);
    }
#endif

    return result;
}

dtk_result dtk_font_get_glyph_metrics(dtk_font* pFont, float scale, dtk_uint32 utf32, dtk_glyph_metrics* pMetrics)
{
    if (pMetrics == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pMetrics);

    if (pFont == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pFont->backend == dtk_graphics_backend_gdi) {
        result = dtk_font_get_glyph_metrics__gdi(pFont, scale, utf32, pMetrics);
    }
#endif
#ifdef DTK_GTK
    if (pFont->backend == dtk_graphics_backend_cairo) {
        result = dtk_font_get_glyph_metrics__cairo(pFont, scale, utf32, pMetrics);
    }
#endif

    return result;
}

dtk_result dtk_font_measure_string(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float* pWidth, float* pHeight)
{
    if (pWidth) *pWidth = 0;
    if (pHeight) *pHeight = 0;
    if (pFont == NULL || text == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pFont->backend == dtk_graphics_backend_gdi) {
        result = dtk_font_measure_string__gdi(pFont, scale, text, textSizeInBytes, pWidth, pHeight);
    }
#endif
#ifdef DTK_GTK
    if (pFont->backend == dtk_graphics_backend_cairo) {
        result = dtk_font_measure_string__cairo(pFont, scale, text, textSizeInBytes, pWidth, pHeight);
    }
#endif

    return result;
}

dtk_result dtk_font_get_text_cursor_position_from_point(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosX, size_t* pCharacterIndex)
{
    if (pTextCursorPosX) *pTextCursorPosX = 0;
    if (pCharacterIndex) *pCharacterIndex = 0;
    if (pFont == NULL || text == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pFont->backend == dtk_graphics_backend_gdi) {
        result = dtk_font_get_text_cursor_position_from_point__gdi(pFont, scale, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosX, pCharacterIndex);
    }
#endif
#ifdef DTK_GTK
    if (pFont->backend == dtk_graphics_backend_cairo) {
        result = dtk_font_get_text_cursor_position_from_point__cairo(pFont, scale, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosX, pCharacterIndex);
    }
#endif

    return result;
}

dtk_result dtk_font_get_text_cursor_position_from_char(dtk_font* pFont, float scale, const char* text, size_t characterIndex, float* pTextCursorPosX)
{
    if (pTextCursorPosX) *pTextCursorPosX = 0;
    if (pFont == NULL || text == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pFont->backend == dtk_graphics_backend_gdi) {
        result = dtk_font_get_text_cursor_position_from_char__gdi(pFont, scale, text, characterIndex, pTextCursorPosX);
    }
#endif
#ifdef DTK_GTK
    if (pFont->backend == dtk_graphics_backend_cairo) {
        result = dtk_font_get_text_cursor_position_from_char__cairo(pFont, scale, text, characterIndex, pTextCursorPosX);
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

void dtk_surface_draw_text(dtk_surface* pSurface, dtk_font* pFont, float scale, const char* text, size_t textLength, dtk_int32 posX, dtk_int32 posY, dtk_color fgColor, dtk_color bgColor)
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
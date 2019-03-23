// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_color dtk_color_transparent = {0,   0,   0,   0  };
dtk_color dtk_color_black       = {0,   0,   0,   255};
dtk_color dtk_color_white       = {255, 255, 255, 255};
dtk_color dtk_color_red         = {255, 0,   0,   255};
dtk_color dtk_color_green       = {0,   255, 0,   255};
dtk_color dtk_color_blue        = {0,   0,   255, 255};

dtk_color dtk_parse_color(const char* color)
{
    if (color != NULL) {
        color = dtk_first_non_whitespace(color);
        if (color[0] == '#' || (color[0] == '0' && (color[1] == 'x' || color[1] == 'X'))) {
            // HTML and hexidecimal style. Support both 0xRRGGBB and 0xRGB format (and #RRGGBB, #RGB).
            if (color[0] == '#') {
                color += 1;
            } else {
                color += 2;
            }

            unsigned int hexvals[6];

            int len = 0;
            for (int i = 0; i < 6; ++i) {
                if (!dtk_hex_char_to_uint(color[i], &hexvals[i])) {
                    break;
                }
                len += 1;
            }

            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;

            if (len == 3) {
                // 0xRGB -> 0xRRGGBB
                hexvals[5] = hexvals[2]; hexvals[4] = hexvals[2];
                hexvals[3] = hexvals[1]; hexvals[2] = hexvals[1];
                hexvals[1] = hexvals[0]; /*hexvals[0] = hexvals[0];*/
            }

            r = (uint8_t)((hexvals[0] << 4) | hexvals[1]);
            g = (uint8_t)((hexvals[2] << 4) | hexvals[3]);
            b = (uint8_t)((hexvals[4] << 4) | hexvals[5]);

            return dtk_rgb(r, g, b);
        } else {
            // R G B style (0 .. 255 per component)
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;

            char c[4];
            color = dtk_next_token(color, c, sizeof(c));
            if (color != NULL) {
                r = (uint8_t)atoi(c);
            }

            color = dtk_next_token(color, c, sizeof(c));
            if (color != NULL) {
                g = (uint8_t)atoi(c);
            }

            color = dtk_next_token(color, c, sizeof(c));
            if (color != NULL) {
                b = (uint8_t)atoi(c);
            }

            return dtk_rgb(r, g, b);
        }
    }

    return dtk_rgb(0, 0, 0);
}

dtk_result dtk_color_to_string(dtk_color color, char* str, size_t strCap)
{
    /* Using hexidecimal format. The alpha component is left out if it's equal to 255 (fully opaque). */
    if (str == NULL || strCap == 0) {
        return DTK_INVALID_ARGS;
    }

    if (color.a == 255) {
        if (strCap < 9) {
            return DTK_INVALID_ARGS;    /* Output buffer too small. */
        }
    } else {
        if (strCap < 11) {
            return DTK_INVALID_ARGS;    /* Output buffer too small. */
        }
    }

    str[0] = '0';
    str[1] = 'x';

    // R
    dtk_uint_to_hex_char((color.r >> 4), &str[2]);
    dtk_uint_to_hex_char((color.r & 15), &str[3]);

    // G
    dtk_uint_to_hex_char((color.g >> 4), &str[4]);
    dtk_uint_to_hex_char((color.g & 15), &str[5]);

    // B
    dtk_uint_to_hex_char((color.b >> 4), &str[6]);
    dtk_uint_to_hex_char((color.b & 15), &str[7]);

    // A. Leave this out if it's 255.
    if (color.a != 255) {
        dtk_uint_to_hex_char((color.a >> 4), &str[8]);
        dtk_uint_to_hex_char((color.a & 15), &str[9]);
        str[10] = '\0';
    } else {
        str[8] = '\0';
    }

    return DTK_SUCCESS;
}

// RGBA8 <-> BGRA8 swap with alpha pre-multiply and vertical flip.
void dtk__rgba8_bgra8_swap__premul_flip(const void* pSrc, void* pDst, unsigned int width, unsigned int height, unsigned int srcStride, unsigned int dstStride)
{
    assert(pSrc != NULL);
    assert(pDst != NULL);

    const unsigned int srcStride32 = srcStride/4;
    const unsigned int dstStride32 = dstStride/4;

    for (unsigned int iRow = 0; iRow < height; ++iRow) {
        const unsigned int* pSrcRow = (const unsigned int*)pSrc + (iRow * srcStride32);
              unsigned int* pDstRow =       (unsigned int*)pDst + ((height - iRow - 1) * dstStride32);
              //unsigned int* pDstRow =       (unsigned int*)pDst + (iRow * dstStride32);

        for (unsigned int iCol = 0; iCol < width; ++iCol) {
            unsigned int srcTexel = pSrcRow[iCol];
            unsigned int srcTexelA = (srcTexel & 0xFF000000) >> 24;
            unsigned int srcTexelB = (srcTexel & 0x00FF0000) >> 16;
            unsigned int srcTexelG = (srcTexel & 0x0000FF00) >> 8;
            unsigned int srcTexelR = (srcTexel & 0x000000FF) >> 0;

            srcTexelB = (unsigned int)(srcTexelB * (srcTexelA / 255.0f));
            srcTexelG = (unsigned int)(srcTexelG * (srcTexelA / 255.0f));
            srcTexelR = (unsigned int)(srcTexelR * (srcTexelA / 255.0f));

            pDstRow[iCol] = (srcTexelR << 16) | (srcTexelG << 8) | (srcTexelB << 0) | (srcTexelA << 24);
        }
    }
}

// RGBA8 <-> BGRA8 swap with alpha pre-multiply.
void dtk__rgba8_bgra8_swap__premul(const void* pSrc, void* pDst, unsigned int width, unsigned int height, unsigned int srcStride, unsigned int dstStride)
{
    assert(pSrc != NULL);
    assert(pDst != NULL);

    const unsigned int srcStride32 = srcStride/4;
    const unsigned int dstStride32 = dstStride/4;

    for (unsigned int iRow = 0; iRow < height; ++iRow) {
        const unsigned int* pSrcRow = (const unsigned int*)pSrc + (iRow * srcStride32);
              unsigned int* pDstRow =       (unsigned int*)pDst + (iRow * dstStride32);

        for (unsigned int iCol = 0; iCol < width; ++iCol) {
            unsigned int srcTexel = pSrcRow[iCol];
            unsigned int srcTexelA = (srcTexel & 0xFF000000) >> 24;
            unsigned int srcTexelB = (srcTexel & 0x00FF0000) >> 16;
            unsigned int srcTexelG = (srcTexel & 0x0000FF00) >> 8;
            unsigned int srcTexelR = (srcTexel & 0x000000FF) >> 0;

            srcTexelB = (unsigned int)(srcTexelB * (srcTexelA / 255.0f));
            srcTexelG = (unsigned int)(srcTexelG * (srcTexelA / 255.0f));
            srcTexelR = (unsigned int)(srcTexelR * (srcTexelA / 255.0f));

            pDstRow[iCol] = (srcTexelR << 16) | (srcTexelG << 8) | (srcTexelB << 0) | (srcTexelA << 24);
        }
    }
}

// Creates a new subfont.
dtk_result dtk_font__init_subfont(dtk_font* pFont, float scale, dtk_subfont* pSubfont);
dtk_result dtk_font__uninit_subfont(dtk_font* pFont, dtk_subfont* pSubfont);

// Function used for consistently and reliably converting a scale to a size in tenths.
dtk_int32 dtk_font__convert_scale_to_size_in_tens(dtk_font* pFont, float scale)
{
    dtk_assert(pFont != NULL);
    dtk_assert(scale > 0);

    return (dtk_int32)(pFont->size * scale) * 10;
}

// Retrieves a cached subfont of the given scale. Returns NULL if the font with the specified scale does not exist.
dtk_subfont* dtk_font__get_cached_subfont(dtk_font* pFont, float scale)
{
    dtk_assert(pFont != NULL);
    dtk_assert(scale > 0);

    dtk_int32 sizeInTens = dtk_font__convert_scale_to_size_in_tens(pFont, scale);
    dtk_assert(sizeInTens > 0);

    for (dtk_uint32 i = 0; i < pFont->cachedSubfontCount; ++i) {
        if (pFont->cachedSubfonts[i].sizeInTens == sizeInTens) {
            return &pFont->cachedSubfonts[i];
        }
    }

    return NULL;
}

// Acquires a new subfont and caches it.
dtk_subfont* dtk_font__acquire_subfont(dtk_font* pFont, float scale)
{
    dtk_assert(pFont != NULL);
    dtk_assert(scale > 0);

    // First check if the scaled font is cached.
    dtk_subfont* pSubfont = dtk_font__get_cached_subfont(pFont, scale);
    if (pSubfont != NULL) {
        return pSubfont;
    }

    // If we get here it means the subfont is not cached. We'll now need to kick one of the cached sub-fonts out.
    if (pFont->cachedSubfontCount == 0) {
        // This is a special case. It means we are acquiring the base subfont, which must always have a scale of 1.
        dtk_assert(scale == 1);
        pSubfont = &pFont->cachedSubfonts[0];
        if (dtk_font__init_subfont(pFont, scale, pSubfont) != DTK_SUCCESS) {
            return NULL;
        }

        pFont->cachedSubfontCount = 1;
        pFont->oldestCachedFontIndex = 1;
    } else {
        // The base subfont has already been cached. We need to make sure this one is never kicked out of the cache.
        if (pFont->cachedSubfontCount < DTK_MAX_CACHED_SUBFONT_COUNT) {
            // There's room in the cache so no need to kick anything out yet.
            pSubfont = &pFont->cachedSubfonts[pFont->cachedSubfontCount];
            if (dtk_font__init_subfont(pFont, scale, pSubfont) != DTK_SUCCESS) {
                return NULL;
            }

            pFont->cachedSubfontCount += 1;
        } else {
            // There's no room in the cache. The oldest one needs to be removed.
            dtk_uint32 subfontIndex = pFont->oldestCachedFontIndex;
            pSubfont = &pFont->cachedSubfonts[subfontIndex];
            dtk_font__uninit_subfont(pFont, pSubfont);
            if (dtk_font__init_subfont(pFont, scale, pSubfont) != DTK_SUCCESS) {
                // Because we have just uninitialized the oldest subfont our entire cache is corrupted. We need to
                // reset the entire thing, making sure we leave the base font in place.
                for (dtk_uint32 i = 1; i < subfontIndex; ++i) {
                    dtk_font__uninit_subfont(pFont, &pFont->cachedSubfonts[i]);
                }
                for (dtk_uint32 i = subfontIndex+1; i < pFont->cachedSubfontCount; ++i) {
                    dtk_font__uninit_subfont(pFont, &pFont->cachedSubfonts[i]);
                }

                pFont->cachedSubfontCount = 1;
                pFont->oldestCachedFontIndex = 1;
                return NULL;
            }

            pFont->oldestCachedFontIndex += 1;
            if (pFont->oldestCachedFontIndex >= pFont->cachedSubfontCount) {
                pFont->oldestCachedFontIndex = 1;
            }
        }
    }

    return pSubfont;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// GDI
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

dtk_result dtk_surface__push_saved_state(dtk_surface* pSurface, dtk_surface_saved_state state);
dtk_result dtk_surface__pop_saved_state(dtk_surface* pSurface, dtk_surface_saved_state* pState);

#ifdef DTK_WIN32
// Fonts
// =====
dtk_result dtk_font__init_subfont__win32(dtk_font* pFont, float scale, dtk_subfont* pSubfont)
{
    dtk_assert(pFont != NULL);
    dtk_assert(scale > 0);

    dtk_context* pTK = pFont->pTK;

    LONG weightGDI = FW_REGULAR;
    switch (pFont->weight)
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
    if (pFont->slant == dtk_font_slant_italic || pFont->slant == dtk_font_slant_oblique) {
        slantGDI = TRUE;
    }

	LOGFONTA logfont;
	memset(&logfont, 0, sizeof(logfont));
    logfont.lfHeight      = -(LONG)(pFont->size * scale);
	logfont.lfWeight      = weightGDI;
	logfont.lfItalic      = slantGDI;
	logfont.lfCharSet     = DEFAULT_CHARSET;
    logfont.lfQuality     = (pFont->optionFlags & DTK_FONT_FLAG_NO_CLEARTYPE) ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY;
    logfont.lfEscapement  = 0;
    logfont.lfOrientation = 0;
    dtk_strncpy_s(logfont.lfFaceName, sizeof(logfont.lfFaceName), pFont->family, _TRUNCATE);

    pSubfont->gdi.hFont = (dtk_handle)CreateFontIndirectA(&logfont);
    if (pSubfont->gdi.hFont == NULL) {
        return DTK_ERROR;
    }


    // Retrieving font metrics is quite slow with GDI so we'll cache it.
    HGDIOBJ hPrevFont = SelectObject((HDC)pTK->win32.hGraphicsDC, (HFONT)pSubfont->gdi.hFont);
    {
        TEXTMETRIC metrics;
        GetTextMetrics((HDC)pTK->win32.hGraphicsDC, &metrics);
        pSubfont->gdi.metrics.ascent     = metrics.tmAscent;
        pSubfont->gdi.metrics.descent    = metrics.tmDescent;
        pSubfont->gdi.metrics.lineHeight = metrics.tmHeight;

        const MAT2 transform = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};        // <-- Identity matrix

        GLYPHMETRICS spaceMetrics;
        DWORD bitmapBufferSize = GetGlyphOutlineW((HDC)pTK->win32.hGraphicsDC, ' ', GGO_NATIVE, &spaceMetrics, 0, NULL, &transform);
        if (bitmapBufferSize == GDI_ERROR) {
			pSubfont->gdi.metrics.spaceWidth = 4;
        } else {
            pSubfont->gdi.metrics.spaceWidth = spaceMetrics.gmCellIncX;
        }
    }
    SelectObject((HDC)pTK->win32.hGraphicsDC, hPrevFont);


    return DTK_SUCCESS;
}

dtk_result dtk_font__uninit_subfont__win32(dtk_font* pFont, dtk_subfont* pSubfont)
{
    dtk_assert(pFont != NULL);
    dtk_assert(pSubfont != NULL);

    (void)pFont;
    DeleteObject((HGDIOBJ)pSubfont->gdi.hFont);
    
    return DTK_SUCCESS;
}


dtk_result dtk_font_init__gdi(dtk_context* pTK, const char* family, float size, dtk_font_weight weight, dtk_font_slant slant, dtk_uint32 optionFlags, dtk_font* pFont)
{
    // Everything with GDI fonts is done as part of the sub-font system.
    (void)pTK;
    (void)family;
    (void)size;
    (void)weight;
    (void)slant;
    (void)optionFlags;
    
    pFont->backend = dtk_graphics_backend_gdi;
    return DTK_SUCCESS;
}

dtk_result dtk_font_uninit__gdi(dtk_font* pFont)
{
    (void)pFont;
    return DTK_SUCCESS;
}

dtk_result dtk_font_get_metrics__gdi(dtk_font* pFont, float scale, dtk_font_metrics* pMetrics)
{
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return DTK_ERROR;
    }

    *pMetrics = pSubfont->gdi.metrics;
    return DTK_SUCCESS;
}

dtk_result dtk_font_get_glyph_metrics__gdi(dtk_font* pFont, float scale, dtk_uint32 utf32, dtk_glyph_metrics* pMetrics)
{
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return DTK_ERROR;
    }

    dtk_result result = DTK_ERROR;
    HGDIOBJ hPrevFont = SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, (HFONT)pSubfont->gdi.hFont);
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

dtk_result dtk_font_measure_string__gdi(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, dtk_int32* pWidth, dtk_int32* pHeight)
{
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return DTK_ERROR;
    }

    dtk_result result = DTK_ERROR;
    HGDIOBJ hPrevFont = SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, (HFONT)pSubfont->gdi.hFont);
    {
        size_t textWLength;
        wchar_t* textW = dtk__mb_to_wchar__win32(pFont->pTK, text, textSizeInBytes, &textWLength);
        if (textW != NULL) {
            SIZE sizeWin32;
            if (GetTextExtentPoint32W((HDC)pFont->pTK->win32.hGraphicsDC, textW, (int)textWLength, &sizeWin32)) {
                if (pWidth)  *pWidth  = (dtk_int32)sizeWin32.cx;
                if (pHeight) *pHeight = (dtk_int32)sizeWin32.cy;
                result = DTK_SUCCESS;
            }
        }
    }
    SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, hPrevFont);

    return result;
}

dtk_result dtk_font_get_text_cursor_position_from_point__gdi(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosX, size_t* pCharacterIndex)
{
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return DTK_ERROR;
    }

    dtk_result result = DTK_ERROR;
    HGDIOBJ hPrevFont = SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, (HFONT)pSubfont->gdi.hFont);
    {
        GCP_RESULTSW results;
        ZeroMemory(&results, sizeof(results));
        results.lStructSize = sizeof(results);
        results.nGlyphs     = (UINT)textSizeInBytes;

        size_t textWLength;
        wchar_t* textW = dtk__mb_to_wchar__win32(pFont->pTK, text, textSizeInBytes, &textWLength);
        if (textW != NULL) {
            if (results.nGlyphs > pFont->pTK->win32.glyphCacheSize) {
                pFont->pTK->win32.pGlyphCache = (dtk_int32*)dtk_realloc(pFont->pTK->win32.pGlyphCache, results.nGlyphs * sizeof(*pFont->pTK->win32.pGlyphCache));
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
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return DTK_ERROR;
    }

    dtk_result result = DTK_ERROR;
    HGDIOBJ hPrevFont = SelectObject((HDC)pFont->pTK->win32.hGraphicsDC, (HFONT)pSubfont->gdi.hFont);
    {
        GCP_RESULTSW results;
        ZeroMemory(&results, sizeof(results));
        results.lStructSize = sizeof(results);
        results.nGlyphs     = (DWORD)(characterIndex + 1);

        size_t textWLength;
        wchar_t* textW = dtk__mb_to_wchar__win32(pFont->pTK, text, (int)results.nGlyphs, &textWLength);
        if (textW != NULL) {
            if (results.nGlyphs > pFont->pTK->win32.glyphCacheSize) {
                pFont->pTK->win32.pGlyphCache = (dtk_int32*)dtk_realloc(pFont->pTK->win32.pGlyphCache, results.nGlyphs * sizeof(*pFont->pTK->win32.pGlyphCache));
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
dtk_result dtk_surface_init_transient_HDC(dtk_context* pTK, dtk_handle hDC, dtk_uint32 width, dtk_uint32 height, dtk_surface* pSurface)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pSurface);

    if (pTK == NULL || hDC == NULL) return DTK_INVALID_ARGS;
    pSurface->pTK = pTK;
    pSurface->backend = dtk_graphics_backend_gdi;
    pSurface->width  = width;
    pSurface->height = height;
    pSurface->isTransient = DTK_TRUE;
    pSurface->gdi.hDC = (HDC)hDC;
    SetGraphicsMode((HDC)pSurface->gdi.hDC, GM_ADVANCED);    // <-- Needed for world transforms (rotate and scale).

    return DTK_SUCCESS;
}

dtk_result dtk_surface_init_image__gdi(dtk_context* pTK, dtk_uint32 width, dtk_uint32 height, dtk_uint32 strideInBytes, const void* pImageData, dtk_surface* pSurface)
{
    (void)pTK;

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth       = (LONG)width;
    bmi.bmiHeader.biHeight      = (LONG)height;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;   // Only supporting 32-bit formats.
    bmi.bmiHeader.biCompression = BI_RGB;
    pSurface->gdi.hBitmap = CreateDIBSection((HDC)pTK->win32.hGraphicsDC, &bmi, DIB_RGB_COLORS, (void**)&pSurface->gdi.pBitmapData, NULL, 0);
    if (pSurface->gdi.hBitmap == NULL) {
        return DTK_ERROR;
    }

    // We need to convert the data so it renders correctly with AlphaBlend().
    if (pImageData != NULL) {
        dtk__rgba8_bgra8_swap__premul_flip(pImageData, pSurface->gdi.pBitmapData, width, height, strideInBytes, width*4);
    }

    // Flush GDI to let it know we are finished with the bitmap object's data.
    GdiFlush();

    pSurface->backend = dtk_graphics_backend_gdi;
    return DTK_SUCCESS;
}

dtk_result dtk_surface_uninit__gdi(dtk_surface* pSurface)
{
    (void)pSurface;

    if (pSurface->isImage) {
        DeleteObject(pSurface->gdi.hBitmap);
    }

    return DTK_SUCCESS;
}


dtk_result dtk_surface_push__gdi(dtk_surface* pSurface)
{
    int token = SaveDC((HDC)pSurface->gdi.hDC);
    if (token == 0) {
        return DTK_ERROR;
    }

    dtk_surface_saved_state state;
    state.gdi.token = token;
    dtk_surface__push_saved_state(pSurface, state);

    return DTK_SUCCESS;
}

dtk_result dtk_surface_pop__gdi(dtk_surface* pSurface)
{
    dtk_surface_saved_state state;
    dtk_result result = dtk_surface__pop_saved_state(pSurface, &state);
    if (result != DTK_SUCCESS) {
        return result;
    }

    //RestoreDC((HDC)pSurface->gdi.hDC, state.gdi.token);
    RestoreDC((HDC)pSurface->gdi.hDC, -1);
    return DTK_SUCCESS;
}


dtk_result dtk_surface_translate__gdi(dtk_surface* pSurface, dtk_int32 offsetX, dtk_int32 offsetY)
{
    //OffsetViewportOrgEx((HDC)pSurface->gdi.hDC, offsetX, offsetY, NULL);

    XFORM transform = {0};
    transform.eM11 = 1;
    transform.eM22 = 1;
    transform.eDx = (float)offsetX;
    transform.eDy = (float)offsetY;
    ModifyWorldTransform((HDC)pSurface->gdi.hDC, &transform, MWT_LEFTMULTIPLY);

    return DTK_SUCCESS;
}

dtk_result dtk_surface_rotate__gdi(dtk_surface* pSurface, float rotationOffsetInDegrees)
{
    float rotationOffsetInRadians = dtk_radiansf(rotationOffsetInDegrees);
    float c = cosf(rotationOffsetInRadians);
    float s = sinf(rotationOffsetInRadians);

    XFORM transform = {0};
    transform.eM11 =  c;
    transform.eM12 =  s;
    transform.eM21 = -s;
    transform.eM22 =  c;
    ModifyWorldTransform((HDC)pSurface->gdi.hDC, &transform, MWT_LEFTMULTIPLY);

    return DTK_SUCCESS;
}

dtk_result dtk_surface_scale__gdi(dtk_surface* pSurface, float scaleX, float scaleY)
{
    XFORM transform = {0};
    transform.eM11 = scaleX;
    transform.eM22 = scaleY;
    ModifyWorldTransform((HDC)pSurface->gdi.hDC, &transform, MWT_LEFTMULTIPLY);

    return DTK_SUCCESS;
}


void dtk_surface_clear__gdi(dtk_surface* pSurface, dtk_color color)
{
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(NULL_PEN));
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor((HDC)pSurface->gdi.hDC, RGB(color.r, color.g, color.b));
    Rectangle((HDC)pSurface->gdi.hDC, 0, 0, (int)pSurface->width, (int)pSurface->height);
}

void dtk_surface_set_clip__gdi(dtk_surface* pSurface, dtk_rect rect)
{
    SelectClipRgn((HDC)pSurface->gdi.hDC, NULL);
    IntersectClipRect((HDC)pSurface->gdi.hDC, rect.left, rect.top, rect.right, rect.bottom);
}

void dtk_surface_get_clip__gdi(dtk_surface* pSurface, dtk_rect* pRect)
{
    RECT rect;
    GetClipBox((HDC)pSurface->gdi.hDC, &rect);

    pRect->left   = rect.left;
    pRect->top    = rect.top;
    pRect->right  = rect.right;
    pRect->bottom = rect.bottom;
}

void dtk_surface_draw_rect__gdi(dtk_surface* pSurface, dtk_rect rect, dtk_color color)
{
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(NULL_PEN));
    SelectObject((HDC)pSurface->gdi.hDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor((HDC)pSurface->gdi.hDC, RGB(color.r, color.g, color.b));
    Rectangle((HDC)pSurface->gdi.hDC, rect.left, rect.top, rect.right, rect.bottom);
}

void dtk_surface_draw_rect_outline__gdi(dtk_surface* pSurface, dtk_rect rect, dtk_color color, dtk_int32 outlineWidth)
{
    HDC hDC = (HDC)pSurface->gdi.hDC;

    SelectObject(hDC, GetStockObject(NULL_PEN));
    SelectObject(hDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hDC, RGB(color.r, color.g, color.b));

    // Now draw the rectangle. The documentation for this says that the width and height is 1 pixel less when the pen is null. Therefore we will
    // increase the width and height by 1 since we have got the pen set to null.

    Rectangle(hDC, rect.left,                 rect.top,                   rect.left  + outlineWidth, rect.bottom);              // Left.
    Rectangle(hDC, rect.right - outlineWidth, rect.top,                   rect.right,                rect.bottom);              // Right.
    Rectangle(hDC, rect.left  + outlineWidth, rect.top,                   rect.right - outlineWidth, rect.top + outlineWidth);  // Top
    Rectangle(hDC, rect.left  + outlineWidth, rect.bottom - outlineWidth, rect.right - outlineWidth, rect.bottom);              // Bottom
}

void dtk_surface_draw_text__gdi(dtk_surface* pSurface, dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, dtk_int32 posX, dtk_int32 posY, dtk_color fgColor, dtk_color bgColor)
{
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return;
    }

    HDC hDC = (HDC)pSurface->gdi.hDC;    // For ease of use.

    size_t textWLength;
    wchar_t* textW = dtk__mb_to_wchar__win32(pSurface->pTK, text, textSizeInBytes, &textWLength);
    if (textW != NULL) {
        SelectObject(hDC, (HFONT)pSubfont->gdi.hFont);

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
            GetTextExtentPoint32W(hDC, textW, (int)textWLength, &textSize);
            rect.left   = (LONG)posX;
            rect.top    = (LONG)posY;
            rect.right  = (LONG)(posX + textSize.cx);
            rect.bottom = (LONG)(posY + textSize.cy);
        }

        SetTextColor(hDC, RGB(fgColor.r, fgColor.g, fgColor.b));
        ExtTextOutW(hDC, (int)posX, (int)posY, options, &rect, textW, (int)textWLength, NULL);
    }
}

void dtk_surface_draw_surface__gdi__mix(void* pSrc, void* pDst, unsigned int width, unsigned int height, unsigned int srcStride, unsigned int dstStride, dtk_color foregroundColor, dtk_color backgroundColor)
{
    if (pSrc == NULL || pDst == NULL) return;

    const unsigned int srcStride32 = (srcStride == 0) ? width : (srcStride/4);
    const unsigned int dstStride32 = (dstStride == 0) ? width : (dstStride/4);

    for (unsigned int iRow = 0; iRow < height; ++iRow) {
        const unsigned int* pSrcRow = (const unsigned int*)pSrc + (iRow * srcStride32);
              unsigned int* pDstRow =       (unsigned int*)pDst + (iRow * dstStride32);

        for (unsigned int iCol = 0; iCol < width; ++iCol) {
            unsigned int srcTexel = pSrcRow[iCol];

            float srcTexelA = ((srcTexel & 0xFF000000) >> 24) / 255.0f;
            float srcTexelR = ((srcTexel & 0x00FF0000) >> 16) / 255.0f;
            float srcTexelG = ((srcTexel & 0x0000FF00) >> 8)  / 255.0f;
            float srcTexelB = ((srcTexel & 0x000000FF) >> 0)  / 255.0f;

            float foregroundColorA = foregroundColor.a / 255.0f;
            float foregroundColorR = foregroundColor.r / 255.0f;
            float foregroundColorG = foregroundColor.g / 255.0f;
            float foregroundColorB = foregroundColor.b / 255.0f;

            float backgroundColorA = backgroundColor.a / 255.0f;
            float backgroundColorR = backgroundColor.r / 255.0f;
            float backgroundColorG = backgroundColor.g / 255.0f;
            float backgroundColorB = backgroundColor.b / 255.0f;

            // Mix foreground color.
            srcTexelR = srcTexelR * (foregroundColorR * foregroundColorA);
            srcTexelG = srcTexelG * (foregroundColorG * foregroundColorA);
            srcTexelB = srcTexelB * (foregroundColorB * foregroundColorA);
            srcTexelA = srcTexelA * (foregroundColorA * foregroundColorA);

            // Mix background color.
            srcTexelR = ((backgroundColorR * (1.0f - srcTexelA)) * backgroundColorA) + srcTexelR;
            srcTexelG = ((backgroundColorG * (1.0f - srcTexelA)) * backgroundColorA) + srcTexelG;
            srcTexelB = ((backgroundColorB * (1.0f - srcTexelA)) * backgroundColorA) + srcTexelB;
            srcTexelA = backgroundColorA + (srcTexelA * (1.0f - backgroundColorA));

            // Construct.
            unsigned int srcTexelAU32 = (unsigned int)(srcTexelA * 255);
            unsigned int srcTexelRU32 = (unsigned int)(srcTexelR * 255);
            unsigned int srcTexelGU32 = (unsigned int)(srcTexelG * 255);
            unsigned int srcTexelBU32 = (unsigned int)(srcTexelB * 255);

            if (srcTexelAU32 > 255) srcTexelA = 255;
            if (srcTexelRU32 > 255) srcTexelR = 255;
            if (srcTexelGU32 > 255) srcTexelG = 255;
            if (srcTexelBU32 > 255) srcTexelB = 255;

            pDstRow[iCol] = (srcTexelRU32 << 16) | (srcTexelGU32 << 8) | (srcTexelBU32 << 0) | (srcTexelAU32 << 24);
        }
    }
}

void dtk_surface_draw_surface__gdi(dtk_surface* pDstSurface, dtk_surface* pSrcSurface, dtk_draw_image_args* pArgs)
{
    HDC hDstDC = (HDC)pDstSurface->gdi.hDC;
    HDC hSrcDC = (HDC)pSrcSurface->gdi.hDC;

    if (pSrcSurface->isImage) {
        hSrcDC = (HDC)pDstSurface->pTK->win32.hGraphicsDC;
        SelectObject(hSrcDC, pSrcSurface->gdi.hBitmap);
    }

    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

    // There is a simplified path when the foreground color is white and the background color is transparent.
    if (dtk_color_equal(pArgs->foregroundColor, dtk_color_white) && pArgs->backgroundColor.a == 0) {
        // Faster path. We can just output directly without any modification.
        if (pArgs->options & DTK_SURFACE_HINT_NO_ALPHA) {
            StretchBlt(hDstDC, (int)pArgs->dstX, (int)pArgs->dstY, (int)pArgs->dstWidth, (int)pArgs->dstHeight, hSrcDC, (int)pArgs->srcX, (int)pArgs->srcY, (int)pArgs->srcWidth, (int)pArgs->srcHeight, SRCCOPY);
        } else {
            ((DTK_PFN_AlphaBlend)pSrcSurface->pTK->win32.AlphaBlend)(hDstDC, (int)pArgs->dstX, (int)pArgs->dstY, (int)pArgs->dstWidth, (int)pArgs->dstHeight, hSrcDC, (int)pArgs->srcX, (int)pArgs->srcY, (int)pArgs->srcWidth, (int)pArgs->srcHeight, blend);
        }

        return;
    }


    // Slower path. Need to draw to an intermediary DC for the background and foreground colors.
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth       = (LONG)pSrcSurface->width;
    bmi.bmiHeader.biHeight      = (LONG)pSrcSurface->height;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;   // Only supporting 32-bit formats.
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pTempImageData;
    HBITMAP hTempBitmap = CreateDIBSection(hSrcDC, &bmi, DIB_RGB_COLORS, (void**)&pTempImageData, NULL, 0);
    if (hTempBitmap != NULL) {
        if (pSrcSurface->gdi.pBitmapData != NULL) {
            // We have direct access to the bitmap data so we can run this on an optimized path.
            dtk_surface_draw_surface__gdi__mix(pSrcSurface->gdi.pBitmapData, pTempImageData, pSrcSurface->width, pSrcSurface->height, 0, 0, pArgs->foregroundColor, pArgs->backgroundColor);
        } else {
            // We don't have direct access to the bitmap data so we'll need to retrieve it. Doing this properly is very ineffeciant - we need to
            // create a temporary DIB section, draw the source surface into it, then do our mixing as per normal.
            void* pTempSrcImageData;
            HBITMAP hTempSrcBitmap = CreateDIBSection(hSrcDC, &bmi, DIB_RGB_COLORS, (void**)&pTempSrcImageData, NULL, 0);
            if (hTempSrcBitmap != NULL) {
                HDC hTempSrcDC = CreateCompatibleDC(hSrcDC);
                SelectObject(hTempSrcDC, hTempSrcBitmap);
                BitBlt(hTempSrcDC, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, hSrcDC, 0, 0, SRCCOPY);
                GdiFlush();

                dtk_surface_draw_surface__gdi__mix(pTempSrcImageData, pTempImageData, pSrcSurface->width, pSrcSurface->height, 0, 0, pArgs->foregroundColor, pArgs->backgroundColor);

                DeleteDC(hTempSrcDC);
            }
        }
    }

    GdiFlush();

    SelectObject(hSrcDC, hTempBitmap);
    ((DTK_PFN_AlphaBlend)pSrcSurface->pTK->win32.AlphaBlend)(hDstDC, (int)pArgs->dstX, (int)pArgs->dstY, (int)pArgs->dstWidth, (int)pArgs->dstHeight, hSrcDC, (int)pArgs->srcX, (int)pArgs->srcY, (int)pArgs->srcWidth, (int)pArgs->srcHeight, blend);

    DeleteObject(hTempBitmap);
}

void dtk_surface_draw_raw_image_rgba__gdi(dtk_surface* pSurface, dtk_uint32 imageWidth, dtk_uint32 imageHeight, dtk_uint32 strideInBytes, const void* pImageData, dtk_draw_image_args* pArgs)
{
    // Fow now, just create an image surface and draw it. This can be optimized later if necessary.
    dtk_surface tempSurface;
    dtk_result result = dtk_surface_init_image(pSurface->pTK, imageWidth, imageHeight, strideInBytes, pImageData, &tempSurface);
    if (result != DTK_SUCCESS) {
        return;
    }

    dtk_surface_draw_surface(pSurface, &tempSurface, pArgs);
    dtk_surface_uninit(&tempSurface);
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
dtk_font_weight dtk_font_weight_from_pango(PangoWeight weight)
{
    if (weight == PANGO_WEIGHT_THIN) {
        return dtk_font_weight_thin;
    } else if (weight == PANGO_WEIGHT_ULTRALIGHT) {
        return dtk_font_weight_extra_light;
    } else if (weight == PANGO_WEIGHT_LIGHT) {
        return dtk_font_weight_light;
    } else if (weight == PANGO_WEIGHT_SEMILIGHT) {
        return dtk_font_weight_semi_light;
    } else if (weight == PANGO_WEIGHT_BOOK) {
        return dtk_font_weight_book;
    } else if (weight == PANGO_WEIGHT_NORMAL) {
        return dtk_font_weight_normal;
    } else if (weight == PANGO_WEIGHT_MEDIUM) {
        return dtk_font_weight_medium;
    } else if (weight == PANGO_WEIGHT_SEMIBOLD) {
        return dtk_font_weight_semi_bold;
    } else if (weight == PANGO_WEIGHT_BOLD) {
        return dtk_font_weight_bold;
    } else if (weight == PANGO_WEIGHT_ULTRABOLD) {
        return dtk_font_weight_extra_bold;
    } else if (weight == PANGO_WEIGHT_HEAVY) {
        return dtk_font_weight_heavy;
    } else if (weight == PANGO_WEIGHT_ULTRAHEAVY) {
        return dtk_font_weight_extra_heavy;
    } else {
        return dtk_font_weight_normal;
    }
}

dtk_font_slant dtk_font_slant_from_pango(PangoStyle slant)
{
    if (slant == PANGO_STYLE_OBLIQUE) {
        return dtk_font_slant_oblique;
    } else if (slant == PANGO_STYLE_ITALIC) {
        return dtk_font_slant_italic;
    } else {
        return dtk_font_slant_none;
    }
}

dtk_result dtk_font__init_subfont__cairo(dtk_font* pFont, float scale, dtk_subfont* pSubfont)
{
    dtk_assert(pFont != NULL);
    dtk_assert(scale > 0);


    cairo_matrix_t fontMatrix;
    cairo_matrix_init_scale(&fontMatrix, (double)(pFont->size * scale), (double)(pFont->size * scale));

    cairo_matrix_t ctm;
    cairo_matrix_init_identity(&ctm);

    cairo_font_options_t* options = cairo_font_options_create();
    cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_SUBPIXEL);    // TODO: Control this with optionFlags.

    pSubfont->cairo.pFont = cairo_scaled_font_create((cairo_font_face_t*)pFont->cairo.pFace, &fontMatrix, &ctm, options);
    if (pSubfont->cairo.pFont == NULL) {
        cairo_font_face_destroy((cairo_font_face_t*)pFont->cairo.pFace);
        return DTK_ERROR;
    }


    // Metrics are cached.
    cairo_font_extents_t fontMetrics;
    cairo_scaled_font_extents((cairo_scaled_font_t*)pSubfont->cairo.pFont, &fontMetrics);

    pSubfont->cairo.metrics.ascent     = fontMetrics.ascent;
    pSubfont->cairo.metrics.descent    = fontMetrics.descent;
    //pSubfont->cairo.metrics.lineHeight = fontMetrics.height;
    pSubfont->cairo.metrics.lineHeight = fontMetrics.ascent + fontMetrics.descent;

    // The width of a space needs to be retrieved via glyph metrics.
    const char space[] = " ";
    cairo_text_extents_t spaceMetrics;
    cairo_scaled_font_text_extents((cairo_scaled_font_t*)pSubfont->cairo.pFont, space, &spaceMetrics);
    pSubfont->cairo.metrics.spaceWidth = spaceMetrics.x_advance;


    return DTK_SUCCESS;
}

dtk_result dtk_font__uninit_subfont__cairo(dtk_font* pFont, dtk_subfont* pSubfont)
{
    dtk_assert(pFont != NULL);
    dtk_assert(pSubfont != NULL);

    (void)pFont;
    cairo_scaled_font_destroy((cairo_scaled_font_t*)pSubfont->cairo.pFont);
    
    return DTK_SUCCESS;
}

dtk_result dtk_font_init__cairo(dtk_context* pTK, const char* family, float size, dtk_font_weight weight, dtk_font_slant slant, dtk_uint32 optionFlags, dtk_font* pFont)
{
    cairo_font_slant_t cairoSlant = CAIRO_FONT_SLANT_NORMAL;
    if (pFont->slant == dtk_font_slant_italic) {
        cairoSlant = CAIRO_FONT_SLANT_ITALIC;
    } else if (pFont->slant == dtk_font_slant_oblique) {
        cairoSlant = CAIRO_FONT_SLANT_OBLIQUE;
    }

    cairo_font_weight_t cairoWeight = CAIRO_FONT_WEIGHT_NORMAL;
    if (pFont->weight == dtk_font_weight_bold || pFont->weight == dtk_font_weight_semi_bold || pFont->weight == dtk_font_weight_extra_bold || pFont->weight == dtk_font_weight_heavy) {
        cairoWeight = CAIRO_FONT_WEIGHT_BOLD;
    }

    pFont->cairo.pFace = cairo_toy_font_face_create(pFont->family, cairoSlant, cairoWeight);
    if (pFont->cairo.pFace == NULL) {
        return DTK_ERROR;
    }

    pFont->backend = dtk_graphics_backend_cairo;
    return DTK_SUCCESS;
}

dtk_result dtk_font_uninit__cairo(dtk_font* pFont)
{
    cairo_font_face_destroy((cairo_font_face_t*)pFont->cairo.pFace);

    return DTK_SUCCESS;
}

dtk_result dtk_font_get_metrics__cairo(dtk_font* pFont, float scale, dtk_font_metrics* pMetrics)
{
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return DTK_ERROR;
    }

    *pMetrics = pSubfont->cairo.metrics;
    return DTK_SUCCESS;
}

dtk_result dtk_font_get_glyph_metrics__cairo(dtk_font* pFont, float scale, dtk_uint32 utf32, dtk_glyph_metrics* pMetrics)
{
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return DTK_ERROR;
    }

    // The UTF-32 code point needs to be converted to a UTF-8 character.
    char utf8[16];
    size_t utf8len = dtk_utf32_to_utf8_ch(utf32, utf8, sizeof(utf8)); // This will null-terminate.
    if (utf8len == 0) {
        return DTK_ERROR;   // Error converting UTF-32 to UTF-8.
    }


    cairo_text_extents_t glyphExtents;
    cairo_scaled_font_text_extents((cairo_scaled_font_t*)pSubfont->cairo.pFont, utf8, &glyphExtents);

    pMetrics->width    = glyphExtents.width;
    pMetrics->height   = glyphExtents.height;
    pMetrics->originX  = glyphExtents.x_bearing;
    pMetrics->originY  = glyphExtents.y_bearing;
    pMetrics->advanceX = glyphExtents.x_advance;
    pMetrics->advanceY = glyphExtents.y_advance;

    return DTK_SUCCESS;
}

dtk_result dtk_font_measure_string__cairo(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, dtk_int32* pWidth, dtk_int32* pHeight)
{
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return DTK_ERROR;
    }

    // Cairo expends null terminated strings, however the input string is not guaranteed to be null terminated.
    char* textNT;
    if (textSizeInBytes != (size_t)-1) {
        textNT = (char*)dtk_malloc(textSizeInBytes + 1);
        if (textNT == NULL) {
            return DTK_ERROR;
        }
        memcpy(textNT, text, textSizeInBytes);
        textNT[textSizeInBytes] = '\0';
    } else {
        textNT = (char*)text;
    }


    cairo_text_extents_t textMetrics;
    cairo_scaled_font_text_extents((cairo_scaled_font_t*)pSubfont->cairo.pFont, textNT, &textMetrics);

    if (pWidth) {
        *pWidth = (dtk_int32)textMetrics.x_advance;
    }
    if (pHeight) {
        //*pHeight = textMetrics.height;
        *pHeight = (dtk_int32)(pSubfont->cairo.metrics.ascent + pSubfont->cairo.metrics.descent);
    }


    if (textNT != text) {
        dtk_free(textNT);
    }

    return DTK_SUCCESS;
}

dtk_result dtk_font_get_text_cursor_position_from_point__cairo(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosX, size_t* pCharacterIndex)
{
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return DTK_ERROR;
    }

    cairo_glyph_t* pGlyphs = NULL;
    int glyphCount = 0;
    cairo_status_t result = cairo_scaled_font_text_to_glyphs((cairo_scaled_font_t*)pSubfont->cairo.pFont, 0, 0, text, textSizeInBytes, &pGlyphs, &glyphCount, NULL, NULL, NULL);
    if (result != CAIRO_STATUS_SUCCESS) {
        return DTK_ERROR;
    }

    float cursorPosX = 0;
    int charIndex = 0;

    // We just iterate over each glyph until we find the one sitting under <inputPosX>.
    float runningPosX = 0;
    for (int iGlyph = 0; iGlyph < glyphCount; ++iGlyph) {
        cairo_text_extents_t glyphMetrics;
        cairo_scaled_font_glyph_extents((cairo_scaled_font_t*)pSubfont->cairo.pFont, pGlyphs + iGlyph, 1, &glyphMetrics);

        float glyphLeft  = runningPosX;
        float glyphRight = glyphLeft + glyphMetrics.x_advance;

        // Are we sitting on top of inputPosX?
        if (inputPosX >= glyphLeft && inputPosX <= glyphRight) {
            float glyphHalf = glyphLeft + ceilf(((glyphRight - glyphLeft) / 2.0f));
            if (inputPosX <= glyphHalf) {
                cursorPosX = glyphLeft;
                charIndex  = iGlyph;
            } else {
                cursorPosX = glyphRight;
                charIndex  = iGlyph + 1;
            }

            break;
        } else {
            // Have we moved past maxWidth?
            if (glyphRight > maxWidth) {
                cursorPosX = maxWidth;
                charIndex  = iGlyph;
                break;
            } else {
                runningPosX = glyphRight;

                cursorPosX = runningPosX;
                charIndex  = iGlyph;
            }
        }
    }

    cairo_glyph_free(pGlyphs);

    if (pTextCursorPosX) *pTextCursorPosX = cursorPosX;
    if (pCharacterIndex) *pCharacterIndex = charIndex;
    return DTK_SUCCESS;
}

dtk_result dtk_font_get_text_cursor_position_from_char__cairo(dtk_font* pFont, float scale, const char* text, size_t characterIndex, float* pTextCursorPosX)
{
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return DTK_ERROR;
    }

    cairo_glyph_t* pGlyphs = NULL;
    int glyphCount = 0;
    cairo_status_t result = cairo_scaled_font_text_to_glyphs((cairo_scaled_font_t*)pSubfont->cairo.pFont, 0, 0, text, -1, &pGlyphs, &glyphCount, NULL, NULL, NULL);
    if (result != CAIRO_STATUS_SUCCESS) {
        return DTK_ERROR;
    }

    float cursorPosX = 0;

    // We just iterate over each glyph until we find the one sitting under <inputPosX>.
    for (int iGlyph = 0; iGlyph < glyphCount; ++iGlyph) {
        if (iGlyph == (int)characterIndex) {
            break;
        }

        cairo_text_extents_t glyphMetrics;
        cairo_scaled_font_glyph_extents((cairo_scaled_font_t*)pSubfont->cairo.pFont, pGlyphs + iGlyph, 1, &glyphMetrics);

        cursorPosX += glyphMetrics.x_advance;
    }

    cairo_glyph_free(pGlyphs);

    if (pTextCursorPosX) *pTextCursorPosX = cursorPosX;
    return DTK_SUCCESS;
}


// Surfaces
// ========
dtk_result dtk_surface_init_transient_cairo(dtk_context* pTK, dtk_ptr pCairoContext, dtk_uint32 width, dtk_uint32 height, dtk_surface* pSurface)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pSurface);

    if (pTK == NULL || pCairoContext == NULL) return DTK_INVALID_ARGS;
    pSurface->pTK = pTK;
    pSurface->backend = dtk_graphics_backend_cairo;
    pSurface->width  = width;
    pSurface->height = height;
    pSurface->isTransient = DTK_TRUE;
    pSurface->cairo.pContext = pCairoContext;
    pSurface->cairo.pSurface = (dtk_ptr)cairo_get_target((cairo_t*)pCairoContext);

    return DTK_SUCCESS;
}

dtk_result dtk_surface_init_image__cairo(dtk_context* pTK, dtk_uint32 width, dtk_uint32 height, dtk_uint32 strideInBytes, const void* pImageData, dtk_surface* pSurface)
{
    // The image data needs to be converted from RGBA to ARGB for cairo.
    dtk_uint32 srcStrideInBytes = (dtk_uint32)cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, (int)width);

    void* pImageDataARGB = dtk_malloc(srcStrideInBytes * height);
    if (pImageDataARGB == NULL) {
        return DTK_OUT_OF_MEMORY;
    }

    // TODO: CAIRO_FORMAT_ARGB32 is in native endian, so may want to do a big-endian rgba8 -> argb8 swap.
    dtk__rgba8_bgra8_swap__premul(pImageData, pImageDataARGB, width, height, strideInBytes, srcStrideInBytes);

    cairo_surface_t* pCairoSurface = cairo_image_surface_create_for_data((unsigned char*)pImageDataARGB, CAIRO_FORMAT_ARGB32, (int)width, (int)height, (int)width*4);
    if (pCairoSurface == NULL) {
        dtk_free(pImageDataARGB);
        return DTK_ERROR;
    }

    pSurface->cairo.pSurface = (dtk_ptr)pCairoSurface;
    pSurface->cairo.pContext = (dtk_ptr)cairo_create(pCairoSurface);
    pSurface->cairo.pImageData = pImageDataARGB;

    return DTK_SUCCESS;
}

dtk_result dtk_surface_uninit__cairo(dtk_surface* pSurface)
{
    if (!pSurface->isTransient) {
        cairo_destroy((cairo_t*)pSurface->cairo.pContext);
        cairo_surface_destroy((cairo_surface_t*)pSurface->cairo.pSurface);
    }

    if (pSurface->cairo.pImageData) {
        dtk_free(pSurface->cairo.pImageData);
    }
    
    return DTK_SUCCESS;
}


dtk_result dtk_surface_push__cairo(dtk_surface* pSurface)
{
    cairo_save((cairo_t*)pSurface->cairo.pContext);
    return DTK_SUCCESS;
}

dtk_result dtk_surface_pop__cairo(dtk_surface* pSurface)
{
    cairo_restore((cairo_t*)pSurface->cairo.pContext);
    return DTK_SUCCESS;
}


dtk_result dtk_surface_translate__cairo(dtk_surface* pSurface, dtk_int32 offsetX, dtk_int32 offsetY)
{
    cairo_translate((cairo_t*)pSurface->cairo.pContext, (double)offsetX, (double)offsetY);
    return DTK_SUCCESS;
}

dtk_result dtk_surface_rotate__cairo(dtk_surface* pSurface, float rotationOffsetInDegrees)
{
    cairo_rotate((cairo_t*)pSurface->cairo.pContext, dtk_radians(rotationOffsetInDegrees));
    return DTK_SUCCESS;
}

dtk_result dtk_surface_scale__cairo(dtk_surface* pSurface, float scaleX, float scaleY)
{
    cairo_scale((cairo_t*)pSurface->cairo.pContext, (double)scaleX, (double)scaleY);
    return DTK_SUCCESS;
}


void dtk_surface_clear__cairo(dtk_surface* pSurface, dtk_color color)
{
    cairo_t* cr = (cairo_t*)pSurface->cairo.pContext;

    cairo_set_source_rgba(cr, color.r/255.0, color.g/255.0, color.b/255.0, color.a/255.0);
    cairo_paint(cr);
}

void dtk_surface_set_clip__cairo(dtk_surface* pSurface, dtk_rect rect)
{
    cairo_t* cr = (cairo_t*)pSurface->cairo.pContext;

    cairo_reset_clip(cr);
    cairo_rectangle(cr, rect.left, rect.top, (rect.right - rect.left), (rect.bottom - rect.top));
    cairo_clip(cr);
}

void dtk_surface_get_clip__cairo(dtk_surface* pSurface, dtk_rect* pRect)
{
    double left;
    double top;
    double right;
    double bottom;
    cairo_clip_extents((cairo_t*)pSurface->cairo.pContext, &left, &top, &right, &bottom);

    pRect->left   = (dtk_int32)left;
    pRect->top    = (dtk_int32)top;
    pRect->right  = (dtk_int32)right;
    pRect->bottom = (dtk_int32)bottom;
}

void dtk_surface_draw_rect__cairo(dtk_surface* pSurface, dtk_rect rect, dtk_color color)
{
    cairo_t* cr = (cairo_t*)pSurface->cairo.pContext;

    cairo_set_source_rgba(cr, color.r/255.0, color.g/255.0, color.b/255.0, color.a/255.0);
    cairo_rectangle(cr, rect.left, rect.top, (rect.right - rect.left), (rect.bottom - rect.top));
    cairo_fill(cr);
}

void dtk_surface_draw_rect_outline__cairo(dtk_surface* pSurface, dtk_rect rect, dtk_color color, dtk_int32 outlineWidth)
{
    cairo_t* cr = (cairo_t*)pSurface->cairo.pContext;

    cairo_set_source_rgba(cr, color.r/255.0, color.g/255.0, color.b/255.0, color.a/255.0);

    // We do this as 4 separate rectangles... but I can't remember why...
    cairo_rectangle(cr, rect.left, rect.top, outlineWidth, rect.bottom - rect.top);                                                     // Left
    cairo_fill(cr);
    cairo_rectangle(cr, rect.right - outlineWidth, rect.top, outlineWidth, rect.bottom - rect.top);                                     // Right
    cairo_fill(cr);
    cairo_rectangle(cr, rect.left + outlineWidth, rect.top, rect.right - rect.left - (outlineWidth*2), outlineWidth);                   // Top
    cairo_fill(cr);
    cairo_rectangle(cr, rect.left + outlineWidth, rect.bottom - outlineWidth, rect.right - rect.left - (outlineWidth*2), outlineWidth); // Bottom
    cairo_fill(cr);
}

void dtk_surface_draw_text__cairo(dtk_surface* pSurface, dtk_font* pFont, float scale, const char* text, size_t textLength, dtk_int32 posX, dtk_int32 posY, dtk_color fgColor, dtk_color bgColor)
{
    dtk_subfont* pSubfont = dtk_font__acquire_subfont(pFont, scale);
    if (pSubfont == NULL) {
        return;
    }
    
    cairo_t* cr = (cairo_t*)pSurface->cairo.pContext;

    // Cairo expends null terminated strings, however the input string is not guaranteed to be null terminated.
    char* textNT;
    if (textLength != (size_t)-1) {
        textNT = (char*)dtk_malloc(textLength + 1);
        memcpy(textNT, text, textLength);
        textNT[textLength] = '\0';
    } else {
        textNT = (char*)text;
    }


    cairo_set_scaled_font(cr, (cairo_scaled_font_t*)pSubfont->cairo.pFont);

    // Background.
    cairo_text_extents_t textMetrics;
    cairo_text_extents(cr, textNT, &textMetrics);
    cairo_set_source_rgba(cr, bgColor.r / 255.0, bgColor.g / 255.0, bgColor.b / 255.0, bgColor.a / 255.0);
    cairo_rectangle(cr, posX, posY, textMetrics.x_advance, pSubfont->cairo.metrics.lineHeight);
    cairo_fill(cr);

    // Text.
    cairo_move_to(cr, posX, posY + pSubfont->cairo.metrics.ascent);
    cairo_set_source_rgba(cr, fgColor.r / 255.0, fgColor.g / 255.0, fgColor.b / 255.0, fgColor.a / 255.0);
    cairo_show_text(cr, textNT);


    if (textNT != text) {
        dtk_free(textNT);
    }
}

void dtk_surface_draw_surface__cairo(dtk_surface* pSurface, dtk_surface* pSrcSurface, dtk_draw_image_args* pArgs)
{
    cairo_t* cr = (cairo_t*)pSurface->cairo.pContext;

    cairo_save(cr);
    cairo_translate(cr, pArgs->dstX, pArgs->dstY);

    // Background.
    if ((pArgs->options & DTK_SURFACE_HINT_NO_ALPHA) == 0) {
        cairo_set_source_rgba(cr,
            pArgs->backgroundColor.r/255.0,
            pArgs->backgroundColor.g/255.0,
            pArgs->backgroundColor.b/255.0,
            pArgs->backgroundColor.a/255.0);
        cairo_rectangle(cr, 0, 0, pArgs->dstWidth, pArgs->dstHeight);
        cairo_fill(cr);
    }

#if 1
    if (pArgs->foregroundColor.r == 255 && pArgs->foregroundColor.g == 255 && pArgs->foregroundColor.b == 255 && pArgs->foregroundColor.a == 255) {
        cairo_scale(cr, pArgs->dstWidth / pArgs->srcWidth, pArgs->dstHeight / pArgs->srcHeight);
        cairo_set_source_surface(cr, (cairo_surface_t*)pSrcSurface->cairo.pSurface, pArgs->srcX, pArgs->srcY);
        cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
        cairo_paint(cr);
    } else {
        // Slower path. The image needs to be tinted. We create a temporary image for this.
        // NOTE: This is incorrect. It's just a temporary solution until I figure out a better way.
        cairo_surface_t* pTempImageSurface = cairo_surface_create_similar_image((cairo_surface_t*)pSrcSurface->cairo.pSurface, CAIRO_FORMAT_ARGB32,
            cairo_image_surface_get_width((cairo_surface_t*)pSrcSurface->cairo.pSurface), cairo_image_surface_get_height((cairo_surface_t*)pSrcSurface->cairo.pSurface));
        if (pTempImageSurface != NULL) {
            cairo_t* cr2 = cairo_create(pTempImageSurface);

            cairo_set_operator(cr2, CAIRO_OPERATOR_SOURCE);
            cairo_set_source_surface(cr2, (cairo_surface_t*)pSrcSurface->cairo.pSurface, 0, 0);
            cairo_pattern_set_filter(cairo_get_source(cr2), CAIRO_FILTER_NEAREST);
            cairo_paint(cr2);

            // Tint.
            cairo_set_operator(cr2, CAIRO_OPERATOR_ATOP);
            cairo_set_source_rgba(cr2,
                pArgs->foregroundColor.r / 255.0,
                pArgs->foregroundColor.g / 255.0,
                pArgs->foregroundColor.b / 255.0,
                pArgs->foregroundColor.a / 255.0);
            cairo_rectangle(cr2, 0, 0, pArgs->dstWidth, pArgs->dstHeight);
            cairo_fill(cr2);

            /*cairo_set_operator(cr2, CAIRO_OPERATOR_MULTIPLY);
            cairo_set_source_surface(cr2, pCairoImage->pCairoSurface, 0, 0);
            cairo_pattern_set_filter(cairo_get_source(cr2), CAIRO_FILTER_NEAREST);
            cairo_paint(cr2);*/

            // Draw the temporary surface onto the main surface.
            cairo_scale(cr, pArgs->dstWidth / pArgs->srcWidth, pArgs->dstHeight / pArgs->srcHeight);
            cairo_set_source_surface(cr, pTempImageSurface, pArgs->srcX, pArgs->srcY);
            cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
            //cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
            cairo_paint(cr);

            cairo_destroy(cr2);
            cairo_surface_destroy(pTempImageSurface);
        }
    }
#endif

    cairo_restore(cr);
}

void dtk_surface_draw_raw_image_rgba__cairo(dtk_surface* pSurface, dtk_uint32 imageWidth, dtk_uint32 imageHeight, dtk_uint32 strideInBytes, const void* pImageData, dtk_draw_image_args* pArgs)
{
    // Fow now, just create an image surface and draw it. This can be optimized later if necessary.
    dtk_surface tempSurface;
    dtk_result result = dtk_surface_init_image(pSurface->pTK, imageWidth, imageHeight, strideInBytes, pImageData, &tempSurface);
    if (result != DTK_SUCCESS) {
        return;
    }

    dtk_surface_draw_surface(pSurface, &tempSurface, pArgs);
    dtk_surface_uninit(&tempSurface);
}
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Fonts
// =====
dtk_result dtk_font__init_subfont(dtk_font* pFont, float scale, dtk_subfont* pSubfont)
{
    dtk_assert(pFont != NULL);
    dtk_assert(scale > 0);

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pFont->pTK->platform == dtk_platform_win32) {
        if (result != DTK_SUCCESS) {
            result = dtk_font__init_subfont__win32(pFont, scale, pSubfont);
        }
    }
#endif
#ifdef DTK_GTK
    if (pFont->pTK->platform == dtk_platform_gtk) {
        if (result != DTK_SUCCESS) {
            result = dtk_font__init_subfont__cairo(pFont, scale, pSubfont);
        }
    }
#endif

    pSubfont->sizeInTens = dtk_font__convert_scale_to_size_in_tens(pFont, scale);
    return result;
}

dtk_result dtk_font__uninit_subfont(dtk_font* pFont, dtk_subfont* pSubfont)
{
    dtk_assert(pFont != NULL);
    dtk_assert(pSubfont != NULL);

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pFont->pTK->platform == dtk_platform_win32) {
        if (result != DTK_SUCCESS) {
            result = dtk_font__uninit_subfont__win32(pFont, pSubfont);
        }
    }
#endif
#ifdef DTK_GTK
    if (pFont->pTK->platform == dtk_platform_gtk) {
        if (result != DTK_SUCCESS) {
            result = dtk_font__uninit_subfont__cairo(pFont, pSubfont);
        }
    }
#endif

    return result;
}

dtk_result dtk_font_init(dtk_context* pTK, const char* family, float size, dtk_font_weight weight, dtk_font_slant slant, dtk_uint32 optionFlags, dtk_font* pFont)
{
    if (pFont == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pFont);

    if (pTK == NULL || family == NULL || size == 0) return DTK_INVALID_ARGS;
    pFont->pTK = pTK;
    dtk_strcpy_s(pFont->family, sizeof(pFont->family), family);
    pFont->size = size;
    pFont->weight = weight;
    pFont->slant = slant;
    pFont->optionFlags = optionFlags;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        if (result != DTK_SUCCESS) {
            result = dtk_font_init__gdi(pTK, family, size, weight, slant, optionFlags, pFont);
        }
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        if (result != DTK_SUCCESS) {
            result = dtk_font_init__cairo(pTK, family, size, weight, slant, optionFlags, pFont);
        }
    }
#endif

    // Item 0 in the cache is always the sub font that represents the base size.
    dtk_font__acquire_subfont(pFont, 1.0f);

    return result;
}

dtk_result dtk_font_init_from_desc(dtk_context* pTK, dtk_font_desc* pDesc, dtk_uint32 optionFlags, dtk_font* pFont)
{
    if (pDesc == NULL) return DTK_INVALID_ARGS;

    return dtk_font_init(pTK, pDesc->family, pDesc->size, pDesc->weight, pDesc->slant, optionFlags, pFont);
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

dtk_result dtk_font_measure_string(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, dtk_int32* pWidth, dtk_int32* pHeight)
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
dtk_result dtk_surface_init_image(dtk_context* pTK, dtk_uint32 width, dtk_uint32 height, dtk_uint32 strideInBytes, const void* pImageData, dtk_surface* pSurface)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pSurface);

    if (pTK == NULL) return DTK_INVALID_ARGS;
    pSurface->pTK = pTK;
    pSurface->width = width;
    pSurface->height = height;
    pSurface->isImage = DTK_TRUE;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pTK->platform == dtk_platform_win32) {
        if (result != DTK_SUCCESS) {
            result = dtk_surface_init_image__gdi(pTK, width, height, strideInBytes, pImageData, pSurface);
        }
    }
#endif
#ifdef DTK_GTK
    if (pTK->platform == dtk_platform_gtk) {
        if (result != DTK_SUCCESS) {
            result = dtk_surface_init_image__cairo(pTK, width, height, strideInBytes, pImageData, pSurface);
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


dtk_result dtk_surface_get_size(dtk_surface* pSurface, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    if (pWidth) *pWidth = 0;
    if (pHeight) *pHeight = 0;

    if (pSurface == NULL) return DTK_INVALID_ARGS;

    if (pWidth) *pWidth = pSurface->width;
    if (pHeight) *pHeight = pSurface->height;

    return DTK_SUCCESS;
}

dtk_uint32 dtk_surface_get_width(dtk_surface* pSurface)
{
    if (pSurface == NULL) return 0;
    return pSurface->width;
}

dtk_uint32 dtk_surface_get_height(dtk_surface* pSurface)
{
    if (pSurface == NULL) return 0;
    return pSurface->height;
}


dtk_result dtk_surface_push(dtk_surface* pSurface)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        result = dtk_surface_push__gdi(pSurface);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        result = dtk_surface_push__cairo(pSurface);
    }
#endif

    return result;
}

dtk_result dtk_surface_pop(dtk_surface* pSurface)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        result = dtk_surface_pop__gdi(pSurface);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        result = dtk_surface_pop__cairo(pSurface);
    }
#endif

    return result;
}


dtk_result dtk_surface_translate(dtk_surface* pSurface, dtk_int32 offsetX, dtk_int32 offsetY)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        result = dtk_surface_translate__gdi(pSurface, offsetX, offsetY);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        result = dtk_surface_translate__cairo(pSurface, offsetX, offsetY);
    }
#endif

    return result;
}

dtk_result dtk_surface_rotate(dtk_surface* pSurface, float rotationOffsetInDegrees)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        result = dtk_surface_rotate__gdi(pSurface, rotationOffsetInDegrees);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        result = dtk_surface_rotate__cairo(pSurface, rotationOffsetInDegrees);
    }
#endif

    return result;
}

dtk_result dtk_surface_scale(dtk_surface* pSurface, float scaleX, float scaleY)
{
    if (pSurface == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_NO_BACKEND;
#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        result = dtk_surface_scale__gdi(pSurface, scaleX, scaleY);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        result = dtk_surface_scale__cairo(pSurface, scaleX, scaleY);
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

void dtk_surface_set_clip(dtk_surface* pSurface, dtk_rect rect)
{
    if (pSurface == NULL) return;

#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        dtk_surface_set_clip__gdi(pSurface, rect);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        dtk_surface_set_clip__cairo(pSurface, rect);
    }
#endif
}

void dtk_surface_get_clip(dtk_surface* pSurface, dtk_rect* pRect)
{
    if (pRect == NULL) return;
    *pRect = dtk_rect_init(0, 0, 0, 0);

    if (pSurface == NULL) return;

#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        dtk_surface_get_clip__gdi(pSurface, pRect);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        dtk_surface_get_clip__cairo(pSurface, pRect);
    }
#endif
}

void dtk_surface_draw_rect(dtk_surface* pSurface, dtk_rect rect, dtk_color color)
{
    if (pSurface == NULL) return;

#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        dtk_surface_draw_rect__gdi(pSurface, rect, color);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        dtk_surface_draw_rect__cairo(pSurface, rect, color);
    }
#endif
}

void dtk_surface_draw_outer_rect(dtk_surface* pSurface, dtk_rect outer, dtk_rect inner, dtk_color color)
{
    if (pSurface == NULL) return;

    // Left.
    if (outer.left < inner.left) {
        dtk_surface_draw_rect(pSurface, dtk_rect_init(outer.left, outer.top, inner.left, outer.bottom), color);
    }

    // Right.
    if (outer.right > inner.right) {
        dtk_surface_draw_rect(pSurface, dtk_rect_init(inner.right, outer.top, outer.right, outer.bottom), color);
    }

    // Top.
    if (outer.top < inner.top) {
        dtk_surface_draw_rect(pSurface, dtk_rect_init(inner.left, outer.top, inner.right, inner.top), color);
    }

    // Bottom.
    if (outer.bottom > inner.bottom) {
        dtk_surface_draw_rect(pSurface, dtk_rect_init(inner.left, inner.bottom, inner.right, outer.bottom), color);
    }
}

void dtk_surface_draw_rect_outline(dtk_surface* pSurface, dtk_rect rect, dtk_color color, dtk_int32 outlineWidth)
{
    if (pSurface == NULL || outlineWidth <= 0) return;

#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        dtk_surface_draw_rect_outline__gdi(pSurface, rect, color, outlineWidth);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        dtk_surface_draw_rect_outline__cairo(pSurface, rect, color, outlineWidth);
    }
#endif
}

void dtk_surface_draw_rect_with_outline(dtk_surface* pSurface, dtk_rect rect, dtk_color color, dtk_int32 outlineWidth, dtk_color outlineColor)
{
    if (pSurface == NULL) return;

    dtk_surface_draw_rect_outline(pSurface, rect, outlineColor, outlineWidth);
    dtk_surface_draw_rect(pSurface, dtk_rect_grow(rect, -outlineWidth), color);
}

void dtk_surface_draw_text(dtk_surface* pSurface, dtk_font* pFont, float scale, const char* text, size_t textLength, dtk_int32 posX, dtk_int32 posY, dtk_color fgColor, dtk_color bgColor)
{
    if (pSurface == NULL || pFont == NULL) return;

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

void dtk_surface_draw_surface(dtk_surface* pSurface, dtk_surface* pSrcSurface, dtk_draw_image_args* pArgs)
{
    if (pSurface == NULL || pSrcSurface == NULL || pArgs == NULL) return;

#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        dtk_surface_draw_surface__gdi(pSurface, pSrcSurface, pArgs);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        dtk_surface_draw_surface__cairo(pSurface, pSrcSurface, pArgs);
    }
#endif
}

void dtk_surface_draw_raw_image_rgba(dtk_surface* pSurface, dtk_uint32 imageWidth, dtk_uint32 imageHeight, dtk_uint32 strideInBytes, const void* pImageData, dtk_draw_image_args* pArgs)
{
    if (pSurface == NULL || pImageData == NULL || pArgs == NULL) return;

#ifdef DTK_WIN32
    if (pSurface->backend == dtk_graphics_backend_gdi) {
        dtk_surface_draw_raw_image_rgba__gdi(pSurface, imageWidth, imageHeight, strideInBytes, pImageData, pArgs);
    }
#endif
#ifdef DTK_GTK
    if (pSurface->backend == dtk_graphics_backend_cairo) {
        dtk_surface_draw_raw_image_rgba__cairo(pSurface, imageWidth, imageHeight, strideInBytes, pImageData, pArgs);
    }
#endif
}

void dtk_surface_draw_svg(dtk_surface* pSurface, dtk_svg* pSVG, dtk_draw_image_args* pArgs)
{
    if (pSurface == NULL || pSVG == NULL || pArgs == NULL) return;

    // For now all backends draw SVG's the same - by using nanosvg for the rasterization and then outputting the image via dtk_surface_draw_raw_image_rgba(). In the future
    // this may change so that each backend can do their own optimized implementation.

    // TODO: Remove this malloc/free and use something a bit more optimal.
    //
    // For now we can just create and delete a temporary buffer with malloc(), but later we'll optimize this.

    dtk_uint32 width  = pArgs->dstWidth;
    dtk_uint32 height = pArgs->dstHeight;

    void* pImageData = dtk_malloc(width * height * 4);
    if (pImageData == NULL) {
        return; // Out of memory.
    }

    dtk_result result = dtk_svg_rasterize(pSVG, pArgs->srcX, pArgs->srcY, pArgs->srcWidth, pArgs->srcHeight, 0, 0, width, height, width*4, pImageData);
    if (result != DTK_SUCCESS) {
        dtk_free(pImageData);
        return;
    }

    dtk_draw_image_args args2 = *pArgs;
    args2.srcX = 0;
    args2.srcY = 0;
    args2.srcWidth = width;
    args2.srcHeight = height;
    dtk_surface_draw_raw_image_rgba(pSurface, width, height, width*4, pImageData, &args2);

    dtk_free(pImageData);
}


void dtk_surface_draw_image(dtk_surface* pSurface, dtk_image* pImage, dtk_draw_image_args* pArgs)
{
    if (pSurface == NULL || pImage == NULL || pArgs == NULL) return;

    switch (pImage->type) {
        case dtk_image_type_raster:
        {
            dtk_surface_draw_surface(pSurface, &pImage->rasterImage, pArgs);
        } break;

        case dtk_image_type_vector: 
        {
            dtk_surface_draw_svg(pSurface, &pImage->vectorImage, pArgs);
        } break;
    }
}


dtk_result dtk_surface__push_saved_state(dtk_surface* pSurface, dtk_surface_saved_state state)
{
    dtk_assert(pSurface != NULL);

    // TODO: Remove this arbitrary restriction by just making it a heap allocated array.
    if (pSurface->savedStateStackCount >= dtk_count_of(pSurface->pSavedStateStack)) {
        return DTK_OUT_OF_RANGE;
    }

    pSurface->pSavedStateStack[pSurface->savedStateStackCount] = state;
    pSurface->savedStateStackCount += 1;

    return DTK_SUCCESS;
}

dtk_result dtk_surface__pop_saved_state(dtk_surface* pSurface, dtk_surface_saved_state* pState)
{
    dtk_assert(pSurface != NULL);
    dtk_assert(pState != NULL);
    
    if (pSurface->savedStateStackCount == 0) {
        return DTK_OUT_OF_RANGE;
    }

    *pState = pSurface->pSavedStateStack[pSurface->savedStateStackCount-1];
    pSurface->savedStateStackCount -= 1;

    return DTK_SUCCESS;
}
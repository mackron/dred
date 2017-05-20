// Copyright (C) 2017 David Reid. See included LICENSE file.

dred_gui_font* dred_font__find_subfont_by_scaled_size(dred_font* pFont, unsigned int scaledSize, size_t* pIndexOut)
{
    assert(pFont != NULL);
    assert(pIndexOut != NULL);

    for (size_t i = 0; i < pFont->subfontCount; ++i) {
        if (pFont->pSubFonts[i].scaledSize == scaledSize) {
            *pIndexOut = i;
            return pFont->pSubFonts[i].pGUIFont;
        }
    }

    return NULL;
}

dred_gui_font* dred_font__create_subfont(dred_font* pFont, unsigned int scaledSize, size_t* pIndexOut)
{
    assert(pFont != NULL);
    assert(pIndexOut != NULL);

    if (pFont->subfontCount == pFont->subfontBufferSize)
    {
        size_t newBufferSize = (pFont->subfontBufferSize == 0) ? 1 : pFont->subfontBufferSize*2;
        dred_subfont* pNewSubFonts = (dred_subfont*)realloc(pFont->pSubFonts, newBufferSize * sizeof(*pNewSubFonts));
        if (pNewSubFonts == NULL) {
            return NULL;
        }

        pFont->pSubFonts = pNewSubFonts;
        pFont->subfontBufferSize = newBufferSize;
    }

    assert(pFont->subfontCount < pFont->subfontBufferSize);

    dred_gui_font* pGUIFont = dred_gui_create_font(pFont->pDred->pGUI, pFont->desc.family, scaledSize, pFont->desc.weight, pFont->desc.slant, 0);
    if (pGUIFont == NULL) {
        return NULL;
    }

    *pIndexOut = pFont->subfontCount;
    pFont->pSubFonts[pFont->subfontCount].referenceCount = 1;
    pFont->pSubFonts[pFont->subfontCount].scaledSize = scaledSize;
    pFont->pSubFonts[pFont->subfontCount].pGUIFont = pGUIFont;
    pFont->subfontCount += 1;

    return pGUIFont;
}

void dred_font__delete_subfont_by_index(dred_font* pFont, size_t index)
{
    assert(pFont != NULL);
    assert(pFont->subfontCount > index);

    dred_gui_delete_font(pFont->pSubFonts[index].pGUIFont);

    if (index+1 < pFont->subfontCount) {
        memmove(pFont->pSubFonts + index, pFont->pSubFonts + (index+1), (pFont->subfontCount - (index+1)) * sizeof(*pFont->pSubFonts));
    }
    
    pFont->subfontCount -= 1;
}


dred_font* dred_font_create(dred_context* pDred, dred_font_desc* pDesc)
{
    if (pDred == NULL || pDesc == NULL) {
        return NULL;
    }

    dred_font* pFont = (dred_font*)malloc(sizeof(*pFont));
    if (pFont == NULL) {
        return NULL;
    }

    pFont->pDred = pDred;
    pFont->desc = *pDesc;
    pFont->subfontBufferSize = 1;
    pFont->subfontCount = 0;
    pFont->pSubFonts = (dred_subfont*)malloc(pFont->subfontBufferSize * sizeof(*pFont->pSubFonts));
    if (pFont->pSubFonts == NULL) {
		free(pFont);
        return NULL;
    }

    pFont->pLibrary = NULL;
    pFont->referenceCount = 0;


    // Create the initial font.
    size_t index;
    dred_gui_font* pGUIFont = dred_font__create_subfont(pFont, pDesc->size, &index);
    if (pGUIFont == NULL) {
        free(pFont->pSubFonts);
        free(pFont);
        return NULL;
    }

    assert(index == 0);

    return pFont;
}

void dred_font_delete(dred_font* pFont)
{
    if (pFont == NULL) {
        return;
    }

    while (pFont->subfontCount > 0) {
        dred_font__delete_subfont_by_index(pFont, pFont->subfontCount-1);   // <-- More efficient to delete back to front.
    }

    free(pFont->pSubFonts);
    free(pFont);
}


dred_gui_font* dred_font_acquire_subfont(dred_font* pFont, float scale)
{
    if (pFont == NULL) {
        return NULL;
    }

    // Not too big, not too small.
    unsigned int scaledSize = (unsigned int)(pFont->desc.size * scale);
    scaledSize = dr_clamp(scaledSize, DRED_MIN_FONT_SIZE, DRED_MAX_FONT_SIZE);

    size_t subfontIndex;
    dred_gui_font* pSubFont = dred_font__find_subfont_by_scaled_size(pFont, scaledSize, &subfontIndex);
    if (pSubFont == NULL) {
        pSubFont = dred_font__create_subfont(pFont, scaledSize, &subfontIndex);
        if (pSubFont == NULL) {
            return NULL;
        }
    }

    assert(pSubFont != NULL);
    pFont->pSubFonts[subfontIndex].referenceCount += 1;

    return pSubFont;
}

void dred_font_release_subfont(dred_font* pFont, dred_gui_font* pSubFont)
{
    if (pFont == NULL || pSubFont == NULL) {
        return;
    }

    for (size_t i = 0; i < pFont->subfontCount; ++i) {
        if (pFont->pSubFonts[i].pGUIFont == pSubFont) {
            assert(pFont->pSubFonts[i].referenceCount > 0);
            pFont->pSubFonts[i].referenceCount -= 1;
            if (pFont->pSubFonts[i].referenceCount == 0) {
                dred_font__delete_subfont_by_index(pFont, i);
            }

            break;
        }
    }
}

dr_bool32 dred_font_desc_to_string(dred_font_desc* pDesc, char* strOut, size_t strOutSize)
{
    char weightStr[256];
    if (!dred_font_weight_to_string(pDesc->weight, weightStr, sizeof(weightStr))) {
        strcpy_s(weightStr, sizeof(weightStr), "default");
    }

    char slantStr[256];
    if (!dred_font_slant_to_string(pDesc->slant, slantStr, sizeof(slantStr))) {
        strcpy_s(slantStr, sizeof(slantStr), "none");
    }

    return snprintf(strOut, strOutSize, "\"%s\" %u %s %s", pDesc->family, pDesc->size, weightStr, slantStr) > 0;
}

dr_bool32 dred_font_to_string(dred_font* pFont, char* strOut, size_t strOutSize)
{
    if (pFont == NULL) {
        return strcpy_s(strOut, strOutSize, "system-font-ui") == 0;
    }

    if (pFont->pLibrary != NULL && pFont == pFont->pDred->config.pSystemFontUI) {
        return strcpy_s(strOut, strOutSize, "system-font-ui") == 0;
    }

    if (pFont->pLibrary != NULL && pFont == pFont->pDred->config.pSystemFontMono) {
        return strcpy_s(strOut, strOutSize, "system-font-mono") == 0;
    }

    return dred_font_desc_to_string(&pFont->desc, strOut, strOutSize);
}


dr_bool32 dred_font_get_metrics(dred_font* pFont, float scale, dtk_font_metrics* pMetricsOut)
{
    if (pFont == NULL) {
        return DR_FALSE;   
    }

    dred_gui_font* pSubFont = dred_font_acquire_subfont(pFont, scale);
    if (pSubFont == NULL) {
        return DR_FALSE;
    }

    dr_bool32 result = dred_gui_get_font_metrics(pSubFont, pMetricsOut);
    
    dred_font_release_subfont(pFont, pSubFont);
    return result;
}

dr_bool32 dred_font_measure_string(dred_font* pFont, float scale, const char* text, size_t textLength, float* pWidthOut, float* pHeightOut)
{
    if (pFont == NULL) {
        return DR_FALSE;   
    }

    dred_gui_font* pSubFont = dred_font_acquire_subfont(pFont, scale);
    if (pSubFont == NULL) {
        return DR_FALSE;
    }

    dr_bool32 result = dred_gui_measure_string(pSubFont, text, textLength, pWidthOut, pHeightOut);
    
    dred_font_release_subfont(pFont, pSubFont);
    return result;
}

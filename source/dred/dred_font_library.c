// Copyright (C) 2016 David Reid. See included LICENSE file.

dred_font* dred_font_library__create_font_for_real(dred_font_library* pLibrary, dred_font_desc* pDesc)
{
    assert(pLibrary != NULL);
    assert(pDesc != NULL);

    if (pLibrary->fontCount == pLibrary->fontBufferSize)
    {
        size_t newBufferSize = (pLibrary->fontBufferSize == 0) ? 1 : pLibrary->fontBufferSize*2;
        dred_font** ppNewFonts = (dred_font**)realloc(pLibrary->ppFonts, newBufferSize * sizeof(*ppNewFonts));
        if (ppNewFonts == NULL) {
            return NULL;
        }

        pLibrary->ppFonts = ppNewFonts;
        pLibrary->fontBufferSize = newBufferSize;
    }

    assert(pLibrary->fontCount < pLibrary->fontBufferSize);

    dred_font* pFont = dred_font_create(pLibrary->pDred, pDesc);
    if (pFont == NULL) {
        return NULL;
    }

    pFont->pLibrary = pLibrary;
    pFont->referenceCount = 1;

    pLibrary->ppFonts[pLibrary->fontCount] = pFont;
    pLibrary->fontCount += 1;

    return pFont;
}

void dred_font_library__delete_font_for_real(dred_font_library* pLibrary, dred_font* pFont)
{
    assert(pLibrary != NULL);
    assert(pFont != NULL);
    assert(pLibrary == pFont->pLibrary);

    // The font needs to be removed from the list.
    for (size_t i = 0; i < pLibrary->fontCount; ++i) {
        if (pLibrary->ppFonts[i] == pFont) {
            if (i+1 < pLibrary->fontCount) {
                memmove(pLibrary->ppFonts + i, pLibrary->ppFonts + (i+1), (pLibrary->fontCount - (i+1)) * sizeof(*pLibrary->ppFonts));
            }
            break;
        }
    }

    pLibrary->fontCount -= 1;
    dred_font_delete(pFont);
}

dred_font* dred_font_library__find_by_desc(dred_font_library* pLibrary, dred_font_desc* pDesc)
{
    for (size_t i = 0; i < pLibrary->fontCount; ++i) {
        if (memcmp(&pLibrary->ppFonts[i]->desc, pDesc, sizeof(dred_font_desc)) == 0) {
            return pLibrary->ppFonts[i];
        }
    }

    return NULL;
}


dr_bool32 dred_font_library_init(dred_font_library* pLibrary, dred_context* pDred)
{
    if (pLibrary == NULL) {
        return DR_FALSE;
    }

    pLibrary->pDred = pDred;
    pLibrary->fontBufferSize = 0;
    pLibrary->fontCount = 0;
    pLibrary->ppFonts = NULL;

    return DR_TRUE;
}

void dred_font_library_uninit(dred_font_library* pLibrary)
{
    if (pLibrary == NULL) {
        return;
    }

    while (pLibrary->fontCount > 0) {
        dred_font_library__delete_font_for_real(pLibrary, pLibrary->ppFonts[pLibrary->fontCount-1]);
    }

    free(pLibrary->ppFonts);
}


dred_font* dred_font_library_create_font(dred_font_library* pLibrary, const char* family, unsigned int size, dred_gui_font_weight weight, dred_gui_font_slant slant, float rotation, unsigned int flags)
{
    dred_font_desc desc;
    if (strcpy_s(desc.family, sizeof(desc.family), family) != 0) {
        return NULL;
    }

    desc.size = size;
    desc.weight = weight;
    desc.slant = slant;
    desc.rotation = rotation;
    desc.flags = flags;

    dred_font* pFont = dred_font_library__find_by_desc(pLibrary, &desc);
    if (pFont == NULL) {
        pFont = dred_font_library__create_font_for_real(pLibrary, &desc);
        if (pFont == NULL) {
            return NULL;
        }
    } else {
        pFont->referenceCount += 1; // Already loaded.
    }

    assert(pFont != NULL);

    return pFont;
}

void dred_font_library_delete_font(dred_font_library* pLibrary, dred_font* pFont)
{
    if (pFont == NULL || pFont->referenceCount == 0) {
        return;
    }

    assert(pLibrary == pFont->pLibrary);


    pFont->referenceCount -= 1;
    if (pFont->referenceCount == 0) {
        dred_font_library__delete_font_for_real(pLibrary, pFont);
    }
}

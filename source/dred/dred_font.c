// Copyright (C) 2019 David Reid. See included LICENSE file.

dred_font* dred_font_create(dred_context* pDred, dred_font_desc* pDesc)
{
    if (pDred == NULL || pDesc == NULL) {
        return NULL;
    }

    dred_font* pFont = (dred_font*)malloc(sizeof(*pFont));
    if (pFont == NULL) {
        return NULL;
    }

    if (dtk_font_init(&pDred->tk, pDesc->family, pDesc->size, pDesc->weight, pDesc->slant, pDesc->flags, &pFont->fontDTK) != DTK_SUCCESS) {
        free(pFont);
        return NULL;
    }

    pFont->pDred = pDred;
    pFont->desc = *pDesc;
    pFont->pLibrary = NULL;
    pFont->referenceCount = 0;

    return pFont;
}

void dred_font_delete(dred_font* pFont)
{
    if (pFont == NULL) {
        return;
    }

    free(pFont);
}


dtk_bool32 dred_font_desc_to_string(dred_font_desc* pDesc, char* strOut, size_t strOutSize)
{
    char weightStr[256];
    if (!dred_font_weight_to_string(pDesc->weight, weightStr, sizeof(weightStr))) {
        strcpy_s(weightStr, sizeof(weightStr), "default");
    }

    char slantStr[256];
    if (!dred_font_slant_to_string(pDesc->slant, slantStr, sizeof(slantStr))) {
        strcpy_s(slantStr, sizeof(slantStr), "none");
    }

    return snprintf(strOut, strOutSize, "\"%s\" %f %s %s", pDesc->family, pDesc->size, weightStr, slantStr) > 0;
}

dtk_bool32 dred_font_to_string(dred_font* pFont, char* strOut, size_t strOutSize)
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


// Copyright (C) 2019 David Reid. See included LICENSE file.

typedef struct
{
    char family[64];
    float size;
    dtk_font_weight weight;
    dtk_font_slant slant;
    unsigned int flags;
} dred_font_desc;

struct dred_font
{
    dtk_font fontDTK;
    dred_context* pDred;
    dred_font_desc desc;

    dred_font_library* pLibrary;    // Can be null, in which case the font is not managed by a library.
    unsigned int referenceCount;    // Used by the font library that owns the font.
};

dred_font* dred_font_create(dred_context* pDred, dred_font_desc* pDesc);
void dred_font_delete(dred_font* pFont);

dtk_bool32 dred_font_desc_to_string(dred_font_desc* pDesc, char* strOut, size_t strOutSize);
dtk_bool32 dred_font_to_string(dred_font* pFont, char* strOut, size_t strOutSize);


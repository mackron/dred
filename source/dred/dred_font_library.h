// Copyright (C) 2017 David Reid. See included LICENSE file.

struct dred_font_library
{
    dred_context* pDred;
    size_t fontBufferSize;
    size_t fontCount;
    dred_font** ppFonts;
};

// Initializes the given font library.
dr_bool32 dred_font_library_init(dred_font_library* pLibrary, dred_context* pDred);

// Uninitializes the given font library.
void dred_font_library_uninit(dred_font_library* pLibrary);


// Creates a font, or returns a reference to an already-loaded one.
//
// Every call to dred_font_library_create_font() should be matched with a called to dred_font_library_delete_font().
dred_font* dred_font_library_create_font(dred_font_library* pLibrary, const char* family, float size, dtk_font_weight weight, dtk_font_slant slant, unsigned int flags);

// Deletes a font that was created by dred_font_library_create_font()
void dred_font_library_delete_font(dred_font_library* pLibrary, dred_font* pFont);
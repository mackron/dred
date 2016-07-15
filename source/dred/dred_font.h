
typedef struct
{
    char family[64];
    unsigned int size;
    drgui_font_weight weight;
    drgui_font_slant slant;
    float rotation;
    unsigned int flags;
} dred_font_desc;

typedef struct
{
    unsigned int referenceCount;
    unsigned int scaledSize;
    drgui_font* pGUIFont;
} dred_subfont;

struct dred_font
{
    dred_context* pDred;
    dred_font_desc desc;
    size_t subfontBufferSize;
    size_t subfontCount;
    dred_subfont* pSubFonts;

    dred_font_library* pLibrary;    // Can be null, in which case the font is not managed by a library.
    unsigned int referenceCount;    // Used by the font library that owns the font.
};

dred_font* dred_font_create(dred_context* pDred, dred_font_desc* pDesc);
void dred_font_delete(dred_font* pFont);

drgui_font* dred_font_acquire_subfont(dred_font* pFont, float scale);
void dred_font_release_subfont(dred_font* pFont, drgui_font* pSubfont);

bool dred_font_desc_to_string(dred_font_desc* pDesc, char* strOut, size_t strOutSize);
bool dred_font_to_string(dred_font* pFont, char* strOut, size_t strOutSize);

// Retrieves the metrics of a font at a given scale.
bool dred_font_get_metrics(dred_font* pFont, float scale, drgui_font_metrics* pMetricsOut);

// Measures a string using the given font and scale.
bool dred_font_measure_string(dred_font* pFont, float scale, const char* text, size_t textLength, float* pWidthOut, float* pHeightOut);

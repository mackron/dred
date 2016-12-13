// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef enum
{
    dtk_graphics_backend_gdi,
    dtk_graphics_backend_cairo,
    //dtk_graphics_backend_x11
} dtk_graphics_backend;


// Colors
// ======
typedef struct
{
    dtk_uint8 r;
    dtk_uint8 g;
    dtk_uint8 b;
    dtk_uint8 a;
} dtk_color;

DTK_INLINE dtk_color dtk_rgba(dtk_uint8 r, dtk_uint8 g, dtk_uint8 b, dtk_uint8 a)
{
    dtk_color color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

DTK_INLINE dtk_color dtk_rgb(dtk_uint8 r, dtk_uint8 g, dtk_uint8 b)
{
    return dtk_rgba(r, g, b, 255);
}


// Fonts
// =====
#define DTK_FONT_FLAG_NO_CLEARTYPE      (1 << 0)

typedef enum
{
    dtk_font_weight_medium = 0,
    dtk_font_weight_thin,
    dtk_font_weight_extra_light,
    dtk_font_weight_light,
    dtk_font_weight_semi_light,
    dtk_font_weight_book,
    dtk_font_weight_semi_bold,
    dtk_font_weight_bold,
    dtk_font_weight_extra_bold,
    dtk_font_weight_heavy,
    dtk_font_weight_extra_heavy,
    dtk_font_weight_normal  = dtk_font_weight_medium,
    dtk_font_weight_default = dtk_font_weight_medium
} dtk_font_weight;

typedef enum
{
    dtk_font_slant_none = 0,
    dtk_font_slant_italic,
    dtk_font_slant_oblique
} dtk_font_slant;

typedef struct
{
    dtk_context* pTK;
    dtk_graphics_backend backend;
    char family[128];
    float size;
    dtk_font_weight weight;
    dtk_font_slant slant;
    float rotation;
    dtk_uint32 optionFlags;
    
    union
    {
    #ifdef DTK_WIN32
        struct
        {
            /*HFONT*/ dtk_handle hFont;
        } gdi;
    #endif
    #ifdef DTK_GTK
        struct
        {
        } cairo;
    #endif
    #ifdef DTK_X11
        struct
        {
            int unused;
        } x11;
    #endif
    };
} dtk_font;

// Initializes a font.
dtk_result dtk_font_init(dtk_context* pTK, const char* family, float size, dtk_font_weight weight, dtk_font_slant slant, float rotation, dtk_uint32 optionFlags, dtk_font* pFont);

// Uninitializes a font.
dtk_result dtk_font_uninit(dtk_font* pFont);






// Surfaces
// ========
typedef struct
{
    dtk_context* pTK;
    dtk_graphics_backend backend;
    dtk_uint32 width;
    dtk_uint32 height;
    
    union
    {
    #ifdef DTK_WIN32
        struct
        {
            /*HDC*/ dtk_handle hDC;
            void* pScratchBuffer;       // For wchar_t <-> char conversions.
            size_t scratchBufferSize;
        } gdi;
    #endif
    #ifdef DTK_GTK
        struct
        {
            /*cairo_surface_t**/ dtk_ptr pSurface;
            /*cairo_t**/ dtk_ptr pContext;
        } cairo;
    #endif
    #ifdef DTK_X11
        struct
        {
            int unused;
        } x11;
    #endif
    };
} dtk_surface;

// Initializes a surface which draws directly to a window.
dtk_result dtk_surface_init_window(dtk_context* pTK, dtk_window* pWindow, dtk_surface* pSurface);

// Uninitializes a surface.
dtk_result dtk_surface_uninit(dtk_surface* pSurface);

// Clears the given surface within it's current clipping region.
void dtk_surface_clear(dtk_surface* pSurface, dtk_color color);

// Draws a quad onto the given surface.
void dtk_surface_draw_rect(dtk_surface* pSurface, dtk_int32 x, dtk_int32 y, dtk_uint32 width, dtk_uint32 height);

// Draws a run of text.
void dtk_surface_draw_text(dtk_surface* pSurface, dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, dtk_int32 posX, dtk_int32 posY, dtk_color fgColor, dtk_color bgColor);
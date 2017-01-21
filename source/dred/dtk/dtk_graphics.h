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
    dtk_int32 ascent;
    dtk_int32 descent;
    dtk_int32 lineHeight;
    dtk_int32 spaceWidth;
} dtk_font_metrics;

typedef struct
{
    dtk_int32 width;
    dtk_int32 height;
    dtk_int32 originX;
    dtk_int32 originY;
    dtk_int32 advanceX;
    dtk_int32 advanceY;
} dtk_glyph_metrics;

typedef struct
{
    union
    {
#ifdef DTK_WIN32
        struct
        {
            int token;
        } gdi;
#endif
#ifdef DTK_GTK
        struct
        {
            int unused;
        } cairo;
#endif
#ifdef DTK_X11
        struct
        {
            int unused;
        } x11;
#endif
    };
} dtk_surface_saved_state;

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
            dtk_font_metrics metrics;   // Font metrics retrieval is slow with GDI, so we cache.
        } gdi;
    #endif
    #ifdef DTK_GTK
        struct
        {
            /*cairo_font_face_t**/ dtk_ptr pFace;

            /*cairo_scaled_font_t**/ dtk_ptr pFont;
            dtk_font_metrics metrics;   // We cache font metrics on the Cairo backend for efficiency.
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

// Retrieves the metrics of a font at the given scale.
dtk_result dtk_font_get_metrics(dtk_font* pFont, float scale, dtk_font_metrics* pMetrics);

// Retrieves the metrics of a glyph when drawn with the given font at the given scale.
//
// NOTE: This API is tempoarary until an improved Unicode implementation is done.
dtk_result dtk_font_get_glyph_metrics(dtk_font* pFont, float scale, dtk_uint32 utf32, dtk_glyph_metrics* pMetrics);

// Retrieves the dimensions of a given string when drawn with the given font at the given scale.
//
// NOTE: This API is tempoarary until an improved Unicode implementation is done.
dtk_result dtk_font_measure_string(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float* pWidth, float* pHeight);

// Retrieves the position to place a text cursor based on the given point for the given string when drawn with the given font.
//
// NOTE: This API is tempoarary until an improved Unicode implementation is done.
dtk_result dtk_font_get_text_cursor_position_from_point(dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosX, size_t* pCharacterIndex);

// Retrieves the position to palce a text cursor based on the character at the given index for the given string when drawn with the given font.
//
// NOTE: This API is tempoarary until an improved Unicode implementation is done.
dtk_result dtk_font_get_text_cursor_position_from_char(dtk_font* pFont, float scale, const char* text, size_t characterIndex, float* pTextCursorPosX);




// Surfaces
// ========
#define DTK_SURFACE_HINT_NO_ALPHA   (1 << 0)

typedef enum
{
    dtk_pixel_format_rgba8,
    dtk_pixel_format_bgra8,
    dtk_pixel_format_argb8,
} dtk_pixel_format;

typedef struct
{
    dtk_context* pTK;
    dtk_graphics_backend backend;
    dtk_uint32 width;
    dtk_uint32 height;
    dtk_bool32 isTransient : 1;
    dtk_bool32 isImage     : 1;
    dtk_surface_saved_state pSavedStateStack[32];
    dtk_uint32 savedStateStackCount;
    dtk_uint32 savedStateStackCapacity;

    union
    {
    #ifdef DTK_WIN32
        struct
        {
            /*HDC*/ dtk_handle hDC;
            /*HBITMAP*/ dtk_handle hBitmap; // Only used with image surfaces.
            void* pBitmapData;              // Only used with image surfaces. It's a buffer to the internal data of hBitmap.
        } gdi;
    #endif
    #ifdef DTK_GTK
        struct
        {
            /*cairo_surface_t**/ dtk_ptr pSurface;
            /*cairo_t**/ dtk_ptr pContext;
            void* pImageData;               // Only used with image surfaces. The internal data for use internally by Cairo. ARGB32.
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

#ifdef DTK_WIN32
// Initializes a transient surface for a Win32 device context.
dtk_result dtk_surface_init_transient_HDC(dtk_context* pTK, dtk_handle hDC, dtk_uint32 width, dtk_uint32 height, dtk_surface* pSurface);
#endif

#ifdef DTK_GTK
// Initializes a transient surface for a Cairo context.
dtk_result dtk_surface_init_transient_cairo(dtk_context* pTK, dtk_ptr pCairoContext, dtk_uint32 width, dtk_uint32 height, dtk_surface* pSurface);
#endif

// Initializes a surface that's used as an image.
//
// Currently, the image data must be in simple 32-bit RGBA format (8-bits per component).
dtk_result dtk_surface_init_image(dtk_context* pTK, dtk_uint32 width, dtk_uint32 height, dtk_uint32 strideInBytes, const void* pImageData, dtk_surface* pSurface);

// Uninitializes a surface.
dtk_result dtk_surface_uninit(dtk_surface* pSurface);


// Saves a copy of the current state for the given surface, which can be restored later with dtk_surface_ppop().
dtk_result dtk_surface_push(dtk_surface* pSurface);

// Restores the last saved state.
dtk_result dtk_surface_pop(dtk_surface* pSurface);


// Clears the given surface within it's current clipping region.
void dtk_surface_clear(dtk_surface* pSurface, dtk_color color);


//// APIs below are temporary until an improved graphics API is implemented. ////

// Sets the clipping rectangle for the given surface.
void dtk_surface_set_clip(dtk_surface* pSurface, dtk_rect rect);

// Retrieves the clipping rectangle for the given surface.
void dtk_surface_get_clip(dtk_surface* pSurface, dtk_rect* pRect);

// Draws a quad onto the given surface.
void dtk_surface_draw_rect(dtk_surface* pSurface, dtk_rect rect, dtk_color color);

// Draws the outline of a rectangle.
void dtk_surface_draw_rect_outline(dtk_surface* pSurface, dtk_rect rect, dtk_color color, dtk_int32 outlineWidth);

// Draws a solid rectangle with an outline.
void dtk_surface_draw_rect_with_outline(dtk_surface* pSurface, dtk_rect rect, dtk_color color, dtk_int32 outlineWidth, dtk_color outlineColor);

// Draws a run of text.
void dtk_surface_draw_text(dtk_surface* pSurface, dtk_font* pFont, float scale, const char* text, size_t textSizeInBytes, dtk_int32 posX, dtk_int32 posY, dtk_color fgColor, dtk_color bgColor);

typedef struct
{
    dtk_int32 dstX;
    dtk_int32 dstY;
    dtk_int32 dstWidth;
    dtk_int32 dstHeight;
    dtk_int32 srcX;
    dtk_int32 srcY;
    dtk_int32 srcWidth;
    dtk_int32 srcHeight;
    dtk_color foregroundTint;
    dtk_color backgroundColor;
    dtk_uint32 options;
} dtk_draw_surface_args;

// Draws an image.
void dtk_surface_draw_surface(dtk_surface* pSurface, dtk_surface* pSrcSurface, dtk_draw_surface_args* pArgs);
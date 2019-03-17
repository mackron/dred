// Copyright (C) 2019 David Reid. See included LICENSE file.

typedef struct
{
    NSVGimage* pNanoSVGImage;    // nanosvg image.
} dtk_svg;

dtk_result dtk_svg_init(dtk_context* pTK, const char* pSVGData, dtk_svg* pSVG);
dtk_result dtk_svg_init_file(dtk_context* pTK, const char* pFilePath, dtk_svg* pSVG);
dtk_result dtk_svg_uninit(dtk_svg* pSVG);

dtk_result dtk_svg_get_size(dtk_svg* pSVG, dtk_uint32* pWidth, dtk_uint32* pHeight);
dtk_uint32 dtk_svg_get_width(dtk_svg* pSVG);
dtk_uint32 dtk_svg_get_height(dtk_svg* pSVG);

dtk_result dtk_svg_rasterize(dtk_svg* pSVG, dtk_int32 srcX, dtk_int32 srcY, dtk_uint32 srcWidth, dtk_uint32 srcHeight, dtk_int32 dstX, dtk_int32 dstY, dtk_uint32 dstWidth, dtk_uint32 dstHeight, dtk_uint32 dstStride, void* pImageDataOut);
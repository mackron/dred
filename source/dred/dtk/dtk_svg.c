// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_result dtk_svg_init__internal(dtk_context* pTK, char* pSVGData, dtk_svg* pSVG)
{
    dtk_assert(pSVGData != NULL);
    dtk_assert(pSVG != NULL);

    pSVG->pNanoSVGImage = nsvgParse(pSVGData, "px", dtk_get_system_dpi_scale(pTK));
    if (pSVG->pNanoSVGImage == NULL) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_svg_init(dtk_context* pTK, const char* pSVGData, dtk_svg* pSVG)
{
    if (pSVG == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pSVG);

    if (dtk_string_is_null_or_empty(pSVGData)) {
        return DTK_INVALID_ARGS;
    }

    // nanosvg modifies the input string, so we need to make a copy since our's is constant.
    size_t svgDataLen = strlen(pSVGData);
    char* pTempSVGData = (char*)dtk_malloc(svgDataLen+1);
    if (pTempSVGData == NULL) {
        return DTK_OUT_OF_MEMORY;
    }

    strcpy_s(pTempSVGData, svgDataLen+1, pSVGData);


    dtk_result result = dtk_svg_init__internal(pTK, pTempSVGData, pSVG);
    if (result != DTK_SUCCESS) {
        return result;
    }

    dtk_free(pTempSVGData);
    return result;
}

dtk_result dtk_svg_init_file(dtk_context* pTK, const char* pFilePath, dtk_svg* pSVG)
{
    if (pSVG == NULL) return DTK_INVALID_ARGS;
    dtk_zero_object(pSVG);

    if (dtk_string_is_null_or_empty(pFilePath)) {
        return DTK_INVALID_ARGS;
    }

    char* pSVGData;
    dtk_result result = dtk_open_and_read_text_file(pFilePath, NULL, &pSVGData);
    if (result != DTK_SUCCESS) {
        return result;
    }

    result = dtk_svg_init__internal(pTK, pSVGData, pSVG);
    
    dtk_free(pSVGData);
    return result;
}

dtk_result dtk_svg_uninit(dtk_svg* pSVG)
{
    if (pSVG != NULL && pSVG->pNanoSVGImage != NULL) {
        nsvgDelete(pSVG->pNanoSVGImage);
    }

    return DTK_SUCCESS;
}

dtk_result dtk_svg_get_size(dtk_svg* pSVG, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    if (pWidth) *pWidth = 0;
    if (pHeight) *pHeight = 0;

    if (pSVG == NULL || pSVG->pNanoSVGImage == NULL) return DTK_INVALID_ARGS;

    if (pWidth) *pWidth = (dtk_uint32)pSVG->pNanoSVGImage->width;
    if (pHeight) *pHeight = (dtk_uint32)pSVG->pNanoSVGImage->height;
    return DTK_SUCCESS;
}

dtk_uint32 dtk_svg_get_width(dtk_svg* pSVG)
{
    if (pSVG == NULL || pSVG->pNanoSVGImage == NULL) return 0;

    return (dtk_uint32)pSVG->pNanoSVGImage->width;
}

dtk_uint32 dtk_svg_get_height(dtk_svg* pSVG)
{
    if (pSVG == NULL || pSVG->pNanoSVGImage == NULL) return 0;

    return (dtk_uint32)pSVG->pNanoSVGImage->height;
}


dtk_result dtk_svg_rasterize(dtk_svg* pSVG, dtk_int32 srcX, dtk_int32 srcY, dtk_uint32 srcWidth, dtk_uint32 srcHeight, dtk_int32 dstX, dtk_int32 dstY, dtk_uint32 dstWidth, dtk_uint32 dstHeight, dtk_uint32 dstStride, void* pImageDataOut)
{
    if (pSVG == NULL || pSVG->pNanoSVGImage == NULL) return DTK_INVALID_ARGS;

    srcWidth  = dtk_min(srcWidth,  dtk_svg_get_width( pSVG));
    srcHeight = dtk_min(srcHeight, dtk_svg_get_height(pSVG));

    float dstScaleX = (float)dstWidth  / srcWidth;
    float dstScaleY = (float)dstHeight / srcHeight;

    if (dstX < 0) {
        srcWidth  += dstX;
        dstWidth  += dstX;
        srcX      -= dstX;
        dstX      -= dstX;
    }
    if (dstY < 0) {
        srcHeight += dstY;
        dstHeight += dstY;
        srcY      -= dstY;
        dstY      -= dstY;
    }

    NSVGrasterizer* pRasterizer = nsvgCreateRasterizer();
    if (pRasterizer == NULL) {
        return DTK_OUT_OF_MEMORY;
    }

    // Unfortunately, nanosvg only has a single scale as opposed to a per-axis scale. When our source and destination rectangles
    // share the same width/height ratio we can run an optimized path (this will be the most common scenario in practice), however
    // when the ratios differ we need to run a slower path which rasterizes and then non-uniformly scales.
    if (dstScaleX == dstScaleY) {
        // Fast path. Rasterize straight into the output buffer.
        float scale = dstScaleX;
        nsvgRasterize(pRasterizer, pSVG->pNanoSVGImage, (dstX-srcX)*scale, (dstY-srcY)*scale, scale, (unsigned char*)pImageDataOut, dstWidth, dstHeight, dstStride);
    } else {
        // Slow path. Requires a manual resize. What we do here is rasterize into a temporary buffer, and then resize that image
        // using a simple raster scale. We always rasterize to the larger dimension, and then downscale.
        dtk_uint32 tempWidth  = dstWidth;
        dtk_uint32 tempHeight = dstHeight;
        
        float scale;
        if (dstScaleX > dstScaleY) {
            scale = dstScaleX;
            tempHeight = (dtk_uint32)(dstHeight * scale);
        } else {
            scale = dstScaleY;
            tempWidth  = (dtk_uint32)(dstWidth  * scale);
        }

        void* pTempData = dtk_malloc(tempWidth * tempHeight * 4);
        if (pTempData == NULL) {
            nsvgDeleteRasterizer(pRasterizer);
            return DTK_OUT_OF_MEMORY;
        }

        nsvgRasterize(pRasterizer, pSVG->pNanoSVGImage, (dstX-srcX)*scale, (dstY-srcY)*scale, scale, (unsigned char*)pTempData, tempWidth, tempHeight, tempWidth*4);
        stbir_resize_uint8((const unsigned char*)pTempData, tempWidth, tempHeight, 0, (unsigned char*)pImageDataOut, dstWidth, dstHeight, 0, 4);

        dtk_free(pTempData);
    }

    nsvgDeleteRasterizer(pRasterizer);
    return DTK_SUCCESS;
}
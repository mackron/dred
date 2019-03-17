// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_result dtk_image_init(dtk_context* pTK, dtk_image_type type, dtk_image* pImage)
{
    if (pTK == NULL || pImage == NULL) return DTK_INVALID_ARGS;

    dtk_zero_object(pImage);
    pImage->type = type;

    return DTK_SUCCESS;
}

dtk_result dtk_image_init_raster(dtk_context* pTK, dtk_uint32 width, dtk_uint32 height, dtk_uint32 strideInBytes, const void* pImageData, dtk_image* pImage)
{
    dtk_result result = dtk_image_init(pTK, dtk_image_type_raster, pImage);
    if (result != DTK_SUCCESS) {
        return result;
    }

    result = dtk_surface_init_image(pTK, width, height, strideInBytes, pImageData, &pImage->rasterImage);
    if (result != DTK_SUCCESS) {
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_image_init_svg(dtk_context* pTK, const char* pSVGData, dtk_image* pImage)
{
    dtk_result result = dtk_image_init(pTK, dtk_image_type_vector, pImage);
    if (result != DTK_SUCCESS) {
        return result;
    }

    result = dtk_svg_init(pTK, pSVGData, &pImage->vectorImage);
    if (result != DTK_SUCCESS) {
        return result;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_image_uninit(dtk_image* pImage)
{
    if (pImage == NULL) return DTK_INVALID_ARGS;

    switch (pImage->type) {
        case dtk_image_type_raster: return dtk_surface_uninit(&pImage->rasterImage);
        case dtk_image_type_vector: return dtk_svg_uninit(&pImage->vectorImage);
    }

    return DTK_INVALID_ARGS;
}


dtk_result dtk_image_get_size(dtk_image* pImage, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    if (pImage == NULL) return DTK_INVALID_ARGS;

    switch (pImage->type) {
        case dtk_image_type_raster: return dtk_surface_get_size(&pImage->rasterImage, pWidth, pHeight);
        case dtk_image_type_vector: return dtk_svg_get_size(&pImage->vectorImage, pWidth, pHeight);
    }

    return DTK_INVALID_ARGS;
}

dtk_uint32 dtk_image_get_width(dtk_image* pImage)
{
    if (pImage == NULL) return 0;

    switch (pImage->type) {
        case dtk_image_type_raster: return dtk_surface_get_width(&pImage->rasterImage);
        case dtk_image_type_vector: return dtk_svg_get_width(&pImage->vectorImage);
    }

    return 0;
}

dtk_uint32 dtk_image_get_height(dtk_image* pImage)
{
    if (pImage == NULL) return 0;

    switch (pImage->type) {
        case dtk_image_type_raster: return dtk_surface_get_height(&pImage->rasterImage);
        case dtk_image_type_vector: return dtk_svg_get_height(&pImage->vectorImage);
    }

    return 0;
}


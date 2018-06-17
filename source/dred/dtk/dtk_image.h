// Copyright (C) 2018 David Reid. See included LICENSE file.

dtk_result dtk_image_init_raster(dtk_context* pTK, dtk_uint32 width, dtk_uint32 height, dtk_uint32 strideInBytes, const void* pImageData, dtk_image* pImage);
dtk_result dtk_image_init_svg(dtk_context* pTK, const char* pSVGData, dtk_image* pImage);
dtk_result dtk_image_uninit(dtk_image* pImage);

dtk_result dtk_image_get_size(dtk_image* pImage, dtk_uint32* pWidth, dtk_uint32* pHeight);
dtk_uint32 dtk_image_get_width(dtk_image* pImage);
dtk_uint32 dtk_image_get_height(dtk_image* pImage);
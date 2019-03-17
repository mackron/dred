// Copyright (C) 2019 David Reid. See included LICENSE file.

typedef struct
{
    const char* pSVGData;
} dred_image_desc_svg;

typedef struct
{
    dtk_uint32 width;
    dtk_uint32 height;
    const void* pImageData;
} dred_image_desc_raster;

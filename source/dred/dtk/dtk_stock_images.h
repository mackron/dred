// Copyright (C) 2019 David Reid. See included LICENSE file.

#define DTK_STOCK_IMAGE_COUNT   1
#define DTK_STOCK_IMAGE_CROSS   0

typedef struct
{
    dtk_image_type type;    // SVG or Raster
    dtk_uint32 width;       // Always 0 for SVG. The file defines the width.
    dtk_uint32 height;      // Always 0 for SVG. The file defines the height.
    const char* pData;      // For SVG this is the SVG content. For raster this is RGBA8 data.
} dtk_stock_image_info;

// Retrieves the SVG data of the stock image with the given ID.
dtk_stock_image_info* dtk_get_stock_image_info(dtk_uint32 stockImageID);
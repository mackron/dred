// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    int temp;
} dtk_svg;

dtk_result dtk_svg_init(dtk_svg* pSVG);
dtk_result dtk_svg_uninit(dtk_svg* pSVG);

dtk_result dtk_svg_get_size(dtk_svg* pSVG, dtk_uint32* pWidth, dtk_uint32* pHeight);
dtk_uint32 dtk_svg_get_width(dtk_svg* pSVG);
dtk_uint32 dtk_svg_get_height(dtk_svg* pSVG);
// Copyright (C) 2017 David Reid. See included LICENSE file.

dtk_result dtk_svg_init(dtk_svg* pSVG)
{
    if (pSVG == NULL) return DTK_INVALID_ARGS;

    dtk_zero_object(pSVG);


    return DTK_SUCCESS;
}

dtk_result dtk_svg_uninit(dtk_svg* pSVG)
{
    if (pSVG == NULL) return DTK_INVALID_ARGS;

    return DTK_SUCCESS;
}

dtk_result dtk_svg_get_size(dtk_svg* pSVG, dtk_uint32* pWidth, dtk_uint32* pHeight)
{
    if (pSVG == NULL) return DTK_INVALID_ARGS;

    // TODO: Implement me.
    if (pWidth) *pWidth = 0;
    if (pHeight) *pHeight = 0;

    return DTK_SUCCESS;
}

dtk_uint32 dtk_svg_get_width(dtk_svg* pSVG)
{
    if (pSVG == NULL) return 0;

    // TODO: Implement me.
    return 0;
}

dtk_uint32 dtk_svg_get_height(dtk_svg* pSVG)
{
    if (pSVG == NULL) return 0;

    // TODO: Implement me.
    return 0;
}
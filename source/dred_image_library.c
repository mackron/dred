
#define DRED_IMAGE_COUNT 1
#define DRED_IMAGE_SCALE_COUNT 3

uint8_t g_ImageData_Cross_0[] = {
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF
};

uint8_t g_ImageData_Cross_1[] = {
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF
};

uint8_t g_ImageData_Cross_2[] = {
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF
};

dred_image_desc g_ImageDesc[DRED_IMAGE_COUNT][DRED_IMAGE_SCALE_COUNT] = {
    {{1.0f, 2, 2, drgui_image_format_rgba8, g_ImageData_Cross_0},
     {1.5f, 2, 2, drgui_image_format_rgba8, g_ImageData_Cross_1},
     {2.0f, 2, 2, drgui_image_format_rgba8, g_ImageData_Cross_2}},
};

bool dred_image_library_init__autogened(dred_image_library* pLibrary, dred_context* pDred)
{
    assert(pLibrary != NULL);
    assert(pDred != NULL);

    for (unsigned int i = 0; i < DRED_IMAGE_COUNT; ++i) {
        dred_image_library_create_image(pLibrary, i, g_ImageDesc[i], DRED_IMAGE_SCALE_COUNT);
    }

    return true;
}



dred_image* dred_image_library__create_image_for_real(dred_image_library* pLibrary, unsigned int id, dred_image_desc* pDesc, size_t descCount)
{
    assert(pLibrary != NULL);
    assert(pDesc != NULL);

    if (pLibrary->imageCount == pLibrary->imageBufferSize)
    {
        size_t newBufferSize = (pLibrary->imageBufferSize == 0) ? 1 : pLibrary->imageBufferSize*2;
        dred_image** ppNewFonts = (dred_image**)realloc(pLibrary->ppImages, newBufferSize * sizeof(*ppNewFonts));
        if (ppNewFonts == NULL) {
            return NULL;
        }

        pLibrary->ppImages = ppNewFonts;
        pLibrary->imageBufferSize = newBufferSize;
    }

    assert(pLibrary->imageCount < pLibrary->imageBufferSize);

    dred_image* pImage = dred_image_create(pLibrary->pDred, id, pDesc, descCount);
    if (pImage == NULL) {
        return NULL;
    }

    pImage->pLibrary = pLibrary;
    pImage->referenceCount = 1;

    pLibrary->ppImages[pLibrary->imageCount] = pImage;
    pLibrary->imageCount += 1;

    return pImage;
}

void dred_image_library__delete_image_for_real(dred_image_library* pLibrary, dred_image* pImage)
{
    assert(pLibrary != NULL);
    assert(pImage != NULL);
    assert(pLibrary == pImage->pLibrary);

    // The image needs to be removed from the list.
    for (size_t i = 0; i < pLibrary->imageCount; ++i) {
        if (pLibrary->ppImages[i] == pImage) {
            if (i+1 < pLibrary->imageCount) {
                memmove(pLibrary->ppImages + i, pLibrary->ppImages + (i+1), pLibrary->imageCount - (i+1));
            }
            break;
        }
    }

    pLibrary->imageCount -= 1;
    dred_image_delete(pImage);
}

dred_image* dred_image_library__find_by_desc(dred_image_library* pLibrary, dred_image_desc* pDesc)
{
    for (size_t i = 0; i < pLibrary->imageCount; ++i) {
        if (memcmp(&pLibrary->ppImages[i]->desc, pDesc, sizeof(dred_image_desc)) == 0) {
            return pLibrary->ppImages[i];
        }
    }

    return NULL;
}


bool dred_image_library_init(dred_image_library* pLibrary, dred_context* pDred)
{
    if (pLibrary == NULL) {
        return false;
    }

    pLibrary->pDred = pDred;
    pLibrary->imageBufferSize = 0;
    pLibrary->imageCount = 0;
    pLibrary->ppImages = NULL;

    return dred_image_library_init__autogened(pLibrary, pDred);
    //return true;
}

void dred_image_library_uninit(dred_image_library* pLibrary)
{
    if (pLibrary == NULL) {
        return;
    }

    while (pLibrary->imageCount > 0) {
        dred_image_library__delete_image_for_real(pLibrary, pLibrary->ppImages[pLibrary->imageCount-1]);
    }

    free(pLibrary->ppImages);
}


dred_image* dred_image_library_create_image(dred_image_library* pLibrary, unsigned int id, dred_image_desc* pDesc, size_t descCount)
{
    dred_image* pImage = dred_image_library__create_image_for_real(pLibrary, id, pDesc, descCount);
    if (pImage == NULL) {
        return NULL;
    }

    assert(pImage != NULL);

    return pImage;
}

void dred_image_library_delete_image(dred_image_library* pLibrary, dred_image* pImage)
{
    if (pImage == NULL || pImage->referenceCount == 0) {
        return;
    }

    assert(pLibrary == pImage->pLibrary);


    pImage->referenceCount -= 1;
    if (pImage->referenceCount == 0) {
        dred_image_library__delete_image_for_real(pLibrary, pImage);
    }
}


dred_image* dred_image_library_get_image_by_id(dred_image_library* pLibrary, unsigned int id)
{
    if (pLibrary == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < pLibrary->imageCount; ++i) {
        if (pLibrary->ppImages[i]->id == id) {
            return pLibrary->ppImages[i];
        }
    }

    return NULL;
}

void dred_image__delete_subimage_by_index(dred_image* pImage, size_t index)
{
    assert(pImage != NULL);
    assert(pImage->subimageCount > index);

    drgui_delete_image(pImage->pSubImages[index].pGUIImage);

    if (index+1 < pImage->subimageCount) {
        memmove(pImage->pSubImages + index, pImage->pSubImages + (index+1), pImage->subimageCount - (index+1));
    }
    
    pImage->subimageCount -= 1;
}

dred_image* dred_image_create(dred_context* pDred, unsigned int id, const dred_image_desc* pDesc, size_t descCount)
{
    if (pDred == NULL || pDesc == NULL || descCount == 0) {
        return NULL;
    }

    dred_image* pImage = (dred_image*)malloc(sizeof(*pImage));
    if (pImage == NULL) {
        return NULL;
    }

    pImage->pDred = pDred;
    pImage->id = id;
    pImage->desc = *pDesc;
    pImage->subimageCount = 0;
    pImage->pSubImages = (dred_subimage*)malloc(descCount * sizeof(*pImage->pSubImages));
    if (pImage->pSubImages == NULL) {
        return NULL;
    }

    pImage->pLibrary = NULL;
    pImage->referenceCount = 0;


    // Create each sub-image; one for each pDesc.
    for (size_t iSubImage = 0; iSubImage < descCount; ++iSubImage)
    {
        pImage->pSubImages[pImage->subimageCount].scale = pDesc[iSubImage].scale;
        pImage->pSubImages[pImage->subimageCount].referenceCount = 1;
        pImage->pSubImages[pImage->subimageCount].pGUIImage = drgui_create_image(pDred->pGUI, pDesc[iSubImage].width, pDesc[iSubImage].height, pDesc[iSubImage].format, 0, pDesc[iSubImage].pImageData);
        if (pImage->pSubImages[pImage->subimageCount].pGUIImage == NULL) {
            continue;   // Skip this sub-image if it was not successfully created.
        }

        pImage->subimageCount += 1;
    }

    if (pImage->subimageCount == 0) {
        free(pImage->pSubImages);
        free(pImage);
        return NULL;    // Failed to create any sub-images.
    }

    return pImage;
}

void dred_image_delete(dred_image* pImage)
{
    if (pImage == NULL) {
        return;
    }

    for (size_t i = 0; i < pImage->subimageCount; ++i) {
        drgui_delete_image(pImage->pSubImages[i].pGUIImage);
    }

    free(pImage->pSubImages);
    free(pImage);
}


drgui_image* dred_image_acquire_subimage(dred_image* pImage, float scale)
{
    if (pImage == NULL) {
        return NULL;
    }

    // We just pick the sub-image with the closest scale.
    dred_subimage* pClosestImage = NULL;
    for (size_t i = 0; i < pImage->subimageCount; ++i) {
        if (pImage->pSubImages[i].scale == scale) {
            pClosestImage = &pImage->pSubImages[i];
            break;
        }

        if (pClosestImage == NULL || fabsf(pClosestImage->scale - scale) > fabsf(pImage->pSubImages[i].scale - scale)) {
            pClosestImage = &pImage->pSubImages[i];
        }
    }

    pClosestImage->referenceCount += 1;
    return pClosestImage->pGUIImage;
}

void dred_image_release_subimage(dred_image* pImage, drgui_image* pSubImage)
{
    if (pImage == NULL || pSubImage == NULL) {
        return;
    }

    for (size_t i = 0; i < pImage->subimageCount; ++i) {
        if (pImage->pSubImages[i].pGUIImage == pSubImage) {
            assert(pImage->pSubImages[i].referenceCount > 0);
            pImage->pSubImages[i].referenceCount -= 1;
            if (pImage->pSubImages[i].referenceCount == 0) {
                dred_image__delete_subimage_by_index(pImage, i);
            }

            break;
        }
    }
}
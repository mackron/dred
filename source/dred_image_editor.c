
typedef struct
{
    drgui_image* pImage;
    float imageScale;
    float offsetX;
    float offsetY;
    int mouseDownPosX;
    int mouseDownPosY;
    bool isMouseDown;
    bool isTransparent;
} dred_image_editor_data;

drgui_image* dred_image_editor__load_image_from_file(dred_context* pDred, const char* filePathAbsolute, int* pOriginalComponents)
{
    size_t fileSize;
    void* pFileData = dr_open_and_read_file(filePathAbsolute, &fileSize);
    if (pFileData == NULL) {
        return NULL;
    }

    // We use trial and error to load the image.
    int sizeX = 0;
    int sizeY = 0;
    int components = 0;
    void* pImageData = NULL;

    // stb_image.
    pImageData = stbi_load_from_memory(pFileData, (int)fileSize, &sizeX, &sizeY, &components, 4);
    if (pImageData != NULL) {
        goto create_image;
    }

    // TODO: dr_dds

    // PCX.
    pImageData = drpcx_load_memory(pFileData, fileSize, false, &sizeX, &sizeY, &components);
    if (pImageData != NULL) {
        // dr_gui requires the image to have 4 components.
        if (components != 4) {
            const uint8_t* pImageDataSrc = pImageData;
            uint8_t* pImageDataDst = (uint8_t*)malloc(sizeX*sizeY*4);
            if (pImageDataDst == NULL) {
                goto on_error;
            }

            void* pNewImageData = pImageDataDst;

            for (int y = 0; y < sizeY; ++y) {
                for (int x = 0; x < sizeX; ++x) {
                    pImageDataDst[0] = 0;
                    pImageDataDst[1] = 0;
                    pImageDataDst[2] = 0;
                    pImageDataDst[3] = 255;
                    for (int c = 0; c < components; ++c) {
                        pImageDataDst[c] = pImageDataSrc[c];
                    }

                    pImageDataDst += 4;
                    pImageDataSrc += components;
                }
            }

            free(pImageData);
            pImageData = pNewImageData;
        }

        goto create_image;
    }


on_error:
    // If we get here it means we failed to load the image.
    dr_free_file_data(pFileData);
    return NULL;


create_image:
    dr_free_file_data(pFileData);
    drgui_image* pImage = drgui_create_image(pDred->pGUI, sizeX, sizeY, drgui_image_format_rgba8, sizeX*4, pImageData);
    if (pImage == NULL) {   
        return NULL;
    }

    if (pOriginalComponents) *pOriginalComponents = components;

    return pImage;
}

void dred_image_editor__on_mouse_button_down(dred_image_editor* pImageEditor, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_image_editor_data* data = (dred_image_editor_data*)dred_editor_get_extra_data(pImageEditor);
    assert(data != NULL);

    if (mouseButton == DRGUI_MOUSE_BUTTON_LEFT) {
        if (!data->isMouseDown) {
            drgui_capture_mouse(pImageEditor);
            data->isMouseDown = true;
            data->mouseDownPosX = mousePosX;
            data->mouseDownPosY = mousePosY;
        }
    }
}

void dred_image_editor__on_mouse_button_up(dred_image_editor* pImageEditor, int mouseButton, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_image_editor_data* data = (dred_image_editor_data*)dred_editor_get_extra_data(pImageEditor);
    assert(data != NULL);

    if (mouseButton == DRGUI_MOUSE_BUTTON_LEFT) {
        if (data->isMouseDown) {
            drgui_release_mouse(pImageEditor->pContext);
            data->isMouseDown = false;
        }
    }
}

void dred_image_editor__on_mouse_move(dred_image_editor* pImageEditor, int mousePosX, int mousePosY, int stateFlags)
{
    (void)stateFlags;

    dred_image_editor_data* data = (dred_image_editor_data*)dred_editor_get_extra_data(pImageEditor);
    assert(data != NULL);

    if (data->isMouseDown) {
        int deltaX = mousePosX - data->mouseDownPosX;
        int deltaY = mousePosY - data->mouseDownPosY;
        data->offsetX += deltaX;
        data->offsetY += deltaY;

        data->mouseDownPosX = mousePosX;
        data->mouseDownPosY = mousePosY;

        // Redraw.
        drgui_dirty(pImageEditor, drgui_get_local_rect(pImageEditor));
    }
}

void dred_image_editor__on_mouse_wheel(dred_image_editor* pImageEditor, int delta, int mousePosX, int mousePosY, int stateFlags)
{
    (void)mousePosX;
    (void)mousePosY;
    (void)stateFlags;

    dred_image_editor_data* data = (dred_image_editor_data*)dred_editor_get_extra_data(pImageEditor);
    assert(data != NULL);

    float oldImageScale = data->imageScale;
    float newImageScale = oldImageScale;
    if (delta > 0) {
        newImageScale = oldImageScale * (1.0f + ( delta * 0.1f));
    } else {
        newImageScale = oldImageScale / (1.0f + (-delta * 0.1f));
    }

    // Always make sure the 100% scale is selectable.
    if ((newImageScale < 1 && oldImageScale > 1) || (newImageScale > 1 && oldImageScale < 1)) {
        newImageScale = 1;
    }

    dred_image_editor_set_image_scale(pImageEditor, newImageScale);
}

void dred_image_editor__on_paint(dred_image_editor* pImageEditor, drgui_rect rect, void* pPaintData)
{
    (void)rect;

    dred_image_editor_data* data = (dred_image_editor_data*)dred_editor_get_extra_data(pImageEditor);
    assert(data != NULL);

    if (data->pImage != NULL){
        unsigned int imageWidth;
        unsigned int imageHeight;
        drgui_get_image_size(data->pImage, &imageWidth, &imageHeight);

        drgui_draw_image_args args;
        args.srcX            = 0;
        args.srcY            = 0;
        args.srcWidth        = (float)imageWidth;
        args.srcHeight       = (float)imageHeight;
        args.dstBoundsX      = 0;
        args.dstBoundsY      = 0;
        args.dstBoundsWidth  = drgui_get_width(pImageEditor);
        args.dstBoundsHeight = drgui_get_height(pImageEditor);
        args.dstWidth        = (float)imageWidth * data->imageScale;
        args.dstHeight       = (float)imageHeight * data->imageScale;
        args.dstX            = data->offsetX + (args.dstBoundsWidth - args.dstWidth) / 2;
        args.dstY            = data->offsetY + (args.dstBoundsHeight - args.dstHeight) / 2;
        args.foregroundTint  = drgui_rgb(255, 255, 255);
        args.backgroundColor = drgui_rgb(48, 48, 48);
        args.boundsColor     = drgui_rgb(48, 48, 48);
        args.options         = DRGUI_IMAGE_DRAW_BOUNDS;
        if (data->isTransparent) {
            args.options |= DRGUI_IMAGE_DRAW_BACKGROUND;
        } else {
            args.options |= DRGUI_IMAGE_HINT_NO_ALPHA;
        }
        drgui_draw_image(pImageEditor, data->pImage, &args, pPaintData);
    } else {
        drgui_draw_rect(pImageEditor, drgui_get_local_rect(pImageEditor), drgui_rgb(48, 48, 48), pPaintData);
    }
}

dred_image_editor* dred_image_editor_create(dred_context* pDred, dred_control* pParent, const char* filePathAbsolute)
{
    dred_image_editor* pImageEditor = dred_editor_create(pDred, pParent, DRED_CONTROL_TYPE_IMAGE_EDITOR, filePathAbsolute, sizeof(dred_image_editor_data));
    if (pImageEditor == NULL) {
        return NULL;
    }

    dred_image_editor_data* data = (dred_image_editor_data*)dred_editor_get_extra_data(pImageEditor);
    assert(data != NULL);

    int originalComponents;
    data->pImage = dred_image_editor__load_image_from_file(pDred, filePathAbsolute, &originalComponents);
    if (data->pImage == NULL) {
        dred_editor_delete(pImageEditor);
        return NULL;
    }

    data->imageScale = 1;
    data->isMouseDown = false;
    data->isTransparent = originalComponents == 4;

    // Events.
    dred_control_set_on_paint(pImageEditor, dred_image_editor__on_paint);
    dred_control_set_on_mouse_button_down(pImageEditor, dred_image_editor__on_mouse_button_down);
    dred_control_set_on_mouse_button_up(pImageEditor, dred_image_editor__on_mouse_button_up);
    dred_control_set_on_mouse_move(pImageEditor, dred_image_editor__on_mouse_move);
    dred_control_set_on_mouse_wheel(pImageEditor, dred_image_editor__on_mouse_wheel);

    return pImageEditor;
}

void dred_image_editor_delete(dred_image_editor* pImageEditor)
{
    if (pImageEditor == NULL) {
        return;
    }

    dred_image_editor_data* data = (dred_image_editor_data*)dred_editor_get_extra_data(pImageEditor);
    if (data != NULL) {
        drgui_delete_image(data->pImage);
    }

    dred_editor_delete(pImageEditor);
}

void dred_image_editor_set_image_scale(dred_image_editor* pImageEditor, float scale)
{
    dred_image_editor_data* data = (dred_image_editor_data*)dred_editor_get_extra_data(pImageEditor);
    if (data == NULL) {
        return;
    }

    if (data->imageScale == scale) {
        return;
    }

    unsigned int baseImageSizeX;
    drgui_get_image_size(data->pImage, &baseImageSizeX, NULL);

    scale = dr_clamp(scale, 0.1f, 8.0f);
    if (data->imageScale != scale) {
        data->imageScale = scale;
        drgui_dirty(pImageEditor, drgui_get_local_rect(pImageEditor));
    }
}

float dred_image_editor_get_image_scale(dred_image_editor* pImageEditor)
{
    dred_image_editor_data* data = (dred_image_editor_data*)dred_editor_get_extra_data(pImageEditor);
    if (data == NULL) {
        return 0;
    }

    return data->imageScale;
}
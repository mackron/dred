// Copyright (C) 2017 David Reid. See included LICENSE file.

// BEGIN STOCK IMAGE LIST : <source file> <id name>
//
// cross.svg            DRED_STOCK_IMAGE_ID_CROSS
// arrow_right.svg      DRED_STOCK_IMAGE_ID_ARROW_RIGHT
// arrow_right_down.svg DRED_STOCK_IMAGE_ID_ARROW_RIGHT_DOWN
// logo.png             DRED_STOCK_IMAGE_ID_LOGO
//
// END STOCK IMAGE LIST

typedef struct
{
    dtk_uint32 id;
    dtk_image* pImage;
} dred_image_library_item;

struct dred_image_library
{
    dred_context* pDred;
    dtk_image pStockImages[DRED_STOCK_IMAGE_COUNT];

    size_t imageBufferSize;
    size_t imageCount;
    dred_image_library_item* pImages;
};

// Initializes the given image library.
dtk_bool32 dred_image_library_init(dred_image_library* pLibrary, dred_context* pDred);

// Uninitializes the given image library.
void dred_image_library_uninit(dred_image_library* pLibrary);


// Creates an image, or returns a reference to an already-loaded one.
//
// Every call to dred_image_library_create_image() should be matched with a called to dred_image_library_delete_image().
//dred_image* dred_image_library_create_image(dred_image_library* pLibrary, unsigned int id, const dred_image_desc* pDesc, size_t descCount);

// Deletes an image that was created by dred_image_library_create_image()
//void dred_image_library_delete_image(dred_image_library* pLibrary, dred_image* pImage);


// Retrieves an image by it's ID.
dtk_image* dred_image_library_get_image_by_id(dred_image_library* pLibrary, unsigned int id);
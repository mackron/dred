// Copyright (C) 2017 David Reid. See included LICENSE file.

// BEGIN STOCK IMAGE LIST : <source file> <id name> <base width> <base height>
//
// cross.svg            DRED_STOCK_IMAGE_ID_CROSS             16  16
// arrow_right.svg      DRED_STOCK_IMAGE_ID_ARROW_RIGHT       7   16
// arrow_right_down.svg DRED_STOCK_IMAGE_ID_ARROW_RIGHT_DOWN  7   16
//
// END STOCK IMAGE LIST

struct dred_image_library
{
    dred_context* pDred;
    size_t imageBufferSize;
    size_t imageCount;
    dred_image** ppImages;
};

// Initializes the given image library.
dtk_bool32 dred_image_library_init(dred_image_library* pLibrary, dred_context* pDred);

// Uninitializes the given image library.
void dred_image_library_uninit(dred_image_library* pLibrary);


// Creates an image, or returns a reference to an already-loaded one.
//
// Every call to dred_image_library_create_image() should be matched with a called to dred_image_library_delete_image().
dred_image* dred_image_library_create_image(dred_image_library* pLibrary, unsigned int id, const dred_image_desc* pDesc, size_t descCount);

// Deletes an image that was created by dred_image_library_create_image()
void dred_image_library_delete_image(dred_image_library* pLibrary, dred_image* pImage);


// Retrieves an image by it's ID.
dred_image* dred_image_library_get_image_by_id(dred_image_library* pLibrary, unsigned int id);
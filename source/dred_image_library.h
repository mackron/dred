
struct dred_image_library
{
    dred_context* pDred;
    size_t imageBufferSize;
    size_t imageCount;
    dred_image** ppImages;
};

// Initializes the given image library.
bool dred_image_library_init(dred_image_library* pLibrary, dred_context* pDred);

// Uninitializes the given image library.
void dred_image_library_uninit(dred_image_library* pLibrary);


// Creates an image, or returns a reference to an already-loaded one.
//
// Every call to dred_image_library_create_image() should be matched with a called to dred_image_library_delete_image().
dred_image* dred_image_library_create_image(dred_image_library* pLibrary, dred_image_desc* pDesc, size_t descCount);

// Deletes an image that was created by dred_image_library_create_image()
void dred_image_library_delete_image(dred_image_library* pLibrary, dred_image* pImage);
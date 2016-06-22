
#define DRED_VERSION_MAJOR          0
#define DRED_VERSION_MINOR          1
#define DRED_VERSION_REVISION       0
#define DRED_VERSION_STRING         "0.1-preview"
#define DRED_PRODUCT_NAME           "dred"
#define DRED_PRODUCT_DESCRIPTION    "dred"
#define DRED_COMPANY_NAME           "David Reid"

#ifdef _WIN32
#define DRED_WIN32
#define DRED_EXE_NAME               "dred.exe"
#endif

#ifdef __linux__
#define DRED_GTK
#define DRED_EXE_NAME               "dred"
#endif

#define DRED_MAX_PATH               4096

#define DRED_MIN_FONT_SIZE          1
#define DRED_MAX_FONT_SIZE          400

#define DRED_MAX_RECENT_FILES       10
#define DRED_MAX_RECENT_COMMANDS    32


// Define these to exclude certain features from the build.
#define DRED_NO_IMAGE_EDITOR

// KEEP THIS BLANK LINE AT THE END. NEEDED FOR THE .RC FILE.

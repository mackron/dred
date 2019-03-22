// Copyright (C) 2019 David Reid. See included LICENSE file.

#ifndef DRED_VERSION_MAJOR
#define DRED_VERSION_MAJOR          0
#endif
#ifndef DRED_VERSION_MINOR
#define DRED_VERSION_MINOR          4
#endif
#ifndef DRED_VERSION_REVISION
#define DRED_VERSION_REVISION       3
#endif
#ifndef DRED_VERSION_STRING
#define DRED_VERSION_STRING         "0.4.3"
#endif
#ifndef DRED_PRODUCT_NAME
#define DRED_PRODUCT_NAME           "dred"
#endif
#ifndef DRED_PRODUCT_DESCRIPTION
#define DRED_PRODUCT_DESCRIPTION    "dred"
#endif
#ifndef DRED_COMPANY_NAME
#define DRED_COMPANY_NAME           "David Reid"
#endif

#ifdef _WIN32
#define DRED_WIN32
#ifndef DRED_EXE_NAME
#define DRED_EXE_NAME               "dred.exe"
#endif
#else
#define DRED_UNIX
#define DRED_GTK
#ifndef DRED_EXE_NAME
#define DRED_EXE_NAME               "dred"
#endif
#endif


#ifdef _WIN32
#ifdef _WIN64
#define DRED_64BIT
#else
#define DRED_32BIT
#endif
#endif

#ifdef __GNUC__
#ifdef __LP64__
#define DRED_64BIT
#else
#define DRED_32BIT
#endif
#endif

#if !defined(DRED_64BIT) && !defined(DRED_32BIT)
#include <stdint.h>
#if SIZE_MAX == ~0ULL
#define DRED_64BIT
#else
#define DRED_32BIT
#endif
#endif

#ifndef NDEBUG
#define DRED_DEBUG
#else
#define DRED_RELEASE
#endif


#define DRED_MAX_PATH               4096

#define DRED_MIN_FONT_SIZE          1
#define DRED_MAX_FONT_SIZE          400

#define DRED_MAX_RECENT_FILES       10
#define DRED_MAX_RECENT_COMMANDS    32
#define DRED_MAX_FAVOURITE_FILES    1024
#define DRED_MAX_THEMES             1024


// Define these to exclude certain features from the build.


// DLL / Shared Object APIs
#ifdef DRED_PACKAGE
#define DRED_PACKAGE_API __declspec(dllexport) 
#else
#define DRED_PACKAGE_API                        // dred never actually imports any external APIs at link time - it's all done at run-time.
#endif

// KEEP THIS BLANK LINE AT THE END. NEEDED FOR THE .RC FILE.

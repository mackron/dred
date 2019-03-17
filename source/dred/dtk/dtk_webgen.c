// Copyright (C) 2019 David Reid. See included LICENSE file.

#ifndef DTK_WEBGEN_CONTENT_FOLDER
#define DTK_WEBGEN_CONTENT_FOLDER    "_drwebgen"
#endif


#ifndef DTK_WEBGEN_GOOGLE_ANALYTICS_TEMPLATE
#define DTK_WEBGEN_GOOGLE_ANALYTICS_TEMPLATE \
    "<script>\n" \
    "  (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){\n" \
    "  (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),\n" \
    "  m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)\n" \
    "  })(window,document,'script','//www.google-analytics.com/analytics.js','ga');\n" \
    "  ga('create', '{{ google-analytics-tracking-id }}', 'auto');\n" \
    "  ga('send', 'pageview');\n" \
    "</script>\n"
#endif


//// stb_stretchy_buffer ////
#ifndef STB_STRETCHY_BUFFER_H_INCLUDED
#define STB_STRETCHY_BUFFER_H_INCLUDED

#ifndef NO_STRETCHY_BUFFER_SHORT_NAMES
#define sb_free   stb_sb_free
#define sb_push   stb_sb_push
#define sb_count  stb_sb_count
#define sb_add    stb_sb_add
#define sb_last   stb_sb_last
#endif

#define stb_sb_free(a)         ((a) ? free(stb__sbraw(a)),0 : 0)
#define stb_sb_push(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define stb_sb_count(a)        ((a) ? stb__sbn(a) : 0)
#define stb_sb_add(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define stb_sb_last(a)         ((a)[stb__sbn(a)-1])

#define stb__sbraw(a) ((int *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]

#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+(n) >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n) : 0)
#define stb__sbgrow(a,n)      ((a) = stb__sbgrowf((a), (n), sizeof(*(a))))

#include <stdlib.h>

static void * stb__sbgrowf(void *arr, int increment, int itemsize)
{
   int dbl_cur = arr ? 2*stb__sbm(arr) : 0;
   int min_needed = stb_sb_count(arr) + increment;
   int m = dbl_cur > min_needed ? dbl_cur : min_needed;
   int *p = (int *) realloc(arr ? stb__sbraw(arr) : 0, itemsize * m + sizeof(int)*2);
   if (p) {
      if (!arr)
         p[1] = 0;
      p[0] = m;
      return p+2;
   } else {
      #ifdef STRETCHY_BUFFER_OUT_OF_MEMORY
      STRETCHY_BUFFER_OUT_OF_MEMORY ;
      #endif
      return (void *) (2*sizeof(int)); // try to force a NULL pointer exception later
   }
}
#endif // STB_STRETCHY_BUFFER_H_INCLUDED


//// Properties ////

dtk_bool32 dtk_webgen_property_init(dtk_webgen_property* pProperty, const char* name, const char* value)
{
    if (pProperty == NULL || name == NULL) {
        return DTK_FALSE;
    }

    if (value == NULL) {
        value = "";
    }

    pProperty->name = dtk_make_string(name);
    if (pProperty->name == NULL) {
        return DTK_FALSE;
    }

    pProperty->value = dtk_make_string(value);
    if (pProperty->value == NULL) {
        dtk_free_string(pProperty->value);
        return DTK_FALSE;
    }

    return DTK_TRUE;
}

void dtk_webgen_property_uninit(dtk_webgen_property* pProperty)
{
    if (pProperty == NULL) {
        return;
    }

    dtk_free_string(pProperty->name);
    dtk_free_string(pProperty->value);
}

void dtk_webgen_property_set_value(dtk_webgen_property* pProperty, const char* value)
{
    if (pProperty == NULL) {
        return;
    }

    dtk_free_string(pProperty->value);
    pProperty->value = dtk_make_string(value);
}



//// Config ////

dtk_bool32 dtk_webgen_config_init(dtk_webgen_config* pConfig, dtk_webgen_config* pParent)
{
    if (pConfig == NULL) {
        return DTK_FALSE;
    }

    pConfig->pParent = pParent;
    pConfig->pProperties = NULL;
    
    return DTK_TRUE;
}

dtk_bool32 dtk_webgen_config_init_default(dtk_webgen_config* pConfig)
{
    if (!dtk_webgen_config_init(pConfig, NULL)) {
        return DTK_FALSE;
    }

    // Defaults.
    dtk_webgen_config_set(pConfig, "year", "2016");

    return DTK_TRUE;
}


void dtk_webgen_config_init_from_file__on_pair(void* pUserData, const char* key, const char* value)
{
    dtk_webgen_config* pConfig = (dtk_webgen_config*)pUserData;
    assert(pConfig != NULL);

    // Remove double quotes.
    char token[4096];
    if (value[0] == '\"') {
        if (dtk_next_token(value, token, sizeof(token)) == NULL) {
            return;
        }
    } else {
        strcpy_s(token, sizeof(token), value);
    }

    dtk_webgen_config_set(pConfig, key, token);
}

void dtk_webgen_config_init_from_file__on_error(void* pUserData, const char* message, unsigned int line)
{
    dtk_webgen_config* pConfig = (dtk_webgen_config*)pUserData;
    assert(pConfig != NULL);

    if (pConfig->_errorOut) {
        snprintf(pConfig->_errorOut, pConfig->_errorOutSize, "Error parsing config: %d: %s", line, message);
    }
}

dtk_bool32 dtk_webgen_config_init_from_file(dtk_webgen_config* pConfig, const char* pConfigFilePath, char* pErrorOut, size_t errorOutSize)
{
    if (pErrorOut != NULL && errorOutSize > 0) {
        pErrorOut[0] = '\0';
    }

    if (!dtk_webgen_config_init_default(pConfig)) {
        return DTK_FALSE;
    }

    pConfig->_errorOut = pErrorOut;
    pConfig->_errorOutSize = errorOutSize;

    dtk_parse_key_value_pairs_from_file(pConfigFilePath, dtk_webgen_config_init_from_file__on_pair, dtk_webgen_config_init_from_file__on_error, pConfig);
    return DTK_TRUE;
}

void dtk_webgen_config_uninit(dtk_webgen_config* pConfig)
{
    if (pConfig == NULL) {
        return;
    }

    int count = stb_sb_count(pConfig->pProperties);
    for (int i = 0; i < count; ++i) {
        dtk_webgen_property_uninit(&pConfig->pProperties[i]);
    }

    stb_sb_free(pConfig->pProperties);
}


dtk_webgen_property* dtk_webgen_config__find_property_no_recursion(dtk_webgen_config* pConfig, const char* name)
{
    assert(pConfig != NULL);
    assert(name != NULL);

    int count = stb_sb_count(pConfig->pProperties);
    for (int i = 0; i < count; ++i) {
        if (strcmp(pConfig->pProperties[i].name, name) == 0) {
            return pConfig->pProperties + i;
        }
    }

    return NULL;
}

const char* dtk_webgen_config_get_no_recursion(dtk_webgen_config* pConfig, const char* name)
{
    if (pConfig == NULL || name == NULL) {
        return NULL;
    }

    dtk_webgen_property* pProperty = dtk_webgen_config__find_property_no_recursion(pConfig, name);
    if (pProperty == NULL) {
        return NULL;
    }

    return pProperty->value;
}

const char* dtk_webgen_config_get(dtk_webgen_config* pConfig, const char* name)
{
    if (pConfig == NULL || name == NULL) {
        return NULL;
    }

    const char* value = dtk_webgen_config_get_no_recursion(pConfig, name);
    if (value != NULL) {
        return value;
    }

    // If we get here it means we could not find the property. Look in the parent.
    if (pConfig->pParent) {
        return dtk_webgen_config_get(pConfig->pParent, name);
    }

    return NULL;
}

void dtk_webgen_config_set(dtk_webgen_config* pConfig, const char* name, const char* value)
{
    if (pConfig == NULL || name == NULL) {
        return;
    }

    if (value == NULL) {
        value = "";
    }

    // If it already exists, replace.
    dtk_webgen_property* pProperty = dtk_webgen_config__find_property_no_recursion(pConfig, name);
    if (pProperty != NULL) {
        dtk_webgen_property_set_value(pProperty, value);
        return;
    }


    // It does not already exist. Append.
    dtk_webgen_property newProperty;
    if (!dtk_webgen_property_init(&newProperty, name, value)) {
        return; // Error initializing.
    }

    stb_sb_push(pConfig->pProperties, newProperty);
}




//// Context ////

dtk_bool32 dtk_webgen_context_init(dtk_webgen_context* pContext, const char* pRootDir, const char* pOutputDir, dtk_webgen_context_on_error_proc onError, void* pUserData)
{
    if (pContext == NULL || pRootDir == NULL || pOutputDir == NULL) {
        return DTK_FALSE;
    }

    // Make every path absolute to begin with to ensure everything is consistent.
    char* pCurrentDir = dtk_get_current_directory();
    if (pCurrentDir == NULL) {
        return DTK_FALSE;   // Probably out of memory.
    }
    
    char pRootDirAbsolute[4096];
    dtk_path_to_absolute(pRootDirAbsolute, sizeof(pRootDirAbsolute), pRootDir, pCurrentDir);

    char pOutputDirAbsolute[4096];
    dtk_path_to_absolute(pOutputDirAbsolute, sizeof(pOutputDirAbsolute), pOutputDir, pCurrentDir);


    memset(pContext, 0, sizeof(*pContext));
    pContext->pRootDir   = dtk_make_string(pRootDirAbsolute);
    pContext->pOutputDir = dtk_make_string(pOutputDirAbsolute);
    pContext->onError    = onError;
    pContext->pUserData  = pUserData;

    char pConfigError[4096];

    char pConfigPath[4096];
    dtk_path_append(pConfigPath, sizeof(pConfigPath), pContext->pRootDir, "_drwebgen/config.drwebgen");
    if (!dtk_webgen_config_init_from_file(&pContext->config, pConfigPath, pConfigError, sizeof(pConfigError))) {
        if (onError) {
            onError(pContext, pConfigError, pUserData);
        }
    
        dtk_free(pCurrentDir);
        return DTK_FALSE;
    }

    dtk_free(pCurrentDir);
    return DTK_TRUE;
}

void dtk_webgen_context_uninit(dtk_webgen_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    dtk_free_string(pContext->pRootDir);
    dtk_free_string(pContext->pOutputDir);
    dtk_webgen_config_uninit(&pContext->config);
}


dtk_bool32 dtk_webgen_context_generate__on_file_iteration(const char* filePath, void* pUserData)
{
    dtk_webgen_context* pContext = (dtk_webgen_context*)pUserData;
    assert(pContext != NULL);

    char pOutputFilePathRelative[4096];
    dtk_path_to_relative(pOutputFilePathRelative, sizeof(pOutputFilePathRelative), filePath, pContext->pRootDir);


    // Ignore the "_drwebgen" folder.
    if (strcmp(pOutputFilePathRelative, "_drwebgen") == 0 || strstr(pOutputFilePathRelative, "_drwebgen/") == pOutputFilePathRelative) {
        return DTK_TRUE;    // <-- Don't return DTK_FALSE, otherwise the iteration will be terminated prematurely.
    }

    
    char pOutputFilePathAbsolute[4096];
    dtk_path_to_absolute(pOutputFilePathAbsolute, sizeof(pOutputFilePathAbsolute), pOutputFilePathRelative, pContext->pOutputDir);


    // If the input file is a directory make sure it's created.
    if (dtk_is_directory(filePath)) {
        dtk_mkdir_recursive(pOutputFilePathAbsolute);
        return DTK_TRUE;
    }


    // We'll get here if it's a normal file (not a directory).

    // For now, only support processing of .html and .htm files.
    if (dtk_path_extension_equal(pOutputFilePathRelative, "html") || dtk_path_extension_equal(pOutputFilePathRelative, "htm")) {
        dtk_string pResolvedStr = dtk_webgen_context_process_file(pContext, filePath);
        if (pResolvedStr == NULL) {
            if (pContext->onError) {
                char error[4096];
                snprintf(error, sizeof(error), "Error processing file: %s. Skipping.", filePath);
                pContext->onError(pContext, error, pContext->pUserData);
            }

            return DTK_TRUE;
        }

        if (!dtk_open_and_write_text_file(pOutputFilePathAbsolute, pResolvedStr)) {
            if (pContext->onError) {
                char error[4096];
                snprintf(error, sizeof(error), "Error creating output file for %s. Skipping.", filePath);
                pContext->onError(pContext, error, pContext->pUserData);
            }
        }

        dtk_free_string(pResolvedStr);
    } else {
        if (!dtk_copy_file(filePath, pOutputFilePathAbsolute, DTK_FALSE)) {
            if (pContext->onError) {
                char error[4096];
                snprintf(error, sizeof(error), "Failed to copy file: %s", filePath);
                pContext->onError(pContext, error, pContext->pUserData);
            }
        }    
    }

    return DTK_TRUE;
}

typedef struct
{
    dtk_webgen_context* pContext;
    dtk_uint32 count;
    dtk_uint32 capacity;
    dtk_string* ppFilePaths;
} dtk_webgen__file_list;

dtk_bool32 dtk_webgen__gather_files_iteration_callback(const char* filePath, void* pUserData)
{
    dtk_webgen__file_list* pFileList = (dtk_webgen__file_list*)pUserData;

    if (pFileList->count == pFileList->capacity) {
        // Not enough room.
        dtk_uint32 newCapacity = (pFileList->capacity == 0) ? 16 : pFileList->capacity*2;
        dtk_string* ppNewFilePaths = (dtk_string*)realloc(pFileList->ppFilePaths, sizeof(*pFileList->ppFilePaths) * newCapacity);
        if (ppNewFilePaths == NULL) {
            return DTK_FALSE;    // Out of memory.
        }

        pFileList->capacity = newCapacity;
        pFileList->ppFilePaths = ppNewFilePaths;
    }

    dtk_string pFilePathCopy = dtk_make_string(filePath);
    if (pFilePathCopy == NULL) {
        return DTK_FALSE;
    }

    pFileList->ppFilePaths[pFileList->count] = pFilePathCopy;
    pFileList->count += 1;
    
    return DTK_TRUE;
}

dtk_bool32 dtk_webgen__gather_files(dtk_webgen_context* pContext, dtk_webgen__file_list* pFileList)
{
    if (pContext == NULL || pFileList == NULL) {
        return DTK_FALSE;
    }

    pFileList->pContext = pContext;
    pFileList->count = 0;
    pFileList->capacity = 0;
    pFileList->ppFilePaths = NULL;
    return dtk_iterate_files(pContext->pRootDir, DTK_TRUE, dtk_webgen__gather_files_iteration_callback, pFileList);
}

void dtk_webgen__uninit_file_list(dtk_webgen__file_list* pFileList)
{
    for (dtk_uint32 iFile = 0; iFile < pFileList->count; ++iFile) {
        dtk_free_string(pFileList->ppFilePaths[iFile]);
    }

    free(pFileList->ppFilePaths);
}

dtk_bool32 dtk_webgen_context_generate(dtk_webgen_context* pContext)
{
    if (pContext == NULL) {
        return DTK_FALSE;
    }

#if 1
    dtk_webgen__file_list fileList;
    if (!dtk_webgen__gather_files(pContext, &fileList)) {
        return DTK_FALSE;
    }

    for (dtk_uint32 iFile = 0; iFile < fileList.count; ++iFile) {
        dtk_webgen_context_generate__on_file_iteration(fileList.ppFilePaths[iFile], pContext);
    }

    dtk_webgen__uninit_file_list(&fileList);
    return DTK_TRUE;
#else
    return dtk_iterate_files(pContext->pRootDir, DTK_TRUE, dtk_webgen_context_generate__on_file_iteration, pContext);
#endif
}

dtk_string dtk_webgen_context_process_file_with_config(dtk_webgen_context* pContext, dtk_webgen_config* pConfig, const char* filePath)
{
    if (pContext == NULL || pConfig == NULL || filePath == NULL) {
        return NULL;
    }

    char filePathAbsolute[4096];
    if (dtk_path_is_absolute(filePath)) {
        strcpy_s(filePathAbsolute, sizeof(filePathAbsolute), filePath);
    } else {
        dtk_path_to_absolute(filePathAbsolute, sizeof(filePathAbsolute), filePath, pContext->pRootDir);
    }


    size_t originalFileSize;
    char* pOriginalStr;
    dtk_result result = dtk_open_and_read_text_file(filePathAbsolute, &originalFileSize, &pOriginalStr);
    if (result != DTK_SUCCESS) {
        return NULL;
    }

    pContext->processingRecursionDepth += 1;

    // We do not resolve relative paths at this point. This is done below in a separate pass.
    dtk_string pProcessedStr = dtk_webgen_context_process_string(pContext, pConfig, pOriginalStr, NULL);
    
    pContext->processingRecursionDepth -= 1;

    // If we are on the base file we want to resolve relative paths.
    if (pContext->processingRecursionDepth == 0) {
        char folderPath[4096];
        dtk_path_remove_file_name(folderPath, sizeof(folderPath), filePathAbsolute);

        char relativePathBase[4096];
        dtk_path_to_relative(relativePathBase, sizeof(relativePathBase), pContext->pRootDir, folderPath);

        dtk_string pProcessedStr2 = dtk_webgen_context_process_string(pContext, pConfig, pProcessedStr, relativePathBase);
        dtk_free_string(pProcessedStr);

        pProcessedStr = pProcessedStr2;
    }

    dtk_free(pOriginalStr);
    return pProcessedStr;
}

dtk_string dtk_webgen_context_process_file(dtk_webgen_context* pContext, const char* filePath)
{
    if (pContext == NULL) {
        return NULL;
    }

    return dtk_webgen_context_process_file_with_config(pContext, &pContext->config, filePath);
}

dtk_string dtk_webgen_context_process_string(dtk_webgen_context* pContext, dtk_webgen_config* pConfig, const char* str, const char* relativePathBase)
{
    if (pContext == NULL || str == NULL) {
        return NULL;
    }

    dtk_string pProcessedStr = NULL;

    // All we do here is find each "{{ }}" pair and replace it with the resolved value within those brackets.
    const char* pRunningStr = str;
    for (;;) {
        const char* pSectionBegStr = pRunningStr;

        char* pOpening = strstr(pRunningStr, "{{");
        if (pOpening == NULL) {
            break;
        }

        pRunningStr = pOpening + 2;
        char* pClosing = strstr(pRunningStr, "}}");
        if (pClosing == NULL) {
            break;
        }

        pRunningStr = pClosing + 2;


        // Append the section before the opening "{{"
        if (pSectionBegStr < pOpening) {
            pProcessedStr = dtk_append_substring(pProcessedStr, pSectionBegStr, pOpening - pSectionBegStr);
            if (pProcessedStr == NULL) {
                return NULL;
            }
        }


        // The content within the brackets needs to be parsed and translated.
        dtk_string pValueInsideBrackets = dtk_append_substring(NULL, pOpening + 2, pClosing - (pOpening + 2));
        if (pValueInsideBrackets != NULL) {
            dtk_trim(pValueInsideBrackets);

            char token[4096];
            const char* pNextToken = dtk_next_token(pValueInsideBrackets, token, sizeof(token));
            if (pNextToken != NULL) {
                const char* args = pNextToken;

                if (strcmp(token, "include") == 0) {
                    // Syntax: include "path relative to _drwebgen"
                    char path[4096];
                    strcpy_s(path, sizeof(path), pContext->pRootDir);
                    strcat_s(path, sizeof(path), "/");
                    strcat_s(path, sizeof(path), DTK_WEBGEN_CONTENT_FOLDER);
                    strcat_s(path, sizeof(path), "/");

                    if (dtk_next_token(args, path + strlen(path), sizeof(path) - strlen(path)) != NULL) {
                        dtk_string pResolvedValue = dtk_webgen_context_process_file_with_config(pContext, pConfig, path);
                        if (pResolvedValue != NULL) {
                            pProcessedStr = dtk_append_string(pProcessedStr, pResolvedValue);
                            dtk_free_string(pResolvedValue);

                            if (pProcessedStr == NULL) {
                                return NULL;
                            }
                        }
                    }
                } else if (strcmp(token, "relative-path") == 0) {
                    // Syntax: relative-path "relative path to the root directory"

                    // For relative paths, we only want to resolve them if our recursion depth is 0.
                    if (pContext->processingRecursionDepth == 0) {
                        char path[4096];
                        strcpy_s(path, sizeof(path), relativePathBase);
                        if (relativePathBase[0] != '\0') {
                            strcat_s(path, sizeof(path), "/");
                        }

                        char relativePath[4096];
                        if (dtk_next_token(args, relativePath, sizeof(relativePath)) != NULL) {
                            if (relativePath[0] == '/') {
                                strcat_s(path, sizeof(path), relativePath + 1);
                            } else {
                                strcat_s(path, sizeof(path), relativePath);
                            }

                            pProcessedStr = dtk_append_string(pProcessedStr, path);
                            if (pProcessedStr == NULL) {
                                return NULL;
                            }
                        }
                    } else {
                        pProcessedStr = dtk_append_substring(pProcessedStr, pOpening, pClosing - pOpening + 2);
                        if (pProcessedStr == NULL) {
                            return NULL;
                        }
                    }
                } else if (strcmp(token, "google-analytics") == 0) {
                    dtk_string pResolvedValue = dtk_webgen_context_process_string(pContext, pConfig, DTK_WEBGEN_GOOGLE_ANALYTICS_TEMPLATE, relativePathBase);
                    if (pResolvedValue != NULL) {
                        pProcessedStr = dtk_append_string(pProcessedStr, pResolvedValue);
                        dtk_free_string(pResolvedValue);

                        if (pProcessedStr == NULL) {
                            return NULL;
                        }
                    }
                } else {
                    // It's not a built-in function. Check if it's a config property.
                    const char* configValue = dtk_webgen_config_get(pConfig, token);
                    if (configValue != NULL) {
                        pProcessedStr = dtk_append_string(pProcessedStr, configValue);
                        if (pProcessedStr == NULL) {
                            return NULL;
                        }
                    } else {
                        // We still don't know what the variable is, so try routing it to a custom handler.
                        dtk_string pResolvedValue = NULL;
                        if (pContext->onResolveValue) {
                            pResolvedValue = pContext->onResolveValue(pContext, token, args, pContext->pUserData);
                            if (pResolvedValue != NULL) {
                                pProcessedStr = dtk_append_string(pProcessedStr, pResolvedValue);
                                if (pProcessedStr == NULL) {
                                    return NULL;
                                }
                            }
                        }
                    }
                }
            }

            dtk_free_string(pValueInsideBrackets);
        }
    }

    // Append the last part that was not processed in the loop above.
    pProcessedStr = dtk_append_string(pProcessedStr, pRunningStr);

    return pProcessedStr;
}

void dtk_webgen_context_set(dtk_webgen_context* pContext, const char* name, const char* value)
{
    if (pContext == NULL) {
        return;
    }

    dtk_webgen_config_set(&pContext->config, name, value);
}

const char* dtk_webgen_context_get(dtk_webgen_context* pContext, const char* name)
{
    if (pContext == NULL) {
        return NULL;
    }

    return dtk_webgen_config_get(&pContext->config, name);
}

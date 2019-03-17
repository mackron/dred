// Copyright (C) 2019 David Reid. See included LICENSE file.

// dtk_webgen is a simple library for doing static website generation. It differs from other solutions like Jekyll in that
// it's implemented as a library rather than a command-line interface and favours simplicity and ease of use over raw
// features and complexity. Indeed, this is not a replacement for Jekyll, Hugo, etc. but is more for people who have very
// simple requirements and would like to have tighter integration with their existing build tools without needing to
// install and depend on yet another software package.
//
//
//
// QUICK NOTES
//
// - dtk_webgen is designed for simple websites. It is not a replacement for more complete solutions such as Jekyll and Hugo.
// - dtk_webgen is not currently optimized, and it's not a high priority. It should still run fairly quickly, though.
// - File paths are limited to about 4000 characters.

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dtk_webgen_config dtk_webgen_config;
typedef struct dtk_webgen_context dtk_webgen_context;

typedef struct
{
    dtk_string name;
    dtk_string value;
} dtk_webgen_property;

struct dtk_webgen_config
{
    // Configs can be chained together to form a very simply hierarchy. Child configs are given priority over parents.
    dtk_webgen_config* pParent;

    // The list of properties within the config. This is manipulated with stb_stretchy_buffer.
    dtk_webgen_property* pProperties;
    size_t propertyCount;

    // Internal use only.
    char* _errorOut;
    size_t _errorOutSize;
};

// Initializes an empty config, tied to a parent.
//
// pConfig [in] A pointer to the config object to initialize.
// pParent [in] The parent config. Can be null.
//
// Returns DTK_TRUE if the config was initialized successfully; DTK_FALSE otherwise.
//
// The config will be tied to the parent, but will not have any local variables set.
dtk_bool32 dtk_webgen_config_init(dtk_webgen_config* pConfig, dtk_webgen_config* pParent);

// Initializes a config with default properties.
//
// pConfig [in] A pointer to the config object to initialize.
//
// Returns DTK_TRUE if the config was initialized successfully; DTK_FALSE otherwise.
//
// The config will not be tied to a parent.
dtk_bool32 dtk_webgen_config_init_default(dtk_webgen_config* pConfig);

// Initializes a config object from a config file.
//
// pConfig         [in] A pointer to the config object to initialize.
// pConfigFilePath [in] The path of the config file to load.
// pErrorOut       [in] A pointer to a buffer that will receive any parsing errors.
// errorOutSize    [in] The size of the buffer pointed to by pErrorOut.
//
// Returns DTK_TRUE if the config was initialized successfully; DTK_FALSE otherwise.
//
// The config will not be tied to a parent and will also be initialized with default properties.
dtk_bool32 dtk_webgen_config_init_from_file(dtk_webgen_config* pConfig, const char* pConfigFilePath, char* pErrorOut, size_t errorOutSize);

// Uninitializes the given config object.
void dtk_webgen_config_uninit(dtk_webgen_config* pConfig);

// Sets a local variable in the given config.
//
// pConfig [in] A pointer to the config object whose property is being set.
// name    [in] The name of the property to set.
// value   [in] The value of the property.
//
// If a property of the same name already exists it will be overwritten.
void dtk_webgen_config_set(dtk_webgen_config* pConfig, const char* name, const char* value);

// Retrieves the value of a config property.
//
// pConfig [in] A pointer to the config object whose property is being retrieved.
// name    [in] The name of the property whose value is being retrieved.
//
// Returns a pointer to the string containing the value of the property, or null if it doesn't exist.
//
// This will first try to find a property to the config object, but if it's not found it will continously search up
// the hierarchy.
const char* dtk_webgen_config_get(dtk_webgen_config* pConfig, const char* name);


typedef char* (* dtk_webgen_context_on_resolve_value_proc)(dtk_webgen_context* pContext, const char* name, const char* args, void* pUserData);
typedef void (* dtk_webgen_context_on_free_value_proc)(dtk_webgen_context* pContext, char* valueReturnedByOnResolve, void* pUserData);
typedef void (* dtk_webgen_context_on_error_proc)(dtk_webgen_context* pContext, const char* message, void* pUserData);

struct dtk_webgen_context
{
    // The path of the root directory of the website.
    dtk_string pRootDir;

    // The path of the output directory.
    dtk_string pOutputDir;

    // The configuration.
    dtk_webgen_config config;

    // The function to call when dtk_webgen wants to give the application a change to resolve a variable. The return
    // value can be freed with onFreeValue().
    dtk_webgen_context_on_resolve_value_proc onResolveValue;

    // The function to call when dtk_webgen needs to free a value returned by onResolveValue.
    dtk_webgen_context_on_free_value_proc onFreeValue;

    // The function to call when an error occurs.
    dtk_webgen_context_on_error_proc onError;

    // The user data passed to onResolveValue and onFreeValue.
    void* pUserData;

    // The recursion depth for file processing. As files are processed they can recursively pull in templates. When
    // processing each of these templates, there's a few things that we only want to handle at the base level. This
    // variable allows us to do this.
    int processingRecursionDepth;
};

// Initializes a dtk_webgen context.
//
// pContext     [in]           A pointer to the context object to initialize.
// pRootDir     [in]           The path of the root directory.
// pOutputDir   [in]           The path of the output directory.
// onError      [in, optional] The function to call when an error occurs.
// pUserData    [in]           A pointer to application-defined data to pass to the callbacks.
//
// Returns DTK_TRUE if the context was initialized successfully; DTK_FALSE otherwise.
//
// The root directory is the root directory of the source files of the website. It will look something like this:
//   <root>
//     _drwebgen/
//     css/
//     img/
//     js/
//     index.html
//
// The "_drwebgen" folder is where _all_ of the dtk_webgen-specific project files like configs and templates are stored.
//
// The output folder is where compiled files are placed. This will include _everything_ in the root directory, except for
// the "_drwebgen" folder.
dtk_bool32 dtk_webgen_context_init(dtk_webgen_context* pContext, const char* pRootDir, const char* pOutputDir, dtk_webgen_context_on_error_proc onError, void* pUserData);

// Uninitializes a dtk_webgen context.
void dtk_webgen_context_uninit(dtk_webgen_context* pContext);

// Performs the actual website generation for the given context.
dtk_bool32 dtk_webgen_context_generate(dtk_webgen_context* pContext);

// Processes the file at the given path using the given config.
//
// This does not output the file, but instead returns a string containing the processed text. Free the returned string with dtk_free_string().
char* dtk_webgen_context_process_file_with_config(dtk_webgen_context* pContext, dtk_webgen_config* pConfig, const char* filePath);

// Processes the file at the given path.
//
// This does not output the file, but instead returns a string containing the processed text. Free the returned string with dtk_free_string().
char* dtk_webgen_context_process_file(dtk_webgen_context* pContext, const char* filePath);

// Processes the given string.
char* dtk_webgen_context_process_string(dtk_webgen_context* pContext, dtk_webgen_config* pConfig, const char* str, const char* relativePathBase);

// Sets the value of a property on the main config.
void dtk_webgen_context_set(dtk_webgen_context* pContext, const char* name, const char* value);

// Retrieves the value of a property on the main config.
const char* dtk_webgen_context_get(dtk_webgen_context* pContext, const char* name);


#ifdef __cplusplus
}
#endif
// Copyright (C) 2019 David Reid. See included LICENSE file.

typedef enum
{
    dtk_binding_var_type_int,
    dtk_binding_var_type_uint,
    dtk_binding_var_type_double,
    dtk_binding_var_type_bool,
    dtk_binding_var_type_color,
    dtk_binding_var_type_string,
    dtk_binding_var_type_data
} dtk_binding_var_type;

typedef struct
{
    size_t name;    /* Offset to the global string pool. */
    dtk_binding_var_type type;
    union
    {
        dtk_int64 i;
        dtk_uint64 u;
        double d;
        dtk_bool32 b;
        dtk_color c;
        const char* str;
        struct
        {
            const void* ptr;
            size_t sizeInBytes;
        } data;
    } value;
} dtk_binding_var;

typedef struct
{
    size_t bindingTarget;  /* Referenced as an offset into the DTK context's global string pool. */
    size_t bindingVar;     /* Referenced as an offset into the DTK context's global string pool. */
} dtk_binding_engine_binding;

typedef struct
{
    dtk_control* pControl;
    dtk_binding_engine_binding* pBindings;  /* Flat array. For now. May turn this into a bit more optimal later on. */
    dtk_uint32 bindingCap;
    dtk_uint32 bindingCount;
    
} dtk_binding_engine_control_bindings;

typedef struct
{
    dtk_context* pTK;
    dtk_binding_engine_control_bindings* pControlBindings;  /* Flat array. */
    dtk_uint32 controlBindingsCap;
    dtk_uint32 controlBindingsCount;
} dtk_binding_engine;

dtk_result dtk_binding_engine_init(dtk_context* pTK, dtk_binding_engine* pBindingEngine);
void dtk_binding_engine_uninit(dtk_binding_engine* pBindingEngine);
dtk_result dtk_binding_engine_bind(dtk_binding_engine* pBindingEngine, dtk_control* pControl, const char* bindingTarget, const char* bindingVar);
dtk_result dtk_binding_engine_unbind(dtk_binding_engine* pBindingEngine, dtk_control* pControl, const char* bindingTarget);
dtk_result dtk_binding_engine_unbind_all(dtk_binding_engine* pBindingEngine, dtk_control* pControl);
const char* dtk_binding_engine_get_binding_var(dtk_binding_engine* pBindingEngine, dtk_control* pControl, const char* bindingTarget);

dtk_result dtk_binding_engine_update_int(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, dtk_int64 bindingValue);
dtk_result dtk_binding_engine_update_uint(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, dtk_uint64 bindingValue);
dtk_result dtk_binding_engine_update_double(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, double bindingValue);
dtk_result dtk_binding_engine_update_bool(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, dtk_bool32 bindingValue);
dtk_result dtk_binding_engine_update_color(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, dtk_color bindingValue);
dtk_result dtk_binding_engine_update_string(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, const char* bindingValue);
dtk_result dtk_binding_engine_update_data(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, const void* bindingValue, size_t bindingValueSize);

dtk_result dtk_binding_engine_handle_update(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, dtk_binding_var* var);    /* Should only be called from DTK's global event handler. */
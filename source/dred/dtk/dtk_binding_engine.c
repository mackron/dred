// Copyright (C) 2019 David Reid. See included LICENSE file.

DTK_PRIVATE dtk_bool32 dtk_binding_engine__find_control(dtk_binding_engine* pBindingEngine, dtk_control* pControl, dtk_uint32* pControlIndex)
{
    dtk_assert(pBindingEngine != NULL);
    dtk_assert(pControl != NULL);
    dtk_assert(pControlIndex != NULL);

    for (dtk_uint32 iControl = 0; iControl < pBindingEngine->controlBindingsCount; ++iControl) {
        if (pBindingEngine->pControlBindings[iControl].pControl == pControl) {
            *pControlIndex = iControl;
            return DTK_TRUE;
        }
    }

    return DTK_FALSE;
}

DTK_PRIVATE dtk_result dtk_binding_engine__add_control(dtk_binding_engine* pBindingEngine, dtk_control* pControl, dtk_uint32* pControlIndex)
{
    dtk_assert(pBindingEngine != NULL);
    dtk_assert(pControl != NULL);
    dtk_assert(pControlIndex != NULL);

    if (pBindingEngine->controlBindingsCount == pBindingEngine->controlBindingsCap) {
        dtk_uint32 newCap = pBindingEngine->controlBindingsCap * 2;
        if (newCap == 0) {
            newCap = 1;
        }

        dtk_binding_engine_control_bindings* pNewControlBindings = (dtk_binding_engine_control_bindings*)dtk_realloc(pBindingEngine->pControlBindings, sizeof(*pNewControlBindings) * newCap);
        if (pNewControlBindings == NULL) {
            return DTK_OUT_OF_MEMORY;
        }

        pBindingEngine->pControlBindings   = pNewControlBindings;
        pBindingEngine->controlBindingsCap = newCap;
    }

    *pControlIndex = pBindingEngine->controlBindingsCount;

    pBindingEngine->pControlBindings[pBindingEngine->controlBindingsCount].pControl     = pControl;
    pBindingEngine->pControlBindings[pBindingEngine->controlBindingsCount].pBindings    = NULL;
    pBindingEngine->pControlBindings[pBindingEngine->controlBindingsCount].bindingCap   = 0;
    pBindingEngine->pControlBindings[pBindingEngine->controlBindingsCount].bindingCount = 0;
    pBindingEngine->controlBindingsCount += 1;

    return DTK_SUCCESS;
}

DTK_PRIVATE dtk_result dtk_binding_engine__find_or_add_control(dtk_binding_engine* pBindingEngine, dtk_control* pControl, dtk_uint32* pControlIndex)
{
    dtk_assert(pBindingEngine != NULL);
    dtk_assert(pControl != NULL);
    dtk_assert(pControlIndex != NULL);

    if (!dtk_binding_engine__find_control(pBindingEngine, pControl, pControlIndex)) {
        dtk_result result = dtk_binding_engine__add_control(pBindingEngine, pControl, pControlIndex);
        if (result != DTK_SUCCESS) {
            return result;
        }
    }

    return DTK_SUCCESS;
}

DTK_PRIVATE void dtk_binding_engine__remove_control(dtk_binding_engine* pBindingEngine, dtk_uint32 controlIndex)
{
    dtk_assert(pBindingEngine != NULL);
    dtk_assert(controlIndex < pBindingEngine->controlBindingsCount);

    dtk_free(pBindingEngine->pControlBindings[controlIndex].pBindings);
    pBindingEngine->pControlBindings[controlIndex].pBindings    = NULL;
    pBindingEngine->pControlBindings[controlIndex].bindingCap   = 0;
    pBindingEngine->pControlBindings[controlIndex].bindingCount = 0;
    
    for (dtk_uint32 iControl = controlIndex; iControl+1 < pBindingEngine->controlBindingsCount; ++iControl) {
        pBindingEngine->pControlBindings[iControl] = pBindingEngine->pControlBindings[iControl+1];
    }
    pBindingEngine->controlBindingsCount -= 1;
}


DTK_PRIVATE dtk_bool32 dtk_binding_engine__find_control_binding(dtk_binding_engine_control_bindings* pControlBindings, size_t bindingTarget, dtk_uint32* pBindingIndex)
{
    dtk_assert(pControlBindings != NULL);
    dtk_assert(pBindingIndex != NULL);

    for (dtk_uint32 iBinding = 0; iBinding < pControlBindings->bindingCount; ++iBinding) {
        if (pControlBindings->pBindings[iBinding].bindingTarget == bindingTarget) {
            *pBindingIndex = iBinding;
            return DTK_TRUE;
        }
    }

    return DTK_FALSE;
}

DTK_PRIVATE dtk_result dtk_binding_engine__add_control_binding(dtk_binding_engine_control_bindings* pControlBindings, size_t bindingTarget, size_t bindingVar)
{
    dtk_assert(pControlBindings != NULL);

    if (pControlBindings->bindingCount == pControlBindings->bindingCap) {
        dtk_uint32 newCap = pControlBindings->bindingCap * 2;
        if (newCap == 0) {
            newCap = 1;
        }

        dtk_binding_engine_binding* pNewBindings = (dtk_binding_engine_binding*)dtk_realloc(pControlBindings->pBindings, sizeof(*pNewBindings) * newCap);
        if (pNewBindings == NULL) {
            return DTK_OUT_OF_MEMORY;
        }

        pControlBindings->pBindings  = pNewBindings;
        pControlBindings->bindingCap = newCap;
    }

    pControlBindings->pBindings[pControlBindings->bindingCount].bindingTarget = bindingTarget;
    pControlBindings->pBindings[pControlBindings->bindingCount].bindingVar    = bindingVar;
    pControlBindings->bindingCount += 1;

    return DTK_SUCCESS;
}

DTK_PRIVATE void dtk_binding_engine__remove_control_binding(dtk_binding_engine_control_bindings* pControlBindings, dtk_uint32 bindingIndex)
{
    dtk_assert(pControlBindings != NULL);
    dtk_assert(bindingIndex < pControlBindings->bindingCount);

    for (dtk_uint32 iBinding = bindingIndex; iBinding+1 < pControlBindings->bindingCount; ++iBinding) {
        pControlBindings->pBindings[iBinding] = pControlBindings->pBindings[iBinding+1];
    }
    pControlBindings->bindingCount -= 1;
}

DTK_PRIVATE dtk_result dtk_binding_engine__replace_control_binding(dtk_binding_engine_control_bindings* pControlBindings, dtk_uint32 bindingIndex, size_t bindingVar)
{
    dtk_assert(pControlBindings != NULL);
    dtk_assert(bindingIndex < pControlBindings->bindingCount);

    pControlBindings->pBindings[bindingIndex].bindingVar = bindingVar;
    return DTK_SUCCESS;
}



dtk_result dtk_binding_engine_init(dtk_context* pTK, dtk_binding_engine* pBindingEngine)
{
    if (pBindingEngine != NULL) {
        dtk_zero_object(pBindingEngine);
    }

    if (pTK == NULL || pBindingEngine == NULL) {
        return DTK_INVALID_ARGS;
    }

    pBindingEngine->pTK = pTK;

    return DTK_SUCCESS;
}

void dtk_binding_engine_uninit(dtk_binding_engine* pBindingEngine)
{
    if (pBindingEngine == NULL) {
        return;
    }

    // At uninitialization time, everything should have been cleaned up by now. But for safety I'm going to go ahead and clean everything up anyway.
    for (dtk_uint32 iControlBindings = 0; iControlBindings < pBindingEngine->controlBindingsCount; ++iControlBindings) {
        dtk_free(pBindingEngine->pControlBindings[iControlBindings].pBindings);
    }
    dtk_free(pBindingEngine->pControlBindings);
}

dtk_result dtk_binding_engine_bind(dtk_binding_engine* pBindingEngine, dtk_control* pControl, const char* bindingTarget, const char* bindingVar)
{
    dtk_result result;
    size_t bindingTargetPos;
    size_t bindingVarPos;
    dtk_uint32 controlIndex;
    dtk_uint32 controlBindingIndex;

    if (pBindingEngine == NULL || pControl == NULL || dtk_string_is_null_or_empty(bindingTarget) || dtk_string_is_null_or_empty(bindingVar)) {
        return DTK_INVALID_ARGS;
    }

    bindingTargetPos = dtk_string_pool_find_or_add(&pBindingEngine->pTK->stringPool, bindingTarget);
    if (bindingTargetPos == 0) {
        return DTK_ERROR;
    }

    bindingVarPos = dtk_string_pool_find_or_add(&pBindingEngine->pTK->stringPool, bindingVar);
    if (bindingTargetPos == 0) {
        return DTK_ERROR;
    }

    /* If the control has not had a binding set on it before we need to add the control to the list. */
    result = dtk_binding_engine__find_or_add_control(pBindingEngine, pControl, &controlIndex);
    if (result != DTK_SUCCESS) {
        return result;
    }

    /* If the binding target does not already exist we need to insert it. If it does already exist we need to replace. */
    if (dtk_binding_engine__find_control_binding(&pBindingEngine->pControlBindings[controlIndex], bindingTargetPos, &controlBindingIndex)) {
        /* Already exists. Replace. */
        result = dtk_binding_engine__replace_control_binding(&pBindingEngine->pControlBindings[controlIndex], controlBindingIndex, bindingVarPos);
        if (result != DTK_SUCCESS) {
            return result;
        }
    } else {
        /* Does not already exist. Insert. */
        result = dtk_binding_engine__add_control_binding(&pBindingEngine->pControlBindings[controlIndex], bindingTargetPos, bindingVarPos);
        if (result != DTK_SUCCESS) {
            return result;
        }
    }

    return DTK_SUCCESS;
}

dtk_result dtk_binding_engine_unbind(dtk_binding_engine* pBindingEngine, dtk_control* pControl, const char* bindingTarget)
{
    dtk_uint32 controlIndex;
    dtk_uint32 controlBindingIndex;
    size_t bindingTargetPos;

    if (pBindingEngine == NULL || pControl == NULL) {
        return DTK_INVALID_ARGS;
    }

    if (!dtk_binding_engine__find_control(pBindingEngine, pControl, &controlIndex)) {
        return DTK_ERROR;   /* Nothing is bound to the control. Likely this means the application has a bug in it, so returning an error so it can know about it. */
    }

    if (!dtk_string_pool_find(&pBindingEngine->pTK->stringPool, bindingTarget, &bindingTargetPos)) {
        return DTK_ERROR;   /* The binding target was not found in the string pool which means it was never bound to the control. */
    }

    if (!dtk_binding_engine__find_control_binding(&pBindingEngine->pControlBindings[controlIndex], bindingTargetPos, &controlBindingIndex)) {
        return DTK_ERROR;   /* The binding target is not bound. */
    }

    dtk_binding_engine__remove_control_binding(&pBindingEngine->pControlBindings[controlIndex], controlBindingIndex);

    /* If the control does not have any bindings we need to remove it from the list. */
    if (pBindingEngine->pControlBindings[controlIndex].bindingCount == 0) {
        dtk_binding_engine__remove_control(pBindingEngine, controlIndex);
    }

    return DTK_SUCCESS;
}

dtk_result dtk_binding_engine_unbind_all(dtk_binding_engine* pBindingEngine, dtk_control* pControl)
{
    dtk_uint32 controlIndex;

    if (pBindingEngine == NULL || pControl == NULL) {
        return DTK_INVALID_ARGS;
    }

    if (!dtk_binding_engine__find_control(pBindingEngine, pControl, &controlIndex)) {
        return DTK_SUCCESS; /* Nothing is bound to the control. */
    }

    dtk_binding_engine__remove_control(pBindingEngine, controlIndex);

    return DTK_SUCCESS;
}

const char* dtk_binding_engine_get_binding_var(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingTarget)
{
    dtk_uint32 controlIndex;
    dtk_uint32 controlBindingIndex;
    size_t bindingTargetOffset;

    dtk_assert(pBindingEngine != NULL);
    dtk_assert(pOriginator != NULL);
    dtk_assert(!dtk_string_is_null_or_empty(bindingTarget));

    if (!dtk_binding_engine__find_control(pBindingEngine, pOriginator, &controlIndex)) {
        return NULL;
    }

    if (!dtk_string_pool_find(&pBindingEngine->pTK->stringPool, bindingTarget, &bindingTargetOffset)) {
        return NULL;    /* Has never been bound to anything. */
    }

    if (!dtk_binding_engine__find_control_binding(&pBindingEngine->pControlBindings[controlIndex], bindingTargetOffset, &controlBindingIndex)) {
        return NULL;
    }

    return dtk_string_pool_cstr(&pBindingEngine->pTK->stringPool, pBindingEngine->pControlBindings[controlIndex].pBindings[controlBindingIndex].bindingVar);
}


DTK_PRIVATE dtk_result dtk_binding_engine_update_var(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, dtk_binding_var* var)
{
    dtk_assert(pBindingEngine != NULL);
    dtk_assert(!dtk_string_is_null_or_empty(bindingVar));
    dtk_assert(var != NULL);

    /*
    There may be many controls bound to the given variable. We don't want to post an event for each one because it could become inefficient. Instead
    we post just one event and let the event handler do the event routing logic for us.
    */
    dtk_event e = dtk_event_init(pBindingEngine->pTK, DTK_EVENT_BINDING, pOriginator);
    e.binding.var = *var;
    
    if (!dtk_string_pool_find(&pBindingEngine->pTK->stringPool, bindingVar, &e.binding.var.name)) {
        return DTK_SUCCESS;   /* The binding variable is not bound to anything. */
    }

    /* We need to make a copy of variable length data. This will be freed in the event handler. */
    if (e.binding.var.type == dtk_binding_var_type_string) {
        e.binding.var.value.str = (const char*)dtk_make_string(e.binding.var.value.str); /* This cast is OK. */
    }
    if (e.binding.var.type == dtk_binding_var_type_data) {
        void* pData = dtk_malloc(e.binding.var.value.data.sizeInBytes);
        if (pData == NULL) {
            return DTK_OUT_OF_MEMORY;
        }
        dtk_copy_memory(pData, e.binding.var.value.data.ptr, e.binding.var.value.data.sizeInBytes);
        e.binding.var.value.data.ptr = pData;
    }

    return dtk_post_global_event(&e);
}

dtk_result dtk_binding_engine_update_int(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, dtk_int64 bindingValue)
{
    if (pBindingEngine == NULL || dtk_string_is_null_or_empty(bindingVar)) {
        return DTK_INVALID_ARGS;
    }

    dtk_binding_var var;
    var.type    = dtk_binding_var_type_int;
    var.value.i = bindingValue;
    return dtk_binding_engine_update_var(pBindingEngine, pOriginator, bindingVar, &var);
}

dtk_result dtk_binding_engine_update_uint(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, dtk_uint64 bindingValue)
{
    if (pBindingEngine == NULL || dtk_string_is_null_or_empty(bindingVar)) {
        return DTK_INVALID_ARGS;
    }

    dtk_binding_var var;
    var.type    = dtk_binding_var_type_uint;
    var.value.u = bindingValue;
    return dtk_binding_engine_update_var(pBindingEngine, pOriginator, bindingVar, &var);
}

dtk_result dtk_binding_engine_update_double(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, double bindingValue)
{
    if (pBindingEngine == NULL || dtk_string_is_null_or_empty(bindingVar)) {
        return DTK_INVALID_ARGS;
    }

    dtk_binding_var var;
    var.type    = dtk_binding_var_type_double;
    var.value.d = bindingValue;
    return dtk_binding_engine_update_var(pBindingEngine, pOriginator, bindingVar, &var);
}

dtk_result dtk_binding_engine_update_bool(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, dtk_bool32 bindingValue)
{
    if (pBindingEngine == NULL || dtk_string_is_null_or_empty(bindingVar)) {
        return DTK_INVALID_ARGS;
    }

    dtk_binding_var var;
    var.type    = dtk_binding_var_type_bool;
    var.value.b = bindingValue;
    return dtk_binding_engine_update_var(pBindingEngine, pOriginator, bindingVar, &var);
}

dtk_result dtk_binding_engine_update_color(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, dtk_color bindingValue)
{
    if (pBindingEngine == NULL || dtk_string_is_null_or_empty(bindingVar)) {
        return DTK_INVALID_ARGS;
    }

    dtk_binding_var var;
    var.type    = dtk_binding_var_type_color;
    var.value.c = bindingValue;
    return dtk_binding_engine_update_var(pBindingEngine, pOriginator, bindingVar, &var);
}

dtk_result dtk_binding_engine_update_string(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, const char* bindingValue)
{
    if (pBindingEngine == NULL || dtk_string_is_null_or_empty(bindingVar)) {
        return DTK_INVALID_ARGS;
    }

    dtk_binding_var var;
    var.type      = dtk_binding_var_type_string;
    var.value.str = bindingValue;
    return dtk_binding_engine_update_var(pBindingEngine, pOriginator, bindingVar, &var);
}

dtk_result dtk_binding_engine_update_data(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, const char* bindingVar, const void* bindingValue, size_t bindingValueSize)
{
    if (pBindingEngine == NULL || dtk_string_is_null_or_empty(bindingVar)) {
        return DTK_INVALID_ARGS;
    }

    dtk_binding_var var;
    var.type                   = dtk_binding_var_type_data;
    var.value.data.ptr         = bindingValue;
    var.value.data.sizeInBytes = bindingValueSize;
    return dtk_binding_engine_update_var(pBindingEngine, pOriginator, bindingVar, &var);
}

dtk_result dtk_binding_engine_handle_update(dtk_binding_engine* pBindingEngine, dtk_control* pOriginator, dtk_binding_var* var)
{
    if (pBindingEngine == NULL || var == NULL) {
        return DTK_INVALID_ARGS;
    }

    /* Any controls with this binding needs to have an event posted to it. */
    dtk_event e = dtk_event_init(pBindingEngine->pTK, DTK_EVENT_BINDING, NULL);
    e.binding.var = *var;

    for (dtk_uint32 iControl = 0; iControl < pBindingEngine->controlBindingsCount; ++iControl) {
        if (pBindingEngine->pControlBindings[iControl].pControl != pOriginator) {
            for (dtk_uint32 iControlBinding = 0; iControlBinding < pBindingEngine->pControlBindings[iControl].bindingCount; ++iControlBinding) {
                if (pBindingEngine->pControlBindings[iControl].pBindings[iControlBinding].bindingVar == var->name) {
                    e.pControl       = pBindingEngine->pControlBindings[iControl].pControl;
                    e.binding.target = dtk_string_pool_cstr(&pBindingEngine->pTK->stringPool, pBindingEngine->pControlBindings[iControl].pBindings[iControlBinding].bindingTarget);
                    dtk_handle_local_event(&e);
                }
            }
        }
    }

    return DTK_SUCCESS;
}

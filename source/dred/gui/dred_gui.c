// Copyright (C) 2019 David Reid. See included LICENSE file.

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <float.h>
#include <math.h>

/////////////////////////////////////////////////////////////////
// Controls

dtk_bool32 dred_control_init(dred_control* pControl, dred_context* pDred, dtk_control* pParent, const char* type, dtk_event_proc onEvent)
{
    if (pControl == NULL || pDred == NULL) return DTK_FALSE;
    memset(pControl, 0, sizeof(*pControl));


    if (dtk_control_init(&pDred->tk, DTK_CONTROL_TYPE_DRED, onEvent, pParent, &pControl->baseControl) != DTK_SUCCESS) {
        return DTK_FALSE;
    }

    (void)type;
    //dred_control_set_type(pControl, type);
    return DTK_TRUE;
}

void dred_control_uninit(dred_control* pControl)
{
    if (pControl == NULL) {
        return;
    }

    dtk_control_uninit(&pControl->baseControl);
}


//dtk_bool32 dred_control_set_type(dred_control* pControl, const char* type)
//{
//    if (pControl == NULL) {
//        return DTK_FALSE;
//    }
//
//    return strcpy_s(pControl->type, sizeof(pControl->type), (type == NULL) ? "" : type) == 0;
//}
//
//const char* dred_control_get_type(dred_control* pControl)
//{
//    if (pControl == NULL) {
//        return NULL;
//    }
//
//    return pControl->type;
//}
//
//dtk_bool32 dred_control_is_of_type(dred_control* pControl, const char* type)
//{
//    if (pControl == NULL || type == NULL) {
//        return DTK_FALSE;
//    }
//
//    return dred_is_control_type_of_type(pControl->type, type);
//}
//
//dtk_bool32 dred_is_control_type_of_type(const char* type, const char* base)
//{
//    if (type == NULL || base == NULL) {
//        return DTK_FALSE;
//    }
//
//    return strncmp(type, base, strlen(base)) == 0;
//}

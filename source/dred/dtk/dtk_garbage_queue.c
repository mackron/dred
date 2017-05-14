// Copyright (C) 2017 David Reid. See included LICENSE file.

dtk_result dtk_garbage_queue_init(dtk_garbage_queue* pQueue)
{
    if (pQueue == NULL) return DTK_INVALID_ARGS;
    
    dtk_zero_object(pQueue);
    dtk_mutex_init(&pQueue->lock);

    return DTK_SUCCESS;
}

dtk_result dtk_garbage_queue_uninit(dtk_garbage_queue* pQueue)
{
    if (pQueue == NULL) return DTK_INVALID_ARGS;

    dtk_mutex_uninit(&pQueue->lock);
    dtk_free(pQueue->pItems);

    return DTK_SUCCESS;
}

dtk_result dtk_garbage_queue_enqueue(dtk_garbage_queue* pQueue, dtk_control* pControl)
{
    if (pQueue == NULL || pControl == NULL) return DTK_INVALID_ARGS;

    dtk_mutex_lock(&pQueue->lock);
    {
        // Couldn't merge. Going to need to enqueue a new item.
        if (pQueue->count == pQueue->capacity) {
            // Not enough memory. Reallocate.
            dtk_uint32 newCapacity = (pQueue->capacity == 0) ? 1 : pQueue->capacity*2;
            dtk_ptr* pNewItems = (dtk_ptr*)dtk_malloc(sizeof(*pNewItems) * newCapacity);
            if (pNewItems == NULL) {
                dtk_mutex_unlock(&pQueue->lock);
                return DTK_OUT_OF_MEMORY;   // Ran out of memory :(
            }

            for (dtk_uint32 i = 0; i < pQueue->count; ++i) {
                pNewItems[i] = pQueue->pItems[(pQueue->iFirstItem + i) % pQueue->capacity];
            }

            dtk_free(pQueue->pItems);
            pQueue->pItems = pNewItems;
            pQueue->capacity = newCapacity;
            pQueue->iFirstItem = 0;
        }

        dtk_assert(pQueue->count < pQueue->capacity);

        dtk_uint32 iNewItem = (pQueue->iFirstItem + pQueue->count) % pQueue->capacity;
        pQueue->pItems[iNewItem] = pControl;
        pQueue->count += 1;

        dtk_post_garbage_dequeue_notification_event(pControl->pTK, pControl);
    }
    dtk_mutex_unlock(&pQueue->lock);

    return DTK_SUCCESS;
}

dtk_result dtk_garbage_queue_dequeue(dtk_garbage_queue* pQueue, dtk_control** ppDequeuedControl)
{
    if (pQueue == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_ERROR;
    dtk_mutex_lock(&pQueue->lock);
    {
        if (pQueue->count > 0) {
            *ppDequeuedControl = (dtk_control*)pQueue->pItems[pQueue->iFirstItem];

            pQueue->iFirstItem = (pQueue->iFirstItem + 1) % pQueue->capacity;
            pQueue->count -= 1;
            result = DTK_SUCCESS;
        } else {
            result = DTK_NO_EVENT;  // There's no items.
        }
    }
    dtk_mutex_unlock(&pQueue->lock);

    return result;
}

dtk_uint32 dtk_garbage_queue_get_count(dtk_garbage_queue* pQueue)
{
    if (pQueue == NULL) return 0;

    dtk_uint32 count;
    dtk_atomic_exchange_32(&count, pQueue->count);

    return count;
}

dtk_control* dtk_garbage_queue_get_next_garbage_control(dtk_garbage_queue* pQueue)
{
    if (pQueue == NULL) return 0;

    dtk_ptr pGarbageControl;
    dtk_atomic_exchange_ptr(&pGarbageControl, pQueue->pItems[pQueue->iFirstItem]);

    return (dtk_control*)pGarbageControl;
}
// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_result dtk_paint_queue_init(dtk_paint_queue* pQueue)
{
    if (pQueue == NULL) return DTK_INVALID_ARGS;
    
    dtk_zero_object(pQueue);
    dtk_mutex_init(&pQueue->lock);

    return DTK_SUCCESS;
}

dtk_result dtk_paint_queue_uninit(dtk_paint_queue* pQueue)
{
    if (pQueue == NULL) return DTK_INVALID_ARGS;

    dtk_mutex_uninit(&pQueue->lock);
    dtk_free(pQueue->pItems);

    return DTK_SUCCESS;
}


dtk_bool32 dtk_paint_queue__try_merge(dtk_paint_queue* pQueue, dtk_window* pWindow, dtk_rect rect)
{
    dtk_assert(pQueue != NULL);
    dtk_assert(pWindow != NULL);

    // We can merge if the previous item uses the same window.
    if (pQueue->count == 0) {
        return DTK_FALSE;
    }

    dtk_uint32 iPrevItem = (pQueue->iFirstItem + pQueue->count - 1) % pQueue->capacity;
    if (pQueue->pItems[iPrevItem].pWindow == pWindow) {
        pQueue->pItems[iPrevItem].rect = dtk_rect_union(pQueue->pItems[iPrevItem].rect, rect);
        return DTK_TRUE;
    }

    // The previous item is not the same window and thus cannot be merged :(
    return DTK_FALSE;
}

dtk_result dtk_paint_queue_enqueue(dtk_paint_queue* pQueue, dtk_window* pWindow, dtk_rect rect)
{
    if (pQueue == NULL || pWindow == NULL) return DTK_INVALID_ARGS;

    dtk_mutex_lock(&pQueue->lock);
    {
        if (!dtk_paint_queue__try_merge(pQueue, pWindow, rect)) {   // <-- This will try merging the new paint request with a previous one if possible. If it fails, we need to enqueue a new one.
            // Couldn't merge. Going to need to enqueue a new item.
            if (pQueue->count == pQueue->capacity) {
                // Not enough memory. Reallocate.
                dtk_uint32 newCapacity = (pQueue->capacity == 0) ? 1 : pQueue->capacity*2;
                dtk_paint_queue_item* pNewItems = (dtk_paint_queue_item*)dtk_malloc(sizeof(*pNewItems) * newCapacity);
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
            pQueue->pItems[iNewItem].pWindow = pWindow;
            pQueue->pItems[iNewItem].rect = rect;
            pQueue->count += 1;

            dtk_post_paint_notification_event(DTK_CONTROL(pWindow)->pTK, pWindow);
        }
    }
    dtk_mutex_unlock(&pQueue->lock);

    return DTK_SUCCESS;
}

dtk_result dtk_paint_queue_dequeue(dtk_paint_queue* pQueue, dtk_paint_queue_item* pItem)
{
    if (pQueue == NULL) return DTK_INVALID_ARGS;

    dtk_result result = DTK_ERROR;
    dtk_mutex_lock(&pQueue->lock);
    {
        if (pQueue->count > 0) {
            *pItem = pQueue->pItems[pQueue->iFirstItem];

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
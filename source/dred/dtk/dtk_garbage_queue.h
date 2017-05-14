// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    dtk_mutex lock;
    dtk_ptr* pItems;
    dtk_uint32 count;
    dtk_uint32 capacity;
    dtk_uint32 iFirstItem;
} dtk_garbage_queue;

dtk_result dtk_garbage_queue_init(dtk_garbage_queue* pQueue);
dtk_result dtk_garbage_queue_uninit(dtk_garbage_queue* pQueue);
dtk_result dtk_garbage_queue_enqueue(dtk_garbage_queue* pQueue, dtk_control* pControl);
dtk_result dtk_garbage_queue_dequeue(dtk_garbage_queue* pQueue, dtk_control** ppDequeuedControl);
dtk_uint32 dtk_garbage_queue_get_count(dtk_garbage_queue* pQueue);
dtk_control* dtk_garbage_queue_get_next_garbage_control(dtk_garbage_queue* pQueue);
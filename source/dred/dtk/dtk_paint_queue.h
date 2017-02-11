// Copyright (C) 2016 David Reid. See included LICENSE file.

typedef struct
{
    dtk_window* pWindow;
    dtk_rect rect;
} dtk_paint_queue_item;

typedef struct
{
    dtk_mutex lock;
    dtk_paint_queue_item* pItems;
    dtk_uint32 count;
    dtk_uint32 capacity;
    dtk_uint32 iFirstItem;
} dtk_paint_queue;

dtk_result dtk_paint_queue_init(dtk_paint_queue* pQueue);
dtk_result dtk_paint_queue_uninit(dtk_paint_queue* pQueue);
dtk_result dtk_paint_queue_enqueue(dtk_paint_queue* pQueue, dtk_window* pWindow, dtk_rect rect);
dtk_result dtk_paint_queue_dequeue(dtk_paint_queue* pQueue, dtk_paint_queue_item* pItem);
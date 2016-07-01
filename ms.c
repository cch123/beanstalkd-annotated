#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "dat.h"

void
ms_init(ms a, ms_event_fn oninsert, ms_event_fn onremove)
{
    a->used = a->cap = a->last = 0;
    a->items = NULL;
    a->oninsert = oninsert;
    a->onremove = onremove;
}

static void
grow(ms a)
{
    void **nitems;
    //看起来全局的tube list的容量是两倍两倍扩容的
    //和全局的hashtable不一样，全局hashtable是按照素数容量来扩容的
    size_t ncap = (a->cap << 1) ? : 1;
    //new items
    nitems = malloc(ncap * sizeof(void *));
    if (!nitems) return;

    //扩容后拷贝所有的items
    memcpy(nitems, a->items, a->used * sizeof(void *));
    //释放原来的items
    free(a->items);
    //修改指针和容量
    a->items = nitems;
    a->cap = ncap;
}

int
ms_append(ms a, void *item)
{
    if (a->used >= a->cap) grow(a);
    if (a->used >= a->cap) return 0;

    a->items[a->used++] = item;
    //如果注册了insert tube的回调，那么调用这个函数
    if (a->oninsert) a->oninsert(a, item, a->used - 1);
    return 1;
}

static int
ms_delete(ms a, size_t i)
{
    void *item;

    if (i >= a->used) return 0;
    item = a->items[i];
    a->items[i] = a->items[--a->used];

    /* it has already been removed now */
    if (a->onremove) a->onremove(a, item, i);
    return 1;
}

void
ms_clear(ms a)
{
    while (ms_delete(a, 0));
    free(a->items);
    ms_init(a, a->oninsert, a->onremove);
}

int
ms_remove(ms a, void *item)
{
    size_t i;

    for (i = 0; i < a->used; i++) {
        if (a->items[i] == item) return ms_delete(a, i);
    }
    return 0;
}

int
ms_contains(ms a, void *item)
{
    size_t i;

    for (i = 0; i < a->used; i++) {
        if (a->items[i] == item) return 1;
    }
    return 0;
}

void *
ms_take(ms a)
{
    void *item;

    if (!a->used) return NULL;

    a->last = a->last % a->used;
    item = a->items[a->last];
    ms_delete(a, a->last);
    ++a->last;
    return item;
}

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "dat.h"

struct ms tubes;

tube
make_tube(const char *name)
{
    tube t;

    t = new(struct tube);
    if (!t) return NULL;

    t->name[MAX_TUBE_NAME_LEN - 1] = '\0';
    strncpy(t->name, name, MAX_TUBE_NAME_LEN - 1);
    if (t->name[MAX_TUBE_NAME_LEN - 1] != '\0') twarnx("truncating tube name");

    //定义ready堆和delay堆的less函数，作为调整堆时的less函数依据
    //ready堆用优先级定义less
    t->ready.less = job_pri_less;
    //delay堆用delay定义less
    t->delay.less = job_delay_less;

    t->ready.rec = job_setheappos;
    t->delay.rec = job_setheappos;
    t->buried = (struct job) { };
    t->buried.prev = t->buried.next = &t->buried;
    ms_init(&t->waiting, NULL, NULL);

    return t;
}

static void
tube_free(tube t)
{
    prot_remove_tube(t);
    free(t->ready.data);
    free(t->delay.data);
    ms_clear(&t->waiting);
    free(t);
}

void
tube_dref(tube t)
{
    if (!t) return;
    if (t->refs < 1) return twarnx("refs is zero for tube: %s", t->name);

    --t->refs;
    if (t->refs < 1) tube_free(t);
}

void
tube_iref(tube t)
{
    if (!t) return;
    ++t->refs;
}

static tube
make_and_insert_tube(const char *name)
{
    int r;
    tube t = NULL;

    t = make_tube(name);
    if (!t) return NULL;

    /* We want this global tube list to behave like "weak" refs, so don't
     * increment the ref count. */
    //像上面注释写的，全局的tube list只是个弱引用，不去修改tube的那个refs变量
    r = ms_append(&tubes, t);
    if (!r) return tube_dref(t), (tube) 0;

    return t;
}

tube
tube_find(const char *name)
{
    tube t;
    size_t i;

    //这里不知道是否可以用二分来做一些优化
    //不过tube总数一般也不多
    for (i = 0; i < tubes.used; i++) {
        t = tubes.items[i];
        if (strncmp(t->name, name, MAX_TUBE_NAME_LEN) == 0) return t;
    }
    return NULL;
}

tube
tube_find_or_make(const char *name)
{
	//在全局的tubes中查找name
	//如果查到了返回tube，没查到创建一个返回
    return tube_find(name) ? : make_and_insert_tube(name);
}


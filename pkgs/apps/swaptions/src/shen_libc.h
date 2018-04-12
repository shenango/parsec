#pragma once

#include <stdlib.h>

#ifdef SHENANGO

extern "C" {
#include <runtime/preempt.h>
#undef min
#undef max
}

static inline void *malloc_np(size_t len)
{
    void *ret;
    preempt_disable();
    ret = malloc(len);
    preempt_enable();
    return ret;
}

static inline void free_np(void *ptr)
{
    preempt_disable();
    free(ptr);
    preempt_enable();
}


#else

#define malloc_np malloc
#define free_np free

#endif

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <base/lock.h>
#include <runtime/sync.h>
#include <runtime/thread.h>

struct barrier {
	spinlock_t lock;
	uint64_t running;
	uint64_t count;
	thread_t **waiters;
};

typedef struct barrier barrier_t;

int barrier_init(barrier_t *barrier, uint64_t count);
int barrier_wait(barrier_t *barrier);
void barrier_destroy(barrier_t *barrier);

#ifdef __cplusplus
}
#endif
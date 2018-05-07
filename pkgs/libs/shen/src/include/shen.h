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

struct join_handle;
int thread_spawn_joinable(struct join_handle **handle, void *(*fn) (void *), void *arg);
void thread_join(struct join_handle *handle, void **retval);

#ifdef __cplusplus
}
#endif
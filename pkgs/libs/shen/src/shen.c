
#include <stdio.h>

#include <dlfcn.h>

#include <runtime/preempt.h>

#include "include/shen.h"

static void *(*real_malloc)(size_t);
static void (*real_free)(void *);
static void *(*real_realloc)(void *, size_t);
static void *(*real_aligned_alloc)(size_t, size_t);

void *malloc(size_t size)
{
	if (unlikely(!real_malloc)) {
		fprintf(stderr, "Hooked malloc\n");
		real_malloc = dlsym(RTLD_NEXT, "malloc");
	}

	preempt_disable();
	void *ret = real_malloc(size);
	preempt_enable();
	return ret;
}

void free(void *p)
{
	if (unlikely(!real_free)) {
		real_free = dlsym(RTLD_NEXT, "free");
	}

	preempt_disable();
	real_free(p);
	preempt_enable();
}

void *realloc(void *p, size_t size)
{
	if (unlikely(!real_realloc)) {
		real_realloc = dlsym(RTLD_NEXT, "realloc");
	}

	preempt_disable();
	void *ret = real_realloc(p, size);
	preempt_enable();
	return ret;
}

void *aligned_alloc(size_t alignment, size_t size)
{
	if (unlikely(!real_aligned_alloc))
		real_aligned_alloc = dlsym(RTLD_NEXT, "aligned_alloc");

	preempt_disable();
	void *ret = real_aligned_alloc(alignment, size);
	preempt_enable();
	return ret;
}

int barrier_init(barrier_t *barrier, uint64_t count)
{
	BUG_ON(count == 0);
	spin_lock_init(&barrier->lock);
	barrier->running = 0;
	barrier->count = count;
	barrier->waiters = malloc(sizeof(thread_t*) * (count - 1));
	if (barrier->waiters == NULL)
		return -ENOMEM;

	return 0;
}

int barrier_wait(barrier_t *barrier)
{
	thread_t *myth;
	uint64_t mypos;

	spin_lock_np(&barrier->lock);
	myth = thread_self();
	mypos = ++barrier->running % barrier->count;
	if (mypos == 0) {
		for (uint64_t i = 0; i < barrier->count - 1; i++)
			thread_ready(barrier->waiters[i]);
		spin_unlock_np(&barrier->lock);
		return 0;
	}
	barrier->waiters[mypos-1] = myth;
	thread_park_and_unlock_np(&barrier->lock);
	return 0;
}

void barrier_destroy(barrier_t *barrier)
{
	free(barrier->waiters);
}


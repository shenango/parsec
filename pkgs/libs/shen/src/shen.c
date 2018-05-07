
#include <stdio.h>

#include <dlfcn.h>

#include <runtime/preempt.h>

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

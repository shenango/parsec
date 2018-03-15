
#include "osdep.h"

#ifdef SHENANGO

struct wgargs {
    thread_fn_t fn;
    void *args;
    void *retval;
    spinlock_t    lock;
    bool        done;
    thread_t    *self;
    thread_t    *waiter;
};

void run_and_wgdone(void *arg) {
  struct wgargs *wgargs = arg;
  void *(*start_routine) (void *) = wgargs->fn;
  wgargs->retval = start_routine(wgargs->args);
  spin_lock_np(&wgargs->lock);
  wgargs->done = true;
  wgargs->self = thread_self();
  if (wgargs->waiter) {
    thread_ready(wgargs->waiter);
  }
  thread_park_and_unlock_np(&wgargs->lock);
};


int shenango_thread_create(struct wgargs **w, void *attr, thread_fn_t fn,
                                  void *arg) {
  BUG_ON(attr != NULL);


  struct wgargs *wgargs;

  thread_t *t = thread_create_with_buf(run_and_wgdone, (void **)&wgargs,
                                       sizeof(struct wgargs));

  if (t == NULL)
    return -EAGAIN;

  wgargs->fn = fn;
  wgargs->args = arg;
  spin_lock_init(&wgargs->lock);
  wgargs->done = false;
  wgargs->waiter = NULL;
  wgargs->self = NULL;
  

  if (w)
    *w = wgargs;

  thread_ready(t);
  return 0;
}

int shenango_thread_join(struct wgargs *wgargs, void **retval) {

  spin_lock_np(&wgargs->lock);
  if (!wgargs->done) {
    wgargs->waiter = thread_self();
    thread_park_and_unlock_np(&wgargs->lock);
    spin_lock_np(&wgargs->lock);
  }
    
  if (retval)
      *retval = wgargs->retval;
  spin_unlock_np(&wgargs->lock);
  thread_ready(wgargs->self);
  return;
}
#endif

#pragma once

extern "C" {
#undef assert
#include <base/list.h>
#include <base/lock.h>
#include <runtime/thread.h>
#include <runtime/sync.h>
#include <runtime/preempt.h>
#undef min
#undef max
}

#include <thread.h>
#include <sync.h>

#include <vector>

namespace rt {

class Barrier {
public:
	Barrier(int count) : count_(count), running_(0) {
		spin_lock_init(&lock_);
		waiters_.reserve(count-1);
	}

	void Wait() {
		thread_t *myth;
		uint64_t mypos;

		spin_lock_np(&lock_);
		myth = thread_self();

		mypos = ++running_ % count_;
		if (mypos == 0) {// last waiter
			for (int i = 0; i < count_ - 1; i++) {
				thread_ready(waiters_[i]);
			}
			spin_unlock_np(&lock_); 
			return;
		}
		waiters_[mypos-1] = myth;
		thread_park_and_unlock_np(&lock_);
	}

	~Barrier() {}

private:
	spinlock_t lock_;
	uint64_t running_;
	uint64_t count_;
	std::vector<thread_t*> waiters_;
};
}

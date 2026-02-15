#ifndef RWLOCK_H
#define RWLOCK_H

#include "queue.h"
#include "spinlock.h"
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

struct entry {
	bool            is_writer;
	pthread_mutex_t  mutex;
	pthread_cond_t   cond;
};

struct rwlock {
	struct spinlock  guard;
	struct queue     wait_q;
	int 			 active_readers;
	bool             writer_active;
};

void rwlock_init(struct rwlock *rw);
void rwlock_acquire_read(struct rwlock *rw);
void rwlock_release_read(struct rwlock *rw);
void rwlock_acquire_write(struct rwlock *rw);
void rwlock_release_write(struct rwlock *rw);

#endif 

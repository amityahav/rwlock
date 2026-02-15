#include "rwlock.h"
#include "queue.h"
#include "spinlock.h"
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

static struct entry *entry_create(bool is_writer);
static void entry_destroy(struct entry *e);
static void entry_wait(struct entry *e);
static void entry_wake(struct entry *e);

void rwlock_init(struct rwlock *rw)
{
	spin_init(&rw->guard);
	q_init(&rw->wait_q);
	rw->active_readers = 0;
	rw->writer_active = 0;
}

void rwlock_acquire_read(struct rwlock *rw)
{
	spin_lock(&rw->guard);
	if(!rw->writer_active && q_is_empty(&rw->wait_q)) {
		rw->active_readers++;
		spin_unlock(&rw->guard);
		return;
	}

	struct entry *e = entry_create(false);
	q_append(&rw->wait_q, e);
	pthread_mutex_lock(&e->mutex);  // hold before releasing guard so waker blocks until we're in cond_wait
	spin_unlock(&rw->guard);
	entry_wait(e);

	// waking up here means we've acquired the read lock
	entry_destroy(e);
}

void rwlock_release_read(struct rwlock *rw)
{
	spin_lock(&rw->guard);
	rw->active_readers--;
	if(rw->active_readers > 0) {
		spin_unlock(&rw->guard);
		return;
	}

	if (q_is_empty(&rw->wait_q)) {
		spin_unlock(&rw->guard);
		return;
	}

	// if q is not empty, then its head must point to a writer waiting
	// in this case, grant him rw.
	struct entry *e = q_pop(&rw->wait_q);

	rw->writer_active = true;
	entry_wake(e);

	spin_unlock(&rw->guard);
	
	return;
}

void rwlock_acquire_write(struct rwlock *rw)
{
	spin_lock(&rw->guard);
	if (!rw->writer_active && rw->active_readers == 0) {
		rw->writer_active = true;
		spin_unlock(&rw->guard);
		return;
	}

	struct entry *e = entry_create(true);
	q_append(&rw->wait_q, e);
	pthread_mutex_lock(&e->mutex);  // hold before releasing guard so waker blocks until we're in cond_wait 
	spin_unlock(&rw->guard);
	entry_wait(e);

	// waking up here means we've acquired the write lock
	entry_destroy(e);
}

void rwlock_release_write(struct rwlock *rw)
{
	spin_lock(&rw->guard);
	rw->writer_active = false;

	// we handle to following scenarios:
	// 1) q: [W, R, R] -> grant lock for write, q: [R, R]
	// 2) q: [R, R] -> grant lock for all readers, q: []
	// 3) q: [R, R, W, R] -> grant lock for the first batch of readers, q: [W, R]
	// 4) q: [] -> lock is kept released
	bool is_batch = false;
	while(!q_is_empty(&rw->wait_q)) {
		struct entry* e = q_peek(&rw->wait_q);
		if (e->is_writer) {
			if (is_batch) {
				// we've already popped a batch of readers, stop here
				spin_unlock(&rw->guard);
				return;
			}

			// grant the lock for the first writer in queue
			e = q_pop(&rw->wait_q);
			rw->writer_active = true;

			entry_wake(e);

			spin_unlock(&rw->guard);
			return;
		}


		is_batch = true;

		e = q_pop(&rw->wait_q);
		rw->active_readers++;
		entry_wake(e);
	}

	spin_unlock(&rw->guard);
}

static struct entry *entry_create(bool is_writer)
{
	struct entry *e = malloc(sizeof(*e));
	if (!e)
		return NULL;
	e->is_writer = is_writer;
	pthread_mutex_init(&e->mutex, NULL);
	pthread_cond_init(&e->cond, NULL);
	return e;
}

static void entry_destroy(struct entry *e)
{
	pthread_cond_destroy(&e->cond);
	pthread_mutex_destroy(&e->mutex);
	free(e);
}

static void entry_wait(struct entry *e)
{
	// must hold e->mutex before releasing rw->guard, so the waker
	// blocks on e->mutex until we're in cond_wait.
	pthread_cond_wait(&e->cond, &e->mutex);
	pthread_mutex_unlock(&e->mutex);
}

static void entry_wake(struct entry *e)
{
	pthread_mutex_lock(&e->mutex);
	pthread_cond_signal(&e->cond);
	pthread_mutex_unlock(&e->mutex);
}

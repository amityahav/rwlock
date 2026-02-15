#include "spinlock.h"

void spin_init(struct spinlock *s)
{
	atomic_init(&s->locked, 0);
}

void spin_lock(struct spinlock *s)
{
	while (atomic_exchange_explicit(&s->locked, 1, memory_order_acquire) != 0) {}
}

void spin_unlock(struct spinlock *s)
{
	atomic_store_explicit(&s->locked, 0, memory_order_release);
}
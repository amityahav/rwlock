#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdatomic.h>

struct spinlock {
	atomic_int locked;
};

void spin_init(struct spinlock *s);
void spin_lock(struct spinlock *s);
void spin_unlock(struct spinlock *s);

#endif

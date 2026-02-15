#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stddef.h>

struct q_node {
	struct q_node *next;
	void *payload;
};

struct queue {
	struct q_node *head;
	struct q_node *tail;
	size_t len;
};

void  q_init(struct queue *q);
void  q_append(struct queue *q, void *payload);
void *q_pop(struct queue *q);
void *q_peek(const struct queue *q);
bool  q_is_empty(const struct queue *q);
void  q_clear(struct queue *q);

#endif 

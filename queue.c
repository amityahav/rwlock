#include "queue.h"
#include <stdlib.h>

void q_init(struct queue *q)
{
	q->head = q->tail = NULL;
	q->len = 0;
}

void q_append(struct queue *q, void *payload)
{
	struct q_node *n = malloc(sizeof(*n));
	if (!n)
		return;
	n->next = NULL;
	n->payload = payload;

	if (q->tail) {
		q->tail->next = n;
		q->tail = n;
	} else {
		q->head = q->tail = n;
	}
	q->len++;
}

void *q_pop(struct queue *q)
{
	struct q_node *n = q->head;
	if (!n)
		return NULL;

	void *payload = n->payload;
	q->head = n->next;
	if (!q->head)
		q->tail = NULL;
	q->len--;
	free(n);
	return payload;
}

void *q_peek(const struct queue *q)
{
	if (!q->head)
		return NULL;
	return q->head->payload;
}

bool q_is_empty(const struct queue *q)
{
	return q->head == NULL;
}

void q_clear(struct queue *q)
{
	while (q_pop(q) != NULL)
		;
}

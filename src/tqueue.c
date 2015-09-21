/*
 * tqueue.c
 *
 *  Created on: Sep 22, 2015
 *      Author: avalluri
 */

#include "tqueue.h"

#include <stdio.h>
#include <stdlib.h>



TAILQ_HEAD(tqueue_head, tqueue_element);

struct tqueue
{
	tqueue_compare_func compare;
	tqueue_free_func free;
	struct tqueue_head head;
};

struct tqueue* tqueue_new(tqueue_compare_func compare_func,
		tqueue_free_func free_func)
{
	struct tqueue* q = calloc(1, sizeof(*q));
	if (!q) return NULL;

	q->compare = compare_func;
	q->free = free_func;
	TAILQ_INIT(&q->head);

	return q;
}

struct tqueue_element*
tqueue_insert(struct tqueue* q, void* data)
{
	struct tqueue_element* elm = calloc(1, sizeof(*elm));
	if (elm) {
		TAILQ_INSERT_TAIL(&(q->head), elm, entry);
		elm->data = data;
	}
	return elm;
}

void tqueue_remove(struct tqueue* q, struct tqueue_element* elm)
{
	if (q && elm) {
		TAILQ_REMOVE(&q->head, elm, entry);
		if (q->free) q->free(elm->data);
		free(elm);
	}
}

struct tqueue_element* tqueue_find(struct tqueue* q, void* data, void* userdata)
{
	struct tqueue_element* elm = NULL;
	if (!q || !q->compare) return NULL;

	TAILQ_FOREACH(elm, &q->head, entry) {
		if (q->compare(elm->data, data, userdata) == 0) return elm;
	}

	return elm;
}

struct tqueue_element* tqueue_nth_element(struct tqueue* q, unsigned index)
{
	struct tqueue_element* elm = NULL;
	unsigned i = 0;
	if (!q) return NULL;

	TAILQ_FOREACH(elm, &q->head, entry) {
		if (i++ == index) return elm;
	}

	return NULL;
}

int tqueue_foreach(struct tqueue* q, tqueue_foreach_func foreach,
		void* userdata)
{
	struct tqueue_element* elm = NULL;
	int rc = 0;

	TAILQ_FOREACH(elm, &(q->head), entry) {
		if ((rc = foreach(elm->data, userdata))) ; return rc;
	}

	return rc;
}

int tqueue_empty(struct tqueue* q)
{
	return q ? TAILQ_EMPTY(&q->head) : 1;
}

struct tqueue_element* tqueue_first(struct tqueue* q)
{
	return tqueue_empty(q) ? NULL : TAILQ_FIRST(&q->head);
}

struct tqueue_element* tqueue_last(struct tqueue* q)
{
	return tqueue_empty(q) ? NULL : TAILQ_LAST(&q->head, tqueue_head);
}

void tqueue_free(struct tqueue* q)
{
	struct tqueue_element* elm = NULL;

	while((elm = TAILQ_FIRST(&q->head))) {
		TAILQ_REMOVE(&q->head, elm, entry);
		if (q->free) q->free(elm->data);
		free(elm);
	}
	free(q);
}

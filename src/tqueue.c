/*
 * tqueue.c
 *
 *  Created on: Sep 22, 2015
 *      Author: avalluri
 */
#include <stdio.h>
#include <stdlib.h>

#include "tqueue.h"

TAILQ_HEAD(tqueue_head, _tqueue_element);

struct _tqueue
{
	tqueue_compare_func_t compare;
	tqueue_free_func_t free;
	struct tqueue_head head;
	size_t size;
};

static
int _tqueu_compare_func_default(void *elm_data, const void* data) {
	return elm_data == data;
}

tqueue* tqueue_new(tqueue_compare_func_t compare_func,
		tqueue_free_func_t free_func)
{
	tqueue* q = calloc(1, sizeof(*q));
	if (!q) return NULL;

	q->compare = compare_func ? compare_func :_tqueu_compare_func_default ;
	q->free = free_func;
	TAILQ_INIT(&q->head);

	return q;
}

tqueue_element*
tqueue_insert(tqueue* q, void* data)
{
	tqueue_element* elm = calloc(1, sizeof(*elm));
	if (elm) {
		TAILQ_INSERT_TAIL(&(q->head), elm, entry);
		elm->data = data;
		q->size++;
	}
	return elm;
}

void tqueue_remove(tqueue* q, tqueue_element* elm)
{
	if (q && elm) {
		TAILQ_REMOVE(&(q->head), elm, entry);
		if (q->free) q->free(elm->data);
		free(elm);
		q->size--;
	}
}

void tqueue_remove_data(tqueue* q, void* data)
{
	tqueue_element* elm = NULL;
	if (q && (elm = tqueue_find(q, data))) {
		TAILQ_REMOVE(&(q->head), elm, entry);
		if (q->free) q->free(elm->data);
		free(elm);
		q->size--;
	}
}

void tqueue_remove_all(tqueue* q) {
	if (q) {
		tqueue_element* elm = NULL;
		while((elm = TAILQ_FIRST(&q->head))) {
			TAILQ_REMOVE(&q->head, elm, entry);
			if (q->free) q->free(elm->data);
			free(elm);
		}
		q->size = 0;
	}
}

tqueue_element* tqueue_find(tqueue* q, const void* data) {
	tqueue_element* elm = NULL;
	if (!q) return NULL;

	TAILQ_FOREACH(elm, &q->head, entry) {
		if ((q->compare ? q->compare(elm->data, data) : elm->data != data) == 0)
			return elm;
	}

	return elm;
}

tqueue_element* tqueue_nth_element(tqueue* q, unsigned index)
{
	tqueue_element* elm = NULL;
	unsigned i = 0;
	if (!q) return NULL;

	TAILQ_FOREACH(elm, &q->head, entry) {
		if (i++ == index) return elm;
	}

	return NULL;
}

int tqueue_foreach(tqueue* q, tqueue_foreach_func_t foreach,
		void* userdata)
{
	tqueue_element* elm = NULL;
	int rc = 0;

	TAILQ_FOREACH(elm, &(q->head), entry) {
		if ((rc = foreach(elm->data, userdata))) return rc;
	}

	return rc;
}

int tqueue_empty(tqueue* q)
{
	return q ? TAILQ_EMPTY(&q->head) : 1;
}

tqueue_element* tqueue_first(tqueue* q)
{
	return q ? TAILQ_FIRST(&q->head) : NULL;
}

tqueue_element* tqueue_last(tqueue* q)
{
	return tqueue_empty(q) ? NULL : TAILQ_LAST(&q->head, tqueue_head);
}

size_t tqueue_size(tqueue *q)
{
	return q ? q->size : 0;
}

void tqueue_free(tqueue* q)
{
	tqueue_element* elm = NULL;

	if (!q) return;

	while((elm = TAILQ_FIRST(&q->head))) {
		TAILQ_REMOVE(&q->head, elm, entry);
		if (q->free) q->free(elm->data);
		free(elm);
	}
	free(q);
}

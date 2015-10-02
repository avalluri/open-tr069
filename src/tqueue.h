/*
 * tqueue.h
 *
 *  Created on: Sep 22, 2015
 *      Author: avalluri
 */

#ifndef SRC_TQUEUE_H_
#define SRC_TQUEUE_H_

#include <sys/queue.h>

typedef struct _tqueue tqueue;

typedef struct _tqueue_element {
	void* data;
	TAILQ_ENTRY(_tqueue_element) entry;
} tqueue_element;

typedef int  (*tqueue_foreach_func_t)(void* element_data, void* userdata);
typedef int  (*tqueue_compare_func_t)(void* element_data, const void* find_data);
typedef void (*tqueue_free_func_t)(void *data);

tqueue* tqueue_new(tqueue_compare_func_t cmp_func,
		tqueue_free_func_t free_func);

tqueue_element* tqueue_insert(tqueue* q, void* data);

void tqueue_remove(tqueue* q, tqueue_element* elm);

void tqueue_remove_all(tqueue* q);

tqueue_element* tqueue_find(tqueue* q, const void* data);

tqueue_element* tqueue_nth_element(tqueue* q, unsigned index);

int tqueue_foreach(tqueue* q, tqueue_foreach_func_t func, void* userdata);

void tqueue_free(tqueue* q);

int tqueue_empty(tqueue* q);

size_t tqueue_size(tqueue *q);

tqueue_element* tqueue_first(tqueue* q);

tqueue_element* tqueue_last(tqueue* q);

#define tqueue_element_next(elm) TAILQ_NEXT(elm, entry)

#define tqueue_element_prev(elm) TAILQ_PREV(elm, entry)

#define TQUEUE_FOREACH(var, q) \
	for((var) = tqueue_first((q)); (var); (var) = TAILQ_NEXT(var, entry))


#endif /* SRC_TQUEUE_H_ */

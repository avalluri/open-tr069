/*
 * tqueue.h
 *
 *  Created on: Sep 22, 2015
 *      Author: avalluri
 */

#ifndef SRC_TQUEUE_H_
#define SRC_TQUEUE_H_

#include <sys/queue.h>

struct tqueue;

struct tqueue_element
{
	void* data;
	TAILQ_ENTRY(tqueue_element) entry;
};

typedef int  (* tqueue_foreach_func)(void* element_data, void* userdata);
typedef int  (* tqueue_compare_func)(void* element_data, void* find_data,
		void *userdata);
typedef void (* tqueue_free_func)(void *data);

struct tqueue* tqueue_new(tqueue_compare_func cmp_func,
		tqueue_free_func free_func);

struct tqueue_element* tqueue_insert(struct tqueue* q, void* data);

void tqueue_remove(struct tqueue* q, struct tqueue_element* elm);

struct tqueue_element* tqueue_find(struct tqueue* q, void* data, void* userdata);

struct tqueue_element* tqueue_nth_element(struct tqueue* q, unsigned index);

int tqueue_foreach(struct tqueue* q, tqueue_foreach_func func, void* userdata);

void tqueue_free(struct tqueue* q);

int tqueue_empty(struct tqueue* q);

struct tqueue_element* tqueue_first(struct tqueue* q);

struct tqueue_element* tqueue_last(struct tqueue* q);

#define tqueue_element_next(elm) TAILQ_NEXT(elm, entry)

#define tqueue_element_prev(elm) TAILQ_PREV(elm, entry)

#define TQUEUE_FOREACH(var, q) \
	for((var) = tqueue_first((q)); (var); (var) = TAILQ_NEXT(var, entry))

#endif /* SRC_TQUEUE_H_ */

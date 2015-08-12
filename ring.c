/*
 * Copyright (c) 2015, Marius Barbu <msb@avengis.com>
 * All rights reserved.
 *
 * Released under MIT/X11. See LICENSE for more information.
 */
#include "ring.h"

static inline size_t
ring_index(struct ring *r, size_t n)
{
	size_t until_end = r->count - r->start;
	return (n >= until_end) ? n - until_end : r->start + n;
}

static inline void *
ring_nth(struct ring *r, size_t n)
{
	return (char *) r->data + ring_index(r, n) * r->size;
}

void *
ring_front(struct ring *r)
{
	if (r->used == 0) {
		return 0;
	}
	return ring_nth(r, 0);
}

void *
ring_back(struct ring *r)
{
	if (r->used == 0) {
		return 0;
	}
	return ring_nth(r, r->used - 1);
}

void *
ring_push_back(struct ring *r)
{
	if (r->used == r->count) {
		return 0;
	}
	r->used++;
	return ring_nth(r, r->used - 1);
}

void *
ring_pop_back(struct ring *r)
{
	if (r->used == 0) {
		return 0;
	}
	r->used--;
	return ring_nth(r, r->used);
}

void *
ring_push_front(struct ring *r)
{
	if (r->used == r->count) {
		return 0;
	}
	r->start = r->start != 0 ? r->start - 1 : r->count - 1;
	r->used++;
	return ring_nth(r, 0);
}

void *
ring_pop_front(struct ring *r)
{
	void *old = ring_nth(r, 0);
	if (r->used == 0) {
		return 0;
	}
	r->used--;
	r->start++;
	if (r->start == r->count) {
		r->start = 0;
	}
	return old;
}

void *
ring_next(struct ring *r, void *elem)
{
	size_t index = (size_t) ((char *) elem - (char *) r->data) / r->size;
	if (index == (r->start + r->used - 1) % r->count) {
		return 0;
	}
	return ring_nth(r, index - r->start + 1);
}

void *
ring_prev(struct ring *r, void *elem)
{
	size_t index = (size_t) ((char *) elem - (char *) r->data) / r->size;
	if (index == r->start) {
		return 0;
	}
	index = index != 0 ? index - 1 : r->count - 1;
	return ring_nth(r, index - r->start);
}

#ifdef TEST_RING
#include <stdio.h>

static void
print_ring(struct ring *r)
{
	printf("ring length = %zu\n", r->used);
	printf("ring->start = %zu\n", r->start);
	printf("ring->end = %zu\n", (r->start + r->used) % r->count);
	RING_EACH(int, i, r) {
		printf("%d ", *i);
	}
	printf("\n");
}

static void
walk(struct ring *r)
{
	int times = 10000;
	int fwd = 1;
	for (int i = 0; i < times; i++) {
		int *v = fwd ? ring_push_back(r) : ring_push_front(r);
		if (v) {
			*v = i;
		} else {
			/* clear the buffer and go the opposite way */
			r->used = 0;
			fwd = !fwd;
			i--;
		}
	}
}

static void
test_ring(struct ring *r)
{
	for (size_t i = 0; i < 10000; i++) {
		walk(r);
	}
}

int
main()
{
	static int buff[4096];
	struct ring r = { buff, sizeof *buff, sizeof buff / sizeof *buff, };

	test_ring(&r);
	print_ring(&r);
}
#endif

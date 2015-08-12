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

size_t
ring_avail(struct ring *r, void **first, size_t *fcount, void **last,
    size_t *lcount)
{
	size_t first_avail = ring_index(r, r->used);

	if (!first) {
		/* caller doesn't need the regions */
		return r->count - r->used;
	}

	if (r->used == r->count) {
		/* no space left, return valid pointers anyway */
		*first = (char *) r->data + first_avail * r->size;
		*fcount = 0;
		if (last) {
			*last = *first;
			*lcount = 0;
		}
		return 0;
	}

	if (first_avail < r->start) {
		/*
		 * the are two used regions, so we have one region available in
		 * the middle [first_avail, r->start)
		 */
		*first = (char *) r->data + first_avail * r->size;
		*fcount = r->start - first_avail;
		if (last) {
			*last = (char *) *first + *fcount * r->size;
			*lcount = 0;
		}
	} else {
		/*
		 * the used region is in the middle of the ring, so we have two
		 * available regions: the first [first_avail, count) and the
		 * second [0, start)
		 */
		*first = (char *) r->data + first_avail * r->size;
		*fcount = r->count - first_avail;
		if (last) {
			*last = r->data;
			*lcount = r->start;
		}
	}

	return r->count - r->used;
}

size_t
ring_used(struct ring *r, void **first, size_t *fcount, void **last,
    size_t *lcount)
{
	size_t last_used;

	if (!first) {
		/* caller doesn't need the regions */
		return r->used;
	}

	if (r->used == 0) {
		/* nothing used, return valid pointers anyway */
		*first = (char *) r->data + r->start * r->size;
		*fcount = 0;
		if (last) {
			*last = *first;
			*lcount = 0;
		}
		return 0;
	}

	last_used = ring_index(r, r->used - 1);

	if (last_used < r->start) {
		/*
		 * the are two used regions
		 */
		*first = (char *) r->data + r->start * r->size;
		*fcount = r->count - r->start;
		if (last) {
			*last = r->data;
			*lcount = last_used + 1;
		}
	} else {
		/*
		 * the used region is in the middle of the ring
		 */
		*first = (char *) r->data + r->start * r->size;
		*fcount = last_used + 1 - r->start;
		if (last) {
			*last = (char *) *first + *fcount * r->size;
			*lcount = 0;
		}
	}

	return r->used;
}

#ifdef TEST_RING
#include <assert.h>
#include <stdio.h>

static void
print_regions(struct ring *r)
{
	struct {
		void *p;
		size_t n;
	} a[2], u[2];
	printf("start = %zu, used = %zu, count = %zu\n", r->start, r->used,
	    r->count);
	ring_avail(r, &a[0].p, &a[0].n, &a[1].p, &a[1].n);
	ring_used(r, &u[0].p, &u[0].n, &u[1].p, &u[1].n);
	printf("available: [%zu, %zu), [%zu, %zu)\n",
	    ((char *) a[0].p - (char *) r->data) / r->size,
	    ((char *) a[0].p - (char *) r->data) / r->size + a[0].n,
	    ((char *) a[1].p - (char *) r->data) / r->size,
	    ((char *) a[1].p - (char *) r->data) / r->size + a[1].n);
	printf("used: [%zu, %zu), [%zu, %zu)\n",
	    ((char *) u[0].p - (char *) r->data) / r->size,
	    ((char *) u[0].p - (char *) r->data) / r->size + u[0].n,
	    ((char *) u[1].p - (char *) r->data) / r->size,
	    ((char *) u[1].p - (char *) r->data) / r->size + u[1].n);
}

static void
test_regions(void)
{
	enum {
		N = 3,
	};
	char buff[N];
	struct ring r = { buff, sizeof *buff, N };
	printf("*** %s ***\n", __func__);
	for (size_t i = 0; i < N; i++) {
		r.start = i;
		for (size_t j = 0; j <= N; j++) {
			r.used = j;
			print_regions(&r);
		}
	}
}

static void
print_ring(struct ring *r)
{
	size_t asize;

	printf("ring capacity = %zu\n", r->count);
	printf("ring length = %zu\n", r->used);
	printf("ring->start = %zu\n", r->start);
	printf("ring->end = %zu\n", (r->start + r->used) % r->count);
	asize = ring_avail(r, NULL, NULL, NULL, NULL);
	printf("available elements = %zu\n",  asize / r->size);
	assert(asize % r->size == 0);
	print_regions(r);
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
	test_regions();
}
#endif

/*
 * Copyright (c) 2015, Marius Barbu <msb@avengis.com>
 * All rights reserved.
 *
 * Released under MIT/X11. See LICENSE for more information.
 */
#ifndef RING_H__
#define RING_H__

/*
 * A zero-copy generic ring buffer backed by an user-supplied array.
 * The library performs only state management; memory management is the
 * responsibility of the user.
 * Conceptually, this library offers iterators into a circular buffer.
 */

#include <stddef.h>

/*
 * A ring object keeps the state associated with a circular buffer. Before
 * calling any of the `ring_*` functions, the following initialization is
 * required:
 * - the `data` field must be a pointer to an element of an array
 * - the `size` field must be set to the size of an element
 * - the `count` field must be set to the number of elements pointed by `data`
 * - the `start` and used fields must be 0
 *
 * Intended usage:
 *
 * TYPE array[N];
 * struct ring r = { array, sizeof *array, sizeof array / sizeof *array };
 *
 * ...
 * TYPE *p = ring_push_back(&r);
 * // work with the element *p
 * ...
 *
 * RING_EACH(TYPE, s, &r) {
 *     // work with the element *s
 * }
 */
struct ring {
	void *data;
	size_t size;
	size_t count;
	size_t start;
	size_t used;
};

/*
 * Returns a pointer to the first element of the ring or NULL if there are no
 * elements in the ring buffer.
 */
void *ring_front(struct ring *r);

/*
 * Updates `r` to account for a new element stored at the front of the ring
 * buffer. Returns a pointer to the prepended element or NULL if there is no
 * space left.
 */
void *ring_push_front(struct ring *r);

/*
 * Updates `r` to account for the removal of the front element. Returns a
 * pointer to the removed element or NULL if there are no elements in the ring.
 * NOTE: any iterators pointing to the removed element are now invalid and
 * passing them to `ring_next` or `ring_prev` results in undefined behaviour.
 */
void *ring_pop_front(struct ring *r);

/*
 * Returns a pointer to the last element of the ring or NULL if there are no
 * elements in the ring buffer.
 */
void *ring_back(struct ring *r);

/*
 * Updates `r` to account for a new element stored at the back of the ring
 * buffer. Returns a pointer to the appended element or NULL if there is no
 * space left.
 */
void *ring_push_back(struct ring *r);

/*
 * Updates `r` to account for the removal of the back element. Returns a
 * pointer to the removed element or NULL if there are no elements in the ring.
 * NOTE: any iterators pointing to the removed element are now invalid and
 * passing them to `ring_next` or `ring_prev` results in undefined behaviour.
 */
void *ring_pop_back(struct ring *r);

/*
 * Returns a pointer to the next element in the ring, following `*p`, or NULL
 * if `*p` is the last element in the ring. The argument `p` must be a pointer
 * to an active element of the ring (located between `ring_front(r)` and
 * `ring_back(r)`), otherwise the behaviour is undefined.
 * NOTE: Calling the `ring_pop_*` functions might break the above invariant.
 */
void *ring_next(struct ring *r, void *p);

/*
 * Returns a pointer to the previous element in the ring, preceding `*p`, or
 * NULL if `*p` is the first element in the ring. The argument `p` must be a
 * pointer to an active element of the ring (located between `ring_front(r)`
 * and `ring_back(r)`), otherwise the behaviour is undefined.
 * NOTE: Calling the `ring_pop_*` functions might break the above invariant.
 */
void *ring_prev(struct ring *r, void *p);

/*
 * Convenience macro for iterating all the elements in a ring buffer, from
 * front to back.
 * `type` is the type of elements in the ring, `obj` is a variable that will
 * store a pointer to the current element and `ring` is a pointer to the
 * `struct ring` object that one wishes to iterate.
 * NOTE: Calling the `ring_pop_*` functions might break the requirement
 */
#define RING_EACH(type, obj, ring) \
    for (type *obj = ring_front(ring); obj != NULL; obj = ring_next(ring, obj))

#endif /* RING_H__ */

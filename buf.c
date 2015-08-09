/*
 * Copyright (c) 2015, Marius Barbu <msb@avengis.com>
 * All rights reserved.
 *
 * Released under MIT/X11. See LICENSE for more information.
 */
#include <stdbool.h> /* true/false are easier to spot */
#include <stdint.h> /* SIZE_MAX */
#include <string.h> /* memcpy */
#include "buf.h"

int
buf_ensure_capacity(struct buf *b, size_t cap)
{
	unsigned char *new_data;

	if (cap <= b->capacity) {
		return true;
	}
	if (!b->realloc) {
		return false;
	}
	new_data = b->realloc(b->data, cap);
	if (!new_data) {
		return false;
	}
	b->data = new_data;
	b->capacity = cap;
	return true;
}

int
buf_ensure_remaining(struct buf *b, size_t rem)
{
	if (b->capacity - b->size >= rem) {
		return true;
	}
	if (rem > SIZE_MAX - b->size) {
		return false;
	}
	return buf_ensure_capacity(b, b->size + rem);
}

int
buf_trim(struct buf *b)
{
	unsigned char *new_data;

	if (b->size == b->capacity) {
		return true;
	}
	if (!b->realloc) {
		return false;
	}
	new_data = b->realloc(b->data, b->size);
	if (!new_data) {
		return false;
	}
	b->data = new_data;
	b->capacity = b->size;
	return true;
}

int
buf_append(struct buf *b, const void *data, size_t size)
{
	if (!buf_ensure_remaining(b, size)) {
		return false;
	}
	memcpy(b->data + b->size, data, size);
	b->size += size;
	return true;
}

int
buf_deep_copy(struct buf *out, const struct buf *in)
{
	if (!buf_ensure_capacity(out, in->size)) {
		return false;
	}
	memcpy(out->data, in->data, in->size);
	out->size = in->size;
	return true;
}

#ifdef TEST_BUF
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int
main()
{
	unsigned char s[] = "Hello World!";
	struct buf b = BUF_IN(s);
	struct buf b2 = { .realloc = realloc };

	printf("first: %s\n", b.data);
	printf("copy: %d\n", buf_deep_copy(&b2, &b));
	assert(b2.size == b.size);
	b2.size--; /* remove trailing '\0' */
	printf("append: %d\n", buf_append(&b2, s, sizeof s));
	assert(b2.size == 2 * sizeof s - 1); /* only one '\0' */
	printf("second: %s\n", b2.data);
	printf("copy back should fail: %d\n", buf_deep_copy(&b, &b2));
	assert(b.size == sizeof s);
	printf("first: %s\n", b.data);
	free(b2.data);

	return 0;
}
#endif

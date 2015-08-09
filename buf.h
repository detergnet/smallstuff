/*
 * Copyright (c) 2015, Marius Barbu <msb@avengis.com>
 * All rights reserved.
 *
 * Released under MIT/X11. See LICENSE for more information.
 */
#ifndef BUF_H__
#define BUF_H__

/*
 * Wrapper for byte buffers that simplifies passing of buffers as arguments to
 * I/O related functions.
 * In general, any well-written function that has to fill a buffer must receive
 * as arguments a pointer to a region of memory to fill, the available size of
 * the memory to fill and it must return an error code and the number of
 * written bytes. The prototype would look like this:
 *
 *     int fill(void *data, size_t size, size_t *filled);
 *
 * making it a pretty verbose prototype as soon as you need to pass more
 * context to the function.
 * With the `struct buf` wrapper, all the information regarding the state of
 * the buffer is tracked in one place and you can also provide transparent
 * dynamic resizing of the buffer.
 *
 * Intended usage:
 *
 * char compressed_buf[4096];
 * struct buf deflated = BUF_OUT(compressed_buf);
 * struct buf inflated = { .realloc = realloc }; // dynamic buffer
 * if (get_compressed_data(&deflated) != 0) {
 *     fail("...");
 * }
 * printf("got %zu bytes in compressed buffer\n", deflated.size);
 * if (decompress(&inflated, &deflated) != 0) {
 *     fail("...");
 * }
 * printf("got %zu bytes of decompressed data\n", inflated.size);
 */

#include <stddef.h>

/*
 * The usable data stored in the buffer is in the range `[data, data + size)`.
 *
 * The `capacity` indicates the total size of the region pointed-to by the
 * `data` field.
 *
 * If the `realloc` field is not NULL, the buffer's capacity will be
 * dynamically adjusted when necessary, by calling the function pointed-to by
 * this field. If a custom memory allocator is not required, this field can be
 * initialised to point to the standard `realloc` function. If this field is
 * NULL, the buffer is non-resizable and any operation that requires a bigger
 * capacity then the buffer's current capacity will fail.
 */
struct buf {
	unsigned char *data;
	size_t size;
	size_t capacity;
	void *(*realloc)(void *, size_t);
};

/*
 * The return values must be interpreted as booleans: 0 is failure, anything
 * else is success.
 */

/*
 * Resize the buffer (if necessary) so that `b->capacity <= rem`.
 */
int buf_ensure_capacity(struct buf *b, size_t cap);

/*
 * Resize the buffer (if necessary) so that `b->size + rem <= b->capacity`.
 */
int buf_ensure_remaining(struct buf *b, size_t rem);

/*
 * If [data, data + size) overlaps [b->data, b->data + size) the behaviour is
 * undefined.
 */
int buf_append(struct buf *b, const void *data, size_t size);

/* Shrinks the buffer to the minimum required capacity (`capacity == size`). */
int buf_trim(struct buf *b);

/*
 * Copy the range `[in->data, in->data + in->size)` to
 * `[out->data, out->data + * in->size)`, increasing the `out` buffer capacity
 * if necessary.
 * If the ranges overlap, the behaviour is undefined.
 */
int buf_deep_copy(struct buf *out, const struct buf *in);

/*
 * Convenience macros for initializing `buf` objects that overlay some other
 * objects.
 * The `BUF_IN_C` macro is provided for those situations in which you want to
 * wrap an immutable object, as there is no way to change `unsigned char *data`
 * into the `const unsigned char *data` required for such an action.  So
 * instead, the macro removes any qualifier from the argument and returns a
 * `const struct buf` initializer to signal that this buffer should not be
 * changed. Unfortunately, the compiler won't catch any attempt to assign to
 * `*data`.
 *
 * NOTE: the trailing zeroes in the initializers, although not required, are
 * added in order to make sure the inclusion of this header doesn't trigger any
 * `missing field initializer` from overzealous compilers.
 */
/* buffer initializer for input objects */
#define BUF_IN(b) \
    (struct buf) { (void *) &b, sizeof b, sizeof b, 0 }
/* remove const from object, return const initializer; considered immutable */
#define BUF_IN_C(b) \
    (const struct buf) { (void *) &b, sizeof b, sizeof b, 0 }
/* buffer initializer for output objects */
#define BUF_OUT(b) \
    (struct buf) { (void *) &b, 0, sizeof b, 0 }

#endif /* BUF_H__ */

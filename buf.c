/* SPDX-License_identifier: AGPL-3.0-only */

#include "buf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCREMENT 128

#define DIE						\
    printf("Error: buf.c line %d: %m\n", __LINE__);	\
    exit(1);

struct buf {
    char *bytes;

    size_t bytes_capacity;
    size_t bytes_written;

    size_t cursor_offset;
};

buf *alloc_buf(void) {
    buf *b;

    b = malloc(sizeof(*b));
    if (b == NULL) {
	DIE;
    }

    b->bytes = malloc(INCREMENT);
    if (b->bytes == NULL) {
	DIE;
    }

    b->bytes[0] = '\0';
    b->bytes_capacity = INCREMENT;
    b->bytes_written = 0;
    b->cursor_offset = 0;
    return b;
}

void free_buf(buf **bp) {
    if (bp == NULL) {
	return;
    }

    free((*bp)->bytes);
    free(*bp);
    *bp = NULL;
}

void append_bytes(buf *b, const char *data, size_t data_len) {
    size_t new_size;
    char *new_buf;

    if (b->bytes_written + data_len + 1 >= b->bytes_capacity) {
	new_size = b->bytes_written + data_len + 1;
	if (new_size % INCREMENT) {
	    new_size = (new_size / INCREMENT + 1) * INCREMENT;
	}
	new_buf = realloc(b->bytes, new_size);
	if (new_buf == NULL) {
	    free_buf(&b);
	    DIE;
	}
	b->bytes = new_buf;
    }
    memcpy(b->bytes + b->bytes_written, data, data_len);
    b->bytes_written += data_len;
    b->bytes[b->bytes_written] = '\0';
}

const char *yield_line(buf *b, size_t *line_len_out) {
    char *end, *ret;

    if (b->cursor_offset == b->bytes_written) {
	*line_len_out = 0;
	return NULL;
    }

    ret = b->bytes + b->cursor_offset;
    end = strchr(b->bytes + b->cursor_offset, '\n');
    if (end == NULL) {
	end = b->bytes + b->bytes_written;
    }

    *line_len_out = end - ret;
    if (end != b->bytes + b->bytes_written) {
	/* skip newline */
	end += 1;
    }
    b->cursor_offset += end - ret;
    return ret;
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

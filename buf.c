/* SPDX-License_identifier: AGPL-3.0-only */

#include "buf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCREMENT 128

#define DIE						\
    printf("Error: buf.c line %d: %m\n", __LINE__);	\
    exit(1);

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

void write_bytes(buf *b, const char *data, size_t data_len) {
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

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

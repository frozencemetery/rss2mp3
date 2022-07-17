/* SPDX-License_identifier: AGPL-3.0-only */

#include "buf.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define INCREMENT 128

#define DIE                                                             \
    printf("Error: buf.c line %d: %s\n", __LINE__, strerror(errno));	\
    exit(1);

#define ROUND_UP(len) (((len - 1) / INCREMENT + 1) * INCREMENT)

struct buf {
    char *bytes;

    size_t bytes_capacity;
    size_t bytes_written;

    size_t cursor_offset;
};

char *destruct_buf(buf **bp) {
    char *ret;

    if (bp == NULL || *bp == NULL) {
	return NULL;
    }

    ret = (*bp)->bytes;

    free(*bp);
    *bp = NULL;

    return ret;
}

void flush_to_file(buf *b, const char *path) {
    ssize_t ret;
    int fd;

    fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (fd == -1) {
	DIE;
    }

    ret = write(fd, b->bytes, b->bytes_written);
    if (ret != (ssize_t)b->bytes_written) {
	DIE;
    }
    close(fd);
}

buf *new_buf(void) {
    buf *b;

    b = calloc(1, sizeof(*b));
    if (b == NULL) {
	DIE;
    }

    b->bytes = malloc(INCREMENT);
    if (b->bytes == NULL) {
	DIE;
    }
    b->bytes_capacity = INCREMENT;
    b->bytes[0] = '\0';

    return b;
}

buf *new_from_file(const char *path) {
    buf *b;
    int fd;
    struct stat s;
    size_t new_size;
    ssize_t len;

    b = calloc(1, sizeof(*b));
    if (b == NULL) {
	DIE;
    }

    fd = open(path, O_CREAT | O_RDONLY, 0600);
    if (fd == -1) {
	DIE;
    }

    if (fstat(fd, &s) < 0) {
	DIE;
    }

    b->bytes_written = s.st_size;
    new_size = ROUND_UP(s.st_size + 1);
    b->bytes = malloc(new_size);
    if (b->bytes == NULL) {
	DIE;
    }
    b->bytes_capacity = new_size;

    len = read(fd, b->bytes, s.st_size);
    if (len != s.st_size) {
	DIE;
    }
    b->bytes[b->bytes_written] = '\0';
    close(fd);
    return b;
}

void append_bytes(buf *b, const char *data, size_t data_len) {
    size_t new_size;
    char *new_buf;

    if (b->bytes_written + data_len + 1 >= b->bytes_capacity) {
	new_size = ROUND_UP(b->bytes_written + data_len + 1);
	new_buf = realloc(b->bytes, new_size);
	if (new_buf == NULL) {
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

void reset_cursor(buf *b) {
    b->cursor_offset = 0;
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

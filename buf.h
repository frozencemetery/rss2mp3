/* SPDX-License_identifier: AGPL-3.0-only */

#pragma once

#include <sys/types.h>

struct buf;
typedef struct buf buf;

buf *new_buf(void);
buf *new_from_file(const char *path);
char *destruct_buf(buf **bp);
void flush_to_file(buf *b, const char *path);
void append_bytes(buf *b, const char *data, size_t data_len);
const char *yield_line(buf *b, size_t *line_len_out);

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

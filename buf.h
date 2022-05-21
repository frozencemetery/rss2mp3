/* SPDX-License_identifier: AGPL-3.0-only */

#ifndef _BUF_H_
#define _BUF_H_

#include <sys/types.h>

typedef struct {
    char *bytes;

    size_t bytes_capacity;
    size_t bytes_written;
} buf;

buf *alloc_buf(void);
void free_buf(buf **bp);
void write_bytes(buf *b, const char *data, size_t data_len);

#endif /* !_BUF_H_ */

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

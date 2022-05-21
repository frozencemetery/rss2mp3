/* SPDX-License_identifier: AGPL-3.0-only */

#ifndef _BUF_H_
#define _BUF_H_

#include <sys/types.h>

struct buf;
typedef struct buf buf;

buf *alloc_buf(void);
void free_buf(buf **bp);
void append_bytes(buf *b, const char *data, size_t data_len);
const char *yield_line(buf *b, size_t *line_len_out);

#endif /* !_BUF_H_ */

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

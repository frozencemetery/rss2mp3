/* SPDX-License_identifier: AGPL-3.0-only */

#pragma once

#include "buf.h"

#include <stdbool.h>

struct feed_context;
typedef struct feed_context feed_context;

feed_context *load_feed(buf *raw_xml, char **title_out);
bool next_item(feed_context *ctx, char **title_out, char **guid_out,
               char **url_out);

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

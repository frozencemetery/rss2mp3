/* SPDX-License_identifier: AGPL-3.0-only */

#pragma once

#include "buf.h"

void dl_to_file(const char *url, const char *path);

buf *dl_to_buf(const char *url);

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

/* SPDX-License_identifier: AGPL-3.0-only */

#include "dl.h"

#include <curl/curl.h>
#include <stdlib.h>

#define assert(cond) \
    if (!(cond)) {                                      \
        printf("Error: dl.c line %d\n", __LINE__);      \
        exit(1);                                        \
    }

static size_t file_write_callback(char *ptr, size_t size, size_t nmemb,
                                  void *userdata) {
    FILE *f = userdata;
    return fwrite(ptr, size, nmemb, f);
}

static size_t buf_write_callback(char *ptr, size_t size, size_t nmemb,
                                 void *userdata) {
    buf *b = userdata;
    append_bytes(b, ptr, size * nmemb);
    return nmemb;
}

static inline void dl(const char *url, void *write_callback,
                      void *write_data) {
    CURL *curl;

    curl = curl_easy_init();
    assert(curl);

    /* This interface is needlessly confusing... it exposes an option to
     * turn it off, which we need to turn off, otherwise it defaults
     * being off, which is off. */
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

    /* This one is another awful default. */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)write_data);

    assert(!curl_easy_perform(curl));
    curl_easy_cleanup(curl);
}

void dl_to_file(const char *url, const char *path) {
    FILE *f;

    f = fopen(path, "w");
    assert(f);

    dl(url, file_write_callback, f);

    fclose(f);
}

buf *dl_to_buf(const char *url) {
    buf *b;

    b = new_buf();
    assert(b);

    dl(url, buf_write_callback, b);
    return b;
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

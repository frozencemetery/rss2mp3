/* SPDX-License_identifier: AGPL-3.0-only */

/* For getline(). */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "buf.h"
#include "dl.h"
#include "feed.h"

#define FEEDS_FILE ".podcasts"
#define GUIDS_FILE ".guids"

struct termios t;
buf *feeds;
buf *guids;

static void cleanup(void) {
    char *buffer;

    /* Re-enable canoncal mode. */
    t.c_lflag |= ICANON;
    tcsetattr(0, TCSANOW, &t);

    buffer = destruct_buf(&feeds);
    free(buffer);

    buffer = destruct_buf(&guids);
    free(buffer);
}

static _Noreturn void die(char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(1);
}

static void help(void) {
    printf("- h: help (this text)\n");
    printf("- a: add a feed\n");
    printf("- l: list feeds\n");
    printf("- u: update feeds\n");
    printf("- q: quit\n");
}

static void load_configs(void) {
    char *home;

    home = getenv("HOME");
    if (home == NULL) {
        die("TODO getenv\n");
    }
    if (chdir(home) == -1) {
        die("TODO chdir %m\n");
    }

    feeds = new_from_file(FEEDS_FILE);
    guids = new_from_file(GUIDS_FILE);
}

static void sanitize(char *s) {
    /* In my experience, the various Android filesystems and interfaces
     * therein tend to be very bad at handling special characters.  Rather
     * than risk creating a file that can't be deleted later, take the
     * coward's way out.
     *
     * My apologies to anyone titling their episodes in a language that
     * requires more of UTF-8.  Rather than create files with non-ascii
     * characters in their names, though, I'd prefer patches to derive the
     * filenames from the URL as well as the title. */

    for (size_t i = 0; s[i]; i++) {
        if ((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'A' && s[i] <= 'Z') ||
            (s[i] >= 'a' && s[i] <= 'z') || s[i] == '.' || s[i] == '_') {
            continue;
        }
        s[i] = '_';
    }
}

static bool seen_guid(char *guid) {
    size_t cur_len, guid_len;
    const char *cur;

    guid_len = strlen(guid);

    reset_cursor(guids);
    while ((cur = yield_line(guids, &cur_len))) {
        if (cur_len != guid_len) {
            continue;
        }

        if (!memcmp(guid, cur, cur_len)) {
            return true;
        }
    }
    return false;
}

static void record_guid(char *guid) {
    reset_cursor(guids);
    append_bytes(guids, guid, strlen(guid));
    append_bytes(guids, "\n", 1);
    flush_to_file(guids, GUIDS_FILE);
}

static void chdir_p(const char *dir) {
    struct stat s;
    int ret;

    ret = stat(dir, &s);
    if (ret && errno == ENOENT) {
        mkdir(dir, 0770);
    }
    ret = chdir(dir);
    if (ret) {
        die("TODO chdir %m\n");
    }
}

static void download_item(const char *feed_name, char *title, char *url) {
    int ret;
    char *filename;
    size_t filename_len;

    chdir_p("Podcasts");
    chdir_p(feed_name);

    sanitize(title);
    filename_len = strlen(title) + strlen(".mp3");
    filename = malloc(filename_len + 1);
    if (!filename) {
        die("TODO malloc %m\n");
    }
    memcpy(filename, title, strlen(title));
    memcpy(filename + strlen(title), ".mp3", strlen(".mp3"));
    filename[filename_len] = '\0';

    printf("url: %s\n", url);
    dl_to_file(url, filename);
    free(filename);

    ret = chdir("../..");
    if (ret) {
        die("TODO chdir %m\n");
    }
}

static void dl_help(void) {
    printf("- h: help (this text)\n");
    printf("- d: download item\n");
    printf("- s: skip item this time\n");
    printf("- f: finish with this feed\n");
    printf("- m: mark as seen\n");
    printf("- q: quit program entirely\n");
}

static void process_items(feed_context *fctx, const char *feed_name) {
    char *title, *guid, *url, cmd;

    while (next_item(fctx, &title, &guid, &url)) {
        if (!seen_guid(guid)) {
            printf("New item: %s\n", title);

        prompt:
            printf("{h for help} ");
            fflush(stdout);
            cmd = getchar();
            printf("\n");

            if (cmd == '\0' || cmd == 'q') {
                printf("see you in the next one...\n");
                exit(0);
            } else if (cmd == 'h') {
                dl_help();
                goto prompt;
            } else if (cmd == 'd') {
                download_item(feed_name, title, url);
                record_guid(guid);
            } else if (cmd == 's') {
                continue;
            } else if (cmd == 'f') {
                return;
            } else if (cmd == 'm') {
                record_guid(guid);
            } else {
                printf("unrecognized command: %c", cmd);
                printf("valid commands:\n");
                dl_help();
                goto prompt;
            }
        }

        feed_free(title);
        feed_free(guid);
        feed_free(url);
    }
}

static void update_feeds(void) {
    const char *line;
    char *url, *title;
    size_t line_len;
    buf *feed_raw;
    feed_context *fctx;

    while ((line = yield_line(feeds, &line_len))) {
        url = strndup(line, line_len);
        if (!url) {
            die("TODO strndup %m\n");
        }

        feed_raw = dl_to_buf(url);
        free(url);
        if (!feed_raw) {
            die("TODO dl error\n");
        }

        fctx = load_feed(feed_raw, &title);
        if (!fctx) {
            die("TODO load_feed error\n");
        }
        printf("Processing %s...\n", title);
        sanitize(title);
        process_items(fctx, title);
        feed_free(title);
    }
    reset_cursor(feeds);
}

static void add_url(void) {
    char *name, *url;
    size_t discard;
    ssize_t url_len;
    buf *feed_buf;
    feed_context *fctx;

    printf("URL: ");
    fflush(stdout);

    url = NULL;
    discard = 0;
    url_len = getline(&url, &discard, stdin);
    if (url_len < 0) {
        die("TODO getline %m\n");
    } else if (url_len == 0) {
        die("TODO you're bad at this\n");
    } else if (url_len < 12 || /* http://a is valid... but no. */
               (strncmp(url, "https://", 8) && strncmp(url, "http://", 7))) {
        die("TODO invalid URL\n");
    }
    while (url[url_len - 1] == '\n') {
        url[--url_len] = '\0';
    }

    feed_buf = dl_to_buf(url);
    if (!feed_buf) {
        die("TODO no download?\n");
    }

    fctx = load_feed(feed_buf, &name);
    if (!fctx) {
        die("TODO no parse?\n");
    }

    append_bytes(feeds, url, url_len);
    append_bytes(feeds, "\n", 1);
    free(url);
    flush_to_file(feeds, FEEDS_FILE);

    process_items(fctx, name);
    feed_free(name);
    unload_feed(fctx);
}

int main() {
    char cmd;
    const char *line;
    size_t line_len;

    /* Disable canonical mode, etc. so we don't wait for newlines. */
    if (tcgetattr(0, &t)) {
        die("tcgetattr: %m\n");
    }
    t.c_lflag &= ~ICANON;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &t)) {
        die("tcsetattr: %m\n");
    }

    atexit(cleanup);
    load_configs();

    while (1) {
        printf("[h for help] ");
        fflush(stdout);

        cmd = getchar();
        printf("\n");
        if (cmd == '\0' || cmd == 'q') {
            printf("see you in the next one...\n");
            exit(0);
        } else if (cmd == 'h') {
            help();
        } else if (cmd == 'a') {
            add_url();
        } else if (cmd == 'l') {
            while (1) {
                line = yield_line(feeds, &line_len);
                if (line == NULL) {
                    break;
                }
                printf("%.*s\n", line_len, line);
            }
            reset_cursor(feeds);
        } else if (cmd == 'u') {
            update_feeds();
        } else {
            printf("command not recognized! Valid commands:\n");
            help();
        }
    }

    printf("see you in the next one...\n");;
    return 0;
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

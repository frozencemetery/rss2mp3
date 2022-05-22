/* SPDX-License_identifier: AGPL-3.0-only */

/* For getline(). */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "buf.h"

#define CONFIG_FILE ".podcasts"

struct termios t;
buf *config;

static void cleanup(void) {
    char *buffer;

    /* Re-enable canoncal mode. */
    t.c_lflag |= ICANON;
    tcsetattr(0, TCSANOW, &t);

    buffer = destruct_buf(&config);
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
    printf("- q: quit\n");
}

static buf *load_config(void) {
    char *home;

    home = getenv("HOME");
    if (home == NULL) {
        die("TODO getenv\n");
    }
    if (chdir(home) == -1) {
        die("TODO chdir %m\n");
    }

    return new_from_file(CONFIG_FILE);
}

int main() {
    char cmd, *name, *url;
    const char *line;
    size_t line_len, discard;
    ssize_t name_len, url_len;

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
            if (config == NULL) {
                config = load_config();
            }

            printf("Feed name: ");
            fflush(stdout);

            name = NULL;
            discard = 0;
            name_len = getline(&name, &discard, stdin);
            if (name_len < 0) {
                die("TODO getline %m\n");
            } else if (name_len == 0) {
                die("TODO you're bad at this\n");
            }

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
                       (strncmp(url, "https://", 8) &&
                        strncmp(url, "http://", 7))) {
                die("TODO invalid URL\n");
            }

            append_bytes(config, "n: ", 3);
            append_bytes(config, name, name_len);
            append_bytes(config, "u: ", 3);
            append_bytes(config, url, url_len);
            free(name);
            free(url);
            flush_to_file(config, CONFIG_FILE);
        } else if (cmd == 'l') {
            if (config == NULL) {
                config = load_config();
            }

            while (1) {
                line = yield_line(config, &line_len);
                if (line == NULL) {
                    break;
                }
                if (line[0] == 'n' || line[0] == 'u') {
                    printf("%.*s\n", line_len, line);
                }
            }
            reset_cursor(config);
        } else {
            printf("command not recognized! Valid commands:\n");
            help();
        }
    }

    free(config);
    printf("see you in the next one...\n");;
    return 0;
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

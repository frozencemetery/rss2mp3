/* SPDX-License_identifier: AGPL-3.0-only */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "buf.h"

struct termios t;

_Noreturn void die(void) {
    /* Re-enable canoncal mode. */
    t.c_lflag |= ICANON;
    tcsetattr(0, TCSANOW, &t);

    printf("see you in the next one...\n");
    exit(0);
}

void help(void) {
    printf("- h: help (this text)\n");
    printf("- l: list feeds\n");
    printf("- q: quit\n");
}

buf *load_config(void) {
    char *home, cursor[1024];
    int fd;
    buf *b;
    ssize_t len;

    b = alloc_buf();

    home = getenv("HOME");
    if (home == NULL) {
        printf("TODO getenv\n");
        die();
    }
    if (chdir(home) == -1) {
        printf("TODO chdir\n");
        die();
    }

    fd = open(".podcasts", O_CREAT | O_RDONLY, 0600);
    if (fd == -1) {
        printf("TODO open %m\n");
        die();
    }

    while (1) {
        len = read(fd, cursor, sizeof(*cursor));
        if (len < 0) {
            printf("TODO read %m\n");
            die();
        } else if (len == 0) {
            break;
        }
        append_bytes(b, cursor, len);
    }
    close(fd);

    return b;
}

int main() {
    char cmd;
    const char *line;
    buf *config = NULL;
    size_t line_len;

    /* Disable canonical mode, etc. so we don't wait for newlines. */
    if (tcgetattr(0, &t)) {
        printf("tcgetattr: %m\n");
        exit(1);
    }
    t.c_lflag &= ~ICANON;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &t)) {
        printf("tcsetattr: %m\n");
        exit(1);
    }

    while (1) {
        printf("[h for help] ");
        fflush(stdout);

        cmd = getchar();
        printf("\n");
        if (cmd == '\0' || cmd == 'q') {
            die();
        } else if (cmd == 'h') {
            help();
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
        } else {
            printf("command not recognized! Valid commands:\n");
            help();
        }
    }

    free(config);
    die();
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

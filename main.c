/* SPDX-License_identifier: AGPL-3.0-only */

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

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
    printf("- q: quit\n");
}

int main() {
    char cmd;

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
        } else {
            printf("command not recognized! Valid commands:\n");
            help();
        }
    }

    die();
}
/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */

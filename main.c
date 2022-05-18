/* SPDX-License_identifier: AGPL-3.0-only */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
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
    printf("- l: list feeds\n");
    printf("- q: quit\n");
}

char *load_config(size_t *config_len) {
    char *home, *config;
    struct stat s;
    int fd;

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

    if (fstat(fd, &s) == -1) {
	printf("TODO fstat %m\n");
	die();
    }

    config = malloc(*config_len);
    if (config == NULL) {
	printf("TODO malloc %m\n");
	die();
    }

    if (s.st_size > 0 && read(fd, config, s.st_size) != s.st_size) {
	printf("TODO read %m\n");
	die();
    }
    config[s.st_size] = '\0';

    *config_len = s.st_size + 1;
    return config;
}

int main() {
    char cmd, *config = NULL, *cursor, *line_end;
    size_t config_len;

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
		config = load_config(&config_len);
		if (config == NULL) {
		    printf("TODO\n");
		    die();
		}
	    }

	    cursor = config;
	    while (1) {
		line_end = strchr(cursor, '\n');
		if (line_end == NULL) {
		    break;
		}
		line_end++;
		if (cursor[0] == 'n' || cursor[0] == 'u') {
		    printf("%.*s", line_end - cursor, cursor);
		}
		cursor = line_end;
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

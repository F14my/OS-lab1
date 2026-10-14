#include "line_io.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

int read_line(FILE *input, char **line, size_t *capacity) {
    size_t length = 0;
    int character;

    if (*line == NULL || *capacity == 0) {
        *line = malloc(128);
        if (*line == NULL) {
            errno = ENOMEM;
            return -1;
        }
        *capacity = 128;
    }

    while ((character = fgetc(input)) != EOF) {
        if (length + 1 >= *capacity) {
            size_t new_capacity;
            char *new_line;

            if (*capacity > SIZE_MAX / 2) {
                errno = ENOMEM;
                return -1;
            }
            new_capacity = *capacity * 2;
            new_line = realloc(*line, new_capacity);
            if (new_line == NULL) {
                errno = ENOMEM;
                return -1;
            }
            *line = new_line;
            *capacity = new_capacity;
        }

        (*line)[length++] = (char)character;
        if (character == '\n') {
            break;
        }
    }

    if (ferror(input)) {
        return -1;
    }
    if (character == EOF && length == 0) {
        return 0;
    }

    (*line)[length] = '\0';
    return 1;
}

#include "line_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int reverse_line(char *line) {
    size_t length = strlen(line);
    size_t text_length = length;
    size_t output_position = 0;
    char *reversed;

    if (length > 0 && line[length - 1] == '\n') {
        --text_length;
    }
    if (text_length > 0 && line[text_length - 1] == '\r') {
        --text_length;
    }
    reversed = malloc(text_length + 1);
    if (reversed == NULL) {
        return -1;
    }

    while (text_length > 0) {
        size_t start = text_length - 1;
        while (start > 0 &&
               (((unsigned char)line[start] & 0xC0) == 0x80)) {
            --start;
               }
        memcpy(reversed + output_position, line + start, text_length - start);
        output_position += text_length - start;
        text_length = start;
    }

    memcpy(line, reversed, output_position);
    free(reversed);
    return 0;
}

int main(void) {
    char *line = NULL;
    size_t capacity = 0;
    int read_result;

    while ((read_result = read_line(stdin, &line, &capacity)) == 1) {
        if (reverse_line(line) == -1) {
            perror("Не удалось обработать строку");
            free(line);
            return EXIT_FAILURE;
        }

        if (fputs(line, stdout) == EOF) {
            perror("Ошибка записи результата");
            free(line);
            return EXIT_FAILURE;
        }
    }

    if (read_result == -1) {
        perror("Ошибка чтения строки");
        free(line);
        return EXIT_FAILURE;
    }

    free(line);
    return EXIT_SUCCESS;
}


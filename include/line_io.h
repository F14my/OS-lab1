#ifndef LINE_IO_H
#define LINE_IO_H

#include <stddef.h>
#include <stdio.h>

/* 1: строка прочитана, 0: конец ввода, -1: ошибка. '\n' сохраняется. */
int read_line(FILE *input, char **line, size_t *capacity);

#endif

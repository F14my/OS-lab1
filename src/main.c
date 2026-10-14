#include "line_io.h"
#include "process.h"

#include <errno.h>
#ifndef _WIN32
#include <signal.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef CHILD_EXECUTABLE_PATH
#ifdef _WIN32
#define CHILD_EXECUTABLE_PATH ".\\lab2_child.exe"
#else
#define CHILD_EXECUTABLE_PATH "./lab2_child"
#endif
#endif

static int read_filename(const char *prompt, char **filename) {
    size_t capacity = 0;

    fputs(prompt, stdout);
    fflush(stdout);

    int read_result = read_line(stdin, filename, &capacity);
    if (read_result != 1) {
        if (read_result == 0) {
            errno = EINVAL;
        }
        return -1;
    }

    (*filename)[strcspn(*filename, "\r\n")] = '\0';
    if ((*filename)[0] == '\0') {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

static int choose_child(void) {
    return (rand() % 100 < 80) ? 1 : 2;
}

static int route_lines(ChildProcess *first_child, ChildProcess *second_child) {
    char *line = NULL;
    size_t capacity = 0;
    int read_result;

#ifdef _WIN32
    puts("Введите строки. Для завершения ввода нажмите Ctrl+Z, затем Enter.");
#else
    puts("Введите строки. Для завершения ввода нажмите Ctrl+D.");
#endif

    while ((read_result = read_line(stdin, &line, &capacity)) == 1) {
        int child_number = choose_child();
        ChildProcess *target = (child_number == 1) ? first_child : second_child;

        if (write_all(target, line, strlen(line)) == -1) {
            print_process_error("Не удалось отправить строку ребёнку");
            free(line);
            return -1;
        }

        printf("Строка отправлена child%d\n", child_number);
    }

    if (read_result == -1) {
        perror("Ошибка чтения строки");
        free(line);
        return -1;
    }

    free(line);
    return 0;
}

int main(void) {
    char *first_filename = NULL;
    char *second_filename = NULL;
    ChildProcess first_child = {0};
    ChildProcess second_child = {0};
    int result = EXIT_FAILURE;

#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (read_filename("Файл для child1: ", &first_filename) == -1 ||
        read_filename("Файл для child2: ", &second_filename) == -1) {
        perror("Не удалось прочитать имя файла");
        free(first_filename);
        free(second_filename);
        return EXIT_FAILURE;
    }

    if (strcmp(first_filename, second_filename) == 0) {
        fputs("Файлы для child1 и child2 должны различаться\n", stderr);
        goto cleanup;
    }

    printf("Файлы результатов: %s и %s\n", first_filename, second_filename);
    srand((unsigned int)time(NULL));

#ifndef _WIN32
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("Не удалось настроить обработку SIGPIPE");
        goto cleanup;
    }
#endif

    if (start_child(&first_child, CHILD_EXECUTABLE_PATH, first_filename) == -1) {
        print_process_error("Не удалось запустить child1");
        goto cleanup;
    }
    if (start_child(&second_child, CHILD_EXECUTABLE_PATH, second_filename) == -1) {
        print_process_error("Не удалось запустить child2");
        goto cleanup;
    }

    if (route_lines(&first_child, &second_child) == -1) {
        goto cleanup;
    }

    result = EXIT_SUCCESS;

cleanup:
    if (first_child.started && finish_child(&first_child) == -1) {
        print_process_error("Ошибка при завершении child1");
        result = EXIT_FAILURE;
    }
    if (second_child.started && finish_child(&second_child) == -1) {
        print_process_error("Ошибка при завершении child2");
        result = EXIT_FAILURE;
    }

    free(first_filename);
    free(second_filename);
    return result;
}

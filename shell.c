#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

void loop(void);
char* read_line(void);
char** parse_line(char* line);
int execute_command(char** args);

int builtin_cd(char** args);
int builtin_help(char** args);
int builtin_exit(char** args);

char* builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_fn[])(char**) = {
    builtin_cd,
    builtin_help,
    builtin_exit
};

int builtins_num() {
    return sizeof(builtin_str) / sizeof(char*);
}

int main(int argc, char** argv) {
    loop();
}

void loop(void) {
    while (true) {
        printf("> ");
        fflush(stdout);

        char* line = read_line();
        char** args = parse_line(line);
        int status = execute_command(args);

        free(line);
        free(args);

        if (!status) {
            break;
        }
    }
}

char* read_line(void) {
    size_t capacity = 1024;
    char* line = malloc(capacity);
    if (!line) {
        fprintf(stderr, "shell: allocation error\n");
        exit(1);
    }

    int c;
    size_t i = 0;

    while ((c = getchar()) != EOF && c != '\n') {
        if (i + 1 >= capacity) {
            capacity *= 2;
            char* temp = realloc(line, capacity);
            if (!temp) {
                free(line);
                fprintf(stderr, "shell: allocation error\n");
                exit(1);
            }
            line = temp;
        }

        line[i++] = c;
    }

    if (i == 0 && c == EOF) {
        free(line);
        exit(1);
    }

    line[i] = '\0';
    return line;
}

char** parse_line(char* line) {
    size_t capacity = 64;
    char** tokens = malloc(capacity * sizeof *tokens);
    if (!tokens) {
        fprintf(stderr, "shell: allocation error\n");
        exit(1);
    }

    const char* delim = " \t";
    char* token = strtok(line, delim);
    size_t i = 0;

    while (token) {
        if (i + 1 >= capacity) {
            capacity *= 2;
            char** temp = realloc(tokens, capacity * sizeof *tokens);
            if (!temp) {
                free(tokens);
                fprintf(stderr, "shell: allocation error\n");
                exit(1);
            }
            tokens = temp;
        }

        tokens[i++] = token;
        token = strtok(NULL, delim);
    }

    tokens[i] = NULL;
    return tokens;
}

int builtin_cd(char** args) {
    if (!args[1]) {
        fprintf(stderr, "shell: cd: expected argument\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

int builtin_help(char** args) {
    printf("shell builtins:\n");
    for (int i = 0; i < builtins_num(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    return 1;
}

int builtin_exit(char** args) {
    return 0;
}
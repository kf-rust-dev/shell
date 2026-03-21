#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>

void loop(void);
char* read_line(char* prompt);
char** parse_line(char* line);
int execute_command(char** args);
int run_command(char** args);

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
        char* line = read_line("> ");
        char** args = parse_line(line);
        int status = execute_command(args);

        free(line);
        free(args);

        if (!status) {
            break;
        }
    }
}

char* read_line(char* prompt) {
    char* line = readline(prompt);
    if (!line) {
        exit(0);
    }
    
    if (strlen(line) > 0) {
        add_history(line);
    }

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

int execute_command(char** args) {
    if (!args[0]) {
        return 1;
    }

    for (int i = 0; i < builtins_num(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return builtin_fn[i](args);
        }
    }

    return run_command(args);
}

int run_command(char** args) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("shell");
        return 1;
    }

    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("shell");
            exit(1);
        }
    }

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("shell");
    }
    return 1;
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
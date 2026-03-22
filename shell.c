#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>

void init(void);
void loop(void);
void cleanup(void);

char* read_line(char* prompt);
char** parse_line(char* line);
int execute_command(char** args);
int run_command(char** args);

int builtin_cd(char** args);
int builtin_help(char** args);
int builtin_exit(char** args);
int builtin_history(char** args);

typedef struct {
    char* name;
    int (*func)(char**);
} builtin;

builtin builtins[] = {
    {"cd", builtin_cd},
    {"help", builtin_help},
    {"exit", builtin_exit},
    {"history", builtin_history}
};

int builtins_num() {
    return sizeof(builtins) / sizeof(builtin);
}

int main(int argc, char** argv) {
    init();
    loop();
    cleanup();
}

void init(void) {
    read_history(".shell_history");
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

void cleanup(void) {
    write_history(".shell_history");
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

    size_t i = 0;
    char* dst = line;
    bool in_single = false;
    bool in_double = false;
    bool in_token = false;
    bool escape = false;
        
    for (char* src = line; *src; src++) {
        char c = *src;

        if (escape) {
            if (!in_token) {
                tokens[i++] = dst;
                in_token = true;
            }
            *dst++ = c;
            escape = false;
            continue;
        }

        switch (c) {
            case '\\':
                if (!in_single) {
                    escape = true;
                    continue;
                }
                break;
            case '"':
                if (!in_single) {
                    if (!in_token) {
                        tokens[i++] = dst;
                        in_token = true;
                    }
                    in_double = !in_double;
                    continue;
                }
                break;
            case '\'':
                if (!in_double) {
                    if (!in_token) {
                        tokens[i++] = dst;
                        in_token = true;
                    }
                    in_single = !in_single;
                    continue;
                }
                break;
            case ' ':
            case '\t':
                if (!in_single && !in_double) {
                    if (in_token) {
                        *dst++ = '\0';
                        in_token = false;
                    }
                    continue;
                }
                break;
            default:
                break;
        }

        if (!in_token) {
            tokens[i++] = dst;
            in_token = true;
        }
        *dst++ = c;

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
    }

    if (in_token) {
        *dst++ = '\0';
    }

    if (in_single || in_double) {
        fprintf(stderr, "shell: unmatched quote\n");
        free(tokens);
        return NULL;
    }

    tokens[i] = NULL;
    return tokens;
}

int execute_command(char** args) {
    if (!args || !args[0]) {
        return 1;
    }

    for (int i = 0; i < builtins_num(); i++) {
        if (strcmp(args[0], builtins[i].name) == 0) {
            return builtins[i].func(args);
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
    char* dir = args[1];
    if (!dir) {
        dir = getenv("HOME");
    }

    if (chdir(dir) != 0) {
        perror("shell");
    }

    return 1;
}

int builtin_help(char** args) {
    printf("shell builtins:\n");
    for (int i = 0; i < builtins_num(); i++) {
        printf("  %s\n", builtins[i].name);
    }
    return 1;
}

int builtin_exit(char** args) {
    return 0;
}

int builtin_history(char** args) {
    HIST_ENTRY** history = history_list();
    if (history) {
        for (int i = 0; history[i]; i++) {
            printf("%d %s\n", i + history_base, history[i]->line);
        }
    }
    return 1;
}
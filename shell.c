#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void loop(void);
char* read_line(void);
char** parse_line(char* line);
int execute_command(char** args);

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
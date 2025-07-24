#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


#define MAX_WORDS 200 // Maximum number of words that can be entered
#define MAX_INPUTS 200 // Maximum number of inputs per line (Safe handling)
#define MAX_INPUT_LENGTH 200 // Maximum size of each input word

int execute(char *exec_args[]) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        if (execvp(exec_args[0], exec_args) == -1) {
            perror("execvp failed");
        }
    } else {
        wait(NULL); 
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int arg_count = argc - 1;
    int no_of_inputs = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '%') {
            no_of_inputs++;
        }
    }

    if (no_of_inputs > 8) {
        fprintf(stderr, "Error: More than 8 input words not allowed\n");
        return 1;
    }

    char lines[MAX_WORDS][no_of_inputs][MAX_INPUT_LENGTH];  
    int word_counts[MAX_WORDS] = {0}; 
    int line_count = 0;

    char input[256];

    while (fgets(input, sizeof(input), stdin) != NULL) {
        char *token = strtok(input, " \t\n");
        int w = 0;

        while (token != NULL && w < no_of_inputs) {
            strcpy(lines[line_count][w], token); 
            w++;
            token = strtok(NULL, " \t\n");
        }

        if (token == NULL && w > 0) {  
            word_counts[line_count] = w;
            line_count++;
        } else if (token != NULL) {
            printf("Error: More than %d input words. \n", no_of_inputs);
        }

        if (line_count >= 100) break;
    }

    char args[arg_count][MAX_INPUT_LENGTH];  
    char *exec_args[arg_count+1];
    int a_index = 0;

    for (int i = 0; i < line_count; i++) {
        a_index = 0;

        for (int k = 1; k < argc - no_of_inputs; k++) {
            if (argv[k][0] != '%') {
                strcpy(args[a_index], argv[k]);
                exec_args[a_index] = args[a_index];
                a_index++;
            }
        }

        for (int j = 0; j < word_counts[i]; j++) {
            strcpy(args[a_index], lines[i][j]);
            exec_args[a_index] = args[a_index];
            a_index++;
        }

        exec_args[a_index] = NULL; 

        if (execute(exec_args) != 0) {
            fprintf(stderr, "Execution failed for line %d\n", i + 1);
        }
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT_SIZE 1024
// #define DEBUG

void parse_command(char *command, int *arg_count, char *args[100], int *input_redir, int *output_redir, char **input_file, char **output_file) {
    int skip_next = 0; // 0: do not skip next token; 1: skip next token for input redirection; 2: skip next token for output redirection
    *input_file = NULL;
    *output_file = NULL;
    char *token = strtok(command, " \t\n");
    while (token != NULL) {
        if (strcmp(token, ">") == 0) {
            if (*output_redir != 0) {
                fprintf(stderr, "syntax error: multiple output files\n");
            }
            *output_redir = 1;
            skip_next = 2;
        }
        else if (strcmp(token, ">>") == 0) {
            if (*output_redir != 0) {
                fprintf(stderr, "syntax error: multiple output files\n");
            }
            *output_redir = 2;
            skip_next = 2;
        }
        else if (strcmp(token, "<") == 0) {
            if (*input_redir != 0) {
                fprintf(stderr, "syntax error: multiple input files\n");
            }
            *input_redir = 1;
            skip_next = 1;
        }
        else {
            if (skip_next == 0) {
                args[*arg_count] = token;
                (*arg_count)++;
            }
            else {
                if (skip_next == 1) {
                    *input_file = token;
                }
                else if (skip_next == 2) {
                    *output_file = token;
                }
                skip_next = 0;
            }
        }
        token = strtok(NULL, " \t\n");
    }
    args[*arg_count] = NULL;
}

void execute_command(char *command) {
    char *args[100];
    int arg_count = 0;

    // int skip_next = 0; // 0: do not skip next token; 1: skip next token for input redirection; 2: skip next token for output redirection
    int input_redir = 0; // 0: no redirection; 1: <
    int output_redir = 0; // 0: no redirection; 1: > ; 2: >>

    char *input_file = NULL;
    char *output_file = NULL;

    parse_command(command, &arg_count, args, &input_redir, &output_redir, &input_file, &output_file);

    // char *token = strtok(command, " \t\n");
    // while (token != NULL) {
    //     if (strcmp(token, ">") == 0) {
    //         if (output_redir != 0) {
    //             fprintf(stderr, "syntax error: multiple output files\n");
    //         }
    //         output_redir = 1;
    //         skip_next = 2;
    //     }
    //     else if (strcmp(token, ">>") == 0) {
    //         if (output_redir != 0) {
    //             fprintf(stderr, "syntax error: multiple output files\n");
    //         }
    //         output_redir = 2;
    //         skip_next = 2;
    //     }
    //     else if (strcmp(token, "<") == 0) {
    //         if (input_redir != 0) {
    //             fprintf(stderr, "syntax error: multiple input files\n");
    //         }
    //         input_redir = 1;
    //         skip_next = 1;
    //     }
    //     else {
    //         if (skip_next == 0) {
    //             args[arg_count] = token;
    //             arg_count++;
    //         }
    //         else {
    //             if (skip_next == 1) {
    //                 input_file = token;
    //             }
    //             else if (skip_next == 2) {
    //                 output_file = token;
    //             }
    //             skip_next = 0;
    //         }
    //     }
    //     token = strtok(NULL, " \t\n");
    // }
    // args[arg_count] = '\0';

    #ifdef DEBUG
    printf("input file: %s\n", input_file);
    printf("output file: %s\n", output_file);
    printf("The command is: \n");
    for (int i = 0; i < arg_count; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    #endif

    if (arg_count == 0) {
        return;
    }

    if (strcmp(args[0], "cd") == 0) {
        if (arg_count != 2) {
            fprintf(stderr, "cd: syntax error\n");
        }
        else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
    } 
    else if (strcmp(args[0], "ln") == 0) {
        if (arg_count != 3) {
            fprintf(stderr, "ln: syntax error\n");
        } 
        else {
            if (link(args[1], args[2]) != 0) {
                perror("ln");
            }
        }
    }
    else if (strcmp(args[0], "rm") == 0) {
        if (arg_count != 2) {
            fprintf(stderr, "rm: syntax error\n");
        } 
        else {
            if (unlink(args[1]) != 0) {
                perror("rm");
            }
        }
    } 
    else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }
    else {

        pid_t pid = fork();
        int status;

        if (pid == 0) {
            if (input_redir != 0 || output_redir != 0) {
                if (input_redir != 0) {
                    close(0);
                    if (open(input_file, O_RDONLY) == -1) {
                        perror(input_file);
                        exit(1);
                    }
                }

                if (output_redir != 0) {
                    close(1);
                    if (output_redir == 1 && open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0777) == -1) {
                        perror(output_file);
                        exit(1);
                    }
                    if (output_redir == 2 && open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0777) == -1) {
                        perror(output_file);
                        exit(1);
                    }
                }
            }


            if (execv(args[0], args) == -1) {
                perror("execv");
            }
            exit(1);
        } 
        waitpid(pid, &status, 0);
    }
    
}

int main() {
    char input[MAX_INPUT_SIZE];

    while (1) {
        #ifdef PROMPT
        printf("33sh> ");
        #endif
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        execute_command(input);
    }
    
    return 0;
}

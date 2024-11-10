#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "./jobs.h"

#define MAX_INPUT_SIZE 1024
// #define DEBUG

job_list_t *job_list;
int jid = 1;

void parse_command(char *command, int *arg_count, char *args[100],
                   int *input_redir, int *output_redir, char **input_file,
                   char **output_file, int *is_bg_job) {
    int amp_pos = -1;
    int skip_next =
        0;  // 0: do not skip next token; 1: skip next token for input
            // redirection; 2: skip next token for output redirection
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
        } else if (strcmp(token, ">>") == 0) {
            if (*output_redir != 0) {
                fprintf(stderr, "syntax error: multiple output files\n");
            }
            *output_redir = 2;
            skip_next = 2;
        } else if (strcmp(token, "<") == 0) {
            if (*input_redir != 0) {
                fprintf(stderr, "syntax error: multiple input files\n");
            }
            *input_redir = 1;
            skip_next = 1;
        } else if (strcmp(token, "&") == 0) {
            amp_pos = *arg_count;
        } else {
            if (skip_next == 0) {
                args[*arg_count] = token;
                (*arg_count)++;
            } else {
                if (skip_next == 1) {
                    *input_file = token;
                } else if (skip_next == 2) {
                    *output_file = token;
                }
                skip_next = 0;
            }
        }
        token = strtok(NULL, " \t\n");
    }

    args[*arg_count] = NULL;

    if (amp_pos == *arg_count) {
        *is_bg_job = 1;
    }
}

void check_background_job() {
    int status;
    pid_t cur_pid;
    while ((cur_pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) >
           0) {
        int cur_jid = get_job_jid(job_list, cur_pid);
        if (WIFEXITED(status)) {
            fprintf(stdout, "[%d] (%d) terminated with exit status %d\n",
                    cur_jid, cur_pid, WEXITSTATUS(status));
            remove_job_pid(job_list, cur_pid);
        } else if (WIFSIGNALED(status)) {
            fprintf(stdout, "[%d] (%d) terminated by signal %d\n", cur_jid,
                    cur_pid, WTERMSIG(status));
            remove_job_pid(job_list, cur_pid);
        } else if (WIFSTOPPED(status)) {
            update_job_pid(job_list, cur_pid, STOPPED);
            fprintf(stdout, "[%d] (%d) suspended by signal %d\n", cur_jid,
                    cur_pid, WSTOPSIG(status));
        } else if (WIFCONTINUED(status)) {
            update_job_pid(job_list, cur_pid, RUNNING);
            fprintf(stdout, "[%d] (%d) resumed\n", cur_jid, cur_pid);
        }
    }
}

void execute_command(char *command) {
    char *args[100];    // parsed command line
    int arg_count = 0;  // length of command line

    int input_redir = 0;   // 0: no redirection; 1: <
    int output_redir = 0;  // 0: no redirection; 1: > ; 2: >>

    int is_bg_job = 0;  // 0: fg; 1: bg

    char *input_file;
    char *output_file;

    parse_command(command, &arg_count, args, &input_redir, &output_redir,
                  &input_file, &output_file, &is_bg_job);

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
        check_background_job();
        return;
    }

    if (strcmp(args[0], "cd") == 0) {
        if (arg_count != 2) {
            fprintf(stderr, "cd: syntax error\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
    } else if (strcmp(args[0], "ln") == 0) {
        if (arg_count != 3) {
            fprintf(stderr, "ln: syntax error\n");
        } else {
            if (link(args[1], args[2]) != 0) {
                perror("ln");
            }
        }
    } else if (strcmp(args[0], "rm") == 0) {
        if (arg_count != 2) {
            fprintf(stderr, "rm: syntax error\n");
        } else {
            if (unlink(args[1]) != 0) {
                perror("rm");
            }
        }
    } else if (strcmp(args[0], "exit") == 0) {
        cleanup_job_list(job_list);
        exit(0);
    } else if (strcmp(args[0], "jobs") == 0) {
        jobs(job_list);
    } else if (strcmp(args[0], "fg") == 0) {
        if (arg_count != 2) {
            fprintf(stderr, "fg: syntax error\n");
        }
        int fg_jid = atoi(args[1] + 1);
        pid_t fg_pid = get_job_pid(job_list, fg_jid);
        if (fg_pid == -1) {
            fprintf(stderr, "Job %d not found\n", fg_jid);
        }

        tcsetpgrp(STDIN_FILENO, fg_pid);
        if (kill(-fg_pid, SIGCONT) < 0) {
            perror("kill(SIGCONT)");
        }

        int status;
        pid_t cur_pid;
        if ((cur_pid = waitpid(fg_pid, &status, WUNTRACED)) > 0) {
            if (WIFSTOPPED(status)) {
                update_job_pid(job_list, fg_pid, STOPPED);
                fprintf(stdout, "[%d] (%d) suspended by signal %d\n", fg_jid,
                        fg_pid, WSTOPSIG(status));
            } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                if (WIFSIGNALED(status)) {
                    fprintf(stderr, "(%d) terminated by signal %d\n", fg_pid,
                            WTERMSIG(status));
                }
                remove_job_pid(job_list, fg_pid);
            }
        }

        tcsetpgrp(STDIN_FILENO, getpgrp());

    } else if (strcmp(args[0], "bg") == 0) {
        if (arg_count != 2) {
            fprintf(stderr, "bg: syntax error\n");
        }
        int bg_jid = atoi(args[1] + 1);
        pid_t bg_pid = get_job_pid(job_list, bg_jid);
        if (bg_pid == -1) {
            fprintf(stderr, "Job %d not found\n", bg_jid);
        }
        if (kill(-bg_pid, SIGCONT) < 0) {
            perror("kill(SIGCONT)");
        }
        update_job_pid(job_list, bg_pid, RUNNING);
    } else {
        pid_t pid = fork();

        if (pid == 0) {
            setpgid(0, 0);
            if (is_bg_job == 0) {
                tcsetpgrp(STDIN_FILENO, getpgrp());
            }

            // Handle I/O Redirection
            if (input_redir != 0 || output_redir != 0) {
                if (input_redir != 0) {
                    close(0);
                    if (open(input_file, O_RDONLY) == -1) {
                        perror(input_file);
                        cleanup_job_list(job_list);
                        exit(1);
                    }
                }

                if (output_redir != 0) {
                    close(1);
                    if (output_redir == 1 &&
                        open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0777) ==
                            -1) {
                        perror(output_file);
                        cleanup_job_list(job_list);
                        exit(1);
                    }
                    if (output_redir == 2 &&
                        open(output_file, O_WRONLY | O_CREAT | O_APPEND,
                             0777) == -1) {
                        perror(output_file);
                        cleanup_job_list(job_list);
                        exit(1);
                    }
                }
            }

            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);

            if (execv(args[0], args) == -1) {
                perror("execv");
            }

            cleanup_job_list(job_list);
            exit(1);
        }
        if (is_bg_job == 0) {  // forground process: reap child
            int status;
            pid_t cur_pid;
            if ((cur_pid = waitpid(pid, &status, WUNTRACED | WCONTINUED)) > 0) {
                if (WIFSIGNALED(status)) {
                    fprintf(stdout, "(%d) terminated by signal %d\n", cur_pid,
                            WTERMSIG(status));
                    remove_job_pid(job_list, cur_pid);
                }
                if (WIFSTOPPED(status)) {
                    add_job(job_list, jid, cur_pid, STOPPED, args[0]);
                    fprintf(stdout, "[%d] (%d) suspended by signal %d\n", jid,
                            cur_pid, WSTOPSIG(status));
                    jid++;
                }
            }
            tcsetpgrp(STDIN_FILENO, getpgrp());
        } else {  // background process: add to job list
            add_job(job_list, jid, pid, RUNNING, args[0]);
            fprintf(stdout, "[%d] (%d)\n", jid, pid);
            jid++;
            is_bg_job = 0;
        }
    }
    check_background_job();
}

int main() {
    char input[MAX_INPUT_SIZE];
    job_list = init_job_list();

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    while (1) {
#ifdef PROMPT
        printf("33sh> ");
#endif
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        execute_command(input);
    }
    cleanup_job_list(job_list);
    return 0;
}

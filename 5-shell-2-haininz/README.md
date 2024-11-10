```
 ____  _          _ _   ____
/ ___|| |__   ___| | | |___ \
\___ \| '_ \ / _ \ | |   __) |
 ___) | | | |  __/ | |  / __/
|____/|_| |_|\___|_|_| |_____|
```

To compile the program, simply do "make clean all".
Here is the overall structure of the program:
- An input buffer will be initialized for taking in inputs from stdin, and a job list will be initialized for keeping track of jobs
- SIG_IGN will be applied initially, and SIG_DFL will be applied in the child process
- There will be a while loop that keep taking user input from stdin
- The first thing to do is parsing the input, construct a valid command line, and identify anything special (i.e. if there is I/O redirection; what is the input and/or output file if any; if the process is running in the background)
- Then the program will start to handle different cases based on parsing result. I will skip over the explanation for Shell 1 cases (i.e. cd, ln, rm, etc.) here
- If the command is "jobs", simply call the given jobs function to print out the job list
- If the command is "fg", find the corresponding process, take it foreground, and make it continue. Then perform the same reaping process as described later below
- If the command is "bg", simply find the corresponding process, take it foreground, and make it continue
- In all other cases, we will fork a child process to handle I/O redirection and execute the command similar to Shell 1
- In the parent process, if the process is foreground, we will reap the single child process differently based on different status and give back control to the shell; otherwise, its a background process, we will add it to the job list
- No matter what, we will want to reap background processes

Here is the pseudo code which represents the program architecture:

int main() {
    while (stdin has input) {
        parse_command();
        /* ... 
           Handle Shell 1 cases, like cd, ln, rm, etc.
           ...
        */
        else if ("fg" command) {
            // continue the process in foreground
            // reap
        }   
        else if ("bg" command) {
            // continue the process in foreground
        }   
        else {
            /* In child process:
               1. Handle I/O redirection
               2. Execute commmand
            */

            /* In parent process:
               1. For foreground process, reap the child process
               2. For background process, add it to job list
            */
        }
        // Reap background processes
    }
    // Clean job list whenever needed
}

There is currently no bug as I could identify in the program. There might be some edge cases that I forget to handle. 
I didn't add any extra features.

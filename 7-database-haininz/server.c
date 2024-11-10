#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "./comm.h"
#include "./db.h"
#include "./server.h"

client_t *thread_list_head;
pthread_mutex_t thread_list_mutex = PTHREAD_MUTEX_INITIALIZER;
client_control_t client_control = {PTHREAD_MUTEX_INITIALIZER,
                                   PTHREAD_COND_INITIALIZER, 0};
server_control_t server_control = {PTHREAD_MUTEX_INITIALIZER,
                                   PTHREAD_COND_INITIALIZER, 0};
int accepting_clients = 1;

//------------------------------------------------------------------------------------------------
// Client threads' constructor and main method

// Called by listener (in comm.c) to create a new client thread
void client_constructor(FILE *cxstr) {
    /*
     * TODO:
     * Part 1A:
     *  You should create a new client_t struct (see server.h) here and
     * initialize ALL of its fields. Remember that these initializations should
     * be error-checked.
     *
     *  Step 1. Allocate memory for a new client and set its connection stream
     *          to the input argument.
     *  Step 2. Initialize the client's list-related fields to a reasonable
     * default. Step 3. Create the new client thread running the `run_client`
     * routine. Step 4. Detach the new client thread.
     */

    // Step 1. Allocate memory for a new client and set its connection stream to
    // the input argument
    client_t *client = (client_t *)malloc(sizeof(client_t));
    assert(client != NULL);

    // Step 2. Initialize the client's list-related fields to a reasonable
    // default
    client->cxstr = cxstr;
    client->next = NULL;
    client->prev = NULL;

    // Step 3. Create the new client thread running the `run_client` routine
    int thread_create_result;
    if ((thread_create_result = pthread_create(
             &client->thread, NULL, run_client, (void *)client)) != 0) {
        free(client);
        handle_error_en(thread_create_result, "pthread_create error");
    }

    // Step 4. Detach the new client thread
    int detach_result = pthread_detach(client->thread);
    if (detach_result != 0) {
        handle_error_en(detach_result, "detach error");
    }
}

void cleanup_unlock_mutex(void *mutex) {
    pthread_mutex_unlock((pthread_mutex_t *)mutex);
}

// Code executed by a client thread
void *run_client(void *arg) {
    /*
     * TODO:
     * Part 1A:
     *  Step 1. For the passed-in client, loop calling `comm_serve` (in comm.c),
     * to output the previous response and then read in the client's next
     * command, until the client disconnects. Execute commands using
     * `interpret_command` (in db.c). Step 2. When the client is done sending
     * commands, call `thread_cleanup`.
     *
     * Part 1B: Before looping, add the passed-in client to the client list. Be
     * sure to protect access to the client list using `thread_list_mutex`.
     *
     * Part 3A: Use `client_control_wait` to stop the client thread from
     * interpreting commands while the server is stopped.
     *
     * Part 3B: Support cancellation of the client thread by instead using
     * cleanup handlers to call `thread_cleanup`.
     *
     * Part 3C: Make sure that the server is still accepting clients before
     * adding a client to the client list (see step 2 in `main`). If not,
     * destroy the passed-in client and return.
     */
    if (accepting_clients == 0) {
        return NULL;
    }

    client_t *client = (client_t *)arg;
    assert(client != NULL);

    // Part 3C
    pthread_mutex_lock(&server_control.server_mutex);
    server_control.num_client_threads += 1;
    pthread_mutex_unlock(&server_control.server_mutex);

    // pthread_mutex_lock(&client_control.go_mutex);
    // if (client_control.stopped) {
    //     pthread_mutex_unlock(&client_control.go_mutex);
    //     client_destructor(client);
    //     return NULL;
    // }
    // pthread_mutex_unlock(&client_control.go_mutex);

    // Part 1B
    pthread_mutex_lock(&thread_list_mutex);
    client->next = thread_list_head;
    if (thread_list_head != NULL) {
        thread_list_head->prev = client;
    }
    thread_list_head = client;
    pthread_mutex_unlock(&thread_list_mutex);

    // Part 3B
    pthread_cleanup_push(thread_cleanup, client);

    char response[BUFLEN] = {0};
    char command[BUFLEN] = {0};

    // Part 1A
    // Step 1. Loop calling `comm_serve` (in comm.c), and execute commands using
    // `interpret_command` (in db.c)
    while (1) {
        if (comm_serve(client->cxstr, response, command) == -1) {
            break;
        }

        // Part 3A
        client_control_wait();

        interpret_command(command, response, BUFLEN);
    }

    // Part 3B
    pthread_cleanup_pop(1);

    // Step 2. When the client is done sending commands, call `thread_cleanup`
    // thread_cleanup(client);

    return NULL;
}

//------------------------------------------------------------------------------------------------
// Methods for client thread cleanup, destruction, and cancellation

void client_destructor(client_t *client) {
    /*
     * TODO:
     * Part 1A: Free and close all resources associated with a client.
     * (Take a look at `comm_shutdown` in comm.c)
     */

    if (client) {
        comm_shutdown(client->cxstr);
        free(client);
    }
}

// Cleanup routine for client threads, called on cancels and exit.
void thread_cleanup(void *arg) {
    /*
     * TODO:
     * Part 1A: Call `client_destructor` on the passed-in client.
     *
     * Part 1B: Remove the passed-in client from the client list before
     * destroying it. Note that the client must be in the list before this
     * routine is ever run. Be sure to protect access to the client list using
     * `thread_list_mutex`.
     */

    // Part 1A
    client_t *client = (client_t *)arg;
    if (client) {
        // Part 1B
        pthread_mutex_lock(&thread_list_mutex);
        if (client->next != NULL) {
            client->next->prev = client->prev;
        }
        if (client->prev != NULL) {
            client->prev->next = client->next;
        } else {
            thread_list_head = client->next;
        }
        pthread_mutex_unlock(&thread_list_mutex);

        // client_destructor(client);
        pthread_mutex_lock(&server_control.server_mutex);
        server_control.num_client_threads -= 1;
        client_destructor(client);
        pthread_cond_signal(&server_control.server_cond);
        pthread_mutex_unlock(&server_control.server_mutex);
    }
}

void delete_all() {
    /*
     * TODO:
     * Part 3C: Cancel every thread in the client thread list with using
     * `pthread_cancel`.
     */
    pthread_mutex_lock(&thread_list_mutex);
    client_t *current = thread_list_head;
    while (current != NULL) {
        pthread_cancel(current->thread);
        current = current->next;
    }
    pthread_mutex_unlock(&thread_list_mutex);
}

//------------------------------------------------------------------------------------------------
// Methods for stop/go server commands

// Called by client threads to wait until progress is permitted
void client_control_wait() {
    /*
     * TODO:
     * Part 3A: Block the calling thread until the main thread calls
     * `client_control_release`. See the `client_control_t` struct.
     *
     * Part 3B: Support thread-safe cancellation of a client thread by
     * using cleanup handlers. (Remember that `pthread_cond_wait` is a
     * cancellation point!)
     */

    // Part 3A
    pthread_mutex_lock(&client_control.go_mutex);

    // Part 3B
    pthread_cleanup_push(cleanup_unlock_mutex, &client_control.go_mutex);

    while (client_control.stopped) {
        pthread_cond_wait(&client_control.go, &client_control.go_mutex);
    }

    // Part 3B
    pthread_cleanup_pop(1);

    // pthread_mutex_unlock(&client_control.go_mutex);
}

// Called by main thread to stop client threads
void client_control_stop() {
    /*
     * TODO:
     * Part 3A: Ensure that the next time client threads call
     * `client_control_wait` in `run_client`, they will block. See the
     * `client_control_t` struct.
     */

    pthread_mutex_lock(&client_control.go_mutex);
    client_control.stopped = 1;
    pthread_mutex_unlock(&client_control.go_mutex);
}

// Called by main thread to resume client threads
void client_control_release() {
    /*
     * TODO:
     * Part 3A: Allow clients that are blocked within `client_control_wait`
     * to continue. See the `client_control_t` struct.
     */

    pthread_mutex_lock(&client_control.go_mutex);
    client_control.stopped = 0;
    pthread_cond_broadcast(&client_control.go);
    pthread_mutex_unlock(&client_control.go_mutex);
}

//------------------------------------------------------------------------------------------------
// SIGINT signal handling

// Code executed by the signal handler thread. 'man 7 signal' and 'man sigwait'
// are both helpful for implementing this function.
// All of the server's client threads should terminate on SIGINT; the server
// (this includes the listener thread), however, should not!
void *monitor_signal(void *arg) {
    /*
     * TODO:
     * Part 3D: Continually wait for a SIGINT to be sent to the server process
     * and cancel all client threads when one arrives. This thread will be
     * canceled by `sig_handler_destructor` - note that `sigwait` is a
     * cancellation point.
     */

    sig_handler_t *handler = (sig_handler_t *)arg;
    int sig;

    while (1) {
        int wait_code;
        if ((wait_code = sigwait(&handler->set, &sig)) != 0) {
            handle_error_en(wait_code, "sigwait error");
        }
        if (sig == SIGINT) {
            fprintf(stderr, "SIGINT received, cancelling all clients\n");
            delete_all();
        }
    }

    return NULL;
}

sig_handler_t *sig_handler_constructor() {
    /*
     * TODO:
     * Part 3D: Create a thread to handle SIGINT. Make sure that the thread that
     * this function creates is the ONLY thread that ever responds to SIGINT
     * (use `pthread_sigmask`!). Be sure to take a look at sig_hander_t in
     * server.h.
     */

    sig_handler_t *handler = malloc(sizeof(sig_handler_t));
    assert(handler != NULL);

    sigemptyset(&handler->set);
    sigaddset(&handler->set, SIGINT);
    int mask_code;
    if ((mask_code = pthread_sigmask(SIG_BLOCK, &handler->set, NULL)) != 0) {
        // free(handler);
        handle_error_en(mask_code, "pthread_sigmask");
    }

    int new_thread;
    if ((new_thread = pthread_create(&handler->thread, NULL, monitor_signal,
                                     (void *)handler)) != 0) {
        // free(handler);
        handle_error_en(new_thread, "pthread_create");
    }

    return handler;
}

void sig_handler_destructor(sig_handler_t *sighandler) {
    /*
     * TODO:
     * Part 3D: Free any resources allocated in sig_handler_constructor, and
     * cancel and join with the signal handler's thread.
     */
    pthread_cancel(sighandler->thread);
    int join_code = pthread_join(sighandler->thread, NULL);
    if (join_code != 0) {
        handle_error_en(join_code, "pthread_join error");
    }
    free(sighandler);
}

//------------------------------------------------------------------------------------------------
// Main function

// The arguments to the server should be the port number.
int main(int argc, char *argv[]) {
    /*
     * TODO:
     * Part 1A:
     *  Step 1. Block SIGPIPE using `pthread_sigmask` so that the server does
     * not abort when a client disconnects. Step 2. Start a listener thread for
     * clients (see `start_listener` in comm.c). Step 3. Join with the listener
     * thread.
     *
     * Part 3A: Before joining the listener thread, loop for command line input
     * and handle any print, stop, and go command requests.
     *
     * Part 3C:
     *  Step 1. Modify the command line loop to break on receiving EOF.
     *  Step 2. After receiving EOF, use a thread-safe mechanism to indicate
     * that the server is no longer accepting clients, and then cancel all
     * client threads using `delete_all`. Think carefully about what happens at
     * the start of `run_client` and ensure that your mechanism does not allow
     * any way for a thread to add itself to the thread list after your
     * mechanism is activated. Step 3. After calling `delete_all`, make sure
     * that the thread list is empty using the `server_control_t` struct. (Note
     * that you will need to modify other functions for the struct to accurately
     * keep track of the number of threads in the list - where does it make
     * sense to modify the `num_client_threads` field?) Step 4. Once the thread
     * list is empty, cleanup the database, and then cancel and join with the
     * listener thread.
     *
     * Part 3D:
     *  Step 1. After blocking SIGPIPE, create a SIGINT signal handler using
     *          `sig_handler_constructor`.
     *  Step 2. Destroy the signal handler using `sig_handler_destructor` right
     * after the server receives EOF.
     */
    (void)argc;

    // Part 1A
    // Step 1. Block SIGPIPE
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    // Part 3D
    sig_handler_t *sig_handler = sig_handler_constructor();

    // Step 2. Start listener thread
    pthread_t listener_thread;
    int port = atoi(argv[1]);
    listener_thread = start_listener(port, client_constructor);

    // Part 3A
    while (1) {
        char cmd[BUFLEN];
        if (fgets(cmd, BUFLEN, stdin) == NULL) {
            break;
        }
        char *token = strtok(cmd, " \t\n");
        if (strcmp(token, "p") == 0) {
            char *fileName = strtok(NULL, " \t\n");
            db_print(fileName);
        } else if (strcmp(token, "g") == 0) {
            client_control_release();
        } else if (strcmp(token, "s") == 0) {
            client_control_stop();
        } else {
            continue;
        }
    }

    // Part 3D
    sig_handler_destructor(sig_handler);

    // Part 3C
    // client_control_stop();
    accepting_clients = 0;
    delete_all();
    pthread_mutex_lock(&server_control.server_mutex);
    pthread_cleanup_push(cleanup_unlock_mutex, &server_control.server_mutex);
    while (server_control.num_client_threads > 0) {
        pthread_cond_wait(&server_control.server_cond,
                          &server_control.server_mutex);
    }
    pthread_cleanup_pop(1);

    db_cleanup();
    pthread_cancel(listener_thread);

    // Step 3. Join with the listener thread
    pthread_join(listener_thread, NULL);

    return 0;
}

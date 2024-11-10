```
 ____        _        _
|  _ \  __ _| |_ __ _| |__   __ _ ___  ___
| | | |/ _` | __/ _` | '_ \ / _` / __|/ _ \
| |_| | (_| | || (_| | |_) | (_| \__ \  __/
|____/ \__,_|\__\__,_|_.__/ \__,_|___/\___|
```
For db.c, the main thing to do is locking. Specifically, implement fine-grained locking (we do this by modify the structure of node in db.c to keep a read-write lock for each node) on existing methods, including search, query, add, delete, and print. To conform with the style with the lab, we lock every parent node (if there is one) before passing it into search(). And upon return of search(), parentpp and the return node (if there is one) will be locked. Then in every function that calls search(), we will make sure to unlock both nodes after all the operations.
Additionally, we modify the function signature of search() to add a new int parameter to denote whether we need a read lock or a write lock.

For server.c, when the server starts, it sets up signal handling and starts listening for client connections. For each new client, a new thread is created using client_constructor. Client threads execute run_client, processing client requests and responding appropriately. The server can be controlled via command-line inputs to pause or resume client operations. There are 2 specially signals to be handled. One is EOF, where the database server will be stopped and deleted upon receiving it. The other is SIGINT, where the database server will not be terminated but instead terminating all the clients.
Additionally, we create a helper function "cleanup_unlock_mutex" that wraps "pthread_mutex_unlock" to unlock safely.

The program does not currently have any unresolved bugs.

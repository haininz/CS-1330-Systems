#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "solver.h"

/*
 * Given a pointer to a maze_room, set its connections in all four directions
 * based on the hex value passed in.
 *
 * For example:
 *      create_room_connections(&maze[3][4], 0xb)
 *
 * 0xb is 1011 in binary, which means we have walls on all sides except the
 * WEST. This means that every connection in maze[3][4] should be set to 1,
 * except the WEST connection, which should be set to 0.
 *
 * See the handout for more details about our hexadecimal representation, as
 * well as examples on how to use bit masks to extract information from this
 * hexadecimal representation.
 *
 * Parameters:
 *  - room: pointer to the current room
 *  - hex: hexadecimal integer (between 0-15 inclusive) that represents the
 *         connections in all four directions from the given room.
 * Returns:
 *  - nothing. The connections should be saved in the maze_room struct
 *    pointed to by the parameter (make sure to use pointers correctly!).
 */
void create_room_connections(struct maze_room *room, unsigned int hex)
{
    // TODO: implement this function
    room->up_room = hex & 1 ? 1 : 0;
    room->down_room = hex & 2 ? 1 : 0;
    room->left_room = hex & 4 ? 1 : 0;
    room->right_room = hex & 8 ? 1 : 0;
}

/*
 * Decodes an encoded maze and stores the resulting data about the maze's rooms'
 * connections in the passed 'maze' variable.
 *
 * Parameters:
 *  - num_rows: number of rows in the maze
 *  - num_cols: number of columns in the maze
 *  - maze: a 2d array of maze_room structs (where to store the decoded
 *          maze)
 *  - encoded_maze: a 2d array of numbers representing a maze
 * Returns:
 *  - nothing; the decoded data about the rooms' connections is stored in the
 *    `maze` variable
 */
void decode_maze(int num_rows, int num_cols,
                 struct maze_room maze[num_rows][num_cols],
                 int encoded_maze[num_rows][num_cols])
{
    // TODO: implement this function
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            create_room_connections(&maze[i][j], encoded_maze[i][j]);
        }
    }
}

/*
 * Recursive depth-first search algorithm for solving your maze.
 * This function should also print out either every visited room as it goes, or
 * the final pruned solution, depending on whether the FULL macro is set.
 *
 * See handout for more details, as well as a pseudocode implementation.
 *
 * Parameters:
 *  - row: row of the current room
 *  - col: column of the current room
 *  - goal_row: row of the goal room
 *  - goal_col: col of the goal room
 *  - num_rows: number of rows in the maze
 *  - num_cols: number of columns in the maze
 *  - maze: a 2d array of maze_room structs representing your maze
 *  - file: the file to write the solution to
 * Returns:
 *  - 1 if the current branch finds a valid solution, 0 if no branches are
 *    valid.
 */
int dfs(int row, int col, int goal_row, int goal_col, int num_rows,
        int num_cols, struct maze_room maze[num_rows][num_cols], FILE *file)
{
    Direction directions[4] = {NORTH, SOUTH, EAST, WEST};
// TODO: implement this function
#ifdef FULL
    if (fprintf(file, "%d, %d\n", row, col) < 0)
    {
        fprintf(stderr, "Error print FULL path.\n");
        exit(1);
    }
#endif
    if (row == goal_row && col == goal_col)
    {
        return 1;
    }
    maze[row][col].visited = 1;
    for (int i = 0; i < 4; i++)
    {
        Direction dir = directions[i];
        struct maze_room *neighbor = get_neighbor(num_rows, num_cols, maze, &maze[row][col], dir);
        if (neighbor != NULL &&
            neighbor->visited == 0 &&
            ((dir == NORTH && maze[row][col].up_room == 0) ||
             (dir == SOUTH && maze[row][col].down_room == 0) ||
             (dir == WEST && maze[row][col].left_room == 0) ||
             (dir == EAST && maze[row][col].right_room == 0)))
        {
            maze[row][col].next = neighbor;
            if (dfs(neighbor->row, neighbor->col, goal_row, goal_col, num_rows, num_cols, maze, file) == 1)
            {
                return 1;
            }
#ifdef FULL
            if (fprintf(file, "%d, %d\n", row, col) < 0)
            {
                fprintf(stderr, "Error print FULL path.\n");
                exit(1);
            }
#endif
        }
    }
    return 0;
}

/*
 * Recursively prints the pruned solution path (using the current maze room
 * and its next pointer).
 *
 * Parameters:
 *  - room: a pointer to the current maze room
 *  - file: the file where to print the path
 * Returns:
 *  - 1 if an error occurs, 0 otherwise
 */
int print_pruned_path(struct maze_room *room, FILE *file)
{
    // TODO: implement this function
    while (room != NULL)
    {
        if (fprintf(file, "%d, %d\n", room->row, room->col) < 0)
        {
            fprintf(stderr, "Error print PRUNED path.\n");
            return 1;
        }
        room = room->next;
    }
    return 0;
}

/*
 * Reads encoded maze from the file passed in.
 *
 * Parameters:
 *  - num_rows: number of rows in the maze
 *  - num_cols: number of columns in the maze
 *  - encoded_maze: a maze_room array as a hexadecimal array based on its
 *                  connections
 *  - file_name: input file to read the encoded maze from
 * Returns:
 *  - 1 if an error occurs, 0 otherwise
 */
int read_encoded_maze_from_file(int num_rows, int num_cols,
                                int encoded_maze[num_rows][num_cols],
                                char *file_name)
{
    int err = 0;

    // open file
    FILE *f = fopen(file_name, "r");
    if (f == NULL)
    {
        fprintf(stderr, "Error opening file.\n");
        return 1;
    }
    // read each hex value into 2D array
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            unsigned int encoded_room;
            err = fscanf(f, "%1x", &encoded_room);
            encoded_maze[i][j] = encoded_room;
            if (err < 0)
            {
                fprintf(stderr, "Reading from file failed");
                return 1;
            }
        }
    }
    // close file
    int close = fclose(f);
    if (close == EOF)
    {
        fprintf(stderr, "Could not close file.\n");
        return 1;
    }
    return 0;
}

/*
 * Main function
 *
 * Parameters:
 *  - argc: the number of command line arguments - for this function 9
 *  - argv: a pointer to the first element in the command line
 *            arguments array - for this function:
 *            ["solver", <input maze file>, <number of rows>, <number of columns>,
 *             <output path file>, <starting row>, <starting column>, <ending row>,
 *             <ending column>]
 * Returns:
 *  - 0 if program exits correctly, 1 if there is an error
 */

int main(int argc, char **argv)
{
    int num_rows, num_cols, start_row, start_col, goal_row, goal_col;
    char *maze_file_name;
    char *path_file_name;
    if (argc != 9)
    {
        printf("Incorrect number of arguments.\n");
        printf(
            "./solver <input maze file> <number of rows> <number of columns>");
        printf(" <output path file> <starting row> <starting column>");
        printf(" <ending row> <ending column>\n");
        return 1;
    }
    else
    {
        maze_file_name = argv[1];
        num_rows = atoi(argv[2]);
        num_cols = atoi(argv[3]);
        path_file_name = argv[4];
        start_row = atoi(argv[5]);
        start_col = atoi(argv[6]);
        goal_row = atoi(argv[7]);
        goal_col = atoi(argv[8]);
    }
    // TODO: implement this function
    if (start_row < 0 || start_row >= num_rows || start_col < 0 || start_col >= num_cols ||
        goal_row < 0 || goal_row >= num_rows || goal_col < 0 || goal_col >= num_cols)
    {
        printf("Bad input row/col\n");
        exit(1);
    }

    int encoded_maze[num_rows][num_cols];
    if (read_encoded_maze_from_file(num_rows, num_cols, encoded_maze, maze_file_name) == 1)
    {
        printf("Error read encoded maze from file.\n");
        exit(1);
    }
    else
    {
        struct maze_room maze[num_rows][num_cols];
        initialize_maze(num_rows, num_cols, maze);
        decode_maze(num_rows, num_cols, maze, encoded_maze);
        FILE *file = fopen(path_file_name, "w");
        if (file == NULL)
        {
            fprintf(stderr, "Error opening file.\n");
            exit(1);
        }
#ifdef FULL
        if (fprintf(file, "FULL\n") < 0)
        {
            fprintf(stderr, "Error print FULL path.\n");
            exit(1);
        }
        dfs(start_row, start_col, goal_row, goal_col, num_rows, num_cols, maze, file);
#else
        if (fprintf(file, "PRUNED\n") < 0)
        {
            exit(1);
        }
        dfs(start_row, start_col, goal_row, goal_col, num_rows, num_cols, maze, file);
        struct maze_room *cur_room = &maze[start_row][start_col];
        if (print_pruned_path(cur_room, file) == 1)
        {
            fprintf(stderr, "Error print PRUNED path.\n");
            exit(1);
        }
#endif

        if (fclose(file) == EOF)
        {
            fprintf(stderr, "Error close file.\n");
            exit(1);
        }
    }
}

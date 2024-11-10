#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "generator.h"

/*
 * Given a direction, get its opposite.
 *
 * Parameters:
 *  - dir: direction
 * Returns:
 *  - the opposite direction to dir
 */
Direction get_opposite_dir(Direction dir)
{
    // TODO: implement this function
    if (dir == NORTH)
    {
        return SOUTH;
    }
    else if (dir == SOUTH)
    {
        return NORTH;
    }
    else if (dir == WEST)
    {
        return EAST;
    }
    else
    {
        return WEST;
    }
}

/*
 * Given an array of the four directions, randomizes the order of the directions.
 *
 * Parameters:
 *  - directions: an array of Direction enums of size 4
 * Returns:
 *  - nothing - the array should be shuffled in place
 */
void shuffle_array(Direction directions[])
{
    // TODO: implement this function
    for (int i = 0; i < 4; i++)
    {
        int r = rand() % 4;
        Direction temp = directions[i];
        directions[i] = directions[r];
        directions[r] = temp;
    }
}

/*
 * Recursive function for stepping through a maze and setting its
 * connections/walls using the drunken_walk algorithm.
 *
 * See handout for more details, as well as a pseudocode implementation.
 *
 * Parameters:
 *  - row: row of the current room
 *  - col: column of the current room
 *  - num_rows: number of rows in the maze
 *  - num_cols: number of columns in the maze
 *  - maze: a 2D array of maze_room structs representing your maze
 * Returns:
 *  - nothing - the state of your maze should be saved in the maze array
 *    being passed in (make sure to use pointers correctly!).
 */
void drunken_walk(int row, int col, int num_rows, int num_cols,
                  struct maze_room maze[num_rows][num_cols])
{
    Direction directions[4] = {NORTH, SOUTH, EAST, WEST};
    // TODO: implement this function
    if (is_in_range(row, col, num_rows, num_cols) == 0)
    {
        return;
    }
    shuffle_array(directions);
    // maze[row][col].row = row;
    // maze[row][col].col = col;
    maze[row][col].visited = 1;
    // printf("row: %d, col: %d\n", row, col);
    for (int i = 0; i < 4; i++)
    {
        // Current direction
        Direction dir = directions[i];
        // printf("Going from (%d, %d) to %d\n", row, col, dir);
        // Neighbor (pointer) at current direction
        struct maze_room *neighbor = get_neighbor(num_rows, num_cols, maze, &maze[row][col], dir);
        // If neighbor out of bounds, store a wall in r at direction dir
        if (neighbor == NULL)
        {
            // printf("Out of bounds.\n");
            if (dir == NORTH)
            {
                maze[row][col].up_room = 1;
            }
            else if (dir == SOUTH)
            {
                maze[row][col].down_room = 1;
            }
            else if (dir == WEST)
            {
                maze[row][col].left_room = 1;
            }
            else
            {
                maze[row][col].right_room = 1;
            }
        }
        else
        {
            // Check if neighbor has been visited
            if (neighbor->visited == 0)
            {
                // Store an opening in r at direction dir
                if (dir == NORTH)
                {
                    maze[row][col].up_room = 0;
                }
                else if (dir == SOUTH)
                {
                    maze[row][col].down_room = 0;
                }
                else if (dir == WEST)
                {
                    maze[row][col].left_room = 0;
                }
                else
                {
                    maze[row][col].right_room = 0;
                }
                // Recursion
                drunken_walk(neighbor->row, neighbor->col, num_rows, num_cols, maze);
            }
            else
            {
                // printf("Neighbor visited.\n");
                if (dir == NORTH)
                {
                    // If n has a wall or opening in the direction opposite of dir
                    if (neighbor->down_room != -1)
                    {
                        // Store that value in r at direction dir
                        maze[row][col].up_room = neighbor->down_room;
                    }
                    // Store a wall in r at direction dir
                    else
                    {
                        maze[row][col].up_room = 1;
                    }
                }
                else if (dir == SOUTH)
                {
                    if (neighbor->up_room != -1)
                    {
                        maze[row][col].down_room = neighbor->up_room;
                    }
                    else
                    {
                        maze[row][col].down_room = 1;
                    }
                }
                else if (dir == WEST)
                {
                    if (neighbor->right_room != -1)
                    {
                        maze[row][col].left_room = neighbor->right_room;
                    }
                    else
                    {
                        maze[row][col].left_room = 1;
                    }
                }
                else
                {
                    if (neighbor->left_room != -1)
                    {
                        maze[row][col].right_room = neighbor->left_room;
                    }
                    else
                    {
                        maze[row][col].right_room = 1;
                    }
                }
            }
        }
    }
}

/*
 * Represents a maze_room struct as an integer based on its connections.
 *
 * Parameters:
 *  - room: a struct maze_room to be converted to an integer
 * Returns:
 *  - the integer representation of a room
 */
int encode_room(struct maze_room room)
{
    // TODO: implement this function
    int sum = 0;
    if (room.up_room == 1)
    {
        sum += 1;
    }
    if (room.down_room == 1)
    {
        sum += 2;
    }
    if (room.left_room == 1)
    {
        sum += 4;
    }
    if (room.right_room == 1)
    {
        sum += 8;
    }
    return sum;
}

/*
 * Represents a maze_room array as a hexadecimal array based on its connections.
 *
 * Parameters:
 *  - num_rows: number of rows in the maze
 *  - num_cols: number of columns in the maze
 *  - maze: a 2D array of maze_room structs representing the maze
 *  - result: a 2D array of integers representing the maze
 * Returns:
 *  - nothing - each maze_room in the maze should be represented
 *    as an integer [0, 15] and put in nums at the corresponding index
 */
void encode_maze(int num_rows, int num_cols,
                 struct maze_room maze[num_rows][num_cols],
                 int result[num_rows][num_cols])
{
    // TODO: implement this function
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            result[i][j] = encode_room(maze[i][j]);
        }
    }
}

/*
 * Writes encoded maze to an output file
 *
 * Parameters:
 *  - num_rows: number of rows in the maze
 *  - num_cols: number of columns in the maze
 *  - encoded_maze: a maze_room array as a hexadecimal array based on its
 *                  connections
 *  - file_name: the name of the output file for the encoded maze
 * Returns:
 *  - 1 if an error occurs, 0 otherwise
 */
int write_encoded_maze_to_file(int num_rows, int num_cols,
                               int encoded_maze[num_rows][num_cols],
                               char *file_name)
{
    int err = 0;

    // open file (create it if necessary)
    FILE *f = fopen(file_name, "w+");
    if (f == NULL)
    {
        fprintf(stderr, "Error opening file.\n");
        return 1;
    }
    // write each room's hex value into file
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            int encoded_room = encoded_maze[i][j];
            err = fprintf(f, "%x", encoded_room);
            if (err < 0)
            {
                fprintf(stderr, "Writing to file failed: %i\n", err);
                return 1;
            }
        }
        // add a newline between rows
        err = fprintf(f, "\n");
        if (err < 0)
        {
            fprintf(stderr, "Writing to file failed: %i\n", err);
            return 1;
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
 *  - argc: the number of command line arguments - for this function 4
 *  - argv: a pointer to the first element in the command line
 *          arguments array - for this function:
 *          ["generator", <output file>, <number of rows>, <number of columns>]
 * Returns:
 *  - 0 if program exits correctly, 1 if there is an error
 */

int main(int argc, char **argv)
{
    char *file_name;
    int num_rows;
    int num_cols;

    srand(time(NULL));

    if (argc != 4)
    {
        printf("Incorrect number of arguments.\n");
        printf(
            "./generator <output file> <number of rows> <number of columns>\n");
        return 1;
    }
    else
    {
        file_name = argv[1];
        num_rows = atoi(argv[2]);
        num_cols = atoi(argv[3]);
    }
    // TODO: implement this function
    if (num_rows > 0 && num_cols > 0)
    {
        struct maze_room maze[num_rows][num_cols];
        initialize_maze(num_rows, num_cols, maze);
        drunken_walk(0, 0, num_rows, num_cols, maze);
        int encoded_maze[num_rows][num_cols];
        encode_maze(num_rows, num_cols, maze, encoded_maze);
        if (write_encoded_maze_to_file(num_rows, num_cols, encoded_maze, file_name) == -1)
        {
            printf("Error write encoded maze to file.\n");
        }
    }
}

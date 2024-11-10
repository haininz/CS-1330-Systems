#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

/*
 * Determines whether or not the room at [row][col] is a valid room within the
 * maze with dimensions num_rows x num_cols.
 *
 * Parameters:
 *  - row: row of the current room
 *  - col: column of the current room
 *  - num_rows: number of rows in the maze
 *  - num_cols: number of columns in the maze
 *
 * Returns:
 *  - 0 if room is not in the maze, 1 if room is in the maze
 */
int is_in_range(int row, int col, int num_rows, int num_cols)
{
    // TODO: implement function
    if (row < 0 || row >= num_rows || col < 0 || col >= num_cols)
    {
        return 0;
    }
    return 1;
}

/*
 * Given a pointer to the room and a Direction to travel in, return a pointer to
 * the room that neighbors the current room on the given Direction. For example:
 * get_neighbor(&maze[3][4], EAST) should return &maze[3][5]. If there is no room in
 * that direction (i.e. the room is at the border of the maze for the given
 * direction) return NULL.
 *
 * Parameters:
 *  - num_rows: number of rows in the maze
 *  - num_cols: number of columns in the maze
 *  - room: pointer to the current room
 *  - dir: Direction to get the neighbor from
 *  - maze: a 2D array of maze_room structs representing your maze
 * Returns:
 *  - pointer to the neighboring room, or NULL if said room is not in the maze.
 */
struct maze_room *get_neighbor(int num_rows, int num_cols,
                               struct maze_room maze[num_rows][num_cols],
                               struct maze_room *room, Direction dir)
{
    // TODO: implement function
    int cur_row = room->row, cur_col = room->col;
    if (is_in_range(cur_row, cur_col, num_rows, num_cols) == 0)
    {
        return NULL;
    }
    if (dir == NORTH && cur_row - 1 >= 0)
    {
        return &maze[cur_row - 1][cur_col];
    }
    else if (dir == SOUTH && cur_row + 1 < num_rows)
    {
        return &maze[cur_row + 1][cur_col];
    }
    else if (dir == WEST && cur_col - 1 >= 0)
    {
        return &maze[cur_row][cur_col - 1];
    }
    else if (dir == EAST && cur_col + 1 < num_cols)
    {
        return &maze[cur_row][cur_col + 1];
    }
    return NULL;
}

/*
 * Initializes a 2D array of maze rooms with all of the necessary values.
 *
 * Parameters:
 *  - num_rows: the number of the rows in the maze
 *  - num_cols: the number of columns in the maze
 *  - maze: a 2D array of uninitialized maze_rooms (to be initialized in
 *     this function)
 *
 * Returns:
 *  - nothing (the initialized maze will be stored in the 'maze' array)
 */
void initialize_maze(int num_rows, int num_cols,
                     struct maze_room maze[num_rows][num_cols])
{
    // TODO: implement function
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            maze[i][j].row = i;
            maze[i][j].col = j;
            maze[i][j].visited = 0;
            maze[i][j].up_room = -1;
            maze[i][j].down_room = -1;
            maze[i][j].left_room = -1;
            maze[i][j].right_room = -1;
            // TODO: initialize "next"
            maze[i][j].next = NULL;
        }
    }
}

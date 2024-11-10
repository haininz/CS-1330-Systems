#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * BEFORE GETTING STARTED:
 *
 * Familiarize yourself with the functions and constants/variables
 * in the following included files.
 * This will make the project a LOT easier as you go!!
 *
 * The diagram in Section 4.1 (Specification) of the handout will help you
 * understand the constants in mm.h
 * Section 4.2 (Support Routines) of the handout has information about
 * the functions in mminline.h and memlib.h
 */
#include "./memlib.h"
#include "./mm.h"
#include "./mminline.h"

block_t *prologue;
block_t *epilogue;

// rounds up to the nearest multiple of WORD_SIZE
static inline long align(long size) {
    return (((size) + (WORD_SIZE - 1)) & ~(WORD_SIZE - 1));
}

/*
 *                             _       _ _
 *     _ __ ___  _ __ ___     (_)_ __ (_) |_
 *    | '_ ` _ \| '_ ` _ \    | | '_ \| | __|
 *    | | | | | | | | | | |   | | | | | | |_
 *    |_| |_| |_|_| |_| |_|___|_|_| |_|_|\__|
 *                       |_____|
 *
 * initializes the dynamic storage allocator (allocate initial heap space)
 * arguments: none
 * returns: 0, if successful
 *         -1, if an error occurs
 */
int mm_init(void) {
    mem_init();

    long initial_heap_size = 2 * TAGS_SIZE;
    if ((prologue = (block_t *)mem_sbrk(initial_heap_size)) == (void *)-1) {
        return -1;
    }

    // Initialize prologue block
    block_set_size_and_allocated(prologue, TAGS_SIZE, 1);

    // Initialize epilogue block
    epilogue = (block_t *)((char *)prologue + TAGS_SIZE);
    block_set_size_and_allocated(epilogue, TAGS_SIZE, 1);

    // Initialize free list
    flist_first = NULL;

    return 0;
}

/*     _ __ ___  _ __ ___      _ __ ___   __ _| | | ___   ___
 *    | '_ ` _ \| '_ ` _ \    | '_ ` _ \ / _` | | |/ _ \ / __|
 *    | | | | | | | | | | |   | | | | | | (_| | | | (_) | (__
 *    |_| |_| |_|_| |_| |_|___|_| |_| |_|\__,_|_|_|\___/ \___|
 *                       |_____|
 *
 * allocates a block of memory and returns a pointer to that block's payload
 * arguments: size: the desired payload size for the block
 * returns: a pointer to the newly-allocated block's payload (whose size
 *          is a multiple of ALIGNMENT), or NULL if an error occurred
 */
void *mm_malloc(long size) {
    // (void) size;  // avoid unused variable warnings
    // TODO

    if (size == 0) {
        return NULL;
    }

    // Align the requested size
    long adjusted_size = (align(size + TAGS_SIZE) < MINBLOCKSIZE)
                             ? MINBLOCKSIZE
                             : align(size + TAGS_SIZE);  // size + headers

    // Traverse free list for a suitable block
    block_t *cur = flist_first;

    if (cur != NULL) {
        do {
            if (!block_allocated(cur) && block_size(cur) >= adjusted_size) {
                long remaining_size = block_size(cur) - adjusted_size;
                if (remaining_size >= MINBLOCKSIZE) {
                    // Original split: allocated on top, free below
                    // block_set_size(cur, adjusted_size);
                    // block_t *new_block = block_next(cur);
                    // block_set_size_and_allocated(new_block, remaining_size,
                    // 0); insert_free_block(new_block);

                    pull_free_block(cur);
                    block_set_size_and_allocated(cur, remaining_size, 0);
                    insert_free_block(cur);

                    block_t *new_block = block_next(cur);
                    block_set_size_and_allocated(new_block, adjusted_size, 1);
                    return (void *)(new_block->payload);
                }
                pull_free_block(cur);
                block_set_allocated(cur, 1);
                return (void *)(cur->payload);
            }
            cur = block_flink(cur);
        } while (cur != flist_first);
    }

    cur = (block_t *)mem_sbrk(adjusted_size);
    if ((long)cur == -1) {
        return NULL;  // mem_sbrk failed
    }

    cur = epilogue;
    block_set_size_and_allocated(cur, adjusted_size, 1);
    epilogue = block_next(cur);
    block_set_size_and_allocated(epilogue, TAGS_SIZE, 1);

    return (void *)(cur->payload);
}

void coalesce(block_t *block) {
    block_t *prev_block = block_prev(block);
    block_t *next_block = block_next(block);
    int prev_block_free = 0;
    if (!block_allocated(prev_block)) {
        pull_free_block(block);
        prev_block_free = 1;
        block_set_size(prev_block, block_size(prev_block) + block_size(block));
    }
    if (!block_allocated(next_block)) {
        pull_free_block(next_block);
        if (prev_block_free) {
            block_set_size(prev_block,
                           block_size(prev_block) + block_size(next_block));
        } else {
            block_set_size(block, block_size(block) + block_size(next_block));
        }
    }
}

/*                              __
 *     _ __ ___  _ __ ___      / _|_ __ ___  ___
 *    | '_ ` _ \| '_ ` _ \    | |_| '__/ _ \/ _ \
 *    | | | | | | | | | | |   |  _| | |  __/  __/
 *    |_| |_| |_|_| |_| |_|___|_| |_|  \___|\___|
 *                       |_____|
 *
 * frees a block of memory, enabling it to be reused later
 * arguments: ptr: pointer to the block's payload
 * returns: nothing
 */
void mm_free(void *ptr) {
    // (void) ptr;  // avoid unused variable warnings
    // TODO
    if (ptr == NULL) {
        return;
    }

    block_t *block = payload_to_block(ptr);
    block_set_allocated(block, 0);
    insert_free_block(block);  // FIXME: insert only when necessary
    coalesce(block);
}

/*
 *                                            _ _
 *     _ __ ___  _ __ ___      _ __ ___  __ _| | | ___   ___
 *    | '_ ` _ \| '_ ` _ \    | '__/ _ \/ _` | | |/ _ \ / __|
 *    | | | | | | | | | | |   | | |  __/ (_| | | | (_) | (__
 *    |_| |_| |_|_| |_| |_|___|_|  \___|\__,_|_|_|\___/ \___|
 *                       |_____|
 *
 * reallocates a memory block to update it with a new given size
 * arguments: ptr: a pointer to the memory block's payload
 *            size: the desired new payload size
 * returns: a pointer to the new memory block's payload
 */
void *mm_realloc(void *ptr, long size) {
    (void)ptr, (void)size;  // avoid unused variable warnings

    // Naive implementation
    // void *new_ptr = mm_malloc(size);
    // memcpy(new_ptr, ptr, align(size));
    // mm_free(ptr);

    // TODO
    if (ptr == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    block_t *current_block = payload_to_block(ptr);
    long current_size = block_size(current_block);

    long new_size = align(size + TAGS_SIZE) < MINBLOCKSIZE
                        ? MINBLOCKSIZE
                        : align(size + TAGS_SIZE);

    // If the new size is less than or equal to the current size, return the
    // same pointer (TODO: split)
    if (new_size <= current_size) {
        return ptr;
    }

    // Try to expand into next block
    block_t *next_block = block_next(current_block);
    if (!block_allocated(next_block) &&
        (current_size + block_size(next_block)) >= new_size) {
        pull_free_block(next_block);
        block_set_size_and_allocated(current_block,
                                     current_size + block_size(next_block), 1);
        return ptr;
    }

    // Unable to expand into next block: malloc a new block and copy the old
    // data
    void *new_ptr = mm_malloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }

    memcpy(new_ptr, ptr, align(size));
    mm_free(ptr);

    return new_ptr;
}

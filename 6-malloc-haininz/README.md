```
 __  __       _ _
|  \/  | __ _| | | ___   ___
| |\/| |/ _` | | |/ _ \ / __|
| |  | | (_| | | | (_) | (__
|_|  |_|\__,_|_|_|\___/ \___|
```
## Strategies for maintaining compaction:
* Always try to coalesce with neighbor blocks, in both mm_free() and mm_realloc(). Specifically, if we want to need a larger block than the current block, then we should first try to coalesce with neighbor blocks to form a new block that will be large enough for the requirement. We will only malloc new blocks if the size of the block is still not large enough after coalesce.
* Always try to split the remaining space into a new block when we use only part of a block. In this way, we can avoid those space being wasted.
## mm_realloc() implementation strategy (throughput & utilization)
* Pretty much the same as stated above, I tried to coalesce with the next block when the current block is not large enough. This single optimization was able to improve the utilization of realloc-bal.rep and realloc2-bal.rep to 49% and 75% separately. 
* Considering about throughput, I choose not to split after every coalesce (I tested and noticed that the performance actually decreases when we split for every coalesced block)
## Other
* There are currently no bugs in the program as far as I can notice
* Certainly, further optimizations could be implemented, like a clever way to perform split, and/or a new mem_sbrk strategy to further improve throughput.

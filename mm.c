#include <assert.h>
#include <stdint.h>

#include "mm.h"

/* -------------------------------------------------------------------------- */
/*                              STATIC VARIABLES                              */
/* -------------------------------------------------------------------------- */
static uint8_t ghost_node[sizeof(uint32_t) + sizeof(hptr_t)];

// Have ghost node point to the root of out RB Tree
/* -------------------------------------------------------------------------- */
/*                           BLOCK MEMBER VARIABLES                           */
/* -------------------------------------------------------------------------- */
uint32_t bk_size(hptr_t block) {
    return BLOCK_PTR(block)->size;
}

hptr_t bk_left(hptr_t block) {
    return BLOCK_PTR(block)->left;
}

void bk_set_left(hptr_t block, hptr_t left) {
    BLOCK_PTR(block)->left = left;
}

hptr_t bk_right(hptr_t block) {
    return BLOCK_PTR(block)->right;
}

void bk_set_right(hptr_t block, hptr_t right) {
    BLOCK_PTR(block)->right = right;
}

hptr_t bk_parent(hptr_t block) {
    return BLOCK_PTR(block)->__pc & 0xFFFFFFFE;
}

void bk_set_parent(hptr_t block, hptr_t parent) {
    BLOCK_PTR(block)->__pc &= 0x1;
    BLOCK_PTR(block)->__pc |= parent;
}

Color bk_color(hptr_t block) {
    return BLOCK_PTR(block)->__pc & 0x1;
}

void bk_set_color(hptr_t block, Color color) {
    BLOCK_PTR(block)->__pc |= (hptr_t)color;
}

// Partition memory block

// Merge memory blocks
// FREE / ALLOCATED MANAGEMENT
// Free a block --> RBT insertion
// Allocating a block --> RBT removal
// 

// TODO: Expand the heap, do we want to expand it by the exact amount? or do we use some heuristic?
/* -------------------------------------------------------------------------- */
/*                             BLOCK MANIPULATION                             */
/* -------------------------------------------------------------------------- */
// Things to store:
// - Size of the heap

hptr_t partition_block(hptr_t block, uint32_t second_block_size) {

}


int mm_init() {
    mem_reset_brk();
    // TODO
    // Allocate space for ghost node in heap
    // OR... Consider storing it in the stack
    return 0;
}

// Lazy initial allocation
void* mm_malloc(size_t size) {
    // Detect whether or not this is our first allocation

    // Introduce padding
    mem_sbrk(ALIGN((uintptr_t)mem_heap_lo()) - (uintptr_t)mem_heap_lo());
    // Allocate space for user's block
    mem_sbrk(ALIGN(sizeof(BlockHeader) + size + sizeof(BlockFooter)));
    
    // Think of it as a free block -- add it to the rbtree

    // --> Include that new free node in RBTree
    // --> Update the ghost node (should be automatic)




    // Proceed as usual

    return 0;
}

void mm_free(void* ptr) {

}

void* mm_realloc(void* ptr, size_t size) {
    return 0;
}

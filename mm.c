#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "common.h"
#include "mm.h"
#include "rbtree.h"

// TODO: Whenever adding bytes to a block for the sake of alignment,
// TODO: we should expand user section rather than footer because it could improve
// TODO: the chance of finding blocks for future requests

// TODO(style): Implement `footer` and `header`

/* -------------------------------------------------------------------------- */
/*                              STATIC VARIABLES                              */
/* -------------------------------------------------------------------------- */
rbtree_t rbtree;

// Have ghost node point to the root of out RB Tree
/* -------------------------------------------------------------------------- */
/*                           BLOCK MEMBER VARIABLES                           */
/* -------------------------------------------------------------------------- */
BlockHeader* bk_header(hptr_t block) {
    return((BlockHeader*)((char*)mem_heap_lo() + block));
}

BlockFooter* bk_footer(hptr_t block) {
    return (BlockFooter*)((char*)mem_heap_lo() + block + bk_size(block));
}

uint32_t bk_size(hptr_t block) {
    return bk_header(block)->__spff & ~0b11;
}

void bk_set_size(hptr_t block, uint32_t size) {
    assert(size % 4 == 0);
    bk_header(block)->__spff &= 0b11;
    bk_header(block)->__spff |= size;
    bk_footer(block)->size = size;
}

hptr_t bk_left(hptr_t block) {
    return bk_header(block)->left;
}

void bk_set_left(hptr_t block, hptr_t left) {
    bk_header(block)->left = left;
}

hptr_t bk_right(hptr_t block) {
    return bk_header(block)->right;
}

void bk_set_right(hptr_t block, hptr_t right) {
    bk_header(block)->right = right;
}

hptr_t bk_parent(hptr_t block) {
    return bk_header(block)->__pc & ~0b1;
}

void bk_set_parent(hptr_t block, hptr_t parent) {
    bk_header(block)->__pc &= 0b1;
    bk_header(block)->__pc |= parent;
}

Color bk_color(hptr_t block) {
    return bk_header(block)->__pc & 0b1;
}

void bk_set_color(hptr_t block, Color color) {
    bk_header(block)->__pc &= ~0b1;
    bk_header(block)->__pc |= (hptr_t)color;
}

// size | prev_free | free

bool bk_is_free(hptr_t block) {
    return bk_header(block)->__spff & 0b1;
}

void bk_set_is_free(hptr_t block, bool is_free) {
    bk_header(block)->__spff &= ~0b1;
    bk_header(block)->__spff |= is_free;
    bk_header(next_block(block))->__spff &= ~0b10;
    bk_header(next_block(block))->__spff |= is_free << 1;
}

/* -------------------------- BLOCK FAMILY MEMEBERS ------------------------- */
hptr_t next_block(hptr_t block){
    return block + sizeof(uint32_t) + bk_size(block);
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

/**
 * @pre Block can actually accomodate for the requested partition
 * 
 * @param size_needed Size needed in "user size" (i.e., do not include metadata)
 * 
 * @remark This function updates the metadata of the blocks after partition
 */
hptr_t partition_block(hptr_t block, uint32_t size_needed) {
    assert(!bk_is_free(block));
    size_needed = ALIGN(sizeof(uint32_t) + size_needed) - sizeof(uint32_t);
    assert(size_needed >= 20);

    uint32_t total_space = sizeof(uint32_t) + bk_size(block);
    uint32_t total_left_space = sizeof(uint32_t) + size_needed;
    uint32_t total_right_space = total_space - total_left_space;

    assert(total_right_space >= sizeof(BlockHeader) + sizeof(BlockFooter));

    bk_set_size(block, size_needed);
    bk_set_size(block + total_left_space, total_right_space - sizeof(uint32_t));

    return block + total_left_space;
}

hptr_t partition_if_worth_it(hptr_t block, uint32_t size_needed) {
    uint32_t block_space = sizeof(uint32_t) + bk_size(block);
    size_needed = ALIGN(sizeof(uint32_t) + size_needed) - sizeof(uint32_t);
    uint32_t total_left_space = sizeof(uint32_t) + size_needed;

    if (block_space - total_left_space >= 20) {
        return partition_block(block, size_needed);
    }

    return NULL_HPTR;
}

int mm_init() {
    rbtree.block = NULL_HPTR;
    mem_reset_brk();
    return 0;
}

// Some issues...
// ALIGNMENT
// Last block may not be free

void* mm_malloc(size_t size) {
    return 0;
}

void mm_free(void* ptr) {

}

void* mm_realloc(void* ptr, size_t size) {
    return 0;
}

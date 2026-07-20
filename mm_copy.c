#include <assert.h>
#include <stdint.h>

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
    return (BlockFooter*)((char*)bk_header(block) + sizeof(BlockHeader) + bk_size(block));
}

uint32_t bk_size(hptr_t block) {
    return bk_header(block)->size;
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
    return bk_header(block)->__pc & ~0b11;
}

void bk_set_parent(hptr_t block, hptr_t parent) {
    bk_header(block)->__pc &= 0b11;
    bk_header(block)->__pc |= parent;
}

Color bk_color(hptr_t block) {
    return bk_header(block)->__pc & 0b01;
}

void bk_set_color(hptr_t block, Color color) {
    bk_header(block)->__pc |= (hptr_t)color;
}

bool bk_is_free(hptr_t block) {
    return bk_header(block)->__pc & 0b10;
}

void bk_set_is_free(hptr_t block, bool is_free) {
    bk_header(block)->__pc &= ~0b10;
    bk_header(block)->__pc |= is_free << 1;
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

    uint32_t total_space = ALIGN(sizeof(BlockHeader) + bk_size(block) + sizeof(BlockFooter));
    uint32_t total_space_left = ALIGN(sizeof(BlockHeader) + size_needed + sizeof(BlockFooter));
    uint32_t total_space_right = total_space - total_space_left;
    assert(total_space_right > sizeof(BlockHeader) + sizeof(BlockFooter));

    // * RBTree info in header is no longer valid for either block.
    // * Thus, it is important that it is never called with a block inside the rbtree
    // Update left block
    bk_header(block)->size = size_needed;
    bk_footer(block)->size = size_needed;
    // Update right block
    hptr_t right_bk = block + total_space_left;
    bk_header(right_bk)->size = total_space_right - sizeof(BlockHeader) - sizeof(BlockFooter);
    bk_footer(right_bk)->size = total_space_right - sizeof(BlockHeader) - sizeof(BlockFooter);

    return right_bk;
}

hptr_t partition_if_worth_it(hptr_t block, uint32_t space_needed) {
    uint32_t bk_occ_space = ALIGN(sizeof(BlockHeader) + bk_size(block) + sizeof(BlockFooter));
    if (bk_occ_space - space_needed >= 64) {
        return partition_block(block, space_needed);
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
    // Lazy initial allocation
    if (rbtree.block == NULL_HPTR) {
        uint32_t padding = ALIGN((uintptr_t)mem_heap_lo()) - (uintptr_t)mem_heap_lo();
        uint32_t ghost_node_size = ALIGN(sizeof(BlockHeader) + sizeof(BlockFooter));
        mem_sbrk(padding + ghost_node_size);
        // Setup ghost node
        rbtree.block = padding;
        bk_set_left(rbtree.block, NULL_HPTR);
    }

    // Search rbtree for free block
    hptr_t best_fit_bk = rbtree_find(rbtree, size);

    if (best_fit_bk != NULL_HPTR) {
        rbtree_remove(rbtree, best_fit_bk);

        hptr_t remaining_bk = partition_if_worth_it(best_fit_bk, size);
        if (remaining_bk != NULL_HPTR) rbtree_insert(rbtree, remaining_bk);
        
        return (char*)(bk_header(best_fit_bk)) + sizeof(BlockHeader);
    }

    // If there is no free block, expand heap

    // If last block is free...
    BlockFooter* last_bk_footer = (BlockFooter*)((char*)mem_heap_hi() - sizeof(BlockFooter) + 1);
    
    hptr_t last_bk = ((uintptr_t)((char*)last_bk_footer - last_bk_footer->size - sizeof(BlockHeader)) - (uintptr_t)mem_heap_lo());

    uint32_t block_size = ALIGN(sizeof(BlockHeader) + size + sizeof(BlockFooter));
    uint32_t needed_space = block_size;
    
    if (bk_is_free(last_bk)) {
        uint32_t reusable_space = sizeof(BlockHeader) + last_bk_footer->size + sizeof(BlockFooter);
        needed_space -= reusable_space;
    }
    
    uint32_t expansion_size = ALIGN(MAX((uint32_t)(EXPANSION_FACTOR * mem_heapsize()), (uint32_t)needed_space));
    mem_sbrk(expansion_size);

    // Get last block out of the red black tree (its size is about to change)
    rbtree_remove(rbtree, last_bk);
    // Update the size
    bk_header(last_bk)->size = bk_size(last_bk) + expansion_size - sizeof(BlockFooter);
    bk_footer(last_bk)->size = bk_size(last_bk);

    // Give to the user the block they need
    hptr_t remaining_bk = partition_if_worth_it(last_bk, size);
    if (remaining_bk != NULL_HPTR) rbtree_insert(rbtree, remaining_bk);

    return (char*)(bk_header(last_bk)) + sizeof(BlockHeader);
}

void mm_free(void* ptr) {

}

void* mm_realloc(void* ptr, size_t size) {
    return 0;
}

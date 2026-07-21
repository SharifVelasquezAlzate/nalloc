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

bool bk_prev_free(hptr_t block) {
    return bk_header(block)->__spff & 0b10;
}

void bk_set_prev_free(hptr_t block, bool prev_free) {
    bk_header(block)->__spff &= ~0b10;
    bk_header(block)->__spff |= prev_free << 1;
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

/* -------------------------- BLOCK FAMILY MEMBERS ------------------------- */
hptr_t next_block(hptr_t block){
    if (block + sizeof(uint32_t) + bk_size(block) >= mem_heapsize()) {
        return rbtree.block;
    }
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
 * @remark This function corrupts the rbtree metadata as well as curr_free
 */
hptr_t partition_block(hptr_t block, uint32_t size_needed) {
    assert(!bk_is_free(block));
    size_needed = ALIGN(sizeof(uint32_t) + size_needed) - sizeof(uint32_t);
    assert(size_needed >= 20);

    uint32_t total_space = sizeof(uint32_t) + bk_size(block);
    uint32_t total_left_space = sizeof(uint32_t) + size_needed;
    uint32_t total_right_space = total_space - total_left_space;

    assert(total_right_space >= sizeof(BlockHeader) + sizeof(BlockFooter));

    hptr_t right_bk = block + total_left_space;

    bk_set_size(block, size_needed);

    bk_set_size(right_bk, total_right_space - sizeof(uint32_t));
    bk_set_is_free(right_bk, true);
    bk_set_prev_free(right_bk, bk_is_free(block));

    return right_bk;
}

hptr_t partition_if_worth_it(hptr_t block, uint32_t size_needed) {
    uint32_t block_space = sizeof(uint32_t) + bk_size(block);
    size_needed = ALIGN(sizeof(uint32_t) + size_needed) - sizeof(uint32_t);
    uint32_t total_left_space = sizeof(uint32_t) + size_needed;

    // If the remaining block can host a 16-byte allocation, let it live
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
    // Lazy initialization
    if (rbtree.block == NULL_HPTR) {
        uint32_t padding = ALIGN((uintptr_t)mem_heap_lo()) - (uintptr_t)mem_heap_lo();
        uint32_t ghost_node_size = ALIGN(sizeof(BlockHeader) + sizeof(BlockFooter));
        mem_sbrk(padding + ghost_node_size);
        // Setup ghost node
        rbtree.block = padding;
        //! Fix this BS
        bk_set_left(rbtree.block, NULL_HPTR);
        bk_set_prev_free(rbtree.block, false);
    }

    size = ALIGN(sizeof(uint32_t) + size) - sizeof(uint32_t);

    hptr_t free_block = rbtree_find(rbtree, size);

    if (free_block != NULL_HPTR) {
        rbtree_remove(rbtree, free_block);
        hptr_t right_bk = partition_if_worth_it(free_block, size);
        if (right_bk != NULL_HPTR) {
            rbtree_insert(rbtree, right_bk);
        }
        return (char*)mem_heap_lo() + free_block + sizeof(uint32_t);
    }

    // There is no free block :(
    // Don't forget to update prev_free on newly introduced block
    uint32_t expansion_size = ALIGN(MAX(
        MAX((uint32_t)(EXPANSION_FACTOR * mem_heapsize()), sizeof(uint32_t) + size),
        sizeof(BlockHeader) + sizeof(BlockFooter)
    ));
    bool is_last_bk_free = bk_prev_free(rbtree.block);
    hptr_t last_bk = NULL_HPTR;

    if (is_last_bk_free) {
        uint32_t last_bk_size = ((BlockFooter*)((char*)mem_heap_hi() - 3))->size;
        last_bk = mem_heapsize() - last_bk_size - sizeof(uint32_t);
        expansion_size -= sizeof(uint32_t) + last_bk_size;
        
        rbtree_remove(rbtree, last_bk);
    }

    mem_sbrk(expansion_size);

    if (is_last_bk_free) {
        bk_set_size(last_bk, bk_size(last_bk) + expansion_size);
    } else {
        // Convert expanded area into a block (last block, by definition)
        last_bk = mem_heapsize() - expansion_size;
        bk_set_size(last_bk, expansion_size - sizeof(uint32_t));
        bk_set_is_free(last_bk, false);
        bk_set_prev_free(last_bk, false);
    }

    hptr_t right_bk = partition_if_worth_it(last_bk, size);
    if (right_bk != NULL_HPTR) {
        rbtree_insert(rbtree, right_bk);
    }

    return (char*)mem_heap_lo() + last_bk + sizeof(uint32_t);
}

// TODO: Reinstate metadata
void mm_free(void* ptr) {
    
}

void* mm_realloc(void* ptr, size_t size) {
    return 0;
}

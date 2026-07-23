#include <stdlib.h>
#include <assert.h>

#include "rbtree.h"

static char* heap;
hptr_t nb = 0;

bool rbtree_eq_vec(rbtree_t rbtree, Node expected[], int size) {
    Node vectree[size];
    rbtree_to_vec(root(rbtree), vectree);

    for (int i = 0; i < size; ++i) {
        if (vectree[i].size != expected[i].size || vectree[i].color != expected[i].color ) {
            return false;
        }
    }

    return true;
}

hptr_t create_block(uint32_t size) {
    assert(size >= 16);
    hptr_t old_nb = nb;
    bk_set_size(nb, size);
    bk_set_left(nb, NULL_HPTR);
    bk_set_right(nb, NULL_HPTR);
    bk_set_parent(nb, NULL_HPTR);
    bk_set_color(nb, RED);
    bk_set_is_free(nb, true);
    nb += sizeof(uint32_t) + size;
    return old_nb;
}

rbtree_t create_rbtree() {
    rbtree_t rbtree;
    uint32_t padding = ALIGN((uintptr_t)mem_heap_lo()) - (uintptr_t)mem_heap_lo();
    uint32_t ghost_node_size = ALIGN(sizeof(BlockHeader) + sizeof(BlockFooter));
    mem_sbrk(padding + ghost_node_size);
    // Setup ghost node
    rbtree.block = padding;

    bk_set_left(rbtree.block, NULL_HPTR);
    bk_set_prev_free(rbtree.block, false);

    return rbtree;
}

bool TEST_LEFT_ROTATION() {
    hptr_t five = create_block(50 * 4);
    hptr_t two = create_block(20 * 4);
    hptr_t ten = create_block(100 * 4);
    hptr_t eight = create_block(80 * 4);
    hptr_t twelve = create_block(120 * 4);
    hptr_t six = create_block(60 * 4);
    hptr_t nine = create_block(90 * 4);

    tlink(five, two, true);
    tlink(five, ten, false);
    tlink(ten, eight, true);
    tlink(ten, twelve, false);
    tlink(eight, six, true);
    tlink(eight, nine, false);

    hptr_t res = left_rotate(five);

    Node vectree[7] = {};
    rbtree_to_vec(res, vectree);
    int expected[] = {
        100 * 4, 50 * 4, 20 * 4, 80 * 4, 60 * 4, 90 * 4, 120 * 4
    };

    for (int i = 0; i < 7; ++i) {
        assert(vectree[i].size == expected[i]);
    }

    return true;
}

bool TEST_RIGHT_ROTATION() {
    hptr_t five = create_block(50*4);
    hptr_t two = create_block(20*4);
    hptr_t ten = create_block(100*4);
    hptr_t eight = create_block(80*4);
    hptr_t twelve = create_block(120*4);
    hptr_t six = create_block(60*4);
    hptr_t nine = create_block(90*4);
    
    tlink(ten, five, true);
    tlink(ten, twelve, false);
    tlink(five, two, true);
    tlink(five, eight, false);
    tlink(eight, six, true);
    tlink(eight, nine, false);

    hptr_t res = right_rotate(ten);

    Node vectree[7] = {};
    rbtree_to_vec(res, vectree);
    int expected[] = {
        50 * 4, 20 * 4, 100 * 4, 80 * 4, 60 * 4, 90 * 4, 120 * 4
    };

    for (int i = 0; i < 7; ++i) {
        assert(vectree[i].size == expected[i]);
    }

    return true;
}


bool TEST_INSERTION() {
    rbtree_t rbtree = create_rbtree();

    rbtree_insert(rbtree, create_block(150 * 4));
    assert(rbtree_eq_vec(rbtree, (Node[1]){
        csnode(BLACK, 150 * 4)
    }, 1));
    rbtree_insert(rbtree, create_block(50 * 4));
    assert(rbtree_eq_vec(rbtree, (Node[2]){
        csnode(BLACK, 150 * 4),
        csnode(RED, 50 * 4)
    }, 2));
    rbtree_insert(rbtree, create_block(10 * 4));
    assert(rbtree_eq_vec(rbtree, (Node[3]){
        csnode(BLACK, 50 * 4),
        csnode(RED, 10 * 4),
        csnode(RED, 150 * 4)
    }, 3));
    rbtree_insert(rbtree, create_block(120 * 4));
    assert(rbtree_eq_vec(rbtree, (Node[4]){
        csnode(BLACK, 50 * 4),
        csnode(BLACK, 10 * 4),
        csnode(BLACK, 150 * 4),
        csnode(RED, 120 * 4)
    }, 4));
    rbtree_insert(rbtree, create_block(130 * 4));
    assert(rbtree_eq_vec(rbtree, (Node[5]){
        csnode(BLACK, 50 * 4),
        csnode(BLACK, 10 * 4),
        csnode(BLACK, 130 * 4),
        csnode(RED, 120 * 4),
        csnode(RED, 150 * 4)
    }, 5));
    rbtree_insert(rbtree, create_block(170 * 4));
    assert(rbtree_eq_vec(rbtree, (Node[6]){
        csnode(BLACK, 50 * 4),
        csnode(BLACK, 10 * 4),
        csnode(RED, 130 * 4),
        csnode(BLACK, 120 * 4),
        csnode(BLACK, 150 * 4),
        csnode(RED, 170 * 4)
    }, 6));
    rbtree_insert(rbtree, create_block(190 * 4));
    assert(rbtree_eq_vec(rbtree, (Node[7]){
        csnode(BLACK, 50 * 4),
        csnode(BLACK, 10 * 4),
        csnode(RED, 130 * 4),
        csnode(BLACK, 120 * 4),
        csnode(BLACK, 170 * 4),
        csnode(RED, 150 * 4),
        csnode(RED, 190 * 4)
    }, 7));
    rbtree_insert(rbtree, create_block(210 * 4));
    assert(rbtree_eq_vec(rbtree, (Node[8]){
        csnode(BLACK, 130 * 4),
        csnode(RED, 50 * 4),
        csnode(BLACK, 10 * 4),
        csnode(BLACK, 120 * 4),
        csnode(RED, 170 * 4),
        csnode(BLACK, 150 * 4),
        csnode(BLACK, 190 * 4),
        csnode(RED, 210 * 4)
    }, 8));

    return true;
}

bool TEST_FIND() {
    rbtree_t rbtree = create_rbtree();

    rbtree_insert(rbtree, create_block(150 * 4));
    rbtree_insert(rbtree, create_block(50 * 4));
    rbtree_insert(rbtree, create_block(10 * 4));
    rbtree_insert(rbtree, create_block(120 * 4));
    rbtree_insert(rbtree, create_block(130 * 4));
    rbtree_insert(rbtree, create_block(170 * 4));
    rbtree_insert(rbtree, create_block(190 * 4));
    rbtree_insert(rbtree, create_block(210 * 4));

    // Check exact matches
    int nums[8] = { 150*4, 50*4, 10*4, 120*4, 130*4, 170*4, 190*4, 210*4 };
    for (int i = 0; i < 8; ++i) {
        assert(bk_size(rbtree_find(rbtree, nums[i])) == nums[i]);
    }

    // Check if it returns lower bound when not an exact match
    assert(bk_size(rbtree_find(rbtree, 110*4)) == 120*4);
    assert(bk_size(rbtree_find(rbtree, 30*4)) == 50*4);
    assert(bk_size(rbtree_find(rbtree, 140*4)) == 150*4);
    assert(rbtree_find(rbtree, 140000*4) == NULL_HPTR);
    assert(bk_size(rbtree_find(rbtree, 0*4)) == 10*4);
    assert(bk_size(rbtree_find(rbtree, 180*4)) == 190*4);
    assert(bk_size(rbtree_find(rbtree, 200*4)) == 210*4);

    return true;
}

bool TEST_REMOVE() {
    rbtree_t rbtree = create_rbtree();

    hptr_t fifteen = create_block(150 * 4);
    hptr_t five = create_block(50 * 4);
    hptr_t one = create_block(10 * 4);
    hptr_t twelve = create_block(120 * 4);
    hptr_t thirteen = create_block(130 * 4);
    hptr_t seventeen = create_block(170 * 4);
    hptr_t nineteen = create_block(190 * 4);
    hptr_t twentyone = create_block(210 * 4);

    rbtree_insert(rbtree, fifteen);
    rbtree_insert(rbtree, five);
    rbtree_insert(rbtree, one);
    rbtree_insert(rbtree, twelve);
    rbtree_insert(rbtree, thirteen);
    rbtree_insert(rbtree, seventeen);
    rbtree_insert(rbtree, nineteen);
    rbtree_insert(rbtree, twentyone);

    rbtree_remove(rbtree, fifteen);
    assert(rbtree_eq_vec(rbtree, (Node[7]){
        csnode(BLACK, 130*4),
        csnode(RED, 50*4),
        csnode(BLACK, 10*4),
        csnode(BLACK, 120*4),
        csnode(RED, 190*4),
        csnode(BLACK, 170*4),
        csnode(BLACK, 210*4)
    }, 7));

    rbtree_remove(rbtree, nineteen);
    assert(rbtree_eq_vec(rbtree, (Node[6]){
        csnode(BLACK, 130*4),
        csnode(RED, 50*4),
        csnode(BLACK, 10*4),
        csnode(BLACK, 120*4),
        csnode(BLACK, 210*4),
        csnode(RED, 170*4)
    }, 6));

    rbtree_remove(rbtree, thirteen);
    assert(rbtree_eq_vec(rbtree, (Node[5]){
        csnode(BLACK, 170*4),
        csnode(RED, 50*4),
        csnode(BLACK, 10*4),
        csnode(BLACK, 120*4),
        csnode(BLACK, 210*4)
    }, 5));

    rbtree_remove(rbtree, one);
    assert(rbtree_eq_vec(rbtree, (Node[4]){
        csnode(BLACK, 170*4),
        csnode(BLACK, 50*4),
        csnode(RED, 120*4),
        csnode(BLACK, 210*4)
    }, 4));

    rbtree_remove(rbtree, seventeen);
    assert(rbtree_eq_vec(rbtree, (Node[3]){
        csnode(BLACK, 120*4),
        csnode(BLACK, 50*4),
        csnode(BLACK, 210*4)
    }, 3));

    rbtree_remove(rbtree, twelve);
    assert(rbtree_eq_vec(rbtree, (Node[2]){
        csnode(BLACK, 210*4),
        csnode(RED, 50*4)
    }, 2));

    rbtree_remove(rbtree, five);
    assert(rbtree_eq_vec(rbtree, (Node[1]){
        csnode(BLACK, 210*4)
    }, 1));

    rbtree_remove(rbtree, twentyone);
    assert(root(rbtree) == NULL_HPTR);

    return true;
}


int main() {
    const int HEAP_SIZE = 1 * 1024 * 1024;
    heap = malloc(HEAP_SIZE);
    mem_init(heap, HEAP_SIZE);

    TEST_LEFT_ROTATION();
    TEST_RIGHT_ROTATION();
    TEST_INSERTION();
    TEST_FIND();
    TEST_REMOVE();

    printf("Success B)\n");
}

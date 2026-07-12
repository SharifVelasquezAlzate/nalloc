#include <assert.h>

#include "rbtree.h"

typedef hptr_t rbtree_t;

// TODO: Change new_node name
void rbtree_insert(rbtree_t rbtree, hptr_t new_node) {
    hptr_t curr = root(rbtree);

    // If the tree is empty...
    if (root(rbtree) == NULL_HPTR) {
        tlink(rbtree, new_node, true);
        bk_set_color(root(rbtree), BLACK);
        return;
    }

    // Navigate down the tree until you find the insertion spot
    while (true) {
        if (bk_size(new_node) < bk_size(curr)) {
            if (bk_left(curr) == NULL_HPTR) {
                tlink(curr, new_node, true);
                break;
            }
            curr = bk_left(curr);
        } else {
            if (bk_right(curr) == NULL_HPTR) {
                tlink(curr, new_node, false);
                break;
            }
            curr = bk_right(curr);
        }
    }

    if (bk_color(curr) == BLACK) {
        return;
    }

    // Every time we solve an error, we make the current node
    // pose as the newly inserted node, and make our parent
    // be the current node. We re-run this until we reach the root.

    while (new_node != rbtree) {
        // Case 0: new node is root --> make it black
        if (root(rbtree) == new_node) {
            bk_set_color(new_node, BLACK);
            break;
        }

        // If there are no problems, we just continue :)
        if (bk_color(new_node) != RED || bk_color(curr) != RED) {
            new_node = curr;
            curr = bk_parent(curr);
            continue;
        }

        // Case 1: new_node's uncle is red
        if (bk_color(uncle(new_node)) == RED) {
            bk_set_color(grandpa(new_node), RED);
            bk_set_color(uncle(new_node), BLACK);
            bk_set_color(bk_parent(new_node), BLACK);
            // Move up the tree
            new_node = curr;
            curr = bk_parent(curr);
        }

        // Case 2: uncle is black, and new_node forms a triangle with its grandpa
        char tridir = check_if_triangle(new_node);
        if (tridir != 0) {
            if (tridir < 0) left_rotate(bk_parent(new_node));
            else right_rotate(bk_parent(new_node));
            // Move up the tree
            hptr_t tmp = curr;
            curr = new_node;
            new_node = tmp;
            continue;
        }

        // Case 3: uncle is black, and new_node forms a line with its grandpa
        char linedir = check_if_line(new_node);
        if (linedir != 0) {
            bk_set_color(grandpa(new_node), RED);
            bk_set_color(bk_parent(new_node), BLACK);

            if (linedir < 0) right_rotate(grandpa(new_node));
            else left_rotate(grandpa(new_node));

            break;
        }
    }
}

hptr_t rbtree_find(rbtree_t rbtree, uint32_t size) {
    hptr_t curr = root(rbtree);
    hptr_t ub = NULL_HPTR;

    while (curr != NULL_HPTR) {
        if (size <= bk_size(curr)) {
            ub = curr;
            curr = bk_left(curr);
        } else if (size > bk_size(curr)) {
            curr = bk_right(curr);
        }
    }

    return ub;
}

void rbtree_remove(rbtree_t rbtree, hptr_t block) {
    // Remove element as if in a BST
    hptr_t succ = min_node(bk_right(block));
    hptr_t pred = max_node(bk_left(block));
    hptr_t heir = (succ != NULL_HPTR) ? succ : pred;

    // If deleting the only node left...
    if (heir == NULL_HPTR && block == root(rbtree)) {
        set_root(rbtree, NULL_HPTR);
        return;
    }

    if (heir != NULL_HPTR) {
        swap(block, heir);
        Color tmp = bk_color(block);
        bk_set_color(block, bk_color(heir));
        bk_set_color(heir, tmp);
    }

    // Getting rid of double black
    // ! block is now thought of as the double black node
    bool is_db_nil = true;
    while (true) {
        // Case 1
        if (bk_color(block) == RED) {
            tlink(bk_parent(block), NULL_HPTR, is_lc(block));
            break;
        }

        // Case 2
        if (block == root(rbtree)) {
            bk_set_color(block, BLACK);
            break;
        }

        // Case 3
        if (bk_color(sibling(block)) == BLACK
        && bk_color(bk_left(sibling(block))) == BLACK
        && bk_color(bk_right(sibling(block))) == BLACK) {
            bk_set_color(sibling(block), RED);
            hptr_t new_db = NULL_HPTR;
            
            if (bk_color(bk_parent(block)) == BLACK) {
                new_db = bk_parent(block);
            } else {
                bk_set_color(bk_parent(block), BLACK);
            }

            if (is_db_nil) {
                tlink(bk_parent(block), NULL_HPTR, is_lc(block));
                is_db_nil = false;
            }

            if (new_db != NULL_HPTR) block = new_db;
            else break;
            continue;
        }

        // Case 4
        if (bk_color(sibling(block)) == RED) {
            Color tmp = bk_color(bk_parent(block));
            bk_set_color(bk_parent(block), bk_color(sibling(block)));
            bk_set_color(sibling(block), tmp);

            if (is_lc(block)) left_rotate(bk_parent(block));
            else right_rotate(bk_parent(block));
            continue;
        }

        // Case 5
        hptr_t far_nephew = is_lc(block) ? bk_right(sibling(block)) : bk_left(sibling(block));
        hptr_t near_nephew = is_lc(block) ? bk_left(sibling(block)) : bk_right(sibling(block));

        if (bk_color(sibling(block)) == BLACK
        && bk_color(far_nephew) == BLACK
        && bk_color(near_nephew) == RED) {
            bk_set_color(sibling(block), RED);
            bk_set_color(near_nephew, BLACK);

            if (is_lc(block)) right_rotate(sibling(block));
            else left_rotate(sibling(block));
            // Apply case 6
        }

        // Re-compute these
        far_nephew = is_lc(block) ? bk_right(sibling(block)) : bk_left(sibling(block));
        near_nephew = is_lc(block) ? bk_left(sibling(block)) : bk_right(sibling(block));

        // Case 6
        Color tmp = bk_color(bk_parent(block));
        bk_set_color(bk_parent(block), bk_color(sibling(block)));
        bk_set_color(sibling(block), tmp);

        if (is_lc(block)) left_rotate(bk_parent(block));
        else right_rotate(bk_parent(block));
        
        if (is_db_nil) {
            tlink(bk_parent(block), NULL_HPTR, is_lc(block));
            is_db_nil = false;
        }
        bk_set_color(far_nephew, BLACK);
        break;
    }
}
/* -------------------------------------------------------------------------- */
/*                                   HELPERS                                  */
/* -------------------------------------------------------------------------- */
/* --------------------------- TREE MODIFICATIONS --------------------------- */
void tlink(hptr_t to_be_parent, hptr_t to_be_child, bool left) {
    if (left) {
        if (bk_left(to_be_parent) != NULL_HPTR) {
            bk_set_parent(bk_left(to_be_parent), NULL_HPTR);
        }
        bk_set_left(to_be_parent, to_be_child);
    } else {
        if (bk_right(to_be_parent) != NULL_HPTR) {
            bk_set_parent(bk_right(to_be_parent), NULL_HPTR);
        }
        bk_set_right(to_be_parent, to_be_child);
    }
    if (to_be_child != NULL_HPTR) bk_set_parent(to_be_child, to_be_parent);
}

void swap(hptr_t a, hptr_t b) {
    // Parent exchange
    if (is_lc(a)) bk_set_left(bk_parent(a), b);
    else bk_set_right(bk_parent(a), b);

    if (is_lc(b)) bk_set_left(bk_parent(b), a);
    else bk_set_right(bk_parent(b), a);

    hptr_t tmp = bk_parent(a);
    bk_set_parent(a, bk_parent(b));
    bk_set_parent(b, tmp);

    // Children exchange
    // Left child
    if (bk_left(a) != NULL_HPTR) bk_set_parent(bk_left(a), b);
    if (bk_left(b) != NULL_HPTR) bk_set_parent(bk_left(b), a);

    tmp = bk_left(a);
    bk_set_left(a, bk_left(b));
    bk_set_left(b, tmp);
    
    // Right child
    if (bk_right(a) != NULL_HPTR) bk_set_parent(bk_right(a), b);
    if (bk_right(b) != NULL_HPTR) bk_set_parent(bk_right(b), a);

    tmp = bk_right(a);
    bk_set_right(a, bk_right(b));
    bk_set_right(b, tmp);
}

hptr_t left_rotate(hptr_t block) {
    assert(bk_right(block) != NULL_HPTR);

    bk_set_parent(bk_right(block), NULL_HPTR);

    if (bk_parent(block) != NULL_HPTR) {
        tlink(bk_parent(block), bk_right(block), is_lc(block));
    }

    hptr_t rl_gc = bk_left(bk_right(block));

    tlink(bk_right(block), block, true);
    bk_set_right(block, NULL_HPTR);
    tlink(block, rl_gc, false);

    return bk_parent(block);
}

hptr_t right_rotate(hptr_t block) {
    assert(bk_left(block) != NULL_HPTR);

    bk_set_parent(bk_left(block), NULL_HPTR);
    
    if (bk_parent(block) != NULL_HPTR) {
        tlink(bk_parent(block), bk_left(block), is_lc(block));
    }

    hptr_t lr_gc = bk_right(bk_left(block));

    tlink(bk_left(block), block, false);
    bk_set_left(block, NULL_HPTR);
    tlink(block, lr_gc, true);

    return bk_parent(block);
}

/* ---------------------------- TREE DATA ACCESS ---------------------------- */
hptr_t root(rbtree_t rbtree) {
    return bk_left(rbtree);
}

void set_root(rbtree_t rbtree, hptr_t new_root) {
    bk_set_left(rbtree, new_root);
}

/* ----------------------------- FAMILY MEMBERS ----------------------------- */
hptr_t grandpa(hptr_t block) {
    return bk_parent(bk_parent(block));
}

hptr_t uncle(hptr_t block) {    
    return is_lc(bk_parent(block)) ? bk_right(grandpa(block)) : bk_left(grandpa(block));
}

hptr_t sibling(hptr_t block) {
    return is_lc(block) ? bk_right(bk_parent(block)) : bk_left(bk_parent(block));
}

/* ---------------------------- NODE INFORMATION ---------------------------- */
bool is_lc(hptr_t block) {
    assert(bk_parent(block) != NULL_HPTR);
    return bk_left(bk_parent(block)) == block;
}

hptr_t min_node(hptr_t block) {
    if (block == NULL_HPTR) return NULL_HPTR;
    while (bk_left(block) != NULL_HPTR) {
        block = bk_left(block);
    }
    return block;
}

hptr_t max_node(hptr_t block) {
    if (block == NULL_HPTR) return NULL_HPTR;
    while (bk_right(block) != NULL_HPTR) {
        block = bk_right(block);
    }
    return block;
}

/* ---------------------------------- SHAPE --------------------------------- */
char check_if_triangle(hptr_t block) {
    if (bk_parent(block) == NULL_HPTR || grandpa(block) == NULL_HPTR) return 0;

    if (!is_lc(block) && is_lc(bk_parent(block))) return -1;
    if (is_lc(block) && !is_lc(bk_parent(block))) return 1;
    return 0;
}

char check_if_line(hptr_t block) {
    if (bk_parent(block) == NULL_HPTR || grandpa(block) == NULL_HPTR) return 0;

    if (is_lc(block) && is_lc(bk_parent(block))) return -1;
    if (!is_lc(block) && !is_lc(bk_parent(block))) return 1;
    return 0;
}

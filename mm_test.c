#include <stdio.h>
#include <stdlib.h>
#include "mm.h"

static char* heap;

bool TEST_PARTITION_BLOCK() {
    bk_set_size(0, 124);
    partition_block(0, 24);
    
    return true;
}

bool TEST_MALLOC() {
    mm_init();
    char* a1 = mm_malloc(23);
    strcpy(a1, "This is 23 bytes long!");
    char* a2 = mm_malloc(42);
    strcpy(a2, "This is 23 bytes long... Oh wait, it's 42");
    
    return true;
}

int main() {
    heap = malloc(128);
    mem_init(heap, 128);

    TEST_MALLOC();
    return 0;
}

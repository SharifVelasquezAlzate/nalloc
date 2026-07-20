#include <stdio.h>
#include <stdlib.h>
#include "mm.h"

static char* heap;

bool TEST_PARTITION_BLOCK() {
    bk_set_size(0, 124);
    partition_block(0, 24);
    
    return true;
}

int main() {
    heap = malloc(128);
    mem_init(heap, 128);

    TEST_PARTITION_BLOCK();
    return 0;
}

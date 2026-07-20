#include <stdio.h>
#include "mm.h"

int main() {
    mem_init();

    char* a1 = mm_malloc(24);
    strcpy(a1, "This is 23 bytes :D!!!");
    printf("Hello!");
    return 0;
}

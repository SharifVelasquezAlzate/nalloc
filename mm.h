#include <stdio.h>
#include <string.h>
#include "memlib.h"

extern int mm_init (void);
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);
extern void *mm_realloc(void *ptr, size_t size);

/* 
 * Students work in teams of one or two.  Teams enter their team name, 
 * personal names and login IDs in a struct of this
 * type in their bits.c file.
 */
typedef struct {
    char *teamname; /* ID1+ID2 or ID1 */
    char *name1;    /* full name of first member */
    char *id1;      /* login ID of first member */
    char *name2;    /* full name of second member (if any) */
    char *id2;      /* login ID of second member */
} team_t;

/* -------------------------------------------------------------------------- */
/*                                   CUSTOM                                   */
/* -------------------------------------------------------------------------- */
typedef uint32_t hptr_t;
#define BLOCK_PTR(x) ((BlockHeader*)((char*)mem_heap_lo() + x))
#define NULL_HPTR UINT32_MAX

#define ALIGNMENT 8
// TODO: Fix ~0x7 to be compatible with any ALIGNMENT
#define ALIGN(addr) (addr + ALIGNMENT - 1) & ~0x7

// TODO: Fix the BS -- Color should be in the rbtree file.

typedef enum : uint8_t {
    RED = 0,
    BLACK = 1
} Color;

// Thank you NegVorsa!
typedef struct {
    const uint32_t size;
    hptr_t left;
    hptr_t right;
    hptr_t __pc;
} BlockHeader;

typedef struct {
    const uint32_t size;
} BlockFooter;

extern uint32_t bk_size(hptr_t block);

extern hptr_t bk_left(hptr_t block);
extern void bk_set_left(hptr_t block, hptr_t left);

extern hptr_t bk_right(hptr_t block);
extern void bk_set_right(hptr_t block, hptr_t right);

extern hptr_t bk_parent(hptr_t block);
extern void bk_set_parent(hptr_t block, hptr_t parent);

extern Color bk_color(hptr_t);
extern void bk_set_color(hptr_t block, Color color);

extern team_t team;


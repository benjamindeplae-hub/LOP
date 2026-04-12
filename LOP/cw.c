#include <stdlib.h>
#include "cw.h"
#include "instance.h"
#include "optimization.h"

typedef struct {
    long int index;
    long long int score;
} CWEntry;

/* void * is a pointer to an unknown type.
 * You can assign the address of any data type to a void *, e.g., int, char, struct, etc.
 * Since void has no size information, you cannot dereference a void * directly. You need to cast it to a specific type first.
 * The const here applies to the data being pointed to, not the pointer itself.
 * So const void *a means:
 * a is a pointer to read-only data of unknown type.
 */
static int cw_compare(const void *a, const void *b) {
    const CWEntry *ea = (const CWEntry *)a;
    const CWEntry *eb = (const CWEntry *)b;
    if (eb->score > ea->score) return  1;
    if (eb->score < ea->score) return -1;
    return 0;
}

/*
 * CW heuristic:
 *   The attractiveness of row i at step s is:
 *   sum_{j = s + 1 to n} ( c[i][j] - c[j][i] )
 *   At each step s, you pick the best remaining row.
 * 
 *   For each item i, compute score[i] = sum_{j != i} ( c[i][j] - c[j][i] )
 *   Then place items in descending order of score.
 *   This computes a static score upfront for each item.
 *
 * Intuition: a high score means item i contributes more when placed before
 * others than when placed after them, so it should appear early.
 * 
 * Example:
 *      j=0 j=1 j=2 j=3
 * i=0 [0,  10,  5, 8]
 * i=1 [3,   0, 12, 2]
 * i=2 [9,   4,  0, 6]
 * i=3 [1,  11,  7, 0]
 * 
 * Step 1: Compute scores
 * Item 0:
 * j=1: c[0][1] - c[1][0] = 10 - 3 = +7
 * j=2: c[0][2] - c[2][0] = 5 - 9 = -4
 * j=3: c[0][3] - c[3][0] = 8 - 1 = +7
 * Score = 7 - 4 + 7 = +10
 * 
 * Item 1:
 * j=0: 3 - 10 = -7
 * j=2: 12 - 4 = +8
 * j=3: 2 - 11 = -9
 * Score = -7 + 8 - 9 = -8
 * 
 * Item 2:
 * j=0: 9 - 5 = +4
 * j=1: 4 - 12 = -8
 * j=3: 6 - 7 = -1
 * Score = 4 - 8 - 1 = -5
 * 
 * Item 3:
 * j=0: 1 - 8 = -7
 * j=1: 11 - 2 = +9
 * j=2: 7 - 6 = +1
 * Score = -7 + 9 + 1 = +3
 * 
 * Step 2: Sort descending by score (item, score)
 * (0, 10), (3, +3), (2, -5), (1, -8)
 * 
 * Step 3: Output solution
 * s = [0, 3, 2, 1]
 * 
 * Item 0 has the highest score (+10), meaning placing it before others is much cheaper than placing it after them, so it goes first. 
 * Item 1 has the lowest score (-8), meaning it's actually cheaper when others come before it, so it goes last.
 */
void cwSolution(long int *s) {
    long int i, j;
    
    /* entries is a pointer to a type called CWEntry.
     * This means it will hold the address of the first element of an array of CWEntry objects.
     * malloc stands for memory allocation.
     * It allocates a block of memory of a specified size on the heap (dynamic memory) and returns a pointer to it.
     */
    CWEntry *entries = (CWEntry *)malloc(PSize * sizeof(CWEntry));

    for (i = 0; i < PSize; i++) {
        entries[i].index = i;
        entries[i].score = 0;
        for (j = 0; j < PSize; j++) {
            if (i != j)
                entries[i].score += CostMat[i][j] - CostMat[j][i];
        }
    }
    
    /* qsort sorts an array in place (descending)
     * Sort PSize elements of the entries array,
     * where each element is of size sizeof(CWEntry),
     * using the cw_compare function to determine the order.
     */
    qsort(entries, (size_t)PSize, sizeof(CWEntry), cw_compare);

    for (i = 0; i < PSize; i++)
        s[i] = entries[i].index;

    free(entries);
}

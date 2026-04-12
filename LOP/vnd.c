#include "instance.h"
#include "algorithms.h"
#include "vnd.h"

static int firstImprovingTranspose(long int* s, long long int* cost) {
    int i;
    for (i = 0; i < (int)PSize - 1; i++) {
        long long int d = deltaTranspose(s, i);
        if (d > 0) {
            *cost += d;
            applyTranspose(s, i);
            return 1;
        }
    }
    return 0;
}

static int firstImprovingExchange(long int* s, long long int* cost) {
    int i, j;
    for (i = 0; i < (int)PSize - 1; i++) {
        for (j = i + 1; j < (int)PSize; j++) {
            long long int d = deltaExchange(s, i, j);
            if (d > 0) {
                *cost += d;
                applyExchange(s, i, j);
                return 1;
            }
        }
    }
    return 0;
}

static int firstImprovingInsert(long int* s, long long int* cost) {
    int i, j;
    for (i = 0; i < (int)PSize; i++) {
        for (j = 0; j < (int)PSize; j++) {
            if (i == j) continue;
            long long int d = deltaInsert(s, i, j);
            if (d > 0) {
                *cost += d;
                applyInsert(s, i, j);
                return 1;
            }
        }
    }
    return 0;
}

long long int vnd(long int* s, long long int cost, VndOrder order) {
    /*
     * Define the two orderings as arrays of function pointers.
     * Each function performs one first-improvement pass and returns
     * 1 if it improved, 0 otherwise.
     * 
     * typedef creates a new name (alias) for an existing type.
     * NeighFn is a type for a pointer to a function,
     * that takes:
     * a pointer to long int
     * a pointer to long long int
     * returns: an int
     */
    typedef int (*NeighFn)(long int*, long long int*);

    /* tei is an array of 3 elements, where each element is a function pointer of type NeighFn. */
    NeighFn tei[3] = { firstImprovingTranspose,
                       firstImprovingExchange,
                       firstImprovingInsert };

    NeighFn tie[3] = { firstImprovingTranspose,
                       firstImprovingInsert,
                       firstImprovingExchange };

    NeighFn* neighs = (order == VND_TEI) ? tei : tie;

    int k = 0;
    while (k < 3) {
        if (neighs[k](s, &cost)) {
            /* improvement found: restart from first neighbourhood */
            k = 0;
        }
        else {
            /* locally optimal for this neighbourhood: try next */
            k++;
        }
    }

    return cost;
}
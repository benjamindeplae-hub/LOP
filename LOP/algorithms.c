#include <stdlib.h>
#include <string.h>
#include "algorithms.h"
#include "instance.h"
#include "optimization.h"

/* =========================================================================
 * INCREMENTAL DELTA FUNCTIONS
 * 
 * The LOP objective is: maximise sum_{i<j} c[s[i]][s[j]]
 * In simple terms:
 * Sum all values above the diagonal of c, but after reordering rows/columns using s.
 * 
 * s = a permutation of {0, 1, ..., n-1}
 * Example n = 4:
 * {1,2,3,4}
 * 
 * c = cost matrix of size n x n
 * Example n = 4:
 * s  1 2 3 4
 * 1 [1 3 2 4
 * 2  2 1 1 3
 * 3  1 2 2 1
 * 4  4 5 1 4]
 * 
 * Pairs (i, j) with i < j:
 * (1, 2) -> c[s[1]][s[2]] = 3
 * (1, 3) -> c[s[1]][s[3]] = 2
 * (1, 4) -> c[s[1]][s[4]] = 4
 * (2, 3) -> c[s[2]][s[3]] = 1
 * (2, 4) -> c[s[2]][s[4]] = 3
 * (3, 4) -> c[s[3]][s[4]] = 1
 * Sum f(s) = 3 + 2 + 4 + 1 + 3 + 1 = 14
 * 
 * delta = the change in the objective value when you modify your current solution.
 * 
 * Delta functions compute the change in objective value from applying a move.
 * A positive delta means the move improves the solution.
 * ========================================================================= */

/*
 * Transpose: swap adjacent s[i] and s[i+1].
 *
 * Only the arc between those two elements changes sign:
 *   Before: ... + c[ s[i]   ] [ s[i+1] ] + ...
 *   After:  ... + c[ s[i+1] ] [ s[i]   ] + ...
 *   delta = c[s[i+1]][s[i]] - c[s[i]][s[i+1]]
 *
 * All other pairs are unaffected.
 * 
 * long long int is simply a data type defined by the C standard that guarantees at least 64 bits (8 bytes) of storage.
 * So it can store very large integers.
 * s is a pointer to a long int.
 * So instead of storing a number, s stores a memory address.
 */
long long int deltaTranspose(long int *s, int i) {
    return (long long int)CostMat[s[i+1]][s[i]]
         - (long long int)CostMat[s[i]][s[i+1]];
}

/*
 * Exchange: swap s[i] and s[j], where i < j.
 *
 * Elements strictly between positions i and j (call them s[m]) each
 * interact with both s[i] and s[j]:
 *   Before: c[s[i]][s[m]] + c[s[m]][s[j]]
 *   After:  c[s[j]][s[m]] + c[s[m]][s[i]]
 *
 * The direct arc between s[i] and s[j] also changes:
 *   Before: c[s[i]][s[j]]
 *   After:  c[s[j]][s[i]]
 *
 * Elements outside [i,j] are unaffected.
 */
long long int deltaExchange(long int *s, int i, int j) {
    int m;
    long long int delta = 0;

    /* Contribution of the direct arc between s[i] and s[j] */
    delta += (long long int)CostMat[s[j]][s[i]]
           - (long long int)CostMat[s[i]][s[j]];

    /* Contributions of elements strictly between i and j */
    for (m = i + 1; m < j; m++) {
        delta += (long long int)CostMat[s[j]][s[m]]
               - (long long int)CostMat[s[i]][s[m]];
        delta += (long long int)CostMat[s[m]][s[i]]
               - (long long int)CostMat[s[m]][s[j]];
    }

    return delta;
}

/*
 * Insert: remove element at position `from` and insert it at position `to`.
 *
 * Let e = s[from] (the element being moved).
 *
 * Case from < to (move element rightward):
 *   Elements in (from, to] shift one position left.
 *   For each such element s[m]:
 *     Before: e is to the left of s[m], contributing c[e][s[m]]
 *     After:  e is to the right of s[m], contributing c[s[m]][e]
 *     Change: c[s[m]][e] - c[e][s[m]]
 *
 * Case from > to (move element leftward):
 *   Elements in [to, from) shift one position right.
 *   For each such element s[m]:
 *     Before: s[m] is to the left of e, contributing c[s[m]][e]
 *     After:  s[m] is to the right of e, contributing c[e][s[m]]
 *     Change: c[e][s[m]] - c[s[m]][e]
 *
 * Elements outside the affected range are unaffected.
 */
long long int deltaInsert(long int *s, int from, int to) {
    int m;
    long long int delta = 0;
    long int e = s[from];

    if (from < to) {
        for (m = from + 1; m <= to; m++) {
            delta += (long long int)CostMat[s[m]][e]
                   - (long long int)CostMat[e][s[m]];
        }
    } else {
        for (m = to; m < from; m++) {
            delta += (long long int)CostMat[e][s[m]]
                   - (long long int)CostMat[s[m]][e];
        }
    }

    return delta;
}

/* =========================================================================
 * APPLY MOVES IN-PLACE
 * ========================================================================= */
void applyTranspose(long int *s, int i) {
    long int tmp = s[i];
    s[i]   = s[i+1];
    s[i+1] = tmp;
}

void applyExchange(long int *s, int i, int j) {
    long int tmp = s[i];
    s[i] = s[j];
    s[j] = tmp;
}

void applyInsert(long int *s, int from, int to) {
    long int e = s[from];
    int m;

    if (from < to) {
        for (m = from; m < to; m++)
            s[m] = s[m+1];
    } else {
        for (m = from; m > to; m--)
            s[m] = s[m-1];
    }
    s[to] = e;
}

/* =========================================================================
 * ITERATIVE IMPROVEMENT — TRANSPOSE
 *
 * Neighbourhood: all adjacent transpositions (n-1 neighbours).
 *
 * First-improvement: apply the first improving transpose found,
 *   then restart scanning from the beginning.
 * Best-improvement: scan all n-1 pairs, apply the best improving one,
 *   then repeat until no improvement exists.
 * ========================================================================= */
long long int iiTranspose(long int *s, long long int cost, PivotRule pivot) {
    int i;
    int improved = 1;

    while (improved) {
        improved = 0;

        if (pivot == FIRST_IMPROVEMENT) {
            for (i = 0; i < (int)PSize - 1; i++) {
                if (deltaTranspose(s, i) > 0) {
                    cost += deltaTranspose(s, i);
                    applyTranspose(s, i);
                    improved = 1;
                    break; /* restart from beginning */
                }
            }
        } else { /* BEST_IMPROVEMENT */
            int bestI = -1;
            long long int bestDelta = 0;

            for (i = 0; i < (int)PSize - 1; i++) {
                long long int d = deltaTranspose(s, i);
                if (d > bestDelta) {
                    bestDelta = d;
                    bestI = i;
                }
            }
            if (bestI >= 0) {
                cost += bestDelta;
                applyTranspose(s, bestI);
                improved = 1;
            }
        }
    }

    return cost;
}

/* =========================================================================
 * ITERATIVE IMPROVEMENT — EXCHANGE
 *
 * Neighbourhood: all pairs (i,j) with i < j  ->  n*(n-1)/2 neighbours.
 * ========================================================================= */
long long int iiExchange(long int *s, long long int cost, PivotRule pivot) {
    int i, j;
    int improved = 1;

    while (improved) {
        improved = 0;

        if (pivot == FIRST_IMPROVEMENT) {
            int found = 0;
            for (i = 0; i < (int)PSize - 1 && !found; i++) {
                for (j = i + 1; j < (int)PSize && !found; j++) {
                    if (deltaExchange(s, i, j) > 0) {
                        cost += deltaExchange(s, i, j);
                        applyExchange(s, i, j);
                        improved = 1;
                        found = 1;
                    }
                }
            }
        } else { /* BEST_IMPROVEMENT */
            int bestI = -1, bestJ = -1;
            long long int bestDelta = 0;

            for (i = 0; i < (int)PSize - 1; i++) {
                for (j = i + 1; j < (int)PSize; j++) {
                    long long int d = deltaExchange(s, i, j);
                    if (d > bestDelta) {
                        bestDelta = d;
                        bestI = i;
                        bestJ = j;
                    }
                }
            }
            if (bestI >= 0) {
                cost += bestDelta;
                applyExchange(s, bestI, bestJ);
                improved = 1;
            }
        }
    }

    return cost;
}

/* =========================================================================
 * ITERATIVE IMPROVEMENT — INSERT
 *
 * Neighbourhood: remove element at position i, reinsert at position j
 *   where j != i  ->  n*(n-1) neighbours.
 * ========================================================================= */
long long int iiInsert(long int *s, long long int cost, PivotRule pivot) {
    int i, j;
    int improved = 1;

    while (improved) {
        improved = 0;

        if (pivot == FIRST_IMPROVEMENT) {
            int found = 0;
            for (i = 0; i < (int)PSize && !found; i++) {
                for (j = 0; j < (int)PSize && !found; j++) {
                    if (i == j) continue;
                    if (deltaInsert(s, i, j) > 0) {
                        cost += deltaInsert(s, i, j);
                        applyInsert(s, i, j);
                        improved = 1;
                        found = 1;
                    }
                }
            }
        } else { /* BEST_IMPROVEMENT */
            int bestFrom = -1, bestTo = -1;
            long long int bestDelta = 0;

            for (i = 0; i < (int)PSize; i++) {
                for (j = 0; j < (int)PSize; j++) {
                    if (i == j) continue;
                    long long int d = deltaInsert(s, i, j);
                    if (d > bestDelta) {
                        bestDelta = d;
                        bestFrom = i;
                        bestTo = j;
                    }
                }
            }
            if (bestFrom >= 0) {
                cost += bestDelta;
                applyInsert(s, bestFrom, bestTo);
                improved = 1;
            }
        }
    }

    return cost;
}

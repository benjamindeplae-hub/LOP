#ifndef _ALGORITHMS_H_
#define _ALGORITHMS_H_

#include "optimization.h"

/* =========================================================================
 * Pivoting rules
 * ========================================================================= */
typedef enum { FIRST_IMPROVEMENT, BEST_IMPROVEMENT } PivotRule;

/* =========================================================================
 * Neighborhood move delta functions
 * ========================================================================= */
long long int deltaTranspose(long int *s, int i);
long long int deltaExchange(long int *s, int i, int j);
long long int deltaInsert(long int *s, int from, int to);

/* =========================================================================
 * Apply a move in-place
 * ========================================================================= */
void applyTranspose(long int *s, int i);
void applyExchange(long int *s, int i, int j);
void applyInsert(long int *s, int from, int to);

/* =========================================================================
 * Iterative improvement
 * ========================================================================= */
long long int iiTranspose(long int *s, long long int cost, PivotRule pivot);
long long int iiExchange(long int *s, long long int cost, PivotRule pivot);
long long int iiInsert(long int *s, long long int cost, PivotRule pivot);

#endif

/*  Heuristic Optimization assignment, 2015.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
    ---
    ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
    Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* -----------------------------------------------------------------------
 * getopt() shim for Windows (MSVC / MinGW).
 * On POSIX the system header provides this; on Windows we define it here.
 * ----------------------------------------------------------------------- */
#ifdef _WIN32
static int opterr = 1;
static int optind = 1;
static int optopt = 0;
static char *optarg = NULL;

static int getopt(int argc, char *const argv[], const char *optstring)
{
    static int sp = 1;
    int c;
    const char *cp;

    if (sp == 1) {
        if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
            return -1;
        if (strcmp(argv[optind], "--") == 0) { ++optind; return -1; }
    }
    c = argv[optind][sp];
    cp = strchr(optstring, c);
    if (cp == NULL || c == ':') {
        if (opterr) fprintf(stderr, "%s: illegal option -- %c\n", argv[0], c);
        optopt = c;
        if (argv[optind][++sp] == '\0') { ++optind; sp = 1; }
        return '?';
    }
    if (cp[1] == ':') {
        if (argv[optind][sp + 1] != '\0')
            optarg = &argv[optind++][sp + 1];
        else if (++optind >= argc) {
            if (opterr) fprintf(stderr, "%s: option requires an argument -- %c\n", argv[0], c);
            optopt = c; sp = 1; return '?';
        } else
            optarg = argv[optind++];
        sp = 1;
    } else {
        if (argv[optind][++sp] == '\0') { ++optind; sp = 1; }
        optarg = NULL;
    }
    return c;
}
#else
#include <getopt.h>
#endif
/* ----------------------------------------------------------------------- */



#include "instance.h"
#include "utilities.h"
#include "timer.h"
#include "optimization.h"

#define INSTANCE_FILE "../data/instances/N-be75eec_150"

char *FileName;

static void setFileName(const char* src) {
    size_t len = strlen(src);
    FileName = (char*)malloc(len + 1);
#ifdef _WIN32
    strncpy_s(FileName, len + 1, src, len);
#else
    strncpy(FileName, src, len + 1);
#endif
}

void readOpts(int argc, char **argv) {
    char opt;
    FileName = NULL;
    while ((opt = (char)getopt(argc, argv, "i:")) > 0)
        switch (opt) {
            case 'i':
                setFileName(optarg);
                break;
            default:
                fprintf(stderr, "Option %c not managed.\n", opt);
        }

    if (!FileName) {
        printf("No instance file provided (use -i <instance_name>). Proceeding with '" INSTANCE_FILE "'.\n\n");
        // exit(1);
        // Fall back to hardcoded default if no -i was given
        setFileName(INSTANCE_FILE);
    }
}


int main_teacher(int argc, char **argv)
{
    long int i, j;
    long int *currentSolution;
    int cost, newCost, temp, firstRandomPosition, secondRandomPosition;

    /* Do not buffer output */
#ifdef _WIN32
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#else
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
#endif

    /* Read parameters */
    readOpts(argc, argv);

    /* Read instance file */
    CostMat = readInstance(FileName);
    printf("Data have been read from instance file. Size of instance = %ld.\n\n", PSize);
   
    /* initialize random number generator, deterministically based on instance.
     * To do this we simply set the seed to the sum of elements in the matrix, so it is constant per-instance,
     but (most likely) varies between instances */
    Seed = (long int)0;
    for (i = 0; i < PSize; ++i)
        for (j = 0; j < PSize; ++j)
            Seed += (long int)CostMat[i][j];
    printf("Seed used to initialize RNG: %ld.\n\n", Seed);

    /* starts time measurement */
    start_timers();

    /* A solution is just a vector of int with the same size as the instance */
    currentSolution = (long int *)malloc(PSize * sizeof(long int));

    /* Create an initial random solution.
        The only constraint is that it should always be a permutation */
    createRandomSolution(currentSolution);

    /* Print solution */
    printf("Initial solution:\n");
    for (j = 0; j < PSize; j++)
        printf(" %ld", currentSolution[j]);
    printf("\n");

    /* Compute cost of solution and print it */
    cost = (int)computeCost(currentSolution);
    printf("Cost of this initial solution: %d\n\n", cost);

    /* Example: apply an exchange operation of two elements at random position */
    firstRandomPosition  = randInt(0, (PSize - 1));
    // Ensure second position is different from first one:
    secondRandomPosition = firstRandomPosition + randInt(1, (PSize - 2));
    if (secondRandomPosition >= PSize)
        secondRandomPosition -= PSize;

    printf("Two positions exchanged: %d and %d. ", firstRandomPosition, secondRandomPosition);

    temp = currentSolution[firstRandomPosition];
    currentSolution[firstRandomPosition]  = currentSolution[secondRandomPosition];
    currentSolution[secondRandomPosition] = temp;

    printf("Solution after exchange:\n");
    for (j = 0; j < PSize; j++)
        printf(" %ld", currentSolution[j]);
    printf("\n");

    /* Recompute cost of solution after the exchange move */
    /* There are some more efficient way to do this, instead of recomputing everything... */
    newCost = (int)computeCost(currentSolution);
    printf("Cost of this solution after applying the exchange move: %d\n", newCost);

    if (newCost == cost)
        printf("Second solution is as good as first one\n");
    else if (newCost > cost)
        printf("Second solution is better than first one\n");
    else
        printf("Second solution is worse than first one\n");

    printf("Time elapsed since we started the timer: %g\n\n", elapsed_time(VIRTUAL));

    free(currentSolution);
    free(FileName);
    return 0;
}

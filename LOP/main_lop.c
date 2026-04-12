/*
 * Main for LOP iterative improvement exercise.
 *
 * Runs all 12 combinations of:
 * - 2 starting solutions: Uninformed Random Picking, Chenery-Watanabe (CW)
 * - 3 neighbourhoods: Transpose, Exchange, Insert
 * - 2 pivoting rules: First-improvement, Best-improvement
 *
 * Output is also written to results.csv for easy import into R.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instance.h"
#include "utilities.h"
#include "timer.h"
#include "optimization.h"
#include "algorithms.h"
#include "cw.h"

long int Seed;

static void copySolution(long int* dst, const long int* src) {
    long int i;
    for (i = 0; i < PSize; i++)
        dst[i] = src[i];
}

static void formatDev(char* buf, size_t buf_size, long long int finalCost, long long int bestKnown, int hasBestKnown) {
    if (hasBestKnown && bestKnown != 0) {
        /* percentage difference between bestKnown and your result */
        double dev = 100.0 * ((double)bestKnown - (double)finalCost) / (double)bestKnown;
#ifdef _WIN32
        /* Windows : use the "safe" sprintf to format a float into a string */
        sprintf_s(buf, buf_size, "%+.4f", dev);
#else
        /* Other platforms : snprintf is safe and portable
         * Prevents buffer overflows while formatting the float
         */
        snprintf(buf, buf_size, "%+.4f", dev);
#endif
    }
    else {
#ifdef _WIN32
        /* Windows : use safe string copy instead of formatting
         * No numeric formatting needed, just copy literal
         */
        strcpy_s(buf, buf_size, "N/A");
#else
        /* Other platforms : use snprintf to write literal string safely
         * snprintf also respects buffer size, preventing overflow
         */
        snprintf(buf, buf_size, "N/A");
#endif
    }
}

static long long int runAlgorithm(const long int* initSol, long long int initCost, int neighbourhood /* 0=Transpose 1=Exchange 2=Insert */, PivotRule pivot, double* timeOut) {
    long int* s = (long int*)malloc(PSize * sizeof(long int));
    copySolution(s, initSol);

    double t0 = elapsed_time(REAL);
    long long int finalCost;
    switch (neighbourhood) {
        case 0:
            finalCost = iiTranspose(s, initCost, pivot);
            break;
        case 1:
            finalCost = iiExchange(s, initCost, pivot);
            break;
        case 2:
            finalCost = iiInsert(s, initCost, pivot);
            break;
        default: 
            finalCost = initCost;
            break;
        }
    *timeOut = elapsed_time(REAL) - t0;
    free(s);
    return finalCost;
}

static void parseArguments(int argc, char** argv, char** instanceFile,
    long long int* bestKnown, int* hasBestKnown) {

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            *instanceFile = argv[++i];
        }
        else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            *bestKnown = atoll(argv[++i]);
            *hasBestKnown = 1;
        }
        else {
            printf("Unknown or incomplete argument: %s\n", argv[i]);
            printf("Usage: %s -i <file> [-b <value>]\n", argv[0]);
            exit(1);
        }
    }

    if (*instanceFile == NULL) {
        printf("Usage: %s -i <file> [-b <value>]\n", argv[0]);
        exit(1);
    }
}

static void buildCsvFilename(const char* instanceFile, char* out, size_t outSize) {
    const char* baseName = instanceFile;
    const char* p = instanceFile;
    while (*p) {
        /* Every time a / or \\ is found, baseName is updated to point to the character after it. */
        if (*p == '/' || *p == '\\') baseName = p + 1;
        /* p walks through the string character by character. */
        p++;
    }
    /* After the loop, baseName points to just the filename with no directory, e.g. "N-be75eec_150". */
    /* Both functions write a formatted string into out, but they differ by platform. */
#ifdef _WIN32
    sprintf_s(out, outSize, "%s-result.csv", baseName);
#else
    snprintf(out, outSize, "%s-result.csv", baseName);
#endif
}

/* 
* argc -> argument count
* argv -> argument values (an array of strings)
*/
int main_lop(int argc, char** argv) {
    long int i, j;
    long long int bestKnown = 0;
    int hasBestKnown = 0;

#ifdef _WIN32
    /* This is about making output appear immediately when you print something, instead of waiting.
     * Normally, C doesn’t always print immediately. 
     * It buffers output, meaning it collects text in memory and prints it later (maybe after a newline \n, or when the buffer is full).
     * This code turns off buffering so everything prints right away.
     * setvbuf(stream, buffer, mode, size) controls buffering.
     * stream -> stdout or stderr
     * buffer -> NULL (we don’t provide a buffer)
     * mode -> _IONBF -> No buffering
     * size -> 0 (ignored when _IONBF is used)
    */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#else
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
#endif

    char* instanceFile = NULL;
    parseArguments(argc, argv, &instanceFile, &bestKnown, &hasBestKnown);

    /* Read instance file */
    CostMat = readInstance(instanceFile);
    printf("Data have been read from instance file. Size of instance = %ld.\n\n", PSize);

    /* Initialize random number generator, deterministically based on instance.
     * To do this we simply set the seed to the sum of elements in the matrix, so it is constant per-instance,
     * but (most likely) varies between instances.
     */
    Seed = (long int)0;
    for (i = 0; i < PSize; i++)
        for (j = 0; j < PSize; j++)
            Seed += (long int)CostMat[i][j];
    printf("Seed used to initialize RNG: %ld.\n\n", Seed);

    /* starts time measurement */
    start_timers();

    /* Build two starting solutions (Uninformed Random Picking / Chenery and Watanabe (CW)) */
    long int* randomSol = (long int*)malloc(PSize * sizeof(long int));
    long int* cwSol = (long int*)malloc(PSize * sizeof(long int));
    createRandomSolution(randomSol);
    cwSolution(cwSol);

    /* Compute cost of the two initial solutions */
    long long int randomCost = computeCost(randomSol);
    long long int cwCost = computeCost(cwSol);
    /* %lld means
     * % -> starts a format specifier
     * l -> stands for long
     * ll -> stands for long long
     * d -> decimal integer
     * print cwCost as a signed long long integer in decimal.
     */
    printf("Random start cost : %lld\n", randomCost);
    printf("CW start cost     : %lld\n\n", cwCost);

    /* neighNames is an array of elements.
    * Each element is a const char*, a pointer to the first character of a string
    */
    const char* neighNames[] = { "Transpose", "Exchange", "Insert" };
    const char* pivotNames[] = { "First-improvement", "Best-improvement" };
    const char* startNames[] = { "Random", "CW" };

    printf("%-6s  %-9s  %-17s  %14s  %10s  %9s\n", 
           "Start", "Neighbour", "Pivot", "Cost", "Time (s)", "Dev (%)");
    printf("------  ---------  -----------------  --------------  ----------  ---------\n");
     
    char csvName[512];
    buildCsvFilename(instanceFile, csvName, sizeof(csvName));
    FILE* csv = fopen(csvName, "w");
    if (csv)
        fprintf(csv, "start,neighbourhood,pivot,cost,time_in_seconds,deviation_percentage\n");

    /* Run the algorithm twice, each time with a different initial solution:
     * startIdx == 0 -> Uninformed Random Picking
     * startIdx == 1 -> Chenery-Watanabe (CW) heuristic
     */
    int startIdx;
    for (startIdx = 0; startIdx < 2; startIdx++) {
        long int* initSol = (startIdx == 0) ? randomSol : cwSol;
        long long int initCost = (startIdx == 0) ? randomCost : cwCost;

		/* Run the algorithm three times, each time with a different neighbourhood sructure:
         * neighborhood == 0 -> Transpose
         * neighborhood == 1 -> Exchange
         * neighborhood == 2 -> Insert
         */
        int neighborhood;
        for (neighborhood = 0; neighborhood < 3; neighborhood++) {

            /* Run the algorithm twice, each time with a different pivoting rule:
             * pivotIdx == 0 -> FIRST_IMPROVEMENT
             * pivotIdx == 1 -> BEST_IMPROVEMENT
             */
            int pivotIdx;
            for (pivotIdx = 0; pivotIdx < 2; pivotIdx++) {
                PivotRule pivot = (pivotIdx == 0) ? FIRST_IMPROVEMENT : BEST_IMPROVEMENT;

                double elapsed;
                long long int finalCost = runAlgorithm(initSol, initCost, neighborhood, pivot, &elapsed);

                char deviationStr[16];
                formatDev(deviationStr, sizeof(deviationStr), finalCost, bestKnown, hasBestKnown);

                printf("%-6s  %-9s  %-17s  %14lld  %10.4f  %9s\n",
                    startNames[startIdx], neighNames[neighborhood], pivotNames[pivotIdx], finalCost, elapsed, deviationStr);

                if (csv) {
                    fprintf(csv, "%s,%s,%s,%lld,%.6f,%s\n",
                        startNames[startIdx],
                        neighNames[neighborhood],
                        pivotNames[pivotIdx],
                        finalCost, elapsed,
                        hasBestKnown ? deviationStr : "NA");
                }
            }
        }
    }

    if (csv) fclose(csv);
    printf("\nResults also written to %s\n", csvName);
    printf("Total wall time: %.4f s\n\n\n", elapsed_time(REAL));

    free(randomSol);
    free(cwSol);
    return 0;
}

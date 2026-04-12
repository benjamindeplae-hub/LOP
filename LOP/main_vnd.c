/*
 * Main for LOP iterative improvement exercise.
 * 
 * Runs 2 combinations of:
 * - VND with order Transpose -> Exchange -> Insert
 * - VND with order Transpose -> Insert   -> Exchange
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
#include "cw.h"
#include "vnd.h"

static void copySolution(long int* dst, const long int* src) {
    long int i;
    for (i = 0; i < PSize; i++)
        dst[i] = src[i];
}

static void formatDev(char* buf, size_t buf_size, long long int finalCost, long long int bestKnown, int hasBestKnown) {
    if (hasBestKnown && bestKnown != 0) {
        double dev = 100.0 * ((double)bestKnown - (double)finalCost) / (double)bestKnown;
#ifdef _WIN32
        sprintf_s(buf, buf_size, "%+.4f", dev);
#else
        snprintf(buf, buf_size, "%+.4f", dev);
#endif
    } else {
#ifdef _WIN32
        strcpy_s(buf, buf_size, "N/A");
#else
        snprintf(buf, buf_size, "N/A");
#endif
    }
}

static long long int runVND(const long int* initSol, long long int initCost, VndOrder order, double* timeOut) {
    long int* s = (long int*)malloc(PSize * sizeof(long int));
    copySolution(s, initSol);

    double t0 = elapsed_time(REAL);
    long long int finalCost = vnd(s, initCost, order);
    *timeOut = elapsed_time(REAL) - t0;
    free(s);
    return finalCost;
}

void parseArguments(int argc, char** argv, char** instanceFile,
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
        if (*p == '/' || *p == '\\') baseName = p + 1;
        p++;
    }
#ifdef _WIN32
    sprintf_s(out, outSize, "%s-result.csv", baseName);
#else
    snprintf(out, outSize, "%s-result.csv", baseName);
#endif
}


int main_vnd(int argc, char** argv) {
    long int i, j;
    long long int bestKnown = 0;
    int hasBestKnown = 0;

#ifdef _WIN32
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#else
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
#endif

    char* instanceFile = NULL;
    parseArguments(argc, argv, &instanceFile, &bestKnown, &hasBestKnown);

    CostMat = readInstance(instanceFile);
    printf("Data have been read from instance file. Size of instance = %ld.\n\n", PSize);

    Seed = (long int)0;
    for (i = 0; i < PSize; i++)
        for (j = 0; j < PSize; j++)
            Seed += (long int)CostMat[i][j];
    printf("Seed used to initialize RNG: %ld.\n\n", Seed);

    start_timers();

    long int* randomSol = (long int*)malloc(PSize * sizeof(long int));
    long int* cwSol = (long int*)malloc(PSize * sizeof(long int));
    createRandomSolution(randomSol);
    cwSolution(cwSol);

    long long int randomCost = computeCost(randomSol);
    long long int cwCost = computeCost(cwSol);
    printf("Random start cost : %lld\n", randomCost);
    printf("CW start cost     : %lld\n\n", cwCost);

    printf("%-20s  %14s  %10s  %9s\n",
           "Algorithm", "Cost", "Time (s)", "Dev (%)");
    printf("--------------------  --------------  ----------  ---------\n");

    char csvName[512];
    buildCsvFilename(instanceFile, csvName, sizeof(csvName));
    FILE* csv = fopen(csvName, "w");
    if (csv)
        fprintf(csv, "algorithm,cost,time_in_seconds,deviation_percentage\n");

    /* Define the two VND variants */
    const char* vndNames[] = {
        "VND_TEI (T->E->I)",
        "VND_TIE (T->I->E)"
    };
    VndOrder vndOrders[] = { VND_TEI, VND_TIE };

    int startIdx;
    for (startIdx = 0; startIdx < 2; startIdx++) {
        double elapsed;
        long long int finalCost = runVND(cwSol, cwCost, vndOrders[startIdx], &elapsed);

        char deviationStr[16];
        formatDev(deviationStr, sizeof(deviationStr), finalCost, bestKnown, hasBestKnown);

        printf("%-20s  %14lld  %10.4f  %9s\n",
            vndNames[startIdx], finalCost, elapsed, deviationStr);

        if (csv)
            fprintf(csv, "%s,%lld,%.6f,%s\n",
                vndNames[startIdx],
                finalCost,
                elapsed,
                hasBestKnown ? deviationStr : "NA");
    }

    if (csv) fclose(csv);
    printf("\nResults also written to %s\n", csvName);
    printf("Total wall time: %.4f s\n\n\n", elapsed_time(REAL));

    free(randomSol);
    free(cwSol);
    return 0;
}

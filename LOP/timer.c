/*****************************************************************************/
/*                                                                           */
/*      Version:  2.00   Date: 01/01/2009   File: timer.c                    */
/* Changes:                                                                  */
/* 30/10/1996 Created   (Thomas Stuetzle)                                    */
/* 01/01/2009 Added windows compatibility (Tommaso Schiavinotto)             */
/*****************************************************************************/
/*                                                                           */
/*===========================================================================*/


/*---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>  /* exit() */
#include "timer.h"

#ifdef _WIN32
#include <windows.h>

static double virtual_time_start;
static double real_time_start;

static double filetime_to_seconds(const FILETIME *ft)
{
    ULARGE_INTEGER ui;
    ui.LowPart = ft->dwLowDateTime;
    ui.HighPart = ft->dwHighDateTime;
    return (double)ui.QuadPart / 1e7;
}

void start_timers(void)
{
    FILETIME creation, ex, kernel, user;
    if (!GetProcessTimes(GetCurrentProcess(), &creation, &ex, &kernel, &user)) {
        fprintf(stderr, "start_timers: GetProcessTimes failed.\n");
        exit(1);
    }
    virtual_time_start = filetime_to_seconds(&user) + filetime_to_seconds(&kernel);
    real_time_start = (double)GetTickCount64() / 1000.0;
}

double elapsed_time(TIMER_TYPE type)
{
    if (type == REAL) {
        return (double)GetTickCount64() / 1000.0 - real_time_start;
    } else {
        FILETIME creation, ex, kernel, user;
        if (!GetProcessTimes(GetCurrentProcess(), &creation, &ex, &kernel, &user)) {
            fprintf(stderr, "elapsed_time: GetProcessTimes failed.\n");
            exit(1);
        }
        return filetime_to_seconds(&user) + filetime_to_seconds(&kernel) - virtual_time_start;
    }
}

#else
#include <sys/time.h>
#include <sys/resource.h>

static double virtual_time_start;
static double real_time_start;

/*
 *  The virtual time of day and the real time of day are calculated and
 *  stored for future use.  The future use consists of subtracting these
 *  values from similar values obtained at a later time to allow the user
 *  to get the amount of time used by the backtracking routine.
 */
void start_timers(void)
{
    struct rusage  res;
    struct timeval tp;
    getrusage(RUSAGE_SELF, &res);
    virtual_time_start = (double)res.ru_utime.tv_sec  + (double)res.ru_stime.tv_sec
                       + (double)res.ru_utime.tv_usec / 1e6
                       + (double)res.ru_stime.tv_usec / 1e6;
    gettimeofday(&tp, NULL);
    real_time_start = (double)tp.tv_sec + (double)tp.tv_usec / 1e6;
}

/*
 *  Stop the stopwatch and return the time used in seconds (either
 *  REALX or VIRTUAL time, depending on ``type'').
 */
double elapsed_time(TIMER_TYPE type)
{
    if (type == REAL) {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        return (double)tp.tv_sec + (double)tp.tv_usec / 1e6 - real_time_start;
    } else {
        struct rusage res;
        getrusage(RUSAGE_SELF, &res);
        return (double)res.ru_utime.tv_sec  + (double)res.ru_stime.tv_sec
             + (double)res.ru_utime.tv_usec / 1e6
             + (double)res.ru_stime.tv_usec / 1e6
             - virtual_time_start;
    }
}
#endif

#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>

#define GLOBAL_PREFIX extern

#define MAX_TIMERS 256
#define MAX_NAME_LENGTH 256

typedef enum {
    TIMER_SUCCESS = 0,
    TIMER_NAME_NOT_FOUND,
    TIMER_INNACCESSIBLE,
    TIMER_ALREADY_RUNNING,
    TIMER_ALREADY_STOPPED,
    TIMER_ALREADY_INITIALIZED
} ALTimer_error_t;

typedef enum {
    STOPPED = 0,
    RUNNING = 1
} ALTimer_state_t;

typedef struct {
    char name [MAX_NAME_LENGTH];
    double firstStartClock;
    double lastStopClock;

    double currentStartClock;
    double lastInterval;
    double totalInterval;

    // For std.dev.
    double totalInvervalSqr;
    int totalIntervalCount;

    ALTimer_state_t state;
} ALTimer_id_t;

typedef struct {
    char name[MAX_NAME_LENGTH];
    double time;
    double stddev;
} ALTimer_result_t;

typedef struct _ALTimer_tree_t {
    char name[MAX_NAME_LENGTH];
    double time;

    struct _ALTimer_tree_t *parent;
    int treeLevel;
    double timeExclChildren;
} ALTimer_tree_t;

extern ALTimer_error_t TIMER_STATUS;

int ALTimer_is_initialized();

ALTimer_error_t ALTimer_init ();

ALTimer_error_t ALTimer_finalize ();

ALTimer_error_t ALTimer_start (const char *);

ALTimer_error_t ALTimer_stop (const char *);

ALTimer_error_t ALTimer_reset (const char *);

void ALTimer_reset_timers ();

/*
 * This function return the current interval,
 * i.e. the interval between the last "timer_start"
 * and now.
 */
double ALTimer_get_current_interval (const char *);

/*
 * This function return the last "recorded" interval.
 * i.e. the interval between the last "timer_start"
 * and "timer_stop"
 */
double ALTimer_get_last_interval (const char *);

double ALTimer_get_total_interval (const char *);

ALTimer_id_t * ALTimer_get_timer_object (const char *);

void ALTimer_stop_timers ();

void ALTimer_print_timers_short ();

void ALTimer_print_timers ();

void ALTimer_print_timer_names ();

ALTimer_error_t ALTimer_print_timer (const char *);

uint32_t ALTimer_get_num_timers ();

char** ALTimer_get_timer_names ();

ALTimer_result_t * ALTimer_get_results_sorted();

ALTimer_tree_t * ALTimer_get_results_tree();

// Prints all timers in a tree format (indented children below parent timers)
// If excludeChildTime is true (non-zero), the time printed for a timer is the
// difference of its full value and the sum of the full values of all child
// timers. Otherwise, time printed is simply the full value for each timer.
void ALTimer_print_results_tree(int excludeChildTime);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif

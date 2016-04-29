/*
 * File:   test_timer.c
 * Author: sriram
 *
 * Created on Jun 5, 2012, 4:00:43 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include <assert.h>

/*
 * Simple C Test Suite
 */

void test1() {
    char timerName [][256] = {"timer1", "timer2"};

    ALTimer_start (timerName [0]);
    sleep (5);
    ALTimer_stop (timerName [0]);

    ALTimer_start (timerName [0]);
    sleep (2);
    ALTimer_stop (timerName [0]);

    printf ("Sleep of %d seconds introduced and the timer says: %lf\n", 7, ALTimer_get_total_interval (timerName [0]));
    ALTimer_reset_timers ();

    ALTimer_start (timerName [1]);
    sleep (2);
    ALTimer_stop (timerName [1]);
    printf ("Sleep of %d seconds introduced and the timer says: %lf\n", 2, ALTimer_get_total_interval (timerName [1]));

    ALTimer_print_timers ();
}


void test_result_sort() {
    int i;
    ALTimer_result_t *results;

    ALTimer_start ("timerOuter");
    sleep (2);
    ALTimer_start ("timerInner");
    sleep (3);
    ALTimer_stop ("timerInner");
    ALTimer_stop ("timerOuter");

    ALTimer_start ("timerTwice");
    sleep (2);
    ALTimer_stop ("timerTwice");

    ALTimer_start ("timerMiddle");
    sleep (2);
    ALTimer_stop ("timerMiddle");

    ALTimer_start ("timerTwice");
    sleep (2);
    ALTimer_stop ("timerTwice");

    printf("\nSORTED RESULTS:\n\n");

    printf("\nExpected results:\n");
    printf("timerOuter: 5\n");
    printf("timerInner: 3\n");
    printf("timerTwice: 4\n");
    printf("timerMiddle: 2\n");
    printf("\nActual results:\n");

    results = ALTimer_get_results_sorted();
    for (i = 0; i < ALTimer_get_num_timers(); i++) {
        printf("%s: %0.4lf\n", results[i].name, results[i].time);
    }
    free(results);

    printf("\nTREE RESULTS (without child excl):\n");

    printf("\nExpected results:\n");
    printf("+ [timerOuter] 5\n");
    printf("++ [timerInner] 3\n");
    printf("+ [timerTwice] 4\n");
    printf("++ [timerMiddle] 2\n");
    printf("\nActual results:\n");

    ALTimer_print_results_tree(0);

    printf("\nTREE RESULTS (with child excl):\n");

    printf("\nExpected results:\n");
    printf("+ [timerOuter] 2\n");
    printf("++ [timerInner] 3\n");
    printf("+ [timerTwice] 2\n");
    printf("++ [timerMiddle] 2\n");
    printf("\nActual results:\n");

    ALTimer_print_results_tree(1);
}


int main(int argc, char** argv)
{
    // To check if re-initting does not crash anything
    ALTimer_init ();
    ALTimer_finalize ();

    ALTimer_init ();
    printf ("Timer initialized\n");
    //test1();

    printf ("List of timers: \n");
    ALTimer_print_timer_names ();
    ALTimer_finalize ();

    ALTimer_init ();
    test_result_sort();
    ALTimer_finalize ();

    printf ("Timer finalized\n");

    return (EXIT_SUCCESS);
}


#include "timer.h"
#include "khash.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

ALTimer_error_t TIMER_STATUS;

double _totalTime = 0;
double _currentTime [MAX_TIMERS];
uint8_t _currentTimerID = 0;
int initLevel = 0;

#if __GNUC__
#if __x86_64__ || __ppc64__
    #define ptr_t int64_t
#else
    #define ptr_t int32_t
#endif
#else
    #define ptr_t int32_t
#endif

KHASH_MAP_INIT_STR(str, ptr_t)

khash_t(str) *_timerTable = 0;
khiter_t _timerIter;

static inline double dclock(void)
{
    struct timeval tv;
    gettimeofday(&tv,0);

    return (double) tv.tv_sec + (double) tv.tv_usec * 1e-6;
}

ALTimer_error_t timer_add (const char *timerName)
{
    // Timer should really be my first call
    double now = dclock ();
    ALTimer_id_t *newTimer = (ALTimer_id_t *) malloc (sizeof (ALTimer_id_t));
    int status = 0;

    strcpy (newTimer->name, timerName);
    newTimer->lastInterval = 0;
    newTimer->totalInterval = 0;
    newTimer->state = RUNNING;
    newTimer->currentStartClock = now;
    newTimer->firstStartClock = now;
    newTimer->lastStopClock = -1;

    newTimer->totalIntervalCount = 0;
    newTimer->totalInvervalSqr = 0;

    _timerIter = kh_put (str, _timerTable, newTimer->name, &status);
    kh_val(_timerTable, _timerIter) = (ptr_t) newTimer;

    TIMER_STATUS = TIMER_SUCCESS;

    return TIMER_STATUS;
}

int ALTimer_is_initialized()
{
	return initLevel > 0;
}

ALTimer_error_t ALTimer_init ()
{
    if (initLevel++ > 0)
        return TIMER_SUCCESS;

    char timerName [] = "ROOT";

    if (_timerTable) {
        TIMER_STATUS = TIMER_ALREADY_INITIALIZED;
        return TIMER_STATUS;
    }

    _timerTable = kh_init (str);
    timer_add (timerName);

    TIMER_STATUS = TIMER_SUCCESS;

    return TIMER_STATUS;
}

ALTimer_error_t ALTimer_start (const char *timerName)
{
    ALTimer_id_t *currentTimer = ALTimer_get_timer_object (timerName);

    if (currentTimer == NULL) {
        timer_add (timerName);
    } else {
        if (currentTimer->state == RUNNING) {
            TIMER_STATUS = TIMER_ALREADY_RUNNING;
            return TIMER_STATUS;
        }

        if (! (strcmp (currentTimer->name, "ROOT"))) {
            TIMER_STATUS = TIMER_INNACCESSIBLE;
            return TIMER_STATUS;
        }

        currentTimer->state = RUNNING;

        const double now = dclock ();
        currentTimer->currentStartClock = now;
        if (currentTimer->firstStartClock < 0)
            currentTimer->firstStartClock = now;
    }

    TIMER_STATUS = TIMER_SUCCESS;

    return TIMER_STATUS;
}

ALTimer_error_t ALTimer_stop (const char *timerName)
{
    const double now = dclock ();
    ALTimer_id_t *currentTimer = ALTimer_get_timer_object (timerName);

    if (currentTimer == NULL) {
        TIMER_STATUS = TIMER_NAME_NOT_FOUND;
        return TIMER_STATUS;
    }

    if (currentTimer->state == STOPPED) {
        TIMER_STATUS = TIMER_ALREADY_STOPPED;
        return TIMER_STATUS;
    }

    if (currentTimer->name == "ROOT") {
        TIMER_STATUS = TIMER_INNACCESSIBLE;
        return TIMER_STATUS;
    }

    currentTimer->state = STOPPED;
    currentTimer->lastStopClock = now;
    currentTimer->lastInterval = now - currentTimer->currentStartClock;
    currentTimer->totalInterval += currentTimer->lastInterval;

    currentTimer->totalIntervalCount++;
    currentTimer->totalInvervalSqr += currentTimer->lastInterval * currentTimer->lastInterval;

    TIMER_STATUS = TIMER_SUCCESS;

    return TIMER_STATUS;
}

ALTimer_error_t ALTimer_reset (const char *timerName)
{
    ALTimer_id_t *currentTimer = ALTimer_get_timer_object (timerName);

    if (currentTimer == NULL) {
        TIMER_STATUS = TIMER_NAME_NOT_FOUND;
        return TIMER_STATUS;
    }

    if (currentTimer->name == "ROOT") {
        TIMER_STATUS = TIMER_INNACCESSIBLE;
        return TIMER_STATUS;
    }

    currentTimer->state = STOPPED;
    currentTimer->lastInterval = 0;
    currentTimer->totalInterval = 0;
    currentTimer->firstStartClock = -1;
    currentTimer->lastStopClock = -1;

    currentTimer->totalIntervalCount = 0;
    currentTimer->totalInvervalSqr = 0;

    TIMER_STATUS = TIMER_SUCCESS;

    return TIMER_STATUS;
}

double ALTimer_get_current_interval (const char *timerName)
{
    double currentStartClock = dclock ();
    ALTimer_id_t *currentTimer = ALTimer_get_timer_object (timerName);

    if (currentTimer == NULL) {
        TIMER_STATUS = TIMER_NAME_NOT_FOUND;
        return 0;
    }

    if (currentTimer->state == STOPPED) {
        return 0;
    }

    TIMER_STATUS = TIMER_SUCCESS;
    return currentStartClock - currentTimer->currentStartClock;
}

double ALTimer_get_last_interval (const char *timerName)
{
    ALTimer_id_t *currentTimer = ALTimer_get_timer_object (timerName);

    if (currentTimer == NULL) {
        TIMER_STATUS = TIMER_NAME_NOT_FOUND;
        return 0;
    }

    TIMER_STATUS = TIMER_SUCCESS;
    return currentTimer->lastInterval;
}

double ALTimer_get_total_interval (const char *timerName)
{
    ALTimer_id_t *currentTimer = ALTimer_get_timer_object (timerName);

    if (currentTimer == NULL) {
        TIMER_STATUS = TIMER_NAME_NOT_FOUND;
        return 0;
    }

    TIMER_STATUS = TIMER_SUCCESS;
    return currentTimer->totalInterval;
}

void ALTimer_print_timers_short ()
{
    for (_timerIter = kh_begin (_timerTable); _timerIter != kh_end (_timerTable); _timerIter ++) {
        ALTimer_id_t *currentTimer = (ALTimer_id_t *) kh_val(_timerTable, _timerIter);
        if (kh_exist (_timerTable, _timerIter)) {
            printf ("[%s] %lf\n", currentTimer->name, currentTimer->totalInterval);
        }
    }
    TIMER_STATUS = TIMER_SUCCESS;
}

void ALTimer_print_timers ()
{
    printf ("\n============================\n");
    for (_timerIter = kh_begin (_timerTable); _timerIter != kh_end (_timerTable); _timerIter ++) {
        ALTimer_id_t *currentTimer = (ALTimer_id_t *) kh_val(_timerTable, _timerIter);
        if (kh_exist (_timerTable, _timerIter)) {
            printf ("[%s] Current = %lf, Last recorded = %lf, Total recorded = %lf, Current State = %s\n",
                                        currentTimer->name,
                                        ALTimer_get_current_interval (currentTimer->name),
                                        currentTimer->lastInterval,
                                        currentTimer->totalInterval,
                                        (currentTimer->state ? "RUNNING" : "STOPPED")
                                         );
        }
    }
    printf ("============================\n\n");
    TIMER_STATUS = TIMER_SUCCESS;
}

ALTimer_error_t ALTimer_print_timer (const char *timerName)
{
    ALTimer_id_t *currentTimer = ALTimer_get_timer_object (timerName);

    if (currentTimer == NULL) {
        printf ("Timer %s not found\n", timerName);
        return TIMER_NAME_NOT_FOUND;
    }

    printf ("[%s] Current = %lf, Last recorded = %lf, Total recorded = %lf, Current State = %s\n",
                                currentTimer->name,
                                ALTimer_get_current_interval (currentTimer->name),
                                currentTimer->lastInterval,
                                currentTimer->totalInterval,
                                (currentTimer->state ? "RUNNING" : "STOPPED")
                                 );

    TIMER_STATUS = TIMER_SUCCESS;
    return TIMER_STATUS;
}

ALTimer_id_t * ALTimer_get_timer_object (const char *timerName)
{
    int status = 0;
    _timerIter = kh_get (str, _timerTable, timerName);

    if (_timerIter == kh_end (_timerTable)) {
        TIMER_STATUS = TIMER_NAME_NOT_FOUND;
        return 0;
    }

    TIMER_STATUS = TIMER_SUCCESS;
    return (ALTimer_id_t *) (kh_value (_timerTable, _timerIter));
}

void timer_print_timers_total ()
{
    for (_timerIter = kh_begin (_timerTable); _timerIter != kh_end (_timerTable); _timerIter ++) {
        if (kh_exist (_timerTable, _timerIter)) {
            ALTimer_id_t *currentTimer = (ALTimer_id_t *) kh_val(_timerTable, _timerIter);
            printf ("[%s] Total time = %lf\n", currentTimer->name, currentTimer->totalInterval);
        }
    }
    TIMER_STATUS = TIMER_SUCCESS;
}

uint32_t ALTimer_get_num_timers ()
{
    return kh_size (_timerTable);
}

char** ALTimer_get_timer_names ()
{
    uint32_t numTimers = ALTimer_get_num_timers ();
    char **timerNames = (char **) malloc (numTimers * sizeof (char *));
    uint32_t i = 0;

    for (_timerIter = kh_begin (_timerTable); _timerIter != kh_end (_timerTable); _timerIter ++) {
         if (kh_exist (_timerTable, _timerIter)) {
            timerNames [i] = (char *) kh_key (_timerTable, _timerIter);
            i ++;
         }
    }
    TIMER_STATUS = TIMER_SUCCESS;

    return timerNames;
}

void timer_clear_timer_names (char ***timerNames)
{
    uint32_t numTimers = ALTimer_get_num_timers ();
    uint32_t i = 0;
    char **timerNamesPtr = *timerNames;

    for (i = 0; i < numTimers; i ++) {
        free (timerNamesPtr [i]);
    }

    free (timerNamesPtr);
    TIMER_STATUS = TIMER_SUCCESS;

    return ;
}

void ALTimer_print_timer_names ()
{
    uint32_t numTimers = ALTimer_get_num_timers ();

    for (_timerIter = kh_begin (_timerTable); _timerIter != kh_end(_timerTable); _timerIter ++) {
        if (kh_exist (_timerTable, _timerIter)) {
            printf ("%s\n", kh_key (_timerTable, _timerIter));
        }
    }
    TIMER_STATUS = TIMER_SUCCESS;
}

int timer_comp(const void *t1, const void *t2) {
    const double st1 = ((const ALTimer_id_t *)t1)->firstStartClock;
    const double st2 = ((const ALTimer_id_t *)t2)->firstStartClock;
    return (st1 > st2) - (st1 < st2);
}

ALTimer_result_t * ALTimer_get_results_sorted()
{
    uint32_t numTimers = ALTimer_get_num_timers ();
    ALTimer_id_t *sortedTimers = (ALTimer_id_t *)calloc(numTimers, sizeof(ALTimer_id_t));
    ALTimer_result_t *results = (ALTimer_result_t *)calloc(numTimers, sizeof(ALTimer_result_t));

    int timerIdx = 0;
    for (_timerIter = kh_begin (_timerTable); _timerIter != kh_end(_timerTable); _timerIter ++) {
        if (kh_exist (_timerTable, _timerIter)) {
            const ALTimer_id_t *curTimer = (const ALTimer_id_t *)kh_val(_timerTable, _timerIter);
            sortedTimers[timerIdx++] = *curTimer;
            //printf ("%s\n", kh_key (_timerTable, _timerIter));
        }
    }

    qsort(sortedTimers, timerIdx, sizeof(ALTimer_id_t), timer_comp);

    for (timerIdx = 0; timerIdx < numTimers; timerIdx++) {
        const ALTimer_id_t *curTimer = &sortedTimers[timerIdx];
        strcpy(results[timerIdx].name, curTimer->name);
        results[timerIdx].time = curTimer->totalInterval;

        const int n = curTimer->totalIntervalCount;
        if (n > 1) {
            const double moment1 = curTimer->totalInterval / n;
            const double moment2 = curTimer->totalInvervalSqr / n;
            const double variance = ((double)n / (n - 1)) * (moment2 - moment1 * moment1);
            const double stddev = sqrt(variance);
            results[timerIdx].stddev = stddev;
        } else {
            results[timerIdx].stddev = INFINITY;
        }

    }

    free(sortedTimers);

    TIMER_STATUS = TIMER_SUCCESS;
    return results;
}

ALTimer_tree_t * ALTimer_get_results_tree()
{
    int i;
    uint32_t numTimers = ALTimer_get_num_timers ();
    ALTimer_result_t *sorted = ALTimer_get_results_sorted();

    ALTimer_tree_t *tree = (ALTimer_tree_t*)calloc(numTimers, sizeof(ALTimer_tree_t));

    // First build the tree based on containment of timed intervals
    // (assume proper nesting, such that if timer A's start is <=
    // timer B's start, and A's stop is >= B's, A contains B, and A
    // is then a parent of B in the tree.
    ALTimer_tree_t *curParent = NULL;
    ALTimer_id_t *curParentTimer = NULL;
    for (i = 0; i < numTimers; i++) {
        ALTimer_result_t *curResult = &sorted[i];
        ALTimer_id_t *curTimer = ALTimer_get_timer_object(curResult->name);

        ALTimer_tree_t *curTimerNode = &tree[i];
        strcpy(curTimerNode->name, curResult->name);
        curTimerNode->time = curResult->time;
        curTimerNode->timeExclChildren = curResult->time;

        // Unwind the ancestry until we find a parent that ends after this timer,
        // or until there are no more parents
        while (curParent != NULL && curParentTimer->lastStopClock < curTimer->lastStopClock) {
            curParent = curParent->parent;
            curParentTimer = curParent != NULL ? ALTimer_get_timer_object(curParent->name) : NULL;
        }

        // Now curParent is our parent
        curTimerNode->parent = curParent;
        curTimerNode->treeLevel = (curParent != NULL) ? curParent->treeLevel + 1 : 0;

        curParent = curTimerNode;
        curParentTimer = curTimer;
    }

    // Iterate over the tree node list backwards (which is guaranteed to be in
    // a topological order of parent pointers), subtracting each child's time
    // from it's parent's time-without-children field.
    for (i = numTimers - 1; i >= 0; i--)
        if (tree[i].parent)
            tree[i].parent->timeExclChildren -= tree[i].time;

    free(sorted); // Cleanup
    return tree;
}

void ALTimer_print_results_tree(int excludeChildTime) {
    int i, j;
    int numTimers = ALTimer_get_num_timers();
    ALTimer_tree_t *tree = ALTimer_get_results_tree();

    char *hasNextSiblings = (char*)calloc(numTimers, sizeof(char));
    for (i = 0; i < numTimers; i++) {
        hasNextSiblings[i] = 0;

        ALTimer_tree_t *curNode = &tree[i];
        for (j = i + 1; j < numTimers; j++) {
            ALTimer_tree_t *otherNode = &tree[j];
            if (otherNode->treeLevel == curNode->treeLevel)
                hasNextSiblings[i] = 1;
            if (otherNode->treeLevel <= curNode->treeLevel)
                break;
        }
    }

    char *siblingBridgePending = (char*)calloc(sizeof(char), numTimers);

    for (i = 0; i < numTimers; ++i) {
        ALTimer_tree_t *curNode = &tree[i];
        ALTimer_tree_t *nextNode = (i < numTimers - 1) ? &tree[i+1] : NULL;

        int hasNextSibling = hasNextSiblings[i];

        // Indent
        for (j = 0; j < curNode->treeLevel; j++) {
            putc(siblingBridgePending[j] ?
                    '\263' : // up, down
                    ' ',
                 stdout);
            putc(' ', stdout);
        }

        if (i == 0)
            putc('\332', stdout); // down, right
        else if (hasNextSibling)
            putc('\303', stdout); // up, down, right
        else
            putc('\300', stdout); // up, right
        putc('\304', stdout); // right

        siblingBridgePending[curNode->treeLevel] = hasNextSibling;

        // Print timer
        printf("*[%s] %0.4lf\n", curNode->name, excludeChildTime ? curNode->timeExclChildren : curNode->time);
    }

    free(tree);
    free(hasNextSiblings);
}

void ALTimer_stop_timers ()
{
    for (_timerIter = kh_begin (_timerTable); _timerIter != kh_end(_timerTable); _timerIter ++) {
        if (kh_exist (_timerTable, _timerIter)) {
            ALTimer_stop (kh_key (_timerTable, _timerIter));
        }
    }

    TIMER_STATUS = TIMER_SUCCESS;
}

void ALTimer_reset_timers ()
{
    for (_timerIter = kh_begin (_timerTable); _timerIter != kh_end(_timerTable); _timerIter ++) {
        if (kh_exist (_timerTable, _timerIter)) {
            const char *timerName = kh_key (_timerTable, _timerIter);

            if (strcmp (timerName, "ROOT") == 0) {
                continue ;
            }

            ALTimer_reset (timerName);
        }
    }

    TIMER_STATUS = TIMER_SUCCESS;
}

ALTimer_error_t ALTimer_finalize ()
{
    if (--initLevel > 0)
        return TIMER_SUCCESS;

    for (_timerIter = kh_begin (_timerTable); _timerIter != kh_end(_timerTable); _timerIter ++) {
        if (kh_exist (_timerTable, _timerIter)) {
            ALTimer_id_t *currentTimer = ALTimer_get_timer_object (kh_key (_timerTable, _timerIter));
            kh_del (str, _timerTable, _timerIter);
            free (currentTimer);
            currentTimer = 0;
        }
    }

    kh_destroy(str, _timerTable);
    _timerTable = 0;
    TIMER_STATUS = TIMER_SUCCESS;

    return TIMER_STATUS;
}

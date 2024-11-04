#ifndef DISPATCHER
#define DISPATCHER

/*
SECTION 1A: C STANDARD LIBRARY
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
SECTION 1B: SYSTEM CALL HEADER FILES
*/
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>

/*
SECTION 1C: OTHER INCLUDES
*/
#include <pcb.h>

/*
SECTION 2: VARIOUS MACROS
*/
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define ARGS_JOBS_FILENAME 1
#define ARGS_EXACT_COUNT 2
#define JOBS_SPLIT_COUNT 3
#define UNIT_CPU_TIME_SIM 1

/*
SECTION 4: FUNCTION PROTOTYPES AND DEFINITIONS
*/
typedef struct
{
    uint64_t total_turnaround;
    uint64_t total_waiting;
    uint64_t total_response;
    uint64_t completed_jobs;
} Metrics;

Metrics metrics;

/*
DESCRIPTION:
    - Reads the job dispatch queue from the jobs file and stores it in a queue.

RETURNS:
    + Block* which is the head of a newly initialized queue.
    + NULL if file is unable to be read.
*/
Block *initializeJobDispatchQueue(char *filename)
{
    FILE *file = fopen(filename, "r");

    if (!file)
    {
        return NULL;
    }

    Block *jobs = NULL;
    Block *process = NULL;
    while (!feof(file))
    {
        process = createNullBlock();

        /*
        NOTE:
            - Read in the arguments and if the arguments don't match then we
            have a problem. Free the block and go to the next input.
        */
        if (fscanf(file, "%d, %d, %d", &(process->arrival_time),
                   &(process->service_time), &(process->priority)) != JOBS_SPLIT_COUNT)
        {
            free(process);
            continue;
        }
        process->remaining_cpu_time = process->service_time;
        process->status = PCB_INITIALIZED;

        jobs = enqueueBlock(jobs, process);
    }

    return jobs;
}

/*
DESCRIPTION:
    - Counts the total number of jobs by traversing through the linked list
    recursively.

RETURNS:
    + The total number of jobs
*/
uint64_t countTotalJobs(Block *head)
{
    if (!head)
    {
        return 0;
    }

    return 1 + countTotalJobs(head->next);
}

/*
DESCRIPTION:
    - Prints out everything in a queue for testing purposes.

RETURN:
    + Nothing
*/
void printQueue(Block *head)
{
    Block *current = head;
    printBlockHeader();

    while (current)
    {
        printBlock(current);
        current = current->next;
    }
}

/*
DESCRIPTION:
    - Gets user input `t0`, `t1`, `t2`, and `W`. Modifies the pointers passed
    into the function. It ensures that all integers are positive.

RETURN:
    + Nothing.
*/
void getUserInput(unsigned int *t0, unsigned int *t1, unsigned int *t2,
                  unsigned int *W)
{
    /*
    NOTE:
        - Loop over until the input is valid for `t0`. Must be a positive integ-
        er and must be a valid parse.
    */
    while (TRUE)
    {
        printf("Enter time quantum for Level-0 (t0): ");
        if (scanf("%u", t0) && *t0 > 0)
            break;
        printf("ERROR: Enter a positive integer\n");
        while (getchar() != '\n')
            ;
    }

    /*
    NOTE:
        - Loop over until the input is valid for `t1`. Must be a positive integ-
        er and must be a valid parse.
    */
    while (TRUE)
    {
        printf("Enter time quantum for Level-1 (t1): ");
        if (scanf("%u", t1) && *t1 > 0)
            break;
        printf("ERROR: Enter a positive integer\n");
        while (getchar() != '\n')
            ;
    }

    /*
    NOTE:
        - Loop over until the input is valid for `t2`. Must be a positive integ-
        er and must be a valid parse.
    */
    while (TRUE)
    {
        printf("Enter time quantum for Level-2 (t2): ");
        if (scanf("%u", t2) && *t2 > 0)
            break;
        printf("ERROR: Enter a positive integer\n");
        while (getchar() != '\n')
            ;
    }

    /*
    NOTE:
        - Loop over until the input is valid for `W`. Must be a positive integ-
        er and must be a valid parse.
    */
    while (TRUE)
    {
        printf("Enter starvation prevention time (W): ");
        if (scanf("%u", W) && *W > 0)
            break;
        printf("ERROR: Enter a positive integer\n");
        while (getchar() != '\n')
            ;
    }
}

/*
DESCRIPTION:
    - Compares `a` and `b` and returns whichever one is smaller.

RETURN:
    + The smaller integer out of the two.
*/
int min(int a, int b)
{
    return (a < b) ? a : b;
}

/*
DESCRIPTION:
    - Compares `a` and `b` and returns whichever one is greater.

RETURN:
    + The greater integer out of the two.
*/
int max(int a, int b)
{
    return (a > b) ? a : b;
}

/*
DESCRIPTION:
    - Checks whether the current process exists. If not, it will take the first
    element in the `queue` and run it. If the current process is not the one in
    the current `queue` then we still run it anyway as we assume the order is
    maintained.

RETURN:
    + Nothing. However, it does change the state of `current_process`. It switch-
    es to something else.
*/
void checkAndRunProcess(Block **current_process, Block *queue, uint64_t timer)
{
    if (!(*current_process))
    {
        /*
        NOTE:
            - If nothing, we just run normally.
        */
        (*current_process) = queue;

        if ((*current_process)->status == PCB_INITIALIZED)
        {
            startBlock(*current_process);
            metrics.total_response += (timer -
                                         (*current_process)->arrival_time);
        }
        else
        {
            resumeBlock(*current_process);
        }
    }
    else if ((*current_process) != queue)
    {
        /*
        NOTE:
            - If the `current_process` is not the first process in the current
            `queue` then we suspend the currently running process.

            -  This function would always take greater precedence (that's the
            assumption, at least).
        */
        suspendBlock(*current_process);
        *current_process = queue;

        if ((*current_process)->status == PCB_INITIALIZED)
        {
            startBlock(*current_process);
            metrics.total_response += (timer -
                                         (*current_process)->arrival_time);
        }
        else
        {
            resumeBlock(*current_process);
        }
    }
}

/*
DESCRIPTION:
    - Checks whether the currently running process has reached its time quantum.
    Demote if it has, and does all these menial dequeue and enqueue stuff.

    - The parameter `from` is the queue to dequeue from and `to` is the queue to
    enqueue to.

RETURN:
    + TRUE if has equalled or exceeded the time quantum.
    + FALSE if not the case.
*/
char checkAndDemote(Block **current_process, int quantum, Block **from, Block **to,
                    int new_priority, uint64_t timer)
{

    if ((*current_process)->cycle_time >= quantum)
    {
        (*current_process)->priority = new_priority;

        /*
        NOTE:
            - Resetting the cycle clock and doing all the dequeueing and enque-
            ueing while suspending the process.
        */
        (*current_process)->cycle_time = 0;

        suspendBlock(*current_process);
        Block *dequeued = dequeueBlock(from);
        dequeued->last_queued = timer;
        *to = enqueueBlock(*to, dequeued);

        *current_process = NULL;

        return TRUE;
    }

    return FALSE;
}

/*
DESCRIPTION:
    - Checks whether the currently running process has reached its time quantum.
    This is a special case of demotion used for the last level queue where we
    just put whatever is in front of the queue to the end of the same queue.

RETURN:
    + TRUE if has equalled or exceeded the time quantum.
    + FALSE if not the case.
*/
char checkAndRequeue(Block **current_process, int quantum, Block **queue,
                     uint64_t timer)
{
    if ((*current_process)->cycle_time >= quantum)
    {
        (*current_process)->cycle_time = 0;

        /*
        NOTE:
            - From the same queue. I reckon we could've just reused the previous
            function instead. But this one is safe.
        */
        suspendBlock(*current_process);
        Block *dequeued = dequeueBlock(queue);
        dequeued->last_queued = timer;
        *queue = enqueueBlock(*queue, dequeued);

        *current_process = NULL;

        return TRUE;
    }

    return FALSE;
}

/*
DESCRIPTION:
    - Checks whether the currently running process has completed. We terminate
    the job and delete it from existence.

RETURN:
    + TRUE if job has finished.
    + FALSE if not the case.
*/
char checkAndTerminate(Block **current_process, Block **from, uint64_t timer)
{
    if ((*current_process)->remaining_cpu_time <= 0)
    {
        Block *dequeued = dequeueBlock(from);
        metrics.total_turnaround += (timer - dequeued->arrival_time);
        terminateBlock(*current_process);

        /*
        NOTE:
            - Freeing to avoid memory leaks and making the pointer to the curr-
            ently running process NULL because that's good practice.
        */
        free(*current_process);
        *current_process = NULL;

        return TRUE;
    }

    return FALSE;
}

/*
DESCRIPTION:
    - Checks for starvation using last_queued timestamp and promotes processes
    to L-0 if they've been queued for too long. Handles both L-1 and L-2 star-
    vation cases.

RETURNS:
    + Nothing. Changes its parameters, though.
*/

void checkAndHandleStarvation(Block **zero, Block **one, Block **two,
                              uint64_t timer, unsigned int W)
{
    /*
    NOTE:
        - Checking level-1 queue jobs.
    */
    if (*one && (timer - (*one)->last_queued - (*one)->cycle_time >= W))
    {
        /*
        NOTE:
            - Moving all level-1 queue jobs.
        */
        while (*one)
        {
            Block *process = dequeueBlock(one);
            process->priority = PCB_PRIORITY_0;
            process->last_queued = timer;
            *zero = enqueueBlock(*zero, process);
        }

        /*
        NOTE:
            - Moving all level-2 queue jobs.
        */
        while (*two)
        {
            Block *process = dequeueBlock(two);
            process->priority = PCB_PRIORITY_0;
            process->last_queued = timer;
            *zero = enqueueBlock(*zero, process);
        }
    }
    /*
    NOTE:
        - Checking level-2 queue jobs.
    */
    else if (*two && (timer - (*two)->last_queued - (*two)->cycle_time >= W))
    {
        /*
        NOTE:
            - Moving level-2 queue jobs.
        */
        while (*two)
        {
            Block *process = dequeueBlock(two);
            process->priority = PCB_PRIORITY_0;
            process->last_queued = timer;
            *zero = enqueueBlock(*zero, process);
        }
    }
}

/*
DESCRIPTION:
    - Simulates a CPU cycle. Updates the timer and pretend to sleep for a CPU
    cycle. Also updates the current process's allotted cycle time and its re-
    quired remaining time.

RETURN:
    + Nothing. But the pointer arguments passed into the function does change
    their states.
*/
void updateCycle(Block **current_process, uint64_t *timer)
{
    sleep(UNIT_CPU_TIME_SIM);
    (*timer)++;

    (*current_process)->cycle_time++;
    (*current_process)->remaining_cpu_time--;
}
#endif

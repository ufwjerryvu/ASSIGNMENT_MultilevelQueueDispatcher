#include <disp.h>

int main(int argc, char *argv[])
{
    /*
    NOTE:
        - Queue declarations and initializations. Along with other miscellaneous
        declarations.
    */
    Block *jobs = NULL, *zero = NULL, *one = NULL, *two = NULL;
    Block *current_process = NULL;
    Block *process = NULL;
    uint64_t n = 0;
    uint64_t timer = 0;

    unsigned int t0, t1, t2, W;
    unsigned int w1 = 0, w2 = 0;

    /*
    SECTION 1: ARGUMENT CHECKING
    */
    if (argc <= 0)
    {
        fprintf(stderr, "FATAL: Bad arguments array\n");
        exit(EXIT_FAILURE);
    }
    else if (argc != ARGS_EXACT_COUNT)
    {
        fprintf(stderr, "USAGE: %s <TESTFILE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /*
    SECTION 2: USER INPUT
    */
    getUserInput(&t0, &t1, &t2, &W);

    /*
    SECTION 3: JOB DISPATCH QUEUE (JDQ) INITIALIZATION
    */
    jobs = initializeJobDispatchQueue(argv[ARGS_JOBS_FILENAME]);
    if (!jobs)
    {
        fprintf(stderr, "ERROR: Could not open \"%s\"\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    printf("\n");
    metrics.completed_jobs = countTotalJobs(jobs);

    /*
    SECTION 4: OS DISPATCHER/SCHEDULER
    */
    while (TRUE)
    {
        /*
        NOTE:
            - There are still jobs in the JDQ.
        */
        if (countTotalJobs(jobs))
        {
            /*
            NOTE:
                - Putting the first job in the JDQ where it belongs if it's time
                for it to arrive.
            */
            while (jobs && timer >= jobs->arrival_time)
            {
                /*
                NOTE:
                    - When dequeueing blocks, we need to pass in the address of
                    the pointer `jobs`. This will effectively modify the value
                    of `jobs` itself. If there's nothing left in the queue, the
                    dequeue function will set `jobs` to NULL.
                */
                Block *dequeued = dequeueBlock(&jobs);
                dequeued->last_queued = timer;
                switch (dequeued->priority)
                {
                case PCB_PRIORITY_0:
                    zero = enqueueBlock(zero, dequeued);
                    break;
                case PCB_PRIORITY_1:
                    one = enqueueBlock(one, dequeued);
                    break;
                case PCB_PRIORITY_2:
                    two = enqueueBlock(two, dequeued);
                    break;
                default:
                    /*
                    NOTE:
                        - Re-enqueue if jobs cannot be categorized. Hopefully this
                        doesn't happen.
                    */
                    jobs = enqueueBlock(jobs, dequeued);
                    break;
                }
            }

            /*
            NOTE:
                - If nothing are in the other queues then we just idle wait for
                processes to come while increasing the timer.
            */
            if (!countTotalJobs(zero) && !countTotalJobs(one) &&
                !countTotalJobs(two))
            {
                /*
                NOTE:
                    - Increase the timer.
                */
                timer++;
                sleep(UNIT_CPU_TIME_SIM);
                continue;
            }
        }

        checkAndHandleStarvation(&zero, &one, &two, timer, W);

        /*
        NOTE:
            - Handling level-0 queue.
        */
        if (countTotalJobs(zero))
        {
            checkAndRunProcess(&current_process, zero, timer);
            updateCycle(&current_process, &timer);

            if (!checkAndTerminate(&current_process, &zero, timer))
            {
                checkAndDemote(&current_process, t0, &zero, &one, PCB_PRIORITY_1,
                    timer);
            }

            continue;
        }

        /*
        NOTE:
            - Handling level-1 queue.
        */
        if (countTotalJobs(one))
        {
            checkAndRunProcess(&current_process, one, timer);
            updateCycle(&current_process, &timer);

            if (!checkAndTerminate(&current_process, &one, timer))
            {
                checkAndDemote(&current_process, t1, &one, &two, PCB_PRIORITY_2,
                    timer);
            }

            continue;
        }

        /*
        NOTE:
            - Handling level-2 queue.
        */
        if (countTotalJobs(two))
        {
            checkAndRunProcess(&current_process, two, timer);
            updateCycle(&current_process, &timer);

            if (!checkAndTerminate(&current_process, &two, timer))
            {
                checkAndRequeue(&current_process, t2, &two, timer);
            }
            
            continue;
        }

        break;
    }

    printf("Average turnaround time: %f\n", ((float)metrics.total_turnaround / (float)metrics.completed_jobs));
    printf("Average response time: %f\n", ((float)metrics.total_response / (float)metrics.completed_jobs));
}

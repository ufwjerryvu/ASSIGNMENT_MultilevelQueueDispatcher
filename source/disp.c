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
            queueFromDispatch(&jobs, &zero, &one, &two, timer);
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
                queueFromDispatch(&jobs, &zero, &one, &two, timer);
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
                queueFromDispatch(&jobs, &zero, &one, &two, timer);
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
                queueFromDispatch(&jobs, &zero, &one, &two, timer);
                checkAndDemote(&current_process, t2, &two, &two, PCB_PRIORITY_2,
                               timer);
            }

            continue;
        }

        break;
    }

    printf("Average turnaround time: %.3f\n", ((float)metrics.total_turnaround / ((float)metrics.completed_jobs)));
    printf("Average waiting time: %.3f\n", ((float)metrics.total_waiting / (float)metrics.completed_jobs));
    printf("Average response time: %.3f\n", ((float)metrics.total_response / (float)metrics.completed_jobs));
}

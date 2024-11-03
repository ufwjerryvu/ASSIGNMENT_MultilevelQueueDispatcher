#include <disp.h>

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

    Metrics metrics;
    unsigned int t0, t1, t2, W;

    /*
    SECTION X: ARGUMENT CHECKING
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
    SECTION X: USER INPUT
    */
    getUserInput(&t0, &t1, &t2, &W);

    /*
    SECTION X: JOB DISPATCH QUEUE (JDQ) INITIALIZATION
    */
    jobs = initializeJobDispatchQueue(argv[ARGS_JOBS_FILENAME]);
    if (!jobs)
    {
        fprintf(stderr, "ERROR: Could not open \"%s\"\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    n = countTotalJobs(jobs);

    /*
    SECTION X: OS DISPATCHER/SCHEDULER
    */
    while (TRUE)
    {
        /*
        NOTE:
            - There are still jobs in the JDQ.
        */
        if (countTotalJobs(jobs))
        {
            printQueue(jobs);
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

        /*
        NOTE:
            - Handling level-0 queue.
        */
        if (countTotalJobs(zero))
        {
            /*
            NOTE:
                - If the current process doesn't already exist or was previously
                set to NULL, get the first level zero job and start running the 
                process. 
            */
            if(!current_process){
                current_process = dequeueBlock(&zero);

                if(current_process->status == PCB_INITIALIZED){
                    startBlock(current_process);
                }else{
                    resumeBlock(current_process);
                }
            }

            /*
            NOTE:
                - Get how long the calling process should sleep for to let the
                subprocess keep running.
            */
            int sleep_time = min(t0, current_process->remaining_cpu_time);
            sleep(UNIT_CPU_TIME_SIM * sleep_time);

            /*
            NOTE:
                - Update the timer with the time that the scheduler has slept
                for. 
            */
            timer += sleep_time;

            current_process->remaining_cpu_time -= sleep_time;

            /*
            NOTE:
                - If there is not required CPU time left then we terminate the
                process.
            */
            if (current_process->remaining_cpu_time <= 0)
            {
                terminateBlock(current_process);
                free(current_process);
            }else{
                /*
                NOTE:
                    - Demotion process includes setting the priority, suspending
                    the block, and enqueue-ing it into the next demotion queue.
                */
                current_process->priority = PCB_PRIORITY_1;
                suspendBlock(current_process);
                one = enqueueBlock(one, current_process);
            }

            current_process = NULL;

            continue;
        }
        
        break;
    }
}

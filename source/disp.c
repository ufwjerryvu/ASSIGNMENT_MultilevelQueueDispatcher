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

int main(int argc, char *argv[])
{
    /*
    NOTE:
        - Queue declarations and initializations. Along with other miscellaneous
        declarations.
    */
    Block *jobs, *zero, *one, *two;
    Block *current_process = NULL;
    Block *process = NULL;
    uint64_t n = 0;
    uint64_t timer = 0;
    unsigned int t0, t1, t2, W;

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
        fprintf(stderr, "Usage: %s <TESTFILE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /*
    SECTION 2: JOB DISPATCH QUEUE (JDQ) INITIALIZATION
    */
    jobs = initializeJobDispatchQueue(argv[ARGS_JOBS_FILENAME]);
    if (!jobs)
    {
        fprintf(stderr, "ERROR: Could not open \"%s\"\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    n = countTotalJobs(jobs);

    // Delete later
    startBlock(jobs);
    sleep(1);
    suspendBlock(jobs);
    sleep(5);
    resumeBlock(jobs);
    sleep(4);
    terminateBlock(jobs);
    return 0;

    /*
    SECTION 3: OS DISPATCHER/SCHEDULER
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
                Block* dequeued = dequeueBlock(&jobs);
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
            if(!countTotalJobs(zero) && !countTotalJobs(one) && 
                !countTotalJobs(two)){
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
            - There are still jobs in the L0 queue.
        */
        if (countTotalJobs(zero))
        {
            Block* dequeued = dequeueBlock(&zero);
            
        }

        /*
        NOTE:
            - There are still jobs in the L1 queue.
        */
        if (countTotalJobs(one))
        {

        }

        /*
        NOTE:
            - There are still jobs in the L2 queue.
        */
        if (countTotalJobs(two))
        {

        }

        break;
    }
}

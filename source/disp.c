#include <disp.h>

/*
DESCRIPTION:
    - Reads the job dispatch queue from the jobs file and stores it in a queue.

RETURNS:
    + Block* which is the head of a newly initialized queue.
    + NULL if file is unable to be read.
*/
Block* initializeJobDispatchQueue(char *filename){
    FILE* file = fopen(filename, "r");
    
    if(!file){
        return NULL;
    }
    
    Block* jobs = NULL;
    Block* process = NULL;
    while(!feof(file)){
        process = createNullBlock();

        /*
        NOTE:
            - Read in the arguments and if the arguments don't match then we 
            have a problem. Free the block and go to the next input.
        */
        if(fscanf(file, "%d, %d, %d", &(process->arrival_time), 
            &(process->service_time), &(process->priority)) != JOBS_SPLIT_COUNT
        ){
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
uint64_t countTotalJobs(Block* head){
    if(!head){
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
void printQueue(Block* head){
    Block* current = head;
    printBlockHeader();

    while(current){
        printBlock(current);
        current = current->next;
    }
}

int main (int argc, char *argv[])
{
    /*
    NOTE:
        - Queue declarations and initializations. Along with other miscellaneous
        declarations.
    */
    Block* jobs, zero, one, two;
    Block* current_process = NULL;
    Block* process = NULL;
    uint64_t n = 0;
    uint64_t timer = 0;

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
    printQueue(jobs);
}

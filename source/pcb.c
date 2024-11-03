#include <pcb.h>

/*
DESCRIPTION:
    - Creates an inactive block. Initializes everything to default values which
    are not usable unless reassigned.

RETURNS:
    + Block* of newly initilaized Block.
    + NULL if failed at allocating memory.
*/
Block *createNullBlock()
{
    Block *block;
    if (!(block = (Block *)malloc(sizeof(Block))))
    {
        fprintf(stderr, "ERROR: Could not create new process control block\n");
        return NULL;
    }
    block->pid = 0;
    block->args[PCB_ARGS_PNAME] = "./process";
    block->args[PCB_ARGS_ENDNULL] = NULL;

    /*
    NOTE:
        - Everything is initialized to zero at the very beginning but we need
        to rework the values later on when we're working with them.
    */
    block->arrival_time = 0;
    block->service_time = 0;
    block->remaining_cpu_time = 0;
    block->last_active_time = 0;
    block->cycle_time = 0;
    block->priority = 0;

    /*
    NOTE:
        - Block status is defined in the macros section in the header file of
        the same name.
    */
    block->priority = PCB_DEFAULT_PRIORITY;
    block->status = PCB_UNINITIALIZED;
    block->next = NULL;

    return block;
}

/*
DESCRIPTION:
    - Queues process (or join queues at the end of the queue). The value `q` is
    the head of the queue and `p` is the process. Everything is in a linked list
    type of data structure.

RETURNS:
    + Pointer to the head of the queue.
*/
Block *enqueueBlock(Block *q, Block *p)
{
    Block *h = q;

    p->next = NULL;

    if (q)
    {
        while (q->next)
            q = q->next;
        q->next = p;
        return h;
    }

    return p;
}

/*
DESCRIPTION:
    - Dequeues the process. This takes a block from the head of the queue and
    the input takes in the pointer to the head of the queue which is denoted
    as `h`. It also sets new head of the queue.

RETURNS:
    + Block* if successfully dequeued.
    + NULL if queue was empty.
*/
Block *dequeueBlock(Block **h)
{
    Block *p;

    if (h && (p = *h))
    {
        *h = p->next;
        return p;
    }

    return NULL;
}

/*
DESCRIPTION:
    - Starts or restarts a process based on the input block `p` that is provided
    as the argument to the function.

RETURNS:
    + Block* of the process.
    + NULL if start/restart has failed.
*/
Block *startBlock(Block *p)
{
    if (!p->pid)
    {
        /*
        NOTE:
            - If the process has not yet been started so we need to fork the
            process and start it
        */
        p->pid = fork();
        if(p->pid > 0){
            /*
            NOTE:
                - We are in the parent process, we simply do nothing. Let it
                break out of the loop naturally.
            */
        }else if(p->pid < 0){
            fprintf(stderr, "FATAL: Could not fork process!\n");
            exit(EXIT_FAILURE);
        }else{
            /*
            NOTE:
                - We are now in the child process if the process ID is a zero.
                This is again defined in the macros section.
            */
            p->pid = getpid();
            p->status = PCB_RUNNING;

            /*
            NOTE:
                - Print out the block header before we replace it completely w-
                ith a different process.
            */
            printBlockHeader();
            printBlock(p);
            fflush(stdout);

            execv(p->args[PCB_ARGS_PNAME], p->args);
            fprintf(stderr, "ALERT: You should never see me!\n");

            exit(EXIT_FAILURE);
        }
        
    }
    else 
    {
        /*
        NOTE:
            - It's already started so just let it continue and we send a SIGCONT
            signal to notify the child process of that.
        */
        kill(p->pid, SIGCONT);
    }

    p->status = PCB_RUNNING;

    return p;
}

/*
DESCRIPTION:
    - Terminates a block or a process. Sends a kill() signal to the process with
    the given process ID.

RETURNS:
    + Block* of the process.
    + NULL if termination failed.
*/
Block *terminateBlock(Block *p)
{
    int status;

    if (!p)
    {
        fprintf(stderr, "ERROR: Cannot terminate a NULL process\n");
        return NULL;
    }
    else
    {
        kill(p->pid, SIGINT);
        waitpid(p->pid, &status, WUNTRACED);
        p->status = PCB_TERMINATED;
        return p;
    }
}

/*
DESCRIPTION:
    - Resumes the block from the suspended state using SIGCONT. 

RETURNS:
    + Block* of the block that was resumed.
    + NULL if couldn't resume? Assuming the process pointer is already NULL.
*/
Block *resumeBlock(Block *p){
    if(!p){
        fprintf(stderr, "ERROR: Cannot resume a NULL process\n");
        return NULL;
    }else{
        p->status = PCB_RUNNING;
        printBlockHeader();
        printBlock(p);
        kill(p->pid, SIGCONT);
    }

    return p;
}

/*
DESCRIPTION:
    - Suspends/pauses a block

RETURNS:
    + Block* of the block that was resumed.
    + NULL if couldn't resume?
*/
Block *suspendBlock(Block *p){
    int status;
    if(!p){
        fprintf(stderr, "ERROR: Cannot suspend a NULL process\n");
        return NULL;
    }else{
        kill(p->pid, SIGTSTP);
        waitpid(p->pid, &status, WUNTRACED);
        p->status = PCB_SUSPENDED;
    }

    return p;
}

/*
DESCRIPTION:
    - Prints process attributes to standard output.

RETURNS:
    + Block* of the process.
*/
Block *printBlock(Block *p)
{
    printf("%7d%7d%9d%11d%13d%10d    ",
           (int)p->pid, p->arrival_time, p->service_time,
           p->remaining_cpu_time, p->last_active_time, p->priority);

    switch (p->status)
    {
    /*
    NOTE:
        - Just printing out the status of the process control block based on the
        macros defined for these PCBs.
    */
    case PCB_UNINITIALIZED:
        printf("UNINITIALIZED");
        break;
    case PCB_INITIALIZED:
        printf("INITIALIZED");
        break;
    case PCB_READY:
        printf("READY");
        break;
    case PCB_RUNNING:
        printf("RUNNING");
        break;
    case PCB_SUSPENDED:
        printf("SUSPENDED");
        break;
    case PCB_TERMINATED:
        printf("TERMINATED");
        break;
    default:
        printf("UNKNOWN");
    }
    printf("\n");

    return p;
}

/*
DESCRIPTION:
    - Prints the header. This goes hand-in-hand with the function `printBlock()`
    as it provides the header for readability.

RETURNS:
    + Nothing.
*/
void printBlockHeader()
{
    printf("    pid arrive  service  cpuremain  last_active  priority   status\n");
}

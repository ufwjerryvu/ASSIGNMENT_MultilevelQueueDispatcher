#ifndef PCB
#define PCB

/*
SECTION 1A: C STANDARD LIBRARY INCLUDES
*/
#include <stdio.h>
#include <stdlib.h>

/*
SECTION 1B: SYSTEM CALL HEADER FILES
*/
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

/*
SECTION 2: AUXILIARY MACROS
*/
#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif

/*
SECTION 3: PROCESS MANAGEMENT MACROS
*/
#define PCB_UNINITIALIZED (0)
#define PCB_INITIALIZED (1)
#define PCB_READY (2)
#define PCB_RUNNING (3)
#define PCB_SUSPENDED (4)
#define PCB_TERMINATED (5)
#define PCB_MAX_ARGS (3)
#define PCB_ARGS_PNAME (0)
#define PCB_ARGS_ENDNULL (1)

#define PCB_DEFAULT_PRIORITY (-1)
#define PCB_PRIORITY_0 (0)
#define PCB_PRIORITY_1 (1)
#define PCB_PRIORITY_2 (2)

/*
SECTION 4: PROCESS CONTROL BLOCK STRUCTURE
*/
struct Process
{
    pid_t pid;

    char *args[PCB_MAX_ARGS];
    int arrival_time;
    int service_time;
    int remaining_cpu_time;
    int last_active_time;
    
    int priority;
    int status;

    struct Process *next;
};

typedef struct Process Block;

/*
SECTION 5: FUNCTION PROTOTYPES
*/
Block *createNullBlock();
Block *enqueueBlock(Block *, Block *);
Block *dequeueBlock(Block **);
Block *startBlock(Block *);
Block *terminateBlock(Block *);
Block *resumeBlock(Block *);
Block *suspendBlock(Block *);
Block *printBlock(Block *);
void printBlockHeader(void);

#endif

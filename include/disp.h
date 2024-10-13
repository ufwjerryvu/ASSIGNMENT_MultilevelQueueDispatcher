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
SECTION 4: FUNCTION PROTOTYPES
*/
Block* initializeJobDispatchQueue(char *filename);
uint64_t countTotalJobs(Block* head);
void printQueue(Block* head);
int updateLevelQueues(Block** jdq, Block* zero, Block* one, Block* two);

#endif

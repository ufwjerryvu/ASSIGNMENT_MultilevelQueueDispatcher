CC=gcc
SRC_DIR=source
INCL_DIR=include
AUX_DIR=auxiliary
TESTS_DIR=tests
IN_FILE_NO=1

SRC_FILES=$(SRC_DIR)/pcb.c $(SRC_DIR)/disp.c

# Compiles and does everything except for running and cleaning
all: CompileProcess CompileDispatcher GenerateRandom

# Compiles the signal trapping process.
CompileProcess:
	$(CC) $(AUX_DIR)/sigtrap.c -o process

# Compiles the dispatcher (our main program)
CompileDispatcher:
	$(CC) -I$(INCL_DIR) -c $(SRC_FILES)
	$(CC) *.o -o dispatcher

# Executes the dispatcher program under the default jobs file
ExecuteProgram: all
	./dispatcher jobs.txt

# Generates random jobs list based on the numbered inputs
GenerateRandom:
	$(CC) $(SRC_DIR)/random.c -lm -o random
	cat $(TESTS_DIR)/params-$(IN_FILE_NO).in | ./random jobs.txt > /dev/null

# Cleans all generated files
CleanAll: CleanObjs CleanBins CleanJobs
	clear

# Cleans object files
CleanObjs:
	rm -rf *.o

# Cleans all binary files
CleanBins:
	rm -rf *.o dispatcher process random 

# Cleans just the jobs file
CleanJobs:
	rm -rf *.o jobs.txt
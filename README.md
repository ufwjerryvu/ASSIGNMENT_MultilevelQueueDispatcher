# Multi-level Queue Dispatcher
The University of Sydney

COMP3520 2024S2

SID: 510156033

## Overview
This program implements a Multi-Level Queue (MLQ) dispatcher for process scheduling with three priority queues. The implementation includes features such as:

- Three priority levels (0-2, with 0 being highest priority)
- Configurable time quantum for each level
- Starvation prevention mechanism
- Process preemption based on priority
- Metrics

##  How to run the program
### Compilation of dispatcher and signal trapper
Make sure you are in the base directory after you extract (i.e., the directory containing the Makefile). 

This program uses a standard Makefile and requires:
- GCC compiler
- C standard libraries.
- Linux operating system
- POSIX-compliant system

The make command below compiles both the signal trapping program (`sigtrap.c`) and our dispatcher.

```
make
```

After calling this command, there should be a binary executable `process` and `dispatcher` in the base directory of this project. 

If you want to compile just the signal trapper:
```
make CompileProcess
```

If you want to compile just the dispatcher:
```
make CompileDispatcher
```

### Compilation and generation of random jobs
Again, make sure you are in the base directory after you extract (i.e., the directory containing the Makefile).

The prequisites are exactly as before. Now, to compile and generate random jobs, just run:

```
make GenerateRandom
```

This will generate a text file called `jobs.txt` in the base directory that is random everytime. Note that this will overwrite the file contents. 

### To run the dispatcher
Make sure you have called `make` with two binary files `process` and `dispatcher` in the base directory and also make sure you have a jobs file `<jobs_file>` which contain one job per line in the format:
```
<arrival_time>, <cpu_time>, <priority>
```

Example:
```
0, 3, 0
2, 8, 2
4, 4, 0
```

The command to run the program is:

```
./dispatcher <jobs_file>
```


Example usage (assuming current directory is the base directory containing the Makefile after calling `make`):
```
./dispatcher jobs.txt
```
## 

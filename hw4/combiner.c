#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

/*
 * ##########################################################
 *                          DEFINES
 * ##########################################################
 */

/*
 * ##########################################################
 *                        PROTOTYPES
 * ##########################################################
 */

void mapper(void);
void reducer(int idx);

/*
 * ##########################################################
 *                         PROCESSES
 * ##########################################################
 */

/*
 * SUMMARY: combiner.c
 * This program maps input tuples from stdin in mapper process to the 
 * reducer process. The mmap'd buffer is protected using an mmap'd binary
 * semaphore.
 */ 

int main(int argc, char **argv)
{
  int bufSize;
  int numWorkers;
  int bufIndex = 0;

  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  // turn off stdout/stdin buffers
  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+

  setbuf(stdout, NULL);
  setbuf(stdin, NULL);

  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  // handle command line arguments
  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+

  if (argc > 3) // check that there's an appropriate # arguments
  {
    printf("ERROR: Expecting 3 command line arguments (./combiner bufSize numWorkers)\n");
  }

  if ((bufSize = atoi(argv[1])) == 0) // check that 2nd param is integer
  {
    printf("ERROR: Second input argument (integer) NumWorkers.\n");
    return -1;
  }

  if ((numWorkers = atoi(argv[2])) == 0) // check that 2nd param is integer
  {
    printf("ERROR: Second input argument (integer) NumWorkers.\n");
    return -1;
  }

  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  // start all worker threads
  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+

  for (int i = 0; i < numWorkers; i++)
  {
    pid_t pid = fork();

    // fork error
    if (pid == -1)
    {
	    printf("ERROR: Fork can't produce child.."); 
      wait(NULL);
      exit(0);
    }

    // child (tasks - reducer)
    else if (pid == 0)
    {
      reducer(bufIndex++);
      return 0;
    }

    // parent (combiner - child manager)
    else
    {
      continue;
    }
  }

  mapper(); // parent maps inputs to worker threads
  
  // wait for all children to finish before exitting.
  for (int i = 0; i < numWorkers; i++)
    wait(NULL);

  return 0;
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 * SUMMARY: mapper (main)
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

void mapper(void)
{
  printf("Mapper\n");
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 * SUMMARY: reducer (worker)
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

void reducer(int idx)
{
  printf("Reducer %d\n", idx);
}
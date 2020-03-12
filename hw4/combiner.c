/*
 * ##########################################################
 *                          INCLUDES
 * ##########################################################
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>

#include "fifo.h"

/*
 * ##########################################################
 *                         GLOBALS
 * ##########################################################
 */

int bufSize;
int numWorkers;
reducer_tuple_fifo_t* fifo;

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
  // create the shared buffer using an mmap.
  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+

  // The fifo data structure wraps the tuple-matrix and controls the 
  // mutex locks for all channels (numWorkers).
  fifo = Fifo(numWorkers, bufSize);

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
      exit(0);
    }

    // child (tasks - reducer)
    else if (pid == 0)
    {
      reducer(bufIndex); // define which buffer child will use
      _exit(0);
    }

    // parent (combiner - child manager)
    else
    {
      bufIndex++; 
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
  printf("Mapper - writing fifo\n");

  reducer_tuple_in_t tx;
  tx.topic[0] = '1';
  tx.userid[0] = '2';
  for (int i = 0; i < 15; i++)
  {
    tx.weight = i;
    while (fifo->write(fifo, i&7, &tx) == -1);

    printf("Writing to channel %d: %d\n", i&7, i);
  }
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 * SUMMARY: reducer (worker)
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */


void reducer(int idx)
{
  printf("Reducer %d - reading fifo\n", idx);

  int count = 0;
  while(1)
  {
    reducer_tuple_in_t* rx = fifo->read(fifo, idx);
    if (rx == NULL)
    {
      count++;
      if (count == 100000)
        return;
    }
    else
    { 
      count = 0;
      printf("Reading from channel %d: %d\n", idx, rx->weight);
    }
  }
}

/*
 * ##########################################################
 *                        FUNCTIONS
 * ##########################################################
 */

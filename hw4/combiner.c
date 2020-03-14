/*
 * ##########################################################
 *                          INCLUDES
 * ##########################################################
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>

#include "fifo.h"
#include "mapper.h"
#include "reducer.h"

/*
 * ##########################################################
 *                        STRUCTS
 * ##########################################################
*/

// This structure will be used by all running processes to 
// determine if the mapper has completed.
typedef struct condition
{
  pthread_mutex_t mutex;
  int flag;
} condition_t;

/*
 * ##########################################################
 *                         GLOBALS
 * ##########################################################
*/

int bufSize;
int numWorkers;
reducer_tuple_fifo_t* fifo;
void* areaCondition;
condition_t* cond;

/*
 * ##########################################################
 *                        PROTOTYPES
 * ##########################################################
*/

void mapper(void);
void reducer(int idx);
int delay(int secs);

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
  // create the shared buffer / condition using an mmap.
  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+

  // The fifo data structure wraps the tuple-matrix and controls the 
  // mutex locks for all channels (numWorkers).
  fifo = Fifo(numWorkers, bufSize);

  // init mmap'd condition for signaling reducers that mapper is complete
  areaCondition = mmap(NULL, sizeof(condition_t), PROT_AREA, MAP_AREA, -1, 0);
  cond = (condition_t*)areaCondition;

  // initialize the mmap'd condition variable before starting all processes
  cond->flag = 0;
  pthread_mutex_init(&cond->mutex, NULL);

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

  mapper(); // parent maps inputs to worker threads.

  // tell the reducers that the mapper is complete.
  pthread_mutex_lock(&cond->mutex);
  cond->flag = 1;
  pthread_mutex_unlock(&cond->mutex);

  printf("MAPPER WAITING FOR CHILDREN!\n");
  
  // wait for all children to finish before exitting.
  for (int i = 0; i < numWorkers; i++)
    wait(NULL);

  printf("ALL CHILDREN DONE!\n");
  exit(0);
  return 0;
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 * SUMMARY: mapper (main)
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/
void mapper(void)
{
  // define local variables
  mapper_tuple_in_t* tupleIn;
  reducer_tuple_in_t* tupleOut;

  // read in + map more tuples until stdin is empty
  while((tupleIn = mapper_read_tuple()) != NULL)
  {
    // printf("Tuple read from input.txt: %.4s - %c - %.15s\n", &tupleIn->userid[0], tupleIn->action, &tupleIn->topic[0]);
    // Map the input tuple to work with the reducer processes
    if ((tupleOut = map(tupleIn)) == NULL)
      printf("Error mapping tuple to reducer format.\n");

    fifo->writeUser(tupleOut->userid, tupleOut);
    free(tupleOut);
    delay(1000);
  }
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 * SUMMARY: reducer (worker)
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/
void reducer(int idx)
{
  int mapperCompleteFlag = 0;
  reducer_tuple_in_t* rx;

  // reduce tuples from the buffer until the mapper is complete
  // and the buffer is empty.
  while(mapperCompleteFlag != 1 || fifo->_size[idx] > 0)
  {
    // if anything was read from the buffer, perform reduction on it.
    if ((rx = fifo->read(fifo, idx)) != NULL)
    {
      printf("Read on channel %d: %.4s - %.15s - %d\n", idx, rx->userid, rx->topic, rx->weight);
      free(rx);
    }

    // check if the mapper thread is complete whenever the read doesn't
    // return anything.
    else
    {
      pthread_mutex_lock(&cond->mutex);
      mapperCompleteFlag = cond->flag;
      pthread_mutex_unlock(&cond->mutex);
      delay(100000); // delay avoids starvation in mapper thread
    }
  }

  printf("Buffer empty.. closing reducer %d\n", idx);
}

/*
 * ##########################################################
 *                        FUNCTIONS
 * ##########################################################
 */

/*
 * SUMMARY: delay
 * This function performs a software delay until specifed num
 * of seconds.
 *
 * NOTE: return is counter value in attempt to avoid optimized
 * compiler from removing the software delay.
*/ 
int delay (int tics) 
{
  int counter = 0;
  for (int i = 0; i < tics; i++)
  {
    counter++;
  }
  return counter;
}

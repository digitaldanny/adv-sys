#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
   
/*
 * ##########################################################
 *                          DEFINES
 * ##########################################################
 */

 #define FUNC_PTR(name, in_type, out_type)    in_type (*name)(out_type)

/*
 * ##########################################################
 *                         STRUCTS
 * ##########################################################
 */
typedef struct reducer_tuple_in 
{
  char userid[4];     // USERID - 4 digit number
  char topic[15];     // TOPIC - Pad this with space if unused.
  int32_t weight;     // WEIGHT - Value mapped from the input action as defined by the set of rules in 'main' summary.
} reducer_tuple_in_t;

typedef struct reducer_tuple_fifo
{

  // public functions
  FUNC_PTR(read, void, void);
  FUNC_PTR(write, void, void);

  // private parameters
  pthread_mutex_t* _mutex;
  reducer_tuple_in_t* _tuple_matrix;

} reducer_tuple_fifo_t;

/*
 * ##########################################################
 *                         GLOBALS
 * ##########################################################
 */

int bufSize;
int numWorkers;
reducer_tuple_in_t* tupleMatrix;

pthread_mutex_t* mutex;
int* value;

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

  value = (int*)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, -1, 0);
  mutex = (pthread_mutex_t*)mmap(NULL, sizeof(pthread_mutex_t), 
                            PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, -1, 0);

  // All tuple data will be held in this matrix.
  void* areaMatrix = mmap(  NULL, 
                            bufSize*numWorkers*sizeof(reducer_tuple_in_t), 
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANON, 
                            -1, 0);
  tupleMatrix = (reducer_tuple_in_t*)areaMatrix; // matrix[numWorkers][bufSize]

  // The fifo data structure wraps the tuple-matrix and controls the 
  // mutex locks for all channels (numWorkers).

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
      return 0;
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

  // test writing to tuple array
  for (int i = 0; i < numWorkers; i++)
  {
    for (int j = 0; j < bufSize; j++)
    {
      // pthread_mutex_lock(mutex);
      *value = j;
      // pthread_mutex_unlock(mutex);
    }
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

  int32_t testNum;
  
  /*
  // test reading from tuple array
  for (int i = 0; i < numWorkers; i++)
  {
    for (int j = 0; j < bufSize; j++)
    {
      pthread_mutex_lock(mutex);
      testNum = tupleMatrix[j + (i-1)*bufSize].weight;
      pthread_mutex_unlock(mutex);

      printf("Reading number: %d", testNum);
    }
  }
  */
}

/*
 * ##########################################################
 *                        FUNCTIONS
 * ##########################################################
 */

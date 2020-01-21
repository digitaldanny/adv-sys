#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>

#include "common.h"
#include "mapper.h"
#include "reducer.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                      DEFINES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

#define COMBINER_DEBUG_MODE 1
#define PIPE_SIZE           100
#define STDIN_FD            0
#define STDOUT_FD           1
#define NUM_CHILDREN        2

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                      TYPEDEFS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

typedef struct channelMap
{
  char userid[LEN_USER_ID+1];
  int channel;
} channelMap_t;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                      PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

void* mapper(void* dummy);
void* reducer(void* dummy);

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                       GLOBALS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */
 
 pthread_mutex_t mutexStdout = PTHREAD_MUTEX_INITIALIZER;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                    MAIN / PROCESSES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int main(int argc, char **argv)
{
  //int bufSize = atoi(argv[1]);
  //int numRThreads = atoi(argv[2]);
  pthread_t mthread;
  //pthread_t rthread[numRThreads];

  // turn off stdout/stdin buffers
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);

  // start all threads..
  pthread_create(&mthread, NULL, mapper, NULL);
  // for (int i = 0; i < numRThreads; i++)
  //   pthread_create(&rthread[i], NULL, reducer, NULL);
  
  // wait for threads to exit..
  pthread_join(mthread, NULL);
  // for (int i = 0; i < numRThreads; i++)
  //   pthread_join(rthread[i], NULL);

  debugger("MAIN - Exitting..", COMBINER_DEBUG_MODE);
  
  return 0;
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                       THREADS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 * SUMMARY: mapper
 * This function maps an input tuple into a different tuple
 * using the rules defined below. The generated tuple is immediately
 * output to the terminal.
 * 
 * RULES:
 * P = 50, L = 20, D = -10, C = 30, S = 40
 * 
 * EXAMPlE INPUT:   (1111,P,history)
 * EXAMPLE OUTPUT:  (1111,history,50)
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */
void* mapper(void* dummy)
{
  pthread_mutex_lock(&mutexStdout);
  debugger("Starting MAPPER..", COMBINER_DEBUG_MODE);
  pthread_mutex_unlock(&mutexStdout);

  int32_t error = 0;
  int channelNum = 0;

  while (!error)
  {
    // get data from the std input
    mTupleIn_t* inputTuple = m_console_tuple_read();

    // map the data to the output tuple and output
    // map(&inputTuple, &outputTuple);

    // check if the new tuple's userid matches any previous ids..
    // If yes, store to that channel
    // if not, store to a new channel, add to the channelMap, and increment the channel num
    

    // output new tuple to the appropriate channel
  }  

  return NULL;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: reducer
 * This process runs the reducer program and routes the 
 * standard input to pipe.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void* reducer(void* dummy)
{
  pthread_mutex_lock(&mutexStdout);
  debugger("Starting REDUCER..", COMBINER_DEBUG_MODE);
  pthread_mutex_unlock(&mutexStdout);

  node_t * dictionary = NULL;

  // initialize the user ID
  char currId[LEN_USER_ID];
  char prevId[LEN_USER_ID];
  for (uint16_t i = 0; i < LEN_USER_ID; i++)
  {
    currId[i] = 'X';
    prevId[i] = 'Y';
  }

  while(1)
  {
    // reinitialize every iteration so the array start off empty.
    rTupleIn_t inputTuple;

    // read in tuples from standard input
    int32_t error = r_console_tuple_read(&inputTuple);

    // no error in tuple format and has not reached end of the file
    if (!error)
    {
      // update the current user id and check if it's still
      // equal to the previous user id.
      copyUserId(currId, inputTuple.userid);

      // check if this is a new id.. if yes, clear out the dictionary 
      // and reinitialize the data structure
      if (compareUserId(currId, prevId) == -1)
      {
        // display all contents of the hash map as a list of tuples
        r_console_tuple_write(prevId, dictionary);

        // deallocate heap memory for hash map
        dictFreeNodes(dictionary);

        // initialize a new hash map
        dictionary = dict();
      }

      // store the new tuple value in the hash map
      reduce(dictionary, &inputTuple);
    }
    // end of file or error found.. allow the heap memory to be deallocated
    else
    {
      debugger("Reducer end of file found", REDUCER_DEBUG_MODE);
      break;
    }
    // update the previous user id with the new user id
    copyUserId(prevId, currId);

#if REDUCER_DEBUG_MODE == 1
    // display the contents of the dictionary to see if elements are being
    // added correctly.
    dictDisplayContents(dictionary);
#endif
  }

  // final check to make sure dictionary was deallocated before exit
  debugger("Reducer exitting..", REDUCER_DEBUG_MODE);
  r_console_tuple_write(currId, dictionary);
  dictFreeNodes(dictionary);

  return NULL;
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                       FUNCTIONS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */
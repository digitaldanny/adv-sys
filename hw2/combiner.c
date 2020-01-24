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
#include "channel.h"

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
 *                      PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

void* mapper(void* dummy);
void* reducer(void* dummy);
void* dummythread(void* channelNumAddr);

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                       GLOBALS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */
 
 pthread_mutex_t mutexFifo = PTHREAD_MUTEX_INITIALIZER;

 volatile uint8_t flagMapperComplete;
 volatile channel_t * chArray;

 pthread_t mthread;
 pthread_t* rthread;
 int bufSize;
 int numRThreads;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                    MAIN / PROCESSES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int main(int argc, char **argv)
{
  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  // turn off stdout/stdin buffers
  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);

  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  // handle command line arguments
  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  bufSize = atoi(argv[1]);
  numRThreads = atoi(argv[2]);

  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  // initialize local + global variables
  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  flagMapperComplete = 0; // flag for mapper to notify reducer of completion
  rthread = (pthread_t*)malloc(numRThreads*sizeof(pthread_t));

  // initialize channels+buffers for passing tuples to reducers
  chArray = (channel_t*)malloc(numRThreads*sizeof(channel_t));

  for (int i = 0; i < numRThreads; i++)
  {
    if (channel((channel_t*)&chArray[i], bufSize) == -1)
      return 0;
  }

  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  // start all threads..
  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+

  // mallocating channel ID so that the thread can initialize before
  // the for loop number changes
  int * channelId = malloc(numRThreads*sizeof(int));

  pthread_create(&mthread, NULL, mapper, NULL);
  for (int i = 0; i < numRThreads; i++)
  {
    channelId[i] = i;
    pthread_create(&rthread[i], NULL, dummythread, (void*)&channelId[i]);
  }
  
  // Wait for threads to exit.. Mapper waits for all reducers,
  // so main only needs to wait for the mapper thread.
  pthread_join(mthread, NULL);

  debugger("MAIN - Exitting..", COMBINER_DEBUG_MODE);
  
  // after all channel buffers have been read by reducer threads,
  // channels may be deallocated.
  free(rthread);
  free(channelId);
  free((void*)chArray);
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
  debugger("Starting MAPPER..", COMBINER_DEBUG_MODE);

  int maxChannelNum = -1;
  int writeErr = 0;
  channel_t * ch;

  while (1)
  {
    // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
    // ** GET NEW TUPLE VALUE **
    // Get data from the std input and check that it is a valid tuple.
    // If valid, continue with the program.
    // If not valid, set flag to notify reducer threads and exit the program.
    // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
    mTupleIn_t* inputTuple = m_console_tuple_read();
    if (inputTuple == NULL)
    {
      debugger("EOF Found..", COMBINER_DEBUG_MODE);
      break;
    }

    // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
    // ** CHOOSE WHICH CHANNEL TO WRITE TUPLE TO **
    // Check if the current tuple's user id matches any mapper channels
    // If yes, write to that channel.
    // If not, create a new channel with the current userid.
    // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
    for (int i = 0; i < numRThreads; i++)
    {
      // ** This statement will be true if the currently selected channel's userid
      //    matches with the current input tuple.
      ch = (channel_t*)&chArray[i];

      if (strncmp(inputTuple->userid, 
                  ch->userid, 
                  LEN_USER_ID*sizeof(char)) == 0)
      {
        printf("MAPPER - found a matching id: %d\n", i);
        break;
      }

      // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
      // ** If the currently selected channel is greater than the channel count, 
      //    make a new channel, store the tuple values to it, and increase the channel
      //    count.
      // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
      if (i > maxChannelNum)
      {
        printf("MAPPER - creating a new id: %d\n", i);
        maxChannelNum = i;
        ch->set_userid(ch, inputTuple->userid);
        break;
      }
    }

    // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
    // ** MAPPING DATA AND STORING TO CHANNEL QUEUE **
    // map the data to the output tuple and output
    // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
    mTupleOut_t* outTuple = map(inputTuple);

    // try to write to the FIFO until the write is successfull
    do
    {
      usleep(50);
      pthread_mutex_lock(&mutexFifo);
      writeErr = ch->write(ch, outTuple);
      pthread_mutex_unlock(&mutexFifo);
    } while(writeErr < 0);
  }  

  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  // ** FREE ALL MEMORY ALLOCATED DATA **
  // Signal the reducer threads that the mapper thread is complete.
  // Wait for all reducer threads to complete, then deallocate the buffer of tuples.
  // +-----+-----+-----+-----+-----+-----+-----+-----+-----+
  flagMapperComplete = 1;
  for (int i = 0; i < numRThreads; i++)
    pthread_join(rthread[i], NULL);

  return NULL;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: reducer
 * This process runs the reducer program and routes the 
 * standard input to pipe.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void* dummythread(void* channelNumAddr)
{
  int channelNum = *(int*)channelNumAddr;
  channel_t* ch = (channel_t*)&chArray[channelNum];
  mTupleOut_t* tuple = NULL;

  // pthread_mutex_lock(&mutexStdout);
  printf("RED: Channel Number - %d\n", channelNum);
  // pthread_mutex_unlock(&mutexStdout); 

  // read data from the fifo, display topic and value to terminal,
  // then sleep.
  while(flagMapperComplete == 0 || ch->count > 0)
  {
    usleep(100000);
    pthread_mutex_lock(&mutexFifo);
    tuple = ch->read(ch); 
    pthread_mutex_unlock(&mutexFifo);

    // print out tuple data to user if there was valid data
    if (tuple != NULL)
    {
      printf("R: %d - %s\n", channelNum, tuple->userid);
      free(tuple);
    }
  }

  debugger("REDUCER returning..", COMBINER_DEBUG_MODE);
  return NULL;
}

void* reducer(void* dummy)
{
  debugger("Starting REDUCER..", COMBINER_DEBUG_MODE);

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
  r_console_tuple_write(currId, dictionary);
  dictFreeNodes(dictionary);

  debugger("Reducer exitting..", REDUCER_DEBUG_MODE);
  return NULL;
}

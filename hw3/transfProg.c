#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>

#include "accountSearchTree.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                       GLOBALS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

pthread_mutex_t mutexComplete;
int flagComplete;
int numWorkers;
pthread_t rthread;
pthread_t* wthread;
pthread_mutex_t* mutexWorkerBuffer;
transfer_buffer_t* workerBuffer;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                      PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

void* reader(void* fid);
void* worker(void* dummy);
int isReaderComplete();
void markReaderComplete();

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                       THREADS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int main(int argc, char **argv)
{
  FILE* inputFid;
  
  // -------------------------------------------------------
  // turn off stdout/stdin buffers + init tree
  // -------------------------------------------------------

  setbuf(stdout, NULL);
  setbuf(stdin, NULL);
  initAccountLinkedList();

  // -------------------------------------------------------
  // handle command line arguments
  // -------------------------------------------------------

  if ((inputFid = fopen(argv[1], "r")) == NULL)
  {
    printf("ERROR: Opening input file - first argument (string) InputFile.\n");
    return -1;
  }
  if ((numWorkers = atoi(argv[2])) == 0)
  {
    printf("ERROR: Second input argument (integer) NumWorkers.\n");
    return -1;
  }

  // -------------------------------------------------------
  // initialize mutex and buffer connecting the reader thread 
  // and the worker threads.
  // -------------------------------------------------------

  mutexComplete = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  mutexWorkerBuffer = (pthread_mutex_t*)malloc(numWorkers*sizeof(pthread_mutex_t));
  for (int i = 0; i < numWorkers; i++)
    mutexWorkerBuffer[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

  // all worker buffer empty flags will be set to mark that they can be filled immediately.
  workerBuffer = (transfer_buffer_t*)malloc(numWorkers*sizeof(transfer_buffer_t));
  for (int i = 0; i < numWorkers; i++)
  {
    workerBuffer[i].empty = 1;
    workerBuffer[i].src = -1;
    workerBuffer[i].dest = -1;
    workerBuffer[i].amount = -1;
  }

  // initialize worker threads.
  wthread = (pthread_t*)malloc(numWorkers*sizeof(pthread_t));

  // -------------------------------------------------------
  // start threads
  // -------------------------------------------------------

  pthread_create(&rthread, NULL, reader, inputFid);
  for (long long i = 0; i < numWorkers; i++)
    pthread_create(&wthread[i], NULL, worker, (void*)i);

  // -------------------------------------------------------
  // cleanup
  // -------------------------------------------------------

  // wait for threads to complete before cleanup
  pthread_join(rthread, NULL);
  for (int i = 0; i < numWorkers; i++)
    pthread_join(wthread[i], NULL);

  printAccountContents();
  destroyAccountLinkedList();

  // close the input file descriptor
  if (fclose(inputFid) != 0)
  {
    printf("ERROR: Closing input file.\n");
    return -1;
  }

  // printf("Program completed successfully.\n");
  return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: reader
 * This thread reads from the input file and assigns work
 * to an available worker thread. Once the EOF is reached
 * it will signal the worker threads and exit. 
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void* reader(void* inputFid)
{
  FILE* fid = (FILE*)inputFid;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  
  // read all lines of the file and determine if the line is
  // an account number or a transfer..
  // 1) If an account number, allocate space for another account.
  // 2) If a transfer, assign work to the worker threads.
  while ((read = getline(&line, &len, fid)) != -1) 
  {
    // This should be a transfer if the first character is a "T"
    if (line[0] == 'T')
    {
      char* token; 
      char* rest = line; 
      pthread_mutex_t* mutex = NULL;
      transfer_buffer_t* buf = NULL;
      int count = 0;

      // transfer information
      int src = -1;
      int dest = -1;
      int amount = -1;

      // separate each line by the delimiter to be loaded into the buffer.
      while ((token = strtok_r(rest, " ", &rest)) > 0) 
      {
        // Source account number.
        if (count == 1)
        {
          if ((src = atoi(token)) == 0)
          {
            printf("ERROR: (integer) 'source' account number is invalid.\n");
            break;
          }
        }

        // Destination account number.
        else if (count == 2)
        {
          if ((dest = atoi(token)) == 0)
          {
            printf("ERROR: (integer) 'destination' account number is invalid.\n");
            break;
          }
        }

        // Amount transfer.
        else if (count == 3)
        {
          if ((amount = atoi(token)) == 0)
          {
            printf("ERROR: (integer) 'amount' is invalid.\n");
            break;
          }
        }

        // More arguments than expected in this line.
        else if (count > 3)
        {
          printf("ERROR: More than 4 arguments for transfer request.\n");
          break;
        }

        count++;
      }

      // find a channel that is available to write to and claim the mutex.
      // if none are available immediately, continue looping until one is found.
      int foundABuffer = 0;
      do
      {
        for (int i = 0; i < numWorkers; i++)
        {
          mutex = &mutexWorkerBuffer[i];
          buf = &workerBuffer[i];

          // try to claim the lock.. if it is unavailable, check the next lock.
          if (pthread_mutex_lock(mutex) == 0)
          {
            // claimed the lock.. transfer data into the buffer if it is empty.
            if (buf->empty)
            {
              buf->empty = 0;
              buf->amount = amount;
              buf->src = src;
              buf->dest = dest;
              foundABuffer = 1;
              //printf("Txd (%d) - src: %d, dest: %d, amount: %d\n", i, src, dest, amount);
              pthread_mutex_unlock(mutex);
              break;
            }
            pthread_mutex_unlock(mutex);
          }
        }
      } while (!foundABuffer);
    }

    // Initializing an account number + starting balance.
    else
    {
      int account_number = -1;
      int account_balance = -1;
      char* token; 
      char* rest = line; 

      // use this counter to make sure there is only 2 arguments in the 
      // account number definition. (accountNo accountBalance)
      int count = 0;

      // separate each line by the delimiter (space).
      while ((token = strtok_r(rest, " ", &rest)) > 0) 
      {
        // Account number definition.
        if (count == 0)
        {
          if ((account_number = atoi(token)) == 0)
          {
            printf("ERROR: (integer) account number is invalid.\n");
            break;
          }
        }

        // Account balance definition.
        else if (count == 1)
        {
          if ((account_balance = atoi(token)) == 0)
          {
            printf("ERROR: (integer) account number is invalid.\n");
            break;
          }
        }

        // More arguments than expected in this line.
        else
        {
          printf("ERROR: More than 2 arguments for account number definition.\n");
          break;
        }

        count++;
      }

      // If the account details were valid, add it to the account tree.
      if (account_number > 0 && account_balance > 0)
        addAccount(account_number, account_balance);
    }
  }

  // signal the worker threads that there will not be any more input.
  markReaderComplete();
  return NULL;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: worker
 * This thread reads from its assigned channel and performs
 * account transfers.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void* worker(void* channel)
{
  int bufferChannel = (int)(long long)channel;
  pthread_mutex_t* mutex = &mutexWorkerBuffer[bufferChannel];
  transfer_buffer_t* buf = &workerBuffer[bufferChannel];

  // transfer variables
  int src;      // copy of the source account.
  int dest;     // copy of the destination account.
  int amount;   // copy of the amount of money to be transferred.
  int transfer; // 1 if transfer can be completed. 0 if it cannot.

  while(1)
  {
    src = -1;
    dest = -1;
    amount = -1;
    transfer = 0;

    // Copy data from the transfer buffer + set the empty flag 
    // so more data can be transferred to this channel.
    pthread_mutex_lock(mutex);
    if (buf->empty == 0)
    {
      src = buf->src;
      dest = buf->dest;
      amount = buf->amount;
      buf->empty = 1;
      transfer = 1;
      //printf("Rxd (%d) - src: %d, dest: %d, amount: %d\n", bufferChannel, src, dest, amount);
    }
    else if (buf->empty == 1 && isReaderComplete())
    {
      //printf("Ending worker thread: (%d)\n", bufferChannel);
      break;
    }
    pthread_mutex_unlock(mutex);

    // Wait for the source + destination to both be available.
    if (transfer)
    {
      
      // attempt to do the transaction until it completes successfully.
      while ((accountTransaction(src, dest, amount)) == -1)
        usleep(10000); // sleep for 10 ms to avoid starvation.
      transfer = 0;
    }

    usleep(10000); // sleep for 10 ms to avoid starvation.
  }

  return NULL;
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                       FUNCTIONS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: isReaderComplete
 * This function checks if the reader thread has marked
 * itself as completed.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
int isReaderComplete()
{
  pthread_mutex_lock(&mutexComplete);
  int complete = flagComplete;
  pthread_mutex_unlock(&mutexComplete);
  return complete;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: markReaderComplete
 * This function marks the reader thread as complete.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void markReaderComplete()
{
  pthread_mutex_lock(&mutexComplete);
  flagComplete = 1;
  pthread_mutex_unlock(&mutexComplete);
}
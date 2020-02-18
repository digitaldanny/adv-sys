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

int numWorkers;
pthread_t rthread;
pthread_mutex_t* mutexWorkerBuffer;
transfer_buffer_t* workerBuffer;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                      PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

void* reader(void* fid);

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                       THREADS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int main(int argc, char **argv)
{
  FILE* inputFid;
  
  // -------------------------------------------------------
  // turn off stdout/stdin buffers
  // -------------------------------------------------------
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);

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

  // -------------------------------------------------------
  // start threads
  // -------------------------------------------------------
  pthread_create(&rthread, NULL, reader, inputFid);

  // -------------------------------------------------------
  // cleanup
  // -------------------------------------------------------

  // wait for threads to complete before cleanup
  pthread_join(rthread, NULL);

  // close the input file descriptor
  if (fclose(inputFid) != 0)
  {
    printf("ERROR: Closing input file.\n");
    return -1;
  }

  printf("Program completed successfully.\n");
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

  initAccountTree();
  
  // read all lines of the file and determine if the line is
  // an account number or a transfer..
  // 1) If an account number, allocate space for another account.
  // 2) If a transfer, assign work to the worker threads.
  while ((read = getline(&line, &len, fid)) != -1) 
  {
    // This should be a transfer if the first character is a "T"
    if (line[0] == 'T')
    {
      printf("NOT IMPLEMENTED: Transfer on line: %s\n", line);

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
        // Transfer keyword.
        if (count == 0)
        {
          continue;
        }

        // Source account number.
        else if (count == 1)
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
        else
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
          if (pthread_mutex_lock(mutex) > 0)
          {
            // could not claim the lock.. go to the next buffer.
          }
          else
          {
            // claimed the lock.. transfer data into the buffer if it is empty.
            if (buf->empty)
            {
              buf->empty = 0;
              buf->amount = amount;
              buf->src = src;
              buf->dest = dest;
              foundABuffer = 1;
              printf("Wrote to a buffer: %d\n", i);
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

  printAccountContents();
  destroyAccountTree();

  return NULL;
}
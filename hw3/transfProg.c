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

pthread_t rthread;

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
  int numWorkers;
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
      printf("Transfer on line: %s\n", line);
      // Claim account mutex and make transfer
    }
    else
    {
      printf("Account number on line: %s\n", line);
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
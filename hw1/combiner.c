#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *                      DEFINES
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */

#define COMBINER_DEBUG_MODE 1
#define PIPE_SIZE           100
#define STDIN_FD            0
#define STDOUT_FD           1

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *                       GLOBALS
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *                      PROTOTYPES
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */

void parent(int pfd, char* buf);
void child(int pfd, char* buf);

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *                    MAIN / PROCESSES
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */

int main(void)
{
  int pipeFd[2];              // pipe file descriptors
  char pipeBuff[PIPE_SIZE];   // buffer for IPC

    setbuf(stdout, NULL);

    // set up pipe to write from parent (mapper) to child (reducer)
    if (pipe(pipeFd) == -1)
      printf("ERROR: Pipe instantiation..");

    switch(fork())
    {
      case -1:
		    printf("ERROR: Fork can't produce child.."); 
        break;

      case 0: // child
        child(pipeFd[0], pipeBuff);
        break;

      default: // parent
        parent(pipeFd[1], pipeBuff);
        break;
    }

    printf("ERROR: Main program exit..");
    return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: parent
 * This process runs the mapper program and routes the 
 * standard output to pipe.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void parent(int pfd, char* buf)
{
  // route the standard output of this process to the input of the pipe
  dup2(pfd, STDOUT_FD);

  char testString[] = "Hello world\n";

  while(1)
  {
    write(pfd, testString, strlen(testString));
    printf("Parent: Message sent to child\n");

    // wait for child to finish, then flush and exit.
    wait(NULL);
    printf("Parent closing\n");
    exit(0); // flush buffer, close process
  }
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: child
 * This process runs the reducer program and routes the 
 * standard input to pipe.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void child(int pfd, char* buf)
{
  ssize_t numRead; // records number of bytes read from pipe

  // route the standard input of this process to be the output of the pipe
  dup2(pfd, STDIN_FD);
  
  while(1)
  {
    // read message from the parent until pipe is empty
    while(1)
    {
      numRead = read(pfd, buf, PIPE_SIZE);
      if (numRead == -1)
      {
        printf("ERROR: Problem reading from pipe\n");
        break;
      }
      else if (numRead == 0)
      {
        break; // EOF found
      }

      // echo the pipe data back to the standard output
      if (write(STDOUT_FD, buf, numRead) != numRead)
      {
        printf("ERROR: Problem printing out message from pipe\n");
        break;
      }
    }

    printf("Child closing\n");
    _exit(0); // close process
  }
}

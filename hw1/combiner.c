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

#define COMBINER_DEBUG_MODE 0
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

void parent(int pfd);
void child(int pfd);

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *                    MAIN / PROCESSES
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */

int main(void)
{
  int pipeFd[2]; // pipe file descriptors

  setbuf(stdout, NULL); // do not buffer stdout

  // set up pipe to write from parent (mapper) to child (reducer)
  if (pipe(pipeFd) == -1)
    printf("ERROR: Pipe instantiation..");

  switch(fork())
  {
    case -1:
	    printf("ERROR: Fork can't produce child.."); 
      break;

    case 0: // child
      child(pipeFd[0]);
      break;

    default: // parent
      parent(pipeFd[1]);
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
void parent(int pfd)
{
  // route the standard output of this process to the input of the pipe
  dup2(pfd, STDOUT_FD);
  int err = execlp("./mapper", "./mapper", NULL);

  // Error if the process gets to this point..
  if (err == -1)
  {
    printf("ERROR: Parent is exitting..\n");
    close(pfd);
    wait(NULL);
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
void child(int pfd)
{
  // route the standard input of this process to be the output of the pipe
  dup2(pfd, STDIN_FD);
  int err = execlp("./reducer", "./reducer", NULL);

  // Error if the process gets to this point..
  if (err == -1)
  {
    printf("ERROR: Child is exitting..\n");
    close(pfd);
    _exit(0);
  }
}

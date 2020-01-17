#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "common.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                      DEFINES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

#define COMBINER_DEBUG_MODE 0
#define PIPE_SIZE           100
#define STDIN_FD            0
#define STDOUT_FD           1
#define NUM_CHILDREN        2

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                      PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

void mapper(int* pipeFd);
void reducer(int* pipeFd);
void errExit(char* string);

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                    MAIN / PROCESSES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int main(void)
{
  int pipeFd[2];                      // pipe file descriptors (0 == read, 1 == write)
  void (*task[NUM_CHILDREN])(int*);   // tasks to be completed by children

  setbuf(stdout, NULL); // do not buffer stdout
  setbuf(stdin, NULL); // do not buffer stdin

  // assign the list of tasks to the children
  task[0] = &mapper;
  task[1] = &reducer;

  // set up pipe so reader/writer handles will be copied to
  // all children.
  if (pipe(pipeFd) == -1)
    printf("ERROR: Pipe instantiation..");

  // instantiate multiple children and pass their tasks
  for (uint16_t i = 0; i < NUM_CHILDREN; i++)
  {
    pid_t pid = fork();

    // fork error
    if (pid == -1)
    {
	    printf("ERROR: Fork can't produce child.."); 
      wait(NULL);
      exit(0);
    }

    // child (tasks - mapper or reducer)
    else if (pid == 0)
    {
      (*task[i])(pipeFd);
    }

    // parent (combiner - child manager)
    else
    {
      continue;
    }
  }

  // close both ends of the pipe since the combiner doesn't
  // need write or read access
  if (close(pipeFd[0]) == -1)
    errExit("ERROR: Closing read end of pipe in parent");

  if (close(pipeFd[1]) == -1)
    errExit("ERROR: Closing write end of pipe in parent");

  // wait for all children to close and then exit
  for (uint16_t i = 0; i < NUM_CHILDREN; i++)
    wait(NULL);

  exit(0);
  return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mapper
 * This process runs the mapper program and routes the 
 * standard output to pipe.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void mapper(int* pipeFd)
{
  debugger("Starting MAPPER..", COMBINER_DEBUG_MODE);

  // this process is the writer, so the read end of the 
  // pipe is closed.
  if (close(pipeFd[0]) == -1)
    errExit("ERROR: Closing read end of pipe in mapper");

  // route the standard output of this process to the input of the pipe
  dup2(pipeFd[1], STDOUT_FD);
  int err = execlp("./mapper", "./mapper", NULL);

  // Error if the process gets to this point..
  if (err == -1)
  {
    printf("ERROR: MAPPER is exitting..");
    close(pipeFd[1]);
    exit(0); // flush buffer, close process
  }
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: reducer
 * This process runs the reducer program and routes the 
 * standard input to pipe.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void reducer(int* pipeFd)
{
  debugger("Starting REDUCER..", COMBINER_DEBUG_MODE);

  // this process is the reader, so the write end of the pipe
  // is closed.
  if (close(pipeFd[1]) == -1)
    errExit("ERROR: Closing write end of pipe in reducer");

  // route the standard input of this process to be the output of the pipe
  dup2(pipeFd[0], STDIN_FD);
  int err = execlp("./reducer", "./reducer", NULL);

  // Error if the process gets to this point..
  if (err == -1)
  {
    printf("ERROR: REDUCER is exitting..\n");
    close(pipeFd[0]);
    exit(0);
  }
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                       FUNCTIONS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: errExit
 * This function prints out the message and exits successfully.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void errExit(char* string)
{
  printf("%s\n", string);
  exit(0);
}
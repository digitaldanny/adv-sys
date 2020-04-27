#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/
void *thread1();
void *thread2();

// usbkbd driver simulator related threads+process
void PROC_usbkbdDriverSimulator();
void THREAD_usbKbdIrq();
void THREAD_usbKbdLed();
void THREAD_usbKbdEvent();
void THREAD_simSideEndpointInterrupt();
void THREAD_simSideEndpointControl();

// keyboard related threads+process
void PROC_keyboard();
void THREAD_endpointInterrupt();
void THREAD_endpointControl();

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                  GLOBALS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

pthread_t threadid[2];

// pipe file descriptors
int pipeInterruptFd[2];
int pipeControlFd[2];
int pipeAckFd[2];

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                 PROCESSES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: main
 * This process initializes all pipes and starts the simulation and keyboard 
 * processes.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
int main() 
{
  setbuf(stdout, NULL); // do not buffer stdout
  setbuf(stdin, NULL); // do not buffer stdin

  // initialize all pipes
  if (pipe(pipeInterruptFd) == -1 || 
      pipe(pipeControlFd) == -1   ||
      pipe(pipeAckFd) == -1)
    printf("ERROR: Pipe initialization.\n");

  // start the keyboard and simulation processes
  switch(fork())
  {
    case -1:
      printf("ERROR (fork): Cannot start both processes.\n");
      wait(NULL);
      exit(0);

    case 0: // child
      PROC_usbkbdDriverSimulator();
      break;

    default: // parent
      PROC_keyboard();
      break;
  }

  printf("ERROR: Main program returned when processes ended!\n");
	return 0;
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                       PROCESS + THREAD DEFINITIONS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: PROC_usbkbdDriverSimulator
 * This process simulates the usb keyboard driver in Linux (usbkbd.c)
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void PROC_usbkbdDriverSimulator() // child
{
  // close unecessary ends of the pipes
  close(pipeControlFd[0]);   // close read end
  close(pipeAckFd[1]);       // close write end
  close(pipeInterruptFd[1]); // close write end

  _exit(0);
}

void THREAD_usbKbdIrq();
void THREAD_usbKbdLed();
void THREAD_usbKbdEvent();
void THREAD_simSideEndpointInterrupt();
void THREAD_simSideEndpointControl();

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: PROC_keyboard
 * This process simulates 
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void PROC_keyboard() // parent
{
  // close unecessary ends of the pipes
  close(pipeControlFd[1]);   // close write end
  close(pipeAckFd[0]);       // close read end
  close(pipeInterruptFd[0]); // close read end

  exit(0);
} 

// keyboard related threads+process
void PROC_keyboard();
void THREAD_endpointInterrupt();
void THREAD_endpointControl();
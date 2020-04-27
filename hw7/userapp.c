#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "pthread.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

// usbkbd driver simulator related threads+process
void PROC_usbkbdDriverSimulator();
void *THREAD_usbKbdIrq();
void *THREAD_usbKbdLed();
void *THREAD_usbKbdEvent();
void *THREAD_simSideEndpointInterrupt();
void *THREAD_simSideEndpointControl();

// keyboard related threads+process
void PROC_keyboard();
void *THREAD_endpointInterrupt();
void *THREAD_endpointControl();

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                  GLOBALS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

pthread_t threadids_proc1[5]; // simulator thread ids
pthread_t threadids_proc2[2]; // keyboard thread ids

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

  // usb_kbd_open

  _exit(0);
}

// simulator related threads
void *THREAD_usbKbdIrq();  // threadid[0]
void *THREAD_usbKbdLed();  // threadid[1]
void *THREAD_usbKbdEvent(); // threadid[2]
void *THREAD_simSideEndpointInterrupt(); // threadid[3]
void *THREAD_simSideEndpointControl(); // threadid[4]

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

  // Launch both endpoint threads
  pthread_create(&threadids_proc2[0], NULL, THREAD_endpointInterrupt, NULL);
  pthread_create(&threadids_proc2[1], NULL, THREAD_endpointControl, NULL);

  // Wait for threads to join, then exit the process
  for (int i = 0; i < 2; i++)
    pthread_join(threadids_proc2[i], NULL);
  exit(0);
} 

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: THREAD_endpointInterrupt
 * This thread forwards stdin to the usbkbd simulator through the IRQ pipe.
 * 
 * OTHER DETAILS:
 * threadid = threadids_proc2[0]
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void *THREAD_endpointInterrupt()
{
  printf("THREAD_endpointInterrupt\n");
  pthread_exit(NULL);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: THREAD_endpointControl
 * This thread manages the final text that is output to the terminal by 
 * reading from the mmap'd led buffer and the control pipe for characters.
 * 
 * OTHER DETAILS:
 * threadid = threadids_proc2[0]
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void *THREAD_endpointControl()
{
  printf("THREAD_endpointControl\n");
  pthread_exit(NULL);
}
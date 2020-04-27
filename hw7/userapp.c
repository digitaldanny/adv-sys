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
 *                                 DEFINES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

#define PIPE_READ           0
#define PIPE_WRITE          1

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                 STRUCTS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

struct urb
{
  int pipe;
  struct input_dev* dev;
};

struct input_dev
{
  int (*event) (struct input_dev *dev, unsigned int type, unsigned int code, int value);
  int led;
};

struct usb_kbd
{
  struct input_dev* dev;
  struct urb* irq;
  struct urb* led;
};

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

// usbkbd driver simulator related threads+process
void PROC_usbkbdDriverSimulator();
void *THREAD_usbKbdIrq(struct urb *urb);
void *THREAD_usbKbdLed(struct urb *urb);
int THREAD_usbKbdEvent(struct input_dev *dev, unsigned int type, unsigned int code, int value);
void *THREAD_simSideEndpointInterrupt();
void *THREAD_simSideEndpointControl();
int FUNC_usbKbdOpen(struct input_dev* dev);
void FUNC_usbSubmitUrb(struct urb* urb);

// keyboard related threads+process
void PROC_keyboard();
void *THREAD_endpointInterrupt();
void *THREAD_endpointControl();

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                  GLOBALS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

// simulator data structures
struct input_dev dev;
struct usb_kbd kbd;

// thread ids for both processes
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

// ****************************** DRIVER PROCESS ********************************

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: PROC_usbkbdDriverSimulator
 * This process simulates the usb keyboard driver in Linux (usbkbd.c)
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void PROC_usbkbdDriverSimulator() // child
{
  // close unecessary ends of the pipes
  close(pipeInterruptFd[PIPE_WRITE]); // close write end
  close(pipeAckFd[PIPE_WRITE]);       // close write end
  close(pipeControlFd[PIPE_READ]);    // close read end

  // define device parameters
  dev.event = &THREAD_usbKbdEvent;
  dev.led = 0;

  // usb_kbd_open to initialize device and urb
  FUNC_usbKbdOpen(&dev);

  while(1) 
  {
    // wait for shutdown signal??
  }

  _exit(0);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: FUNC_usbKbdOpen
 * This function creates a urb and submits it using the usbSubmitUrb function.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
int FUNC_usbKbdOpen(struct input_dev* dev) // usb_kbd_open
{
  printf("usb_open_kbd\n");

  // Get device data ---> struct usb_kbd *kbd = input_get_drvdata(dev);
	struct usb_kbd *kbd;
  kbd->dev = dev;
	kbd->irq->dev = dev; // added this - probable segfault

	FUNC_usbSubmitUrb(kbd->irq);
	return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: FUNC_usbSubmitUrb
 * During its first iteration, this function 
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void FUNC_usbSubmitUrb(struct urb* urb) // usb_submit_urb
{
  printf("usb_submit_urb\n");
  static int count = 0; // defaults to 0

  // launch usb_kbd_irq and usb_kbd_led threads on the first iteration. 
  if (count == 0)
  {
    printf("Launching server side endpoints\n");
    pthread_create(&threadids_proc1[0], NULL, THREAD_simSideEndpointInterrupt, NULL);
    pthread_create(&threadids_proc1[1], NULL, THREAD_simSideEndpointControl, NULL);
    pthread_detach(threadids_proc1[0]);
    pthread_detach(threadids_proc1[1]);
    count++;
  }

  // possibly trigger urb completion handler due to missing ACK?
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: THREAD_usbKbdIrq
 * This thread handles input from the interrupt endpoint and reports
 * which key is pressed to the input subsystem.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void *THREAD_usbKbdIrq(struct urb *urb) // usb_kbd_irq
{
  printf("usb_kbd_irq\n");
  //struct usb_kbd *kbd = urb->context;

  // find out key presses and key releases, then submit to input subsystem
  // for each event in old and not in new, check report key release with input_report_key
  //FUNC_usbSubmitUrb(urb); // resubmit URB
  pthread_exit(NULL);
}

void *THREAD_usbKbdLed(struct urb *urb) // usb_kbd_led
{
  printf("usb_kbd_led\n");
  pthread_exit(NULL);
}

int THREAD_usbKbdEvent(struct input_dev *dev, unsigned int type, unsigned int code, int value)
{
  printf("usb_kbd_event\n");
  pthread_exit(NULL);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: THREAD_simSideEndpointInterrupt
 * ~This thread reads characters from the interrupt endpoint pipe and triggers
 * the usbd_kbd_irq callback.
 * ~Additionally, this thread signals the keyboard process that keystrokes
 * can begin transferring.
 * 
 * OTHER DETAILS:
 * threadid = threadids_proc1[4]
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void *THREAD_simSideEndpointInterrupt()
{
  printf("Launching driver_irq_endpoint\n");
  static int count = 0;

  // if this is the first time running, signal interrupt endpoint to begin sending
  // characters through the pipe.


  pthread_exit(NULL);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: THREAD_simSideEndpointControl
 * ...
 * 
 * OTHER DETAILS:
 * threadid = threadids_proc1[5]
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void *THREAD_simSideEndpointControl()
{
  printf("Launching driver_control_endpoint\n");
  pthread_exit(NULL);
}

// ****************************** KEYBOARD PROCESS ********************************

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: PROC_keyboard
 * This process simulates 
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void PROC_keyboard() // parent
{
  // close unecessary ends of the pipes
  close(pipeInterruptFd[PIPE_READ]); // close read end
  close(pipeAckFd[PIPE_READ]);       // close read end
  close(pipeControlFd[PIPE_WRITE]);  // close write end

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
void *THREAD_endpointInterrupt() // usb core?
{
  char keyboardInput;

  printf("THREAD_endpointInterrupt\n"); // DEBUG

  // Sync point waiting for driver threads to initialize

  // forward characters from stdin to the pipe
  while(read(STDIN_FILENO, &keyboardInput, 1) > 0)
  {
    write(STDOUT_FILENO, &keyboardInput, 1); // DEBUG
    //write(pipeInterruptFd[PIPE_WRITE], &keyboardInput, 1); // redirect standard input to keyboard driver
  }

  // Signal all threads to close

  printf("THREAD_endpointInterrupt ending\n");
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
  char ack = 'A';
  char cmd = 'X'; // Default value - X forces thread to wait for command
  int ledStatus = 0;

  printf("THREAD_endpointControl\n");

  while(1)
  {
    // wait for the command to read from LED buffer 
    while(cmd != 'C')
    {
      read(pipeControlFd[PIPE_READ], &cmd, 1);
    }

    // read from the leds buffer

    // Determine if the LED output should be ON or OFF
    if (ledStatus == 1)
    {
      // Turn capslock ON
    }
    else
    {
      // Turn capslock OFF
    }

    // ACK - Notify the driver that the command has been received and handled
    write(pipeAckFd[PIPE_WRITE], &ack, 1);

    // force thread to wait for new command from driver
    cmd = 'X';
  }

  pthread_exit(NULL);
}
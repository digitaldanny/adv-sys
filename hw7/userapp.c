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
#include <ctype.h>

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                 DEFINES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

#define PIPE_READ           0
#define PIPE_WRITE          1
#define CAPSLOCK_OFF        0
#define CAPSLOCK_ON         1

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                 STRUCTS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: thread_signal
 * This struct is used to safely signal other threads to continue.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
typedef struct thread_signal
{ 
  pthread_mutex_t mutex;
  int cond;
} thread_signal_t;

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: urb
 * This struct is used to 
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
struct urb
{
  void* context; // "parameters" to pass into completion routine
  void* (*complete)(void* urb); // completion routine that runs once urb submitted
  char transfer_buffer; // for this application, the buffer is a 1 wide character

  // extra attributes
  pthread_mutex_t transfer_buffer_lock; // multiple threads using transfer buff requires lock
  char debug; // ID used to determine that URBs are being passed correctly
  thread_signal_t poll; // signal set by usb_submit_urb to poll again
};

struct input_dev
{
  int (*event) (struct input_dev *dev, unsigned int type, unsigned int code, int value);
  int led;
  pthread_mutex_t event_lock;
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
void *THREAD_usbKbdIrq(void* urb);
void *THREAD_usbKbdLed(void* urb);
int THREAD_usbKbdEvent(struct input_dev *dev, unsigned int type, unsigned int code, int value);
void *THREAD_simSideEndpointInterrupt();
void *THREAD_simSideEndpointControl();
int FUNC_usbKbdOpen(struct input_dev* dev);
void FUNC_usbSubmitUrb(struct urb* urb);
void FUNC_inputReportKey(struct input_dev* dev, char keystroke);

// keyboard related threads+process
void PROC_keyboard();
void *THREAD_endpointInterrupt();
void *THREAD_endpointControl();

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                  GLOBALS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

// syncronization used to wait for all simulation threads to set up
// before receiving data from the keyboard.
pthread_barrier_t waitForSimulationSetup;

// thread safe signals for both processes to signal shutting down the simulation
thread_signal_t keyboardShutdown;
thread_signal_t driverShutdown;

// simulator data structures
struct input_dev dev;
struct usb_kbd kbd;
struct urb irq_urb;
struct urb led_urb;

// thread ids for both processes
pthread_t threadids_proc1[5]; // simulator thread ids
pthread_t threadids_proc2[2]; // keyboard thread ids

// pipe file descriptors
int pipeInterruptFd[2];
int pipeControlFd[2];
int pipeAckFd[2];
int pipeSync[2];

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
      pipe(pipeAckFd) == -1       ||
      pipe(pipeSync) == -1)
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
  close(pipeSync[PIPE_READ]);         // close read end

  // initialize shutdown signal
  driverShutdown.cond = 0;
  pthread_mutex_init(&driverShutdown.mutex, NULL);

  // define device parameters
  dev.event = &THREAD_usbKbdEvent;
  dev.led = 0;
  pthread_mutex_init(&dev.event_lock, NULL);

  // usb_kbd_open to initialize device and urb
  FUNC_usbKbdOpen(&dev);

  // wait for the command to read from LED buffer 
  pthread_mutex_lock(&driverShutdown.mutex);
  while(driverShutdown.cond == 0)
  {
    pthread_mutex_unlock(&keyboardShutdown.mutex);
    sleep(0.1); // avoid starvation issues
    pthread_mutex_lock(&keyboardShutdown.mutex);
  }

  printf("Simulation closing\n");
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
  kbd.dev = dev;
  kbd.irq = &irq_urb;
  kbd.led = &led_urb;

  // configure urb parameters
  kbd.irq->debug = 'I';
  kbd.irq->complete = THREAD_usbKbdIrq;
  pthread_mutex_init(&kbd.irq->transfer_buffer_lock, NULL);

  kbd.led->debug = 'L';
  kbd.led->complete = THREAD_usbKbdLed;
  pthread_mutex_init(&kbd.led->transfer_buffer_lock, NULL);

	FUNC_usbSubmitUrb(kbd.irq);
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
  int sync = 'S';
  static int count = 0; // defaults to 0

  // launch usb_kbd_irq and usb_kbd_led threads on the first iteration. 
  if (count == 0)
  {
    printf("Launching server side endpoints\n");
    pthread_barrier_init (&waitForSimulationSetup, NULL, 3);
    pthread_create(&threadids_proc1[0], NULL, THREAD_simSideEndpointInterrupt, (void*)kbd.irq);
    pthread_create(&threadids_proc1[1], NULL, THREAD_simSideEndpointControl, (void*)kbd.led);
    pthread_detach(threadids_proc1[0]);
    pthread_detach(threadids_proc1[1]);

    // wait for simulation-side threads to get set up, then signal keyboard process
    pthread_barrier_wait (&waitForSimulationSetup);
    write(pipeSync[PIPE_WRITE], &sync, 1);
    count++;
  }

  // signal appropriate urb handler to poll again
  pthread_mutex_lock(&urb->poll.mutex);
  printf("usb_submit_urb enabling polling for URB ID: %c", urb->debug);
  urb->poll.cond = 1;
  pthread_mutex_unlock(&urb->poll.mutex);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: FUNC_inputReportKey
 * This function outputs the character input by the usb_kbd_irq thread and
 * also creates the usb_kbd_event if the device shows that the led is on or
 * off.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void FUNC_inputReportKey(struct input_dev* dev, char keystroke) // input_report_key
{
  // next time this function is entered, this variable will be used to
  // determine if the character pressed needs to be modified to be capital or
  // lowercase.
  static int capslockState = CAPSLOCK_OFF;

  // check if CAPSLOCK event needs to be started
  pthread_mutex_lock(&kbd.dev->event_lock);
  if (kbd.dev->led == 1)
  {
    pthread_mutex_unlock(&kbd.dev->event_lock);

    // if the CAPSLOCK is currently on, turn it off.
    // if the CAPSLOCK is currently off, turn it on.
    if (capslockState == CAPSLOCK_ON)
      capslockState = CAPSLOCK_OFF; 
    else
      capslockState = CAPSLOCK_ON; 

    // start the new keyboard event so the keyboard process can turn the led on
    //printf("CAPSLOCK EVENT - Start new event thread.\n");
    kbd.dev->led = 0; // reset so one keyboard event doesn't cause multiple
    return;
  }
  pthread_mutex_unlock(&kbd.dev->event_lock);

  // ignored '&' symbol as an event in usb_kbd_irq, so I must ignore it here to avoid outputting it..
  if (keystroke == '&')
    return;

  // If there is no keyboard event, determine whether the keystroke needs to be modified
  if (capslockState == CAPSLOCK_ON)
    putchar(toupper(keystroke));
  else
    putchar(keystroke);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: THREAD_usbKbdIrq
 * This thread handles input from the interrupt endpoint and reports
 * which key is pressed to the input subsystem.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void *THREAD_usbKbdIrq(void* surb) // usb_kbd_irq
{
  struct urb* urb = (struct urb*)surb;

  // only start an input report call if the key is non-#
  pthread_mutex_lock(&urb->transfer_buffer_lock);
  if (urb->transfer_buffer != '#')
  {
    //printf("usb_kbd_irq: %c\n", urb->transfer_buffer);
    // determine if the kbd->dev->led should be on or off
    // based on whether the capslock button was pressed or not.
    pthread_mutex_lock(&kbd.dev->event_lock);
    if (urb->transfer_buffer == '@')
      kbd.dev->led = 1; // capslock event
    else
      kbd.dev->led = 0; // no capslock event
    pthread_mutex_unlock(&kbd.dev->event_lock);

    // report the key that was pressed along with modified kbd device
    FUNC_inputReportKey(kbd.dev, urb->transfer_buffer);
  }
  pthread_mutex_unlock(&urb->transfer_buffer_lock);

  // find out key presses and key releases, then submit to input subsystem
  // for each event in old and not in new, check report key release with input_report_key
  //FUNC_usbSubmitUrb(urb); // resubmit URB
  pthread_exit(NULL);
}

void *THREAD_usbKbdLed(void* surb) // usb_kbd_led
{
  struct urb* urb = (struct urb*)surb;
  printf("usb_kbd_led - URB ID: %c\n", urb->debug);
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
void *THREAD_simSideEndpointInterrupt(struct urb *urb) // USB core
{
  char keyboardStroke;
  printf("Launching driver_irq_endpoint - URB ID: %c\n", urb->debug);

  // sync all simulation side threads
  pthread_barrier_wait (&waitForSimulationSetup);

  // this while-loop waits for the usb_submit_urb function to allow
  // polling before reading from the interrupt endpoint pipe and 
  // starting a new thread handler.
  while(1)
  {
    // wait for the usb_submit_urb command before getting another character from the pipe
    pthread_mutex_lock(&urb->poll.mutex);
    while(urb->poll.cond == 0)
    {
      pthread_mutex_unlock(&urb->poll.mutex);
      sleep(0.1);
      pthread_mutex_lock(&urb->poll.mutex);
    }
    pthread_mutex_unlock(&urb->poll.mutex);

    // exit the infinite loop if the pipe is out of data to read
    if (read(pipeInterruptFd[PIPE_READ], &keyboardStroke, 1) == 0)
      break;

    // start a new usb_kbd_irq to handle the keyboard stroke
    pthread_mutex_lock(&urb->transfer_buffer_lock);
    urb->transfer_buffer = keyboardStroke;
    pthread_mutex_unlock(&urb->transfer_buffer_lock);
    pthread_create(&threadids_proc1[2], NULL, urb->complete, (void*)urb); // start urb completion handler
    pthread_join(threadids_proc1[2], NULL); // wait for urb completion handler
  }

  printf("Simulation side IRQ endpoint closing..\n");
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
void *THREAD_simSideEndpointControl(struct urb *urb)
{
  printf("Launching driver_control_endpoint - URB ID: %c\n", urb->debug);

  // sync all simulation side threads
  pthread_barrier_wait (&waitForSimulationSetup);

  // CAPSLOCK stuff goes here..

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
  close(pipeSync[PIPE_WRITE]);       // close read end

  // initialize shutdown signal
  keyboardShutdown.cond = 0;
  pthread_mutex_init(&keyboardShutdown.mutex, NULL);

  // Launch both endpoint threads
  pthread_create(&threadids_proc2[0], NULL, THREAD_endpointInterrupt, NULL);
  pthread_create(&threadids_proc2[1], NULL, THREAD_endpointControl, NULL);

  // Wait for threads to join, then exit the process
  for (int i = 0; i < 2; i++)
    pthread_join(threadids_proc2[i], NULL);

  printf("Keyboard closing\n");
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
  char sync = 'X';

  printf("THREAD_endpointInterrupt\n"); // DEBUG

  // Sync point waiting for driver threads to initialize
  while(sync != 'S')
  {
    read(pipeSync[PIPE_READ], &sync, 1);
  }

  // forward characters from stdin to the pipe
  while(read(STDIN_FILENO, &keyboardInput, 1) > 0)
  {
    write(pipeInterruptFd[PIPE_WRITE], &keyboardInput, 1); // redirect standard input to keyboard driver
  }

  // Signal all threads to close
  pthread_mutex_lock(&keyboardShutdown.mutex);
  keyboardShutdown.cond = 1;
  pthread_mutex_unlock(&keyboardShutdown.mutex);

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

  // make the control pipe's read end noneblocking
  fcntl(pipeControlFd[PIPE_READ], F_SETFL, fcntl(pipeControlFd[PIPE_READ], F_GETFL) | O_NONBLOCK);

  while(1)
  {
    // wait for the command to read from LED buffer 
    pthread_mutex_lock(&keyboardShutdown.mutex);
    while(cmd != 'C' && keyboardShutdown.cond == 0)
    {
      pthread_mutex_unlock(&keyboardShutdown.mutex);
      read(pipeControlFd[PIPE_READ], &cmd, 1);
      pthread_mutex_lock(&keyboardShutdown.mutex);
      sleep(0.1); // avoid starvation issues
    }
    pthread_mutex_unlock(&keyboardShutdown.mutex);

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

    // check if the thread should close down
    pthread_mutex_lock(&keyboardShutdown.mutex);
    if (keyboardShutdown.cond == 1)
    {
      printf("Control endpoint received shutdown signal\n");
      pthread_mutex_unlock(&keyboardShutdown.mutex);
      break;
    }
    pthread_mutex_unlock(&keyboardShutdown.mutex);
  }

  printf("Control endpoint closing\n");
  pthread_exit(NULL);
}
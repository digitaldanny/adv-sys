#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>

#define DEVICE "/dev/a5"

#define CDRV_IOC_MAGIC 'Z'
#define E2_IOCMODE1 _IOWR(CDRV_IOC_MAGIC, 1, int)
#define E2_IOCMODE2 _IOWR(CDRV_IOC_MAGIC, 2, int)

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/
void *thread1();
void *thread2();

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                                  GLOBALS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/
pthread_t threadid[2];

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                             THREAD DEFINITIONS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: main
 * This thread launches threads 1 and 2 then waits for them to join. For this
 * demo, the final message should not display. This will show that the 
 * program is in a deadlock.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
int main() 
{
    printf("Device %s\n", DEVICE);

    pthread_create(&threadid[0], NULL, thread1, NULL);
    sleep(1);
    pthread_create(&threadid[1], NULL, thread2, NULL);
    
    pthread_join(threadid[0], NULL);
    pthread_join(threadid[1], NULL);

	// Execution will never get this far, so there is no reason to try closing.
    printf("If you see this message, the program DID NOT deadlock.\n");
	return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: thread1
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void *thread1()
{
    // variable definitions
	int fd;
	char ch, write_buf[100], read_buf[10];
	int offset, origin;
	
    // OPEN DEVICE A FIRST TIME
	fd = open(DEVICE, O_RDWR);
	if(fd == -1) 
    {
		printf("File %s either does not exist or has been locked by another "
				"process\n", DEVICE);
		exit(-1);
	}
    printf("T1: Opened device.\n");

    sleep(2);
    printf("T1: Switching to MODE 2\n");
    ioctl(fd, E2_IOCMODE2);

    // Shouldn't be able to get this far into the program
    pthread_exit(NULL);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: thread2
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void *thread2()
{
    // variable definitions
	int fd;
	char ch, write_buf[100], read_buf[10];
	int offset, origin;
	
    // OPEN DEVICE A FIRST TIME
    printf("T2: Opening device.\n");
	fd = open(DEVICE, O_RDWR);
	if(fd == -1) 
    {
		printf("File %s either does not exist or has been locked by another "
				"process\n", DEVICE);
		exit(-1);
	}

    printf("T2: Device opened. This message should not display!!!\n");
    pthread_exit(NULL);
}

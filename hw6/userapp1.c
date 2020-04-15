#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEVICE "/dev/a5"

#define CDRV_IOC_MAGIC 'Z'


int main() 
{
	// variable definitions
	int fd, fd2;
	char ch, write_buf[100], read_buf[10];
	int offset, origin;

    printf("Device %s\n", DEVICE);
	
    // OPEN DEVICE A FIRST TIME
    printf("Open 1.\n");
	fd = open(DEVICE, O_RDWR);
	if(fd == -1) 
    {
		printf("File %s either does not exist or has been locked by another "
				"process\n", DEVICE);
		exit(-1);
	}
	printf("Open 1 complete.\n");

    // OPEN DEVICE A SECOND TIME
    printf("Open 2.\n");
    fd2 = open(DEVICE, O_RDWR);
	if(fd2 == -1) 
    {
		printf("File %s either does not exist or has been locked by another "
				"process\n", DEVICE);
		exit(-1);
	}
	printf("Open 2 complete.\n");

	// Execution will never get this far, so there is no reason to try closing.
    printf("If you see this message, the program DID NOT deadlock.\n");
	return 0;
}

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
#define ASP_CLEAR_BUF _IOW(CDRV_IOC_MAGIC, 1, int)


int main(int argc, char *argv[]) {

	// program argument handler
	if (argc < 2) 
	{
		fprintf(stderr, "Device number not specified\n");
		return 1;
	}

	// variable definitions
	int dev_no = atoi(argv[1]);
	char dev_path[20];
	int i,fd;
	char ch, write_buf[100], read_buf[10];
	int offset, origin;

	// open the requested device
	sprintf(dev_path, "%s%d", DEVICE, dev_no);
	printf("Opening following device: ");
	printf(dev_path, sizeof(DEVICE)+1, 0);
	printf("\n");
	
	fd = open(dev_path, O_RDWR);
	if(fd == -1) {
		printf("File %s either does not exist or has been locked by another "
				"process\n", DEVICE);
		exit(-1);
	}

	// list program options
	printf(" r = read from device after seeking to desired offset\n"
			" w = write to device \n");
	printf(" c = Clear buffer\n");
	printf("\n\n enter command :");

	scanf("%c", &ch);
	switch(ch) 
	{

	/*
	 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
	 * WRITE
	 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
	*/
	case 'w':
		printf("Enter Data to write: ");
		scanf(" %[^\n]", write_buf);
		write(fd, write_buf, sizeof(write_buf));
		break;

	/*
	 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
	 * CLEAR
	 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
	*/
	case 'c':
		printf("\n Will clear data \n");
		int rc = ioctl(fd, ASP_CLEAR_BUF, 0);
		if (rc == -1) { 
			perror("\n***error in ioctl***\n");
			return -1;
		}
		printf("\n Data cleared \n");
		break;

	/*
	 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
	 * LSEEK + READ
	 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
	*/
	case 'r':
		printf("Origin \n 0 = beginning \n 1 = current \n 2 = end \n\n");
		printf(" enter origin :");
		scanf("%d", &origin);
		printf(" \n enter offset :");
		scanf("%d", &offset);

		lseek(fd, offset, origin);
		if (read(fd, read_buf, sizeof(read_buf)) > 0) {
			printf("\ndevice: %s\n", read_buf);
		} else {
			fprintf(stderr, "Reading failed\n");
		}
		break;

	/*
	 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
	 * OTHER
	 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
	*/
	default:
		printf("Command not recognized\n");
		break;

	}
	close(fd);
	return 0;
}

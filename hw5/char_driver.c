#include <linux/module.h>	/* for modules */
#include <linux/fs.h>		/* file_operations */
#include <linux/uaccess.h>	/* copy_(to,from)_user */
#include <linux/init.h>		/* module_init, module_exit */
#include <linux/slab.h>		/* kmalloc */
#include <linux/cdev.h>		/* cdev utilities */
#include <linux/moduleparam.h> /* moduleparam */

/*
 * *************************************************************************
 *                                 DEFINES
 * *************************************************************************
*/

#define MYDEV_NAME "mycdrv"
#define ramdisk_size (size_t) (16 * PAGE_SIZE) // default ramdisk size 

#define CDRV_IOC_MAGIC 'Z'
#define ASP_CLEAR_BUF _IOW(CDRV_IOC_MAGIC, 1, int)

#define LSEEK_ORIGIN_BEGIN 		0
#define LSEEK_ORIGIN_CURRENT 	1
#define LSEEK_ORIGIN_END 		2

/*
 * *************************************************************************
 *                                 STRUCTS
 * *************************************************************************
*/

typedef struct ASP_mycdrv {
	struct cdev cdev;
	char* ramdisk;
	size_t ramsize;
	struct semaphore sem;
	int devNo;
	int count;
} ASP_mycdrv_t;

/*
 * *************************************************************************
 *                                 GLOBALS
 * *************************************************************************
*/

ASP_mycdrv_t* device; 		// array of devices
dev_t first; 				// will contain major number + FIRST assigned minor number
struct class* device_class; // blueprint struct for making variable number of devices

// Module parameter defaults
int NUM_DEVICES = 3; 	// Max number of devices.
int major = 500;
int minor = 0;

/*
 * *************************************************************************
 *                                PROTOTYPES
 * *************************************************************************
*/

static int mycdrv_open(struct inode *inode, struct file *file);
static int mycdrv_release(struct inode *inode, struct file *file);
static ssize_t mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos);
static ssize_t mycdrv_write(struct file *file, const char __user * buf, size_t lbuf, loff_t * ppos);
static loff_t mycdrv_llseek(struct file *filp, loff_t off, int whence);
static long mycdrv_ioctl(struct file *filp, unsigned int cmd, unsigned long dir);
static int __init my_init(void);
static void __exit my_exit(void);

/*
 * *************************************************************************
 *                           DRIVER CONFIGURATIONS
 * *************************************************************************
*/

// device structure
static const struct file_operations mycdrv_fops = 
{
	.owner = THIS_MODULE,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.open = mycdrv_open,
	.release = mycdrv_release,
	.llseek = mycdrv_llseek,
	.unlocked_ioctl = mycdrv_ioctl,
};

// module init+exit
module_init(my_init);
module_exit(my_exit);

// module parameters (all modifiable at load time)
module_param(NUM_DEVICES, int, S_IRUGO);

// other module details
MODULE_AUTHOR("Daniel Hamilton");
MODULE_LICENSE("GPL v2");

/*
 * *************************************************************************
 *                           FUNCTION DEFINITIONS
 * *************************************************************************
*/

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: my_init
 * This function runs on installation of this device driver. It is 
 * responsible for allocating memory for the device.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static int __init my_init(void)
{
	int i; 						 // making drivers c89/c90 compliant
	unsigned int baseMinor 	= 0; // first of the requested range of minor numbers

	// get range of minor numbers and dynamic major number.
	if (alloc_chrdev_region(&first, baseMinor, NUM_DEVICES, MYDEV_NAME) != 0)
	{
		pr_info("ERROR (mycdrv): Could not create dynamic major number.\n");
		return -1;
	}
	major = MAJOR(first); // for easily finding device id (dev_t) of each minor number

	
	// create class for instantiating variable number of devices.
	if ((device_class = class_create(THIS_MODULE, MYDEV_NAME)) == NULL)
	{
		pr_info("ERROR (mycdrv): Class creation failed.\n");
		return -1;
	}

	// allocate and zero space for the variable number of devices.
	device = (ASP_mycdrv_t*)kzalloc(NUM_DEVICES*sizeof(ASP_mycdrv_t), GFP_KERNEL);
	
	// allocate ramdisk space and create device nodes for total number of devices (init all cdevs)
	for (i = 0; i < NUM_DEVICES; i++)
	{
		dev_t devId = MKDEV(major, i); // find device id based on major and minor number (provided to mknod)
		ASP_mycdrv_t* d = &device[i];
		
		// add character device to the system
		cdev_add(&d->cdev, devId, 1);

		// define the cdev + device parameters
		d->cdev.owner = THIS_MODULE;
		cdev_init(&d->cdev, &mycdrv_fops);
		d->ramsize = ramdisk_size;
		d->count = 0;
		d->devNo = i;
		d->ramdisk = (char*)kzalloc(ramdisk_size, GFP_KERNEL);
		sema_init(&d->sem, 1); // binary semaphore = mutex

		// Create device with following configurations:
		// ~ Parent Device = None
		// ~ Device name = mycdrv[deviceNo]
		device_create(device_class, NULL, devId, NULL, MYDEV_NAME "%d", i);

		pr_info("\nSucceeded in registering character device %s%d\n", MYDEV_NAME, i);
	}

	pr_info("\nWOO! Debugging installation successful!\n");
	return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: my_exit
 * This function runs on uninstallation of this device driver. It is 
 * responsible for deallocating device memory.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static void __exit my_exit(void)
{
	int i;
	pr_info("NOTICE: About to unregister device");

	// deallocate each device's ramdisk, cdev, and device
	for (i = 0; i < NUM_DEVICES; i++)
	{
		ASP_mycdrv_t* d = &device[i];

		kfree(d->ramdisk);
		pr_info("NOTICE: Free ramdisk for device %d\n", i);
	
		device_destroy(device_class, MKDEV(major, i));
		pr_info("NOTICE: Destroyed device node %d\n", i);


		cdev_del(&d->cdev);
		pr_info("NOTICE: Deleted cdev for device %d\n", i);
	}

	// deallocate the devices
	kfree(device);
	pr_info("NOTICE: Deallocated devices\n");

	// destroy the device class
	class_destroy(device_class);
	pr_info("NOTICE: Destroyed class blueprint\n");

	// unregister the device region for all minor numbers
	unregister_chrdev_region(first, NUM_DEVICES);
	pr_info("NOTICE: Unregistered device regions\n");
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_open
 * Point file* to appropriate device structure based on the requested
 * inode number.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static int mycdrv_open(struct inode *inode, struct file *file)
{
	ASP_mycdrv_t* p; // comply with c90 requirements
	pr_info("Entered open function!\n");

	// find the device with the same cdev as passed through the inode
	// and assign file pointer's data to the device.
	p = container_of(inode->i_cdev, struct ASP_mycdrv, cdev);
	file->private_data = p;

	// increment number of times device was opened (could result in data
	// race if multiple processes try to open same device) 
	down_interruptible(&p->sem);
	p->count++;
	up(&p->sem);

	pr_info(" OPENED device: %s:\n\n", MYDEV_NAME);
	return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_release
 * Decrement count for the specified device.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static int mycdrv_release(struct inode *inode, struct file *file)
{
	ASP_mycdrv_t* p; // comply with c90 requirements
	pr_info("Entered release function!\n");
	p = (ASP_mycdrv_t*)file->private_data;

	// increment number of times device was opened (could result in data
	// race if multiple processes try to open same device) 
	down_interruptible(&p->sem);
	p->count--;
	up(&p->sem);

	pr_info(" CLOSED device: %s:\n\n", MYDEV_NAME);
	return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_read
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static ssize_t mycdrv_read(struct file *file, char __user * buf, size_t count, loff_t * ppos)
{
	int nbytes = 0;
	ASP_mycdrv_t* p;

	p = (ASP_mycdrv_t*)file->private_data;
	down_interruptible(&p->sem);

	// check if the user is trying to read past end of the device
	if ((count + *ppos) > p->ramsize) 
	{
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}

	// copy data from kernel space to user space
	nbytes = count - copy_to_user(buf, p->ramdisk + *ppos, count);
	*ppos += nbytes;

	up(&p->sem);

	pr_info("\n READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_write
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static ssize_t mycdrv_write(struct file *file, const char __user * buf, size_t count, loff_t * ppos)
{
	int nbytes = 0;
	ASP_mycdrv_t* p;

	p = (ASP_mycdrv_t*)file->private_data;
	down_interruptible(&p->sem);

	// check if the user is trying to write past end of the device
	if ((count + *ppos) > p->ramsize) 
	{
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}

	// copy data from user space into kernel space
	nbytes = count - copy_from_user(p->ramdisk + *ppos, buf, count);
	*ppos += nbytes;

	up(&p->sem);

	pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_llseek
 * This function changes the character device's pointer to requested
 * origin + offset. If the origin + offset is larger than the device's
 * current size, the device memory is reallocated to make room.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static loff_t mycdrv_llseek(struct file *file, loff_t off, int whence)
{
	ASP_mycdrv_t* p;
	char* newRamdisk;
	size_t origSize, newSize, i;

	// acquire device lock
	p = (ASP_mycdrv_t*)file->private_data;
	down_interruptible(&p->sem);
	
	// adjust the file's position value based on the whence value
	switch(whence)
	{
		case LSEEK_ORIGIN_BEGIN:
			// set the cursor to the beginning of the device + offset 
			file->f_pos = 0 + off;
			break;

		case LSEEK_ORIGIN_CURRENT:
			// set the cursor to the current cursor position + offset
			file->f_pos += off;
			break;

		case LSEEK_ORIGIN_END:
			// set the cursor to the end of file + offset
			pr_info("NOTICE (mycdrv_llseek): Reallocating device size.\n");
			file->f_pos = ramdisk_size + off - 1;

			// reallocate the ramdisk size
			// 1.) Kzalloc new size memory range with all values init to 0
			// 2.) Copy data over from original ramdisk
			// 3.) Free the original ramdisk
			// 4.) Reassign ramdisk to new ramdisk + ramsize
			origSize = p->ramsize;
			newSize = p->ramsize + off*sizeof(char);

			// 1.
			if ((newRamdisk = (char*)kzalloc(newSize, GFP_KERNEL)) != NULL)
			{
				// 2.
				for (i = 0; i < origSize; i++)
					newRamdisk[i] = p->ramdisk[i];

				// 3.
				kfree(p->ramdisk);

				// 4.
				p->ramdisk = newRamdisk;
				p->ramsize = newSize;
			}
			else
			{
				pr_info("ERROR (llseek): Could not reallocate memory.\n");
			}
			break;

		default:
			pr_info("ERROR (mycdrv_llseek): Invalid offset or seek mode.\n");
			break;
	}

	// release device lock
	up(&p->sem);

	return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_ioctl
 * This function resets device memory and places file offset to 0.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static long mycdrv_ioctl(struct file *file, unsigned int cmd, unsigned long dir)
{
	ASP_mycdrv_t* p;
	p = (ASP_mycdrv_t*)file->private_data;

	switch(cmd)
	{
		case ASP_CLEAR_BUF:
			// Reset device memory and set position to 0.
			pr_info("NOTICE: Clearing device memory.\n");
			down_interruptible(&p->sem);
			memset((void *)p->ramdisk, 0, p->ramsize);
			file->f_pos = 0;
			up(&p->sem);
			break;
		
		default:
			// Undefined command returns error.
			pr_info("ERROR (mycdrv_ioctl): Invalid cmd flag.\n");
			return -1;
	}

	return 0;
}

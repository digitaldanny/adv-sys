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
#define ramdisk_size (size_t) (16 * PAGE_SIZE) // ramdisk size 

#define CDRV_IOC_MAGIC 'Z'
#define ASP_CLEAR_BUF _IOW(CDRV_IOC_MAGIC, 1, int)

/*
 * *************************************************************************
 *                                 STRUCTS
 * *************************************************************************
*/

typedef struct ASP_mycdrv {
	struct cdev cdev;
	char *ramdisk;
	struct semaphore sem;
	int devNo;
} ASP_mycdrv_t;

/*
 * *************************************************************************
 *                                 GLOBALS
 * *************************************************************************
*/

ASP_mycdrv_t* device; 		// array of devices
dev_t first; 				// will contain major number + FIRST assigned minor number
int major; 					// for easily finding device id for each minor number
struct class* device_class; // blueprint struct for making variable number of devices

// Module parameter defaults
int NUM_DEVICES = 3; 	// Max number of devices.
int MAJOR = 500;
int MINOR = 0;

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
	.compat_ioctl = mycdrv_ioctl,
};

// module init+exit
module_init(my_init);
module_exit(my_exit);

// module parameters (all modifiable at load time)
module_param(NUM_DEVICES, int, S_IRUGO);
module_param(MAJOR, int, S_IRUGO);
module_param(MINOR, int, S_IRUGO);

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
		d->cdev.owner = THIS_MODULE;
		d->cdev.ops = &mycdrv_fops;
		d->devNo = i;
		d->ramdisk = (char*)kzalloc(ramdisk_size, GFP_KERNEL);
		sema_init(&d->sem, 1); // binary semaphore = mutex
		cdev_add(&d->cdev, devId, 1);

		/*
		 * Create device with following configurations:
		 * Parent Device = None
		 * Device name = mycdrv[deviceNo]
		*/
		device_create(device_class, NULL, devId, NULL, MYDEV_NAME "%d", i);
		pr_info("\nSucceeded in registering character device %s%d\n", MYDEV_NAME, i);
	}
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
	/*
	cdev_del(my_cdev);
	unregister_chrdev_region(first, count);
	pr_info("\ndevice unregistered\n");
	kfree(ramdisk);
	*/
	int i;

	// deallocate each device's ramdisk, cdev, and device
	for (i = 0; i < NUM_DEVICES; i++)
	{
		ASP_mycdrv_t* d = &device[i];
		kfree(&d->ramdisk);
		cdev_del(&d->cdev);
		device_destroy(device_class, MKDEV(major, i));
	}

	// deallocate the devices
	kfree(device);

	// destroy the device class
	class_destroy(device_class);

	// unregister the device region for all minor numbers
	unregister_chrdev_region(first, NUM_DEVICES);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_open
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static int mycdrv_open(struct inode *inode, struct file *file)
{
	pr_info(" OPENING device: %s:\n\n", MYDEV_NAME);
	return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_release
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static int mycdrv_release(struct inode *inode, struct file *file)
{
	pr_info(" CLOSING device: %s:\n\n", MYDEV_NAME);
	return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_read
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static ssize_t mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
	int nbytes = 0;

	/*
	if ((lbuf + *ppos) > ramdisk_size) {
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}
	nbytes = lbuf - copy_to_user(buf, ramdisk + *ppos, lbuf);
	*ppos += nbytes;
	pr_info("\n READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	*/

	return nbytes;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_write
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static ssize_t mycdrv_write(struct file *file, const char __user * buf, size_t lbuf, loff_t * ppos)
{
	int nbytes = 0;
	
	/*
	if ((lbuf + *ppos) > ramdisk_size) {
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}
	nbytes = lbuf - copy_from_user(ramdisk + *ppos, buf, lbuf);
	*ppos += nbytes;
	pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	*/

	return nbytes;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_llseek
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static loff_t mycdrv_llseek(struct file *filp, loff_t off, int whence)
{
	return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_ioctl
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static long mycdrv_ioctl(struct file *filp, unsigned int cmd, unsigned long dir)
{
	return 0;
}

#include <linux/module.h>	/* for modules */
#include <linux/fs.h>		/* file_operations */
#include <linux/uaccess.h>	/* copy_(to,from)_user */
#include <linux/init.h>		/* module_init, module_exit */
#include <linux/slab.h>		/* kmalloc */
#include <linux/cdev.h>		/* cdev utilities */

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
	struct cdev dev;
	char *ramdisk;
	struct semaphore sem;
	int devNo;

	// any other field you may want to add
} ASP_mycdrv_t;

/*
 * *************************************************************************
 *                                 GLOBALS
 * *************************************************************************
*/

/*
static dev_t first;
static unsigned int count = 1;
static int my_major = 500, my_minor = 0;
*/
static ASP_mycdrv_t *my_cdev;

// NUM_DEVICES defaults to 3 unless specified during insmod
static int NUM_DEVICES = 3;

/*
 * *************************************************************************
 *                                PROTOTYPES
 * *************************************************************************
*/
static int mycdrv_open(struct inode *inode, struct file *file);
static int mycdrv_release(struct inode *inode, struct file *file);
static ssize_t mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos);
static ssize_t mycdrv_write(struct file *file, const char __user * buf, size_t lbuf, loff_t * ppos);
static int __init my_init(void);
static void __exit my_exit(void);

/*
 * *************************************************************************
 *                           DRIVER CONFIGURATIONS
 * *************************************************************************
*/

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.open = mycdrv_open,
	.release = mycdrv_release,
};

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Daniel Hamilton");
MODULE_LICENSE("GPL v2");

/*
 * *************************************************************************
 *                           FUNCTION DEFINITIONS
 * *************************************************************************
*/

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
	int nbytes;
	if ((lbuf + *ppos) > ramdisk_size) {
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}
	nbytes = lbuf - copy_to_user(buf, ramdisk + *ppos, lbuf);
	*ppos += nbytes;
	pr_info("\n READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: mycdrv_write
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static ssize_t mycdrv_write(struct file *file, const char __user * buf, size_t lbuf, loff_t * ppos)
{
	int nbytes;
	if ((lbuf + *ppos) > ramdisk_size) {
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}
	nbytes = lbuf - copy_from_user(ramdisk + *ppos, buf, lbuf);
	*ppos += nbytes;
	pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: my_init
 * This function runs on installation of this device driver. It is 
 * responsible for allocating memory for the device.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
static int __init my_init(void)
{
	ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
	first = MKDEV(my_major, my_minor);
	register_chrdev_region(first, count, MYDEV_NAME);
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &mycdrv_fops);
	cdev_add(my_cdev, first, count);
	pr_info("\nSucceeded in registering character device %s\n", MYDEV_NAME);
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
	cdev_del(my_cdev);
	unregister_chrdev_region(first, count);
	pr_info("\ndevice unregistered\n");
	kfree(ramdisk);
}

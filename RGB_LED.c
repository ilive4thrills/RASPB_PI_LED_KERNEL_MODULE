/* Joseph Garcia */
/* ECE 331 - Sheaff */
/* Project 1 */

#include <linux/module.h>    /* These two are needed for all module programming */
#include <linux/kernel.h>
#include <linux/fs.h>      /* allows us to use file_operations structure */
#include <linux/cdev.h>       /* allows us to use character device "structures" */
#include <linux/semaphore.h> /* enable locking */
#include <asm/uaccess.h>   /* copy_to_user and copy_from_user */
#include <linux/kdev_t.h>   /* use MAJOR macro for dev_t types */
#include <sys/stat.h>    /* set permissions of device file */
#include <RGB_LED_IOCTL.h> /* includes linux/ioctl.h> */

#define DEV_NAME "RGB_LED" 

#define DEV_AUTHOR "Joseph Garcia"
#define LICENSE "GPL"
#define MOD_DESC "This device allows multiple readers, but will only allow writing after previous value has been read.(ECE 331 HW 7)"

static struct rgb_led_colors RGB_LED; /*create instance of RGB_LED */
static struct cdev* mycdev;  /* pointer "mycdev" points to cdev structure, contains module owner info, fops struct pointer, device number, etc. */
static struct class* dev_cls;

static int majornum;  
static int func_ret; /*generic variable to store values returned by functions */
static dev_t devnum; /* struct to store major and minor number dynamically allocated by the kernel. */
static struct semaphore sema; /*global variable to act as semaphore for the RGB LED */


long rgb_led_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{


}

int open_device(struct inode* inode, struct file* file)   /* open the device */
{
		
	printk(KERN_INFO "Device has been opened\n");
	return 0;  /* successful open */
}

int release_device(struct inode* inode, struct file* file)   /* release the device */
{
	printk(KERN_INFO "Device has been closed.\n"); /* no locking on open or release, since multiple processes can use driver */
	return 0;
}

struct file_operations fops = {
	.owner = THIS_MODULE,		/* "prevent unloading of module when operations are being used" SolidusCode (links function pointers to driver, I assiume) */
	.unlocked_ioctl = rgb_led_ioctl,
	.open = open_device,              /* " " opening the device */
	.release = release_device  /* " " closing the device */
};

/* register capabilities of the device driver with kernel */

static int init_driver(void) {
	
	if((func_ret = alloc_chrdev_region(&devnum,0,1,DEV_NAME)) < 0) {
		printk(KERN_ALERT "unable to allocate major number for device driver.\n");
		return func_ret; /* get that error number */
	}                                                                                     /* allocate device dynamically, check to make sure it worked. */
	

	if ((dev_cls = class_create(THIS_MODULE, "chardrv")) == NULL) {
		unregister_chrdev_region(devnum,1);
		return -1;                                                  /* create class for character device */
	}

	if(device_create(dev_cls, NULL, devnum, NULL, "RGB_LED") == NULL) {
		class_destroy(dev_cls);
		aunregister_chrdev_region(devnum,1);

	majornum = MAJOR(devnum); /* grab 12 MSB from devnum, store them in majornum as the driver's major number */
	mycdev = cdev_alloc(); /* allocate space for charcter device struct */
	if (mycdev == NULL) {
		printk(KERN_ALERT "Failed to allocate and return a cdev structure\n");
	}

	mycdev->ops = &fops; /* ops expects pointer to file_operations struct DO NOT YET HAVE*/
	mycdev->owner = THIS_MODULE; /* say this driver owns the character device structur mycdev */
	
	/* add the character device to the kernel */
	func_ret = cdev_add(mycdev,devnum,1); /*1 since only want 1 minor number */
	if(func_ret < 0) {
		printk(KERN_ALERT "Unable to add character device (driver) struct to kernel. Value %d returned\n",func_ret);
		return func_ret;
	}  /* making sure that struct was added to kernel */

	/* initialize semaphore capabilities */
	sema_init(&sema,1); /* initialize semaphore to 1 (does not initialize mutex semaphore at runtime) */
	
	return 0; /* successful driver initialization */
}

/* function to set the permissions of the device. */
int chmod("/dev/RGB_LED", 0666);  /* make it so that everyone can read and write to the device file "/dev/RGB_LED" */
static int cleanup_driver(void) {  /* unregister everything in reverse order. */
	cdev_del(mycdev); /* free the char device structure space */
	/* N.B. no "cdev_unalloc" function seems to exist. cdev_del somehow cleans up everything.*/
	unregister_chrdev_region(devnum,1); /* start unregistering (1) device number, starting at device number (devnum). */
	printk(KERN_INFO "The kernel char driver hw7_driver has been unloaded from the system.\n");
	/* don't need to return anything */
}

module_init(init_driver);
module_exit(cleanup_driver);            /* driver initialization and exiting macros */

MODULE_AUTHOR(DEV_AUTHOR);
MODULE_LICENSE(LICENSE);            /* For modinfo information listings */
MODULE_DESCRIPTION(MOD_DESC);

#if 0
B)

obj-m += HW7.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
#endif                                                                      


ssize_t read_device(struct file* filp, char* userbuf, size_t bufsize, loff_t* offset)  /* read data from device */
{
      /* using copy_to_user(destination,source,amt2transfer) */
	printk(KERN_INFO "Reading from the device.\n");
	func_ret = copy_to_user(userbuf,hw7_device.value,sizeof(hw7_device.value)); /* copy data from kernel space into user space */
	if (func_ret != 0) {
		printk(KERN_ALERT "did not read all the data from the device.\n");
	}
	if (down_interruptible(&sema) != 0) {  /* If process has sema locked, we unlock it, assuming read went okay */
		up(&sema);
	}
	return func_ret;
}

ssize_t write_device(struct file* filp, const char __user * userbuf, size_t bufsize, loff_t* offset)   /* write data to device */
{
	printk(KERN_INFO "writing to the device\n");
	if (down_interruptible(&sema) != 0) {
		printk(KERN_INFO "Device has already been written to; please read data put in earlier.");
	}
	if(down_interruptible(&sema) == 0) { /*device is available */
		func_ret = copy_from_user(hw7_device.value,userbuf,bufsize); /* copy data from user space */
		if (func_ret != 0) {
			printk(KERN_ALERT "writing to the device did not work.\n");
		}
		if (func_ret == 0) {  /* successful write */
			down_interruptible(&sema);  /* add lock, we successfully wrote the value */
		}
	}
	return func_ret;
}

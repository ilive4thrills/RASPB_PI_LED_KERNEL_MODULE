/* Joseph Garcia */
/* ECE 331 - Sheaff */
/* Project 1 */

#include <linux/module.h>    /* These two are needed for all module programming */
#include <linux/kernel.h>
#include <linux/fs.h>      /* allows us to use file_operations structure */
#include <linux/cdev.h>       /* allows us to use character device "structures" */
#include <linux/semaphore.h> /* enable semaphore locking */
#include <asm/uaccess.h>   /* copy_to_user and copy_from_user */
#include <linux/kdev_t.h>   /* use MAJOR macro for dev_t types */
#include <linux/device.h>    /* for device representation in the kernel. */
#include <linux/types.h>
#include <linux/delay.h>  /* doing delays in kernel space */
#include <linux/gpio.h>   /*includes errno.h as well. */
//#include <sys/stat.h>    /* set permissions of device file */
#include "/home/joseph/Desktop/ece_331_led1/RGB_LED_IOCTL.h" /* includes linux/ioctl.h> */

#define DEV_NAME "RGB_LED" 
#define DEV_AUTHOR "Joseph Garcia"                  /* Information for modinfo command. */
#define LICENSE "GPL"
#define MOD_DESC "This device allows multiple readers, but will only allow writing after previous value has been read.(ECE 331 HW 7)"

static struct cdev* mycdev;  /* pointer "mycdev" points to cdev structure, contains module owner info, fops struct pointer, device number, etc. */
static struct class* dev_cls;  /* create class type for the character device */

char* RGB_LED_GPIO_LBLS[4] = {"Red","Green","Blue","CLK"};   /* Create labels for GPIO pins (used in initialization.) */
static int majornum;  
static int func_ret; /*generic variable to store values returned by functions */
static dev_t devnum; /* struct to store major and minor number dynamically allocated by the kernel. */
static struct semaphore sema; /*global variable to act as semaphore for the RGB LED */

/* led lighting varaibles */
rgb_led_colors RGB_LED;       /* global variable representing color information passed to character device driver. */
int red = 0;
int green = 0; 
int blue = 0;
int redb = 0;             /* local variables used in ioctl() for lighting the LED. */
int greenb = 0;
int blueb = 0;
int j = 0;

static int open_device(struct inode* inode, struct file* file)   /* open the device */
{
	printk(KERN_INFO "Device has been opened\n");
	return 0;  /* successful open */
}

static int release_device(struct inode* inode, struct file* file)   /* release the device */
{
	printk(KERN_INFO "Device has been closed.\n"); /* no locking on open or release, since multiple processes can use driver */
	return 0;
}

static long rgb_led_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd) {
		
		case RGB_LED_R:
			printk(KERN_INFO "Reading RGB_LED not a supported operation for device.\n");
			return -EBADRQC;  								/* return bad request code, cannot read from the RGB LED */
		break;
	
		case RGB_LED_RW:
			printk(KERN_INFO "Read & Write Not a supported operation for device.\n");   /* Cannot read/write with RGB LED */
			return -EBADRQC;
		break;

		case RGB_LED_W:   /* writing to the RGB LED permitted. */
	//	TO DO: 
			// Set device file permissions!!!! (NOT DONE)
			// SET LEDs LOW in the device driver removal (DONE)
			
			if (down_interruptible(&sema) != 0) {
				printk(KERN_ALERT "Another process is currently writing to the RGB_LED device.\n"); 
				return -1;   /* lock the device so only one process can write to it at a time. */
			}
			if ((func_ret = copy_from_user(&RGB_LED, (rgb_led_colors *)arg, sizeof(rgb_led_colors))) != 0) {
				printk(KERN_ALERT "Unable to copy information from user space.\n");  /* ensure that information was able to be passed from user space */
				return func_ret;
			}
			if (func_ret == 0) {
				red = ~(RGB_LED.red);
				green = ~(RGB_LED.green);    /* take complements of color values (ranging from 0 to 2047) passed in by user. */
				blue = ~(RGB_LED.blue);
		   		for(j = 0; j < 11; j++) {
                	  		redb = (red >> (10 - j)) & 1; /* shift red color value msb to 0-bit position, and with 1 to get proper output signal (0 or 1) */
                  			gpio_set_value(RED, redb);

                  			greenb = (green >> (10 - j)) & 1;    /* same as above */
                  			gpio_set_value(GREEN, greenb);

                  			blueb = (blue >> (10 - j)) & 1;     /* same as above */
                  			gpio_set_value(BLUE, blueb);
 
			                  /* create HIGH-LOW clock cycle */
					gpio_set_value(CLK, 1);
					udelay(14) ;   
					gpio_set_value(CLK, 0);
					udelay(14);
				}
				up(&sema);   /* free the lock, now other processes can write to the LED */
			}
		break;  
		default:
			printk(KERN_ALERT "User passed in unrecognized ioctl command for /dev/RGB_LED device.\n");
			return -EBADRQC;
		break;
	}
}


struct file_operations fops = {
	.owner = THIS_MODULE,		/* "prevent unloading of module when operations are being used" SolidusCode (links function pointers to driver, I assiume) */
	.unlocked_ioctl = rgb_led_ioctl,   /* pointer to ioctl function for writing methods on the RGB LED device. */
	.open = open_device,              /* " " opening the device */
	.release = release_device  /* " " closing the device */
};

#if 0
char *module_perm(struct device *dev, umode_t *mode)
{
    if(mode) *mode = 0666;
    return NULL;
}
#endif

/* register capabilities of the device driver with kernel */

static int init_driver(void) {
	int k = 0;
	majornum = MAJOR(devnum); //* grab 12 MSB from devnum, store them in majornum as the driver's major number */
	
	if((func_ret = alloc_chrdev_region(&devnum,0,1,DEV_NAME)) < 0) {
		printk(KERN_ALERT "unable to allocate major number for device driver.\n");
		return func_ret; /* get that error number */
	}                                                                                     /* allocate character device dynamically, check to make sure it worked. */
	

	if ((dev_cls = class_create(THIS_MODULE, "chardrv")) == NULL) {
		unregister_chrdev_region(devnum,1);
		return -1;                                                                 /* create class "chardrv" for character device */
	}

	if(device_create(dev_cls, NULL, devnum, NULL, "RGB_LED") == NULL) { /*ERR_PTR is another name for the failure value */
		class_destroy(dev_cls);
		unregister_chrdev_region(devnum,1);           /* create /dev file for the RGB LED */
		return -1;
	}

	if ((mycdev = cdev_alloc()) == NULL) { /* allocate space for charcter device struct, contains file operations struct */
		printk(KERN_ALERT "Failed to allocate and return a cdev structure\n");
	}

	cdev_init(mycdev, &fops);  /* initialize the character device operations struct (how the kernel represents the character device internally) */		
	mycdev->owner = THIS_MODULE; /* say this driver owns the character device structure mycdev */
	
	/* add the character device to the kernel */
	if((func_ret = cdev_add(mycdev,devnum,1)) < 0) {  /* add character device struct to kernel, 1 flag since we only want one minor number */
		device_destroy(dev_cls, devnum);
		class_destroy(dev_cls);
		unregister_chrdev_region(devnum,1);
		printk(KERN_ALERT "Unable to add character device (driver) struct to kernel. Value %d returned\n",func_ret);
		return func_ret;
	}  /* making sure that struct was added to kernel */

	/* initialize semaphore capabilities */
	sema_init(&sema,1); /* initialize semaphore to 1 (does not initialize mutex semaphore at runtime) */

	for (k = 22; k < 26; k++) {
		if ((func_ret = gpio_request_one(k, GPIOF_OUT_INIT_LOW, RGB_LED_GPIO_LBLS[k-22])) < 0) {
			printk(KERN_ALERT "Unable to request GPIO Pin %d for color %s.\n", k, RGB_LED_GPIO_LBLS[k-22]);  /* Check that GPIO pins can be requested */
			return func_ret;
		} 
	}		
	printk(KERN_INFO "RGB_LED: Device registered.\n");
	return 0; /* successful driver initialization */
}

static int cleanup_driver(void) {  /* unregister everything in reverse order. */
    
    	int k = 0;
	for (k = 22; k < 26; k++) {
		gpio_set_value(k,0);   /* set each GPIO pin back to LOW. (make sure LED is shut off upon module removal) */
		gpio_free(k);              /* freeing the GPIO pins */
	}	
    	cdev_del(mycdev); /* free the char device structure space */
    	device_destroy(dev_cls,devnum);   /* destroy kernel representation of character device */
    	class_destroy(dev_cls);           /* destroy class representing character device */
	unregister_chrdev_region(devnum,1); /* start unregistering (1) device number, starting at device number (devnum). */
	printk(KERN_INFO "RGB_LED: Device unloaded.\n");	
	return 0;
}

module_init(init_driver);
module_exit(cleanup_driver);            /* driver initialization and exiting macros */

MODULE_AUTHOR(DEV_AUTHOR);
MODULE_LICENSE(LICENSE);            /* For modinfo information listings */
MODULE_DESCRIPTION(MOD_DESC);                                                                    


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
#include <linux/device.h>
#include <linux/types.h>
#include <linux/delay.h>  /* doing delays in kernel space */
#include <linux/gpio.h>   /*includes errno.h as well. */
//#include <sys/stat.h>    /* set permissions of device file */
#include "/home/joseph/Desktop/ece_331_led1/RGB_LED_IOCTL.h" /* includes linux/ioctl.h> */

#define DEV_NAME "RGB_LED" 
#define DEV_AUTHOR "Joseph Garcia"
#define LICENSE "GPL"
#define MOD_DESC "This device allows multiple readers, but will only allow writing after previous value has been read.(ECE 331 HW 7)"

 /*create instance of RGB_LED */  /* I think this will come in handy..... */
static struct cdev* mycdev;  /* pointer "mycdev" points to cdev structure, contains module owner info, fops struct pointer, device number, etc. */
static struct class* dev_cls;  /* create class type for the character device */

char* RGB_LED_GPIO_LBLS[4] = {"Red","Green","Blue","CLK"};
static int majornum;  
static int func_ret; /*generic variable to store values returned by functions */
static dev_t devnum; /* struct to store major and minor number dynamically allocated by the kernel. */
static struct semaphore sema; /*global variable to act as semaphore for the RGB LED */

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
	rgb_led_colors RGB_LED; 	
	switch(cmd) {
		
		case RGB_LED_R:
			printk(KERN_INFO "Reading RGB_LED not a supported operation for device.\n");
			return -EBADRQC;  /* return bad request code */
		break;
	
		case RGB_LED_RW:
			printk(KERN_INFO "Read & Write Not a supported operation for device.\n");
			return -EBADRQC;
		break;

		case RGB_LED_W:   /* Turn user space code into kernel space code. */
		TO DO: 
			// Piece of code to reject incoming processes if writing is still going on DONEISH
			// Initialize the clock, found on PIN 25. It needs to be set low initially. DONE
			// Set device file permissions
			// Piece of code to allow writing to the RGB_LED to occur. 
			// set __init and __exit flags????
			
			if (down_interruptible(&sema) != 0) {
				printk(KERN_ALERT "Another process is currently writing to the RGB_LED device.\n");
				return -1;
			}
			if ((func_ret = copy_from_user(RGB_LED, (rgb_led_colors *)arg, sizeof(rgb_led_colors))) != 0) {
				printk(KERN_ALERT "Unable to copy information from user space.\n");
				return func_ret;
			}
			if (func_ret == 0) {
				down_interruptible(&sema);
		   		for(k = 0; k < 11; k++) {
                	  		redb = (red >> (10 - k)) & 1; /* shift red color value msb to 0-bit position, and with 1 to get proper output signal (0 or 1) */
                  			set_gpio_value...bcm2835_gpio_write(RED, redb);
                  			greenb = (green >> (10 - k)) & 1;    /* same as above */
                  			bcm2835_gpio_write(GREEN, greenb);
                  			blueb = (blue >> (10 - k)) & 1;     /* same as above */
                  			bcm2835_gpio_write(BLUE, blueb);
 
			                  /* create HIGH-LOW clock cycle */
        	         		 bcm2835_gpio_write(CLK,HIGH);
                	  		 bcm2835_delayMicroseconds(14);   /* delay for 14 us to account for "twitching" in signal. */
                  			 bcm2835_gpio_write(CLK,LOW);
                  			 bcm2835_delayMicroseconds(14);  /* then move to next iteration */
          			}
				up(&sema);
			}
		break;  
	}
// set __init and __exit flags????
}


struct file_operations fops = {
	.owner = THIS_MODULE,		/* "prevent unloading of module when operations are being used" SolidusCode (links function pointers to driver, I assiume) */
	.unlocked_ioctl = rgb_led_ioctl,
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
		gpio_free(k);              /* freeing the GPIO pins */
	}	
    	cdev_del(mycdev); /* free the char device structure space */
    	device_destroy(dev_cls,devnum);
    	class_destroy(dev_cls);
	unregister_chrdev_region(devnum,1); /* start unregistering (1) device number, starting at device number (devnum). */
	printk(KERN_INFO "RGB_LED: Device unloaded.\n");	/* don't need to return anything */
	return 0;
}

module_init(init_driver);
module_exit(cleanup_driver);            /* driver initialization and exiting macros */

MODULE_AUTHOR(DEV_AUTHOR);
MODULE_LICENSE(LICENSE);            /* For modinfo information listings */
MODULE_DESCRIPTION(MOD_DESC);                                                                    

#if 0
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
#endif

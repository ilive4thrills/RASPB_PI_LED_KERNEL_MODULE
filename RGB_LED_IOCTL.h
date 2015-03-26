/* This is the header file for the RGB_LED DEVICE DRIVER */
/* The device will consistently be called RGB_LED */
/* Joseph Garcia */
/* ECE 331 3/23/2015 */
/* Sheaff */

/* defining the ioctl commands here */

#ifndef RGB_LED_IOCTL_H
#define RGB_LED_IOCTL_H

#include <linux/ioctl.h>

#define RED 22
#define GREEN 23
#define BLUE 24   /* create constants for GPIO pins */

typedef struct {
	int red;
	int blue;
	int green;
} rgb_led_colors;

#define RGB_LED_MN 178  /* 0xB2 = 178, it is available, magic number for device*/
#define RGB_LED_WRITE _IOW(RGB_LED_MN, 1, rgb_led_colors *)
#define RGB_LED_READ _IOR(RGB_LED_MN, 2, int)   /* two basic commands I can think of for the device driver. Write the numbers, then read status of the LED? */

#endif


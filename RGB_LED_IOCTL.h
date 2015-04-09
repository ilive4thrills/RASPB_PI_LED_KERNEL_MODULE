/* Joseph Garcia */
/* ECE 331 3/23/2015 */
/* Sheaff */

/* defining the ioctl commands here */

#ifndef RGB_LED_IOCTL_H
#define RGB_LED_IOCTL_H

#include <linux/ioctl.h>

#define RED 22
#define GREEN 23
#define BLUE 24   /* create constants for GPIO pin numbers */
#define CLK 25

typedef struct {
	int red;
	int blue;      /* declaration of the rgb_led_colors struct type */
	int green;
} rgb_led_colors;

#define RGB_LED_MN 178  /* magic number for device*/
#define RGB_LED_W 420
#define RGB_LED_R 421  /* declaration of ioctl command numbers. */ 
#define RGB_LED_RW 422	/* checked /usr/src/linux-3.18.7/Documentation/ioctl/ioctl-number.txt for available ioctl commands */
#endif


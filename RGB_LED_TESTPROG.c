/* Joseph Garcia */
/* ECE 331 Project 1 */
/* Helper/Tester Program */
/* Sheaff */

#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#include <unistd.h>
#include <inttypes.h>
#include <linux/fs.h>
#include <linux/hdreg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "RGB_LED_IOCTL.h" /* header file for ioctl system calls specific to RGB_LED device */
int main(int argc, char *argv[])
{
        int fd = 0;
	int red = 0;
	int green = 0;
	int blue = 0;
	rgb_led_colors RGB_LED;

	fd = open("/dev/RGB_LED",S_IWUSR|S_IWOTH|S_IWGRP);   /* may have to revice THESE FLAGS!!!!! */     
	
	/* make sure THAT ALL ARGUMENTS ARE PASSED SO WE DON'T SEG FAULT */ 
	red = atoi(argv[1]);
        green = atoi(argv[2]);   /* convert input strings into numbers */
        blue = atoi(argv[3]);

 printf("Passed in numbers for R = %d, G = %d, B = %d\n",red,green,blue);

	RGB_LED.red = red;
	RGB_LED.green = green;
	RGB_LED.blue = blue;
 
	ioctl(fd, RGB_LED_W, &RGB_LED);  /* This is the basic ioctl command... */        
        /* close the program */
	close(fd);
	return 0;
}

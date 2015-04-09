/* Joseph Garcia */
/* ECE 331 Project 1 */
/* Testing Suite */
/* Sheaff */

#include <stdio.h>  
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <linux/fs.h>
#include <linux/hdreg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>   /* for error return standardization */
#include <sys/ioctl.h>
#include "RGB_LED_IOCTL.h" /* header file for ioctl system calls specific to RGB_LED device */

/* Function prototypes go here */
void user_access(int read, int readwrite, int write, int fd, rgb_led_colors RGB_LED);
void run_thru_colors(int fd, rgb_led_colors RGB_LED);

int main(int argc, char *argv[])
{
    	int fd = 0;
	int red = 0;
	int green = 0;
	int blue = 0;   /* local variable initializations. */
	int ret = 0;
	pid_t childpid = 0;
	rgb_led_colors RGB_LED;
	int k = 0;       /* initializations for fork process variables */	
	int j = 0;

	if (argc != 4) {
        	printf("Please enter the proper amount of arguments.\n");  /* account for three colors as well as executable name. */
	        exit(1);
    	}

	if ((fd = open("/dev/RGB_LED",O_RDWR)) == -1) {
        	return fd;                                     /* open device file for read and write from user space perspective */
        	exit(1);
	}
	
	red = atoi(argv[1]);
    	green = atoi(argv[2]);   /* convert input strings into numbers after verifying that three numbers were passed.*/
    	blue = atoi(argv[3]);

    	printf("Passed in numbers for R = %d, G = %d, B = %d\n",red,green,blue);
    	RGB_LED.red = red;
	RGB_LED.green = green;
	RGB_LED.blue = blue;

	user_access(RGB_LED_R, RGB_LED_RW, RGB_LED_W, fd, RGB_LED);  /* Test Item 3 */
	run_thru_colors(fd, RGB_LED);            /* Test Item 1 */

	if ((childpid = fork()) < 0) {
		printf("Unable to create child process.\n");
	}
	
	if (childpid > 0) {      /* parent process */
		RGB_LED.red = 2047;
		RGB_LED.green = 0;
		RGB_LED.blue = 0;
		for (k = 0; k < 1000; k++) {
			if ((ret = ioctl(fd,RGB_LED_W, &RGB_LED)) < 0) {
				printf("Unable to perform operation desired.\n");   /* parent process tries to set the LED red. */
			}
		}
	}
	if (childpid == 0) {      /* child process */
		RGB_LED.red = 0;
		RGB_LED.green = 0;
		RGB_LED.blue = 2047;
		for (j = 0; j < 1000; j++) {
			if ((ret = ioctl(fd,RGB_LED_W, &RGB_LED)) < 0) {
				printf("Unable to perform operation desired.\n");   /* child process tries to set the LED blue. */
			}
		}
	}

	close(fd);    /* close the file descriptor */
	return 0;
}

void user_access(int read, int readwrite, int write, int fd, rgb_led_colors RGB_LED)
{
	int ret = 0;
	if ((ret = ioctl(fd, read, &RGB_LED)) < 0) {
		printf("Reading not supported.\n");
	}

	if ((ret = ioctl(fd, readwrite, &RGB_LED)) < 0) {       /* Verify that reading and writing are not supported. */
		printf("Read/write not supported.\n");
	}

	if ((ret = ioctl(fd, write, &RGB_LED)) < 0) {
		printf("Writing not supported.\n");
	}
}

void run_thru_colors(int fd, rgb_led_colors RGB_LED)
{
	int ret = 0;
	int color = 0;
	for (color = 0; color < 2048; color++) {
		RGB_LED.red = color;	
		RGB_LED.green = 0;
		RGB_LED.blue = 0;
		if ((ret = ioctl(fd, RGB_LED_W, &RGB_LED)) < 0) {
			printf("Unable to perform operation.\n");
		}
	}
	sleep(1);		
	for (color = 0; color < 2048; color++) {
		RGB_LED.red = 0;	
		RGB_LED.green = color;                                /* Make sure we can set different colors (each for loop) */
		RGB_LED.blue = 0;                                     /* make sure we can change color intensities (each color iteration) */
		if ((ret = ioctl(fd, RGB_LED_W, &RGB_LED)) < 0) {
			printf("Unable to perform operation.\n");
		}
	}
	sleep(1);
	for (color = 0; color < 2048; color++) {
		RGB_LED.red = 0;	
		RGB_LED.green = 0;
		RGB_LED.blue = color;
		if ((ret = ioctl(fd, RGB_LED_W, &RGB_LED)) < 0) {
			printf("Unable to perform operation.\n");
		}
	}
	sleep(1);
}

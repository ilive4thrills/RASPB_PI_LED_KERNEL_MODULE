/* Joseph Garcia */
/* ECE 331 Project 1 */
/* Helper/Tester Program */
/* Sheaff */

#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#include <unistd.h>
#include <inttypes.h>
#include "RGB_LED_IOCTL.h" /* header file for ioctl system calls specific to RGB_LED device */
int main(int argc, char *argv[])
{
         if (bcm2835_init() != 1) {
                 printf("bcm2835 library not successfully opened.\n");
                 return 1;
         }

	 int fd = 0;

	fd = open("/dev/RGB_LED",O_WRONLY|O_NONBLOCK|S_IWRUSR);   /* may have to revice THESE FLAGS!!!!! */
         const uint8_t CLK = 25;
         const int NBITS = 11;
         int blue = 0;
         int red = 0;             /* integer values of colors */
         int green = 0;
         int redb = 0;
         int greenb = 0;  /* result of color integers being bitshifted right and then anded with 1 to give appropriate rgb combo to clock */
         int blueb = 0;
         int k = 0; /* looping variable */
 
         /* initialize state of pins */
         bcm2835_gpio_write(RED,LOW);
         bcm2835_gpio_write(GREEN,LOW);
         bcm2835_gpio_write(BLUE,LOW);
         bcm2835_gpio_write(CLK,LOW);
 
         /* intialize R,G,B pins for output */
         bcm2835_gpio_fsel(RED,BCM2835_GPIO_FSEL_OUTP);
         bcm2835_gpio_fsel(GREEN,BCM2835_GPIO_FSEL_OUTP);
         bcm2835_gpio_fsel(BLUE,BCM2835_GPIO_FSEL_OUTP);
         bcm2835_gpio_fsel(CLK,BCM2835_GPIO_FSEL_OUTP);
 
         /* pass in RGB on cmd line. N.B. Enter ./program 0 0 0 on command line to have lights go off */
        /* make sure THAT ALL ARGUMENTS ARE PASSED SO WE DON'T SEG FAULT */ 
	red = atoi(argv[1]);
         green = atoi(argv[2]);   /* convert input strings into numbers */
         blue = atoi(argv[3]);

         if (red < 0 || green < 0 || blue < 0 || red > 2047 || green > 2047 || blue > 2047) {
                 printf("One of your numbers is invalid. Exiting Program.\n");     /* make sure numbers are valid */
                 return 2;
         }
         printf("Passed in numbers for R = %d, G = %d, B = %d\n",red,green,blue);
 
         red = ~red;
         green = ~green;  /* complement values for later bitshifting */
         blue = ~blue;
 
         for(k = 0; k < NBITS; k++) {
                 redb = (red >> (10 - k)) & 1; /* shift red color value msb to 0-bit position, and with 1 to get proper output signal (0 or 1) */
                 bcm2835_gpio_write(RED, redb);
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

         /* close the program */
         if (bcm2835_close() == 0) {
                 printf("Unable to deallocate memory and close program.\n");
                 return 3;
         }
	return 0;
}

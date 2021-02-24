/* pi_blink_sysfs.c
 *
 * Raspberry Pi GPIO example using sysfs interface.
 * Guillermo A. Amaral B. <g@maral.me>
 * Modified by Solan Bongase
 * This file blinks GPIO 18while reading GPIO 23.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>
#include <sys/ioctl.h>

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define PIN  23
#define POUT 18

#define VALUE_MAX 45
/*
 * waitForInterrupt:
 *	Wait for Interrupt on a GPIO pin.
 *	This is actually done via the /sys/class/gpio interface regardless of
 *	the wiringPi access mode in-use. Maybe sometime it might get a better
 *	way for a bit more efficiency.
 *********************************************************************************
 */

int waitForInterruptSys (int fd, int mS)
{
  int x ;
  uint8_t c ;
  struct pollfd polls ;

// Setup poll structure

  polls.fd     = fd ;
  polls.events = POLLPRI ;	// Urgent data!

// Wait for it ...

  x = poll (&polls, 1, mS) ;

//  Do a dummy read to clear the interrupt
//	A one character read appars to be enough.

  //(void)read (fd, &c, 1) ;
    int i, count;
    ioctl (fd, FIONREAD, &count) ;
    for (i = 0 ; i < count ; ++i)
        read (fd, &c, 1) ;        
  

  return x ;
}

static int setEdgeGPIO(int gpio, char *edge) {
    char buf[VALUE_MAX];
    int len = snprintf(buf, VALUE_MAX, "/sys/class/gpio/gpio%d/edge", gpio);
    int fd = open(buf, O_WRONLY);
    write(fd, edge, strlen(edge) + 1);
    close(fd);
    return 0;
}

static int openGPIO(int gpio) {
    int fd;
    int len;
    char buf[VALUE_MAX];
	snprintf(buf, VALUE_MAX, "/sys/class/gpio/gpio%d/value", gpio);
	fd = open(buf, O_RDONLY);
    return fd;
}

static int
GPIOExport(int pin)
{
#define BUFFER_MAX 3
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

static int
GPIOUnexport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

static int
GPIODirection(int pin, int dir)
{
	static const char s_directions_str[]  = "in\0out";

#define DIRECTION_MAX 35
	char path[DIRECTION_MAX];
	int fd;

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		return(-1);
	}

	if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
		fprintf(stderr, "Failed to set direction!\n");
		return(-1);
	}

	close(fd);
	return(0);
}

static int
GPIORead(int pin)
{

	char path[VALUE_MAX];
	char value_str[3];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
	}

	if (-1 == read(fd, value_str, 3)) {
		fprintf(stderr, "Failed to read value!\n");
		return(-1);
	}

	close(fd);

	return(atoi(value_str));
}

static int
GPIOWrite(int pin, int value)
{
	static const char s_values_str[] = "01";

	char path[VALUE_MAX];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return(-1);
	}

	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}

	close(fd);
	return(0);
}

int
main(int argc, char *argv[])
{
	int fd;
	uint32_t wRepeat = 30;
	/*
	 * Enable GPIO pins
	 */
	if (-1 == GPIOExport(POUT) || -1 == GPIOExport(PIN))
		return(1);

	/*
	 * Set GPIO directions
	 */
	if (-1 == GPIODirection(POUT, OUT) || -1 == GPIODirection(PIN, IN))
		return(2);
#if 0
	do {
		if (GPIORead(PIN) == 1) {
			// Turn off LED
			GPIOWrite(POUT,1);
		}
		else {
			// Turn On LED
			GPIOWrite(POUT,0);
		}
		usleep(500 * 1000);
	}
	while (wRepeat--);
#endif	
    //setEdgeGPIO(PIN, "rising");
    setEdgeGPIO(PIN, "both");
    
    fd = openGPIO(PIN);
        
    int i, count;
    uint8_t c;
    ioctl (fd, FIONREAD, &count) ;
    for (i = 0 ; i < count ; ++i)
        read (fd, &c, 1) ;        
        
	do {
		waitForInterruptSys(fd, -1);
		
		if (GPIORead(PIN) == 1) {
			// Turn off LED
			GPIOWrite(POUT,1);
		}
		else {
			// Turn On LED
			GPIOWrite(POUT,0);
		}
		usleep(500 * 1000);
	    printf("Interrupt\n");
	}
	while (wRepeat--);
	
	
	/*
	 * Disable GPIO pins
	 */
	if (-1 == GPIOUnexport(POUT) || -1 == GPIOUnexport(PIN))
		return(4);

	return(0);
}

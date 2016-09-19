/*addtion.c*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "key.h"


#define SDA_PATH "/media/sda1/srcseq.txt"

#define LED_ALL 0xff

#define LEDPATH	"/dev/led"

static int sda_exist(void)
{
	int ret = access(SDA_PATH, F_OK);
	return ret == 0 ? 1 : 0;
}

void led_blink(int led_num)
{
	int i, ledfd;

	ledfd = open(LEDPATH, O_RDWR);
	if(-1 == ledfd) {
		perror("open");
		return;
	}

	for(i = 0; i < 3; i++) {
		if(0xff == led_num) {
			ioctl(ledfd, LED_ON, LED_Z);
			ioctl(ledfd, LED_ON, LED_W);
			ioctl(ledfd, LED_ON, LED_set);
			usleep(500000);
			ioctl(ledfd, LED_OFF, LED_Z);
			ioctl(ledfd, LED_OFF, LED_W);
			ioctl(ledfd, LED_OFF, LED_set);
			usleep(500000);
		}
		else {
			ioctl(ledfd, LED_ON, led_num);
			usleep(500000);
			ioctl(ledfd, LED_OFF, led_num);
			usleep(500000);
		}
	}
	
}

void check_usb_function(void)
{
	printf("check_usb_function\n");
	if(sda_exist()) {
		printf("usb is ok\n");
		led_blink(LED_Z);
	}
}


#ifndef _KEY_H
#define _KEY_H

#include <sys/time.h>

#define LED_set	0
#define LED_Z	3
#define LED_W	4


#define LED_ON 	1
#define LED_OFF	0

extern int led_fd;
extern int led_cnt;

extern void *key_event_process(void *args);
extern void set_led_onoff(int led, int on);
extern void start_led_timer(int duration);

#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/input.h>
#include "mtZdo.h"
#include "commands.h"
#include "mtSapi.h"
#include "key.h"

#define NOKEY 0
#define PERMIT_JOINING_DURATION	120

extern devStates_t devState;

pthread_mutex_t state_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t state_wait = PTHREAD_COND_INITIALIZER;

int led_cnt = -1;
int led_fd;


void set_led_onoff(int led, int on)
{
	if(led_fd > 0)
		ioctl(led_fd, on, led);
}

#if 0
int *get_led_cnt(void)
{
	return &led_cnt;
}

void set_led_cnt(int cnt)
{
	led_cnt = cnt;
}

void *wait_led_off(void *arg)
{
	int *ledcnt;
	for(;;) {
		ledcnt = get_led_cnt();
		//printf("led_cnt is %d\n", *ledcnt);
		if(*ledcnt > 0)
			(*ledcnt)--;
		else if(*ledcnt == 0) {
			set_led_onoff(LED_set, LED_OFF);
			(*ledcnt)--;
		}
		sleep(1);
	}
}
#endif

static unsigned char clear_flag = 0;
static unsigned char key_down = 0;
static int ledcnt = -1;

void start_led_timer(int duration)
{	
	ledcnt = duration;
	ioctl(led_fd, LED_ON, LED_set);
}

void *key_down_count(void *arg)
{
	int count = 0;
	while(1) {
		if(key_down) {
			//printf("count:%d\n", count);
			ledcnt = PERMIT_JOINING_DURATION;
			if(count++ > 30) {
				clear_flag = 1;
				ioctl(led_fd, LED_OFF, LED_W);
				usleep(500000);
				ioctl(led_fd, LED_ON, LED_W);
				usleep(500000);
			}
		}
		else {
			//printf("key up\n");
			count = 0;
			if(ledcnt > 0) {
				//printf("ledcnt: %d\n", ledcnt);
				ledcnt--;
			}
			else if(ledcnt == 0) {
				//printf("ledcnt: 0\n");
				ioctl(led_fd, LED_OFF, LED_set);
				ledcnt--;
			}
		}
		sleep(1);
	}
	
	return (void *)0;
}

void led_monitor_start(void)
{
	pthread_t tid;
	
	led_cnt = -1;
	//pthread_create(&tid, NULL, wait_led_off, NULL);
	pthread_create(&tid, NULL, key_down_count, NULL);
}

extern uint8_t initDone;

void *key_event_process(void *args)
{
	int keys_fd;	
	struct input_event t;
	char *dev;
	PermitJoiningReqFormat_t request;
	//struct sigaction sa;
	//struct itimerval timer;
	printf("key_event_process\n");
	pthread_mutex_lock(&state_lock);
	if(devState != DEV_ZB_COORD) {
		pthread_cond_wait(&state_wait, &state_lock);
	}
	pthread_mutex_unlock(&state_lock);

	set_led_onoff(LED_Z, LED_ON);
	initDone = 1;
	
    setvbuf(stdout, (char *)NULL, _IONBF, 0);//disable stdio out buffer;
	dev = getenv("KEYPAD_DEV");
    
    keys_fd = open(dev, O_RDONLY);
	if(keys_fd<=0)
	{
        printf("open %s device error!\n",dev);
		return 0;
	}

	led_monitor_start();
#if 0	
	request.Destination = 0x0000;
	request.Timeout = 0; 
	sendcmd((unsigned char *)&request, ZB_PERMIT_JOINING_REQ);
#endif	
	while(1)
	{	
        if(read(keys_fd,&t,sizeof(t))==sizeof(t)) {
		    if((t.type==EV_KEY) && (t.value==0 || t.value==1))
			{
				switch(t.code)
				{
			    	case 257:
			    		printf("key257 %s\n",(t.value)?"Released":"Pressed");
			    	break;
			    	
			    	case 258:
			    		printf("key258 %s\n",(t.value)?"Released":"Pressed");
						if(0 == t.value) {
							key_down = 1;
						}
						else if(1 == t.value) {
							key_down = 0;
							if(clear_flag) {
								printf("clear nv param\n");
								system("touch /home/root/neednv");
								system("rm /home/root/gateway.db");																
								ioctl(led_fd, LED_OFF, LED_W);
								usleep(500000);
								ioctl(led_fd, LED_ON, LED_W);
								usleep(500000);
								system("reboot");
							}
							else {
								printf("start_led_timer\n");
								start_led_timer(PERMIT_JOINING_DURATION);
								request.Timeout = PERMIT_JOINING_DURATION; //allowed joining whthin 60s
								sendcmd((unsigned char *)&request, ZB_PERMIT_JOINING_REQ);
							}
						}
						#if 0
						if(1 == t.value) {
							set_led_cnt(PERMIT_JOINING_DURATION);
							set_led_onoff(LED_set, LED_ON);							
							request.Timeout = PERMIT_JOINING_DURATION; //allowed joining whthin 60s
							sendcmd((unsigned char *)&request, ZB_PERMIT_JOINING_REQ);
						}
						#endif
							
			    	break;
			    	
			    	case 259:
			    		printf("key259 %s\n",(t.value)?"Released":"Pressed");
			    	break;
			    	
			    	case 260:
			    		printf("key260 %s\n",(t.value)?"Released":"Pressed");
			    	break;

					case 261:
						printf("key261 %s\n",(t.value)?"Released":"Pressed");
					break;
			    	
			    	default:
			    		break;
			    }
			}
		}
	}	
	close(keys_fd);
	
    return 0;
}


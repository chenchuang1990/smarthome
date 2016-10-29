
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "connection.h"
#include "cetimer.h"
#include "socket.h"
#include "innercmd.h"

typedef void (*sighandler_t)(int);


struct cetimer{ 
	struct itimerval timer;
	sighandler_t handler;
	struct timeval tnexttime;
	struct timeval tinterval;
	int wfd;
	int reconnwfd;
	//int ledwfd;
};

static struct cetimer * s_timer = NULL;

void checkstatus(int i){ 
	sendnonblocking(s_timer->wfd, CECHECK, 1);
	sendnonblocking(s_timer->reconnwfd, CERECONN, 1);
	time_t t = time(NULL);
	if(t%60==0){
		sendnonblocking(s_timer->wfd, HEARTBEAT, 1);
	}
	printf("[checkstatus]\n");
	/*if(s_timer->ledwfd > 0) {
		sendnonblocking(s_timer->ledwfd, LEDCHECK, 1);
	}*/
}

struct cetimer * cetimer_create(unsigned int nextvalue, unsigned int interval, int wfd, int reconnwfd){
	if(s_timer == NULL){
		struct cetimer * timer = (struct cetimer *)malloc(sizeof(struct cetimer));
		timer->tnexttime.tv_sec = nextvalue;
		timer->tinterval.tv_sec = interval;
		timer->timer.it_interval = timer->tinterval;
		timer->timer.it_value = timer->tnexttime;
		setitimer(ITIMER_REAL, &timer->timer, 0);
		timer->handler = checkstatus;
		make_socket_non_blocking(wfd);
		timer->wfd = wfd;
		make_socket_non_blocking(reconnwfd);
		timer->reconnwfd = reconnwfd;
		//timer->ledwfd = -1;

		s_timer = timer;
	}

	return s_timer;
}

void cetimer_start(struct cetimer * timer){
	signal(SIGALRM, timer->handler);
}



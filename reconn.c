#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "eventhub.h"
#include "connection.h"
#include "socket.h"
#include "gateway.h"
#include "innercmd.h"

struct connection * g_serverconn = NULL;

struct reconn{ 
	int rfd;
	int wfd;
	pthread_t reconnthread;
	struct eventhub * hub;

};

void event_reconnect(struct eventhub * hub, int wfd){
	if(!connlist_check(CONNSOCKETSERVER)){
	//	fprintf(stdout, "errno %d %s %s\n", errno, strerror(errno), __FUNCTION__);
		struct connection * serverconn = connectserver();
		
		if(serverconn){
		        eventhub_register(hub,connection_getfd(serverconn)); 
			int n = write(wfd, CESEND, 1);
			if(n <= 0){
				fprintf(stdout, "%d %s \n ", errno, strerror(errno));
			}
		}
	}
}

void * ceconnect(void * args){ 
	struct reconn * rc = (struct reconn *)args;
	int rfd = rc->rfd;
	int wfd = rc->wfd;
	struct eventhub * hub = rc->hub;
	for(;;){
		char buf[1024];

		int n = read(rfd, buf, sizeof(buf)); 
		if(n > 0)
			event_reconnect(hub, wfd);
	}
}

void reconn_start(int rfd, int wfd, struct eventhub * hub){
	struct reconn  * rc = (struct reconn *)malloc(sizeof(struct reconn));
	memset(rc, 0, sizeof(struct reconn));
	rc->rfd = rfd;
	rc->hub = hub;
	rc->wfd = wfd;
	pthread_t threadid;
	pthread_create(&threadid, NULL, &ceconnect, rc);
}

#ifndef __EVENTHUB_H_H_
#define __EVENTHUB_H_H_

#include "eventhub.h"

struct eventhubconf{
	char port[8];
};

struct eventhub{
	struct eventhubconf * conf;
	int epollfd;
	int listenfd;
	struct epoll_event * events;
};


struct eventhub * eventhub_create(struct eventhubconf * conf);
void eventhub_register(struct eventhub * hub,int fd);
void eventhub_deregister(struct eventhub * hub,int fd);
void eventhub_start(struct eventhub * hub);

#endif

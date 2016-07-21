#ifndef __EVENTHUB_H_H_
#define __EVENTHUB_H_H_

struct eventhubconf{
	char port[8];
};

struct eventhub;

struct eventhub * eventhub_create(struct eventhubconf * conf);
void eventhub_register(struct eventhub * hub,int fd);
void eventhub_start(struct eventhub * hub);

#endif

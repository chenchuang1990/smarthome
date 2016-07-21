#ifndef __CE_CONNECTION_H_H
#define __CE_CONNECTION_H_H

#include <time.h>
#include "list.h"
#include "rbtree.h"
#include "kfifo.h"

#define CONNFREE 0
#define CONNZNP 1
#define CONNSOCKETCLIENT 2
#define CONNSOCKETSERVER 3
#define CONNSOCKETCMD 4

struct connection{
	int fd;
	unsigned char type;
	struct kfifo * rawfifo;
	struct list_head list;
	struct rb_node node;
	time_t timestamp;
};

void connection_init(struct connection * c, int fd, unsigned char type);
void connection_put(struct connection * c, unsigned char * buf, unsigned int buflen);
unsigned int connection_get(struct connection* c, unsigned char * buffer, unsigned int msglen);
unsigned int connection_readbuf_getlen(struct connection *c);
int connection_readbuf_getahead(struct connection *c, unsigned char * buf, unsigned int buflen);
void connection_readbuf_pop(struct connection *c);
int connection_getfd(struct connection * c);
unsigned char connection_gettype(struct connection * c);
void connection_close(struct connection * c);

struct connection * freeconnlist_getconn(); 
void freeconnlist_add(struct connection * c); 

struct list_head * connlist_get();
int connlist_check(unsigned char conntype); 
void connlist_checkstatus(long timestamp);
int connlist_getserverfd();

struct connection * connrbtree_getconn(int fd);
void connrbtree_insert(struct connection *c);
void connrbtree_del(struct connection * c); 

#endif

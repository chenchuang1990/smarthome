#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include "connection.h"
#include "ceconf.h"

#define MAXBUFLEN 1024

struct connection * connection_create(){
	struct connection * conn = (struct connection*) malloc(sizeof(struct connection));
	memset(conn, 0, sizeof(struct connection));
	conn->rawfifo = kfifo_init(1024);
	INIT_LIST_HEAD(&conn->list);
	conn->timestamp = time(NULL);

	return conn;
}

void connection_destroy(struct connection * c){ 
	kfifo_free(c->rawfifo);
	free(c);
}

void connection_init(struct connection * c, int fd, unsigned char type){
	c->fd = fd;
	c->type = type;
}

void connection_put(struct connection * c, unsigned char * buf, unsigned int buflen){ 
	kfifo_put(c->rawfifo, buf, buflen);
}

unsigned int connection_get(struct connection * c, unsigned char * buffer, unsigned int msglen){
	return kfifo_get(c->rawfifo, buffer, msglen);
}

unsigned int connection_readbuf_getlen(struct connection *c){
	return kfifo_len(c->rawfifo);
}	

int connection_readbuf_getahead(struct connection *c, unsigned char * buf, unsigned int buflen){
	return kfifo_getahead(c->rawfifo, buf, buflen);
}

void connection_readbuf_pop(struct connection *c){ 
	kfifo_pop(c->rawfifo);
}

int connection_getfd(struct connection * c){
	return c->fd;
}

unsigned char connection_gettype(struct connection * c){
	return c->type;
}

void connection_close(struct connection * c){
	close(c->fd);
	connrbtree_del(c); 
}

static LIST_HEAD(freeconnlisthead);

struct connection * freeconnlist_getconn(){ 
	if(list_empty(&freeconnlisthead)){
		return connection_create();
	}else{
		struct connection *c = list_first_entry(&freeconnlisthead, struct connection, list);
		kfifo_reset(c->rawfifo);
		list_del_init(&c->list);
		c->timestamp = time(NULL);
		c->type=CONNFREE;
		c->fd=0;
		memset(&c->node, 0, sizeof(struct rb_node));
		
		return c;
	}
}

void freeconnlist_add(struct connection * c){ 
	list_add_tail(&c->list, &freeconnlisthead);
}

static struct rb_root connrbtreeroot = {NULL};

static LIST_HEAD(connlisthead);

struct list_head * connlist_get(){
	return &connlisthead;
}

int connlist_check(unsigned char conntype){ 
	struct list_head * pos, *n;
	list_for_each_safe(pos, n, &connlisthead){
		struct connection * c = list_entry(pos, struct connection, list);
		if(c->type == conntype){
			return 1;
		}
	}

	return 0;
}

void connlist_checkstatus(long timestamp){
	struct list_head * pos, *n;
	list_for_each_safe(pos, n, &connlisthead){
		struct connection * c = list_entry(pos, struct connection, list);
		if((c->type == CONNSOCKETSERVER || c->type == CONNSOCKETCLIENT) && 
				timestamp - c->timestamp > ceconf_gettimeout()){
			connection_close(c);
		}
	}
}

int connlist_getserverfd(){
	struct list_head * pos, *n;
	list_for_each_safe(pos, n, &connlisthead){
		struct connection * c = list_entry(pos, struct connection, list);
		if(c->type == CONNSOCKETSERVER ){
			return c->fd;
		}
	}

	return -1;
}

struct connection * connrbtree_getconn(int fd){
	struct rb_root *root = &connrbtreeroot;
	struct rb_node *node = root->rb_node;

	struct connection * c; 
	while(node){
		c  = rb_entry(node, struct connection, node);
		if(fd > c->fd){
			node = node->rb_right;
		}else if(fd < c->fd){
			node = node->rb_left;
		}else{
			return c;
		}
	}

	return NULL;
}


void _connrbtree_dump(){ 
	struct rb_node * node = rb_first(&connrbtreeroot);
	struct connection *c = rb_entry(node, struct connection, node);
	fprintf(stdout, "--fd %d type %d \n", c->fd, c->type);
	struct rb_node * n = rb_next(&c->node);
	while(n){
		struct connection *c = rb_entry(n, struct connection, node);
		fprintf(stdout, "--fd %d type %d \n", c->fd, c->type);
		n=rb_next(&c->node);
	}
}

struct connection * _connrbtree_insert(struct connection *c){
	struct rb_node **newnode = &connrbtreeroot.rb_node, *parent = NULL; 

	struct connection * conn;

	while(*newnode){ 
		parent = *newnode;
		conn  = rb_entry(parent, struct connection, node);
		
		if(c->fd < conn->fd){
			newnode = &((*newnode)->rb_left);
		}else if(c->fd > conn->fd){
			newnode = &((*newnode)->rb_right);
		}else{
			fprintf(stdout, "!!!(((rbtree alread has the fd  %d the conn type is %d\n", c->fd, c->type);
			//_connrbtree_dump();
			return conn;
		}
	}
	rb_link_node(&c->node, parent, newnode);

	return NULL;
}

void connrbtree_insert(struct connection *c){
	//fprintf(stdout, "insert %d %d %x\n", c->fd, c->type, (unsigned int)c);
	struct connection * ret;

	if(( ret = _connrbtree_insert(c))){
		assert(0);
		return;
	}
	rb_insert_color(&c->node, &connrbtreeroot);
	list_add_tail(&c->list, &connlisthead);
	//_connrbtree_dump();
}

void connrbtree_del(struct connection * c){ 
	//fprintf(stdout, "del %d %d %x\n", c->fd, c->type, (unsigned int)c);
	rb_erase(&c->node, &connrbtreeroot);
	list_del_init(&c->list);
	freeconnlist_add(c);
	//_connrbtree_dump();
}

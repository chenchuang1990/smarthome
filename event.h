#ifndef _EVENT_H_H_H_
#define _EVENT_H_H_H_

struct eventhub;
void event_accept(int fd);
void event_recvmsg(struct eventhub *, int fd, unsigned char * buf, int buflen);
void event_close(int fd);
void event_recvznp(struct eventhub * hub, int fd);

#endif
